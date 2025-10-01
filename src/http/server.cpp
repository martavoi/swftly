#include "server.hpp"
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/json.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/trivial.hpp>
#include <boost/system/system_error.hpp>
#include <csignal>
#include <expected>
#include <format>
#include <thread>
#include <vector>

namespace http
{

using namespace std::string_view_literals;
using namespace std::chrono_literals;
namespace json = boost::json;

Server::Server(const conf::Config &config, logging::logger_t &logger, const Router &router,
               boost::asio::io_context &ioc) noexcept
    : config_(config), logger_(logger), router_(router), ioc_(ioc), signals_(ioc, SIGINT, SIGTERM)
{
}

auto Server::start() -> std::expected<void, ServerError>
{
    try
    {
        auto const address = boost::asio::ip::make_address(config_.address());
        boost::asio::ip::tcp::endpoint endpoint{address, static_cast<std::uint16_t>(config_.port())};

        running_ = true;

        BOOST_LOG_SEV(logger_, boost::log::trivial::info)
            << std::format("Swftly URL shortener started on http://{}:{}", config_.address(), config_.port());
        BOOST_LOG_SEV(logger_, boost::log::trivial::info) << "Press Ctrl+C to stop";

        // Setup graceful shutdown
        setup_signal_handling();

        // Spawn the listener coroutine using C++20 coroutines
        boost::asio::co_spawn(ioc_, do_listen(),
                              [this](std::exception_ptr e)
                              {
                                  if (e)
                                  {
                                      try
                                      {
                                          std::rethrow_exception(e);
                                      }
                                      catch (const std::exception &ex)
                                      {
                                          BOOST_LOG_SEV(logger_, boost::log::trivial::error)
                                              << "Listener exception: " << ex.what();
                                      }
                                  }
                              });

        // Run the I/O service on multiple threads if configured
        std::vector<std::thread> threads;
        threads.reserve(config_.threads() - 1);

        for (int i = config_.threads() - 1; i > 0; --i)
        {
            threads.emplace_back([this] { ioc_.run(); });
        }

        // Run on the main thread
        ioc_.run();

        // Wait for all threads to finish
        for (auto &t : threads)
        {
            t.join();
        }

        return {};
    }
    catch (const boost::system::system_error &e)
    {
        const auto ec = e.code();
        if (ec == boost::asio::error::invalid_argument)
        {
            return std::unexpected(ServerError::InvalidAddress);
        }
        if (ec == boost::asio::error::address_in_use)
        {
            return std::unexpected(ServerError::AddressInUse);
        }
        if (ec == boost::asio::error::access_denied)
        {
            return std::unexpected(ServerError::PermissionDenied);
        }

        return std::unexpected(ServerError::UnexpectedError);
    }
    catch (const std::exception &e)
    {
        return std::unexpected(ServerError::UnexpectedError);
    }
}

void Server::stop() noexcept
{
    running_ = false;
    ioc_.stop();
}

void Server::setup_signal_handling() noexcept
{
    signals_.async_wait([this](const boost::system::error_code &error, int signal_number)
                        { handle_signal(error, signal_number); });
}

void Server::handle_signal(const boost::system::error_code &error, int signal_number) noexcept
{
    if (!error)
    {
        std::string_view signal_name;
        switch (signal_number)
        {
        case SIGINT:
            signal_name = "SIGINT";
            break;
        case SIGTERM:
            signal_name = "SIGTERM";
            break;
        default:
            signal_name = "Unknown";
            break;
        }

        BOOST_LOG_SEV(logger_, boost::log::trivial::info)
            << std::format("Received signal {} ({}), shutting down gracefully...", signal_name, signal_number);
        stop();
    }
}

auto Server::do_listen() -> boost::asio::awaitable<void>
{
    auto executor = co_await boost::asio::this_coro::executor;
    boost::asio::ip::tcp::acceptor acceptor(executor);

    // Get the endpoint from the config
    auto const address = boost::asio::ip::make_address(config_.address());
    boost::asio::ip::tcp::endpoint endpoint{address, static_cast<std::uint16_t>(config_.port())};

    // Configure acceptor
    acceptor.open(endpoint.protocol());
    acceptor.set_option(boost::asio::socket_base::reuse_address(true));
    acceptor.bind(endpoint);
    acceptor.listen(boost::asio::socket_base::max_listen_connections);

    BOOST_LOG_SEV(logger_, boost::log::trivial::trace) << "Listener started, accepting connections...";

    // Accept connections forever;
    for (;;)
    {
        auto [ec, socket] = co_await acceptor.async_accept(boost::asio::as_tuple(boost::asio::use_awaitable));

        if (ec)
        {
            BOOST_LOG_SEV(logger_, boost::log::trivial::error) << std::format("accept: {}", ec.message());
        }
        else
        {
            // Spawn a new C++20 coroutine for this connection
            boost::asio::co_spawn(executor, do_session(boost::beast::tcp_stream(std::move(socket))),
                                  [this](std::exception_ptr e)
                                  {
                                      if (e)
                                      {
                                          try
                                          {
                                              std::rethrow_exception(e);
                                          }
                                          catch (const std::exception &ex)
                                          {
                                              BOOST_LOG_SEV(logger_, boost::log::trivial::error)
                                                  << "Session exception: " << ex.what();
                                          }
                                      }
                                  });
        }
    }
}

auto Server::do_session(boost::beast::tcp_stream stream) -> boost::asio::awaitable<void>
{
    boost::beast::flat_buffer buffer;

    BOOST_LOG_SEV(logger_, boost::log::trivial::info)
        << std::format("New connection from {}", stream.socket().remote_endpoint().address().to_string());

    // Handle requests on this connection
    for (;;)
    {
        try
        {
            // Set timeout for this request;
            stream.expires_after(kRequestTimeout);

            // Read the request using C++20 co_await
            boost::beast::http::request<boost::beast::http::string_body> req;
            auto [ec, bytes_read] = co_await boost::beast::http::async_read(
                stream, buffer, req, boost::asio::as_tuple(boost::asio::use_awaitable));

            if (ec)
            {
                // A timeout on a keep-alive connection is a normal event.
                if (ec == boost::beast::error::timeout)
                {
                    BOOST_LOG_SEV(logger_, boost::log::trivial::trace) << "Closing idle connection due to timeout.";
                }
                // This is a graceful shutdown by the remote peer.
                else if (ec == boost::beast::http::error::end_of_stream)
                {
                    BOOST_LOG_SEV(logger_, boost::log::trivial::trace) << "Client closed connection gracefully.";
                }
                // All other reasons are unexpected errors.
                else
                {
                    BOOST_LOG_SEV(logger_, boost::log::trivial::error) << std::format("read: {}", ec.message());
                }
                // On a read error, the connection is closed, so we can exit the session.
                // The stream's destructor will handle closing the socket.
                co_return;
            }

            // Log the request
            BOOST_LOG_SEV(logger_, boost::log::trivial::info)
                << std::format("REQ {} {} - processing", std::string(boost::beast::http::to_string(req.method())),
                               std::string(req.target()));

            // Create the response object that the handler will populate.
            response_t response;

            // Dispatch to the handler. The handler is responsible for the status,
            // content-type, and body.
            co_await router_.dispatch(&req, &response);

            // The server is responsible for common headers.
            response.set(http::field::server, "Swftly");
            response.version(req.version());
            response.keep_alive(req.keep_alive());
            response.prepare_payload();

            // Log response status
            BOOST_LOG_SEV(logger_, boost::log::trivial::info)
                << std::format("RESP: {}", static_cast<unsigned>(response.result()));

            // Send response using C++20 co_await
            auto [write_ec, bytes_written] = co_await boost::beast::http::async_write(
                stream, response, boost::asio::as_tuple(boost::asio::use_awaitable));

            if (write_ec)
            {
                BOOST_LOG_SEV(logger_, boost::log::trivial::error) << std::format("write: {}", write_ec.message());
                break;
            }

            if (response.need_eof())
            {
                // Server decided to close (Connection: close header)
                BOOST_LOG_SEV(logger_, boost::log::trivial::info) << "Closing connection (Connection: close)";
                break;
            }
        }
        catch (const std::exception &e)
        {
            BOOST_LOG_SEV(logger_, boost::log::trivial::error) << std::format("Session exception: {}", e.what());
            break;
        }
    }

    // Graceful shutdown
    boost::beast::error_code ec;
    stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
    if (ec && ec != boost::asio::error::not_connected)
    {
        BOOST_LOG_SEV(logger_, boost::log::trivial::warning) << std::format("Socket shutdown error: {}", ec.message());
    }
    BOOST_LOG_SEV(logger_, boost::log::trivial::info) << "Connection closed gracefully";
}

} // namespace http

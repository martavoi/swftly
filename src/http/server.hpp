#pragma once

#include "conf/conf.hpp"
#include "logging/logger_setup.hpp"
#include "router.hpp"
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
#include <cstdint>
#include <expected>

namespace http
{

using namespace std::string_view_literals;

constexpr std::chrono::seconds kRequestTimeout = std::chrono::seconds(30);

/**
 * @brief Defines the possible errors that the Server can encounter during startup.
 */
enum class ServerError : std::uint8_t
{
    InvalidAddress,   ///< The configured IP address is not valid.
    AddressInUse,     ///< The configured port is already occupied.
    PermissionDenied, ///< Lacking permissions to bind to the address/port (e.g., ports < 1024).
    UnexpectedError   ///< An unknown or unexpected error occurred.
};

/**
 * @brief Manages the core HTTP server lifecycle and connection handling.
 *
 * This class is responsible for setting up the network listener, accepting incoming
 * connections, and orchestrating the request/response cycle using C++20 coroutines.
 * It does not contain any application-specific routing logic, which is delegated
 * to a Router instance provided during construction.
 */
class Server
{
  public:
    /**
     * @brief Constructs the server instance.
     * @param config The application configuration.
     * @param logger The logger instance to use.
     * @param router The router instance for dispatching requests.
     * @param ioc The io_context to use for async operations.
     */
    explicit Server(const conf::Config &config, logging::logger_t &logger, const Router &router,
                    boost::asio::io_context &ioc) noexcept;
    ~Server() = default;

    Server(const Server &) = delete;
    auto operator=(const Server &) -> Server & = delete;
    Server(Server &&) = delete;
    auto operator=(Server &&) -> Server & = delete;

    /**
     * @brief Starts the server and begins listening for connections.
     *
     * This function blocks until the server is stopped via a signal or a call to stop().
     * @return An std::expected indicating success or a ServerError on failure.
     */
    [[nodiscard]] auto start() -> std::expected<void, ServerError>;

    /**
     * @brief Stops the server gracefully.
     *
     * This is thread-safe and can be called from any thread, typically a signal handler.
     */
    void stop() noexcept;

    /**
     * @brief Checks if the server is currently running.
     * @return True if the server is running, false otherwise.
     */
    [[nodiscard]] auto is_running() const noexcept -> bool
    {
        return running_;
    }

  private:
    void setup_signal_handling() noexcept;
    void handle_signal(const boost::system::error_code &error, int signal_number) noexcept;

    // Coroutine-based operations
    auto do_listen() -> boost::asio::awaitable<void>;
    auto do_session(boost::beast::tcp_stream stream) -> boost::asio::awaitable<void>;

    const conf::Config &config_;
    bool running_ = false;
    logging::logger_t &logger_;
    const Router &router_;

    boost::asio::io_context &ioc_;
    boost::asio::signal_set signals_;
};

} // namespace http
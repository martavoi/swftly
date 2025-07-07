#include "storage_service.hpp"
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/consign.hpp>
#include <boost/asio/detached.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/trivial.hpp>
#include <boost/redis/connection.hpp>
#include <boost/redis/logger.hpp>
#include <boost/redis/request.hpp>
#include <boost/redis/response.hpp>
#include <boost/redis/src.hpp> // Required: include this in exactly one source file
#include <format>
#include <string_view>

using namespace std::string_view_literals;

namespace storage
{

StorageService::StorageService(boost::asio::any_io_executor executor, logging::logger_t logger)
    : conn_{std::make_shared<boost::redis::connection>(executor)}, logger_{std::move(logger)}
{
}

auto StorageService::connect(std::string_view host, std::string_view port) -> void
{
    BOOST_LOG_SEV(logger_, boost::log::trivial::info) << "Connecting to Redis at " << host << ":" << port;

    boost::redis::config cfg;
    cfg.addr.host = host;
    cfg.addr.port = port;

    // Start the connection (this handles reconnection automatically)
    conn_->async_run(cfg, boost::redis::logger{boost::redis::logger::level::err},
                     boost::asio::consign(boost::asio::detached, conn_));
}

auto StorageService::generate_next_id() const -> boost::asio::awaitable<std::uint64_t>
{
    BOOST_LOG_SEV(logger_, boost::log::trivial::debug) << "Generating next ID from Redis";

    boost::redis::request req;
    req.push("INCR"sv, kCounterKey);

    boost::redis::response<long long> resp;
    co_await conn_->async_exec(req, resp, boost::asio::use_awaitable);

    const auto &result = std::get<0>(resp);
    if (!result.has_value())
    {
        BOOST_LOG_SEV(logger_, boost::log::trivial::error) << "Redis INCR command returned unexpected response";
        throw std::runtime_error("Redis INCR command returned unexpected response");
    }

    const auto id = static_cast<std::uint64_t>(result.value());

    BOOST_LOG_SEV(logger_, boost::log::trivial::debug) << "Generated ID: " << id;

    co_return id;
}

auto StorageService::store_url(std::uint64_t id, std::string_view url) const -> boost::asio::awaitable<void>
{
    const auto key = std::format("{}{}"sv, kUrlPrefix, id);

    BOOST_LOG_SEV(logger_, boost::log::trivial::debug) << "Storing URL for ID " << id << ": " << url;

    boost::redis::request req;
    req.push("SET"sv, key, url);

    boost::redis::response<std::string> resp;
    co_await conn_->async_exec(req, resp, boost::asio::use_awaitable);

    // Redis SET returns "OK" on success
    const auto &result = std::get<0>(resp);
    if (!result.has_value())
    {
        BOOST_LOG_SEV(logger_, boost::log::trivial::error)
            << "Redis SET command returned unexpected response for ID " << id;
        throw std::runtime_error("Redis SET command returned unexpected response");
    }

    if (result.value() != "OK"sv)
    {
        const auto error_msg = std::format("Redis SET command failed: {}", result.value());
        BOOST_LOG_SEV(logger_, boost::log::trivial::error) << error_msg;
        throw std::runtime_error(error_msg);
    }

    BOOST_LOG_SEV(logger_, boost::log::trivial::debug) << "Successfully stored URL for ID " << id;
}

auto StorageService::get_url(std::uint64_t id) const -> boost::asio::awaitable<std::optional<std::string>>
{
    const auto key = std::format("{}{}"sv, kUrlPrefix, id);

    BOOST_LOG_SEV(logger_, boost::log::trivial::debug) << "Retrieving URL for ID " << id;

    boost::redis::request req;
    req.push("GET"sv, key);

    boost::redis::response<std::string> resp;
    co_await conn_->async_exec(req, resp, boost::asio::use_awaitable);

    const auto &result = std::get<0>(resp);
    if (result.has_value())
    {
        BOOST_LOG_SEV(logger_, boost::log::trivial::debug) << "Found URL for ID " << id << ": " << result.value();
        co_return std::make_optional(result.value());
    }

    BOOST_LOG_SEV(logger_, boost::log::trivial::debug) << "No URL found for ID " << id;
    co_return std::nullopt;
}

auto StorageService::exists(std::uint64_t id) const -> boost::asio::awaitable<bool>
{
    const auto key = std::format("{}{}"sv, kUrlPrefix, id);

    BOOST_LOG_SEV(logger_, boost::log::trivial::debug) << "Checking existence for ID " << id;

    boost::redis::request req;
    req.push("EXISTS"sv, key);

    boost::redis::response<long long> resp;
    co_await conn_->async_exec(req, resp, boost::asio::use_awaitable);

    const auto &result = std::get<0>(resp);
    if (!result.has_value())
    {
        BOOST_LOG_SEV(logger_, boost::log::trivial::error)
            << "Redis EXISTS command returned unexpected response for ID " << id;
        throw std::runtime_error("Redis EXISTS command returned unexpected response");
    }

    // Redis EXISTS returns 1 if key exists, 0 if not
    const bool exists = result.value() == 1;

    BOOST_LOG_SEV(logger_, boost::log::trivial::debug) << "ID " << id << (exists ? " exists" : " does not exist");

    co_return exists;
}

auto StorageService::ping() const -> boost::asio::awaitable<bool>
{
    BOOST_LOG_SEV(logger_, boost::log::trivial::debug) << "Pinging Redis server";

    boost::redis::request req;
    req.push("PING"sv);

    boost::redis::response<std::string> resp;
    co_await conn_->async_exec(req, resp, boost::asio::use_awaitable);

    // Redis PING returns "PONG"
    const auto &result = std::get<0>(resp);
    if (!result.has_value())
    {
        BOOST_LOG_SEV(logger_, boost::log::trivial::error) << "Redis PING command returned unexpected response";
        throw std::runtime_error("Redis PING command returned unexpected response");
    }

    const bool success = result.value() == "PONG"sv;

    BOOST_LOG_SEV(logger_, boost::log::trivial::info) << "Redis ping " << (success ? "successful" : "failed");

    co_return success;
}

} // namespace storage
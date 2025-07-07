#pragma once

#include "logging/logger_setup.hpp"
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/redis/connection.hpp>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace storage
{

/**
 * @brief Pure storage service for URL shortener.
 *
 * This service provides basic storage operations:
 * - Generate incremented integer IDs
 * - Store URL mappings with integer keys
 * - Retrieve URLs by integer keys
 *
 * No encoding/decoding logic - that's handled by higher layers.
 * Methods throw exceptions on errors - callers should handle appropriately.
 */
class StorageService
{
  public:
    /**
     * @brief Constructs the storage service.
     * @param executor The asio executor the connection will use to run.
     * @param logger Logger for Redis operations (passed by value, moved for efficiency).
     */
    explicit StorageService(boost::asio::any_io_executor executor, logging::logger_t logger);

    /**
     * @brief Connect to Redis server.
     *
     * @param host Redis server host
     * @param port Redis server port
     */
    void connect(std::string_view host, std::string_view port);

    /**
     * @brief Generate next incremented ID for a new URL.
     *
     * Uses Redis INCR command to atomically increment a counter.
     *
     * @return The next available ID
     * @throws std::exception on any error
     */
    [[nodiscard]] auto generate_next_id() const -> boost::asio::awaitable<std::uint64_t>;

    /**
     * @brief Store URL mapping with the given ID.
     *
     * @param id The unique identifier for this URL
     * @param url The long URL to store
     * @throws std::exception on any error
     */
    [[nodiscard]] auto store_url(std::uint64_t id, std::string_view url) const -> boost::asio::awaitable<void>;

    /**
     * @brief Retrieve URL by ID.
     *
     * @param id The unique identifier to look up
     * @return The URL if found, nullopt if not found
     * @throws std::exception on any error
     */
    [[nodiscard]] auto get_url(std::uint64_t id) const -> boost::asio::awaitable<std::optional<std::string>>;

    /**
     * @brief Check if an ID exists in storage.
     *
     * @param id The unique identifier to check
     * @return true if ID exists, false if not found
     * @throws std::exception on any error
     */
    [[nodiscard]] auto exists(std::uint64_t id) const -> boost::asio::awaitable<bool>;

    /**
     * @brief Test Redis connectivity with a PING command.
     *
     * @return true if Redis responds correctly
     * @throws std::exception on any error
     */
    [[nodiscard]] auto ping() const -> boost::asio::awaitable<bool>;

  private:
    /// @brief The Redis connection instance
    std::shared_ptr<boost::redis::connection> conn_;

    /// @brief Logger for Redis operations (mutable to allow logging in const methods)
    mutable logging::logger_t logger_;

    // Redis key constants
    static constexpr std::string_view kCounterKey = "url_counter";
    static constexpr std::string_view kUrlPrefix = "url:";
};

} // namespace storage
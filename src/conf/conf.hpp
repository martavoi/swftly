#pragma once

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <expected>
#include <string>
#include <string_view>

namespace conf
{

using namespace std::literals::string_view_literals;

#ifdef NDEBUG
constexpr std::string_view kDefaultLogLevel = "info"sv;
#else
constexpr std::string_view kDefaultLogLevel = "trace"sv;
#endif

constexpr std::string_view kDefaultAddress = "127.0.0.1"sv;
constexpr int kDefaultPort = 8080;
constexpr int kMinPort = 1;
constexpr int kMaxPort = 65535;
constexpr int kMinThreads = 1;

// Redis configuration defaults
constexpr std::string_view kDefaultRedisHost = "127.0.0.1"sv;
constexpr int kDefaultRedisPort = 6379;

/**
 * @brief Defines errors that can occur during configuration loading.
 */
enum class ConfigError : std::uint8_t
{
    HelpRequested,   ///< The user requested the help message (--help). Not a true error.
    InvalidPort,     ///< The specified port is outside the valid range (1-65535).
    InvalidThreads,  ///< The specified thread count is not a positive number.
    EmptyAddress,    ///< The server address string is empty.
    ParseError,      ///< An error occurred while parsing command-line arguments.
    InvalidLogLevel, ///< The specified log level is not one of the allowed values.
    UnexpectedError  ///< An unknown or unexpected error occurred.
};

/**
 * @brief Manages application configuration loaded from command-line arguments.
 *
 * This class uses boost::program_options to define and parse server settings.
 * It also performs validation on the loaded configuration.
 */
class Config
{
  public:
    Config() = default;

    /**
     * @brief Loads and validates configuration from command-line arguments.
     * @param argc The argument count from main().
     * @param argv The argument vector from main().
     * @return An std::expected indicating success or a ConfigError on failure.
     */
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
    [[nodiscard]] auto load(const int &argc, const char *argv[]) -> std::expected<void, ConfigError>;

    /// @brief Gets the server's network listening address.
    [[nodiscard]] auto address() const noexcept
    {
        return std::string_view{address_};
    }

    /// @brief Gets the server's network listening port.
    [[nodiscard]] auto port() const noexcept
    {
        return port_;
    }

    /// @brief Gets the number of worker threads for the server.
    [[nodiscard]] auto threads() const noexcept
    {
        return threads_;
    }

    /// @brief Gets the configured log severity level.
    [[nodiscard]] auto log_level() const noexcept
    {
        return std::string_view{log_level_};
    }

    /// @brief Gets the Redis server host address.
    [[nodiscard]] auto redis_host() const noexcept
    {
        return std::string_view{redis_host_};
    }

    /// @brief Gets the Redis server port.
    [[nodiscard]] auto redis_port() const noexcept
    {
        return redis_port_;
    }

  private:
    [[nodiscard]] auto is_valid() const -> std::expected<void, ConfigError>;

    std::string address_;
    int port_{};
    int threads_{};
    std::string log_level_;
    std::string redis_host_;
    int redis_port_{};
};
} // namespace conf
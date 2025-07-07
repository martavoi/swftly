#include "conf.hpp"
#include <expected>
#include <string_view>
#include <unordered_set>

namespace conf
{

namespace po = boost::program_options;

const static std::unordered_set<std::string_view> kValidLogLevels = {"trace",   "debug", "info",
                                                                     "warning", "error", "fatal"};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
auto Config::load(const int &argc, const char *argv[]) -> std::expected<void, ConfigError>
{
    try
    {
        // Define command line options
        po::options_description desc("Swftly Server Configuration");
        desc.add_options()("help,h", "Show this help message")(
            "address,a", po::value<std::string>(&address_)->default_value(std::string(kDefaultAddress)),
            "Server bind address")("port,p", po::value<int>(&port_)->default_value(kDefaultPort), "Server port")(
            "threads,t", po::value<int>(&threads_)->default_value(kMinThreads), "Number of worker threads")(
            "log-level,l", po::value<std::string>(&log_level_)->default_value(std::string(kDefaultLogLevel)),
            "Log level (trace, debug, info, warning, error, fatal)")(
            "redis-host", po::value<std::string>(&redis_host_)->default_value(std::string(kDefaultRedisHost)),
            "Redis server host address")("redis-port", po::value<int>(&redis_port_)->default_value(kDefaultRedisPort),
                                         "Redis server port");

        // Parse command line arguments
        boost::program_options::variables_map vmap;
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vmap);
        boost::program_options::notify(vmap);

        // Validate configuration
        return is_valid();
    }
    catch (const boost::program_options::error &e)
    {
        return std::unexpected(ConfigError::ParseError);
    }
    catch (const std::exception &e)
    {
        return std::unexpected(ConfigError::UnexpectedError);
    }
}

auto Config::address() const noexcept -> std::string_view
{
    return address_;
}

auto Config::port() const noexcept -> int
{
    return port_;
}

auto Config::threads() const noexcept -> int
{
    return threads_;
}

auto Config::log_level() const noexcept -> std::string_view
{
    return log_level_;
}

auto Config::redis_host() const noexcept -> std::string_view
{
    return redis_host_;
}

auto Config::redis_port() const noexcept -> int
{
    return redis_port_;
}

auto Config::is_valid() const -> std::expected<void, ConfigError>
{
    if (port_ < kMinPort || port_ > kMaxPort)
    {
        return std::unexpected(ConfigError::InvalidPort);
    }

    if (threads_ < kMinThreads)
    {
        return std::unexpected(ConfigError::InvalidThreads);
    }

    if (address_.empty())
    {
        return std::unexpected(ConfigError::EmptyAddress);
    }

    if (!kValidLogLevels.contains(log_level_))
    {
        return std::unexpected(ConfigError::InvalidLogLevel);
    }

    // Validate Redis configuration
    if (redis_host_.empty())
    {
        return std::unexpected(ConfigError::EmptyAddress);
    }

    if (redis_port_ < kMinPort || redis_port_ > kMaxPort)
    {
        return std::unexpected(ConfigError::InvalidPort);
    }

    return {};
}

} // namespace conf
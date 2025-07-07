#include "conf/conf.hpp"
#include "encode/encoder.hpp"
#include "http/handlers/new_short_code_handler.hpp"
#include "http/handlers/ping_handler.hpp"
#include "http/handlers/root_handler.hpp"
#include "http/handlers/short_code_handler.hpp"
#include "http/server.hpp"
#include "logging/logger_setup.hpp"
#include "storage/storage_service.hpp"
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/log/trivial.hpp>
#include <format>
#include <print>

auto main(int argc, const char *argv[]) -> int
{
    // Load configuration from command line
    conf::Config config;
    if (auto result = config.load(argc, argv); !result)
    {
        switch (result.error())
        {
        case conf::ConfigError::HelpRequested:
            return 0; // Help requested is not an error
        case conf::ConfigError::InvalidPort:
            std::println(stderr, "Error: Invalid port. Port must be between 1-65535");
            return 1;
        case conf::ConfigError::InvalidThreads:
            std::println(stderr, "Error: Invalid thread count. Must be positive");
            return 1;
        case conf::ConfigError::EmptyAddress:
            std::println(stderr, "Error: Address cannot be empty");
            return 1;
        case conf::ConfigError::ParseError:
            std::println(stderr, "Error: Failed to parse command line arguments");
            return 1;
        case conf::ConfigError::InvalidLogLevel:
            std::println(stderr, "Error: Invalid log level. Must be one of: trace, debug, info, warning, error, "
                                 "fatal");
            return 1;
        case conf::ConfigError::UnexpectedError:
            std::println(stderr, "Error: Unexpected configuration error");
            return 1;
        }
    }

    try
    {
        // Setup logging
        logging::setup(config);
        logging::logger_t logger;

        // Create io_context and executor first
        boost::asio::io_context ioc;
        auto executor = ioc.get_executor();

        // Create services that need the executor
        storage::StorageService storage{executor, logger};
        encode::Encoder encoder{};

        // Connect to Redis
        BOOST_LOG_SEV(logger, boost::log::trivial::trace)
            << std::format("Starting Redis connection to {}:{}", config.redis_host(), config.redis_port());

        // Start Redis connection (async_run handles everything internally)
        storage.connect(config.redis_host(), std::to_string(config.redis_port()));

        BOOST_LOG_SEV(logger, boost::log::trivial::trace) << "Redis connection started";

        // Setup routing
        http::Router router{http::handler::ShortCodeHandler{executor, encoder, storage}};
        router.add_route(http::RouteKey{http::beast::http::verb::get, "/"}, http::handler::RootHandler{});
        router.add_route(http::RouteKey{http::beast::http::verb::get, "/ping"}, http::handler::PingHandler{});
        router.add_route(http::RouteKey{http::beast::http::verb::post, "/api/urls"},
                         http::handler::NewShortCodeHandler{executor, encoder, storage});

        // Create server with io_context
        http::Server server{config, logger, router, ioc};
        BOOST_LOG_SEV(logger, boost::log::trivial::info) << "Starting Swftly URL shortener...";
        if (auto result = server.start(); !result)
        {
            switch (result.error())
            {
            case http::ServerError::InvalidAddress:
                BOOST_LOG_SEV(logger, boost::log::trivial::fatal)
                    << "Failed to start server: Invalid address configured.";
                break;
            case http::ServerError::AddressInUse:
                BOOST_LOG_SEV(logger, boost::log::trivial::fatal)
                    << "Failed to start server: Address is already in use.";
                break;
            case http::ServerError::PermissionDenied:
                BOOST_LOG_SEV(logger, boost::log::trivial::fatal)
                    << "Failed to start server: Permission denied "
                       "(are you trying to use a privileged port < 1024?).";
                break;
            case http::ServerError::UnexpectedError:
                BOOST_LOG_SEV(logger, boost::log::trivial::fatal)
                    << "Failed to start server due to an unexpected error.";
                break;
            }
            return 1;
        }
    }
    catch (const std::exception &e)
    {
        std::println(stderr, "Fatal error during startup: {}", e.what());
        return 1;
    }

    std::println(stdout, "[OK] Server stopped gracefully");
    return 0;
}
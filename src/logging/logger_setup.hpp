#pragma once

#include "conf/conf.hpp"
#include <boost/log/core.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>

namespace logging
{

/// @brief A convenience alias for the Boost severity logger type used throughout the application.
using logger_t = boost::log::sources::severity_logger<boost::log::trivial::severity_level>;

/**
 * @brief Initializes the global Boost.Log sinks and formatters.
 *
 * This function sets up a console logger with a timestamp and severity level.
 * @param config The application configuration, used to set the logging severity filter.
 */
void setup(const conf::Config &config);

} // namespace logging

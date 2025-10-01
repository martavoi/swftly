#include "logger_setup.hpp"
#include "conf/conf.hpp"
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <cstdio>
#include <iostream>
#include <sstream>

namespace logging
{

void setup(const conf::Config &config)
{
    boost::log::trivial::severity_level level = boost::log::trivial::info;
    std::stringstream ss;
    ss << config.log_level();
    ss >> level;

    boost::log::add_common_attributes();
    boost::log::add_console_log(
        std::cout,
        boost::log::keywords::format =
            (boost::log::expressions::stream
             << "["
             << boost::log::expressions::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
             << "] [" << boost::log::trivial::severity << "] " << boost::log::expressions::smessage));

    boost::log::core::get()->set_filter(boost::log::trivial::severity >= level);
}

} // namespace logging
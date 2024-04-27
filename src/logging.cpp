#include "logging.h"

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/support/date_time.hpp> // Ensure this is included for date-time support


// Define the global logger
src::severity_logger<boost::log::trivial::severity_level> lg;

void init_logging() {
    logging::add_common_attributes();  // Adds common attributes like LineID, TimeStamp

    logging::add_console_log(
        std::cout,
        keywords::format = (
            expr::stream
                << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S")
                << " [" << logging::trivial::severity << "] "
                << expr::smessage
        )
    );

    logging::add_file_log(
        keywords::file_name = "serial_application_%N.log",  // File name pattern
        keywords::rotation_size = 10 * 1024 * 1024,          // Rotate files every 10 MiB
        keywords::time_based_rotation = logging::sinks::file::rotation_at_time_point(0, 0, 0),
        keywords::format = (
            expr::stream
                << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S")
                << " [" << logging::trivial::severity << "] "
                << expr::smessage
        )
    );

    logging::core::get()->set_filter(
        logging::trivial::severity >= logging::trivial::info
    );
}

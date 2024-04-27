#ifndef LOGGING_H
#define LOGGING_H

#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>

// Namespace aliases
namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

extern src::severity_logger<logging::trivial::severity_level> lg;

void init_logging();

#endif // LOGGING_H

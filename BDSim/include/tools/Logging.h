#pragma once
#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <fstream>

#include <boost/timer/timer.hpp>
#include <boost/current_function.hpp>

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>

#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>

#include <boost/log/sinks/text_file_backend.hpp>

#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/from_stream.hpp>

#include <boost/log/attributes/timer.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>

#include <boost/filesystem.hpp>


namespace Logging {
	/// \brief Initializes the Logger.
	/// Loads its configuration from logging.ini file which is to be places in the conf folder. Also sets the precision of
	/// floating points numbers logged to the maximum reasonable value.
	inline void initLogger(){
		// Load attributes for the log messages
		boost::log::add_common_attributes();
		boost::log::register_simple_formatter_factory< boost::log::trivial::severity_level, char >("Severity");
		boost::log::core::get()->add_global_attribute("Timeline", boost::log::attributes::timer());
		// Load configuration
		std::ifstream file("./conf/logging.ini");
		boost::log::init_from_stream(file);
		int digits = 53.0 * log10(2); // Approximate number of decimal digits a 64 bit IEEE 754 float number can hold
		BOOST_LOG_TRIVIAL(info) << std::setprecision(digits);
	}
}
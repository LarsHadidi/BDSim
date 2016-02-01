// Basic headers
#include <iterator>
#include <array>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <exception>
// System environment
#include <thread>
#include <limits>
// Boost lib
#include <boost/predef.h>
#include <boost/program_options.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
// Local header-only libraries
#include <tools/Logging.h>
#include <tools/Setup.h>
// Project components
#include <datastructures/Datastructures.h>
#include <simulator/Simulator.h>

// ---------------------------------------------------------------
// -                Program entry point                          -
// ---------------------------------------------------------------

int main(int argc, char *argv[]) {
	try {
		Logging::initLogger();
	} catch (std::exception & e) {
		std::cout << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}

	// ---------------------------------------------------------------
	// -                Introductory logging                         -
	// ---------------------------------------------------------------
	BOOST_LOG_TRIVIAL(info) << "BDSim started at " << boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time());
	// -------------------information about computer------------------
	BOOST_LOG_TRIVIAL(info) << "Running on the following environment:";
	BOOST_LOG_TRIVIAL(info) << "\tNumber of cores found: " << std::thread::hardware_concurrency();

	#if BOOST_OS_WINDOWS
		BOOST_LOG_TRIVIAL(info) << "\tOperating system: Windows";
	#elif BOOST_OS_LINUX
		BOOST_LOG_TRIVIAL(info) << "\tOperating system: Linux";
	#elif BOOST_OS_MACOS
		BOOST_LOG_TRIVIAL(info) << "\tOperating system: MacOS";
	#elif BOOST_OS_BSD
		BOOST_LOG_TRIVIAL(info) << "\tOperating system: BSD";
	#endif

	// ---------------------information about libs -------------------
	BOOST_LOG_TRIVIAL(info) << "\tBoost library version: " << BOOST_VERSION;

	// ---------------------information about data types--------------


	BOOST_LOG_TRIVIAL(info) << "\tFloat bit size: " << sizeof(float) * 8;
	BOOST_LOG_TRIVIAL(info) << "\tUsing double precision.";
	BOOST_LOG_TRIVIAL(info) << "\t\tBit size: " << sizeof(double) * 8;
	BOOST_LOG_TRIVIAL(info) << "\t\tPrecision: " << std::numeric_limits<double>::epsilon();
	BOOST_LOG_TRIVIAL(info) << "\t\tMinimum value: " << std::numeric_limits<double>::min();
	BOOST_LOG_TRIVIAL(info) << "\t\tMaximum value: " << std::numeric_limits<double>::max();



	// ---------------------------------------------------------------
	// -                Check for required files                     -
	// ---------------------------------------------------------------
	if(!boost::filesystem::exists("./conf/sensors.ini")){
		BOOST_LOG_TRIVIAL(error) << "Sensors initialization file not found: conf/sensors.ini missing";
		exit(EXIT_FAILURE);
	}
	if(!boost::filesystem::exists("./conf/settings.ini")){
		BOOST_LOG_TRIVIAL(error) << "Settings file not found: conf/settings.ini missing";
		exit(EXIT_FAILURE);
	}

	// ---------------------------------------------------------------
	// -                Parse the CLI options                        -
	// ---------------------------------------------------------------

	boost::program_options::options_description opt_description("Allowed options");
	// Program options which has to be passed as command line parameters
	// For list value use boost::program_options::value<std::vector<std::string>>()->multitoken()
	// For boolean flag use boost::program_options::bool_switch()->default_value(false)
	opt_description.add_options()
			("outputpath,o"		,boost::program_options::value<std::string>()	,"set output path")
			("runfile,i"		,boost::program_options::value<std::string>()	,"name of runfile")
			("help,h", "produce help message")
	;
	// Read the provided command line parameters
	boost::program_options::variables_map opt_map;
	try {
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, opt_description), opt_map);
		boost::program_options::notify(opt_map);
	}
	catch(std::exception & e){
		BOOST_LOG_TRIVIAL(error) << e.what();
	}
	// Print help message if requested
	if(opt_map.count("help") != 0) {
		std::cout << opt_description << std::endl;
	}

	// ---------------------------------------------------------------
	// -                Parse the runfile                            -
	// ---------------------------------------------------------------

	boost::property_tree::iptree* initConf = new boost::property_tree::iptree();
	// Check if a runfile has been provided and parse it
	if (opt_map.count("runfile") != 0) {
		try {
			initConf = Setup::readRunFile(opt_map["runfile"].as<std::string>());
		}
		catch (std::exception & e) {
			BOOST_LOG_TRIVIAL(error) << e.what();
		}
	}
	else {
		BOOST_LOG_TRIVIAL(error) << "No runfile provided.";
			exit(1);
	}

	// ---------------------------------------------------------------
	// -                Override runfile with CLI settings           -
	// ---------------------------------------------------------------
	// Check if a output filepath has beed provided and override runfile setting
	if(opt_map.count("outputpath") != 0) {
		initConf->put("outputpath", opt_map["outputpath"].as<std::string>());
		BOOST_LOG_TRIVIAL(info) << "Output path reset to: " << initConf->get<std::string>("outputpath");
	}


	BOOST_LOG_TRIVIAL(info)<< "Parameters set.";



	// ---------------------------------------------------------------
	// -           Initialize the grid and start simulation          -
	// ---------------------------------------------------------------

	BOOST_LOG_TRIVIAL(info)<< "*** Starting Simulator-Component ***";

	Simulator::initialize(initConf);
	Simulator::run();
	Simulator::validateSystem(initConf);
	Simulator::tearDown();

	BOOST_LOG_TRIVIAL(info)<< "Application terminated.";

	exit(EXIT_SUCCESS);
}

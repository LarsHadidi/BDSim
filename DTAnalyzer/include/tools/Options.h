#pragma once

#include <boost/predef.h>
#include <boost/program_options.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <exception>
#include <iostream>
#include <fstream>
#include <tools/Logging.h>

namespace tools {

    class Options {
        public:
            Options();
            ~Options();

            void parseOptMap(boost::program_options::variables_map& opt_map);
            void parseParamsFile(boost::filesystem::path file);
            void parseSettingsFile(boost::filesystem::path file);


            boost::filesystem::path inputFolder;
            boost::filesystem::path outputPath;
            uint32_t particleCount;
            double dimX;
            double dimY;
            double timeStep;
            double simTime;
            double relaxTime;
            double temperature;
            double threshold;

            size_t cacheSize;
            unsigned int threadCount;
    };
}

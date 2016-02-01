#pragma once

#include <regex>
#include <boost/lexical_cast.hpp>

#include <boost/predef.h>
#include <boost/program_options.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

namespace tools {

    class Options {
        public:
            enum DATATYPE {CONFS, VIZ, DEFECTS, PSI6, DTRJ, VCELLS};
            Options();
            ~Options();

            void parseOptMap(boost::program_options::variables_map& opt_map);
            void parseParamsFile(boost::filesystem::path file);

            boost::filesystem::path inputFolder;
            boost::filesystem::path outputPath;
            uint32_t particleCount;
            uint64_t startFrame = 0;
            uint64_t endFrame = 0;
            double dimX;
            double dimY;
            DATATYPE datatype;
    };
}

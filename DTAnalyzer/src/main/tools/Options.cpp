#include <tools/Options.h>

namespace tools {

    Options::Options() {}
    Options::~Options(){}

    void Options::parseSettingsFile(boost::filesystem::path file) {
        try {
            boost::property_tree::ptree paramTree;
            boost::property_tree::ini_parser::read_ini(file.string(), paramTree);

            this->cacheSize = paramTree.get<size_t>("Buffer.cacheSize");
            this->threadCount = paramTree.get<unsigned int>("Buffer.threadCount");

            BOOST_LOG_TRIVIAL(info) << "Software cache size: " << this->cacheSize << " Bytes";
            BOOST_LOG_TRIVIAL(info) << "Number of threads: " << this->threadCount;
        }
        catch(boost::property_tree::ini_parser_error& e){
            BOOST_LOG_TRIVIAL(error) << e.what();
            exit(EXIT_FAILURE);
        }
    }

    void Options::parseParamsFile(boost::filesystem::path file) {
        try {
            boost::property_tree::ptree paramTree;
            boost::property_tree::ini_parser::read_ini(file.string(), paramTree);

            this->particleCount = paramTree.get<uint32_t>("Parameters.particles");
            this->dimX = paramTree.get<double>("Parameters.dimx");
            this->dimY = paramTree.get<double>("Parameters.dimy");
            this->timeStep = paramTree.get<double>("Parameters.timestep");
            this->simTime = paramTree.get<double>("Parameters.simtime");
            this->relaxTime = paramTree.get<double>("Parameters.relaxation");
            this->temperature = paramTree.get<double>("Parameters.temperature");


            BOOST_LOG_TRIVIAL(info) << "Simulation box width: " << this->dimX;
            BOOST_LOG_TRIVIAL(info) << "Simulation box height: " << this->dimY;
            BOOST_LOG_TRIVIAL(info) << "Simulation box particle count: " << this->particleCount;
        }
        catch(boost::property_tree::ini_parser_error& e){
            BOOST_LOG_TRIVIAL(error) << e.what();
            exit(EXIT_FAILURE);
        }
    }

    void Options::parseOptMap(boost::program_options::variables_map& opt_map) {


        if (opt_map.count("input") != 0) {
            this->inputFolder = boost::filesystem::path(opt_map["input"].as<std::string>());
            BOOST_LOG_TRIVIAL(info)<< "Input folder: " << this->inputFolder.string();
        }
        else {
            BOOST_LOG_TRIVIAL(error) << "No input folder specified";
            exit(EXIT_FAILURE);
        }

        if (opt_map.count("output") != 0) {
            this->outputPath = boost::filesystem::path(opt_map["output"].as<std::string>());
            BOOST_LOG_TRIVIAL(info)<< "Output path: " << this->outputPath.string();
        }
        else {
            BOOST_LOG_TRIVIAL(error) << "No output path specified";
            exit(EXIT_FAILURE);
        }
    }
}

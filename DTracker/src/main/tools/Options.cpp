#include <tools/Options.h>
#include <tools/Logging.h>

namespace tools {

    Options::Options() {}
    Options::~Options(){}

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

            EffectiveDiameter effectiveDiameter;
            this->threshold = 2 * effectiveDiameter.estimate(this->temperature);

            BOOST_LOG_TRIVIAL(info) << "Simulation box width: " << this->dimX;
            BOOST_LOG_TRIVIAL(info) << "Simulation box height: " << this->dimY;
            BOOST_LOG_TRIVIAL(info) << "Simulation box particle count: " << this->particleCount;
            BOOST_LOG_TRIVIAL(info) << "Gating threshold: " << this->threshold;
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

    void Options::parseSettingsFile(boost::filesystem::path file) {
        try {
            boost::property_tree::ptree paramTree;
            boost::property_tree::ini_parser::read_ini(file.string(), paramTree);

            this->cacheSize = paramTree.get<size_t>("Buffer.cacheSize");

            BOOST_LOG_TRIVIAL(info) << "Software cache size: " << this->cacheSize << " Bytes";
        }
        catch(boost::property_tree::ini_parser_error& e){
            BOOST_LOG_TRIVIAL(error) << e.what();
            exit(EXIT_FAILURE);
        }
    }
}

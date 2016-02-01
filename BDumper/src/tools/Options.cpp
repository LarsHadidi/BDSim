#include <tools/Options.h>

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
        if (opt_map.count("type") != 0) {
            std::string inputType = opt_map["type"].as<std::string>();
            std::transform(inputType.begin(), inputType.end(), inputType.begin(), ::toupper);
            if(inputType == "CONFS") {
                this->datatype = CONFS;
                BOOST_LOG_TRIVIAL(info)<< "Input type: CONFIGURATIONS";
            }
            else if(inputType == "VIZ"){
                this->datatype = VIZ;
                BOOST_LOG_TRIVIAL(info)<< "Input type: VISUALIZATION";
            }
            else if(inputType == "DEFECTS"){
                this->datatype = DEFECTS;
                BOOST_LOG_TRIVIAL(info)<< "Input type: DEFECTS";
            }
            else if(inputType == "PSI6"){
                this->datatype = PSI6;
                BOOST_LOG_TRIVIAL(info)<< "Input type: PSI6";
            }
            else if(inputType == "DTRJ"){
                this->datatype = DTRJ;
                BOOST_LOG_TRIVIAL(info)<< "Input type: DEFECTS-TRAJECTORIES";
            }
            else if(inputType == "VCELLS"){
                this->datatype = VCELLS;
                BOOST_LOG_TRIVIAL(info)<< "Input type: VORONOI-CELLS";
            }

        }
        else {
            BOOST_LOG_TRIVIAL(error) << "No file type specified";
            exit(EXIT_FAILURE);
        }
        if (opt_map.count("frames") != 0) {
            std::string frames = opt_map["frames"].as<std::string>();
            const std::regex frameCountPattern("^\\s*([0-9]+):([0-9]+)\\s*$");
            std::smatch matches;
            if (std::regex_search(frames, matches, frameCountPattern)) {
                try {
                    this->startFrame = boost::lexical_cast<int>(matches[1]);
                    this->endFrame = boost::lexical_cast<int>(matches[2]);
                }
                catch(boost::bad_lexical_cast& e) {
                    BOOST_LOG_TRIVIAL(error) << e.what();
                    exit(EXIT_FAILURE);
                }
                BOOST_LOG_TRIVIAL(info)<< "Dumping frames " << this->startFrame << " to " << this->endFrame;
            }
            else {
                BOOST_LOG_TRIVIAL(error) << "Wrong syntax for framecount option";
                exit(EXIT_FAILURE);
            }
        }
        else {
            this->startFrame = 0;
            this->endFrame = std::numeric_limits<uint64_t>::max();
            BOOST_LOG_TRIVIAL(info) << "Dumping all frames";
        }
    }
}

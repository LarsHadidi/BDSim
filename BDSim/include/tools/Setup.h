#pragma once

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <exception>
#include <iostream>
#include <fstream>
/// \brief Class to read and set up the initial configuration.
/// \author Lars Hadidi
class Setup {
    public:
        /// \brief Read the run-file which yields the initial system setup. JSON-Format
        /// \param[in] Path to the run-file.
        /// \return Pointer to property tree holding all initial configuration data.
        static boost::property_tree::iptree* readRunFile(std::string runfile) {

            boost::property_tree::iptree* tree = new boost::property_tree::iptree();
            try {
                boost::property_tree::read_json(runfile, *tree);



                double timeStep = 0;
                double simulationTime = 0;
                double relaxationTime = 0;


                try {
                    timeStep = tree->get<double>("timestep");
                }
                catch (boost::property_tree::ptree_bad_path& e) {
                    BOOST_LOG_TRIVIAL(error) << "Time step not set.";
                    exit(1);
                }
                catch(boost::property_tree::ptree_bad_data& e){
                    BOOST_LOG_TRIVIAL(error) << "Bad format in time step.";
                    exit(1);
                }

                try {
                    simulationTime = tree->get<double>("simtime");
                }
                catch (boost::property_tree::ptree_bad_path& e){
                    BOOST_LOG_TRIVIAL(error) << "Simulation time not set.";
                    exit(1);
                }
                catch(boost::property_tree::ptree_bad_data& e){
                    BOOST_LOG_TRIVIAL(error) << "Bad format in simulation time.";
                    exit(1);
                }

                try {
                    relaxationTime = tree->get<double>("relaxation");
                }
                catch (boost::property_tree::ptree_bad_path& e){
                    BOOST_LOG_TRIVIAL(info) << "No relaxation";
                }
                catch(boost::property_tree::ptree_bad_data& e){
                    BOOST_LOG_TRIVIAL(error) << "Bad format in relaxation.";
                }

                if(timeStep <= 0){
                    BOOST_LOG_TRIVIAL(error) << "Timestep zero or less";
                    exit(1);
                }
                if(relaxationTime < 0){
                    BOOST_LOG_TRIVIAL(error) << "Relaxation time negative";
                    exit(1);
                }
                if(simulationTime <= 0){
                    BOOST_LOG_TRIVIAL(error) << "Simulation time zero or less";
                    exit(1);
                }
                if(timeStep > simulationTime){
                    BOOST_LOG_TRIVIAL(error) << "Timestep greater than simulation time";
                    exit(1);
                }
                if(relaxationTime != 0 && timeStep > relaxationTime){
                    BOOST_LOG_TRIVIAL(error) << "Timestep greater than relaxation time.";
                    exit(1);
                }

                BOOST_LOG_TRIVIAL(info) << "Timestep set to: " << timeStep;
                BOOST_LOG_TRIVIAL(info) << "Relaxation time set to: " << relaxationTime;
                BOOST_LOG_TRIVIAL(info) << "Simulation time set to: " << simulationTime;

                try {
                    BOOST_LOG_TRIVIAL(info) << "Temperature set to " << tree->get<double>("temperature");
                }
                catch (boost::property_tree::ptree_bad_path& e){
                    BOOST_LOG_TRIVIAL(error) << "Temperature not set.";
                    exit(1);
                }
                catch(boost::property_tree::ptree_bad_data& e){
                    BOOST_LOG_TRIVIAL(error) << "Bad format in temperature.";
                    exit(1);
                }
                try {
                    BOOST_LOG_TRIVIAL(info) << "Output path set to \'" << tree->get<std::string>("outputpath") << "\'";
                }
                catch (boost::property_tree::ptree_bad_path& e){
                    BOOST_LOG_TRIVIAL(info) << "Output path not set via runfile.";
                }

                std::set<std::string> sensorNames = {"ENERGY","PRESSURE","CONFS","VIZ","DEFECTS","DEFECT-DENSITIES","PSI6", "GLOBAL-PSI6", "VORO-CELLS"};

                if (boost::optional<boost::property_tree::iptree&> node = tree->get_child_optional("sensors")) {
                    BOOST_FOREACH(boost::property_tree::iptree::value_type& v, *node) {
                                    boost::to_upper(v.second.data());
                                    if (sensorNames.find(v.second.data()) == sensorNames.end()){
                                        BOOST_LOG_TRIVIAL(info) << "Unknown sensor: " << v.second.data();
                                    }
                                }
                } else {
                    BOOST_LOG_TRIVIAL(info) << "No sensors.";
                }

                BOOST_LOG_TRIVIAL(info) << "Initstate:";
                std::set<std::string> tesselationNames = {"QUAD", "HEXA"};

                if (boost::optional<boost::property_tree::iptree&> node = tree->get_child_optional("initstate.tesselation")) {
                    boost::to_upper(node->data());
                    if (tesselationNames.find(node->data()) == tesselationNames.end()){
                        BOOST_LOG_TRIVIAL(info) << "Unknown tesselation: " << node->data();
                        exit(1);
                    }
                    std::string tesselationType = node->data();
                    BOOST_LOG_TRIVIAL(info) << "\tTesselation set to: " << tesselationType;
                    if (boost::optional<boost::property_tree::iptree&> node = tree->get_child_optional("initstate.tesselation-parameters")) {

                        if(tesselationType.compare("QUAD") == 0){
                            std::set<std::string> distributionNames = {"UNIFORM", "SCATTER"};

                            if (node = node->get_child_optional("particle-distribution")) {
                                boost::to_upper(node->data());
                                if (tesselationNames.find(node->data()) == tesselationNames.end()){
                                    BOOST_LOG_TRIVIAL(info) << "Unknown distribution: " << node->data();
                                    exit(1);
                                }
                                BOOST_LOG_TRIVIAL(info) << "\tParticle distribution set to " << node->data();
                            }
                            else {
                                BOOST_LOG_TRIVIAL(error) << "\tParticle distribution not set.";
                                exit(1);
                            }
                            try {
                                uint32_t particles =  node->get<uint32_t>("particles");
                                if (particles > 0){
                                    BOOST_LOG_TRIVIAL(info) << "Particles amount: " << particles;
                                }
                                else {
                                    BOOST_LOG_TRIVIAL(error) << "Particle amount is zero.";
                                    exit(1);
                                }
                            }
                            catch (boost::property_tree::ptree_bad_path& e){
                                BOOST_LOG_TRIVIAL(error) << "Number of particles not set.";
                                exit(1);
                            }
                            catch (boost::property_tree::ptree_bad_data& e){
                                BOOST_LOG_TRIVIAL(error) << "Bad format in number of particles.";
                                exit(1);
                            }

                            try {
                                double x = node->get<double>("dimx");
                                if(x > 0){
                                    BOOST_LOG_TRIVIAL(info) << "Length in x dimension set to " << x;
                                }
                                else {
                                    BOOST_LOG_TRIVIAL(error) << "x-Length negative or zero.";
                                    exit(1);
                                }

                            }
                            catch (boost::property_tree::ptree_bad_path& e){
                                BOOST_LOG_TRIVIAL(error) << "Length in x dimension not set.";
                                exit(1);
                            }
                            catch (boost::property_tree::ptree_bad_data& e){
                                BOOST_LOG_TRIVIAL(error) << "Bad format in x length.";
                                exit(1);
                            }

                            try {
                                double y = node->get<double>("dimy");
                                if (y > 0) {
                                    BOOST_LOG_TRIVIAL(info) << "Length in y dimension set to " << y;
                                }
                                else {
                                    BOOST_LOG_TRIVIAL(error) << "y-Length negative or zero.";
                                    exit(1);
                                }
                            }
                            catch (boost::property_tree::ptree_bad_path& e){
                                BOOST_LOG_TRIVIAL(error) << "Length in y dimension not set.";
                                exit(1);
                            }
                            catch (boost::property_tree::ptree_bad_data& e){
                                BOOST_LOG_TRIVIAL(error) << "Bad format in y length.";
                                exit(1);
                            }
                        }
                        else if(tesselationType.compare("HEXA") == 0) {
                            uint16_t rows = 0;
                            uint16_t rowSites = 0;
                            uint32_t defects = 0;

                            try{
                                defects = node->get<uint32_t>("holes");
                                BOOST_LOG_TRIVIAL(info) << "\tInitial holes: "  << defects;
                            }
                            catch (boost::property_tree::ptree_bad_path& e){
                                BOOST_LOG_TRIVIAL(info) << "No initial defects.";
                                node->put("defects", 0);
                            }
                            catch (boost::property_tree::ptree_bad_data& e){
                                BOOST_LOG_TRIVIAL(error) << "Bad format in defects.";
                                exit(1);
                            }

                            try {
                                rows = node->get<uint16_t>("rows");
                                BOOST_LOG_TRIVIAL(info) << "\tRows: "  << rows;
                            }
                            catch (boost::property_tree::ptree_bad_path& e){
                                BOOST_LOG_TRIVIAL(info) << "\tNo rows set.";
                                exit(1);
                            }
                            catch (boost::property_tree::ptree_bad_data& e){
                                BOOST_LOG_TRIVIAL(error) << "Bad format in rows.";
                                exit(1);
                            }

                            try {
                                rowSites = node->get<uint16_t>("row-sites");
                                BOOST_LOG_TRIVIAL(info) << "\tParticles per row: "  << rowSites;
                            }
                            catch (boost::property_tree::ptree_bad_path& e){
                                BOOST_LOG_TRIVIAL(info) << "\tNo row-sites set.";
                                exit(1);
                            }
                            catch (boost::property_tree::ptree_bad_data& e){
                                BOOST_LOG_TRIVIAL(error) << "Bad format in row-sites.";
                                exit(1);
                            }


                            tree->put("particles", rows * rowSites - defects);

                            try {
                                double density = node->get<double>("density");
                                if(density > 0){
                                    BOOST_LOG_TRIVIAL(info) << "\tDensity: " << density;
                                }
                                else {
                                    BOOST_LOG_TRIVIAL(error) << "Density zero or less";
                                    exit(1);
                                }
                            }
                            catch (boost::property_tree::ptree_bad_path& e){
                                BOOST_LOG_TRIVIAL(info) << "\tNo density set.";
                                exit(1);
                            }
                            catch (boost::property_tree::ptree_bad_data& e){
                                BOOST_LOG_TRIVIAL(error) << "Bad format in density.";
                                exit(1);
                            }
                        }

                    } else {
                        BOOST_LOG_TRIVIAL(error) << "\tNo tesselation paramters set.";
                        exit(1);
                    }
                } else {
                    BOOST_LOG_TRIVIAL(error) << "\tNo initstate tesselation set.";
                    exit(1);
                }
            }
            catch (boost::property_tree::json_parser_error &e) {
                BOOST_LOG_TRIVIAL(error) << e.what();
            }
            catch (std::exception &e) {
                BOOST_LOG_TRIVIAL(error) << e.what();
            }

            return tree;
        }
};

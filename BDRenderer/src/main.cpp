// Standard
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>

// Renderers
#include "ConfigurationsRenderer.h"
#include "DefectsRenderer.h"
#include "VisualizationRenderer.h"
#include "PSI6Renderer.h"
#include "DefectsTrajectoriesRenderer.h"
#include "EffectiveDiameter.h"
#include "VoroCellsRenderer.h"

// Boost lib
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

enum DATATYPE {CONFIGURATIONS, VISUALIZATION, DEFECTS, PSI6_RE, PSI6_IM,
               DEFECTS_TRAJECTORIES, VORONOI_CELLS_TOPO, VORONOI_CELLS_PSI6_IM, VORONOI_CELLS_PSI6_RE};

typedef struct {
    uint32_t W,H,N,N_THR;
    double X, Y, TEMPERATURE;
    bool serial;
} Parameters;

void init(int argc, char** argv, boost::program_options::variables_map* opt_map)
{
    boost::program_options::options_description opt_description("Allowed options");
    opt_description.add_options()
            ("help,help", "produce help message")
            ("type,t", boost::program_options::value<std::string>(),"input type: CONFS, VIZ, DEFECTS, PSI6-RE, PSI6-IM, DTRJ, VCELLS-TOPO, VCELLS-PSI6-RE, VCELLS-PSI6-IM")
            ("input,i", boost::program_options::value<std::string>(),"input folder")
            ("output,o",boost::program_options::value<std::string>(),"output file")
            ("width,W",boost::program_options::value<uint32_t >(),"video width")
            ("height,H",boost::program_options::value<uint32_t >(),"video height")
            ("threads,N",boost::program_options::value<uint32_t >(),"Number of threads")
            ("serial,s",boost::program_options::bool_switch()->default_value(false),"Serial processing for large files")
            ;

    try {
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, opt_description), *opt_map);
        boost::program_options::notify(*opt_map);
    } catch (uint32_t ID) {
        BOOST_LOG_TRIVIAL(info)<< "Failed to parse command line options.\n";
    }
    if (opt_map->count("help") != 0) {
        std::cout << opt_description << "\n";
        exit(0);
    }
}

void parseParams(boost::program_options::variables_map opt_map, std::string& inputFile, std::string& outputFile, DATATYPE& datatype, Parameters& parameters)
{
    std::string inputFolder;
    boost::property_tree::ptree paramTree;

    if (opt_map.count("type") != 0) {
        std::string inputType = opt_map["type"].as<std::string>();
        std::transform(inputType.begin(), inputType.end(), inputType.begin(), ::toupper);
        if(inputType == "CONFS") {
            datatype = CONFIGURATIONS;
            BOOST_LOG_TRIVIAL(info)<< "Input type: CONFIGURATIONS";
        }
        else if(inputType == "VIZ"){
            datatype = VISUALIZATION;
            BOOST_LOG_TRIVIAL(info)<< "Input type: VISUALIZATION";
        }
        else if(inputType == "DEFECTS") {
            datatype = DEFECTS;
            BOOST_LOG_TRIVIAL(info) << "Input type: DEFECTS";
        }
        else if(inputType == "PSI6-RE"){
            datatype = PSI6_RE;
            BOOST_LOG_TRIVIAL(info)<< "Input type: PSI6-RE";
        }
        else if(inputType == "PSI6-IM"){
            datatype = PSI6_IM;
            BOOST_LOG_TRIVIAL(info)<< "Input type: PSI6-IM";
        }
        else if(inputType == "DTRJ") {
            datatype = DEFECTS_TRAJECTORIES;
            BOOST_LOG_TRIVIAL(info) << "Input type: DEFECTS-TRAJECTORIES";
        }
        else if(inputType == "VCELLS-TOPO") {
            datatype = VORONOI_CELLS_TOPO;
            BOOST_LOG_TRIVIAL(info) << "Input type: VORONOI-CELLS-TOPO";
        }
        else if(inputType == "VCELLS-PSI6-RE") {
            datatype = VORONOI_CELLS_PSI6_RE;
            BOOST_LOG_TRIVIAL(info) << "Input type: VORONOI-CELLS-PSI6-RE";
        }
        else if(inputType == "VCELLS-PSI6-IM") {
            datatype = VORONOI_CELLS_PSI6_IM;
            BOOST_LOG_TRIVIAL(info) << "Input type: VORONOI-CELLS-PSI6-IM";
        }
        else {
            BOOST_LOG_TRIVIAL(error) << "Unknown input type.";
            exit(EXIT_FAILURE);
        }

    } else {
        BOOST_LOG_TRIVIAL(error) << "No input type specified.";
        exit(EXIT_FAILURE);
    }
    if (opt_map.count("input") != 0) {
        inputFolder = opt_map["input"].as<std::string>();
        switch (datatype) {
            case CONFIGURATIONS:
                inputFile = (boost::filesystem::path(inputFolder)/"CONFS").string();
                break;
            case VISUALIZATION:
                inputFile = (boost::filesystem::path(inputFolder)/"VIZ").string();
                break;
            case DEFECTS:
                inputFile = (boost::filesystem::path(inputFolder)/"DEFECTS-CONFIGURATIONS").string();
                break;
            case PSI6_RE: case PSI6_IM:
                inputFile = (boost::filesystem::path(inputFolder)/"PSI6").string();
                break;
            case DEFECTS_TRAJECTORIES:
                inputFile = std::string(inputFolder);
                break;
            case VORONOI_CELLS_TOPO: case VORONOI_CELLS_PSI6_RE: case VORONOI_CELLS_PSI6_IM:
                inputFile = (boost::filesystem::path(inputFolder)/"VORO-CELLS").string();
                break;
        }
        BOOST_LOG_TRIVIAL(info)<< "Input: " << inputFile;
    } else {
        BOOST_LOG_TRIVIAL(error) << "No input folder.";
        exit(EXIT_FAILURE);
    }
    if (opt_map.count("output") != 0) {
        outputFile = opt_map["output"].as<std::string>();
        BOOST_LOG_TRIVIAL(info)<< "Output file: " << outputFile;
    } else {
        BOOST_LOG_TRIVIAL(error) << "No output file.";
        exit(EXIT_FAILURE);
    }

    if (opt_map.count("width") != 0) {
        parameters.W = opt_map["width"].as<uint32_t>();
        BOOST_LOG_TRIVIAL(info)<< "Video width: " << parameters.W;
    } else {
        BOOST_LOG_TRIVIAL(error) << "Video width missing.";
        exit(EXIT_FAILURE);
    }
    if (opt_map.count("height") != 0) {
        parameters.H = opt_map["height"].as<uint32_t>();
        BOOST_LOG_TRIVIAL(info)<< "Video height: " << parameters.H;
    } else {
        BOOST_LOG_TRIVIAL(error) << "Video height missing.";
        exit(EXIT_FAILURE);
    }
    if (opt_map.count("threads") != 0) {
        parameters.N_THR = opt_map["threads"].as<uint32_t>();
        BOOST_LOG_TRIVIAL(info)<< "Number of threads: " << parameters.N_THR;
    } else {
        parameters.N_THR  = std::thread::hardware_concurrency();
        BOOST_LOG_TRIVIAL(info)<< "Number of threads: " << parameters.N_THR;
    }
    if (opt_map.count("serial") != 0) {
        parameters.serial = opt_map["serial"].as<bool>();
        if (parameters.serial) BOOST_LOG_TRIVIAL(info) << "Serial processing";
    }

    // -----------------------------------------------------------------------------------------------------------------

    try {
        boost::property_tree::ini_parser::read_ini((boost::filesystem::path(inputFolder)/"params").string(), paramTree);
        parameters.N = paramTree.get<uint32_t>("Parameters.particles");
        parameters.X = paramTree.get<double>("Parameters.dimx");
        parameters.Y = paramTree.get<double>("Parameters.dimy");
        parameters.TEMPERATURE = paramTree.get<double>("Parameters.temperature");
        BOOST_LOG_TRIVIAL(info) << "Simulation box width: " << parameters.X;
        BOOST_LOG_TRIVIAL(info) << "Simulation box height: " << parameters.Y;
        BOOST_LOG_TRIVIAL(info) << "Simulation box particle count: " << parameters.N;
    }
    catch(boost::property_tree::ini_parser_error& e){
        BOOST_LOG_TRIVIAL(error) << e.what();
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[]){
    boost::program_options::variables_map opt_map;
    std::string inputFile;
    std::string outputFile;
    DATATYPE datatype;
    Parameters parameters;

    init(argc,argv,&opt_map);
    parseParams(opt_map, inputFile, outputFile, datatype, parameters);

    EffectiveDiameter effectiveDiameter;
    const double EFFECTIVE_RADIUS = effectiveDiameter.estimate(parameters.TEMPERATURE) / 2.0;

    switch(datatype){
        case CONFIGURATIONS:
            if(parameters.serial) ConfigurationsRenderer::processSerially();
            ConfigurationsRenderer::setThreadAmount(parameters.N_THR);
            ConfigurationsRenderer::setOutputFile(outputFile);
            ConfigurationsRenderer::setGridDimensions(parameters.X, parameters.Y);
            ConfigurationsRenderer::setVideoDimensions(parameters.W, parameters.H);
            ConfigurationsRenderer::setParticleAmount(parameters.N);
            ConfigurationsRenderer::setParticleRadius(EFFECTIVE_RADIUS);
            ConfigurationsRenderer::render(inputFile);
            break;
        case VISUALIZATION:
            if(parameters.serial) VisualizationRenderer::processSerially();
            VisualizationRenderer::setThreadAmount(parameters.N_THR);
            VisualizationRenderer::setOutputFile(outputFile);
            VisualizationRenderer::setGridDimensions(parameters.X, parameters.Y);
            VisualizationRenderer::setVideoDimensions(parameters.W, parameters.H);
            VisualizationRenderer::setParticleAmount(parameters.N);
            VisualizationRenderer::setParticleRadius(EFFECTIVE_RADIUS);
            VisualizationRenderer::render(inputFile);
            break;
        case DEFECTS:
            DefectsRenderer::setThreadAmount(parameters.N_THR);
            DefectsRenderer::setOutputFile(outputFile);
            DefectsRenderer::setGridDimensions(parameters.X, parameters.Y);
            DefectsRenderer::setVideoDimensions(parameters.W, parameters.H);
            DefectsRenderer::setParticleRadius(EFFECTIVE_RADIUS);
            DefectsRenderer::render(inputFile);
            break;
        case PSI6_RE: case PSI6_IM:
            if(datatype == PSI6_RE) {
                PSI6Renderer::setComponent(PSI6Renderer::REAL);
            }
            else if (datatype == PSI6_IM) {
                PSI6Renderer::setComponent(PSI6Renderer::IMAGINARY);
            }
            if(parameters.serial) PSI6Renderer::processSerially();
            PSI6Renderer::setThreadAmount(parameters.N_THR);
            PSI6Renderer::setOutputFile(outputFile);
            PSI6Renderer::setGridDimensions(parameters.X, parameters.Y);
            PSI6Renderer::setVideoDimensions(parameters.W, parameters.H);
            PSI6Renderer::setParticleAmount(parameters.N);
            PSI6Renderer::setParticleRadius(EFFECTIVE_RADIUS);
            PSI6Renderer::render(inputFile);
            break;
        case DEFECTS_TRAJECTORIES:
            DefectsTrajectoriesRenderer::setThreadAmount(1);
            DefectsTrajectoriesRenderer::setOutputFile(outputFile);
            DefectsTrajectoriesRenderer::setGridDimensions(parameters.X, parameters.Y);
            DefectsTrajectoriesRenderer::setVideoDimensions(parameters.W, parameters.H);
            DefectsTrajectoriesRenderer::setParticleAmount(parameters.N);
            DefectsTrajectoriesRenderer::setParticleRadius(EFFECTIVE_RADIUS);
            DefectsTrajectoriesRenderer::render(inputFile);
            break;
        case VORONOI_CELLS_TOPO: case VORONOI_CELLS_PSI6_RE: case VORONOI_CELLS_PSI6_IM:
            if(datatype == VORONOI_CELLS_TOPO) {
                VoroCellsRenderer::setComponent(VoroCellsRenderer::TOPOCHARGE);
            }
            else if (datatype == VORONOI_CELLS_PSI6_RE) {
                VoroCellsRenderer::setComponent(VoroCellsRenderer::PSI6REAL);
            }
            else if (datatype == VORONOI_CELLS_PSI6_IM) {
                VoroCellsRenderer::setComponent(VoroCellsRenderer::PSI6IMAGINARY);
            }
            if(parameters.serial) VoroCellsRenderer::processSerially();
            VoroCellsRenderer::setThreadAmount(parameters.N_THR);
            VoroCellsRenderer::setOutputFile(outputFile);
            VoroCellsRenderer::setGridDimensions(parameters.X, parameters.Y);
            VoroCellsRenderer::setVideoDimensions(parameters.W, parameters.H);
            VoroCellsRenderer::setParticleAmount(parameters.N);
            VoroCellsRenderer::setParticleRadius(EFFECTIVE_RADIUS);
            VoroCellsRenderer::render(inputFile);
            break;
    }
}
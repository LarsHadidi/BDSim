#include <iostream>
#include <fstream>
#include <thread>

#include <tools/Logging.h>
#include <tools/Options.h>
#include <matcher/Matcher.h>
#include <deserializers/DefectsReader.h>
#include <transcoders/TopoDeinterlacer.h>

#include <boost/lexical_cast.hpp>

int main(int argc, char* argv[]) {
    try {
        Logging::initLogger();

    } catch (std::exception & e) {
        std::cout << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    BOOST_LOG_TRIVIAL(info) << "DTracker started at " << boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time());
    BOOST_LOG_TRIVIAL(info) << "Number of cores: " << std::thread::hardware_concurrency();

    #if BOOST_OS_WINDOWS
        BOOST_LOG_TRIVIAL(info) << "Operating system: Windows";
    #elif BOOST_OS_LINUX
        BOOST_LOG_TRIVIAL(info) << "Operating system: Linux";
    #elif BOOST_OS_MACOS
        BOOST_LOG_TRIVIAL(info) << "Operating system: MacOS";
    #elif BOOST_OS_BSD
        BOOST_LOG_TRIVIAL(info) << "Operating system: BSD";
    #endif


    boost::program_options::variables_map opt_map;
    boost::program_options::options_description opt_description("Allowed options");

    opt_description.add_options()
            ("input,i"		,boost::program_options::value<std::string>()	,"path to input folder")
            ("output,o"		,boost::program_options::value<std::string>()	,"output path")
            ("help,h", "produce help message")
            ;

    try {
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, opt_description), opt_map);
        boost::program_options::notify(opt_map);
    }
    catch(std::exception & e){
        BOOST_LOG_TRIVIAL(error) << e.what();
    }

    if(opt_map.count("help") != 0) {
        std::cout << opt_description << std::endl;
        exit(EXIT_SUCCESS);
    }

    tools::Options options;
    options.parseOptMap(opt_map);
    options.parseParamsFile((boost::filesystem::path(options.inputFolder)/"params").string());
    options.parseSettingsFile((boost::filesystem::path("conf")/"settings.ini"));

    if(!boost::filesystem::exists(options.outputPath)){
        boost::filesystem::create_directory(options.outputPath);
    }
    boost::filesystem::copy_file((boost::filesystem::path(options.inputFolder)/"params"), options.outputPath);

    boost::filesystem::path inputFile = boost::filesystem::path(options.inputFolder / "DEFECTS-CONFIGURATIONS");
    deserializers::DefectsReader* defectsReader = new deserializers::DefectsReader(inputFile);

    BOOST_LOG_TRIVIAL(info) << "Scanning file to find topological charge boundaries";
    std::pair<int64_t, int64_t> bounds = defectsReader->findTopoChargeBoundaries();
    BOOST_LOG_TRIVIAL(info) << "Min charge: " << bounds.first;
    BOOST_LOG_TRIVIAL(info) << "Max charge: " << bounds.second;
    delete defectsReader;
    transcoders::TopoDeinterlacer* deinterlacer = new transcoders::TopoDeinterlacer(inputFile, options.outputPath, bounds.first, bounds.second);
    BOOST_LOG_TRIVIAL(info) << "Deinterlacing data";
    deinterlacer->deinterlace();
    delete deinterlacer;
    const int64_t N = bounds.second - bounds.first + 1;
    Matcher* matchers[N];
    std::ofstream outStreams[N];
    BOOST_LOG_TRIVIAL(info) << "Opening output streams and starting matchers.";
    std::vector<std::thread> threads;
    for(int i = 0; i < N; ++i) {
        if(i != std::abs(bounds.first)) {
            std::string strOutputFile = (options.outputPath / (std::string("TR_") + boost::lexical_cast<std::string>(i + bounds.first))).string();
            outStreams[i].open(strOutputFile, std::ios::out | std::ios::binary);
            std::string strInputFile = (options.outputPath / (std::string("DC_") + boost::lexical_cast<std::string>(i + bounds.first))).string();
            boost::filesystem::path inFile(strInputFile);
            matchers[i] = new Matcher(inFile, (outStreams + i), options);
        }
    }
    for(size_t i = 0; i < N; ++i) {
        if(i != std::abs(bounds.first)) {
            threads.emplace_back(std::thread(&Matcher::match, matchers[i]));
        }
    }
      for(auto &thread : threads){
          thread.join();
      }

    BOOST_LOG_TRIVIAL(info) << "Tracking finished.";
    for(size_t i = 0; i < N; ++i) {
        if(i != std::abs(bounds.first)) {
            delete matchers[i];
        }
    }
}

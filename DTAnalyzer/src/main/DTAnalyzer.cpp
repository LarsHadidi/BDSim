#include <boost/lexical_cast.hpp>

#include <tools/Logging.h>
#include <tools/Options.h>
#include <thread>
#include <regex>

#include <buffering/Buffering.h>
#include <deserializers/DTRJReader.h>
#include <analyzer/Analyzer.h>

#include <omp.h>

template<typename T>
void printVector(const std::vector<T>& vector){
    #pragma omp critical
    {
        std::cout << "VECTOR " << &vector << "\n";
        for (size_t i = 0; i < vector.size(); ++i) {
            std::cout << vector[i] << "\n";
        }
        std::cout << std::endl;
    }
}

inline void buildBuffersFromFile (boost::filesystem::path& file,
                                  tools::Options* options,
                                  std::vector<buffering::Buffer>** addrPtrBuffers)
{
    buffering::BufferBuilder bufferBuilder(file, boost::filesystem::file_size(file),options->threadCount, options->cacheSize);
    deserializers::DTRJReader trajectoryFileReader(file);

    const std::vector<std::streampos>* const SPLIT_POINTS = trajectoryFileReader.findSplitPoints();
    const std::vector<std::streampos>* const DISPATCH_POINTS = bufferBuilder.dispatchPoints(SPLIT_POINTS);

    *addrPtrBuffers = bufferBuilder.createBuffers(DISPATCH_POINTS);
}

inline void resetBuffers(std::vector<buffering::Buffer>* ptrBuffers) {
    for(buffering::Buffer& buffer : *ptrBuffers) {
        buffer.reset();
    }
}

inline void buildKernelsFromBuffers (std::vector<buffering::Buffer>* ptrBuffers,
                                    tools::Options* options,
                                    std::vector<analyzer::Kernel*>** addrPtrKernels)
{
    analyzer::Kernel::setup(options);
    (*addrPtrKernels) = new std::vector<analyzer::Kernel*>();

    for(size_t i = 0; i < ptrBuffers->size(); ++i) {
        (*addrPtrKernels)->push_back(new analyzer::Kernel(&(ptrBuffers->at(i))));
    }
}

inline void cleanUp (std::vector<buffering::Buffer>** addrPtrBuffers,
                     std::vector<analyzer::Kernel*>** addrPtrKernels)
{
    delete *addrPtrBuffers;
    for(analyzer::Kernel* k : **addrPtrKernels){
        delete k;
    }
    delete *addrPtrKernels;
}

int main(int argc, char* argv[]) {
    try {
        Logging::initLogger();

    } catch (std::exception & e) {
        std::cout << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    BOOST_LOG_TRIVIAL(info) << "DTAnalyzer started at " << boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time());
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
    catch(std::exception & e) {
        BOOST_LOG_TRIVIAL(error) << e.what();
    }

    if(opt_map.count("help") != 0) {
        std::cout << opt_description << std::endl;
        exit(EXIT_SUCCESS);
    }

    tools::Options* options = new tools::Options();
    options->parseOptMap(opt_map);
    options->parseParamsFile((boost::filesystem::path(options->inputFolder)/"params"));
    options->parseSettingsFile((boost::filesystem::path("conf")/"settings.ini"));

    std::vector<boost::filesystem::path> inputFiles;
    std::vector<int> chargeMap;

    static const std::regex fileNamePattern("^TR_(-?[0-9]+)$");
    std::smatch matches;

    boost::filesystem::path inputPath(options->inputFolder);
    for(boost::filesystem::directory_entry& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(inputPath), {})) {
        const std::string filename = entry.path().stem().string();
        if (std::regex_search(filename, matches, fileNamePattern)) {
            BOOST_LOG_TRIVIAL(info) << "Found file containing trajectories for topological charge " << matches[1];
            inputFiles.push_back(entry.path());
            chargeMap.push_back(boost::lexical_cast<int>(matches[1]));
        }
    }

    assert(inputFiles.size() > 0);

    const int MIN_CHARGE = *std::min_element(chargeMap.begin(), chargeMap.end());
    const int MAX_CHARGE = *std::max_element(chargeMap.begin(), chargeMap.end());

    #ifdef _OPENMP
        omp_set_nested(1);
    #endif


    const unsigned long FILE_COUNT = inputFiles.size();
    std::vector<analyzer::Kernel::RMSD_POINTS_MAP*> results(FILE_COUNT);

    #pragma omp parallel for num_threads(FILE_COUNT)
    for(int TID_LVL1 = 0; TID_LVL1 < FILE_COUNT; ++TID_LVL1)
    {
        boost::filesystem::path& file = inputFiles[TID_LVL1];
        std::vector<buffering::Buffer>* ptrBuffers;
        std::vector<analyzer::Kernel*>* ptrKernels;

        buildBuffersFromFile(file, options, &ptrBuffers);



        resetBuffers(ptrBuffers);
        buildKernelsFromBuffers(ptrBuffers, options, &ptrKernels);
        const unsigned long BUFFER_COUNT = ptrBuffers->size();


        BOOST_LOG_TRIVIAL(info) << "Thread " << TID_LVL1 << " starting " << BUFFER_COUNT << " parallel kernels";
        #pragma omp parallel for num_threads(BUFFER_COUNT)
        for(int TID_LVL2 = 0; TID_LVL2 < BUFFER_COUNT; ++TID_LVL2)
        {
            ptrKernels->at(TID_LVL2)->execute();
        }

        #pragma omp critical
        {
            BOOST_LOG_TRIVIAL(info) << "Thread " << TID_LVL1 << " is aggregating results";
            analyzer::Kernel::RMSD_POINTS_MAP* ptrResult = analyzer::Kernel::aggregate(ptrKernels);
            results[TID_LVL1] = ptrResult;
        }

        cleanUp(&ptrBuffers, &ptrKernels);
    }


    BOOST_LOG_TRIVIAL(info) << "Last-level aggregator started";
    analyzer::Aggregator aggregator(&results);
    BOOST_LOG_TRIVIAL(info) << "\tPerforming reduction for RMSD";
    aggregator.writeRootMeanSquareDisplacementDistribution(boost::filesystem::path(options->outputPath / "RMSD"));
    BOOST_LOG_TRIVIAL(info) << "\tPerforming reduction for LTH";
    aggregator.writeLifeTimeHistrgram(boost::filesystem::path(options->outputPath / "LTH"));
    BOOST_LOG_TRIVIAL(info) << "Process finished";
    BOOST_LOG_TRIVIAL(info) << "Free ressources";
    for(analyzer::Kernel::RMSD_POINTS_MAP* ptrResult : results){
        delete ptrResult;
    }
}
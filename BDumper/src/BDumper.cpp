#include <iostream>

#include <tools/Options.h>
#include <dumpers/Dumpers.h>

void init(int argc, char** argv, boost::program_options::variables_map* opt_map)
{
    boost::program_options::options_description opt_description("Allowed options");
    opt_description.add_options()
            ("help,help", "produce help message")
            ("type,t", boost::program_options::value<std::string>(),"file type to dump: CONFS, VIZ, DEFECTS, PSI6, DTRJ, VCELLS")
            ("input,i", boost::program_options::value<std::string>(),"input folder")
            ("output,o",boost::program_options::value<std::string>(),"output file/folder")
            ("frames,n",boost::program_options::value<std::string>(), "frames to dump: format is START:END")
           ;
    try {
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, opt_description), *opt_map);
        boost::program_options::notify(*opt_map);
    } catch (uint32_t ID) {
        BOOST_LOG_TRIVIAL(info)<< "Failed to parse command line options.\n";
    }
    if (opt_map->count("help") != 0) {
        std::cout << opt_description << "\n";
        exit(EXIT_SUCCESS);
    }
}


int main(int argc, char* argv[]) {
    boost::program_options::variables_map opt_map;
    init(argc, argv, &opt_map);

    tools::Options options;
    options.parseOptMap(opt_map);
    options.parseParamsFile((boost::filesystem::path(options.inputFolder)/"params").string());

    dumpers::Dumper* dumper;
    switch(options.datatype) {
        case tools::Options::CONFS:
            dumper = new dumpers::CONFSDumper(options);
            break;
        case tools::Options::VIZ:
            dumper = new dumpers::VIZDumper(options);
            break;
        case tools::Options::DEFECTS:
            dumper = new dumpers::DEFECTSDumper(options);
            break;
        case tools::Options::PSI6:
            dumper = new dumpers::PSI6Dumper(options);
            break;
        case tools::Options::DTRJ:
            dumper = new dumpers::DTRJDumper(options);
            break;
        case tools::Options::VCELLS:
            dumper = new dumpers::VCELLSDumper(options);
            break;
    }
    BOOST_LOG_TRIVIAL(info) << "Starting to dump data into " << options.outputPath.string();
    dumper->dump();
    BOOST_LOG_TRIVIAL(info) << "Done.";
    delete dumper;

    return EXIT_SUCCESS;
}
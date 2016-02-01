#include <blackBoxFactories/SamplerFactory.h>


namespace blackBoxFactories {

    boost::property_tree::iptree            SamplerFactory::sensorConf;
    boost::property_tree::iptree*           SamplerFactory::initConf            = nullptr;
    datastructures::linkedParticleList**    SamplerFactory::particlesAddress    = nullptr;
    datastructures::CellList*               SamplerFactory::cells               = nullptr;
    interfaces::ForceField*                 SamplerFactory::forceField          = nullptr;
    VoronoiDecomposer::VoronoiDecomposer*   SamplerFactory::voronoiDecomposer   = nullptr;

    void SamplerFactory::initialize(boost::property_tree::iptree *initConf,
                                    datastructures::linkedParticleList **const particlesAddress,
                                    datastructures::CellList *const cells,
                                    interfaces::ForceField *const forceField,
                                    VoronoiDecomposer::VoronoiDecomposer*const voronoiDecomposer)
    {
        SamplerFactory::initConf            = initConf;
        SamplerFactory::particlesAddress    = particlesAddress;
        SamplerFactory::cells               = cells;
        SamplerFactory::forceField          = forceField;
        SamplerFactory::voronoiDecomposer   = voronoiDecomposer;
        try {
            boost::property_tree::ini_parser::read_ini((boost::filesystem::path("conf") / "sensors.ini").string(), SamplerFactory::sensorConf);
        }
        catch (std::exception & e) {
            BOOST_LOG_TRIVIAL(error) << e.what();
        }
    }



    interfaces::Sampler* SamplerFactory::createPressureSampler() {
        BOOST_LOG_TRIVIAL(info) << "\tInitializing PRESSURE-Sampler";
        double samplingrate = 0;
        double datapoints = 1;
        try {
            BOOST_LOG_TRIVIAL(info) << "\t\tReading sensors.ini";
            samplingrate = SamplerFactory::sensorConf.get<double>("PRESSURE.samplingrate");
            datapoints = SamplerFactory::sensorConf.get<double>("PRESSURE.datapoints");
            BOOST_LOG_TRIVIAL(info) << "\t\tSamplingrate: " << samplingrate;
            BOOST_LOG_TRIVIAL(info) << "\t\tDatapoints: " << datapoints;
        }
        catch(const boost::property_tree::ptree_error &e) {
            BOOST_LOG_TRIVIAL(error) << e.what();
        }
        double simulationSteps = SamplerFactory::initConf->get<double>("simtime") / SamplerFactory::initConf->get<double>("timestep");
        double totalSamplesCount = samplingrate * simulationSteps;
        double smpCountPerDatapoint = totalSamplesCount / datapoints;

        samplers::PressureSampler *sampler = new samplers::PressureSampler(SamplerFactory::initConf,
                                                                           SamplerFactory::forceField,
                                                                           samplingrate,
                                                                           smpCountPerDatapoint);

        BOOST_LOG_TRIVIAL(info) << "\tPRESSURE-Sampler initialized.";
        return sampler;
    }

    interfaces::Sampler* SamplerFactory::createConfigurationsSampler() {
        BOOST_LOG_TRIVIAL(info) << "\tInitializing CONFS-Sampler";
        double samplingrate = 0;
        double buffersize = 1;
        try {
            BOOST_LOG_TRIVIAL(info) << "\t\tReading sensors.ini";
            samplingrate = SamplerFactory::sensorConf.get<double>("CONFS.samplingrate");
            buffersize = SamplerFactory::sensorConf.get<uint32_t>("CONFS.buffersize");
            BOOST_LOG_TRIVIAL(info) << "\t\tSamplingrate: " << samplingrate;
            BOOST_LOG_TRIVIAL(info) << "\t\tBuffersize: " << buffersize;
        }
        catch(const boost::property_tree::ptree_error &e) {
            BOOST_LOG_TRIVIAL(error) << e.what();
        }

        samplers::ConfigurationsSampler *sampler = new samplers::ConfigurationsSampler(SamplerFactory::initConf,
                                                                                       SamplerFactory::particlesAddress,
                                                                                       SamplerFactory::forceField,
                                                                                       samplingrate,
                                                                                       buffersize);

        BOOST_LOG_TRIVIAL(info) << "\tCONFS-Sampler initialized.";
        return sampler;
    }

    interfaces::Sampler* SamplerFactory::createEnergySampler() {
        BOOST_LOG_TRIVIAL(info) << "\tInitializing ENERGY-Sampler";
        double samplingrate = 0;
        double datapoints = 1;
        try {
            BOOST_LOG_TRIVIAL(info) << "\t\tReading sensors.ini";
            samplingrate = SamplerFactory::sensorConf.get<double>("ENERGY.samplingrate");
            datapoints = SamplerFactory::sensorConf.get<double>("ENERGY.datapoints");
            BOOST_LOG_TRIVIAL(info) << "\t\tSamplingrate: " << samplingrate;
            BOOST_LOG_TRIVIAL(info) << "\t\tDatapoints: " << datapoints;
        }
        catch(const boost::property_tree::ptree_error &e) {
            BOOST_LOG_TRIVIAL(error) << e.what();
        }
        double simulationSteps = SamplerFactory::initConf->get<double>("simtime") / SamplerFactory::initConf->get<double>("timestep");
        double totalSamplesCount = samplingrate * simulationSteps;
        double smpCountPerDatapoint = totalSamplesCount / datapoints;

         samplers::EnergySampler *sampler = new samplers::EnergySampler  (SamplerFactory::initConf,
                                                                         SamplerFactory::forceField,
                                                                         samplingrate,
                                                                         smpCountPerDatapoint);

        BOOST_LOG_TRIVIAL(info) << "\tENERGY-Sampler initialized.";
        return sampler;
    }

    interfaces::Sampler* SamplerFactory::createVisualizationSampler() {
        BOOST_LOG_TRIVIAL(info) << "\tInitializing VIZ-Sampler";
        double buffersize = 1;
        try {
            BOOST_LOG_TRIVIAL(info) << "\t\tReading sensors.ini";
            buffersize = SamplerFactory::sensorConf.get<uint32_t>("VIZ.buffersize");
            BOOST_LOG_TRIVIAL(info) << "\t\tBuffersize: " << buffersize;
        }
        catch(const boost::property_tree::ptree_error &e) {
            BOOST_LOG_TRIVIAL(error) << e.what();
        }

        samplers::VisualizationSampler *sampler = new samplers::VisualizationSampler  (SamplerFactory::initConf,
                                                                                       SamplerFactory::cells,
                                                                                       SamplerFactory::forceField,
                                                                                       buffersize);

        BOOST_LOG_TRIVIAL(info) << "\tVIZ-Sampler initialized.";
        return sampler;
    }

    interfaces::Sampler* SamplerFactory::createDefectDensitiesSampler() {
        BOOST_LOG_TRIVIAL(info) << "\tInitializing DEFECT-DENSITIES-Sampler";

        double samplingrate = 0;
        double datapoints = 1;
        try {
            BOOST_LOG_TRIVIAL(info) << "\t\tReading sensors.ini";
            samplingrate = SamplerFactory::sensorConf.get<double>("DEFECT-DENSITIES.samplingrate");
            datapoints = SamplerFactory::sensorConf.get<double>("DEFECT-DENSITIES.datapoints");
            BOOST_LOG_TRIVIAL(info) << "\t\tSamplingrate: " << samplingrate;
            BOOST_LOG_TRIVIAL(info) << "\t\tDatapoints: " << datapoints;
        }
        catch(const boost::property_tree::ptree_error &e) {
            BOOST_LOG_TRIVIAL(error) << e.what();
        }
        double simulationSteps = SamplerFactory::initConf->get<double>("simtime") / SamplerFactory::initConf->get<double>("timestep");
        double totalSamplesCount = samplingrate * simulationSteps;
        double smpCountPerDatapoint = totalSamplesCount / datapoints;

        samplers::DefectDensitiesSamplers* sampler = new samplers::DefectDensitiesSamplers(SamplerFactory::initConf,
                                                                                           SamplerFactory::particlesAddress,
                                                                                           SamplerFactory::voronoiDecomposer,
                                                                                           samplingrate,
                                                                                           smpCountPerDatapoint);
        BOOST_LOG_TRIVIAL(info) << "\tDEFECT-DENSITIES-Sampler initialized.";
        return sampler;
    }


    interfaces::Sampler* SamplerFactory::createDefectsSampler() {
        BOOST_LOG_TRIVIAL(info) << "\tInitializing DEFECTS-Sampler";

        double samplingrate = 0;
        uint32_t buffersize = 1;
        try {
            BOOST_LOG_TRIVIAL(info) << "\t\tReading sensors.ini";
            samplingrate = SamplerFactory::sensorConf.get<double>("DEFECTS.samplingrate");
            buffersize = SamplerFactory::sensorConf.get<uint32_t>("DEFECTS.buffersize");

            BOOST_LOG_TRIVIAL(info) << "\t\tSamplingrate: " << samplingrate;
            BOOST_LOG_TRIVIAL(info) << "\t\tBuffersize: " << buffersize;
        }
        catch(const boost::property_tree::ptree_error &e) {
            BOOST_LOG_TRIVIAL(error) << e.what();
        }

        samplers::DefectsSampler *sampler = new samplers::DefectsSampler(SamplerFactory::initConf,
                                                                         SamplerFactory::particlesAddress,
                                                                         SamplerFactory::voronoiDecomposer,
                                                                         samplingrate,
                                                                         buffersize);
        BOOST_LOG_TRIVIAL(info) << "\tDEFECTS-Sampler initialized.";
        return sampler;
    }

    interfaces::Sampler* SamplerFactory::createPSI6Sampler(){
        BOOST_LOG_TRIVIAL(info) << "\tInitializing PSI6-Sampler";

        double samplingrate = 0;
        uint32_t buffersize = 1;
        try {
            BOOST_LOG_TRIVIAL(info) << "\t\tReading sensors.ini";
            samplingrate = SamplerFactory::sensorConf.get<double>("PSI6.samplingrate");
            buffersize = SamplerFactory::sensorConf.get<uint32_t>("PSI6.buffersize");

            BOOST_LOG_TRIVIAL(info) << "\t\tSamplingrate: " << samplingrate;
            BOOST_LOG_TRIVIAL(info) << "\t\tBuffersize: " << buffersize;
        }
        catch(const boost::property_tree::ptree_error &e) {
            BOOST_LOG_TRIVIAL(error) << e.what();
        }

        samplers::PSI6Sampler *sampler = new samplers::PSI6Sampler  (SamplerFactory::initConf,
                                                                     SamplerFactory::particlesAddress,
                                                                     SamplerFactory::voronoiDecomposer,
                                                                     samplingrate,
                                                                     buffersize);
        BOOST_LOG_TRIVIAL(info) << "\tPSI6-Sampler initialized.";
        return sampler;
    }

    interfaces::Sampler* SamplerFactory::createGlobalPSI6Sampler() {
        BOOST_LOG_TRIVIAL(info) << "\tInitializing GLOBAL-PSI6-Sampler";
        double samplingrate = 0;
        double datapoints = 1;
        try {
            BOOST_LOG_TRIVIAL(info) << "\t\tReading sensors.ini";
            samplingrate = SamplerFactory::sensorConf.get<double>("GLOBAL-PSI6.samplingrate");
            datapoints = SamplerFactory::sensorConf.get<double>("GLOBAL-PSI6.datapoints");
            BOOST_LOG_TRIVIAL(info) << "\t\tSamplingrate: " << samplingrate;
            BOOST_LOG_TRIVIAL(info) << "\t\tDatapoints: " << datapoints;
        }
        catch(const boost::property_tree::ptree_error &e) {
            BOOST_LOG_TRIVIAL(error) << e.what();
        }
        double simulationSteps = SamplerFactory::initConf->get<double>("simtime") / SamplerFactory::initConf->get<double>("timestep");
        double totalSamplesCount = samplingrate * simulationSteps;
        double smpCountPerDatapoint = totalSamplesCount / datapoints;

        samplers::GlobalPSI6Sampler *sampler = new samplers::GlobalPSI6Sampler  (SamplerFactory::initConf,
                                                                                 SamplerFactory::particlesAddress,
                                                                                 SamplerFactory::voronoiDecomposer,
                                                                                 samplingrate,
                                                                                 smpCountPerDatapoint);

        BOOST_LOG_TRIVIAL(info) << "\tGLOBAL-PSI6-Sampler initialized.";
        return sampler;
    }

    interfaces::Sampler* SamplerFactory::createVoroCellsSampler() {
        BOOST_LOG_TRIVIAL(info) << "\tInitializing VORO-CELLS-Sampler";

        double samplingrate = 0;
        uint32_t buffersize = 1;
        try {
            BOOST_LOG_TRIVIAL(info) << "\t\tReading sensors.ini";
            samplingrate = SamplerFactory::sensorConf.get<double>("VORO-CELLS.samplingrate");
            buffersize = SamplerFactory::sensorConf.get<uint32_t>("VORO-CELLS.buffersize");

            BOOST_LOG_TRIVIAL(info) << "\t\tSamplingrate: " << samplingrate;
            BOOST_LOG_TRIVIAL(info) << "\t\tBuffersize: " << buffersize;
        }
        catch(const boost::property_tree::ptree_error &e) {
            BOOST_LOG_TRIVIAL(error) << e.what();
        }

        samplers::VoroCellsSampler *sampler = new samplers::VoroCellsSampler    (SamplerFactory::initConf,
                                                                                SamplerFactory::particlesAddress,
                                                                                SamplerFactory::voronoiDecomposer,
                                                                                samplingrate,
                                                                                buffersize);
        BOOST_LOG_TRIVIAL(info) << "\tVORO-CELLS-Sampler initialized.";
        return sampler;
    }
}

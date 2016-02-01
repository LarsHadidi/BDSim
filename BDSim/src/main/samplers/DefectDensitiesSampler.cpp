#include <samplers/DefectDensitiesSampler.h>

namespace samplers {
    DefectDensitiesSamplers::DefectDensitiesSamplers(boost::property_tree::iptree* initConf, datastructures::linkedParticleList**const particlesAddress, VoronoiDecomposer::VoronoiDecomposer*const voronoiDecomposer, double samplingrate, double  smpCountPerDatapoint) :
            PTR_PTR_PARTICLES(particlesAddress),
            VORONOI_DECOMPOSER(voronoiDecomposer),
            CALLS_PER_SAMPLE(1 / samplingrate),
            PARTICLE_COUNT(initConf->get<uint32_t>("particles")),
            SAMPLES_PER_DATAPOINT(smpCountPerDatapoint)
    {
        this->smpCalls = 0;
        this->samples = 0;
        this->cumulativeDefectDensity = 0;
        this->outputPath = boost::filesystem::path(initConf->get<std::string>("outputpath"));
        this->outStream.open((this->outputPath / "DEFECT-DENSITIES").string(), std::ios::out);
    }


    DefectDensitiesSamplers::~DefectDensitiesSamplers() {
        this->outStream.flush();
        this->outStream.close();
    }

    void DefectDensitiesSamplers::reset() {
        BOOST_LOG_TRIVIAL(info) << "\tClosing output stream of DEFECT-DENSITIES-Sampler.";
        this->outStream.close();
        boost::filesystem::remove((this->outputPath / "DEFECT-DENSITIES").string());
        this->outStream.open((this->outputPath / "DEFECT-DENSITIES").string(), std::ios::out);
    }
    void DefectDensitiesSamplers::sample() {
        ++this->smpCalls;
        if(this->smpCalls >= this->CALLS_PER_SAMPLE){
            VoronoiDecomposer::Decomposition* voroCells = this->VORONOI_DECOMPOSER->update();
            uint32_t defects = 0;
            for(VoronoiDecomposer::vCell& vCell : *voroCells) {
                int topoCharge2D = vCell.cell.p - TOPO_CHARGE_BASE;
                if (topoCharge2D != 0) {
                    ++defects;
                }
            }
            this->cumulativeDefectDensity += (double)defects / this->PARTICLE_COUNT;

            this->smpCalls = 0;
            ++this->samples;
        }
        if(this->samples >= this->SAMPLES_PER_DATAPOINT) {
            double D = this->cumulativeDefectDensity / this->SAMPLES_PER_DATAPOINT;

            this->outStream << std::setprecision(DIGITS) << D << std::endl;

            this->samples = 0;
            this->cumulativeDefectDensity = 0;
        }
    }
}

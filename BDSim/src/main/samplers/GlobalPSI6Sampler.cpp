#include <samplers/GlobalPSI6Sampler.h>

namespace samplers {
    GlobalPSI6Sampler::GlobalPSI6Sampler(boost::property_tree::iptree* initConf, datastructures::linkedParticleList**const particlesAddress, VoronoiDecomposer::VoronoiDecomposer*const voronoiDecomposer, double samplingrate, double  smpCountPerDatapoint) :
            PTR_PTR_PARTICLES(particlesAddress),
            VORONOI_DECOMPOSER(voronoiDecomposer),
            CALLS_PER_SAMPLE(1 / samplingrate),
            SAMPLES_PER_DATAPOINT(smpCountPerDatapoint),
            DIM_X(initConf->get<double>("dimx")),
            DIM_Y(initConf->get<double>("dimy")),
            PARTICLE_COUNT(initConf->get<uint32_t>("particles"))

    {
        this->smpCalls = 0;
        this->samples = 0;
        this->cumulativeAbsoluteValue = 0;
        this->globalRE = 0;
        this->globalIM = 0;
        this->neighbourIDs.reserve(16);
        this->outputPath = boost::filesystem::path(initConf->get<std::string>("outputpath"));
        this->outStream.open((this->outputPath / "PSI6-GLOBAL").string(), std::ios::out);
    }


    GlobalPSI6Sampler::~GlobalPSI6Sampler() {
        this->outStream.flush();
        this->outStream.close();
    }

    void GlobalPSI6Sampler::reset() {
        BOOST_LOG_TRIVIAL(info) << "\tClosing output stream of GLOBAL-PSI6-Sampler.";
        this->outStream.close();
        boost::filesystem::remove((this->outputPath / "PSI6-GLOBAL").string());
        this->outStream.open((this->outputPath / "PSI6-GLOBAL").string(), std::ios::out);
    }

    void GlobalPSI6Sampler::sample(){
         ++this->smpCalls;
         if(this->smpCalls >= this->CALLS_PER_SAMPLE){
             VoronoiDecomposer::Decomposition* voroCells = this->VORONOI_DECOMPOSER->update();

             for(VoronoiDecomposer::vCell& vCell : *voroCells) {
                 vCell.cell.neighbors(this->neighbourIDs);
                 assert(this->neighbourIDs.size() > 0);
                 double RE = 0;
                 double IM = 0;
                 for(int neighbourID : this->neighbourIDs){
                     const double X = (*(this->PTR_PTR_PARTICLES))->data[neighbourID].value.posX;
                     const double Y = (*(this->PTR_PTR_PARTICLES))->data[neighbourID].value.posY;
                     const double dX = X - vCell.x;
                     const double dY = Y - vCell.y;
                     // Minimum Image Convention
                     double dX_NPI = 0;
                     double dY_NPI = 0;
                     const double dXabs = fabs(dX);
                     const double dYabs = fabs(dY);

                     if(dXabs > DIM_X / 2.0) {
                         const double d = DIM_X - dXabs;
                         dX_NPI = (dX > 0) ? - d : + d;
                     }
                     else {
                         dX_NPI = dX;
                     }
                     if (dYabs > DIM_Y / 2.0) {
                         const double d = DIM_Y - dYabs;
                         dY_NPI += (dY > 0) ? - d : + d;
                     }
                     else {
                         dY_NPI = dY;
                     }
                     // PSI-6
                     const double R = std::sqrt(dX_NPI * dX_NPI + dY_NPI * dY_NPI);
                     assert(R != 0);
                     const double COS = dX_NPI / R;
                     const double SIN = dY_NPI / R;
                     const double COS_SQUARED = COS * COS;
                     const double SIN_SQUARED = SIN * SIN;

                     RE += SIN_SQUARED * COS_SQUARED * (15 * SIN_SQUARED - 15 * COS_SQUARED) + COS_SQUARED * COS_SQUARED * COS_SQUARED - SIN_SQUARED * SIN_SQUARED * SIN_SQUARED;
                     IM += SIN * COS * (6 * COS_SQUARED * COS_SQUARED - 20 * SIN_SQUARED * COS_SQUARED + 6 * SIN_SQUARED * SIN_SQUARED);
                 }
                 this->globalRE += RE / this->neighbourIDs.size();
                 this->globalIM += IM / this->neighbourIDs.size();
             }
             // normalization will be done outside the loop
             this->cumulativeAbsoluteValue += std::sqrt(this->globalRE * this->globalRE + this->globalIM * this->globalIM);

             this->globalRE = 0;
             this->globalIM = 0;

             this->smpCalls = 0;
             ++this->samples;
         }
         if(this->samples >= this->SAMPLES_PER_DATAPOINT) {
             const double GLOBAL_PSI6 = this->cumulativeAbsoluteValue / (this->SAMPLES_PER_DATAPOINT * this->PARTICLE_COUNT);

             this->outStream << std::setprecision(DIGITS) << GLOBAL_PSI6 << std::endl;

             this->samples = 0;
             this->globalRE = 0;
             this->globalIM = 0;
             this->cumulativeAbsoluteValue = 0;
         }
    }
}

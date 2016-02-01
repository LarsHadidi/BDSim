#include <samplers/PSI6Sampler.h>

namespace samplers {
    PSI6Sampler::PSI6Sampler(boost::property_tree::iptree* initConf, datastructures::linkedParticleList**const particlesAddress, VoronoiDecomposer::VoronoiDecomposer*const voronoiDecomposer, double samplingrate, uint32_t szBuffer) :
            PTR_PTR_PARTICLES(particlesAddress),
            VORONOI_DECOMPOSER(voronoiDecomposer),
            CALLS_PER_SAMPLE(1 / samplingrate),
            BUFFER_SIZE(szBuffer),
            buffer(new PSI6Point*[BUFFER_SIZE]),
            PARTICLE_COUNT(initConf->get<uint32_t>("particles")),
            DIM_X(initConf->get<double>("dimx")),
            DIM_Y(initConf->get<double>("dimy"))

    {
        for(uint32_t i = 0; i < this->BUFFER_SIZE; ++i) {
            PSI6Point* pData = new PSI6Point[this->PARTICLE_COUNT];
            this->buffer[i] = pData;
        }

        this->bufferPosition = 0;
        this->smpCalls = 0;
        this->neighbourIDs.reserve(16);
        this->outputPath = boost::filesystem::path(initConf->get<std::string>("outputpath"));
        this->outStream.open((this->outputPath / "PSI6").string(), std::ios::out | std::ios::binary);
    }


    PSI6Sampler::~PSI6Sampler() {
        BOOST_LOG_TRIVIAL(info) << "\tClosing output stream of PSI6-Sampler.";
        if(this->bufferPosition > 0){
            BOOST_LOG_TRIVIAL(info) << "\t\tFlushing remaining data of PSI6-Sampler.";
            this->writeBufferUntilPos();
        }
        this->outStream.flush();
        this->outStream.close();
        for(uint32_t i = 1; i < this->BUFFER_SIZE; ++i) {
            delete this->buffer[i];
        }
        delete this->buffer;
    }

    void PSI6Sampler::reset() {
        for(uint32_t i = 1; i < this->BUFFER_SIZE; ++i) {
            delete this->buffer[i];
        }
        for(uint32_t i = 0; i < this->BUFFER_SIZE; ++i) {
            PSI6Point* pData = new PSI6Point[this->PARTICLE_COUNT];
            this->buffer[i] = pData;
        }
        this->bufferPosition = 0;
        this->smpCalls = 0;
        this->outStream.close();
        boost::filesystem::remove((this->outputPath / "PSI6").string());
        this->outStream.open((this->outputPath / "PSI6").string(), std::ios::out);
    }

    void PSI6Sampler::sample(){
        ++this->smpCalls;
        if(this->smpCalls >= this->CALLS_PER_SAMPLE){
            VoronoiDecomposer::Decomposition* voroCells = this->VORONOI_DECOMPOSER->update();

            uint32_t i = 0;
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
                assert(RE / this->neighbourIDs.size() <= 1);
                assert(IM / this->neighbourIDs.size() <= 1);
                this->buffer[this->bufferPosition][i].position.x(vCell.x);
                this->buffer[this->bufferPosition][i].position.y(vCell.y);
                this->buffer[this->bufferPosition][i].orderParameter.re = (RE / this->neighbourIDs.size());
                this->buffer[this->bufferPosition][i].orderParameter.im = (IM / this->neighbourIDs.size());
                ++i;
            }

            this->smpCalls = 0;
            this->bufferPosition++;
            if (this->bufferPosition == this->BUFFER_SIZE) {
                this->writeBufferUntilPos();
                this->bufferPosition = 0;
            }
        }
    }

    void PSI6Sampler::writeBufferUntilPos() {
        for(uint32_t pos = 0; pos < this->bufferPosition; ++pos){
            this->outStream.write((char*)this->buffer[pos], this->PARTICLE_COUNT * sizeof(PSI6Point));
        }
    }
}
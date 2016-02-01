#include <samplers/DefectsSampler.h>

namespace samplers {
    DefectsSampler::DefectsSampler	(boost::property_tree::iptree* initConf, datastructures::linkedParticleList**const particlesAddress, VoronoiDecomposer::VoronoiDecomposer*const voronoiDecomposer, double samplingrate, uint32_t szBuffer) :
        PTR_PTR_PARTICLES(particlesAddress),
        VORONOI_DECOMPOSER(voronoiDecomposer),
        CALLS_PER_SAMPLE(1 / samplingrate),
        BUFFER_SIZE(szBuffer),
        buffer(new Defect*[BUFFER_SIZE]),
        PARTICLE_COUNT(initConf->get<uint32_t>("particles"))
    {
        this->bufferedDataSize = (uint32_t*)calloc(this->BUFFER_SIZE, sizeof(uint32_t));
        for(uint32_t i = 0; i < this->BUFFER_SIZE; ++i) {
            Defect* pData = (Defect*)calloc(this->PARTICLE_COUNT, sizeof(Defect));
            this->buffer[i] = pData;
        }

        this->bufferPosition = 0;
        this->smpCalls = 0;

        this->outputPath = boost::filesystem::path(initConf->get<std::string>("outputpath"));
        this->outStream.open((this->outputPath / "DEFECTS-CONFIGURATIONS").string(), std::ios::out | std::ios::binary);
    }


    DefectsSampler::~DefectsSampler(){
            BOOST_LOG_TRIVIAL(info) << "\tClosing output stream of DEFECTS-Sampler.";
            if(this->bufferPosition > 0){
                BOOST_LOG_TRIVIAL(info) << "\t\tFlushing remaining data of DEFECTS-Sampler.";
                this->writeBufferUntilPos();
            }

        this->outStream.flush();
        this->outStream.close();

        for(uint32_t i = 1; i < this->BUFFER_SIZE; ++i) {
            free(this->buffer[i]);
        }
        free(this->buffer);
        free(this->bufferedDataSize);

    }

    void DefectsSampler::reset(){
        delete this->bufferedDataSize;
        this->bufferedDataSize = new uint32_t[BUFFER_SIZE]();

        for(uint32_t i = 1; i < this->BUFFER_SIZE; ++i) {
            free(this->buffer[i]);
        }
        for(uint32_t i = 0; i < this->BUFFER_SIZE; ++i) {
            Defect* pData = (Defect*)calloc(this->PARTICLE_COUNT, sizeof(Defect));
            this->buffer[i] = pData;
        }
        this->bufferPosition = 0;
        this->smpCalls = 0;


        this->outStream.close();
        boost::filesystem::remove((this->outputPath / "DEFECTS-CONFIGURATIONS").string());
        this->outStream.open((this->outputPath / "DEFECTS-CONFIGURATIONS").string(), std::ios::out | std::ios::binary);

    }

    void DefectsSampler::sample() {
        this->smpCalls++;

        if (this->smpCalls >= this->CALLS_PER_SAMPLE) {
            uint32_t defectsWritten = 0;

            VoronoiDecomposer::Decomposition* voroCells = this->VORONOI_DECOMPOSER->update();

            for(VoronoiDecomposer::vCell& vCell : *voroCells) {
                // voro::voronoicell_base::p holds the total number of vertices in the current cell
                int64_t topoCharge2D = vCell.cell.p - TOPO_CHARGE_BASE;
                if (topoCharge2D != 0) {

                    this->buffer[this->bufferPosition][defectsWritten].x = vCell.x;
                    this->buffer[this->bufferPosition][defectsWritten].y = vCell.y;
                    this->buffer[this->bufferPosition][defectsWritten].topoCharge = topoCharge2D;

                    ++defectsWritten;
                }
            }

            this->smpCalls = 0;
            this->bufferedDataSize[this->bufferPosition] = defectsWritten;
            this->bufferPosition++;

            if (this->bufferPosition == this->BUFFER_SIZE) {
                this->writeBufferUntilPos();
                this->bufferPosition = 0;
            }
        }
    }


    /// Writes current state to numeric output file and flushes buffer.
    void DefectsSampler::writeBufferUntilPos(){
        for(uint32_t pos = 0; pos < this->bufferPosition; ++pos){
            this->outStream.write((char*)&this->bufferedDataSize[pos], sizeof(uint32_t));
            this->outStream.write((char*)this->buffer[pos], this->bufferedDataSize[pos] * sizeof(Defect));
        }
    }
}

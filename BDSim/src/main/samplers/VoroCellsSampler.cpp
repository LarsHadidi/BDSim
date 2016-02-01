#include <samplers/VoroCellsSampler.h>
#include <VoronoiDecomposer/VoronoiDecomposer.h>

namespace samplers {

    VoroCellsSampler::VoroCellsSampler(boost::property_tree::iptree* initConf, datastructures::linkedParticleList**const particlesAddress, VoronoiDecomposer::VoronoiDecomposer*const voronoiDecomposer, double samplingrate, uint32_t szBuffer) :
            PTR_PTR_PARTICLES(particlesAddress),
            VORONOI_DECOMPOSER(voronoiDecomposer),
            CALLS_PER_SAMPLE(1 / samplingrate),
            BUFFER_SIZE(szBuffer),
            buffer(new double*[BUFFER_SIZE]),
            vertexCount(new uint32_t*[BUFFER_SIZE]),
            topoCharges(new int64_t*[BUFFER_SIZE]),
            psi6Re(new double*[BUFFER_SIZE]),
            psi6Im(new double*[BUFFER_SIZE]),
            PARTICLE_COUNT(initConf->get<uint32_t>("particles")),
            DIM_X(initConf->get<double>("dimx")),
            DIM_Y(initConf->get<double>("dimy"))
        {
            for(uint32_t i = 0; i < this->BUFFER_SIZE; ++i) {
                double* polygons = (double*)calloc(this->PARTICLE_COUNT * MAX_POLYGON_VERTICES, VERTEX_SIZE);
                uint32_t* vertexCounters = (uint32_t*)calloc(this->PARTICLE_COUNT, sizeof(uint32_t));
                int64_t* topoChargeCounters = (int64_t*)calloc(this->PARTICLE_COUNT, sizeof(int64_t));
                double* psi6ReCounters = (double*)calloc(this->PARTICLE_COUNT, sizeof(double));
                double* psi6ImCounters = (double*)calloc(this->PARTICLE_COUNT, sizeof(double));
                this->buffer[i] = polygons;
                this->vertexCount[i] = vertexCounters;
                this->topoCharges[i] = topoChargeCounters;
                this->psi6Re[i] = psi6ReCounters;
                this->psi6Im[i] = psi6ImCounters;
            }

            this->polygon.reserve(2 * MAX_POLYGON_VERTICES);
            this->neighbourIDs.reserve(16);

            this->bufferPosition = 0;
            this->smpCalls = 0;

            this->outputPath = boost::filesystem::path(initConf->get<std::string>("outputpath"));
            this->outStream.open((this->outputPath / "VORO-CELLS").string(), std::ios::out | std::ios::binary);
        }

    VoroCellsSampler::~VoroCellsSampler(){
        BOOST_LOG_TRIVIAL(info) << "\tClosing output stream of VORO-CELLS-Sampler.";
        if(this->bufferPosition > 0){
            BOOST_LOG_TRIVIAL(info) << "\t\tFlushing remaining data of VORO-CELLS-Sampler.";
            this->writeBufferUntilPos();
        }

        this->outStream.flush();
        this->outStream.close();

        for(uint32_t i = 1; i < this->BUFFER_SIZE; ++i) {
            free(this->buffer[i]);
            free(this->vertexCount[i]);
            free(this->topoCharges[i]);
            free(this->psi6Re[i]);
            free(this->psi6Im[i]);
        }
        delete(this->buffer);
        delete(this->vertexCount);
        delete(this->topoCharges);
        delete(this->psi6Re);
        delete(this->psi6Im);
    }

    void VoroCellsSampler::reset(){
        for(uint32_t i = 1; i < this->BUFFER_SIZE; ++i) {
            free(this->buffer[i]);
            free(this->vertexCount[i]);
            free(this->topoCharges[i]);
            free(this->psi6Re[i]);
            free(this->psi6Im[i]);
        }
        for(uint32_t i = 0; i < this->BUFFER_SIZE; ++i) {
            double* polygons = (double*)calloc(this->PARTICLE_COUNT * MAX_POLYGON_VERTICES, VERTEX_SIZE);
            uint32_t* vertexCounters = (uint32_t*)calloc(this->PARTICLE_COUNT, sizeof(uint32_t));
            int64_t* topoChargeCounters = (int64_t*)calloc(this->PARTICLE_COUNT, sizeof(int64_t));
            double* psi6ReCounters = (double*)calloc(this->PARTICLE_COUNT, sizeof(double));
            double* psi6ImCounters = (double*)calloc(this->PARTICLE_COUNT, sizeof(double));
            this->buffer[i] = polygons;
            this->vertexCount[i] = vertexCounters;
            this->topoCharges[i] = topoChargeCounters;
            this->psi6Re[i] = psi6ReCounters;
            this->psi6Im[i] = psi6ImCounters;
        }

        this->polygon.resize(0);

        this->bufferPosition = 0;
        this->smpCalls = 0;


        this->outStream.close();
        boost::filesystem::remove((this->outputPath / "VORO-CELLS").string());
        this->outStream.open((this->outputPath / "VORO-CELLS").string(), std::ios::out | std::ios::binary);

    }

    void VoroCellsSampler::sample() {
        ++this->smpCalls;
        if(this->smpCalls >= this->CALLS_PER_SAMPLE){
            VoronoiDecomposer::Decomposition* voroCells = this->VORONOI_DECOMPOSER->update();

            assert(voroCells->size() == this->PARTICLE_COUNT);
            for(size_t i = 0; i < voroCells->size(); ++i) {
                // This loop could be bootstrapped, calculating the BASE_ADDRESS using bufferedDataSize[i - 1]
                // but a parallelization of that loop will need a barrier at each iteration
                // this loop will have the drawback of memory waste and fragmented final copy on writeBuffer

                // write vertices of the voronoi cell polygon into vector this->polygon
                voroCells->at(i).cell.vertices_in_order(voroCells->at(i).x, voroCells->at(i).y, this->polygon);
                // keep track of the amount of vertices for each polygon, the output of cell_2d::vertices returns
                // a vector containing the linearized representation of a vector containing two-dimensional vectors
                assert((this->polygon.size() / 2) < 32);
                this->vertexCount[this->bufferPosition][i] = (this->polygon.size() / 2);


                // Calculate PSI6 order parameter and topological charge
                this->topoCharges[this->bufferPosition][i] = voroCells->at(i).cell.p - TOPO_CHARGE_BASE;
                this->calculatePSI6(this->psi6Re[this->bufferPosition][i], this->psi6Im[this->bufferPosition][i], voroCells->at(i));

                // copy vertex data into buffer
                double* const POLYGON_BASE_ADDRESS = (this->buffer[this->bufferPosition] + i * (MAX_POLYGON_VERTICES * 2));
                memcpy(POLYGON_BASE_ADDRESS, this->polygon.data(), this->polygon.size() * sizeof(double));
            }

            this->smpCalls = 0;
            this->bufferPosition++;
            if (this->bufferPosition == this->BUFFER_SIZE) {
                this->writeBufferUntilPos();
                this->bufferPosition = 0;
            }
        }
    }

    void VoroCellsSampler::writeBufferUntilPos(){
        for(uint32_t pos = 0; pos < this->bufferPosition; ++pos){
            for(size_t i = 0; i < this->PARTICLE_COUNT; ++i){
                assert(this->vertexCount[pos][i] < 32);
                this->outStream.write((char*)(this->vertexCount[pos] + i), sizeof(uint32_t));
                this->outStream.write((char*)(this->topoCharges[pos] + i), sizeof(int64_t));
                this->outStream.write((char*)(this->psi6Re[pos] + i), sizeof(double));
                this->outStream.write((char*)(this->psi6Im[pos] + i), sizeof(double));
                this->outStream.write((char*)(this->buffer[pos] + i * (MAX_POLYGON_VERTICES * 2)), this->vertexCount[pos][i] * VERTEX_SIZE);
            }
        }
    }

    void VoroCellsSampler::calculatePSI6(double& RE, double& IM, VoronoiDecomposer::vCell& vCell){
        vCell.cell.neighbors(this->neighbourIDs);
        assert(this->neighbourIDs.size() > 0);
        RE = 0;
        IM = 0;
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
        RE /= this->neighbourIDs.size();
        IM /= this->neighbourIDs.size();
        assert(RE <= 1);
        assert(IM <= 1);
    }
}

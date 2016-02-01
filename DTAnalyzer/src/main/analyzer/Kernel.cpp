#include <analyzer/Kernel.h>

namespace analyzer {
    Kernel::RMSD_POINTS_MAP* Kernel::ptrAggregatedMap = nullptr;
    double Kernel::DIM_X;
    double Kernel::DIM_Y;
    double Kernel::SIM_TIMESTEPS;

    void Kernel::setup(tools::Options* options) {
        Kernel::DIM_X = options->dimX;
        Kernel::DIM_Y = options->dimY;
        Kernel::SIM_TIMESTEPS = (options->simTime / options->timeStep);
    }

    Kernel::Kernel(buffering::Buffer* buffer) :
    BUFFER(buffer)
    {
        this->distributionMap = new Kernel::RMSD_POINTS_MAP();
    }

    Kernel::~Kernel() {
        distributionMap->clear();
        delete this->distributionMap;
    }

    double Kernel::squaredDistanceUnderPBC(const double X1, const double Y1, const double X0, const double Y0) {
        const double dX = X1 - X0;
        const double dY = Y1 - Y0;
        const double dXabs = fabs(dX);
        const double dYabs = fabs(dY);

        double dX_NPI = 0;
        double dY_NPI = 0;

        if(dXabs > DIM_X / 2.0) {
            const double d = DIM_X - dXabs;
            dX_NPI = (dX > 0) ? - d : + d;
        }
        else {
            dX_NPI = dX;
        }
        if (dYabs > DIM_Y / 2.0) {
            const double d = DIM_Y - dYabs;
            dY_NPI = (dY > 0) ? - d : + d;
        }
        else {
            dY_NPI = dY;
        }

        return (dX_NPI * dX_NPI + dY_NPI * dY_NPI);
    }

    void Kernel::execute() {
        while(BUFFER->hasBytes()) {
            uint32_t count = *((uint32_t*)(this->BUFFER->getBytes(sizeof(uint32_t))));
            assert(count <= Kernel::SIM_TIMESTEPS && "Valid input data");
            datastructures::TrackPoint *ptrTrackPoints = (datastructures::TrackPoint*)(this->BUFFER->getBytes(count * sizeof(datastructures::TrackPoint)));
            for (size_t i = 0; i < count; ++i) {
                this->processTrackPoint(ptrTrackPoints + i);
            }
        }
    }

    void Kernel::processTrackPoint(datastructures::TrackPoint *ptrCurrentTrackPoint) {
        RMSDPoint& point = (*(this->distributionMap))[ptrCurrentTrackPoint->trackID]; // Using insert & custom constructor introduces additional overhead of std::pair creation

        if (point.displacement->size() == 0) {
            point.initialX = ptrCurrentTrackPoint->x;
            point.initialY = ptrCurrentTrackPoint->y;
            point.x = ptrCurrentTrackPoint->x;
            point.y = ptrCurrentTrackPoint->y;
            point.displacement->push_back(0);
        }
        else {
            const double SQUARED_DISTANCE = this->squaredDistanceUnderPBC(ptrCurrentTrackPoint->x, ptrCurrentTrackPoint->y, point.x, point.y);
            point.displacement->push_back(point.displacement->back() + SQUARED_DISTANCE);

            point.x = ptrCurrentTrackPoint->x;
            point.y = ptrCurrentTrackPoint->y;
        }
    }

    Kernel::RMSD_POINTS_MAP* Kernel::getDistributionMap() {
        return this->distributionMap;
    }

    Kernel::RMSD_POINTS_MAP* Kernel::aggregate(std::vector<Kernel*>* ptrKernelList) {
        Kernel::ptrAggregatedMap = new Kernel::RMSD_POINTS_MAP();
        // Insertion loop
        for (size_t i = 0; i < ptrKernelList->size(); ++i) {
            Kernel::RMSD_POINTS_MAP* ptrKernelDistributionMap = ((Kernel *) (ptrKernelList->at(i)))->getDistributionMap();

            for (Kernel::RMSD_POINTS_MAP::iterator kernelMapIterator = ptrKernelDistributionMap->begin(); kernelMapIterator != ptrKernelDistributionMap->end(); ++kernelMapIterator) {

                RMSDPoint& refKernelMapPoint = kernelMapIterator->second;
                DOUBLE_VECTOR *ptrKernelVector = refKernelMapPoint.displacement;
                DOUBLE_VECTOR::iterator kernelVectorIterator= ptrKernelVector->begin();

                RMSDPoint& refAggrMapPoint = (*(Kernel::ptrAggregatedMap))[kernelMapIterator->first];
                DOUBLE_VECTOR* ptrAggrVector = refAggrMapPoint.displacement;

                double maxRMSD = 0;
                if(!ptrAggrVector->empty()){
                    const double LAST_RMSD = ptrAggrVector->back();
                    const double RMSD_DIFF = squaredDistanceUnderPBC(refKernelMapPoint.initialX, refKernelMapPoint.initialY, refAggrMapPoint.x, refAggrMapPoint.y);
                    maxRMSD = LAST_RMSD + RMSD_DIFF;
                }

                while (kernelVectorIterator != ptrKernelVector->end()) {
                    ptrAggrVector->push_back(*kernelVectorIterator + maxRMSD);
                    ++kernelVectorIterator;
                }
                refAggrMapPoint.x = refKernelMapPoint.x;
                refAggrMapPoint.y = refKernelMapPoint.y;
            }
        }

        return Kernel::ptrAggregatedMap;
    }
}
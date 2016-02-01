#include "VoroCellsRenderer.h"

double      VoroCellsRenderer::R             = 0;
double      VoroCellsRenderer::X             = 0;
double      VoroCellsRenderer::Y             = 0;
uint32_t    VoroCellsRenderer::W             = 0;
uint32_t    VoroCellsRenderer::H             = 0;
uint32_t    VoroCellsRenderer::frameSize     = 0;
uint32_t    VoroCellsRenderer::N             = 0;
uint32_t    VoroCellsRenderer::N_THR         = 1;
size_t      VoroCellsRenderer::SZ_BODY       = 0;
int64_t     VoroCellsRenderer::minTopoCharge = 0;
int64_t     VoroCellsRenderer::maxTopoCharge = 0;
bool        VoroCellsRenderer::serial        = false;
std::string VoroCellsRenderer::outputFile;
const std::streamsize   VoroCellsRenderer::SZ_CELL_HEADER    = sizeof(uint32_t) + sizeof(uint64_t) + sizeof(double) + sizeof(double);
VoroCellsRenderer::COMPONENT VoroCellsRenderer::component;

void VoroCellsRenderer::setThreadAmount(uint32_t N){
    VoroCellsRenderer::N_THR = N;
}
void VoroCellsRenderer::setOutputFile(std::string filename) {
    VoroCellsRenderer::outputFile = filename;
}
void VoroCellsRenderer::setParticleAmount(uint32_t N) {
    VoroCellsRenderer::N = N;
}
void VoroCellsRenderer::setGridDimensions(double X, double Y) {
    VoroCellsRenderer::X = X;
    VoroCellsRenderer::Y = Y;
}
void VoroCellsRenderer::setVideoDimensions(uint32_t W, uint32_t H) {
    VoroCellsRenderer::W = W;
    VoroCellsRenderer::H = H;
}

void VoroCellsRenderer::setParticleRadius(double r) {
    VoroCellsRenderer::R = r;
}

void VoroCellsRenderer::setComponent(COMPONENT component) {
    VoroCellsRenderer::component = component;
}

void VoroCellsRenderer::processSerially() {
    VoroCellsRenderer::serial = true;
}

std::vector<std::streampos>* VoroCellsRenderer::findSplitPoints(std::ifstream& inStream) {
    std::vector<std::streampos>* splitPoints = new std::vector<std::streampos>();
    inStream.seekg(0, std::ios::beg);

    while(inStream.peek() != EOF) {
        splitPoints->push_back(inStream.tellg());
        for(size_t i = 0; i < VoroCellsRenderer::N; ++i) {
            uint32_t vertexCount;
            inStream.read((char*) &vertexCount, sizeof(uint32_t));
            inStream.seekg(VoroCellsRenderer::SZ_CELL_HEADER - sizeof(uint32_t), std::ios::cur);
            std::streamoff polygonSize = vertexCount * (2 * sizeof(double));
            inStream.seekg(polygonSize, std::ios::cur);
        }
    }
    inStream.clear();
    return splitPoints;
}

std::vector<std::streampos>* VoroCellsRenderer::findDispatchPoints(std::vector<std::streampos>& splitPoints) {
    std::vector<std::streampos> *dispatchPoints = new std::vector<std::streampos>();

    uint32_t proposalStep = VoroCellsRenderer::SZ_BODY / N_THR;
    uint32_t basePos = 0;

    while(dispatchPoints->size() < N_THR) {
        uint32_t dispatchPoint = 0;
        bool dispatchPointFound = false;
        uint32_t i = 0;
        while(!dispatchPointFound) {
            uint32_t splitPoint = splitPoints.at(i);
            uint32_t splitPointNeighbourRight = splitPoints.at(i + 1);
            if (splitPoint < basePos && splitPointNeighbourRight > basePos) {
                std::streamsize distanceLeft = abs(splitPoint - basePos);
                std::streamsize distanceRight = abs(splitPointNeighbourRight - basePos);
                dispatchPoint = (distanceLeft < distanceRight) ? splitPoint : splitPointNeighbourRight;
                dispatchPointFound = true;
            }
            else if (splitPoint == basePos) {
                dispatchPoint = basePos;
                dispatchPointFound = true;
            }
            i++;
            if (i == splitPoints.size() - 1 && !dispatchPointFound) {
                uint32_t minDistancePoint = splitPoints.at(0);
                for(uint32_t splitPoint : splitPoints) {
                    uint32_t distance = abs(splitPoint - basePos);
                    uint32_t minDistance = abs(minDistancePoint - basePos);
                    if (distance < minDistance) minDistancePoint = splitPoint;
                }
                dispatchPoint = minDistancePoint;
                dispatchPointFound = true;
            }
        }
        dispatchPoints->push_back(dispatchPoint);
        basePos += proposalStep;
    }
    for (int j = 0; j < dispatchPoints->size(); ++j) {
        assert(std::find(splitPoints.begin(), splitPoints.end(), dispatchPoints->at(j)) != splitPoints.end());
    }
    return dispatchPoints;
}

std::pair<int64_t , int64_t> VoroCellsRenderer::parallelFindTopoChargeBoundaries(int TID, size_t bufferSize, char* buffer) {
    int64_t min = std::numeric_limits<int64_t>::max();
    int64_t max = std::numeric_limits<int64_t>::min();

    size_t pos = 0;
    while (pos < bufferSize) {
        for(size_t i = 0; i < VoroCellsRenderer::N; ++i) {
            uint32_t vertexCount;
            int64_t topoCharge;
            vertexCount = *((uint32_t*)(buffer + pos));
            topoCharge = *((int64*)(buffer + pos + sizeof(uint32_t)));

            if (topoCharge > max) max = topoCharge;
            if (topoCharge < min) min = topoCharge;

            pos += VoroCellsRenderer::SZ_CELL_HEADER;
            pos += vertexCount * (2 * sizeof(double));
        }
    }
    return std::pair<int64_t,int64_t>(min, max);
}

void VoroCellsRenderer::parallelEncode(int TID, size_t bufferSize, char* buffer) {
    std::stringstream filename;
    cv::VideoWriter videoWriter;

    shapes::Colorbar colorbar(0.90 * W, 0.05 * H, 0.02 * W, 0.10 * H, VoroCellsRenderer::maxTopoCharge - VoroCellsRenderer::minTopoCharge, VoroCellsRenderer::minTopoCharge);

    const double C_SCALE = 255.0/(VoroCellsRenderer::maxTopoCharge - VoroCellsRenderer::minTopoCharge);
    const double ISO_SCALER = std::min(W / X, H / Y);

    const double CIRCLE_RADIUS = R * std::min(W / X, H / Y); // isotropic scaling

    filename << VoroCellsRenderer::outputFile;
    filename << "-";
    filename << TID;
    filename << ".avi";

    videoWriter.open(filename.str(), CV_FOURCC('X', 'V', 'I', 'D'), 60, cv::Size(W, H), true);
    if (videoWriter.isOpened()) {
        BOOST_LOG_TRIVIAL(info) << "Thread " << TID << " opened " << filename.str() << " for writing.";
    }
    else {
        BOOST_LOG_TRIVIAL(error) << "Failed to open output video file in thread " << TID;
    }

    cv::Mat BGR(H, W, CV_8UC3);
    cv::Point vertexList[MAX_POLYGON_VERTICES];
    const cv::Point* ptrVertexList[1] = { vertexList };
    size_t pos = 0;
    while (pos < bufferSize) {
        cv::Mat image = cv::Mat::zeros(H, W, CV_8UC3);

        uint32_t vertexCount;
        uint64_t topoCharge;
        double psi6Re;
        double psi6Im;

        for(size_t i = 0; i < VoroCellsRenderer::N; ++i) {
            vertexCount = *((uint32_t*)(buffer + pos));
            pos += sizeof(uint32_t);
            topoCharge = *((uint64_t*)(buffer + pos));
            pos += sizeof(uint64_t);
            psi6Re = *((double*)(buffer + pos));
            pos += sizeof(double);
            psi6Im = *((double*)(buffer + pos));
            pos += sizeof(double);
            double* polygon = (double*)(buffer + pos);


            for(uint64_t j = 0; j < 2 * vertexCount; j += 2){
                double x = polygon[j];
                double y = polygon[j + 1];
                x = x * ISO_SCALER;
                y = H - y * ISO_SCALER;
                assert(j % 2 == 0);
                vertexList[j / 2] = cv::Point(x, y);
            }

            uint8_t color;
            switch(VoroCellsRenderer::component) {
                case TOPOCHARGE:
                    color = (topoCharge - VoroCellsRenderer::minTopoCharge) * C_SCALE;
                    break;
                case PSI6REAL:
                    color = (psi6Re + 1) * C_SCALE;
                    break;
                case PSI6IMAGINARY:
                    color = (psi6Im + 1) * C_SCALE;
                    break;
            }

            cv::fillConvexPoly(image, vertexList, vertexCount, cv::Scalar(color, 255, 255), CV_AA);
            int aryVertexCount[1] = {vertexCount};
            cv::polylines(image, ptrVertexList, aryVertexCount, 1, true, cv::Scalar(0, 0, 255), 1, CV_AA);

            pos += vertexCount * (2 * sizeof(double));
        }

        cv::cvtColor(image, BGR, CV_HSV2BGR);
        videoWriter << BGR;
    }
    free(buffer);
    BOOST_LOG_TRIVIAL(info) << "Thread " << TID << " finished.";
}

void VoroCellsRenderer::render(std::string inputFileName) {
    std::ifstream inStream(inputFileName, std::ios::in | std::ios::binary | std::ios::ate);
    if(!inStream){
        BOOST_LOG_TRIVIAL(error) << "Could not open input file.";
        exit(1);
    }

    VoroCellsRenderer::SZ_BODY = (size_t)inStream.tellg();

    inStream.seekg(0, std::ios::beg);
    BOOST_LOG_TRIVIAL(info) << "Finding frames.";
    std::vector<std::streampos>* splitPoints = findSplitPoints(inStream);
    BOOST_LOG_TRIVIAL(info) << "Found " << splitPoints->size() << " frames.";
    BOOST_LOG_TRIVIAL(info) << "Finding dispatch points for render-units.";
    std::vector<std::streampos>* dispatchPoints = findDispatchPoints(*splitPoints);
    BOOST_LOG_TRIVIAL(info) << "Filling buffers.";
    // Fill the buffers -------------------------------------------------------------------------------------------
    inStream.seekg(0, std::ios::beg);
    char** const BUFFERS = new char*[N_THR];
    size_t* const BUFFER_SIZES = new size_t[N_THR];

    for(int i = 0; i < N_THR - 1; ++i){
        size_t bufferSize = dispatchPoints->at(i + 1) - dispatchPoints->at(i);
        BUFFERS[i] = new char[bufferSize];
        BUFFER_SIZES[i] = bufferSize;
        inStream.read(BUFFERS[i], BUFFER_SIZES[i]);
    }
    size_t bufferSize = VoroCellsRenderer::SZ_BODY - dispatchPoints->at(N_THR - 1);
    BUFFERS[N_THR - 1] = new char[bufferSize];
    BUFFER_SIZES[N_THR - 1] = bufferSize;
    inStream.read(BUFFERS[N_THR - 1],  BUFFER_SIZES[N_THR - 1]);
    inStream.close();
    // -------------------------------------------------------------------------------------------------------------
    BOOST_LOG_TRIVIAL(info) << "Finding minimal and maximal topological charge values.";
    std::vector<std::pair<int64_t,int64_t>> boundaries;
    std::vector<std::future<std::pair<int64_t,int64_t>>> futures;
    for(int i = 0; i < N_THR - 1; ++i){
        futures.push_back(std::async(parallelFindTopoChargeBoundaries, i + 1, BUFFER_SIZES[i], BUFFERS[i]));
    }
    futures.push_back(std::async(parallelFindTopoChargeBoundaries, N_THR, BUFFER_SIZES[N_THR - 1], BUFFERS[N_THR - 1]));

    for(auto &future : futures){
        boundaries.push_back(future.get());
    }

    int64_t min = std::numeric_limits<int64_t>::max();
    int64_t max = std::numeric_limits<int64_t>::min();
    for(std::pair<int64_t,int64_t> b : boundaries){
        if (b.first < min) min  = b.first;
        if (b.second > max) max = b.second;
    }
    VoroCellsRenderer::minTopoCharge = min;
    VoroCellsRenderer::maxTopoCharge = max;
    BOOST_LOG_TRIVIAL(info) << "Found minimal topological charge to be " << min;
    BOOST_LOG_TRIVIAL(info) << "Found maximal topological charge to be " << max;
    BOOST_LOG_TRIVIAL(info) << "Dispatching to render units.";

    std::vector<std::thread> threads;
    for(int i = 0; i < N_THR - 1; ++i){
        threads.emplace_back(std::thread(parallelEncode, i + 1, BUFFER_SIZES[i], BUFFERS[i]));
    }
    threads.emplace_back(std::thread(parallelEncode, N_THR, BUFFER_SIZES[N_THR - 1], BUFFERS[N_THR - 1]));

    for(auto &thread : threads){
        thread.join();
    }

    inStream.close();
    threads.clear();
}

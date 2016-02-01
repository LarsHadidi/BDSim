#include "DefectsRenderer.h"

double      DefectsRenderer::R              = 0;
double      DefectsRenderer::X              = 0;
double      DefectsRenderer::Y              = 0;
uint32_t    DefectsRenderer::W              = 0;
uint32_t    DefectsRenderer::H              = 0;
int64_t     DefectsRenderer::minTopoCharge  = 0;
int64_t     DefectsRenderer::maxTopoCharge  = 0;
size_t      DefectsRenderer::SZ_BODY        = 0;
uint32_t    DefectsRenderer::N_THR          = 1;
std::string DefectsRenderer::outputFile;

void DefectsRenderer::setThreadAmount(uint32_t N){
    DefectsRenderer::N_THR = N;
}
void DefectsRenderer::setOutputFile(std::string filename) {
    DefectsRenderer::outputFile = filename;
}
void DefectsRenderer::setVideoDimensions(uint32_t W, uint32_t H) {
    DefectsRenderer::W = W;
    DefectsRenderer::H = H;
}
void DefectsRenderer::setGridDimensions(double X, double Y) {
    DefectsRenderer::X = X;
    DefectsRenderer::Y = Y;
}

void DefectsRenderer::setParticleRadius(double r) {
    DefectsRenderer::R = r;
}

std::vector<std::streampos>* DefectsRenderer::findSplitPoints(std::ifstream& inStream) {
    std::vector<std::streampos>* splitPoints = new std::vector<std::streampos>();
    inStream.seekg(0, std::ios::beg);

    uint32_t defectCount;
    while(inStream.peek() != EOF) {
        splitPoints->push_back(inStream.tellg());
        inStream.read((char*)&defectCount, sizeof(uint32_t));
        std::streamoff offset = defectCount * sizeof(Defect);
        inStream.seekg(inStream.tellg() + offset);
    }
    inStream.clear();
    return splitPoints;
}

std::vector<std::streampos>* DefectsRenderer::findDispatchPoints(std::vector<std::streampos>& splitPoints) {
    std::vector<std::streampos> *dispatchPoints = new std::vector<std::streampos>();

    uint32_t proposalStep = DefectsRenderer::SZ_BODY / N_THR;
    uint32_t basePos = 0;

    while(dispatchPoints->size() < N_THR) {
        uint32_t dispatchPoint = 0;
        bool dispatchPointFound = false;
        uint32_t i = 0;
        while (!dispatchPointFound) {
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
            if (!dispatchPointFound && i == splitPoints.size() - 1) {
                uint32_t minDistancePoint = splitPoints.at(0);
                for (uint32_t splitPoint : splitPoints) {
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


std::pair<int64_t , int64_t> DefectsRenderer::parallelFindTopoChargeBoundaries(int TID, size_t bufferSize, char* buffer) {
    int64_t min = std::numeric_limits<int64_t>::max();
    int64_t max = std::numeric_limits<int64_t>::min();

    size_t pos = 0;
    while (pos < bufferSize) {
        uint32_t defectCount = 0;
        defectCount = *((uint32_t*)(buffer + pos));
        pos += sizeof(uint32_t);
        for(uint32_t i = 0; i < defectCount; ++i){
            Defect defect;
            defect = *((Defect*)(buffer + pos));
            if (defect.topoCharge > max) max = defect.topoCharge;
            if (defect.topoCharge < min) min = defect.topoCharge;
            pos += sizeof(Defect);
        }
    }
    return std::pair<int64_t,int64_t>(min, max);
}

void DefectsRenderer::parallelEncode(int TID, size_t bufferSize, char* buffer) {
    std::stringstream filename;
    cv::VideoWriter videoWriter;

    shapes::Colorbar colorbar(0.90 * W, 0.05 * H, 0.02 * W, 0.10 * H, DefectsRenderer::maxTopoCharge - DefectsRenderer::minTopoCharge, DefectsRenderer::minTopoCharge);

    const double C_SCALE = 255.0/(DefectsRenderer::maxTopoCharge - DefectsRenderer::minTopoCharge);
    const double ISO_SCALER = std::min(W / X, H / Y);
    const double CIRCLE_RADIUS = R * ISO_SCALER;

    filename << DefectsRenderer::outputFile;
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
    size_t pos = 0;
    while (pos < bufferSize) {
        uint32_t defectCount;
        defectCount = *((uint32_t*)(buffer + pos));
        pos += sizeof(uint32_t);
        cv::Mat image = cv::Mat::zeros(H, W, CV_8UC3);
        for(uint32_t i = 0; i < defectCount; ++i){
            Defect defect;
            defect = *((Defect*)(buffer + pos));
            defect.x = defect.x * ISO_SCALER;
            defect.y = H - defect.y * ISO_SCALER; // flip y-axis to represent system coordinates correctly on screen coordinates
            const uint8_t COLOR = (defect.topoCharge - DefectsRenderer::minTopoCharge) * C_SCALE;

            cv::circle(image, cv::Point(defect.x, defect.y), CIRCLE_RADIUS, cv::Scalar(COLOR, 255, 255), CV_FILLED, CV_AA, 0);
            //cv::putText(image, std::to_string(defect.topoCharge), cv::Point(defect.x - 2.5, defect.y + 2.5), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.5, cv::Scalar(0,0,255));
            pos += sizeof(Defect);
        }
        colorbar.render(image);
        cv::cvtColor(image, BGR, CV_HSV2BGR);
        videoWriter << BGR;
    }
    free(buffer);
    BOOST_LOG_TRIVIAL(info) << "Thread " << TID << " finished.";
}

void DefectsRenderer::render(std::string inputFileName) {
    std::ifstream inStream(inputFileName, std::ios::in | std::ios::binary | std::ios::ate);
    if(!inStream){
        BOOST_LOG_TRIVIAL(error) << "Could not open input file.";
        exit(1);
    }

    DefectsRenderer::SZ_BODY = (size_t)inStream.tellg();

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
    size_t bufferSize = DefectsRenderer::SZ_BODY - dispatchPoints->at(N_THR - 1);
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
    DefectsRenderer::minTopoCharge = min;
    DefectsRenderer::maxTopoCharge = max;
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


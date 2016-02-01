#include "VisualizationRenderer.h"

double      VisualizationRenderer::R            = 0;
double      VisualizationRenderer::X            = 0;
double      VisualizationRenderer::Y            = 0;
double      VisualizationRenderer::cellSizeX    = 0;
double      VisualizationRenderer::cellSizeY    = 0;
uint32_t    VisualizationRenderer::W            = 0;
uint32_t    VisualizationRenderer::H            = 0;
uint32_t    VisualizationRenderer::frameSize    = 0;
uint32_t    VisualizationRenderer::N            = 0;
uint32_t    VisualizationRenderer::cellsX       = 0;
uint32_t    VisualizationRenderer::cellsY       = 0;
uint32_t    VisualizationRenderer::N_THR        = 1;
bool        VisualizationRenderer::serial       = false;
std::string VisualizationRenderer::outputFile;

void VisualizationRenderer::setThreadAmount(uint32_t N){
    VisualizationRenderer::N_THR = N;
}
void VisualizationRenderer::setOutputFile(std::string filename) {
    VisualizationRenderer::outputFile = filename;
}
void VisualizationRenderer::setParticleAmount(uint32_t N) {
    VisualizationRenderer::N = N;
}
void VisualizationRenderer::setGridDimensions(double X, double Y) {
    VisualizationRenderer::X = X;
    VisualizationRenderer::Y = Y;
}
void VisualizationRenderer::setVideoDimensions(uint32_t W, uint32_t H) {
    VisualizationRenderer::W = W;
    VisualizationRenderer::H = H;
}

void VisualizationRenderer::setParticleRadius(double r) {
    VisualizationRenderer::R = r;
}

void VisualizationRenderer::parallelEncode(int TID, size_t bufferSize, char *buffer) {
    std::stringstream filename;
    cv::VideoWriter videoWriter;

    const double ISO_SCALER = std::min(W / X, H / Y);

    const double CIRCLE_RADIUS = R * ISO_SCALER;

    filename << VisualizationRenderer::outputFile;
    filename << "-";
    filename << TID;
    filename << ".avi";

    videoWriter.open(filename.str(), CV_FOURCC('X','V','I','D'), 60 , cv::Size(W, H), true);
    if(videoWriter.isOpened()){
        BOOST_LOG_TRIVIAL(info) << "Thread " << TID << " opened " << filename.str() << " for writing " << (bufferSize / frameSize) << " frames.";
    }
    else {
        BOOST_LOG_TRIVIAL(error) << "Failed to open output video file in thread " << TID;
    }

    for(uint32_t frameBasePosition = 0; frameBasePosition < bufferSize; frameBasePosition += frameSize){

        cv::Mat image = cv::Mat::zeros(H, W, CV_8UC3);

        for(uint32_t elementOffset = 0; elementOffset < frameSize; elementOffset += sizeof(VisualizationDatapoint)){

            VisualizationDatapoint data;
            data = *((VisualizationDatapoint*)(buffer + frameBasePosition + elementOffset));

            // Rescale to Video-Frame and and point of origin
            data.pX = data.pX * ISO_SCALER;
            data.pY = H - data.pY * ISO_SCALER;
            data.pXn = data.pXn * ISO_SCALER;
            data.pYn = H - data.pYn * ISO_SCALER;

            //Draw the particle
            cv::circle(image, cv::Point(data.pX, data.pY), CIRCLE_RADIUS, cv::Scalar(255, 255, 255), CV_FILLED, CV_AA, 0);
            //Draw force acting on particle
            cv::line(image, cv::Point(data.pX, data.pY), cv::Point(data.pX + data.fX, data.pY - data.fY), cv::Scalar(255, 255, 255), 1, CV_AA, 0);
            //Draw link to neighbour
            cv::line(image,cv::Point(data.pX, data.pY), cv::Point(data.pXn, data.pYn), cv::Scalar(0, 255, 0), 1, CV_AA, 0);
            //Draw cell ID number into particle)
            cv::putText(image, std::to_string(data.cID), cv::Point(data.pX - 2.5,data.pY + 2.5), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.5, cv::Scalar(20,20,200));
            //Draw the cell grid
            for(uint32_t u = 1; u < cellsX; ++u){
                double x = (u * cellSizeX) * ISO_SCALER;
                cv::line(image,cv::Point(x, 0), cv::Point(x, Y * ISO_SCALER), cv::Scalar(255, 0, 0), 1, 8, 0);
            }
            for(uint32_t v = 1; v < cellsY; ++v) {
                double y = (v * cellSizeY) * ISO_SCALER;
                cv::line(image,cv::Point(0, y), cv::Point(X * ISO_SCALER, y), cv::Scalar(255, 0, 0), 1, 8, 0);
            }
        }
        videoWriter << image;
    }
    free(buffer);
    BOOST_LOG_TRIVIAL(info) << "Thread " << TID << " finished.";
}


void VisualizationRenderer::render(std::string inputFileName) {
    uint64_t         steps            = 0;
    size_t           blockSize        = 0;
    size_t           lastBlockSize    = 0;

    std::ifstream inStream(inputFileName, std::ios::in | std::ios::binary | std::ios::ate);
    if(!inStream){
        BOOST_LOG_TRIVIAL(error) << "Could not open input file.";
        exit(1);
    }

    size_t szHeader = 2 * sizeof(double) + 2 * sizeof(uint32_t);
    size_t sz = inStream.tellg();
    sz -=  szHeader;

    inStream.seekg(0);
    inStream.read((char*)&VisualizationRenderer::cellSizeX, sizeof(double));
    inStream.read((char*)&VisualizationRenderer::cellSizeY, sizeof(double));
    inStream.read((char*)&VisualizationRenderer::cellsX, sizeof(uint32_t));
    inStream.read((char*)&VisualizationRenderer::cellsY, sizeof(uint32_t));

    VisualizationRenderer::frameSize = sizeof(VisualizationDatapoint) * N;
    steps = sz / frameSize;
    blockSize = std::ceil((double)steps / N_THR) * frameSize;
    lastBlockSize = sz % blockSize;

    BOOST_LOG_TRIVIAL(info) << "File contains " << steps << " timesteps.";
    BOOST_LOG_TRIVIAL(info) << "Render units: " << N_THR;
    BOOST_LOG_TRIVIAL(info) << "Buffer size per unit: " << blockSize << " bytes";
    BOOST_LOG_TRIVIAL(info) << "Remainder unit has buffer size of " << lastBlockSize << " bytes";

    std::vector<std::thread> threads;
    int32_t  i = (lastBlockSize == 0) ? N_THR: N_THR - 1;
    while(--i >= 0)
    {
        int fpos = szHeader + i * blockSize;
        inStream.seekg(fpos);

        char* buffer = new char[blockSize];
        inStream.read(buffer, blockSize);

        if (serial) {
            parallelEncode(i + 1 , blockSize, buffer);
        }
        else {
            threads.emplace_back(std::thread(parallelEncode, i + 1, blockSize, buffer));
        }
    }
    if(lastBlockSize != 0){
        char* lastBuffer = new char[lastBlockSize];
        inStream.seekg(szHeader + (N_THR - 1) * blockSize);
        inStream.read(lastBuffer, lastBlockSize);
        threads.emplace_back(std::thread(parallelEncode, N_THR, lastBlockSize, lastBuffer));
    }

    for(auto &thread : threads){
        thread.join();
    }

    inStream.close();
    threads.clear();
}

void VisualizationRenderer::processSerially() {
    VisualizationRenderer::serial = true;
}

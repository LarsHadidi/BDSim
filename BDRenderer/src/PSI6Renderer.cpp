#include "PSI6Renderer.h"

double      PSI6Renderer::R             = 0;
double      PSI6Renderer::X             = 0;
double      PSI6Renderer::Y             = 0;
uint32_t    PSI6Renderer::W             = 0;
uint32_t    PSI6Renderer::H             = 0;
uint32_t    PSI6Renderer::frameSize     = 0;
uint32_t    PSI6Renderer::N             = 0;
uint32_t    PSI6Renderer::N_THR         = 1;
bool        PSI6Renderer::serial        = false;
std::string                 PSI6Renderer::outputFile;
PSI6Renderer::COMPONENT     PSI6Renderer::component;

void PSI6Renderer::setThreadAmount(uint32_t N){
    PSI6Renderer::N_THR = N;
}
void PSI6Renderer::setOutputFile(std::string filename) {
    PSI6Renderer::outputFile = filename;
}
void PSI6Renderer::setParticleAmount(uint32_t N) {
    PSI6Renderer::N = N;
}
void PSI6Renderer::setGridDimensions(double X, double Y) {
    PSI6Renderer::X = X;
    PSI6Renderer::Y = Y;
}
void PSI6Renderer::setVideoDimensions(uint32_t W, uint32_t H) {
    PSI6Renderer::W = W;
    PSI6Renderer::H = H;
}

void PSI6Renderer::setParticleRadius(double r) {
    PSI6Renderer::R = r;
}

void PSI6Renderer::setComponent(COMPONENT component) {
    PSI6Renderer::component = component;
}

void PSI6Renderer::processSerially() {
    PSI6Renderer::serial = true;
}

void PSI6Renderer::parallelEncode(int TID, size_t bufferSize, char* buffer){
    std::stringstream filename;
    cv::VideoWriter videoWriter;
    const COMPONENT comp = component;

    const double ISO_SCALER = std::min(W / X, H / Y);
    const double C_SCALE = 255.0 / 2.0;
    const double CIRCLE_RADIUS = R * ISO_SCALER;

    filename << PSI6Renderer::outputFile;
    filename << "-";
    filename << TID;
    filename << ".avi";

    uint32_t particleSize = frameSize / N;

    videoWriter.open(filename.str(), CV_FOURCC('X','V','I','D'), 60 , cv::Size(W, H), true);
    if(videoWriter.isOpened()){
        BOOST_LOG_TRIVIAL(info) << "Thread " << TID << " opened " << filename.str() << " for writing " << (bufferSize / frameSize) << " frames.";
    }
    else {
        BOOST_LOG_TRIVIAL(error) << "Failed to open output video file in thread " << TID;
    }


    cv::Mat BGR(H, W, CV_8UC3);
    for(uint32_t frameBasePosition = 0; frameBasePosition < bufferSize; frameBasePosition+= frameSize){

        cv::Mat image = cv::Mat::zeros(H, W, CV_8UC3);

        for(uint32_t particleOffset = 0; particleOffset < frameSize; particleOffset += particleSize){

            double x = 0;
            double y = 0;
            double re = 0;
            double im = 0;

            uint32_t pos = frameBasePosition + particleOffset;

            x = *((double*)(buffer + pos + 0 * sizeof(double)));
            y = *((double*)(buffer + pos + 1 * sizeof(double)));
            re = *((double*)(buffer + pos + 2 * sizeof(double)));
            im = *((double*)(buffer + pos + 3 * sizeof(double)));

            x = x * ISO_SCALER;
            y = H - y * ISO_SCALER;
            re = (re + 1) * C_SCALE;
            im = (im + 1) * C_SCALE;

            if(comp == REAL) {
                cv::circle(image, cv::Point(x, y), CIRCLE_RADIUS, cv::Scalar(re, 255, 255), CV_FILLED, CV_AA, 0);
            }
            else {
                cv::circle(image, cv::Point(x, y), CIRCLE_RADIUS, cv::Scalar(im, 255, 255), CV_FILLED, CV_AA, 0);
            }
        }

        cv::cvtColor(image, BGR, CV_HSV2BGR);
        videoWriter << BGR;
    }
    free(buffer);
    BOOST_LOG_TRIVIAL(info) << "Thread " << TID << " finished.";
}

void PSI6Renderer::render(std::string inputFileName) {
    uint64_t        steps             = 0;
    size_t          blockSize         = 0;
    size_t          lastBlockSize     = 0;

    std::ifstream inStream(inputFileName, std::ios::in | std::ios::binary | std::ios::ate);
    if(!inStream){
        BOOST_LOG_TRIVIAL(error) << "Could not open input file.";
        exit(1);
    }
    size_t sz = inStream.tellg();
    inStream.seekg(0);

    PSI6Renderer::frameSize = sizeof(PSI6Point) * PSI6Renderer::N;
    steps = sz / PSI6Renderer::frameSize;
    blockSize = std::ceil((double)steps / N_THR) * PSI6Renderer::frameSize;
    lastBlockSize = sz % blockSize;

    BOOST_LOG_TRIVIAL(info) << "File size is " << sz << " bytes and it contains data for " << steps << " timesteps.";
    BOOST_LOG_TRIVIAL(info) << "Render units: " << N_THR;
    BOOST_LOG_TRIVIAL(info) << "Buffer size per unit: " << blockSize << " bytes";
    BOOST_LOG_TRIVIAL(info) << "Remainder unit has buffer size of " << lastBlockSize << " bytes";

    std::vector<std::thread> threads;
    int32_t  i = (lastBlockSize == 0) ? N_THR: N_THR - 1;
    while(--i >= 0)
    {
        inStream.seekg(i * blockSize);

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
        inStream.seekg((N_THR - 1) * blockSize);
        inStream.read(lastBuffer, blockSize);
        threads.emplace_back(std::thread(parallelEncode, N_THR, lastBlockSize, lastBuffer));
    }

    for(auto &thread : threads) {
        thread.join();
    }

    inStream.close();
    threads.clear();
}




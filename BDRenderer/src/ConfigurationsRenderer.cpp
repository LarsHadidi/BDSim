#include "ConfigurationsRenderer.h"

double      ConfigurationsRenderer::R           = 0;
double      ConfigurationsRenderer::X           = 0;
double      ConfigurationsRenderer::Y           = 0;
uint32_t    ConfigurationsRenderer::W           = 0;
uint32_t    ConfigurationsRenderer::H           = 0;
uint32_t    ConfigurationsRenderer::frameSize   = 0;
uint32_t    ConfigurationsRenderer::N           = 0;
uint32_t    ConfigurationsRenderer::N_THR       = 1;
bool        ConfigurationsRenderer::serial      = false;
std::string ConfigurationsRenderer::outputFile;

void ConfigurationsRenderer::setThreadAmount(uint32_t N){
    ConfigurationsRenderer::N_THR = N;
}
void ConfigurationsRenderer::setOutputFile(std::string filename) {
    ConfigurationsRenderer::outputFile = filename;
}
void ConfigurationsRenderer::setParticleAmount(uint32_t N) {
    ConfigurationsRenderer::N = N;
}
void ConfigurationsRenderer::setGridDimensions(double X, double Y) {
    ConfigurationsRenderer::X = X;
    ConfigurationsRenderer::Y = Y;
}
void ConfigurationsRenderer::setVideoDimensions(uint32_t W, uint32_t H) {
    ConfigurationsRenderer::W = W;
    ConfigurationsRenderer::H = H;
}

void ConfigurationsRenderer::setParticleRadius(double r) {
    ConfigurationsRenderer::R = r;
}

void ConfigurationsRenderer::parallelEncode(int TID, size_t bufferSize, char* buffer) {
    std::stringstream filename;
    cv::VideoWriter videoWriter;

    // anisotropic scaling would be rendered like this:
    // const cv::Size ellipseAxes(R * W / X, R * H / Y);
    //cv::ellipse(image, cv::Point(x, y), ellipseAxes, 0, 0, 360, cv::Scalar(200, 200, 200), -1, CV_AA, 0);

    const double ISO_SCALER = std::min(W / X, H / Y);
    const double CIRCLE_RADIUS = R * ISO_SCALER;

    filename << ConfigurationsRenderer::outputFile;
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



    for(uint32_t frameBasePosition = 0; frameBasePosition < bufferSize; frameBasePosition+= frameSize) {

        cv::Mat image = cv::Mat::zeros(H, W, CV_8UC3);

        for(uint32_t particleOffset = 0; particleOffset < frameSize; particleOffset += particleSize){

            double x = 0;
            double y = 0;

            uint32_t pos = frameBasePosition + particleOffset;

            x = *((double*)(buffer + pos + 0 * sizeof(double)));
            y = *((double*)(buffer + pos + 1 * sizeof(double)));

            // Taking vector data , rescale and rasterize it afterwards on the target image (drawing shape onto matrix at target resolution)
            // rasterizing the vector data to the matrix, and rescale it afterwards using cv::resize means rescaling rasterized graphics => low quality

            x = x * ISO_SCALER;
            y = H - y * ISO_SCALER;

            cv::circle(image, cv::Point(x, y), CIRCLE_RADIUS, cv::Scalar(200, 200, 200), CV_FILLED, CV_AA, 0);
        }
        videoWriter << image;
    }
    free(buffer);
    BOOST_LOG_TRIVIAL(info) << "Thread " << TID << " finished.";
}

void ConfigurationsRenderer::render(std::string inputFileName) {
    uint64_t         steps            = 0;
    size_t           blockSize        = 0;
    size_t           lastBlockSize    = 0;


    std::ifstream inStream(inputFileName, std::ios::in | std::ios::binary | std::ios::ate);
    if(!inStream){
        BOOST_LOG_TRIVIAL(error) << "Could not open input file.";
        exit(1);
    }
    size_t sz = inStream.tellg();
    inStream.seekg(0);

    ConfigurationsRenderer::frameSize = sizeof(Particle) * ConfigurationsRenderer::N;
    steps = sz / ConfigurationsRenderer::frameSize;
    blockSize = std::ceil((double)steps / N_THR) * ConfigurationsRenderer::frameSize;
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

    for(auto &thread : threads){
        thread.join();
    }

    inStream.close();
    threads.clear();
}

void ConfigurationsRenderer::processSerially() {
    ConfigurationsRenderer::serial = true;
}
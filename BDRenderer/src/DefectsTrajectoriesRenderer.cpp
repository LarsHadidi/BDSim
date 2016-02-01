#include "DefectsTrajectoriesRenderer.h"

double      DefectsTrajectoriesRenderer::R      = 0;
double      DefectsTrajectoriesRenderer::X      = 0;
double      DefectsTrajectoriesRenderer::Y      = 0;
uint32_t    DefectsTrajectoriesRenderer::W      = 0;
uint32_t    DefectsTrajectoriesRenderer::H      = 0;
uint32_t    DefectsTrajectoriesRenderer::N      = 0;
uint32_t    DefectsTrajectoriesRenderer::N_THR  = 1;
cv::Mat*    DefectsTrajectoriesRenderer::image  = nullptr;
bool DefectsTrajectoriesRenderer::serial        = false;
std::string DefectsTrajectoriesRenderer::outputFile;
std::vector<std::vector<DefectsTrajectoriesRenderer::TrackPoint>>* DefectsTrajectoriesRenderer::previousBuffers;
std::vector<std::vector<DefectsTrajectoriesRenderer::TrackPoint>>* DefectsTrajectoriesRenderer::currentBuffers;
std::vector<cv::Scalar> DefectsTrajectoriesRenderer::colorMap;


void DefectsTrajectoriesRenderer::setThreadAmount(uint32_t N){
    DefectsTrajectoriesRenderer::N_THR = N;
}
void DefectsTrajectoriesRenderer::setOutputFile(std::string filename) {
    DefectsTrajectoriesRenderer::outputFile = filename;
}
void DefectsTrajectoriesRenderer::setParticleAmount(uint32_t N) {
    DefectsTrajectoriesRenderer::N = N;
}
void DefectsTrajectoriesRenderer::setGridDimensions(double X, double Y) {
    DefectsTrajectoriesRenderer::X = X;
    DefectsTrajectoriesRenderer::Y = Y;
}
void DefectsTrajectoriesRenderer::setVideoDimensions(uint32_t W, uint32_t H) {
    DefectsTrajectoriesRenderer::W = W;
    DefectsTrajectoriesRenderer::H = H;
}

void DefectsTrajectoriesRenderer::setParticleRadius(double r) {
    DefectsTrajectoriesRenderer::R = r;
}

void DefectsTrajectoriesRenderer::render(std::string inputFolder){
    DefectsTrajectoriesRenderer::image = new cv::Mat(cv::Mat::zeros(H, W, CV_8UC3));
    cv::VideoWriter videoWriter(DefectsTrajectoriesRenderer::outputFile + ".avi", CV_FOURCC('X','V','I','D'), 60 , cv::Size(W, H), true);

    std::vector<std::ifstream*> inputStreams;
    std::vector<int> chargeMap;

    static const std::regex fileNamePattern("^TR_(-?[0-9]+)$");
    std::smatch matches;

    boost::filesystem::path inputPath(inputFolder);
    for(boost::filesystem::directory_entry& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(inputPath), {})) {
        if (std::regex_search(entry.path().stem().string(), matches, fileNamePattern)) {
	        std::ifstream* f = new std::ifstream(entry.path().string(), std::ios::in | std::ios::binary);
            BOOST_LOG_TRIVIAL(info) << "Found file containing matchings for topological charge " << matches[1];
            inputStreams.push_back(f);
            chargeMap.push_back(boost::lexical_cast<int>(matches[1]));
        }
    }
    const int MIN_CHARGE = *std::min_element(chargeMap.begin(), chargeMap.end());
    const int MAX_CHARGE = *std::max_element(chargeMap.begin(), chargeMap.end());
    const double C_SCALE = 255.0 / (MAX_CHARGE - MIN_CHARGE);
    for(int i = 0; i < chargeMap.size(); ++i){
        int charge = chargeMap[i];
        const uint8_t COLOR = (double)(charge - MIN_CHARGE) * C_SCALE;
        colorMap.push_back(cv::Scalar(COLOR, 255, 255));
    }

    previousBuffers = new std::vector<std::vector<TrackPoint>>(inputStreams.size());
    currentBuffers = new std::vector<std::vector<TrackPoint>>(inputStreams.size());

    for(size_t i = 0; i < inputStreams.size(); ++i) {
        (*previousBuffers)[i].reserve(BUFFER_SIZE);
        (*currentBuffers)[i].reserve(BUFFER_SIZE);
    }
    // Bootstrapping ------------------------------------------------------------------------
    for(size_t i = 0; i < inputStreams.size(); ++i) {
        uint32_t sz;
        inputStreams[i]->read((char*)&sz, sizeof(uint32_t));
        (*previousBuffers)[i].resize(sz);
        inputStreams[i]->read((char*)(*previousBuffers)[i].data(), sz * sizeof(TrackPoint));
    }
    // --------------------------------------------------------------------------------------
    BOOST_LOG_TRIVIAL(info) << "Rendering trajectories.";

    size_t finishedStreams = 0;
    while(finishedStreams < inputStreams.size()) {
        finishedStreams = 0;
        for (size_t i = 0; i < inputStreams.size(); ++i) {
            if (inputStreams[i]->peek() != EOF) {

                uint32_t sz;
                inputStreams[i]->read((char *) &sz, sizeof(uint32_t));

                /*TrackPoint trackPoint;
                for(size_t j = 0; j < sz; ++j){
                    inputStreams[i]->read((char*)&trackPoint, sizeof(TrackPoint));
                    (*currentBuffers)[i].push_back(trackPoint);
                }*/
                (*currentBuffers)[i].resize(sz);
                inputStreams[i]->read((char *) (*currentBuffers)[i].data(), sz * sizeof(TrackPoint));
                encode(i);
            }
            else {
                finishedStreams++;
            }
        }

        cv::Mat BGR(H, W, CV_8UC3);
        cv::cvtColor(*image, BGR, CV_HSV2BGR);
        videoWriter << BGR;
        std::vector<std::vector<TrackPoint>> *tmp = previousBuffers;
        previousBuffers = currentBuffers;
        currentBuffers = tmp;

        for (size_t i = 0; i < inputStreams.size(); ++i) {
            (*currentBuffers)[i].resize(0);
        }

    }

    for(size_t i = 0; i < inputStreams.size(); ++i){
        inputStreams[i]->close();
    }
    BOOST_LOG_TRIVIAL(info) << "Done.";
    delete image;
    delete previousBuffers;
    delete currentBuffers;
}

void DefectsTrajectoriesRenderer::encode(const int ID) {
    const double ISO_SCALER = std::min(W / X, H / Y);

    for(TrackPoint previousTrackPoint : (*previousBuffers)[ID]) {

        double pX = previousTrackPoint.x;
        double pY = previousTrackPoint.y;

        bool matched = false;
        TrackPoint currentTrackPoint;
        size_t i = 0;
        while(!matched && i < (*currentBuffers)[ID].size()) {
            currentTrackPoint = (*currentBuffers)[ID][i];
            if(previousTrackPoint.trackID == currentTrackPoint.trackID){
                matched = true;
            }
            ++i;
        }

        if(matched){
            double cX = currentTrackPoint.x;
            double cY = H - currentTrackPoint.y;
            const double dX = pX - cX;
            const double dY = pY - cY;
            const double dXsquared = dX * dX;
            const double dYsquared = dY * dY;
            if(4 * dXsquared < X * X && 4 * dYsquared < Y * Y) {
                cX = cX * ISO_SCALER;
                pX = pX * ISO_SCALER;
                cY = H - cY * ISO_SCALER;
                pY = H - pY * ISO_SCALER;
                cv::line(*image, cv::Point(pX, pY), cv::Point(cX, cY), DefectsTrajectoriesRenderer::colorMap[ID], 1, CV_AA, 0);
            }
        }
    }
}

void DefectsTrajectoriesRenderer::processSerially() {
    DefectsTrajectoriesRenderer::serial = true;
}
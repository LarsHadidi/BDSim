#pragma once

#define BUFFER_SIZE 2048

#include <string>
#include <stdint.h>
#include <sstream>
#include <regex>

#include <thread>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <vector>
#include <utility>
#include <future>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>


class DefectsTrajectoriesRenderer {
#pragma pack(push)
#pragma pack(1)
    typedef struct TrackPoint_s {
        uint64_t trackID;
        double x;
        double y;
    } TrackPoint;
#pragma pack(pop)
public:
    static void processSerially();
    static void setThreadAmount(uint32_t N);
    static void setParticleAmount(uint32_t N);
    static void setVideoDimensions(uint32_t W, uint32_t H);
    static void setGridDimensions(double X, double Y);
    static void setParticleRadius(double r);
    static void setOutputFile(std::string filename);
    static void render(std::string inputFolder);
private:
    static void encode(const int ID);
    static uint32_t W,H,N;
    static double X,Y;
    static double R;
    static std::string outputFile;
    static uint32_t N_THR;
    static cv::Mat* image;
    static std::vector<std::vector<TrackPoint>>* previousBuffers;
    static std::vector<std::vector<TrackPoint>>* currentBuffers;
    static std::vector<cv::Scalar> colorMap;
    static bool serial;

};


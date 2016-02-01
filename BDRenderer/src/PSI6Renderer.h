#pragma once

#include <string>
#include <stdint.h>
#include <sstream>
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

class PSI6Renderer {
#pragma pack(push)
#pragma pack(1)
    typedef struct {
        double x;
        double y;
        double re;
        double im;
    } PSI6Point;
#pragma pack(pop)
public:
    enum COMPONENT {REAL, IMAGINARY};
    static void processSerially();
    static void setThreadAmount(uint32_t N);
    static void setComponent(COMPONENT);
    static void setParticleAmount(uint32_t N);
    static void setVideoDimensions(uint32_t W, uint32_t H);
    static void setGridDimensions(double X, double Y);
    static void setParticleRadius(double r);
    static void setOutputFile(std::string filename);
    static void render(std::string);
private:
    static void parallelEncode(int, size_t , char*);
    static uint32_t W,H,frameSize, N;
    static double X,Y;
    static double R;
    static std::string outputFile;
    static COMPONENT component;
    static uint32_t N_THR;
    static bool serial;
};

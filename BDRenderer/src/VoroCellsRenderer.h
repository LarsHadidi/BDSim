#pragma once

#define MAX_POLYGON_VERTICES 32

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

#include <shapes/Colorbar.h>

class VoroCellsRenderer {
public:
    enum COMPONENT {TOPOCHARGE, PSI6REAL, PSI6IMAGINARY};
    static void processSerially();
    static void setThreadAmount(uint32_t N);
    static void setVideoDimensions(uint32_t W, uint32_t H);
    static void setComponent(COMPONENT);
    static void setGridDimensions(double X, double Y);
    static void setParticleRadius(double r);
    static void setParticleAmount(uint32_t N);
    static void setOutputFile(std::string filename);
    static void render(std::string);
private:
    static std::vector<std::streampos>* findSplitPoints(std::ifstream&);
    static std::vector<std::streampos>* findDispatchPoints(std::vector<std::streampos>&);
    static std::pair<int64_t,int64_t> parallelFindTopoChargeBoundaries(int, size_t, char*);
    static void parallelEncode(int, size_t, char*);
    static uint32_t W,H,frameSize, N;
    static double X,Y;
    static double R;
    static int64_t maxTopoCharge, minTopoCharge;
    static const std::streamsize SZ_CELL_HEADER;
    static size_t SZ_BODY;
    static std::string outputFile;
    static uint32_t N_THR;
    static bool serial;
    static COMPONENT component;
};

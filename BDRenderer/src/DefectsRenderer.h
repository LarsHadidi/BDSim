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

#include <shapes/Colorbar.h>


class DefectsRenderer {
#pragma pack(push)
#pragma pack(1)
    typedef struct {
        double x;
        double y;
        int64_t topoCharge;
    } Defect;
#pragma pack(pop)
public:
    static void setThreadAmount(uint32_t N);
    static void setVideoDimensions(uint32_t W, uint32_t H);
    static void setGridDimensions(double X, double Y);
    static void setParticleRadius(double r);
    static void setOutputFile(std::string filename);
    static void render(std::string);
private:
    static std::vector<std::streampos>* findSplitPoints(std::ifstream&);
    static std::vector<std::streampos>* findDispatchPoints(std::vector<std::streampos>&);
    static void parallelEncode(int, size_t, char*);
    static std::pair<int64_t,int64_t> parallelFindTopoChargeBoundaries(int, size_t, char*);
    static uint32_t W,H,frameSize;
    static double X,Y;
    static double R;
    static int64_t maxTopoCharge, minTopoCharge;
    static size_t SZ_BODY;
    static std::string outputFile;
    static uint32_t N_THR;
};

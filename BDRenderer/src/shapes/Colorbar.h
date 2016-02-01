#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>

#include <vector>

namespace shapes {
    class Colorbar {
        public:
            Colorbar(const double x, const double y, const double W, const double H, const size_t intervalWidth, const int intervalStart);
            ~Colorbar();
            void render(cv::Mat& image);
        private:
            const cv::Point2d BASE_POINT;
            const double WIDTH;
            const double HEIGHT;
            const double COLOR_SCALE_FACTOR;
            const uint64_t INTERVAL_WIDTH;
            const int INTERVAL_START;
            const double FRAGMENT_HEIGHT;
            std::vector<cv::Scalar> colorMap;
    };

}

#include <shapes/Colorbar.h>

namespace shapes {
    Colorbar::Colorbar(const double x, const double y, const double W, const double H, const size_t intervalWidth, const int intervalStart) :
    BASE_POINT(x, y), WIDTH(W), HEIGHT(H),
    COLOR_SCALE_FACTOR(255.0 / intervalWidth), INTERVAL_WIDTH(intervalWidth), INTERVAL_START(intervalStart), FRAGMENT_HEIGHT(H / (intervalWidth + 1))
    {
        // hue = (charge - intervalStart) / invervalWidth * 255
        // offset charge to zero, normalize it to invervalWidth and scale up to the domain size
        // last two steps correspond to the scaleFactor
        // first step corresponds to the counter being already shifted, starting at zero and running to upper intervalBound being shifted
        for(size_t k = 0; k <= this->INTERVAL_WIDTH; ++k) {
            const uint8_t COLOR = k * this->COLOR_SCALE_FACTOR;
            colorMap.push_back(cv::Scalar(COLOR, 255, 255));
        }
    }

    Colorbar::~Colorbar() {

    }

    void Colorbar::render(cv::Mat& image) {
        cv::rectangle(image,
                      this->BASE_POINT,
                      cv::Point(this->BASE_POINT.x + this->WIDTH, this->BASE_POINT.y + this->HEIGHT),
                      cv::Scalar(0, 0, 255),
                      2, CV_AA, 0);
        for(int k = 0; k <= this->INTERVAL_WIDTH; ++k) {
            // there are (intervalWidth + 1) integer charges, fragmentHeight corresponding to that
            // on every iteration, the Y_OFFSET will be increased by that fragmentHeight
            const double OFFSET_Y = k * this->FRAGMENT_HEIGHT;
            cv::rectangle(image,
                         cv::Point(this->BASE_POINT.x, this->BASE_POINT.y + OFFSET_Y),
                         cv::Point(this->BASE_POINT.x + this->WIDTH,  this->BASE_POINT.y + OFFSET_Y + this->FRAGMENT_HEIGHT),
                         this->colorMap[k],
                         CV_FILLED, CV_AA, 0);
            cv::putText(image,
                       std::to_string(k + this->INTERVAL_START),
                       cv::Point(this->BASE_POINT.x + this->WIDTH * 1.2, this->BASE_POINT.y + OFFSET_Y + 0.8 * this->FRAGMENT_HEIGHT),
                       cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8,
                       cv::Scalar(0,0,255));
        }
    }
}
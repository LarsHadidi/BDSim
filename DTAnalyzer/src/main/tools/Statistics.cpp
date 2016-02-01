#include <tools/Statistics.h>

namespace tools {

    Statistics::Statistics() : sampleCount(0) { }

    Statistics::~Statistics() { }

    void Statistics::clear() {
        sampleCount = 0;
    }

    void Statistics::push(double x) {
        sampleCount++;

        if (sampleCount == 1) {
            currentMean = x;
            previousMean = currentMean;
            previousAccVar = 0.0;
        }
        else {
            currentMean = previousMean + (x - previousMean) / sampleCount;
            currentAccVar = previousAccVar + (x - previousMean) * (x - currentMean);

            previousMean = currentMean;
            previousAccVar = currentAccVar;
        }
    }

    int Statistics::getSampleCount() const {
        return sampleCount;
    }

    double Statistics::getMean() const {
        return (sampleCount > 0) ? currentMean : 0.0;
    }

    double Statistics::getVariance() const {
        return ((sampleCount > 1) ? currentAccVar / (sampleCount - 1) : 0.0);
    }

    double Statistics::getStandardDeviation() const {
        return sqrt(getVariance());
    }
}
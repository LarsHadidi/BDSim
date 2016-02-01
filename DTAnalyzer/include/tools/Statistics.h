#pragma once

#include <math.h>

namespace tools {

    class Statistics {
    public:
        Statistics();

        ~Statistics();

        void clear();

        void push(double x);

        int getSampleCount() const;

        double getMean() const;

        double getVariance() const;

        double getStandardDeviation() const;

    private:
        int sampleCount;
        double previousMean, currentMean, previousAccVar, currentAccVar;
    };
}
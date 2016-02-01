#pragma once

#include <math.h>
#include <gsl/gsl_integration.h>


class EffectiveDiameter {
    public:
        EffectiveDiameter();
        ~EffectiveDiameter();
        double estimate(double temperature);
    private:
        gsl_integration_workspace* gslWorkspace;
        static double kernel(double x, void* parameters);
        double parameter;
        gsl_function F;
};




#include <tools/EffectiveDiameter.h>

EffectiveDiameter::EffectiveDiameter() {
    this->gslWorkspace = gsl_integration_workspace_alloc (1000);

    this->F.function = &(EffectiveDiameter::kernel);
    this->F.params = &(this->parameter);

}
EffectiveDiameter::~EffectiveDiameter() {
    gsl_integration_workspace_free (this->gslWorkspace);
}

double EffectiveDiameter::kernel(double x, void *parameters) {
    double T = *(double *) parameters;
    double f = pow(x + 0.5, -7.0/6.0) * exp(-4.0 * (x * x) / T);
    return f;
}

double EffectiveDiameter::estimate(double temperature) {
    this->parameter = temperature;
    double result, error;
    gsl_integration_qagiu(&F, 0, 0, 1e-10, 1000, this->gslWorkspace, &result, &error);
    result = pow(2, 1.0/6.0) - 1.0/6.0 * result;
    return result;
}
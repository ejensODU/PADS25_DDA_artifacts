#include "Dist.h"

#include <array>


IntDist::IntDist()  {}
IntDist::~IntDist()  {}

UniformIntDist::UniformIntDist(int lower, int upper, int distSeed)
:_lower(lower), _upper(upper)
{
   std::uniform_int_distribution<>::param_type ps(lower, upper);
   _distribution.param(ps);
   //int s = std::chrono::system_clock::now().time_since_epoch().count();
   _generator.seed(distSeed);

   _ID = "uniform int, lower " + std::to_string(_lower) + ", upper " + std::to_string(_upper);
}

int UniformIntDist::GenRV() { return _distribution(_generator); }

std::string UniformIntDist::getID() { return _ID; }

UniformIntDist::~UniformIntDist()  {}


RealDist::RealDist()  {}
RealDist::~RealDist()  {}

ConstantRealDist::ConstantRealDist(double value)
:_value(value)
{
   _ID = "constant, value " + std::to_string(_value);
}

double ConstantRealDist::GenRV() { return _value; }

std::string ConstantRealDist::getID() { return _ID; }

ConstantRealDist::~ConstantRealDist()  {}


NormalDist::NormalDist(double mu, double sigma, int distSeed)
:_mu(mu), _sigma(sigma)
{
   std::normal_distribution<double>::param_type ps(mu, sigma);
   _distribution.param(ps);
   //int s = std::chrono::system_clock::now().time_since_epoch().count();
   _generator.seed(distSeed);

   _ID = "normal, mu " + std::to_string(_mu) + ", sigma " + std::to_string(_sigma);
}

double NormalDist::GenRV() { return _distribution(_generator); }

std::string NormalDist::getID() { return _ID; }

NormalDist::~NormalDist()  {}


ExpoDist::ExpoDist(double lambda, int distSeed)
:_lambda(lambda)
{
   std::exponential_distribution<double>::param_type ps(lambda);
   _distribution.param(ps);
   //int s = std::chrono::system_clock::now().time_since_epoch().count();
   _generator.seed(distSeed);

   _ID = "exponential, lambda " + std::to_string(_lambda);
}

double ExpoDist::GenRV() { return _distribution(_generator); /*printf("expo rv: %lf\n", expo_rv); return expo_rv;*/ }

std::string ExpoDist::getID() { return _ID; }

ExpoDist::~ExpoDist()  {}


UniformRealDist::UniformRealDist(double lower, double upper, int distSeed)
:_lower(lower), _upper(upper)
{
   std::uniform_real_distribution<double>::param_type ps(lower, upper);
   _distribution.param(ps);
   //int s = std::chrono::system_clock::now().time_since_epoch().count();
   _generator.seed(distSeed);

   _ID = "uniform real, lower " + std::to_string(_lower) + ", upper " + std::to_string(_upper);
}

double UniformRealDist::GenRV() { return _distribution(_generator); }

std::string UniformRealDist::getID() { return _ID; }

UniformRealDist::~UniformRealDist()  {}


TriangularDist::TriangularDist(double min, double peak, double max, int distSeed)
:_min(min), _peak(peak), _max(max)
{

   std::array<double,3> intervals{_min, _peak, _max};
   std::array<double,3> weights{0, 1, 0};

   std::piecewise_linear_distribution<double>::param_type ps(intervals.begin(), intervals.end(), weights.begin());
   _distribution.param(ps);
   //int s = std::chrono::system_clock::now().time_since_epoch().count();
   _generator.seed(distSeed);

   _ID = "triangular, min " + std::to_string(_min) + ", peak " + std::to_string(_peak) + ", max " + std::to_string(_max);
}

double TriangularDist::GenRV() { return _distribution(_generator); }

std::string TriangularDist::getID() { return _ID; }

TriangularDist::~TriangularDist()  {}

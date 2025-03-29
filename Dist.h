#pragma once

#include <string>
#include <chrono>
#include <random>

class IntDist
{
   public:
      IntDist();
      virtual int GenRV() = 0;
      virtual std::string getID() = 0;
      virtual ~IntDist();
};

class UniformIntDist : public IntDist
{
   public:
      UniformIntDist(int lower, int upper, int distSeed);
      int GenRV();
      void Reset(int distSeed)  { _generator.seed(distSeed); }
      std::string getID();
      ~UniformIntDist();

   private:
      std::uniform_int_distribution<> _distribution;
      std::default_random_engine _generator;
      int _upper;
      int _lower;
      std::string _ID;
};


class RealDist
{
   public:
      RealDist();
      virtual double GenRV() = 0;
      virtual std::string getID() = 0;
      virtual ~RealDist();
};


class ConstantRealDist : public RealDist
{
   public:
      ConstantRealDist(double value);
      double GenRV();
      std::string getID();
      ~ConstantRealDist();

   private:
      double _value;
      std::string _ID;
};


class NormalDist : public RealDist
{
   public:
      NormalDist(double mu, double sigma, int distSeed);
      double GenRV();
      std::string getID();
      ~NormalDist();

   private:
      std::normal_distribution<double> _distribution;
      std::default_random_engine _generator;
      double _mu;
      double _sigma;
      std::string _ID;
};


class ExpoDist : public RealDist
{
   public:
      ExpoDist(double lambda, int distSeed);
      double GenRV();
      std::string getID();
      ~ExpoDist();

   private:
      std::exponential_distribution<double> _distribution;
      std::default_random_engine _generator;
      double _lambda;
      std::string _ID;
};


class UniformRealDist : public RealDist
{
   public:
      UniformRealDist(double lower, double upper, int distSeed);
      double GenRV();
      double getMinVal()  { return _lower; }
      std::string getID();
      ~UniformRealDist();

   private:
      std::uniform_real_distribution<double> _distribution;
      std::default_random_engine _generator;
      double _upper;
      double _lower;
      std::string _ID;
};


class TriangularDist : public RealDist
{
   public:
      TriangularDist(double min, double peak, double max, int distSeed);
      double GenRV();
      double getMinVal()  { return _min; }
      void Reset(int distSeed)  { _generator.seed(distSeed); }
      std::string getID();
      ~TriangularDist();

   private:
      std::piecewise_linear_distribution<double> _distribution;
      std::default_random_engine _generator;
      double _min;
      double _peak;
      double _max;
      std::string _ID;
};

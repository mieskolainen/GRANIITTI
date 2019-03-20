// Random number class
//
// (c) 2017-2019 Mikael Mieskolainen
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.


#ifndef MRANDOM_H
#define MRANDOM_H

// C++
#include <complex>
#include <random>
#include <vector>


namespace gra {

class MRandom  {

public:
	
	// Calling constructors of member functions
	MRandom() : flat(0,1), gaussian(0,1) {
		rng.seed(); // Default initialization
	}
	~MRandom() {}

	// Set random number engine seed
	void SetSeed(int seed) {
		
		const int SEEDMAX = 2147483647;
		if (seed > SEEDMAX) {
			std::string str = "MRandom::SetSeed: Invalid input seed: " +
				std::to_string(seed) + " > SEEDMAX = " + std::to_string(SEEDMAX);
			throw std::invalid_argument(str);
		}
		rng.seed(seed);
		RNDSEED = seed;
	}
	
	// Return current random seed
	unsigned int GetSeed() const {
		return RNDSEED;
	}
	
	// Random sampling functions
	double U(double a, double b);
	double G(double mu, double sigma);	
	double PowerRandom(double a, double b, double alpha);
	double RelativisticBWRandom(double m, double Gamma, double LIMIT = 15.0); // Keep it high to avoid artifacts
	double CauchyRandom(double m, double Gamma, double LIMIT = 15.0);
	int    NBDRandom(double avgN, double k, int maxvalue);
	int    PoissonRandom(double lambda);
	double ExpRandom(double lambda);
	int    LogRandom(double p, int maxvalue);
	void   DirRandom(const std::vector<double>& alpha, std::vector<double>& y);

	double NBDpdf(int n, double avgN, double k);
	double Logpdf(int k, double p);

	// 64-bit Mersenne Twister by Matsumoto and Nishimura, 2000 (fast)
	std::mt19937_64 rng; 

	// 48-bit RANLUX (slow)
	//std::ranlux48 rng;
	
	// Distribution engines
	std::uniform_real_distribution<double> flat;
	std::normal_distribution<double> gaussian;
	unsigned int RNDSEED = 0;    // Random seed set

private:
	// Nothing
};

} // gra namespace ends


#endif

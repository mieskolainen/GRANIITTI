// "Meta class" for Spherical Harmonics Expansion - contains
// dataprocessing/Minuit/ROOT plotting functions.
//
// All spherical harmonic processing is separated to MSpherical namespace.
// 
// 
// (c) 2017-2019 Mikael Mieskolainen
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.

// C++
#include <complex>
#include <iostream>
#include <random>
#include <vector>

// ROOT
#include "TCanvas.h"
#include "TColor.h"
#include "TGraphErrors.h"
#include "TH1.h"
#include "TH2.h"
#include "TLegend.h"
#include "TLine.h"
#include "TLorentzVector.h"
#include "TMinuit.h"
#include "TProfile.h"
#include "TStyle.h"

// Own
#include "Graniitti/Analysis/MHarmonic.h"
#include "Graniitti/MSpherical.h"
#include "Graniitti/MMath.h"
#include "Graniitti/MKinematics.h"
#include "Graniitti/M4Vec.h"
#include "Graniitti/MAux.h"
#include "Graniitti/MPDG.h"

// Eigen
#include <Eigen/Dense>


using gra::aux::indices;
using gra::math::msqrt;
using gra::math::zi;
using gra::math::PI;
using gra::math::pow2;


namespace gra {


// Constructor
MHarmonic::MHarmonic() {
}


// Initialize
void MHarmonic::Init(const HPARAM& hp) {


	LMAX             = hp.LMAX;
	REMOVEODD        = hp.REMOVEODD;
	REMOVENEGATIVEM  = hp.REMOVENEGATIVEM;
	LAMBDA           = hp.LAMBDA;

	NCOEF            = (LMAX + 1)*(LMAX + 1);

	// ------------------------------------------------------------------
	// SETUP parameters of the fit
	ACTIVE.resize(NCOEF); // Active moments
	ACTIVENDF = 0;

	for (int l = 0; l <= LMAX; ++l) {
		for (int m = -l; m <= l; ++m) {
			const int index = gra::spherical::LinearInd(l, m);
			ACTIVE[index] = true;

			// FIX ODD MOMENTS TO ZERO
			if (REMOVEODD && ((l % 2) != 0)) {
				ACTIVE[index] = false;
			}
			// FIX NEGATIVE M TO ZERO
			if (REMOVENEGATIVEM && (m < 0)) {
				ACTIVE[index] = false;
			}
			if (ACTIVE[index]) {
				++ACTIVENDF;
			}
		}
	}
	
	// ------------------------------------------------------------------
	std::cout << std::endl;
	std::cout << "MHarmonic::Init: LMAX = " << LMAX << std::endl;
	std::cout << std::endl;
	// ------------------------------------------------------------------

	// Init arrays used by Minuit function
	t_lm       = std::vector<double>(NCOEF,    0.0);
	t_lm_error = std::vector<double>(NCOEF,    0.0);
	errmat     = MMatrix<double>(NCOEF, NCOEF, 0.0);
	covmat     = MMatrix<double>(NCOEF, NCOEF, 0.0);

	// Test functions
	gra::spherical::TestSphericalIntegrals(LMAX);

	std::cout << rang::fg::yellow <<
				"<Spherical Harmonic Based (costheta,phi)_r.f. Decomposition and Efficiency inversion>" << std::endl << std::endl;
	std::cout << "TERMINOLOGY:" << rang::fg::reset << std::endl;
	std::cout << "  {G} Generated == Events in the angular flat phase space (no cuts on final states, only on the system)" << std::endl;
	std::cout << "  {F} Fiducial  == Events within the strict fiducial (geometric-kinematic) final state phase space (cuts on final states)" << std::endl;
	std::cout << "  {S} Selected  == Events after the detector efficiency losses" << std::endl;
	std::cout << std::endl;
	std::cout << "{S} subset of {F} subset of {G} (this strict hierarchy might be violated in some special cases)" << std::endl;
	std::cout << std::endl;
	std::cout << "  Basic idea is to define {G} such that minimal extrapolation " << std::endl;
	std::cout << "  is required from the strict fiducial (geometric) phase space {F}. " << std::endl;
	std::cout << "  Flatness requirement of {G} is strictly required to represent moments in unmixed basis (non-flat phase space <=> geometric moment mixing)." << std::endl;
	std::cout << std::endl;
	std::cout << rang::fg::yellow << "EXAMPLE OF A FORMALLY VALID DEFINITION:" << rang::fg::reset << std::endl;
	std::cout << "  G = {|Y(system)| < 0.9}" << std::endl;
	std::cout << "  F = {|eta(pi)|   < 0.9 && pt(pi) > 0.1 GeV}" << std::endl;
	std::cout << std::endl << std::endl;
	std::cout << "Note also the rotation between inactive coefficients and active one, due to moment mixing." << std::endl;
	std::cout << std::endl << std::endl;
}


// Destructor
MHarmonic::~MHarmonic() {
}


bool MHarmonic::PrintLoop(const std::string& output) const {

	/*
	// ------------------------------------------------------------------
	// Print out results for EXTERNAL analysis
	FILE* fp;

	char buff[100];
	snprintf(buff, sizeof(buff), "./fits/%s_%s.m", output.c_str(), FRAME.c_str());
	std::string outputfile = buff;

	fp = fopen(outputfile.c_str(), "w");

	if (fp == NULL) {
		fprintf(stderr, "Cannot open outputfile %s !\n", outputfile.c_str());
		return false;
	}

	fprintf(fp, "FRAME = %s; \n",     FRAME.c_str());
	fprintf(fp, "LMAX = %d; \n",      LMAX);
	fprintf(fp, "LAMBDA = %0.3f; \n", LAMBDA);
	fprintf(fp, "ACTIVE = [");

	for (std::size_t i = 0; i < ACTIVE.size(); ++i) {
		fprintf(fp, "%d ", (int)ACTIVE[i]);
	}
	fprintf(fp, "]; \n\n");
	fprintf(fp, "McellS = [");

	for (std::size_t i = 0; i < masscenter.size(); ++i) {
		fprintf(fp, "%0.3f ", masscenter[i]);
	}
	fprintf(fp, "]; \n\n");

	fprintf(fp, "A{1} = [");
	PrintMatrix(fp, ful.t_lm_FIT);
	fprintf(fp, "];\n");
	fprintf(fp, "A{2} = [");
	PrintMatrix(fp, fid.t_lm_FIT);
	fprintf(fp, "];\n");
	fprintf(fp, "A{3} = [");
	PrintMatrix(fp, det.t_lm_FIT);
	fprintf(fp, "];\n");

	fprintf(fp, "A{4} = [");
	PrintMatrix(fp, ful.t_lm_OBS);
	fprintf(fp, "];\n");
	fprintf(fp, "A{5} = [");
	PrintMatrix(fp, fid.t_lm_OBS);
	fprintf(fp, "];\n");
	fprintf(fp, "A{6} = [");
	PrintMatrix(fp, det.t_lm_OBS);
	fprintf(fp, "];\n");

	fclose(fp);
	*/
	return true;
}


void MHarmonic::PlotAll() const {
/*
	// **** EFFICIENCY ****
	PlotFigures(ful.E_lm,     "E_lm_ful", 47);
	PlotFigures(fid.E_lm,     "E_lm_fid", 44);
	PlotFigures(det.E_lm,     "E_lm_det", 41);

	// **** FITTED MOMENTS ****
	PlotFigures(ful.t_lm_FIT, "t_lm_FIT_ful", 16);
	PlotFigures(fid.t_lm_FIT, "t_lm_FIT_fid", 38);
	PlotFigures(det.t_lm_FIT, "t_lm_FIT_det", 29);
	
	// **** ALGEBRAIC MOMENTS ****
	PlotFigures(ful.t_lm_OBS, "t_lm_OBS_ful", 28);
	PlotFigures(fid.t_lm_OBS, "t_lm_OBS_fid", 25);
	PlotFigures(det.t_lm_OBS, "t_lm_OBS_det", 25);
*/
}


// Print figures
void MHarmonic::PlotFigures(const std::vector<std::vector<double>>& matrix,
                            const std::string& outputfile, int barcolor) const {

	/*
	TCanvas* c1 = new TCanvas("c1", "c1", 700, 500); // horizontal, vertical
	c1->Divide(sqrt(NCOEF), std::sqrt(NCOEF), 0.002, 0.001);

	// Loop over moments
	int kk = 0;
	std::vector<TGraphErrors*> gr(NCOEF, NULL);

	for (int l = 0; l <= LMAX; ++l) {
		for (int m = -l; m <= l; ++m) {
			const int index = gra::spherical::LinearInd(l, m);

			// Set canvas position
			c1->cd(kk + 1);

			// Loop over mass cells
			double x[MASScellS]     = {0};
			double y[MASScellS]     = {0};
			double x_err[MASScellS] = {1e-2};
			double y_err[MASScellS] = {1e-2};

			for (std::size_t k = 0; k < MASScellS; ++k) {
				x[k] = masscenter[k];
				y[k] = matrix[k][index];
			}
			
			// Draw horizontal line
			
			//TLine* line = new TLine(x[0]*0.9, 0.0,
			//x[MASScellS-1]*1.1, 0.0);
			//
			//if (!(l == 0 && m == 0)) {
			//    line->SetLineColor(kRed);
			//    line->SetLineWidth(1.0);
			//    line->Draw("same");
			//}
			
			// Create the TGraphErrors and draw it
			gr[kk] = new TGraphErrors(MASScellS, x, y, x_err, y_err);
			gr[kk]->SetTitle(Form("<%d,%d>", l, m));

			// gr->SetMarkerColor(4); // blue

			gr[kk]->SetMarkerStyle(20);
			gr[kk]->SetMarkerSize(0.5);

			gStyle->SetBarWidth(0.8);
			gr[kk]->SetFillColor(barcolor); // gray

			gr[kk]->GetXaxis()->SetTitleSize(0.05);
			gr[kk]->GetXaxis()->SetLabelSize(0.05);

			gr[kk]->GetYaxis()->SetTitleSize(0.05);
			gr[kk]->GetYaxis()->SetLabelSize(0.05);

			gStyle->SetTitleFontSize(0.08);

			gr[kk]->Draw("A B");
			// gr[kk]->Draw("ALP");
			// c1->Update();
			++kk;
		}
	}
	c1->Print(Form("./harmonicfit/%s_FRAME_%s.pdf", outputfile.c_str(), FRAME.c_str()));

	for (std::size_t i = 0; i < gr.size(); ++i) {
		delete gr[i];
	}

	delete c1;
	*/
}


// Loop over system mass and pt-cells
void MHarmonic::HyperLoop(void (*fitfunc)(int&, double*, double&, double*, int),
		const std::vector<gra::spherical::Omega>& MC,
		const std::vector<gra::spherical::Omega>& DATA) {

	// --------------------------------------------------------
	// Pre-Calculate once Spherical Harmonics for MINUIT fit
	DATA_events = DATA;
	Y_lm = gra::spherical::YLM(DATA_events, LMAX);
	// --------------------------------------------------------

	double chi2 = 0.0;

	const unsigned int MASScellS  = 50;
	const unsigned int PTcellS    = 1;
	const unsigned int YcellS     = 1;

	ful = MTensor<gra::spherical::SH>({MASScellS, PTcellS, YcellS});
	fid = MTensor<gra::spherical::SH>({MASScellS, PTcellS, YcellS});
	det = MTensor<gra::spherical::SH>({MASScellS, PTcellS, YcellS});
	
	// System Mass limits
	const double M_MIN   = 0.4;
	const double M_MAX   = 2.0;

	// System Pt limits
	const double PT_MIN  = 0.0;
	const double PT_MAX  = 5.0;

	// System rapidity limits
	const double Y_MIN   = -2.0;
	const double Y_MAX   =  2.0;

	const double  M_STEP = (M_MAX  - M_MIN)  / MASScellS;
	const double PT_STEP = (PT_MAX - PT_MIN) / PTcellS;
	const double  Y_STEP = (Y_MAX  - Y_MIN)  / YcellS;

	// ------------------------------------------------------------------
	// Hyperloop
	for (std::size_t i = 0; i < MASScellS; ++i) {
		const double M_min = i * M_STEP + M_MIN;
		const double M_max = M_min + M_STEP;

	for (std::size_t j = 0; j < PTcellS;   ++j) {
		const double Pt_min = j * PT_STEP + PT_MIN;
		const double Pt_max = Pt_min + PT_STEP;

	for (std::size_t k = 0; k < YcellS;    ++k) {
		const double Y_min = k * Y_STEP + Y_MIN;
		const double Y_max = Y_min + Y_STEP;

		// Cell
		const std::vector<std::size_t> cell = {i,j,k};
		
		// --------------------------------------------------------------
		// Get indices for this interval
		const std::vector<std::size_t> MC_ind = 
			gra::spherical::GetIndices(MC, {M_min, M_max}, {Pt_min, Pt_max}, {Y_min, Y_max});

		DATA_ind = gra::spherical::GetIndices(DATA_events, {M_min, M_max}, {Pt_min, Pt_max}, {Y_min, Y_max});

		const unsigned int MINEVENTS = 75;
		if (DATA_ind.size() < MINEVENTS) {
			std::cout << rang::fg::red << "WARNING: Less than "
					  << MINEVENTS << " in the cell!" << rang::fg::reset << std::endl;
		}

		// --------------------------------------------------------------
		// Acceptance mixing matrices
		ful(cell).MIXlm = gra::spherical::GetGMixing(MC, MC_ind, LMAX, 0);
		fid(cell).MIXlm = gra::spherical::GetGMixing(MC, MC_ind, LMAX, 1);
		det(cell).MIXlm = gra::spherical::GetGMixing(MC, MC_ind, LMAX, 2);
		// --------------------------------------------------------------

		// --------------------------------------------------------------
		// Get efficiency decomposition for this interval
		std::pair<std::vector<double>, std::vector<double>> E0 = gra::spherical::GetELM(MC, MC_ind, LMAX, 0);
		std::pair<std::vector<double>, std::vector<double>> E1 = gra::spherical::GetELM(MC, MC_ind, LMAX, 1);
		std::pair<std::vector<double>, std::vector<double>> E2 = gra::spherical::GetELM(MC, MC_ind, LMAX, 2);

		ful({i,j,k}).E_lm       = E0.first;
		ful({i,j,k}).E_lm_error = E0.second;

		fid({i,j,k}).E_lm       = E1.first;
		fid({i,j,k}).E_lm_error = E1.second;

		det({i,j,k}).E_lm       = E2.first;
		det({i,j,k}).E_lm_error = E2.second;
		// --------------------------------------------------------------


		// ==============================================================
		// ALGORITHM 1: DIRECT / OBSERVED / ALGEBRAIC decomposition

		fid({i,j,k}).t_lm_OBS = gra::spherical::SphericalMoments(DATA_events, DATA_ind, LMAX, 1);
		det({i,j,k}).t_lm_OBS = gra::spherical::SphericalMoments(DATA_events, DATA_ind, LMAX, 2);

		// Forward matrix
		Eigen::MatrixXd M     = gra::aux::Matrix2Eigen(det({i,j,k}).MIXlm);

		// Matrix pseudoinverse
		const double REGPARAM = 1e-3; // SVD singular value regularization strength
		Eigen::VectorXd b     = gra::aux::Vector2Eigen(det({i,j,k}).t_lm_OBS);
		Eigen::VectorXd x     = gra::math::PseudoInverse(M, REGPARAM) * b;

		// Collect the inversion result
		ful({i,j,k}).t_lm_OBS = gra::aux::Eigen2Vector(x);

		// ==============================================================


		// Update these to proper errors (TBD. PUT ZERO FOR NOW!!)
		ful({i,j,k}).t_lm_OBS_error = std::vector<double>(ful({i,j,k}).t_lm_OBS.size(), 0.0);
		fid({i,j,k}).t_lm_OBS_error = std::vector<double>(fid({i,j,k}).t_lm_OBS.size(), 0.0);
		det({i,j,k}).t_lm_OBS_error = std::vector<double>(det({i,j,k}).t_lm_OBS.size(), 0.0);

		
		std::cout << "Algebraic Moore-Penrose/SVD (unmixed) moments in the (angular flat) reference phase space:" << std::endl;
		gra::spherical::PrintOutMoments(ful({i,j,k}).t_lm_OBS, ful({i,j,k}).t_lm_OBS_error, ACTIVE, LMAX);

		std::cout << "Algebraic (mixed) moments in the fiducial space:" << std::endl;
		gra::spherical::PrintOutMoments(fid({i,j,k}).t_lm_OBS, fid({i,j,k}).t_lm_OBS_error, ACTIVE, LMAX);

		std::cout << "Algebraic (mixed) moments in the detector space:" << std::endl;
		gra::spherical::PrintOutMoments(det({i,j,k}).t_lm_OBS, det({i,j,k}).t_lm_OBS_error, ACTIVE, LMAX);


		// ==============================================================
		// ALGORITHM 2: Extended Maximum Likelihood Fit

		// *** Get TMinuit based fit decomposition ***
		MomentFit({i,j,k}, fitfunc);

		// Save result
		ful({i,j,k}).t_lm_FIT = t_lm;
		fid({i,j,k}).t_lm_FIT = fid({i,j,k}).MIXlm * t_lm; // Moment forward rotation: Matrix * Vector
		det({i,j,k}).t_lm_FIT = det({i,j,k}).MIXlm * t_lm; // Moment forward rotation: Matrix * Vector

		// Errors should be propagated after moment rotation (PUT SAME FOR ALL NOW)
		ful({i,j,k}).t_lm_FIT_error = t_lm_error;
		fid({i,j,k}).t_lm_FIT_error = t_lm_error;
		det({i,j,k}).t_lm_FIT_error = t_lm_error;

		// ==============================================================


		std::cout << "Maximum-Likelihood (unmixed) moments in the (angular flat) reference phase space:" << std::endl;
		gra::spherical::PrintOutMoments(ful({i,j,k}).t_lm_FIT, ful({i,j,k}).t_lm_FIT_error, ACTIVE, LMAX);

		std::cout << "Maximum-Likelihood Re-Back-Projected (mixed) moments in the fiducial space:" << std::endl;
		gra::spherical::PrintOutMoments(fid({i,j,k}).t_lm_FIT, fid({i,j,k}).t_lm_FIT_error, ACTIVE, LMAX);

		std::cout << "Maximum-Likelihood Re-Back-Projected (mixed) moments in the detector space:" << std::endl;
		gra::spherical::PrintOutMoments(det({i,j,k}).t_lm_FIT, det({i,j,k}).t_lm_FIT_error, ACTIVE, LMAX);

		// --------------------------------------------------------------
		// Print results
		chi2 += PrintOutHyperCell({i,j,k});

		// --------------------------------------------------------------
		// Make comparison of synthetic data vs estimate
		int fiducial = 0;
		int selected = 0;
		for (const auto& l : DATA_ind) {
			if (DATA_events[l].fiducial){ ++fiducial; }
			if (DATA_events[l].fiducial && DATA_events[l].selected) { ++selected; }
		}
		std::cout << std::endl;
		printf("cf. Synthetic data events generated               = %u \n", (unsigned int) DATA_ind.size());
		printf("    Synthetic data events fiducial                = %d (acceptance %0.1f percent) \n",
			fiducial, fiducial / (double) DATA_ind.size() * 100);
		printf("    Synthetic data events fiducial and selected   = %d (efficiency %0.1f percent) \n", 
			selected, selected / (double) fiducial * 100);
	}}}

	gra::aux::PrintBar("=");
	double reducedchi2 = chi2 / (double)(ACTIVENDF * MASScellS);
	if (reducedchi2 < 3) {
		std::cout << rang::fg::green;
	} else {
		std::cout << rang::fg::red;
	}
	printf("Total Chi2 / (ACTIVENDF x MASScellS) = %0.2f / (%d x %d) = %0.2f \n", 
		chi2, ACTIVENDF, MASScellS, reducedchi2);
	std::cout << rang::fg::reset;
	gra::aux::PrintBar("=");
	std::cout << std::endl << std::endl;
}


// Print out results for the hypercell
double MHarmonic::PrintOutHyperCell(const std::vector<std::size_t>& cell) {
	
	// Loop over moments
	double chi2 = 0;
	for (int l = 0; l <= LMAX; ++l) {
		for (int m = -l; m <= l; ++m) {
			const  int index = gra::spherical::LinearInd(l, m);
			
			const double obs = det(cell).t_lm_OBS[index];
			const double fit = det(cell).t_lm_FIT[index];
			
			// Moment active
			if (ACTIVE[index]) {
				chi2 += pow2(obs - fit) / pow2(obs);
			}
		}
	}
	std::cout << std::endl;
	
	const double reducedchi2 = chi2 / (double) ACTIVENDF;
	if (reducedchi2 < 3) {
		std::cout << rang::fg::green;
	} else {
		std::cout << rang::fg::red;
	}
	printf("Chi2 / ndf = %0.3f / %d = %0.3f \n", chi2, ACTIVENDF, reducedchi2);
	std::cout << rang::fg::reset << std::endl;
	

	// From the Maximum Likelihood fit
	double sum_FUL = t_lm[0];
	printf("Estimate of events in the reference phase space = %0.1f +- %0.1f \n", sum_FUL, 0.0);

	double sum_FID = gra::spherical::HarmDotProd(fid(cell).E_lm, t_lm, ACTIVE, LMAX);
	printf("Estimate of events in the fiducial  phase space = %0.1f +- %0.1f \n", sum_FID, 0.0);

	double sum_DET = gra::spherical::HarmDotProd(det(cell).E_lm, t_lm, ACTIVE, LMAX);
	printf("Estimate of events in the detector        space = %0.1f +- %0.1f \n", sum_DET, 0.0);

	return chi2;
}


// MINUIT based fit routine for the Extended Maximum Likelihood formalism
void MHarmonic::MomentFit(const std::vector<std::size_t>& cell,
						  void (*fitfunc)(int&, double*, double&, double*, int)) {

	std::cout << "MomentFit: Starting ..." << std::endl << std::endl;

	 // **** This must be set for the loss function ****
	activecell = cell;

	// Init TMinuit
	TMinuit* gMinuit = new TMinuit(NCOEF); // initialize TMinuit with a maximum of N params
	gMinuit->SetFCN(fitfunc);

	// Set Print Level
	// -1 no output
	// 1 standard output
	gMinuit->SetPrintLevel(-1);

	// Set error Definition
	// 1 for Chi square
	// 0.5 for negative log likelihood
	//    gMinuit->SetErrorDef(0.5);
	double arglist[2];
	int ierflg = 0;
	arglist[0] = 0.5; // 0.5 <=> We use negative log likelihood cost function
	gMinuit->mnexcm("SET ERR", arglist, 1, ierflg);

	//double fminbest = 1e32;
	// Try different initial values to find out true minimum
	const int TRIALMAX = 1;

	for (std::size_t trials = 0; trials < TRIALMAX; ++trials) {

		for (int l = 0; l <= LMAX; ++l) {
			for (int m = -l; m <= l; ++m) {
				const int index = gra::spherical::LinearInd(l, m);

				const std::string str = "t_" + std::to_string(l) + std::to_string(m);

				const double max = 1e8;  // Physically bounded ultimately by the number of events
				const double min = -1e5;

				// ======================================================
				// *** Use the algebraic inverse solution as a good starting value ***
				const double start_value = ful(activecell).t_lm_OBS[index];
				// ======================================================
				
				const double step_value = 0.1; // in units of Events

				gMinuit->mnparm(index, str, start_value, step_value, min, max, ierflg);

				// After first trial, fix t_00
				if (trials > 0) {
					gMinuit->mnparm(0, "t_00", t_lm[0], 0, 0, 0, ierflg);
					gMinuit->FixParameter(0);
				}
				// FIX ODD MOMENTS TO ZERO
				if (REMOVEODD && ((l % 2) != 0)) {
					gMinuit->mnparm(index, str, 0, 0, 0, 0, ierflg);
					gMinuit->FixParameter(index);
				}
				// FIX NEGATIVE M TO ZERO
				if (REMOVENEGATIVEM && (m < 0)) {
					gMinuit->mnparm(index, str, 0, 0, 0, 0, ierflg);
					gMinuit->FixParameter(index);
				}
			}
		}
		// Scan main parameter
		// gMinuit->mnscan();
		arglist[0] = 100000;  // Minimum number of function calls
		arglist[1] = 0.00001; // Minimum tolerance

		// First simplex to find approximate answers
		gMinuit->mnexcm("SIMPLEX", arglist, 2, ierflg);

		// Numerical Hessian (2nd derivatives matrix), inverse of this -> covariance matrix
		gMinuit->mnexcm("MIGRAD", arglist, 2, ierflg);

		// Confidence intervals based on the profile likelihood ratio
		gMinuit->mnexcm("MINOS", arglist, 2, ierflg);

        // Calculate error & covariance matrix
        gMinuit->mnemat(&errmat[0][0], NCOEF);

        for (int i = 0; i < NCOEF; ++i) {
            for (int j = 0; j < NCOEF; ++j){
                covmat[i][j] = sqrt(errmat[i][i]*errmat[j][j]);
	            if (covmat[i][j] > 1E-80) {
	                covmat[i][j] = errmat[i][j]/covmat[i][j];
	            }
	            else covmat[i][j] = 0.0;
            }
        }
       	errmat.Print("Error matrix");
        covmat.Print("Covariance matrix");

		// Print results
		double fmin, fedm, errdef = 0.0;
		int nvpar, nparx, istat = 0;
		gMinuit->mnstat(fmin, fedm, errdef, nvpar, nparx, istat);

		// Collect fit result
		for (int l = 0; l <= LMAX; ++l) {
			for (int m = -l; m <= l; ++m) {
				const int index = gra::spherical::LinearInd(l, m);

				// Set results into t_lm[], t_lm_error[]
				gMinuit->GetParameter(index, t_lm[index], t_lm_error[index]);
			}
		}

	} // TRIALS LOOP END

	// Sum of N random variables Y = X_1 + X_2 + ... X_N
	// sigma_Y^2 = \sum_i^N \sigma_i^2 + 2 \sum_i \sum_i < j
	// covariance(X_i, X_j)

	// Print out results (see MINUIT manual for these parameters)
	double fmin, fedm, errdef = 0.0;
	int nvpar, nparx, istat = 0;
	gMinuit->mnstat(fmin, fedm, errdef, nvpar, nparx, istat);
	gMinuit->mnprin(4, fmin);

	std::cout << "<MINUIT (MIGRAD+MINOS)> done." << std::endl << std::endl;

	delete gMinuit;
}


// Uncellned Extended Maximum Likelihood function
// Extended means that the number of events (itself) is a Poisson distributed
// random variable and that is incorporated to the fit.
void MHarmonic::logLfunc(int& npar, double* gin, double& f, double* par, int iflag) const {

	// Collect fit t_LM coefficients from MINUIT
	std::vector<double> T(NCOEF, 0.0);

	for (const auto& i : indices(T)) {
		T[i] = par[i];
		// printf("T[%d] = %0.5f \n", i);
	}
	// This is the number of events in the current phase space point at detector level
	const int METHOD = 2;
	double nhat = 0.0;
	
	// Equivalent estimator 1
	if (METHOD == 1) {
		const std::vector<double> t_lm_det = det(activecell).MIXlm * T; // Matrix * Vector
		nhat = t_lm_det[0];
	}
	// Equivalent estimator 2
	if (METHOD == 2) {
		nhat = gra::spherical::HarmDotProd(det(activecell).E_lm, T, ACTIVE, LMAX);
	}


	// For each event, calculate \sum_{LM} t_{lm}
	// Re[Y_{lm}(costheta,phi_k)], k is the event index
	std::vector<double> I0;
	const double V = msqrt(4.0 * PI); // Normalization volume

	for (const auto& k : DATA_ind) {

		// Event is accepted
		if ( DATA_events[k].fiducial && DATA_events[k].selected ) {
			// fine
		} else { continue; }

		// Loop over (l,m) terms
		double sum = 0.0;
		for (int l = 0; l <= LMAX; ++l) {
			for (int m = -l; m <= l; ++m) {
				const int index = gra::spherical::LinearInd(l, m);

				if (ACTIVE[index]) {

					// Calculate here
            		//const std::complex<double> Y =
            		//	gra::math::Y_complex_basis(DATA_events[k].costheta,DATA_events[k].phi, l,m);
            		//const double ReY = gra::math::NReY(Y,l,m);
            		
            		// Pre-calculated for speed
            		const double ReY = Y_lm[k][index];

            		// Add
            		sum += T[index] * ReY;
				}
			}
		}
		I0.push_back(V * sum);
	}
	// Fidelity term
	double logL = 0;

	for (const auto& k : indices(I0)) {

		if (I0[k] > 0) {
			logL += std::log(I0[k]);
			// Positivity is not satisfied, make strong penalty
		} else {
			logL = -1e32;
			// The Re[Y_lm] (purely real) formulation here does
			// not, explicitly, enforce non-negativity.
			// Where as |A|^2 formulation would enforce it by
			// construction,
			// but that would have -l <= m <= l parameters, i.e.,
			// over redundant representation problem
			// on the other hand.
		}
	}
	
	// Note the minus sign on logL term
	f = -logL + nhat;

	// High penalty, total event count estimate has gone out of physical
	if (nhat < 0){
		f = 1e32;
	}

	// L1-norm regularization (Laplace prior), -> + ln(P_laplace)
	double l1term = 0.0;
	const unsigned int START = 1;
	for (std::size_t i = START; i < T.size(); ++i) {
		l1term += std::abs(T[i]);
	}
	l1term /= T[0]; // Normalization

	f += l1term * LAMBDA;

	// printf("MHarmonic:: cost-functional = %0.4E, nhat = %0.1f \n", f, nhat);
}

} // gra namespace ends

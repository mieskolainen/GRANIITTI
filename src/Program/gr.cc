// GRANIITTI - Monte Carlo event generator for high energy diffraction
// https://github.com/mieskolainen/graniitti
//
// <Main generator program>
//
// (c) 2017-2020 Mikael Mieskolainen
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.

// C++
#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>

// Own
#include "Graniitti/MAux.h"
#include "Graniitti/MGraniitti.h"
#include "Graniitti/MMatrix.h"

// Libraries
#include "cxxopts.hpp"
#include "json.hpp"
#include "rang.hpp"

using namespace gra;

// Main
int main(int argc, char *argv[]) {
  aux::PrintArgv(argc, argv);

  // Save the number of input arguments
  const int NARGC = argc - 1;

  // Create generator object first
  std::unique_ptr<MGraniitti> gen = std::make_unique<MGraniitti>();

  try {
    cxxopts::Options options(argv[0], "");

    options.add_options("")("i,INPUT", "Input card", cxxopts::value<std::string>())("H,help",
                                                                                    "Help");

    options.add_options("GENERALPARAM")

        ("o,OUTPUT", "Output name            <string>", cxxopts::value<std::string>())(
            "f,FORMAT", "Output format          <hepmc3|hepmc2|hepevt>",
            cxxopts::value<std::string>())("n,NEVENT", "Number of events       <integer32>",
                                           cxxopts::value<unsigned int>())(
            "g,INTEGRATOR", "Integrator             <VEGAS|FLAT>", cxxopts::value<std::string>())(
            "w,WEIGHTED", "Weighted events        <true|false>", cxxopts::value<std::string>())(
            "c,CORES", "Number of CPU threads  <integer>", cxxopts::value<unsigned int>());

    options.add_options("PROCESSPARAM")("p,PROCESS", "Process                 <string>",
                                        cxxopts::value<std::string>())(
        "e,ENERGY", "CMS energy              <double>", cxxopts::value<double>())(
        "l,POMLOOP", "Screening Pomeron loop  <true|false>", cxxopts::value<std::string>())(
        "s,NSTARS", "Excite protons          <0|1|2>", cxxopts::value<unsigned int>())(
        "q,LHAPDF", "Set LHAPDF              <string>", cxxopts::value<std::string>())(
        "h,HIST", "Histogramming           <0|1|2>", cxxopts::value<unsigned int>())(
        "r,RNDSEED", "Random seed             <integer32>", cxxopts::value<unsigned int>());

    auto r = options.parse(argc, argv);

    if (r.count("help") || NARGC == 0) {
      gen->GetProcessNumbers();
      std::cout << options.help({"", "GENERALPARAM", "PROCESSPARAM"}) << std::endl;
      std::cout << rang::style::bold << " Arrow operators:" << rang::style::reset << std::endl;
      std::cout << "  use -> between initial and final states" << std::endl;
      std::cout << "  use &> (instead of ->) for a decoupled central system phase space (use with "
                   "<F>, <P> class, e.g. for s-channel resonances)"
                << std::endl;
      std::cout
          << "  use  > for recursive decaytree branchings with curly brackets { grand daughters }"
          << std::endl;
      std::cout << std::endl;
      std::cout << rang::style::bold
                << " Inline 'on-the-flight' parameters to concatenate with PROCESS string:"
                << rang::style::reset << std::endl
                << std::endl;

      std::cout << rang::style::bold << "  [Generic]" << rang::style::reset << std::endl;

      std::cout << "  @FLATAMP:N                              flat matrix element for 'pure' phase "
                   "space generation, set N to -1 for more info"
                << std::endl;
      std::cout << "  @FLATMASS2:true                         flat sampling in M^2 instead of "
                   "relativistic Breit-Wigner f(M^2) in decay trees"
                << std::endl;
      std::cout << "  @OFFSHELL:X                             how many +- full widths particles "
                   "off-shell in decay trees (X = 0 on-shell, X = 5 default)"
                << std::endl;
      std::cout
          << "  @PDG[X]{M:350.0, W:5.0}                 new mass and width for pdg particle id X"
          << std::endl;
      std::cout
          << "  @R[f0_980]{M:0.990, W:0.065}            set new central resonance mass and width"
          << std::endl;
      std::cout << "  @RES{rho_770:1, ..., f2_1270:1}         set active resonances in the "
                   "amplitude (1 active, 0 inactive)"
                << std::endl;

      std::cout << std::endl;
      std::cout << rang::style::bold << "  [Pomeron amplitudes]" << rang::style::reset << std::endl;

      std::cout << "  @SPINGEN:true                           set generation 2->1 spin "
                   "correlations active (true, false)"
                << std::endl;
      std::cout << "  @SPINDEC:true                           set decay 1->2 spin correlations "
                   "active (true, false)"
                << std::endl;

      std::cout << "  @FRAME:X                                set Lorentz frame definition for the "
                   "spin density matrix (X = CS, HX, CM)"
                << std::endl;
      std::cout << "  @R[f2_1270]{JZ0:0.5, JZ1:0.0, JZ2:0.5}  set new diagonal spin density matrix "
                   "elements for resonances"
                << std::endl;
      std::cout << "  @JMAX:X                                 set maximum Pomeron helicity for the "
                   "sliding helicity amplitudes"
                << std::endl;

      std::cout << std::endl;
      std::cout << rang::style::bold << "  [Tensor pomeron amplitudes]" << rang::style::reset
                << std::endl;

      std::cout << "  @R[f0_980]{g0:1.0, g1:0.2, ...}         set new production couplings {g0,g1} "
                   "[scalar/pseudoscalar] {g0,...,g6} [tensor]"
                << std::endl;

      std::cout << std::endl;
      std::cout << rang::style::bold << " PROCESS string examples:" << rang::style::reset
                << std::endl;

      std::cout << "  yy[CON]<C> -> mu+ mu-" << std::endl;
      std::cout << "  yy[Higgs]<F> &> 22 22" << std::endl;
      std::cout << "  yy[CON]<C> -> 992 -992 @PDG[992]{M:1500,W:0}" << std::endl;
      std::cout << "  yP[RESTENSOR]<C> -> pi+ pi- @RES{rho_770:1}" << std::endl;
      std::cout << "  PP[CON]<C> -> rho(770)0 > {pi+ pi-} rho(770)0 > {pi+ pi-}" << std::endl;
      std::cout << "  PP[CON]<F> -> pi+ pi- pi+ pi-" << std::endl;
      std::cout << "  PP[CON]<C> -> p+ p- @FLATAMP:2" << std::endl;
      std::cout << "  PP[RES+CON]<F> -> pi+ pi- @RES{f0_500:0,rho_770:1,f0_980:1,f2_1270:1} "
                   "@R[f0_980]{M:0.98,W:0.065}"
                << std::endl;

      std::cout << std::endl;

      std::cout << rang::style::bold << rang::fg::green
                << " A steering card example with no additional input:" << rang::style::reset
                << std::endl;
      std::cout << "  " << argv[0] << " -i ./input/test.json" << std::endl << std::endl;

      std::cout << rang::style::bold << rang::fg::red
                << " A steering card example with commandline input override:" << rang::style::reset
                << std::endl;
      std::cout << "  " << argv[0] << " -i ./input/test.json -p \"yy[QED]<F> -> e+ e-\""
                << std::endl
                << std::endl;

      aux::CheckUpdate();

      return EXIT_FAILURE;
    }

    // -------------------------------------------------------------------
    // This first
    // Process re-set from commandline
    if (r.count("p")) {
      gen->ReadInput(r["i"].as<std::string>(), r["p"].as<std::string>());
    } else {
      gen->ReadInput(r["i"].as<std::string>());
    }

    // -------------------------------------------------------------------
    // Quick custom parameters

    // General parameters (order is important)
    if (r.count("n")) { gen->SetNumberOfEvents(r["n"].as<unsigned int>()); }
    if (r.count("o") && r.count("f")) {
      gen->SetOutput(r["o"].as<std::string>());
      gen->SetFormat(r["f"].as<std::string>());
    } else if ((r.count("o") && !r.count("f")) || (!r.count("o") && r.count("f"))) {
      throw std::invalid_argument("Error: Use options -o and -f together.");
    }
    if (r.count("g")) {
      const std::string val = r["g"].as<std::string>();
      gen->SetIntegrator(val);
    }
    if (r.count("w")) {
      const std::string val = r["w"].as<std::string>();
      gen->SetWeighted(val == "true");
    }
    if (r.count("c")) { gen->SetCores(r["c"].as<unsigned int>()); }

    // Process parameters (adding more might be involved due to initialization
    // in
    // MProcess...)
    if (r.count("l")) {
      const std::string val = r["l"].as<std::string>();
      gen->proc->SetScreening(val == "true");
    }
    if (r.count("e")) {
      double E = r["e"].as<double>() / 2.0;  // CMS-energy in symmetric collider
      gen->proc->SetBeamEnergies(E, E);
    }
    if (r.count("s")) { gen->proc->SetExcitation(r["s"].as<unsigned int>()); }
    if (r.count("q")) { gen->proc->SetLHAPDF(r["q"].as<std::string>()); }
    if (r.count("h")) { gen->proc->SetHistograms(r["h"].as<unsigned int>()); }
    if (r.count("r")) { gen->proc->random.SetSeed(r["r"].as<unsigned int>()); }

    // -------------------------------------------------------------------

    // Initialize generator (always last!)
    gen->Initialize();

    // Generate events
    gen->Generate();

    // Print histograms
    gen->PrintHistograms();

    // Exception handling
  } catch (const std::invalid_argument &e) {
    gen->GetProcessNumbers();
    gra::aux::PrintGameOver();
    std::cerr << rang::fg::red << "Exception catched: " << rang::fg::reset << e.what() << std::endl;
    return EXIT_FAILURE;
  } catch (const std::ios_base::failure &e) {
    gra::aux::PrintGameOver();
    std::cerr << rang::fg::red << "Exception catched: std::ios_base::failure: " << rang::fg::reset
              << e.what() << std::endl;
    return EXIT_FAILURE;
  } catch (const cxxopts::OptionException &e) {
    gra::aux::PrintGameOver();
    std::cerr << rang::fg::red << "Exception catched: Commandline options: " << rang::fg::reset
              << e.what() << std::endl;
    return EXIT_FAILURE;
  } catch (const nlohmann::json::exception &e) {
    gra::aux::PrintGameOver();
    std::cerr << rang::fg::red << "Exception catched: JSON input: " << rang::fg::reset << e.what()
              << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    gen->GetProcessNumbers();
    gra::aux::PrintGameOver();
    std::cerr << rang::fg::red << "Exception catched: Unspecified (...) (Probably JSON input)"
              << rang::fg::reset << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "[gr: done]" << std::endl;
  aux::CheckUpdate();

  return EXIT_SUCCESS;
}

#!/bin/sh
#
# Simulation and analysis with CMS cuts
#
# Run with: source ./tests/run_xxx/run.sh

read -p "cmsrun: Generate events (or only analyze)? [y/n] " -n 1 -r
echo # New line

if [[ $REPLY =~ ^[Yy]$ ]]
then

N=100000

# Generate
./bin/gr -i ./tests/processes/CMS19_2pi.json -w true -l false -n $N
./bin/gr -i ./tests/processes/CMS19_2K.json -w true -l false -n $N
./bin/gr -i ./tests/processes/CMS19_ppbar.json -w true -l false -n $N

fi

# Analyze
# Hard-coded integrated screening factor (for speed, set 1 if Pomeron loop was on)
S2=0.18

./bin/analyze \
-i "CMS19_2pi, CMS19_2K, CMS19_ppbar" \
-g "211, 321, 2212" \
-n "2, 2, 2" \
-l "#pi^{+}#pi^{-}, #it{K}^{+}#it{K}^{-}, #it{p}#bar{#it{p}}" \
-M "95, 0.0, 3.0" \
-Y "95,-2.5, 2.5" \
-P "95, 0.0, 2.0" \
-u ub \
-t "#sqrt{s} = 13 TeV, |#eta| < 2.5, p_{T} > 0.15 GeV" \
-S "$S2, $S2, $S2" #-X 1000

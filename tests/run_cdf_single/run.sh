#!/bin/sh
#
# Simulation and analysis with CDF cuts
#
# Run with: source ./tests/run_xxx/run.sh

POMLOOP=false

# Hard-coded integrated screening factor (for speed, set 1 if Pomeron loop was on)
S2=0.2

read -p "Generate events (or only analyze)? [y/n] " -n 1 -r
echo # New line

if [[ $REPLY =~ ^[Yy]$ ]]
then

N=100000

# Generate
make -j4 && ./bin/gr -i ./tests/processes/CDF14_2pi.json -n $N -l $POMLOOP -w true

fi

# Analyze
make -j4 && ./bin/analyze \
-i CDF14_2pi \
-g 211 \
-n 2 \
-l 'GRANIITTI #pi^{+}#pi^{-}' \
-t '#sqrt{s} = 1.96 TeV, |#eta| < 1.3, p_{T} > 0.4 GeV, |Y_{x}| < 1.0' \
-M "95, 0.0, 2.5" \
-Y "95,-1.5, 1.5" \
-P "95, 0.0, 1.5" \
-u ub \
-S $S2

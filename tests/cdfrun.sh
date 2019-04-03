#!/bin/sh

# Hard-coded integrated screening factor (for speed, set 1 if Pomeron loop was on)
S2=0.2

make -j4 && ./bin/gr -i ./tests/processes/CDF14_2pi.json -n 100000 -l false -w true

make -j4 ROOT=TRUE && ./bin/analyze -i CDF14_2pi -g 211 -n 2 -l \
	'#pi^{+}#pi^{-}' -t '#sqrt{s} = 1.96 TeV, |#eta| < 1.3, p_{T} > 0.4 GeV, |Y_{X}| < 1.0' \
	-M 2.5 -Y 1.5 -P 1.5 -u ub -S $S2

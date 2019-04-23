#!/bin/sh

# Hard-coded integrated screening factor (for speed, set 1 if Pomeron loop was on)
S2=0.18

make -j4 && ./bin/analyze \
-i ALICE_2pi,ALICE_2K,ALICE_ppbar \
-g 211,321,2212 \
-n 2,2,2 \
-l '#pi^{+}#pi^{-}','#it{K}^{+}#it{K}^{-}','#it{p}#bar{#it{p}}' \
-M 3.0 -Y 1.5 -P 2.0 \
-u ub \
-t '#sqrt{s} = 13 TeV, |#eta| < 0.9, p_{T} > 0.15 GeV' \
-S $S2,$S2,$S2 #-X 1000

#convert -density 600 -trim .h1_S_M_logy.pdf -quality 100 output.jpg
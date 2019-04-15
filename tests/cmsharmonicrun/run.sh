#!/bin/sh
#
# Run with: source ./tests/harmonic/run.sh

read -p "cmsharmonicrun: Generate events (or only analyze)? [y/n] " -n 1 -r
echo # New line

if [[ $REPLY =~ ^[Yy]$ ]]
then

# Generate
./bin/gr -i ./tests/cmsharmonicrun/SH_2pi_J0_CMS.json -n 100000
./bin/gr -i ./tests/cmsharmonicrun/SH_2pi_CMS.json    -n 100000

fi

# ***********************************************************************
# Fiducial cuts <ETAMIN,ETAMAX,PTMIN,PTMAX>
FIDCUTS=-2.5,2.5,0.1,100.0
# ***********************************************************************

# System kinematic variables binning <bins,min,max>
MBINS=40,0.28,2.0
PBINS=1,0.0,2.0
YBINS=1,-0.9,0.9

# PARAMETERS
LMAX=2
REMOVEODD=true
REMOVENEGATIVE=true
SVDREG=1e-5
L1REG=0 #1e-5
EML=true

# Lorentz frames
for FRAME in HE # CS PG SR
do

# Analyze
./bin/fitharmonic -r SH_2pi_J0_CMS -i SH_2pi_J0_CMS,SH_2pi_CMS \
-l 'GRANIITTI J=0,GRANIITTI #pi^{+}#pi^{-}' -m MC,MC -s true,true  \
-t '#Omega{Detector}: |#eta| < 2.5 #wedge p_{T} > 0.1 GeV,#Omega{Fiducial}: |#eta| < 2.5 #wedge p_{T} > 0.1 GeV,#Omega{Flat}: |Y_{x}| < 2.5' \
-c $FIDCUTS \
-f $FRAME -g $LMAX -o $REMOVEODD -n $REMOVENEGATIVE -a $SVDREG -b $L1REG -e $EML \
-M $MBINS -P $PBINS -Y $YBINS \
-X 100000

done

# Implement 2D harmonic plots (M,Pt)
# ...
% Setup script

% Indices
x_ind  = 1;
Re_ind = 2;
Im_ind = 3;

% CMS-Energies
%{
sqrts  = [62 546 1800 7000 13000 60000];
factor = [20000 400 20 1 1/20 1/1000]; % Visualization
%}

%
sqrts  = [546 1800 7000 13000 60000];
factor = [200 20 1 1/20 1/1000]; % Visualization
%}

s      = sqrts.^2;

% Units
GeV2fm = 0.1973;
GeV2mb = 0.389;

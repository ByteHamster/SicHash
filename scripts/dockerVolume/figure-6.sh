#!/bin/bash

# Run benchmark
cd /opt/sichash/build
./Comparison --numKeys 5M --numQueries 0 --loadFactor 0.85 --sichash --sichashOnlyPartial --pthash --chd | tee figure-6.txt
./Comparison --numKeys 5M --numQueries 0 --loadFactor 0.95 --sichash --sichashOnlyPartial --pthash --chd | tee --append figure-6.txt
./Comparison --numKeys 5M --numQueries 0 --recsplit --bbhash | tee --append figure-6.txt

# Build plot
cd /opt/sichash/scripts
cp /opt/sichash/build/figure-6.txt figure-6.txt
/opt/sqlplot-tools/build/src/sqlplot-tools figure-6.tex
pdflatex figure-6.tex
pdflatex figure-6.tex
cp figure-6.pdf /opt/dockerVolume
cp figure-6.txt /opt/dockerVolume

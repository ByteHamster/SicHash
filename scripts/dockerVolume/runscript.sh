#!/bin/bash

# Run benchmark
cd /opt/sichash/build
./Comparison --numKeys 5M --numQueries 0 --loadFactor 0.85 --bbhash --sichash --pthash --chd | tee comparisonPlot.txt
./Comparison --numKeys 5M --numQueries 0 --loadFactor 0.95 --bbhash --sichash --pthash --chd | tee --append comparisonPlot.txt
./Comparison --numKeys 5M --numQueries 0 --recsplit | tee --append comparisonPlot.txt

# Build plot
cd /opt/sichash/scripts
cp /opt/sichash/build/comparisonPlot.txt comparisonPlot.txt
/opt/sqlplot-tools/build/src/sqlplot-tools comparisonPlot.tex
pdflatex comparisonPlot.tex
cp comparisonPlot.pdf /opt/dockerVolume
cp comparisonPlot.txt /opt/dockerVolume


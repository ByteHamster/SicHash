#!/bin/bash

# Run benchmark
./ComparisonN --pthashParameter 3.35 --numKeys 1M    --numQueries 20M --iterations 2 | tee figure-5.txt
./ComparisonN --pthashParameter 3.75 --numKeys 3M    --numQueries 20M --iterations 2 | tee --append figure-5.txt
./ComparisonN --pthashParameter 3.95 --numKeys 10M   --numQueries 20M --iterations 2 | tee --append figure-5.txt
./ComparisonN --pthashParameter 4.30 --numKeys 30M   --numQueries 20M --iterations 2 | tee --append figure-5.txt
./ComparisonN --pthashParameter 4.45 --numKeys 100M  --numQueries 20M --iterations 2 | tee --append figure-5.txt

# Build plot
cd /opt/sichash/scripts
cp /opt/sichash/build/figure-5.txt figure-5.txt
/opt/sqlplot-tools/build/src/sqlplot-tools figure-5.tex
pdflatex figure-5.tex
pdflatex figure-5.tex
cp figure-5.pdf /opt/dockerVolume
cp figure-5.txt /opt/dockerVolume

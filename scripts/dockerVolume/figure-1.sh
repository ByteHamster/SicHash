#!/bin/bash

# Run benchmark
cd /opt/sichash/build
function repeat() {
    repetitions=$1
    shift
    for i in $(seq "$repetitions"); do
      $@
    done
}

function runMultipleM () {
  repeat 2000 ./MaxLoadFactor -m 500  $@
  repeat  500 ./MaxLoadFactor -m 5k   $@
  repeat  100 ./MaxLoadFactor -m 50k  $@
  repeat   20 ./MaxLoadFactor -m 500k $@
}

runMultipleM --percentage4 100 | tee figure-1.txt
runMultipleM --percentage2 50 --percentage4 0.00 --percentage8 0.50 | tee --append figure-1.txt
runMultipleM --percentage2 33 --percentage4 0.34 --percentage8 0.33 | tee --append figure-1.txt
runMultipleM --percentage2 10 --percentage4 0.80 --percentage8 0.10 | tee --append figure-1.txt

# Build plot
cd /opt/sichash/scripts
cp /opt/sichash/build/figure-1.txt figure-1.txt
/opt/sqlplot-tools/build/src/sqlplot-tools figure-1.tex
pdflatex figure-1.tex
pdflatex figure-1.tex
cp figure-1.pdf /opt/dockerVolume
cp figure-1.txt /opt/dockerVolume

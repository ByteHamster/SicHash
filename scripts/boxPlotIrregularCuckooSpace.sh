#!/bin/bash
hostname
strings MaxLoadFactor | grep fPIC

function repeat() {
    repetitions=$1
    shift
    # shellcheck disable=SC2034
    for i in $(seq "$repetitions"); do
      # shellcheck disable=SC2068
      $@
    done
}

# shellcheck disable=SC2068
function runMultipleM () {
  repeat 500 ./MaxLoadFactor -m 500  $@
  repeat 500 ./MaxLoadFactor -m 5k   $@
  repeat 500 ./MaxLoadFactor -m 50k  $@
  repeat 500 ./MaxLoadFactor -m 500k $@
}

runMultipleM --percentage4 100
# x*1 + y*2 + (1-x-y)*3 = 2
runMultipleM --percentage2 50 --percentage4  0 --percentage8 50
runMultipleM --percentage2 33 --percentage4 34 --percentage8 33
runMultipleM --percentage2 10 --percentage4 80 --percentage8 10

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
# x*2 + y*4 + (1-x-y)*8 = 4
runMultipleM --percentage2 67 --percentage4  0 --percentage8 33
runMultipleM --percentage2 50 --percentage4 25 --percentage8 25

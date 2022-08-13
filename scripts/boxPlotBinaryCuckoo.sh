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

repeat 2000 ./MaxLoadFactor -m 500  --percentage2 100
repeat 2000 ./MaxLoadFactor -m 1k   --percentage2 100
repeat 2000 ./MaxLoadFactor -m 5k   --percentage2 100
repeat 2000 ./MaxLoadFactor -m 10k  --percentage2 100
repeat 2000 ./MaxLoadFactor -m 50k  --percentage2 100
repeat 2000 ./MaxLoadFactor -m 100k --percentage2 100
repeat 2000 ./MaxLoadFactor -m 1M   --percentage2 100

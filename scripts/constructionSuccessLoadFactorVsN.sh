#!/bin/bash
hostname
strings ConstructionSuccess | grep fPIC

function benchmark() {
  M=$1
  iterations=$2
  fromN=$(printf "%.0f\n" "$(echo "0.4 * $M" | bc -l)")
  middleN=$(printf "%.0f\n" "$(echo "0.5 * $M" | bc -l)")
  toN=$(printf "%.0f\n" "$(echo "0.6 * $M" | bc -l)")
  stepN=$(printf "%.0f\n" "$(echo "0.003 * $M" | bc -l)")

  # Go downwards from 0.5 until it always succeeds
  success100inARow=0
  for N in $(seq "$middleN" "-$stepN" "$fromN"); do
    result=$(./ConstructionSuccess --numKeys "$N" --numLocations "$M" --iterations "$iterations" --percentage2 100)

    echo "$result"
    if [[ $result == *"success=$iterations "* ]]; then
      success100inARow=$((success100inARow + 1))
    else
      success100inARow=0
    fi
    if [[ $success100inARow == 10 ]]; then
      break
    fi
  done

  # Go upwards from 0.5 until it never succeeds
  success0inARow=0
  for N in $(seq "$middleN" "$stepN" "$toN"); do
    result=$(./ConstructionSuccess --numKeys "$N" --numLocations "$M" --iterations "$iterations" --percentage2 100)

    echo "$result"
    if [[ $result == *"success=0 "* ]]; then
      success0inARow=$((success0inARow + 1))
    else
      success0inARow=0
    fi
    if [[ $success0inARow == 10 ]]; then
      break
    fi
  done
}

benchmark $((2 ** 12)) 4000
benchmark $((2 ** 15)) 800
benchmark $((2 ** 18)) 100
benchmark $((2 ** 22)) 20

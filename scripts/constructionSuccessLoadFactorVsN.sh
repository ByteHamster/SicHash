#!/bin/bash
hostname
strings ConstructionSuccess | grep fPIC

Ms=(
  $((2 ** 12))
  $((2 ** 15))
  $((2 ** 18))
  $((2 ** 22))
)

for M in "${Ms[@]}"; do
  fromN=$(printf "%.0f\n" "$(echo "0.4 * $M" | bc -l)")
  toN=$(printf "%.0f\n" "$(echo "0.6 * $M" | bc -l)")
  stepN=$(printf "%.0f\n" "$(echo "0.001 * $M" | bc -l)")

  for N in $(seq "$fromN" "$stepN" "$toN"); do
    ./ConstructionSuccess --numKeys "$N" --numLocations "$M" --iterations 40 --percentage2 100
  done
done

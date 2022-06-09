#!/bin/bash
hostname
strings MaxLoadFactor | grep fPIC

M=1000000

./MaxLoadFactor -m $M --percentage2 100 --name "Standard-d-ary"
./MaxLoadFactor -m $M --percentage3 100 --name "Standard-d-ary"
./MaxLoadFactor -m $M --percentage4 100 --name "Standard-d-ary"
./MaxLoadFactor -m $M --percentage5 100 --name "Standard-d-ary"
./MaxLoadFactor -m $M --percentage6 100 --name "Standard-d-ary"
./MaxLoadFactor -m $M --percentage7 100 --name "Standard-d-ary"
./MaxLoadFactor -m $M --percentage8 100 --name "Standard-d-ary"

function iterate() {
  for a in $(seq 0 "$3" 100); do
    b=$((100 - a))
    ./MaxLoadFactor -m "$M" "--percentage$1" "$a" "--percentage$2" "$b" --name "Mix-$1/$2"
  done
}
iterate 2 4 2
iterate 2 8 1
iterate 2 3 5
iterate 2 6 1

for i in $(seq 25 3 85); do
  for j in $(seq 20 3 45); do
    k=$((100 - i - j))
    if [[ $((i + j)) -gt '100' ]]; then
        continue
    fi
    ./MaxLoadFactor -m "$M" --percentage2 "$i" --percentage4 "$j" --percentage8 "$k" --name "Different-2/4/8"
  done
done

function solve() {
  python - <<END
from sympy import symbols, nsolve
import math
e = math.e
x = symbols('x')
print(nsolve($1, 1, verify=False))
END
}

for k10 in $(seq 201 3 400); do
  k=$(echo "0.01 * $k10" | bc -l)
  t=$(solve "x*(e**x-1)/(e**x-1-x) - $k")
  c=$(solve "$t/($k*(1-e**-$t)**($k-1)) - x")
  echo "RESULT name=Theory loadFactor=$c averageHashFunctions=$k"
done

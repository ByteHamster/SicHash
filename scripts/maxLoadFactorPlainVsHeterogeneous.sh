#!/bin/bash
hostname
strings ConstructionSuccess | grep fPIC

checkMaxLoadFactor() {
  M="$1"
  N="$2" # Initial N to test
  params="$3"
  name="$4"
  success=""
  while [ "$success" = "" ]; do
    result=$(./ConstructionSuccess --numKeys "$N" --numLocations "$M" --iterations 20 --name "$name" $params)
    echo "$result"
    success=$(echo "$result" | grep "success=20")
    N=$(printf "%.0f\n" "$(echo "$N - 0.002 * $M" | bc -l)")
  done
}

checkMaxLoadFactor 100000 53000 "--percentage2 100" "Standard-d-ary"
checkMaxLoadFactor 100000 94000 "--percentage3 100" "Standard-d-ary"
checkMaxLoadFactor 100000 100000 "--percentage4 100" "Standard-d-ary"
checkMaxLoadFactor 100000 100000 "--percentage5 100" "Standard-d-ary"
checkMaxLoadFactor 100000 100000 "--percentage6 100" "Standard-d-ary"
checkMaxLoadFactor 100000 100000 "--percentage7 100" "Standard-d-ary"
checkMaxLoadFactor 100000 100000 "--percentage8 100" "Standard-d-ary"

for a in $(seq 10 5 95); do
  b=$((100 - a))
  checkMaxLoadFactor 100000 100000 "--percentage2 $a --percentage4 $b" "Mix-2/4"
  checkMaxLoadFactor 100000 100000 "--percentage2 $a --percentage8 $b" "Mix-2/8"
done


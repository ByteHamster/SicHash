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
    N=$(printf "%.0f\n" "$(echo "$N - 0.001 * $M" | bc -l)")
  done
}

factor="00"

checkMaxLoadFactor 100$factor 53$factor "--percentage2 100" "Standard-d-ary"
checkMaxLoadFactor 100$factor 94$factor "--percentage3 100" "Standard-d-ary"
checkMaxLoadFactor 100$factor 100$factor "--percentage4 100" "Standard-d-ary"
checkMaxLoadFactor 100$factor 100$factor "--percentage5 100" "Standard-d-ary"
checkMaxLoadFactor 100$factor 100$factor "--percentage6 100" "Standard-d-ary"
checkMaxLoadFactor 100$factor 100$factor "--percentage7 100" "Standard-d-ary"
checkMaxLoadFactor 100$factor 100$factor "--percentage8 100" "Standard-d-ary"

for a in $(seq 2 2 98); do
  b=$((100 - a))
  checkMaxLoadFactor 100$factor 100$factor "--percentage2 $a --percentage4 $b" "Mix-2/4"
  checkMaxLoadFactor 100$factor 100$factor "--percentage2 $a --percentage8 $b" "Mix-2/8"
done

for i in $(seq 25 3 85); do
  for j in $(seq 20 3 45); do
    k=$((100 - i - j))
    if [[ $((i + j)) -gt '100' ]]; then
        continue
    fi
    checkMaxLoadFactor 100$factor 100$factor "--percentage2 $i --percentage4 $j --percentage8 $k" "Different-2/4/8"
  done
done

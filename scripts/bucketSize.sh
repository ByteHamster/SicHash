#!/bin/bash
hostname
strings SicHashBenchmark | grep fPIC

for i in $(seq 25 5 85); do
  for j in $(seq 20 5 65); do
    k=$((100 - i - j))
    if [[ $((i + j)) -gt '100' ]]; then
        continue
    fi
    ./SicHashBenchmark --numKeys 5M --bucketSize 100 --loadFactor 0.9 --percentage2 "$j" --percentage4 "$k"
    ./SicHashBenchmark --numKeys 5M --bucketSize 500 --loadFactor 0.9 --percentage2 "$j" --percentage4 "$k"
    ./SicHashBenchmark --numKeys 5M --bucketSize 1000 --loadFactor 0.9 --percentage2 "$j" --percentage4 "$k"
    ./SicHashBenchmark --numKeys 5M --bucketSize 2000 --loadFactor 0.9 --percentage2 "$j" --percentage4 "$k"
    ./SicHashBenchmark --numKeys 5M --bucketSize 5000 --loadFactor 0.9 --percentage2 "$j" --percentage4 "$k"
  done
done

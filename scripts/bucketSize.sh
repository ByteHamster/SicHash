#!/bin/bash
hostname
strings SicHashBenchmark | grep fPIC

for bucketSize in $(seq 700 100 5000); do
  ./SicHashBenchmark --numKeys 20M --bucketSize $bucketSize --loadFactor 0.95 --percentage2 46 --percentage4 32
  ./SicHashBenchmark --numKeys 20M --bucketSize $bucketSize --loadFactor 0.85 --percentage2 61 --percentage4 38
done

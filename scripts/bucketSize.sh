#!/bin/bash
hostname
strings SicHashBenchmark | grep fPIC

for bucketSize in $(seq 500 500 10000); do
  ./SicHashBenchmark --numKeys 5M --bucketSize $bucketSize --loadFactor 0.95 --percentage2 46 --percentage4 32
done

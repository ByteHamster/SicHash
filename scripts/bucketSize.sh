#!/bin/bash
hostname
strings PhfBenchmark | grep fPIC

for bucketSize in $(seq 2000 12000 2000); do
  ./PhfBenchmark --numKeys 3M --bucketSize $bucketSize --repetitions 3 --loadFactor 0.95
done

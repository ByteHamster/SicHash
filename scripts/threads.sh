#!/bin/bash
hostname
strings ComparisonThreads | grep fPIC

for i in $(seq 0 8 64); do
  threads=$i
  if [[ $threads == 0 ]]; then
    threads=1
  fi
  ./ComparisonThreads --numKeys 10M --numThreads "$threads" --iterations 2 --numQueries 0
done

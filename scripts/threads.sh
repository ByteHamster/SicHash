#!/bin/bash
hostname
strings ComparisonN | grep fPIC

for i in $(seq 64); do
  ./ComparisonN --pthashParameter 3.95 --numKeys 10M --numThreads "$i" --numQueries 0
done

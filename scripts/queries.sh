#!/bin/bash
hostname
strings ComparisonN | grep fPIC

# PTHash with the same parameter generates data structures with different space efficiency when varying N.
# Parameter selected so that it achieves roughly 1.8 bits in each measurement.
./ComparisonN --pthashParameter 3.35 --numKeys 1M    --iterations 4
./ComparisonN --pthashParameter 3.55 --numKeys 2M    --iterations 3
./ComparisonN --pthashParameter 3.75 --numKeys 5M    --iterations 3
./ComparisonN --pthashParameter 3.95 --numKeys 10M   --iterations 2
./ComparisonN --pthashParameter 4.10 --numKeys 20M   --iterations 2
./ComparisonN --pthashParameter 4.30 --numKeys 50M
./ComparisonN --pthashParameter 4.45 --numKeys 100M
./ComparisonN --pthashParameter 4.60 --numKeys 200M

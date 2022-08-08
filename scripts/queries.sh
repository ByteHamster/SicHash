#!/bin/bash
hostname
strings ComparisonN | grep fPIC

# pthash with the same parameter generates data structures with different space efficiency when varying N
# Parameter selected so that it achieves roughly the same amount of space.
./ComparisonN --pthashParameter 3.0 --numKeys 1M    --iterations 5
./ComparisonN --pthashParameter 3.5 --numKeys 2M    --iterations 5
./ComparisonN --pthashParameter 3.7 --numKeys 5M    --iterations 4
./ComparisonN --pthashParameter 4.0 --numKeys 10M   --iterations 3
./ComparisonN --pthashParameter 4.5 --numKeys 20M   --iterations 3
./ComparisonN --pthashParameter 5.0 --numKeys 50M
./ComparisonN --pthashParameter 5.5 --numKeys 100M
./ComparisonN --pthashParameter 6.0 --numKeys 200M

./ComparisonN --numKeys 1M    --mphfWbpmOnly
./ComparisonN --numKeys 2M    --mphfWbpmOnly
./ComparisonN --numKeys 5M    --mphfWbpmOnly
./ComparisonN --numKeys 10M   --mphfWbpmOnly

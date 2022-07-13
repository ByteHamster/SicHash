#!/bin/bash
hostname
strings ComparisonN | grep fPIC

./ComparisonN --numKeys 1M    --iterations 3
./ComparisonN --numKeys 1.1M  --iterations 3
./ComparisonN --numKeys 5M    --iterations 2
./ComparisonN --numKeys 10M
./ComparisonN --numKeys 11M
./ComparisonN --numKeys 50M
./ComparisonN --numKeys 100M
./ComparisonN --numKeys 110M
./ComparisonN --numKeys 500M
./ComparisonN --numKeys 1G

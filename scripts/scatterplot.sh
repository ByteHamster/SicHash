#!/bin/bash
hostname
strings Comparison | grep fPIC

echo "Keys: $1"

./Comparison --numKeys "$1" --recsplitOnly
./Comparison --numKeys "$1" --loadFactor 0.8
./Comparison --numKeys "$1" --loadFactor 0.85
./Comparison --numKeys "$1" --loadFactor 0.9
./Comparison --numKeys "$1" --loadFactor 0.95
./Comparison --numKeys "$1" --loadFactor 0.97
./Comparison --numKeys "1M" --mphfWbpmOnly

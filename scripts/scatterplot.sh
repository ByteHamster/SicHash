#!/bin/bash
hostname
strings Comparison | grep fPIC

./Comparison --numKeys 5M --loadFactor 0.8
./Comparison --numKeys 5M --loadFactor 0.85
./Comparison --numKeys 5M --loadFactor 0.9
./Comparison --numKeys 5M --loadFactor 0.95
./Comparison --numKeys 5M --loadFactor 0.97
./Comparison --numKeys 5M --recsplitOnly
./Comparison --numKeys 1M --mphfWbpmOnly

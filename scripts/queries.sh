#!/bin/bash
hostname
strings Comparison | grep fPIC

./ComparisonN --numKeys 1M --loadFactor 0.95 --recsplit
./ComparisonN --numKeys 5M --loadFactor 0.95 --recsplit
./ComparisonN --numKeys 10M --loadFactor 0.95 --recsplit
./ComparisonN --numKeys 50M --loadFactor 0.95 --recsplit
./ComparisonN --numKeys 100M --loadFactor 0.95 --recsplit

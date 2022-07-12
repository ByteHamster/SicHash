#!/bin/bash
hostname
strings Comparison | grep fPIC

./Comparison --numKeys 1M --loadFactor 0.95 --recsplit
./Comparison --numKeys 10M --loadFactor 0.95 --recsplit
./Comparison --numKeys 100M --loadFactor 0.95 --recsplit

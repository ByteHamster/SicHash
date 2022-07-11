#!/bin/bash
hostname
strings Comparison | grep fPIC

./Comparison --numKeys 5M --loadFactor 0.8 --recsplit
./Comparison --numKeys 5M --loadFactor 0.85
./Comparison --numKeys 5M --loadFactor 0.9
./Comparison --numKeys 5M --loadFactor 0.95
./Comparison --numKeys 5M --loadFactor 0.97

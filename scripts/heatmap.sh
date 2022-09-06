#!/bin/bash
hostname
strings Comparison | grep fPIC

./Comparison --numKeys 5M --numQueries 0 --loadFactor 0.7  --sichash --pthash --chd
./Comparison --numKeys 5M --numQueries 0 --loadFactor 0.75 --sichash --pthash --chd
./Comparison --numKeys 5M --numQueries 0 --loadFactor 0.8  --sichash --pthash --chd
./Comparison --numKeys 5M --numQueries 0 --loadFactor 0.85 --sichash --pthash --chd
./Comparison --numKeys 5M --numQueries 0 --loadFactor 0.9  --sichash --pthash --chd
./Comparison --numKeys 5M --numQueries 0 --loadFactor 0.95 --sichash --pthash --chd
./Comparison --numKeys 5M --numQueries 0 --loadFactor 1.0  --recsplit --bbhash

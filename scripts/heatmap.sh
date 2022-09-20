#!/bin/bash
hostname
strings Comparison | grep fPIC

./Comparison --numKeys 5M --numQueries 0 --loadFactor 0.800 --sichash --pthash --chd
./Comparison --numKeys 5M --numQueries 0 --loadFactor 0.825 --sichash --pthash --chd
./Comparison --numKeys 5M --numQueries 0 --loadFactor 0.850 --sichash --pthash --chd
./Comparison --numKeys 5M --numQueries 0 --loadFactor 0.875 --sichash --pthash --chd
./Comparison --numKeys 5M --numQueries 0 --loadFactor 0.900 --sichash --pthash --chd
./Comparison --numKeys 5M --numQueries 0 --loadFactor 0.925 --sichash --pthash --chd
./Comparison --numKeys 5M --numQueries 0 --loadFactor 0.950 --sichash --pthash --chd
./Comparison --numKeys 5M --numQueries 0 --loadFactor 0.975 --sichash --pthash --chd
./Comparison --numKeys 5M --numQueries 0 --loadFactor 1.000 --recsplit --bbhash

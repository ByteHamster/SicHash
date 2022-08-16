#!/bin/bash
hostname
strings Comparison | grep fPIC

./Comparison --numKeys 20M --numQueries 0 --loadFactor 0.8   --sichash --pthash --chd --bdz --chm
./Comparison --numKeys 20M --numQueries 0 --loadFactor 0.85  --sichash --pthash --chd --bdz --chm
./Comparison --numKeys 20M --numQueries 0 --loadFactor 0.9   --sichash --pthash --chd --bdz --chm
./Comparison --numKeys 20M --numQueries 0 --loadFactor 0.95  --sichash --pthash --chd --bdz --chm
./Comparison --numKeys 20M --numQueries 0 --loadFactor 0.97  --sichash --pthash --chd --bdz --chm
./Comparison --numKeys 20M --numQueries 0 --recsplit --bbhash

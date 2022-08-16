#!/bin/bash
hostname
strings Comparison | grep fPIC

./Comparison --numKeys 5M --numQueries 0 --loadFactor 0.8   --sichash --pthash --chd --bdz --bmz --chm --fch
./Comparison --numKeys 5M --numQueries 0 --loadFactor 0.85  --sichash --pthash --chd --bdz --bmz --chm --fch
./Comparison --numKeys 5M --numQueries 0 --loadFactor 0.9   --sichash --pthash --chd --bdz --bmz --chm --fch
./Comparison --numKeys 5M --numQueries 0 --loadFactor 0.95  --sichash --pthash --chd --bdz --bmz --chm --fch
./Comparison --numKeys 5M --numQueries 0 --loadFactor 0.97  --sichash --pthash --chd --bdz --bmz --chm --fch
./Comparison --numKeys 5M --numQueries 0 --recsplit --mphfWbpm --bbhash

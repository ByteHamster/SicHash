#!/bin/bash
hostname
strings Comparison | grep fPIC

./Comparison --numKeys 5M --loadFactor 0.8   --bbhash --sichash --pthash --chd --bdz --bmz --chm --fch
./Comparison --numKeys 5M --loadFactor 0.85  --bbhash --sichash --pthash --chd --bdz --bmz --chm --fch
./Comparison --numKeys 5M --loadFactor 0.9   --bbhash --sichash --pthash --chd --bdz --bmz --chm --fch
./Comparison --numKeys 5M --loadFactor 0.95  --bbhash --sichash --pthash --chd --bdz --bmz --chm --fch
./Comparison --numKeys 5M --loadFactor 0.97  --bbhash --sichash --pthash --chd --bdz --bmz --chm --fch
./Comparison --numKeys 5M --recsplit --mphfWbpm

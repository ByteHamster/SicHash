#!/bin/bash
hostname
strings SicHashBenchmark | grep fPIC

function runBenchmark() {
  export averageTries
  exec 5>&1
  R=$(./SicHashBenchmark --numKeys 5M --bucketSize "$1" --loadFactor 0.9 --percentage2 "$2" --percentage4 "$3"|tee >(cat - >&5))
  averageTries=$(echo "$R" | sed -n 's/\(.*\)averageTries=\([0-9.]*\) \(.*\)/\2/p')
  if [[ "$averageTries" == "" ]]; then
    averageTries="1000"
  fi
}

for i in $(seq 40 2 85); do
  for j in $(seq 15 2 60); do
    if [[ $((i + j)) -gt '100' ]]; then
        continue
    fi
    echo "Trying $i $j"
    # The larger the hash table for a single configuration, the more tries are needed.
    # When a small bucket size already times out, a larger one with the same configuration will probably time out as well.
    # Therefore, continue with the next loop iteration as soon as one of the methods times out.
    timeoutTries=50

    runBenchmark 100 "$i" "$j"
    if (( $(echo "$averageTries > $timeoutTries" | bc -l) )); then continue; fi
    runBenchmark 200 "$i" "$j"
    if (( $(echo "$averageTries > $timeoutTries" | bc -l) )); then continue; fi
    runBenchmark 500 "$i" "$j"
    if (( $(echo "$averageTries > $timeoutTries" | bc -l) )); then continue; fi
    runBenchmark 1000 "$i" "$j"
    if (( $(echo "$averageTries > $timeoutTries" | bc -l) )); then continue; fi
    runBenchmark 2000 "$i" "$j"
    if (( $(echo "$averageTries > $timeoutTries" | bc -l) )); then continue; fi
    runBenchmark 5000 "$i" "$j"
    if (( $(echo "$averageTries > $timeoutTries" | bc -l) )); then continue; fi
    runBenchmark 10000 "$i" "$j"
    if (( $(echo "$averageTries > $timeoutTries" | bc -l) )); then continue; fi
    runBenchmark 20000 "$i" "$j"
    if (( $(echo "$averageTries > $timeoutTries" | bc -l) )); then continue; fi
    runBenchmark 50000 "$i" "$j"
    if (( $(echo "$averageTries > $timeoutTries" | bc -l) )); then continue; fi
    runBenchmark 100000 "$i" "$j"
  done
done

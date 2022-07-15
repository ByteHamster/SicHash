#!/bin/bash
hostname
strings ComparisonN | grep fPIC

./ComparisonN --numKeys 1M    --iterations 5
./ComparisonN --numKeys 2M    --iterations 5
./ComparisonN --numKeys 5M    --iterations 4
./ComparisonN --numKeys 10M   --iterations 3
./ComparisonN --numKeys 20M   --iterations 3
./ComparisonN --numKeys 50M
./ComparisonN --numKeys 100M
./ComparisonN --numKeys 200M
./ComparisonN --numKeys 500M
./ComparisonN --numKeys 1G

./ComparisonN --numKeys 1M    --mphfWbpmOnly
./ComparisonN --numKeys 2M    --mphfWbpmOnly
./ComparisonN --numKeys 5M    --mphfWbpmOnly
./ComparisonN --numKeys 10M   --mphfWbpmOnly
./ComparisonN --numKeys 20M   --mphfWbpmOnly

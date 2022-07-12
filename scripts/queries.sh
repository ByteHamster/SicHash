#!/bin/bash
hostname
strings Comparison | grep fPIC

./ComparisonN --numKeys 1M
./ComparisonN --numKeys 5M
./ComparisonN --numKeys 10M
./ComparisonN --numKeys 50M
./ComparisonN --numKeys 100M

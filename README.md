# SicHash

A (Minimum) Perfect Hash Function based on irregular cuckoo hashing and retrieval.

### Building the examples

```
git clone --recursive git@github.com:ByteHamster/SicHash.git
mkdir SicHash/build
cd SicHash/build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j
```

### Library usage

Add the following to your `CMakeLists.txt`.

```
add_subdirectory(path/to/SicHash)
target_link_libraries(YourTarget PRIVATE SicHash)
```

### Competitors

For benchmarking, this repository contains a number of competitors.
When simply adding the `SicHash` target to your project, those are not included.

### License

This code is licensed under the [GPLv3](/LICENSE).
If you use the project in an academic context or publication, please cite our paper.

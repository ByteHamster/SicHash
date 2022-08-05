# SicHash

A (Minimum) Perfect Hash Function based on irregular cuckoo hashing, retrieval, and overloading.

### Building the examples and benchmarks

```
mkdir SicHash/build
cd SicHash/build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j
```

### Benchmarks in Docker Container
We provide a docker file to reproduce a small number of experiments inside a docker container.

```
cd scripts
sudo docker-compose up
```

The resulting measurements are then available in the `scripts/dockerVolume` folder.
Building and running the container on an average PC takes about 90 minutes.

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

# SicHash

A (Minimum) Perfect Hash Function based on irregular cuckoo hashing, retrieval, and overloading.

### Building the examples and benchmarks

The setup of the development machine and plot generation tools are rather inconventient, so we provide a docker file for easy reproducibility.
While the experiments in our paper are run without Docker, we recommend using the docker file for easier setup.
If you want to run the experiments outside a docker container, you can use the Dockerfile as a reference for setting up the tools.

To build the docker image with a pre-compiled SicHash install and all benchmark utilities, run the following command.
Building the container takes about 5 minutes.

```
docker build -t sichash .
```

The run scripts we provide are simplified versions of the plots from our paper.
They run fewer iterations and output fewer data points, but run faster.
You can modify the scripts in `scripts/dockerVolume` without re-building the container, but if you want to modify the c++ code, you need to re-build it.
Different experiments can be started by changing the launch command of the docker image.

| Figure in paper | Launch command                | Estimated runtime  |
| :-------------- | :---------------------------- | :----------------- |
| 1               | /opt/dockerVolume/figure-1.sh | 10 minutes         |
| 5               | /opt/dockerVolume/figure-5.sh | 15 minutes         |
| 6               | /opt/dockerVolume/figure-6.sh |          |

```
docker run --interactive --tty -v "$(pwd)/scripts/dockerVolume:/opt/dockerVolume" sichash /opt/dockerVolume/figure-6.sh
```

The output pdf files containing the plots are stored in the `scripts/dockerVolume` folder.

### Library usage

Clone (with submodules) this repo and add the following to your `CMakeLists.txt`.
For benchmarking, this repository contains a number of competitors.
When simply adding the `SicHash` target to your project, those are not compiled.

```
add_subdirectory(path/to/SicHash)
target_link_libraries(YourTarget PRIVATE SicHash)
```

### License

This code is licensed under the [GPLv3](/LICENSE).
If you use the project in an academic context or publication, please cite our paper.

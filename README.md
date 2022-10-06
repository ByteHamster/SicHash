# SicHash

A (Minimum) Perfect Hash Function based on irregular cuckoo hashing, retrieval, and overloading.

[<img src="https://raw.githubusercontent.com/ByteHamster/SicHash/main/plots.png" alt="Plots preview">](https://arxiv.org/pdf/2210.01560)

### Library Usage

Clone (with submodules) this repo and add the following to your `CMakeLists.txt`.
For benchmarking, this repository contains a number of competitors.
When simply adding the `SicHash` target to your project, those are not compiled.

```
add_subdirectory(path/to/SicHash)
target_link_libraries(YourTarget PRIVATE SicHash)
```

### Reproducing Experiments

This repository contains the source code and our reproducibility artifacts for SicHash.
Due to the plethora of dependencies required by our competitors, we provide an easy to use Docker image to quickly reproduce our results.
Alternatively, you can look at the `Dockerfile` to see all libraries, tools, and commands necessary to compile SicHash and its competitors.

#### Building the Docker Image

Run the following command to build the Docker image.
Building the image takes about 5 minutes, as some packages (including LaTeX for the plots) have to be installed.

```bash
docker build -t sichash --no-cache .
```

Some compiler warnings (red) are expected when building competitors and will not prevent building the image or running the experiments.
Please ignore them!

#### Running the Experiments
Due to the long total running time of all experiments in our paper, we provide run scripts for a slightly simplified version of the experiments.
They run fewer iterations and output fewer data points.

You can modify the benchmarks scripts in `scripts/dockerVolume` if you want to change the number of runs or data points.
This does not require the Docker image to recompile.
Different experiments can be started by using the following command:

```bash
docker run --interactive --tty -v "$(pwd)/scripts/dockerVolume:/opt/dockerVolume" sichash /opt/dockerVolume/figure-<number>.sh
```

`<number>` should be either `1`, `5` or `6`, depending on the experiment you want to run.
The number also refers to the figure in the paper.

| Figure in paper | Launch command                | Estimated runtime  |
| :-------------- | :---------------------------- | :----------------- |
| 1               | /opt/dockerVolume/figure-1.sh | 10 minutes         |
| 5               | /opt/dockerVolume/figure-5.sh | 20 minutes         |
| 6               | /opt/dockerVolume/figure-6.sh | 45 minutes         |

The resulting plots can be found in `scripts/dockerVolume` and are called `figure-<number>.pdf`.

### License

This code is licensed under the [GPLv3](/LICENSE).
If you use the project in an academic context or publication, please cite our paper:

```
@article{sichash2022,
  author    = {Lehmann, Hans-Peter and Sanders, Peter and Walzer, Stefan},
  title     = {SicHash -- Small Irregular Cuckoo Tables for Perfect Hashing},
  publisher = {arXiv},
  year      = {2022},
  doi       = {10.48550/ARXIV.2210.01560}
}
```

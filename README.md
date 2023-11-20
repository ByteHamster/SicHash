# SicHash

A (Minimal) Perfect Hash Function based on irregular cuckoo hashing, retrieval, and overloading.

## Construction performance (n = 5 million)

[<img src="https://raw.githubusercontent.com/ByteHamster/SicHash/main/plots-construction.png" alt="Plots preview">](https://arxiv.org/pdf/2210.01560)

## Query performance

[<img src="https://raw.githubusercontent.com/ByteHamster/SicHash/main/plots-query.png" alt="Plots preview">](https://arxiv.org/pdf/2210.01560)

### Library Usage

Clone (with submodules) this repo and add the following to your `CMakeLists.txt`.

```
add_subdirectory(path/to/SicHash)
target_link_libraries(YourTarget PRIVATE SicHash)
```

### Reproducing Experiments

This repository contains the source code and our reproducibility artifacts for the benchmarks specific to SicHash.
Benchmarks that compare SicHash to competitors are available in a different repository: https://github.com/ByteHamster/MPHF-Experiments

We provide an easy to use Docker image to quickly reproduce our results.
Alternatively, you can look at the `Dockerfile` to see all libraries, tools, and commands necessary to compile SicHash.

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
docker run --interactive --tty -v "$(pwd)/scripts/dockerVolume:/opt/dockerVolume" sichash /opt/dockerVolume/figure-1.sh
```

The number also refers to the figure in the paper.

| Figure in paper | Launch command                | Estimated runtime  |
| :-------------- | :---------------------------- | :----------------- |
| 1               | /opt/dockerVolume/figure-1.sh | 10 minutes         |

The resulting plots can be found in `scripts/dockerVolume` and are called `figure-<number>.pdf`.
More experiments comparing SicHash with competitors can be found in a different repository: https://github.com/ByteHamster/MPHF-Experiments

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

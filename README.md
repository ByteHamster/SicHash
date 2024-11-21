# SicHash

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
![Build status](https://github.com/ByteHamster/SicHash/actions/workflows/build.yml/badge.svg)

A perfect hash function (PHF) maps a set S of n keys to the first m integers without collisions.
It is called _minimal_ perfect (MPHF) if m=n.
Perfect hash functions have applications in databases, bioinformatics, and as a building block of various space-efficient data structures.

SicHash is a (minimal) perfect hash function based on irregular cuckoo hashing, retrieval, and overloading.
Each input key has a small number of choices for output positions.
Using cuckoo hashing, SicHash determines a mapping from each key to one of its choices,
such that there are no collisions between keys.
It then stores the mapping from keys to their candidate index space-efficiently using
the [BuRR](https://github.com/lorenzhs/BuRR) retrieval data structure.

SicHash offers a very good trade-off between construction performance, query performance, and space consumption.

### Library Usage

Clone this repo and add the following to your `CMakeLists.txt`.
Note that the repo has submodules, so either use `git clone --recursive` or `git submodule update --init --recursive`.

```
add_subdirectory(path/to/SicHash)
target_link_libraries(YourTarget PRIVATE SicHash)
```

Constructing a SicHash perfect hash function is then straightforward:

```cpp
std::vector<std::string> keys = {"abc", "def", "123", "456"};
sichash::SicHashConfig config;
sichash::SicHash<true> hashFunc(keys, config);
std::cout << hashFunc("abc") << std::endl;
```

### Construction Performance

[![Plots preview](https://raw.githubusercontent.com/ByteHamster/SicHash/main/plots-construction.png)](https://arxiv.org/pdf/2210.01560)

### Query Performance

[![Plots preview](https://raw.githubusercontent.com/ByteHamster/SicHash/main/plots-query.png)](https://arxiv.org/pdf/2210.01560)

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
If you use the project in an academic context or publication, please cite [our paper](https://doi.org/10.1137/1.9781611977561.ch15):

```
@inproceedings{lehmann2023sichash,
  author       = {Hans{-}Peter Lehmann and
                  Peter Sanders and
                  Stefan Walzer},
  title        = {SicHash - Small Irregular Cuckoo Tables for Perfect Hashing},
  booktitle    = {{ALENEX}},
  pages        = {176--189},
  publisher    = {{SIAM}},
  year         = {2023},
  doi          = {10.1137/1.9781611977561.CH15}
}
```

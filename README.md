# Naive CRC Analysis

A simple program to test how various CRC error detection capabilities depending on how much error is introducted. 

# Installation

First clone the project followed by updating the git submodules used in this project, which are required in order to compile the program.
```
git clone <this repo url>
cd <git workspace>
git submodule update --init --recursive
```

Afterward, it is as simple as follow the following commands.

```
mkdir build && cd build
cmake ..
make
```


```
CRCAnalysis --help
```

# Examples

```
CRCAnalysis --samples=100000000 --data-chunk-size=256 -m 1 --crc=xor8
```
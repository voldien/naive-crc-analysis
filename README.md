# Naive CRC Analysis Tool
[![Actions Build Status](https://github.com/voldien/crc-collision-anlysis/workflows/crc-collision-anlysis/badge.svg?branch=master)](https://github.com/voldien/crc-collision-anlysis/actions)
[![GitHub release](https://img.shields.io/github/release/voldien/crc-collision-anlysis.svg)](https://github.com/voldien/vecfield/releases)

A simple program to test how various CRC error detection capabilities depending on how much error is introduced. The program will create an n-number of
messages and an its respective error message with n number of bit errors. Afterward, compute the CRC for both the untouched message and the error message and check if they are not equal.
Because if equal, it means a collision has occurred, which would mean that the software using the CRC think it is a valid message. 

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



# Examples

```
CRCAnalysis --samples=100000000 --data-chunk-size=256 -m 1 --crc=xor8
```

The support command line options can be view with the following command.
```
CRCAnalysis --help
```
A example output can be the following.
```
Naive CRC Analysis
A simple program for checking error detection
Usage:
  CRCAnalysis [OPTION...]

  -v, --version                 Version information
  -h, --help                    helper information
  -c, --crc arg                 CRC (default: crc8)
  -p, --data-chunk-size arg     DataChunk (default: 5)
  -e, --error-correction        Perform Error Correction
  -s, --samples arg             Samples (default: 1000000)
  -t, --task-size arg           Task (default: 2000)
  -m, --number-of-bit-error arg
                                Number of bit error per message (default: 1)
```
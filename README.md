# Naive CRC Analysis Tool

[![crc-collision-anlysis](https://github.com/voldien/naive-crc-analysis/actions/workflows/cmake.yml/badge.svg)](https://github.com/voldien/naive-crc-analysis/actions/workflows/cmake.yml)
[![GitHub release](https://img.shields.io/github/release/voldien/naive-crc-analysis.svg)](https://GitHub.com/voldien/naive-crc-analysis/releases/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A simple program to test how various CRC error detection capabilities depending on how much error is introduced. The program will create an n-number of
messages and its respective error message with n number of bit errors. Afterward, compute the CRC for both the untouched message and the error message and check if they are not equal.
Because if equal, it means a collision has occurred, which would mean that the software using the CRC think it is a valid message.

## Installation

First clone the project followed by updating the git submodules used in this project, which are required in order to compile the program.

```bash
git clone <this repo url>
cd <git workspace>
git submodule update --init --recursive
```

Afterward, it is as simple as follow the following commands.

```bash
mkdir build && cd build
cmake ..
make
```

The executable can be located in the bin directory as *CRCAnalysis*.

## Examples

```bash
CRCAnalysis --samples=100000000 --message-data-size=256 -b 1 --crc=xor8
```

```bash
CRCAnalysis --samples=100000000 --message-data-size=256 --tasks=10000 -b 2 --crc=xor8
```

The support command line options can be view with the following command.

```bash
CRCAnalysis --help
```

An example output can be the following.

```bash
Naive CRC Analysis
A simple program for checking error detection
Usage:
  CRCAnalysis [OPTION...]

  -v, --version                Version information
  -h, --help                   helper information.
  -c, --crc arg                CRC Algorithm (default: crc8)
  -p, --message-data-size arg  Size of each messages in bytes. (default: 5)
  -e, --error-correction       Perform Error Correction.
  -s, --samples arg            Samples (default: 1000000)
  -t, --tasks arg              Task (default: 2000)
  -b, --nr-of-error-bits arg   Number of bits error added to each message. 
                               (default: 1)
  -f, --forever                Run it forever.
  -l, --show-crc-list          List of support CRC and Checksum Alg
  -P, --error-probability arg  Probability of adding error in data package. 
                               (default: 1)
```

### Supported CRC Algorithms

The list of supported CRC algorithms.

```bash
crc64
crc40_gsm
crc32_mpeg2
crc32_c
crc32
crc30
crc32_bzip2
crc24_lteb
crc24_ltea
crc24_flexrayb
xor8
crc24_flexraya
crc21_can
crc16_xmodem
crc16_t10dif
crc16_modbus
crc24_nrc
crc16_genibus
crc16_dectx
crc16_dectr
crc16_cms
xor16
crc16_cdma2000
crc16_usb
crc16_ccittfalse
crc24
crc16_arc
xor32
crc16_maxim
crc15_mpt1327
crc17_can
crc13_bcc
crc12_umts
crc16_mcrf4xx
crc12_dect
crc12_cdma2000
crc16_x25
crc16_buypass
crc11_nr
crc11
crc32_q
crc10_cdma2000
crc10
crc32_posix
crc8_wcdma
crc8_maxim
crc16_kermit
crc8_ebu
xor8_masked
crc16_dnp
crc8
crc15
crc6_nr
crc6_itu
crc8_lte
crc6_cmda2000b
crc7
crc6_cmda2000a
crc5_usb
crc5_itu
crc5_epc
crc4_itu
```

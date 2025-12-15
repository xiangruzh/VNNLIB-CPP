# VNN-LIB C++

A C++ library for parsing and manipulating neural network queries in the [VNN-LIB format](https://www.vnnlib.org/). 

## Features

Features include:
- Parsing: Convert a VNN-LIB file or string into a type-checked AST.
- Manipulations: 
    - Generic traversal of the AST.
    - Conversion of arithmetic expressions into a linear form.
    - Conversion of boolean expressions to Disjunctive Normal Form (DNF).
    - Conversion of query into the reachability format used in prior VNN-COMPs.

## Installation

You can either download the [latest release](https://github.com/VNNLIB/VNNLIB-CPP/releases) of the pre-built library or can follow the [CONTRIBUTING.md](https://github.com/VNNLIB/VNNLIB-CPP/blob/main/CONTRIBUTING.md) instructions
to build from source. Note if doning the latter then you must follow the instructions
to regenerate the parser if you are not on a Linux system.

## Basic Usage

```cpp
#include "VNNLib.h"

TQuery* query = parseQueryFile(std::string path);

for (auto assertion : query.assertions) {
    ...
}
```

## Version compatibility

| VNNLIB-C++ version | VNNLIB version |
| --- | --- |
| v1.0 | v2.0 |


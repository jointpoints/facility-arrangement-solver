# Developer tools for Facility Arrangement Solver

This folder contains convenience tools for developers of FAS.

## Compilation

You can use `Makefile` located in this folder to compile the program on your own. Before
running `make` make sure to properly set the following variables in `Makefile`:

* `CPLEX_PATH` - should point to the folder where CPLEX has been intalled.
* `MSVC_PATH` - should point to the folder where MSVC compiler has been installed
*(ignored on Linux)*.
* `WINKITS_PATH` - should point to the folder where Windows Kits have been installed
*(ignored on Linux)*.

##### Available `make` targets

* `make build` or `make` - compiles the program and creates an executable in a new folder
also containing all necessary files to run it.
* `make debug` - the same as `make build` but the executable also includes the debugging
information (e.g., to debug or profile the program later).

> **Note!** MSVC may produce some warning during compilation. These will appear because
MSVC is 
[still not fully compliant with C++20 standard](https://en.cppreference.com/w/cpp/compiler_support/20).

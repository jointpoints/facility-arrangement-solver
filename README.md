# Facility Arrangement Solver

## Brief

A tool to arrange *subjects* (such as departments, machines, etc.) within the given facility. Subjects are categorised into *groups*. Subjects within one group share common properties and are mutually interchangeable in the production pipeline. Workflow within the facility is modelled via interactions between different subjects. For more delails, see the Wiki pages.

## External dependencies

This tool relies on the following external packages:

* [CPLEX](https://www.ibm.com/products/ilog-cplex-optimization-studio)
	* Proprietary software by IBM.
	* Not supplied together with the source code in this repository, acquisition of a licensed copy on your own is required.

## Installation

Before you can use Facility Arrangement Solver, you will have to install:

* Python 3.8,
* `cplex` module for the respective version of Python.

##### 1. Installation of Python

Go to [the official web-site](https://www.python.org/downloads/) of Python and download the distributive of Python 3.8 that is compatible with your system. Complete the installation, add the path to the executable file to the PATH system variable.

##### 2. Installation of `cplex` module

Visit `/cplex/python/3.8/S` folder contained within your main CPLEX directory substituting `S` with your system name (will be the only folder in `/cplex/python/3.8`). In this folder, you will find a file called `setup.py`. Open a terminal in this folder and run

    python setup.py install

##### 3. Installation of Facility Arrangement Solver

Download the latest release of Facility Arrangement Solver, unpack the archive into whatever folder you prefer.

## Usage

Open a terminal in the folder where you unpacked the archive with Facility Arrangement Solver. Run

    python fas.py

For more tips, see the Wiki section of this repository.

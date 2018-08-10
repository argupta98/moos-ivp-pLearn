
# MOOS-IvP pLearn
# Author: Arjun Gupta

# Introduction

The moos-ivp-pLearn repository is an extension of MOOS-IvP, a powerful marine
autonomy library

# Directory Structure

The directory structure for the moos-ivp-extend is decribed below:

-bin: Directory for generated executable files
-build: Directory for build object files
-build.sh: Script for building moos-ivp-pLearn
-CMakeLists.txt: CMake configuration file for the project
-lib: Directory for generated library files
-README: Contains helpful information - (this file).
-src: Directory for source code
-pLearn: Directory for Reinforcement Learning 
-docs: Contains Documentation and Instructions for using pLearn toolbox
 


# Build Instructions

The pLearn toolbox has many dependencies, including MOOS-IvP and Python2.7.
Please view the docs directory for a detailed explanation of how to setup the build
environment before continuing.


# Linux and Mac Users

To build on Linux and Apple platforms, execute the build script within this
directory:

   $ ./build.sh

To build without using the supplied script, execute the following commands
within this directory:

   $ mkdir -p build
   $ cd build
   $ cmake ../
   $ make
   $ cd ..



# Windows Users

To build on Windows platform, open CMake using your favorite shortcut. Then 
set the source directory to be this directory and set the build directory
to the "build" directory inside this directory.

The source directory is typically next to the question:
   "Where is the source code?"

The build directory is typically next to the question:
   "Where to build the binaries?"

Alternatively, CMake can be invoked via the command line. However, you must
specify your gernerator. Use "cmake --help" for a list of generators and
additional help.


# Environment variables

The moos-ivp-extend binaries files should be added to your path to allow them
to be launched from pAntler. 

In order for generated IvP Behaviors to be recognized by the IvP Helm, you
should add the library directory to the "IVP_BEHAVIOR_DIRS" environment 
variable.



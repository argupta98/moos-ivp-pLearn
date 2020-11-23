
# MOOS-IvP pLearn
**Original Author: Arjun Gupta**
**Maintenance: Michael "Misha" Novitzky

## Introduction

The moos-ivp-pLearn repository is an extension of Project Aquaticus which is based on MOOS-IvP, a powerful marine
autonomy library.  This repository assumes that the moos-ivp and moos-ivp-aquaticus repositories have been built and applications available through system PATH variable.

## Directory Structure

The directory structure for the moos-ivp-extend is decribed below:

- **bin:**            Directory for generated executable files
- **build:**          Directory for build object files
- **build.sh:**       Script for building moos-ivp-pLearn
- **CMakeLists.txt:** CMake configuration file for the project
- **lib:**            Directory for generated library files
- **README.md:**         Contains helpful information - (this file).
- **src:**            Directory for source code
- **pLearn:**         Directory for Reinforcement Learning scripts
- **docs:**           Contains Documentation and Instructions for using pLearn toolbox
 


## Build Instructions

The pLearn toolbox has many dependencies, including MOOS-IvP, moos-ivp-aquaticus, and Python2.7.
Please view the [documentation](docs/Documentation/pLearn.pdf) for a detailed explanation of how to setup the build
environment before continuing.


### Linux and Mac Users

To build on Linux and Apple platforms, execute the build script within this
directory:
```
   $ ./build.sh
```
To build without using the supplied script, execute the following commands
within this directory:
```
   $ mkdir -p build
   $ mkdir lib
   $ cd build
   $ cmake ../
   $ make
   $ cd ..
```

## Environment variables

The moos-ivp-extend binaries files should be added to your path to allow them
to be launched from pAntler. 

In order for generated IvP Behaviors to be recognized by the IvP Helm, you
should add the library directory to the `IVP_BEHAVIOR_DIRS` environment 
variable.

## The following installation instructions were verified in Ubuntu 18.04 LTS

## Install MOOS-IvP
```
$ sudo apt update
$ sudo apt install -y libncurses-dev sudo subversion

$ svn co https://oceanai.mit.edu/svn/moos-ivp-aro/releases/moos-ivp-19.8.1 moos-ivp

$ sudo apt-get install g++ subversion xterm cmake libfltk1.3-dev freeglut3-dev libpng-dev libjpeg-dev libxft-dev libxinerama-dev libtiff5-dev


$ cd moos-ivp

$ ./build.sh
```

//update your .bashrc file to include the following variables
//update the variable User_name with your system's username
User_name=mikey 
PATH="/home/${User_name}/moos-ivp/bin:${PATH}"
IVP_BEHAVIOR_DIRS="/home/${User_name}/moos-ivp/lib:${IVP_BEHAVIOR_DIRS}"
export PATH
export IVP_BEHAVIOR_DIRS

//after saving the .bashrc file -- you should load it by typing:
$bash 


//verify moos-ivp installation
$ cd moos-ivp/ivp/missions/s1_alpha
$ pAntler alpha.moos
//Start simulation by clicking 'DEPLOY' on lower right of the GUI



## Install moos-ivp-aquaticus

cd ~

svn co https://oceanai.mit.edu/svn/moos-ivp-aquaticus-oai/trunk/ moos-ivp-aquaticus

cd moos-ivp-aquaticus

$./build.sh

//update your .bashrc file to include the following variables
//update the variable User_name with your system's username
User_name=mikey 
PATH="/home/${User_name}/moos-ivp-aquaticus/bin:${PATH}"
IVP_BEHAVIOR_DIRS="/home/${User_name}/moos-ivp-aquaticus/lib:${IVP_BEHAVIOR_DIRS}"
export PATH
export IVP_BEHAVIOR_DIRS

//after saving the .bashrc file -- you should load it by typing:
$bash 


//Test moos-ivp-aquaticus

$ cd ~/moos-ivp-aquaticus/missions/bots-only-example
$ ./launch_demo

//A gui should come up and the robots will start moving when you click on the 'DEPLOY' button in the lower right.



## Install moos-ivp-pLearn

pip2 install --no-cache-dir numpy matplotlib 'tensorflow==1.14.0'
//using tensorflow version 1.14.0 and keras 2.0.8
pip2 install 'keras==2.0.8'

sudo apt install git

git clone https://github.com/mnovitzky/moos-ivp-pLearn.git

$cd moos-ivp-pLearn
$ ./build.sh

//can follow pLearn quickstart in 
//moos-ivp-pLearn/docs/Documentation/pLearn.pdf



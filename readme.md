
# MOOS-IvP pLearn
**Original Author: Arjun Gupta**

**Maintainer: Michael "Misha" Novitzky**

## Introduction

The moos-ivp-pLearn repository is an extension of Project Aquaticus which is based on MOOS-IvP, a powerful marine
autonomy library.  This repository assumes that the moos-ivp and moos-ivp-aquaticus repositories have been built and applications available through system PATH variable.

## RUN AS DOCKER IMAGE
  A container is a running process. Docker Image is the file system. An image includes everything needed to run an application.

Make sure docker is installed:
```
   $ docker run hello-world
```
If Docker is installed properly, it should display text including 'Hello from Docker!'


If it is not installed, please follow the instructions:
```
https://docs.docker.com/engine/install/ubuntu/
```

Pull the repository down:
```
   $ git clone https://github.com/mnovitzky/moos-ivp-pLearn.git
```

```
   $ cd moos-ivp-pLearn
   $ docker build -t plearn:1.0 .
```

```
   $  xhost +
```

```
   $ docker run --env="DISPLAY" --volume="/tmp/.X11-unix:/tmp/.X11-unix" --name  plearn -it plearn:1.0 bash 

```

Useful commands to run with a docker container:

exit while leaving it running (de-attach):

Ctrl+p Ctrl+q

Attach to a running container:
```
   $ docker exec -i -t plearn bash
```

restarting a container that has exited:
```
  $ docker start  `docker ps -q -l` # restart it in the background
  $ docker attach `docker ps -q -l` # reattach the terminal & stdin
```

username:moos
password:moos



## FROM SOURCE

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
   $ sudo apt install g++ subversion xterm cmake libfltk1.3-dev freeglut3-dev libpng-dev libjpeg-dev libxft-dev libxinerama-dev libtiff5-dev
   $ cd moos-ivp
   $ ./build.sh
```

Update your .bashrc file to include the following variables

//update the variable User_name with your system's username
```
User_name=mikey 
PATH="/home/${User_name}/moos-ivp/bin:${PATH}"
IVP_BEHAVIOR_DIRS="/home/${User_name}/moos-ivp/lib:${IVP_BEHAVIOR_DIRS}"
export PATH
export IVP_BEHAVIOR_DIRS
```

//after saving the .bashrc file -- you should load it by typing:
```
   $ bash 
```

//verify moos-ivp installation
```
   $ cd moos-ivp/ivp/missions/s1_alpha
   $ pAntler alpha.moos
```
//Start simulation by clicking 'DEPLOY' on lower right of the GUI



## Install moos-ivp-aquaticus

```
   $ cd ~
   $ svn co https://oceanai.mit.edu/svn/moos-ivp-aquaticus-oai/trunk/ moos-ivp-aquaticus
   $ cd moos-ivp-aquaticus
   $ ./build.sh
```

Update your .bashrc file to include the following variables

//update the variable User_name with your system's username
```
User_name=mikey 
PATH="/home/${User_name}/moos-ivp-aquaticus/bin:${PATH}"
IVP_BEHAVIOR_DIRS="/home/${User_name}/moos-ivp-aquaticus/lib:${IVP_BEHAVIOR_DIRS}"
export PATH
export IVP_BEHAVIOR_DIRS
```

//after saving the .bashrc file -- you should load it by typing:
```
   $ bash 
```

//Test moos-ivp-aquaticus
```
   $ cd ~/moos-ivp-aquaticus/missions/bots-only-example
   $ ./launch_demo
```
//A gui should come up and the robots will start moving when you click on the 'DEPLOY' button in the lower right.



## Install moos-ivp-pLearn
Install some dependencies for python

```
   $ sudo apt update
   $ sudo apt install -y libncurses-dev sudo python2.7-dev python-pip python-tk
   $ sudo apt install git
```

//Specifically verified using tensorflow version 1.5 and keras 2.0.8
```
   $ pip2 install --no-cache-dir numpy matplotlib 'tensorflow==1.5' 
   $ pip2 install 'keras==2.0.8' colorama
   
   $ git clone https://github.com/mnovitzky/moos-ivp-pLearn.git
```
Update your .bashrc file to include the following variables

//Update the variable User_name with your system's username
```
User_name=mikey
PATH="/home/${User_name}/moos-ivp-pLearn/bin:${PATH}"
IVP_BEHAVIOR_DIRS="/home/${User_name}/moos-ivp-pLearn/lib"
PYTHONPATH="${PYTHONPATH}:/home/${User_name}/moos-ivp-pLearn/pLearn/learning_code:/home/${User_name}/moos-ivp-pLearn/src/lib_python"

export PATH
export IVP_BEHAVIOR_DIRS
export PYTHONPATH
```

//After updating your .bashrc file, load it using:
```
   $ bash
```

//Now build the moos-ivp-pLearn specific c++/python applications:
```
   $ cd moos-ivp-pLearn
   $ ./build.sh
```

//can follow pLearn quickstart in 
//moos-ivp-pLearn/docs/Documentation/pLearn.pdf



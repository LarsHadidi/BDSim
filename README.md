# BDSim

<img src="https://cdn.rawgit.com/wiki/LarsHadidi/BDSim/res/img/Logo.svg" width="90" align="left" hspace="10" vspace="8" alt="Logo"/>

**BDSim** is a 2D brownian dynamics simulation software. Its primary purpose is the numerical analysis of *topological defects*, providing serveral programs for simulation, visualization and analysis. This project has been developed during research on topological defects in twodimensional brownian systems at the physics department of Johannes-Gutenberg Universität Mainz.

A detailed introduction can be found [in the wiki.](https://github.com/LarsHadidi/BDSim/wiki)

## Software Structure

The following figure illustrates the organization of the software system:

<img src="https://cdn.rawgit.com/wiki/LarsHadidi/BDSim/res/img/SoftwareArchitecture.svg" width="600" alt="Software Structure"/>

- **BDSim** is the core component which performs the simulation of a twodimensional system of brownian particles. It can measure different thermodynamic quantities and system states as well as extract topological defects.
- **DTracker** generates the trajectories according to the time evolution of the topological defects
- **DTAnalyzer** creates statistical analyses of the generated trajectories.
- **BDRenderer** is used to render video files of the simulated systems, visualizing different aspects like trajectories of the defects, the local bond orientational order parameter, the distribution of the topological charges among the system and several other characteristics.

There is also **BDumper** for converting the binary file formats used by BDSim into text based data.

## Installation

In order to install BDSim several prerequisites must be met.

### Dependencies

The following open source libraries must be installed on your system.

For every program:
- [Kitware's CMake](https://cmake.org/)
- [Boost C++ Libraries](http://www.boost.org/)

For the components:

**BDSim**
- A modified 2D version of [C. Rycroft's Voro++](http://math.lbl.gov/voro++/), provided in this repository
- [Boost.SIMD](https://github.com/NumScale/boost.simd) by [Numscale](https://www.numscale.com)

**DTracker**
- [iGraph C library](http://igraph.org/c/)
- [GNU Scientific Library (GSL)](http://www.gnu.org/software/gsl/)

**DTAnalyzer**
- [STXXL](http://stxxl.sourceforge.net/)

**BDRenderer**
- [OpenCV for C++](http://opencv.org/)
- [GNU Scientific Library (GSL)](http://www.gnu.org/software/gsl/)

### Compilation

If all dependencies are present on your system, every component of the BDSim Software can be built using a C++11 compliant compiler. There is a standard CMake configuration file for each component, a detailed description how to set up such a configuration file is given [in the wiki](https://github.com/LarsHadidi/BDSim/wiki).
Assuming that the standard CMake configuration is sufficient, the following steps are to be performed for each of the five folders which contain the corresponding  components  running:
1. Create a build folder to and out-of-place build.
2. Run cmake from within that folder to generate the standard build files.
3. Run the build tool to compile the code.
4. Move the compiled executable into to top-folder which contains the necessary configuration files in subfolders. For more details on this, check out the [wiki](https://github.com/LarsHadidi/BDSim/wiki).

## Usage

Every program in this software package is a command line interface application which takes several arguments. Some of the programs need supplementary configuration files which must be located in a special subfolder of the one in which the program is run. Most of the programs take an input path and an output path as start-up parameters. Some output needs extra processing which can be done via simple scripts which are given in the *supplementary materials* folder. The user manual can be found [in the wiki](https://github.com/LarsHadidi/BDSim/wiki).

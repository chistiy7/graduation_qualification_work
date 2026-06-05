# Shaft production planner
[![en](https://img.shields.io/badge/lang-en-green.svg)](https://github.com/kolyaka32/Shaft-production-planner/blob/Main/README.md)  [![ru](https://img.shields.io/badge/lang-ru-green.svg)](https://github.com/kolyaka32/Shaft-production-planner/blob/Main/README-ru.md)

This codebase implement the system for planning and development of shaft prouction factory.

## Usage
After entering program you suppose to enter input and output part parameters.

![Screenshot of product input stage](/screenshots/product-en.png?raw=true)

Based on this parameters program create production chain, calulate required parameters and showing required equipment and cost.

![Screenshot of process stage](/screenshots/process-en.png?raw=true)

Based on required equipment you should place it in grid, creating workshop layout.
Also you can allow to program do it by itself and get automatick acceptable result.

![Screenshot of workshop planner stage](/screenshots/factory-en.png?raw=true)

After all calculation program provide end result list with all calculated values.
Also program allows to save/load in any state of work, remembering all inputed values.


## Supported languages
* English
* Russian (Русский)


## Installing of project
1. Download archieve from [releases](https://github.com/kolyaka32/Shaft-production-planner/releases), unzip and run 'Planner.exe'!
2. You can build project by yourself, watch point 'Instalation for own use'.


## Instalation for own use (windows)
1. Install C++ compiler (such as [MSYS2](https://www.msys2.org/#installation));
2. Install [Make](https://sourceforge.net/projects/gnuwin32/files/make/3.81/make-3.81.exe/download);
3. Install [CMake](https://sourceforge.net/projects/cmake.mirror/);
4. Download and install external [SFML library](https://github.com/SFML/SFML);
For build and install code by yourself: [download](https://github.com/SFML/SFML/releases/) source code, unzip to any folder and run comand line **as administrator**:
```bash
cd ./SFML-3.0.1
mkdir build
cd build/
cmake ../
make
make install
```
5. (*Optional) Code editor [Visual Studio Code](https://code.visualstudio.com/download) (for better usage) (Recomended to use with plugins on c++ and cmake).
6. Compile project itself:
```bash
cd ./Shaft-production-planner
mkdir build
cmake --build ./build --config Release --target all --
```


## Instalation for own use (macOS)

1. Install tools: `brew install cmake sfml` (SFML 3 required).
2. Build from repository root: `./scripts/build.sh`
3. Run from project root (folder `resources/` must be present): `./scripts/run.sh`

Use a normal terminal with GUI access; headless/sandboxed runs may fail with `mutex lock failed`.


## Instalation for own use (unux systems)
1. Check availability of compilers[g++], [make] and [cmake];
2. Install external library [SFML](https://github.com/SFML/SFML)
(For Debian-like systems):
```bash
sudo apt-get update
sudo apt-get install libsfml-dev
```
Or build and install code by yourself: [download](https://github.com/SFML/SFML/releases/) source code, unzip to any folder and run comand line **as administrator**:
```bash
cd ./SFML-3.0.1
mkdir build
cd build/
cmake ../
make
make install
```
3. (*Optional) Code editor [Visual Studio Code](https://code.visualstudio.com/download) (for better usage) (Recomended to use with plugins on c++ and cmake).
4. Compile project itself:
```bash
cd ./Shaft-production-planner
mkdir build
cmake --build ./build --config Release --target all --
```


## Prepaired sources
Sources with all self-builded libraries can be located at [releases](https://github.com/kolyaka32/Shaft-production-planner/releases) (Development-kit).


## Used external libraries:
* https://github.com/SFML/SFML/releases/

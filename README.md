# Cinder for Bit Studio

## Prerequisites
* CMake
* GStreamer x64 (+devel) (Tested with 1.12.4)
* OpenCV 3.4.0
* Boost 1.60.0

## Setting up developer environments
* Use CMake to build OpenCV (Static link) with debug;release
* Build boost static link (need sgd flag), x64 with visual studio native command prompt.
```
	bootstrap.bat
	b2 -j8 toolset=msvc-version address-model=64 link=static --build-type=complete stage
```
* Use CMake to build Cinder
* Copy template from `BIT_PROJ_TEMPLATE`
* Enjoy

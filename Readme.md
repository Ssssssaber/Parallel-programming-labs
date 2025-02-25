# About
Laboratory projects for parallel image processing and clustering

# Usage
#### App A
Relief and minimization
```console
app_a.exe {input.bmp} {output.bmp} {numThreads}
```
#### App B
Erosion
```console
app_b.exe {input.bmp} {output.bmp} {numThreads} [intencityThreshold] [erosionStep]
```
#### App C
Clusterization with Silhouette index
```console
app_c.exe [--csv input.csv] [--x name] [--y name] [--max uint] [--K uint] [--thrCount uint] [--outSVG out.svg]
```

# Requirements
* GCC > version 8
* CMake > version 3.10
* Windows MinGW64 link: https://sourceforge.net/projects/mingw-w64/

# Build from source
#### Windows
```console
./build.bat
```
#### Linux
```console
./build.sh
```
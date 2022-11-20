# aff3ct-ldpc-decoder

This repository contains a simple CLI LDPC decoder using
[AFF3CT](https://aff3ct.github.io/) as a library.

Currently it only supports the CCSDS AR4JA r=1/2 k=1024 code, but it is
relatively simple to include support for other codes by obtaining the alist
describing the code (for instance with
[ldpc-toolbox](https://github.com/daniestevez/ldpc-toolbox)) and ajusting some
of the decoder parameters.

This repository is based on the examples in
[my_project_with_aff3ct](https://github.com/aff3ct/my_project_with_aff3ct).

## Building

The first step is to compile AFF3CT into a library.

Get the AFF3CT library:

```bash
$ git submodule update --init --recursive
```

Compile the library on Linux/MacOS/MinGW:

```bash
$ cd lib/aff3ct
$ mkdir build
$ cd build
$ cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-funroll-loops -march=native" -DAFF3CT_COMPILE_EXE="OFF" -DAFF3CT_COMPILE_STATIC_LIB="ON" -DAFF3CT_COMPILE_SHARED_LIB="ON"
$ make -j4
```

Compile the library on Windows (Visual Studio project)

```bash
$ cd lib/aff3ct
$ mkdir build
$ cd build
$ cmake .. -G"Visual Studio 15 2017 Win64" -DCMAKE_CXX_FLAGS="-D_CRT_SECURE_NO_DEPRECATE /EHsc /MP4" -DAFF3CT_COMPILE_EXE="OFF" -DAFF3CT_COMPILE_STATIC_LIB="ON" -DAFF3CT_COMPILE_SHARED_LIB="ON"
$ devenv /build Release aff3ct.sln
```

Now the AFF3CT library has been built in the `lib/aff3ct/build` folder.

Copy the cmake configuration files from the AFF3CT build

```bash
$ mkdir -p cmake/Modules
$ cp lib/aff3ct/build/lib/cmake/aff3ct-*/* cmake/Modules
```

Compile the code on Linux/MacOS/MinGW:

```bash
$ mkdir build
$ cd build
$ cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-funroll-loops -march=native"
$ make
```

Compile the code on Windows (Visual Studio project)

```bash
$ mkdir build
$ cd build
$ cmake .. -G"Visual Studio 15 2017 Win64" -DCMAKE_CXX_FLAGS="-D_SCL_SECURE_NO_WARNINGS /EHsc"
$ devenv /build Release my_project.sln
```

The compiled binary is in `build/bin/aff3ct-ldpc-decoder`.

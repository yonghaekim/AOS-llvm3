# AOS LLVM

## Dependencies
You will likely need the following packages.
```
git cmake python-dev libncurses5-dev swig libedit-dev libxml2-dev build-essential gcc-7-plugin-dev clang-6 libclang-6-dev lld-6
```

## Clone Git Repositories
Clone LLVM 8.0.1 and place the AOS-llvm repository.
```
mkdir LLVM
git clone -b llvmorg-8.0.1 https://github.com/yonghaekim/llvm-project.git
rm -rf llvm-project/llvm
git clone https://github.com/yonghaekim/AOS-llvm llvm-project/llvm
```

## Download Toolchains for Cross-Compilation
Linaro toolchains are needed to cross-compile programs using the AArch64 ISA.
```
wget https://releases.linaro.org/components/toolchain/binaries/7.3-2018.05/aarch64-linux-gnu/sysroot-glibc-linaro-2.25-2018.05-aarch64-linux-gnu.tar.xz
tar xJf sysroot-glibc-linaro-2.25-2018.05-aarch64-linux-gnu.tar.xz
wget https://releases.linaro.org/components/toolchain/binaries/7.3-2018.05/aarch64-linux-gnu/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu.tar.xz
tar xJf gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu.tar.xz
```

## Build Instructions
```
mkdir aos-build
cd aos-build
cmake -G Ninja \
          -DCMAKE_INSTALL_PREFIX=${HOME}/opt/AOS  \
          -DCMAKE_BUILD_TYPE=Debug                \
          -DBUILD_SHARED_LIBS=On                  \
          -DLLVM_TARGETS_TO_BUILD=AArch64         \
          -DLLVM_BUILD_TOOLS=On                   \
          -DLLVM_BUILD_TESTS=Off                  \
          -DLLVM_BUILD_EXAMPLES=Off               \
          -DLLVM_BUILD_DOCS=Off                   \
          -DLLVM_INCLUDE_EXAMPLES=Off             \
          -DLLVM_ENABLE_LTO=Off                   \
          -DLLVM_ENABLE_DOXYGEN=Off               \
          -DLLVM_ENABLE_RTTI=Off                  \
          -DLLVM_ENABLE_PROJECTS="clang"          \
          "../llvm-project/llvm"
ninja
```

## How to OPT an IR file (e.g., test.ll) using AOS opt passes?
```
opt -O0 -aos=enable -aos-opt -S test.ll -o test_aos.ll
```

## How to compile an instrumented IR file (e.g., test_aos.ll)?
```
$LLVM_PATH/aos-build/bin/clang++ -O3 --target=aarch64-linux-gnu \
          -march=armv8.3-a -I$LLVM_PATH/sysroot-glibc-linaro-2.25-2018.05-aarch64-linux-gnu/usr/include \
          -B$LLVM_PATH/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu \
          -Wall -Wextra -fPIC -fvisibility=hidden \
          --sysroot=$LLVM_PATH/sysroot-glibc-linaro-2.25-2018.05-aarch64-linux-gnu -static \
          test_aos.ll -o test_aos
```

## Publications
```
@inproceedings{kim:aos,
  title        = {{Hardware-based Always-On Heap Memory Safety}},
  author       = {Yonghae Kim and Jaekyu Lee and Hyesoon Kim},
  booktitle    = {Proceedings of the 53rd IEEE/ACM International Symposium on Microarchitecture (MICRO)},
  month        = nov,
  year         = 2020,
  address      = {Athens, Greece},
}
```

## Original LLVM README
================================

This directory and its subdirectories contain source code for LLVM,
a toolkit for the construction of highly optimized compilers,
optimizers, and runtime environments.

LLVM is open source software. You may freely distribute it under the terms of
the license agreement found in LICENSE.txt.

Please see the documentation provided in docs/ for further
assistance with LLVM, and in particular docs/GettingStarted.rst for getting
started with LLVM and docs/README.txt for an overview of LLVM's
documentation setup.

If you are writing a package for LLVM, see docs/Packaging.rst for our
suggestions.

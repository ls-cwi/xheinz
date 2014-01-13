Compilation instructions
========================

Dependencies
--------------

* Boost (>= 1.49)
* LEMON 1.3
* ILOG CPLEX (>= 12.0)
* Compiler that supports C++11

Installation instruction for MacOS X 10.9 and 10.8.5
----------------------------------------------------

In Mavericks the default C\+\+ standard library is libc\+\+ instead of
libstdc\+\+. Since the CPLEX library is using libstdc\+\+, we are forced to use
this as well. However, the provided Clang compiler is not able to compile
C\+\+11 code when using libstdc\+\+. That is why we resort to gcc 4.8.

To install gcc-4.8 via homebrew:

    brew tap homebrew/versions
    brew install gcc48

If an outdated version of brew is installed, this step will fail with the
following error

     Error: undefined method `cxxstdlib_check' for gcc48:Gcc48

In which case the latest version of brew have to be installed :

    brew update 

Note that `brew update` might fail with the following error

    Error: undefined method `to_sym' for nil:NilClas

In which case it's enough to re-run `brew update`.



Now LEMON 1.3 can be installed via:

    wget http://lemon.cs.elte.hu/pub/sources/lemon-1.3.tar.gz
    cmake -DCMAKE_INSTALL_PREFIX=~/lemon -DCMAKE_CXX_COMPILER=/usr/local/bin/g++-4.8 -DCMAKE_C_COMPILER=/usr/local/bin/gcc-4.8
    make install

Next step is to install boost:

    brew install boost

Now compile xHeinz from the root of the git repository as follows. Note that
CPLEX will be auto-detected if it's installed in ~/ILOG.

    mkdir build
    cd build
    cmake -DCMAKE_CXX_COMPILER=/usr/local/bin/g++-4.8 -DCMAKE_C_COMPILER=/usr/local/bin/gcc-4.8 ..


On os 10.8.5, the auto-detection of the ILOG install directory failed, and the
install dir must be specified explicitly, e.g. by running the following cmake
command

    cmake \
    -DCMAKE_CXX_COMPILER=/usr/local/bin/g++-4.8 \
    -DCMAKE_C_COMPILER=/usr/local/bin/gcc-4.8 \
    -DCPLEX_INC_DIR=~/ILOG/cplex/include/ \
    -DCPLEX_LIB_DIR=~/ILOG/cplex/lib/x86-64_osx/static_pic \
    -DCONCERT_LIB_DIR=~/ILOG/concert/lib/x86-64_osx/static_pic \
    -DCONCERT_INC_DIR=~/ILOG/concert/include/ ..

Next step is to compile the binary,

     make

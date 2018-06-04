
OpenCRun -- Yet another OpenCL runtime
======================================

OpenCRun is an OpenCL runtime developed for research purpose at [Politecnico di
Milano][1]. It used [LLVM][2] and [Clang][3] to compile OpenCL kernels into
executable code.

It can support different devices. Right now, only CPU devices (i386 and X86_64)
are supported.

Integration with LLVM/Clang
---------------------------

The whole runtime heavily exploit LLVM/Clang libraries and build system.
LLVM/Clang must be installed before building OpenCRun.

Currently OpenCRun has been built and tested with LLVM/Clang up to release 6.
Tags are also available in the OpenCRun repository to revert back its source
code to versinons compatible with LLVM/Clang 3.5 and 3.7 (Note: v.3.5 requires
GCC < 4.9). 

Current quality status
----------------------

Most of the OpenCL API and the OpenCL C library functions are supported. The
`unittests` directory contains automated tests using OpenCRun through the OpenCL
C++ API. Moreover, in the `bench` directory there are some more complex
examples.

Installing
----------

OpenCRun requires that LLVM is built with run-time type information enabled.
If building from a sub-directory of the LLVM source tree:

```
cmake .. -DCMAKE_INSTALL_PREFIX=${HOME}/local -DLLVM_ENABLE_RTTI:BOOL=1
```

Then, the LLVM/Clang source and build directories must be specified when
building OpenCRun:

```
cmake .. -DCMAKE_INSTALL_PREFIX=${HOME}/local 
  -DLLVM_SRC_ROOT=<LLVM source dir>
  -DLLVM_OBJ_ROOT=<LLVM build dir>
```

Authors
-------

For any question, please contact [Ettore Speziale][4] or [Alberto Magni][5].

[1]: http://www.polimi.it
[2]: http://www.llvm.org
[3]: http://clang.llvm.org
[4]: mailto:speziale.ettore@gmail.com
[5]: mailto:alberto.magni86@gmail.com

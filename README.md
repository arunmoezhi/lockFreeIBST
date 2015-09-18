# lockFreeIBST
This repository contains implementation of a lock-free internal Binary Search Tree.<br>
The algorithm is described in our paper *A Fast Lock-Free Internal Binary Search Tree* published in ICDCN'15<br>
The technical report is available in the paper directory<br>
Binary file is in bin directory<br>
Source files are in src directory<br>
How to compile?<br>
1. Change the Makefile appropriately and run make<br>
How to run?<br>
`$./bin/lockFreeIbst.o numOfThreads readPercentage insertPercentage deletePercentage durationInSeconds maximumKeySize initialSeed`<br>
Example: `$./bin/lockFreeIbst.o 64 90 9 1 5 100000 0`<br>
Output is a semicolon separated line with the last entry denoting throughput in millions of operations per second<br>
Required Libraries:<br>
* GSL to create random numbers
* Intel "Threading Building Blocks(TBB)" atomic library(freely available)
* C++ std library can be used by commenting out "#define TBB" in header.h file
* Some memory allocator like jemalloc, tcmalloc, tbbmalloc,etc

Any questions?

contact - arunmoezhi at gmail dot com

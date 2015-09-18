# lockFreeIBST
This repository contains implementation of a lock-free internal Binary Search Tree.
The algorithm is described in our paper "A Fast Lock-Free Internal Binary Search Tree" Arunmoezhi Ramachandran and Neeraj Mittal published in ICDCN'15
The technical report is available at http://www.utdallas.edu/~arunmoezhi/fastLockFreeInternalBST.pdf
Binary file is in bin directory
Source files are in src directory
How to compile?
1. Change the Makefile appropriately and run make
How to run?
$./bin/lockFreeIbst.o numOfThreads readPercentage insertPercentage deletePercentage durationInSeconds maximumKeySize initialSeed
Example: $./bin/lockFreeIbst.o 64 90 9 1 5 100000 0
Output is a semicolon separated line with the last entry denoting throughput in millions of operations per second
Required Libraries:
1. GSL to create random numbers
2. Intel "Threading Building Blocks(TBB)" atomic library(freely available). C++ std library can be used by commenting out "#define TBB" in header.h file
Some memory allocator like jemalloc, tcmalloc, tbbmalloc,etc
Any questions?
contact - arunmoezhi at utdallas dot edu

# sofa-designer
Calculates bounds on the moving sofa problem

On either linux or Mac OS X, run make to produce the binary. Compilation requires GMP 6.1.2 library and `-pthread` compilation option enabled.

    make

The binary named `exec` will appear in the same directory. Also, the object files are produced in the `build` directory. 

To use the binary, run `exec` as the following.

    ./exec < init.sofa

The binary takes input from stdin as the format of file `init.sofa`. 
Although one can provide the same format in stdin, I encourage you to have the formatted file pipelined to exec.

    Number of angles: 5
    24 7 25
    56 33 65
    120 119 169
    33 56 65
    7 24 25
    
    Index to fix mu: 2
    
    Number of initial sofas: 4
    
    Target: 237/100

The index to fix mu can be from zero to any number less than the number of angles. 
This is the index of L-shape with one fixed degree of freedom.
The number of initial sofas can by any positive integer.
Target is the bound we want to show. Instead of using priority queue, this software branches out bounding boxes 
in a DFS sense until the sofa reaches an area lower than the specified target.
Also, one can controll the number of threads and other options by modifying the constants in `src/exec/main.cpp`.

Type the following to remove all object and binary files (and possibly recompile from scratch).

    make clean


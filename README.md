# xpoly
Polyhedral Transformation, Linear Programming Solver, Polyhedral based Dependence Analysis

The software is licensed under the BSD License.

This software implement several linear programming solvers, include 
exact simplex based on rational arithmetic, the approximate method based
on double/float arithmetic, the 0-1 integer programming solver, and the 
mixed integer programming solver.

This software also implement various loop optimization using the polyhedral
model. Compilers can use this software as a tool to perform loop optimizations.

The polyhedral model (also called the polytope method) is a mathematical
framework for loop nest optimization in program optimization. The polytope
method treats each loop iteration within nested loops as lattice points inside
mathematical objects called polytopes, performs affine transformations or more
general non-affine transformations such as tiling on the polytopes, and then
converts the transformed polytopes into equivalent, but optimized (depending on
targeted optimization goal), loop nests through polyhedra scanning.

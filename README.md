# Profiler
An automatic, instrumented CPU profiler. This profiler works by hooking into the prolog and epilog of each function, and inserting a small, lightweight bit of code that times each tree and builds a function hierarchy. The function tree is then printed when the program exits.

# Requirements
Must be an x86 program with access to windows functions through Windows.h

# How to Integrate
Simply include the source files in your project. For the profiler to run successfully, optimizations must be turned off for Profiler.cpp, though they can be turned on for the rest of the program. /GH and /Gh compiler flags must be set for every compilation unit EXCEPT Profiler.cpp.

# Pros and Cons
The profiler is incredibly simple to use and requires no integration, however it prevents whole program optimization and adds overhead to every function it is a part of. Generally, the profiler will only work effectively with relatively small codebases.

# Future Improvements
JSON serialization is the first improvement I'd like to make. Additionally, adding the fraction of total time taken, as well as real time taken would be good, simple improvements. Finally, if possible I'd like to reduce the profiler's footprint.

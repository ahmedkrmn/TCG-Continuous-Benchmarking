## Benchmarks

This directory contains all of the benchmarks developed throughout the project which are used both in the [reports](https://ahmedkrmn.github.io/TCG-Continuous-Benchmarking/) for demonstrating how the new tools work, and are also used in the [nightly tests](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/tree/master/tools/qemu_nightly_tests) for daily performance monitoring of QEMU.

**Benchmarks List**:

- **coulomb:**

  N electrons are given. They are randomly distributed on a 1m x 1m surface.

  This program calculates the resulting force according to Coulomb law for each electron. The resulting force is a pair (magnitude, angle), where the unit for magnitude is Newton, and angle is calculated with x-axis as a reference (zero angle), and is expressed in degrees (0-360 degrees).

  Different versions using float, double, and long double FP are available.

- **dijkstra_double:**

  Find the shortest path between the source node and all other nodes using Dijkstra’s algorithm. The graph contains n nodes where all nxn distances are double values. The value of n can be specified using the `-n` flag. The default value is 2000.

- **dijkstra_int32:**

  Find the shortest path between the source node and all other nodes using Dijkstra’s algorithm. The graph contains n nodes where all nxn distances are int32 values. The value of n can be specified using the `-n` flag. The default value is 2000.

- **matmult_double:**

  Standard matrix multiplication of an n\*n matrix of randomly generated double numbers from 0 to 100. The value of n is passed as an argument with the `-n` flag. The default value is 200.

- **matmult_int32:**

  Standard matrix multiplication of an n\*n matrix of randomly generated integer numbers from 0 to 100. The value of n is passed as an argument with the `-n` flag. The default value is 200.

- **qsort_double:**

  Quick sort of an array of n randomly generated double numbers from 0 to 1000. The value of n is passed as an argument with the `-n` flag. The default value is 300000.

- **qsort_int32:**

  Quick sort of an array of n randomly generated integer numbers from 0 to 50000000. The value of n is passed as an argument with the `-n` flag.The default value is 300000.

- **qsort_string:**

  Quick sort of an array of 10000 randomly generated strings of size 8 (including null terminating character). The sort process is repeated n number of times. The value of n is passed as an argument with the `-n` flag. The default value is 20.

- **search_string:**

  Search for the occurrence of a small string in a much larger random string (“needle in a hay”). The search process is repeated n number of times and each time, a different large random string (“hay”) is generated. The value of n can be specified using the `-n` flag. The default value is 20.


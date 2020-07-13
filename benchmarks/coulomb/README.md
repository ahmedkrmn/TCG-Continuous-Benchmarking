## Coulomb Benchmark

N electrons are given. They are randomly distributed on a 1m x 1m surface.

This program calculates the resulting force according to Coulomb law for each electron. The resulting force is a pair (magnitude, angle), where the unit for magnitude is Newton, and angle is calculated with x-axis as a reference (zero angle), and is expressed in degrees (0-360 degrees).

Different versions using float, double, and long double FP are available.

### Usage

#### Compile

`gcc coulomb_double.c -o coulomb_double -lm`

#### Run

`./coulomb_double -n 1000`

Where `n` is the number of electrons on the surface. Default is 1000.

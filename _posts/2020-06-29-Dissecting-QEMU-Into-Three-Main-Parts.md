---
layout: post
title: Dissecting QEMU Into Three Main Parts
subtitle: Code Generation, JIT Execution, and Helpers Execution
tags: [Callgrind, Code Generation, JIT Execution, Helpers Execution]
excerpt: The previous report presented an overview of measuring basic performance metrics of QEMU, and one of these metrics, naturally, was the total number of executed host instructions. This report further utilizes Callgrind to break down that total number into numbers that correspond to three main parts of QEMU operation; code generation, JIT-ed code execution, and helpers execution.
---

## Intro

The previous report presented an overview of measuring basic performance metrics of QEMU, and one of these metrics, naturally, was the total number of executed host instructions. This report further utilizes Callgrind to break down that total number into numbers that correspond to three main parts of QEMU operation: code generation, JIT-ed code execution, and helpers execution.

## Table of Contents

- [Breaking Down QEMU Execution Phases](#breaking-down-qemu-execution-phases)
  - [Principle of Operation](#principle-of-operation)
  - [Example of Usage](#example-of-usage)
- [Comparing 17 Targets of QEMU](#comparing-17-targets-of-qemu)
  - [Overview](#overview)
  - [Results](#results)
- [Discussion of Results](#discussion-of-results)
  - [mips/mips64 vs mipsel/mips64el](#mipsmips64-vs-mipselmips64el)
  - [m68k (non-RISC) target vs RISC targets](#m68k-non-risc-target-vs-risc-targets)

## Breaking Down QEMU Execution Phases

Execution of an instance of QEMU can be split into three main parts: code generation, JIT execution and helpers execution. Code generation is often referred as "translation time" (the target code is translated to intermediate code, and, in turn, to host code), while JIT execution and helpers execution are often referred as "execution time" (host code is being executed). So JIT and helpers execution are similar in the sense that they execute host code, however, since their origin and internal organization is very different, it is useful to distinguish between the two.

There are perhaps some other parts of QEMU that are not taken into account here - for example, initialization of QEMU itself. However, for all intents and purposes, and for measuring emulation of a benchmark of all sizes except the smallest, these parts are negligible, and not subject of interest of this report. For example, QEMU initialization will be included in code generation part while doing calculations in this report, but, still, that will not impact the accuracy of results in any substantial way.

The three parts of QEMU execution mentioned above are not, of course, executed sequentially, akin to phases - their execution is interleaved. However, it is still useful to know information about each part separately. This report presents, as its key idea, a script called `dissect.py` that prints the total number of instructions spent in each of said QEMU parts.

The script is available on the project [GitHub page](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/tree/master/tools/dissect_qemu_instructions).

### Example of Usage

Using the same [Coulomb](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/tree/master/benchmarks/coulomb) benchmark from the previous report, it can be compiled on an x86_64 Linux machine using:

```bash
gcc -static -O2 coulomb_double.c -o coulomb_double -lm
```

And then the `dissect.py` script can be invoked using:

```bash
./dissect.py -- <qemu-build>/x86_64-linux-user/qemu-x86_64 coulomb_double
```

The script displays the total number of instructions, and then divides this number into the three components:

```
Total Instructions:        4,702,865,362

Code Generation:             115,819,309	 2.463%
JIT Execution:             1,081,980,528	23.007%
Helpers:                   3,505,065,525	74.530%
```

### Principle of Operation

Callgrind distinguishes two measures for each function: "self" (for execution only within the function itself) and "inclusive" (for execution in both the function and all of its calees, to any depth). Also, there is an important `--tree` option of `callgrind_annotated` utilized in an important fashion in the `dissect.py` script.

Firstly, the script executes the passed QEMU invocation command with Callgrind. Secondly, it executes `callgrind_annotate` using the `--tree=caller` flag to print the callers of each function. Calculation for each part is done this way:

- The number of "self" instructions for the JIT execution can be directly obtained.
- The number of instructions spent in helpers can be easily calculated by subtracting the "self" number for JIT from correspondent "inclusive" number.
- The number of code generation instructions is obtained by subtracting the "inclusive" number of JIT from the program’s total number of instructions.

## Comparing 17 Targets of QEMU

### Overview

One very handy usage of the `dissect.py` script is to compare how QEMU performs in each of its thee phases across different targets. To perform this task, a small helper Python script is used.

```python
import csv
import os
import subprocess


############### Script Options ###############
qemu_build_path = "<qemu-build>"
benchmark_args = ["-n", "1000"]
targets = {
    "aarch64":  "aarch64-linux-gnu-gcc",
    "alpha":    "alpha-linux-gnu-gcc",
    "arm":      "arm-linux-gnueabi-gcc",
    "hppa":     "hppa-linux-gnu-gcc",
    "m68k":     "m68k-linux-gnu-gcc",
    "mips":     "mips-linux-gnu-gcc",
    "mipsel":   "mipsel-linux-gnu-gcc",
    "mips64":   "mips64-linux-gnuabi64-gcc",
    "mips64el": "mips64el-linux-gnuabi64-gcc",
    "ppc":      "powerpc-linux-gnu-gcc",
    "ppc64":    "powerpc64-linux-gnu-gcc",
    "ppc64le":  "powerpc64le-linux-gnu-gcc",
    "riscv64":  "riscv64-linux-gnu-gcc",
    "s390x":    "s390x-linux-gnu-gcc",
    "sh4":      "sh4-linux-gnu-gcc",
    "sparc64":  "sparc64-linux-gnu-gcc",
    "x86_64":   "gcc"
}
##############################################

# Store dissect.py output for each target
targets_data = []
for target_name, target_compiler in targets.items():
    print("Measuring instructions for target: " + target_name)
    compile_target = subprocess.run([target_compiler,
                                     "-O2",
                                     "-static",
                                     "coulomb_double.c",
                                     "-lm",
                                     "-o",
                                     "/tmp/coulomb_double"])
    dissect_target = subprocess.run((["./dissect.py",
                                      "--",
                                      "{}/{}-linux-user/qemu-{}".format(qemu_build_path,
                                                                        target_name,
                                                                        target_name),
                                      "/tmp/coulomb_double"] + benchmark_args),
                                    stdout=subprocess.PIPE)
    os.unlink("/tmp/coulomb_double")
    # Read the dissect output
    lines = dissect_target.stdout.decode("utf-8").split('\n')
    # Extract measurements
    total_instructions = lines[0].split()[-1]
    code_generation_percentage = lines[2].split()[-1]
    jit_execution_percentage = lines[3].split()[-1]
    helpers_execution_percentage = lines[4].split()[-1]
    # Save measurements to the targets_data list
    targets_data.append([target_name,
                         total_instructions,
                         code_generation_percentage,
                         jit_execution_percentage,
                         helpers_execution_percentage])

# Save output to CSV
csv_headers = ["Target", "Total Instructions",
               "Code Generation %", "JIT Execution %", "Helpers %"]
with open("dissect_targets.csv", "w") as csv_file:
    # Declare the writer
    writer = csv.writer(csv_file)
    # write CSV file header names
    writer.writerow(csv_headers)
    # For each target, write its collected measurements
    for target in targets_data:
        writer.writerow(target)

```

After providing the script with the required options, for each target, it compiles the Coulomb benchmark, and then runs `dissect.py` on the compiled executable. The results are saved in a CSV file.

### Results

<style>
  /* Right align all table fields */
  table#results td:nth-last-child(-n + 4) {
    text-align: right;
  }
  table#results th:nth-last-child(-n + 4) {
    text-align: right;
  }
  /* Borders for table header */
  table#results th{
    border: 1px solid black;
  }
  /* Borders for table data */
  table#results td {
    border-right: 1px solid black;
  }
  table#results td:first-child {
    border-left: 1px solid black;
  }
  /* Borders for last table row */
  table#results tr:nth-last-child(-n + 1) td {
    border-bottom: 1px solid black;
  }
</style>
<div style="overflow-x: auto;">
  <table id="results" style="width: 100%;">
    <thead>
      <tr>
        <th>Target</th>
        <th>Total Instructions</th>
        <th>Code Generation %</th>
        <th>JIT Execution %</th>
        <th>Helpers Execution %</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td style="color: #4dac26;">4 692 357 988</td>
        <td style="color: #d7191c;">2.758%</td>
        <td style="color: #d7191c;">32.437%</td>
        <td style="color: #4dac26;">64.804%</td>
      </tr>
      <tr>
        <td>alpha</td>
        <td>10 804 422 926</td>
        <td style="color: #4dac26;">0.958%</td>
        <td>11.042%</td>
        <td>88.000%</td>
      </tr>
      <tr>
        <td>arm</td>
        <td style="color: #d7191c; font-weight:bold">39 325 544 973</td>
        <td style="color: #4dac26; font-weight:bold">0.483%</td>
        <td style="color: #d7191c; font-weight:bold">76.003%</td>
        <td style="color: #4dac26; font-weight:bold">23.514%</td>
      </tr>
      <tr>
        <td>hppa</td>
        <td>12 005 435 084</td>
        <td>0.975%</td>
        <td style="color: #4dac26;">8.988%</td>
        <td style="color: #d7191c;">90.037%</td>
      </tr>
      <tr>
        <td>m68k</td>
        <td>7 266 676 762</td>
        <td>1.116%</td>
        <td style="color: #4dac26; font-weight:bold">5.904%</td>
        <td style="color: #d7191c; font-weight:bold">92.980%</td>
      </tr>
      <tr>
        <td>mips</td>
        <td>10 440 969 560</td>
        <td>1.366%</td>
        <td>10.643%</td>
        <td>87.990%</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td>11 715 714 129</td>
        <td>1.247%</td>
        <td>10.012%</td>
        <td>88.741%</td>
      </tr>
      <tr>
        <td>mips64</td>
        <td>10 337 898 389</td>
        <td>1.409%</td>
        <td>9.790%</td>
        <td>88.801%</td>
      </tr>
      <tr>
        <td>mips64el</td>
        <td>11 596 334 956</td>
        <td>1.281%</td>
        <td>9.118%</td>
        <td>89.601%</td>
      </tr>
      <tr>
        <td>ppc</td>
        <td>12 713 132 146</td>
        <td>1.115%</td>
        <td>10.215%</td>
        <td>88.671%</td>
      </tr>
      <tr>
        <td>ppc64</td>
        <td style="color: #d7191c;">12 716 587 866</td>
        <td>1.122%</td>
        <td>9.760%</td>
        <td>89.119%</td>
      </tr>
      <tr>
        <td>ppc64le</td>
        <td>12 694 752 808</td>
        <td>1.118%</td>
        <td>9.611%</td>
        <td>89.271%</td>
      </tr>
      <tr>
        <td>riscv64</td>
        <td style="color: #4dac26; font-weight:bold">4 149 509 947</td>
        <td style="color: #d7191c;">5.626%</td>
        <td>19.113%</td>
        <td>75.261%</td>
      </tr>
      <tr>
        <td>s390x</td>
        <td>10 946 821 241</td>
        <td style="color: #4dac26;">0.843%</td>
        <td style="color: #4dac26;">8.850%</td>
        <td style="color: #d7191c;">90.307%</td>
      </tr>
      <tr>
        <td>sh4</td>
        <td style="color: #d7191c;">12 728 200 623</td>
        <td>1.344%</td>
        <td>18.057%</td>
        <td>80.598%</td>
      </tr>
      <tr>
        <td>sparc64</td>
        <td>11 979 151 647</td>
        <td style="color: #d7191c; font-weight:bold">5.634%</td>
        <td>12.907%</td>
        <td>81.459%</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td style="color: #4dac26;">4 703 175 766</td>
        <td>2.469%</td>
        <td style="color: #d7191c;">23.005%</td>
        <td style="color: #4dac26;">74.526%</td>
      </tr>
    </tbody>
  </table>
</div>

## Discussion of Results

The table above offers a lot of material for discussion and exploration. For now, only a couple of them will be touched.

### mips/mips64 vs mipsel/mips64el

There is one thing intriguing about mips targets: big endian versions are faster than little endian versions. This is a sort of counterintuitive, since the host is intel, a little endian system. Let's see what are top 15 functions for mips target:

```bash
./topN_callgrind.py -n 15 -- <qemu-build>/mips-linux-user/qemu-mips coulomb_double-mips
```

Results:

<pre class="highlight" style="font-size:14.6205px">
  <code>
   No.  Percentage  Function Name                   Source File
  ----  ----------  ------------------------------  ------------------------------
     1     21.974%  soft_f64_addsub                 &lt;qemu&gt;/fpu/softfloat.c
     2     16.445%  soft_f64_mul                    &lt;qemu&gt;/fpu/softfloat.c
     3     10.643%  0x0000000008664000              ???
     4      6.685%  ieee_ex_to_mips.part.2          &lt;qemu&gt;/target/mips/fpu_helper.c
     5      6.340%  soft_f64_mul                    &lt;qemu&gt;/include/fpu/softfloat-macros.h
     6      3.312%  float64_add                     &lt;qemu&gt;/fpu/softfloat.c
     7      3.284%  <b>helper_float_mul_d</b>              &lt;qemu&gt;/target/mips/fpu_helper.c
     8      3.274%  soft_f64_addsub                 &lt;qemu&gt;/include/qemu/bitops.h
     9      3.197%  <b>helper_float_madd_d</b>             &lt;qemu&gt;/target/mips/fpu_helper.c
    10      3.011%  <b>helper_float_sub_d</b>              &lt;qemu&gt;/target/mips/fpu_helper.c
    11      2.753%  <b>helper_float_add_d</b>              &lt;qemu&gt;/target/mips/fpu_helper.c
    12      2.676%  soft_f64_mul                    &lt;qemu&gt;/include/qemu/bitops.h
    13      2.454%  soft_f64_addsub                 &lt;qemu&gt;/include/fpu/softfloat-macros.h
    14      1.606%  float64_sub                     &lt;qemu&gt;/fpu/softfloat.c
    15      1.190%  <b>helper_cmp_d_lt</b>                 &lt;qemu&gt;/target/mips/fpu_helper.c
  </code>
</pre>

And for mipsel target:

```bash
./topN_callgrind.py -n 15 -- <qemu-build>/mipsel-linux-user/qemu-mipsel coulomb_double-mipsel
```

Results:

<pre class="highlight" style="font-size: 14.6205px;">
  <code>
   No.  Percentage  Function Name                   Source File
  ----  ----------  ------------------------------  ------------------------------
     1     26.635%  soft_f64_addsub                 &lt;qemu&gt;/fpu/softfloat.c
     2     14.656%  soft_f64_mul                    &lt;qemu&gt;/fpu/softfloat.c
     3     10.012%  0x0000000008664000              ???
     4      7.559%  ieee_ex_to_mips.part.2          &lt;qemu&gt;/target/mips/fpu_helper.c
     5      5.650%  soft_f64_mul                    &lt;qemu&gt;/include/fpu/softfloat-macros.h
     6      5.584%  <b>helper_float_mul_d</b>              &lt;qemu&gt;/target/mips/fpu_helper.c
     7      4.603%  <b>helper_float_add_d</b>              &lt;qemu&gt;/target/mips/fpu_helper.c
     8      3.929%  soft_f64_addsub                 &lt;qemu&gt;/include/qemu/bitops.h
     9      3.299%  soft_f64_addsub                 &lt;qemu&gt;/include/fpu/softfloat-macros.h
    10      3.247%  <b>helper_float_sub_d</b>              &lt;qemu&gt;/target/mips/fpu_helper.c
    11      2.385%  soft_f64_mul                    &lt;qemu&gt;/include/qemu/bitops.h
    12      1.060%  <b>helper_cmp_d_lt</b>                 &lt;qemu&gt;/target/mips/fpu_helper.c
    13      1.036%  float64_lt                      &lt;qemu&gt;/fpu/softfloat.c
    14      0.946%  float64_add                     &lt;qemu&gt;/fpu/softfloat.c
    15      0.901%  soft_f64_div                    &lt;qemu&gt;/fpu/softfloat.c
  </code>
</pre>

From the two lists above, it is visible that, for some strange reasons that are beyond QEMU, big endian mips target uses multiply-add instructions, while little endian mips target uses separate multiply instruction and separate add instructions. This can be concluded from the presence of `helper_float_madd_d` in big endian case only. This means that corresponding helpers are different. Moreover, the number of executed helpers will be also different - less helpers will be called in big endian case. Numerically, the outcome will be accurate in both cases, however, the number of invoked helpers matters, resulting in better overall performance of big endian mips target.

This is not really a QEMU issue, it could be claimed that cross compiler for mips exhibits strange differences between big endian and little endian cases.

### m68k (non-RISC) target vs RISC targets

m68k instruction set is not a RISC set in a strict sense. For example it contains instructions that calculate mathematical function `sin()` - which is not present in, for example, mips or arm instruction sets. Again, let's examine top 15 functions for m68k case:

```bash
./topN_callgrind.py -n 15 -- <qemu-build>/m68k-linux-user/qemu-m68k coulomb_double-m68k
```

Results:

<pre class="highlight" style="font-size: 14.6205px;">
  <code>
   No.  Percentage  Function Name                   Source File
  ----  ----------  ------------------------------  ------------------------------
     1     21.128%  roundAndPackFloatx80            &lt;qemu&gt;/fpu/softfloat.c
     2      6.646%  floatx80_mul                    &lt;qemu&gt;/fpu/softfloat.c
     3      5.904%  0x00000000082db000              ???
     4      5.542%  floatx80_mul                    &lt;qemu&gt;/include/fpu/softfloat-macros.h
     5      3.958%  subFloatx80Sigs                 &lt;qemu&gt;/fpu/softfloat.c
     6      3.780%  helper_ftst                     &lt;qemu&gt;/target/m68k/fpu_helper.c
     7      3.739%  float64_to_floatx80             &lt;qemu&gt;/fpu/softfloat.c
     8      3.528%  addFloatx80Sigs                 &lt;qemu&gt;/fpu/softfloat.c
     9      2.447%  floatx80_div                    &lt;qemu&gt;/include/fpu/softfloat-macros.h
    10      2.437%  floatx80_mul                    &lt;qemu&gt;/include/fpu/softfloat.h
    11      2.136%  subFloatx80Sigs                 &lt;qemu&gt;/include/fpu/softfloat-macros.h
    12      2.072%  roundAndPackFloat64             &lt;qemu&gt;/fpu/softfloat.c
    13      1.900%  <b>floatx80_sin</b>                    &lt;qemu&gt;/target/m68k/softfloat.c
    14      1.890%  helper_ftst                     &lt;qemu&gt;/include/fpu/softfloat.h
    15      1.884%  <b>floatx80_cos</b>                    &lt;qemu&gt;/target/m68k/softfloat.c
  </code>
</pre>

m68k target has fewer instructions to translate, but some of its instruction require complex softfloat helpers (displayed above), whereas mips, for example, has much more instruction to translate and execute, but they are, in great majority, basically, additions and multiplications, that require relatively simple softfloat helpers.

All in all, m68k approach seems to be more efficient from the stand point of QEMU performance. There is nothing to improve in that sense for, let's say mips, this is just inherent consequence of differences of m68k and mips instruction sets.

---
layout: post
title: Measuring Basic Performance Metrics of QEMU
subtitle: Application of Perf and Callgrind
tags: [Perf, Callgrind, Basic Metrics, Top N]
excerpt: Welcome to the TCG Continuous Benchmarking project! This report presents two Linux profiling tools - Perf and Callgrind. It gives you an overview of their setup, usage, pros and cons. The motivation is to provide you with a better understanding of them. This way, you will be able to make decisions regarding their usage in a better and easier way. You will also learn two ways of finding top N most executed QEMU functions of your scenario, without even having to know absolutely any detail of the used profiling tools!
---

## Intro

Welcome to the TCG Continuous Benchmarking project! During this project, multiple Linux profiling tools will be used. They will help you profile your scenarios, and locate performance bottlenecks and performance regressions. A number of QEMU targets will be covered. Generally, a pragmatic, tool-agnostic approach will be followed in this project. This means a wide range of tools will be considered and the ones that are the most suitable for a given situation will be used.

This particular report presents two Linux profiling tools - Perf and Callgrind. It gives you an overview of their setup, usage, pros and cons. The motivation is to provide you with a better understanding of them. This way, you will be able to make decisions regarding their usage in a better and easier way. You will also learn two ways of finding top N most executed QEMU functions of your scenario, without even having to know absolutely any detail of the used profiling tools!

## Table of Contents

- [Measuring Basic Performance Metrics](#measuring-basic-performance-metrics)
  - [Prerequisites](#prerequisites)
  - [Measuring with Perf](#measuring-with-perf)
  - [Measuring with Callgrind](#measuring-with-callgrind)
- [Finding The 25 Most Executed Functions](#the-25-most-executed-functions)
  - [Using Perf](#using-perf)
  - [Using Callgrind](#using-callgrind)
- [Comparison of Perf and Callgrind Results](#comparison-of-perf-and-callgrind-results)
  - [Basic Performance Metrics](#basic-performance-metrics)
  - [Size of Examined Executable](#size-of-examined-executable)
  - [Source File Location](#source-file-location)
  - [JIT-ed Code Execution](#jit-ed-code-execution)
  - [Percentages of Individual Items](#percentages-of-individual-items)
- [Stability of Perf and Callgrind Results](#stability-of-perf-and-callgrind-results)
  - [Idea of the Experiment](#idea-of-the-experiment)
  - [Stability Experiment](#stability-experiment)
  - [Results of the Experiment](#results-of-the-experiment)
- [Resources](#resources)
  - [Perf Resources](#perf-resources)
  - [Callgrind Resources](#callgrind-resources)
- [Appendix](#appendix)
  - [Installing Perf](#installing-perf)
  - [Installing Valgrind](#installing-valgrind)

## Measuring Basic Performance Metrics

For the purpose of this report, basic performance metrics are defined as: number of instructions, number of branches, and number of branch misses that occurred while executing a particular scenario. Two methods for measuring them under Linux OS will be shown: one using Perf tool, and another using Callgrind tool.

**Perf**

[Perf](https://perf.wiki.kernel.org/index.php/Main_Page) is a profiler tool based on sampling and usage of CPU performance counters. It also provides per task, per CPU and per-workload counters, and source code event annotation as well. It depends to a great extent on kernel and CPU support. It does not instrument the code, so, consequently, exhibits a fast speed of execution, that is very close to the speed of the regular execution of executable or system that is observed.

**Callgrind**

Callgrind is a part of [Valgrind](http://www.valgrind.org/). Valgrind is an instrumentation framework for building dynamic analysis tools. It includes multiple tools, each covering its respective area. Callgrind is one of them, and it identifies the number of instructions executed for each line of source code, with per-function, per-module and whole-program summaries plus extra information about callers, callees, and call graphs for every function. It can also measure branch misses using its own simulation. Given that it’s based on instrumentation, Valgrind (in setup to use Callgrind as underlying tool) runs programs about 20–300x slower than normal, depending on the tool that is used, and numerous user-defined options.

### Prerequisites

1. Install Perf and Valgrind on your system (Callgrind will be installed as a prt of Valgrind). Please refer to the [Appendix](#appendix) for details.

2. Setup and build QEMU from source code. The methods presented here work for both debug and non-debug QEMU builds, but it makes the most sense to apply them on non-debug builds. In this report (and in all other reports in this series), QEMU source tree root directory will be denoted as `<qemu>`, and QEMU build directory as `<qemu-build>`.

3. Download the [Coulomb](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/blob/master/benchmakrs/coulomb/coulomb_double.c) benchmark which is used in this report. It computes the net forces acting on all **n** electrons randomly scattered across a 1m x 1m surface. **n** can be passed as a command line argument or, if not, it defaults to 1000.

4. Compile the program:

   ```bash
   gcc -static coulomb_double.c -o coulomb_double -lm
   ```

   The `-static` flag is used just for convenience - so that invocations of QEMU do not have to rely on giving them the path to target libraries.

### Measuring with Perf

Perf tool offers a rich set of subcommands to collect and analyze performance. To get the basic performance metrics of a program, the best option is subcommand `perf stat`. It runs a given executable and gathers performance data using CPU's performance counter statistics. By default, `perf stat` measures multiple metrics (in perf parlance called events), such as `task-clock`, `context-switches`, `cpu-migrations` and more. Please check the “[Perf Resources](#perf-resources)” section in to learn more about Perf.

Events displayed by Perf can be specified using the `-e, --event <event>` argument. To only measure the number of instructions, branches and branch-misses, Perf can be run using:

```bash
sudo perf stat -e instructions,branches,branch-misses <qemu-build>/x86_64-linux-user/qemu-x86_64 coulomb_double
```

And the output is:

```
     8,184,824,850      instructions
     1,968,845,899      branches
        25,016,032      branch-misses             #    1.27% of all branches

       0.846118331 seconds time elapsed

       0.846212000 seconds user
       0.000000000 seconds sys
```

### Measuring with Callgrind

The general command line form for running a program with Callgrind is the following:

```bash
valgrind --tool=callgrind [callgrind options] program [program options]
```

Using the same [Coulomb](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/blob/master/benchmakrs/coulomb/coulomb_double.c) program used with Perf, instructions and branch misses can be measured with Callgrind using the following command:

```bash
valgrind --tool=callgrind --branch-sim=yes <qemu-build>/x86_64-linux-user/qemu-x86_64 coulomb_double
```

Console output:

```
==26339== I   refs:      8,197,830,086
==26339==
==26339== Branches:      1,396,703,000  (1,385,648,906 cond + 11,054,094 ind)
==26339== Mispredicts:      12,692,618  (   10,503,444 cond +  2,189,174 ind)
==26339== Mispred rate:            0.9% (          0.8%     +       19.8%   )
```

`I refs` represents the number of instructions executed (and it’s always printed by default). Branches, branch misses and corresponding percentage can be seen below `I refs`.

Callgrind also produces a file in the current working directory named `callgrind.out.<pid>`. This data file contains information about the various performance statistics observed during the measurement. This file is inspected in the "[Finding Most Executed Functions](#most-executed-functions)" section of this report.

## The 25 Most Executed Functions

In this section, the most executed functions are measured using both Perf and Callgrind. This helps us pin point the program hotspots. Please make sure that you've followed all of the instructions in the "[Prerequisites](#prerequisites)" section before proceeding.

### Using Perf

Perf offers the `record` option which runs a program to be analyzed, collects profile data and stores the profile data into a file named `perf.data` in the current working directory.
Given that Perf is based on sampling, it's much better to have longer running programs to detect all running functions; so this time, the program is executed with an input of 30,000 electrons instead of the default 1000.

```bash
sudo perf record <qemu-build>/x86_64-linux-user/qemu-x86_64 coulomb_double -n 30000
```

To inspect the results, the `report` command can be used with the `--stdio` flag to specify Perf standard output display profile.

```bash
sudo perf report --stdio | head -n 36 | tail -n 28
```

The output is piped to `head` and `tail` to only display the top 25 functions which are sorted by percentage.

```
# Overhead  Command      Shared Object            Symbol
# ........  ...........  .......................  ..............................................
#
    18.80%  qemu-x86_64  qemu-x86_64              [.] float64_mul
    14.05%  qemu-x86_64  qemu-x86_64              [.] float64_add
    13.85%  qemu-x86_64  qemu-x86_64              [.] float64_sub
     6.06%  qemu-x86_64  qemu-x86_64              [.] helper_mulsd
     5.26%  qemu-x86_64  qemu-x86_64              [.] helper_addsd
     4.71%  qemu-x86_64  qemu-x86_64              [.] helper_subsd
     4.57%  qemu-x86_64  qemu-x86_64              [.] helper_lookup_tb_ptr
     3.08%  qemu-x86_64  qemu-x86_64              [.] f64_compare
     2.96%  qemu-x86_64  qemu-x86_64              [.] helper_ucomisd
     1.14%  qemu-x86_64  qemu-x86_64              [.] helper_pand_xmm
     0.81%  qemu-x86_64  qemu-x86_64              [.] float64_div
     0.52%  qemu-x86_64  qemu-x86_64              [.] helper_pxor_xmm
     0.37%  qemu-x86_64  qemu-x86_64              [.] helper_por_xmm
     0.36%  qemu-x86_64  qemu-x86_64              [.] float64_compare_quiet
     0.33%  qemu-x86_64  [JIT] tid 18993          [.] 0x00007f3784043840
     0.32%  qemu-x86_64  qemu-x86_64              [.] helper_cc_compute_all
     0.30%  qemu-x86_64  [JIT] tid 18993          [.] 0x00007f37840463c0
     0.29%  qemu-x86_64  [JIT] tid 18993          [.] 0x00007f3784043a80
     0.25%  qemu-x86_64  qemu-x86_64              [.] round_to_int
     0.24%  qemu-x86_64  [JIT] tid 18993          [.] 0x00007f3784046180
     0.22%  qemu-x86_64  qemu-x86_64              [.] soft_f64_addsub
     0.18%  qemu-x86_64  qemu-x86_64              [.] round_to_int_and_pack
     0.18%  qemu-x86_64  qemu-x86_64              [.] helper_cvttsd2si
     0.12%  qemu-x86_64  qemu-x86_64              [.] helper_divsd
     0.11%  qemu-x86_64  qemu-x86_64              [.] float64_to_int32_scalbn
     0.10%  qemu-x86_64  [JIT] tid 18993          [.] 0x00007f3784049115
     0.10%  qemu-x86_64  [JIT] tid 18993          [.] 0x00007f378403ec3b
     0.09%  qemu-x86_64  [JIT] tid 18993          [.] 0x00007f378403f003
     0.09%  qemu-x86_64  [JIT] tid 18993          [.] 0x00007f378403eb83
     0.08%  qemu-x86_64  qemu-x86_64              [.] sf_canonicalize
     0.07%  qemu-x86_64  [JIT] tid 18993          [.] 0x00007f378403d570
     0.07%  qemu-x86_64  [JIT] tid 18993          [.] 0x00007f378403d297
     0.07%  qemu-x86_64  qemu-x86_64              [.] helper_pandn_xmm
     0.07%  qemu-x86_64  [JIT] tid 18993          [.] 0x00007f37840463d3
     0.07%  qemu-x86_64  [JIT] tid 18993          [.] 0x00007f3784043a93
```

**Alternatively, you can run the [topN_perf](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/tree/master/tools/topN_perf) Python script from our GitHub repo. The arguments should match how you would normally execute the program with QEMU.**

```bash
python topN_perf.py -- <qemu-build>/x86_64-linux-user/qemu-x86_64 coulomb_double -n 30000
```

The script runs both `perf record` and `perf report` and prints the list of top functions.

```
 No.  Percentage  Name                       Caller
----  ----------  -------------------------  -------------------------
   1      16.25%  float64_mul                qemu-x86_64
   2      12.01%  float64_sub                qemu-x86_64
   3      11.99%  float64_add                qemu-x86_64
   4       5.69%  helper_mulsd               qemu-x86_64
   5       4.68%  helper_addsd               qemu-x86_64
   6       4.43%  helper_lookup_tb_ptr       qemu-x86_64
   7       4.28%  helper_subsd               qemu-x86_64
   8       2.71%  f64_compare                qemu-x86_64
   9       2.71%  helper_ucomisd             qemu-x86_64
  10       1.04%  helper_pand_xmm            qemu-x86_64
  11       0.71%  float64_div                qemu-x86_64
  12       0.63%  helper_pxor_xmm            qemu-x86_64
  13       0.50%  0x00007f7b7004ef95         [JIT] tid 491
  14       0.50%  0x00007f7b70044e83         [JIT] tid 491
  15       0.36%  helper_por_xmm             qemu-x86_64
  16       0.32%  helper_cc_compute_all      qemu-x86_64
  17       0.30%  0x00007f7b700433f0         [JIT] tid 491
  18       0.30%  float64_compare_quiet      qemu-x86_64
  19       0.27%  soft_f64_addsub            qemu-x86_64
  20       0.26%  round_to_int               qemu-x86_64
  21       0.25%  0x00007f7b7004c240         [JIT] tid 491
  22       0.25%  0x00007f7b70049900         [JIT] tid 491
  23       0.20%  0x00007f7b700496c0         [JIT] tid 491
  24       0.20%  0x00007f7b7004c000         [JIT] tid 491
  25       0.20%  0x00007f7b7004efbe         [JIT] tid 491
```

### Using Callgrind

Unlike Perf, Callgrind doesn’t need a relatively long-running programs to produce meaningful performance data. It works by instrumenting the program with extra instructions that record activity and keep counters. The results of such tracking is recorded in file named `callgrind.out.<pid>` (where `<pid>` is pid of the process being measured).

To view the contents of the file, you can use `callgrind_annotate` (an external utility shipped with Valgrind). If you’re following this report in order, you would already have a Callgrind output in the current working directory, if not, run the program with Callgrind as follows:

```bash
valgrind --tool=callgrind <qemu-build>/x86_64-linux-user/qemu-x86_64 coulomb_double
```

Now run `callgrind_annotate` with the Callgrind output where `pid` is the process of the Valgrind run.

```bash
callgrind_annotate callgrind.out.pid | head -n 50 | tail -n 28
```

The output is piped to `head` and `tail` to only display the top 25 functions which are sorted by the number of instructions.

```
--------------------------------------------------------------------------------
           Ir  file:function
--------------------------------------------------------------------------------
2,014,193,756  ???:0x00000000082db000 [???]
1,677,340,458  <qemu>/fpu/softfloat.c:float64_mul [<qemu-build>/x86_64-linux-user/qemu-x86_64]
1,206,367,069  <qemu>/fpu/softfloat.c:float64_sub [<qemu-build>/x86_64-linux-user/qemu-x86_64]
1,136,213,139  <qemu>/fpu/softfloat.c:float64_add [<qemu-build>/x86_64-linux-user/qemu-x86_64]
  399,610,730  <qemu>/target/i386/ops_sse.h:helper_mulsd [<qemu-build>/x86_64-linux-user/qemu-x86_64]
  308,725,510  <qemu>/target/i386/ops_sse.h:helper_subsd [<qemu-build>/x86_64-linux-user/qemu-x86_64]
  290,848,450  <qemu>/target/i386/ops_sse.h:helper_addsd [<qemu-build>/x86_64-linux-user/qemu-x86_64]
  179,112,825  <qemu>/target/i386/ops_sse.h:helper_ucomisd [<qemu-build>/x86_64-linux-user/qemu-x86_64]
  136,652,565  <qemu>/include/exec/tb-lookup.h:helper_lookup_tb_ptr
  136,174,015  <qemu>/fpu/softfloat.c:f64_compare [<qemu-build>/x86_64-linux-user/qemu-x86_64]
  123,638,928  <qemu>/accel/tcg/tcg-runtime.c:helper_lookup_tb_ptr [<qemu-build>/x86_64-linux-user/qemu-x86_64]
   52,058,289  <qemu>/include/exec/exec-all.h:helper_lookup_tb_ptr
   50,458,684  <qemu>/fpu/softfloat.c:float64_div [<qemu-build>/x86_64-linux-user/qemu-x86_64]
   41,182,050  <qemu>/target/i386/ops_sse.h:helper_pand_xmm [<qemu-build>/x86_64-linux-user/qemu-x86_64]
   41,131,601  <qemu>/include/fpu/softfloat.h:float64_mul
   39,043,872  <qemu>/target/i386/cpu.h:helper_lookup_tb_ptr
   35,822,565  <qemu>/fpu/softfloat.c:float64_compare_quiet [<qemu-build>/x86_64-linux-user/qemu-x86_64]
   33,919,580  <qemu>/target/i386/ops_sse.h:helper_pxor_xmm [<qemu-build>/x86_64-linux-user/qemu-x86_64]
   28,941,066  <qemu>/fpu/softfloat.c:round_to_int [<qemu-build>/x86_64-linux-user/qemu-x86_64]
   28,409,072  <qemu>/target/i386/cc_helper.c:helper_cc_compute_all [<qemu-build>/x86_64-linux-user/qemu-x86_64]
   20,854,735  <qemu>/fpu/softfloat.c:soft_f64_addsub [<qemu-build>/x86_64-linux-user/qemu-x86_64]
   19,778,659  <qemu>/tcg/tcg.c:liveness_pass_1 [<qemu-build>/x86_64-linux-user/qemu-x86_64]
   19,521,936  <qemu>/include/exec/tb-hash.h:helper_lookup_tb_ptr
   16,997,134  <qemu>/fpu/softfloat.c:round_to_int_and_pack [<qemu-build>/x86_64-linux-user/qemu-x86_64]
   15,259,670  <qemu>/target/i386/ops_sse.h:helper_por_xmm [<qemu-build>/x86_64-linux-user/qemu-x86_64]
```

**Alternatively, you can run the [topN_callgrind](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/tree/master/tools/topN_callgrind) Python script from our GitHub repo. The arguments should match how you would normally execute the program with QEMU.**

```bash
python topN_callgrind.py -- <qemu-build>/x86_64-linux-user/qemu-x86_64 coulomb_double
```

The script runs both `callgrind` and `callgrind_annotate` and prints a better formatted list of top functions, just like in the previous example with Perf.

```
 No.  Percentage  Name                       Source File
----  ----------  -------------------------  ------------------------------
   1     24.577%  0x00000000082db000         ???
   2     20.467%  float64_mul                <qemu>/fpu/softfloat.c
   3     14.720%  float64_sub                <qemu>/fpu/softfloat.c
   4     13.864%  float64_add                <qemu>/fpu/softfloat.c
   5      4.876%  helper_mulsd               <qemu>/target/i386/ops_sse.h
   6      3.767%  helper_subsd               <qemu>/target/i386/ops_sse.h
   7      3.549%  helper_addsd               <qemu>/target/i386/ops_sse.h
   8      2.185%  helper_ucomisd             <qemu>/target/i386/ops_sse.h
   9      1.667%  helper_lookup_tb_ptr       <qemu>/include/exec/tb-lookup.h
  10      1.662%  f64_compare                <qemu>/fpu/softfloat.c
  11      1.509%  helper_lookup_tb_ptr       <qemu>/accel/tcg/tcg-runtime.c
  12      0.635%  helper_lookup_tb_ptr       <qemu>/include/exec/exec-all.h
  13      0.616%  float64_div                <qemu>/fpu/softfloat.c
  14      0.502%  helper_pand_xmm            <qemu>/target/i386/ops_sse.h
  15      0.502%  float64_mul                <qemu>/include/fpu/softfloat.h
  16      0.476%  helper_lookup_tb_ptr       <qemu>/target/i386/cpu.h
  17      0.437%  float64_compare_quiet      <qemu>/fpu/softfloat.c
  18      0.414%  helper_pxor_xmm            <qemu>/target/i386/ops_sse.h
  19      0.353%  round_to_int               <qemu>/fpu/softfloat.c
  20      0.347%  helper_cc_compute_all      <qemu>/target/i386/cc_helper.c
  21      0.254%  soft_f64_addsub            <qemu>/fpu/softfloat.c
  22      0.238%  helper_lookup_tb_ptr       <qemu>/include/exec/tb-hash.h
  23      0.233%  liveness_pass_1            <qemu>/tcg/tcg.c
  24      0.207%  round_to_int_and_pack      <qemu>/fpu/softfloat.c
  25      0.186%  helper_por_xmm             <qemu>/target/i386/ops_sse.h
```

## Comparison of Perf and Callgrind Results

Perf's and Callgrind's underlying profiling methods are very different. As a consequence, the differences in their results are unavoidable, and, to an extent, expected. In some cases, these differences can be proved to be useful.

### Basic Performance Metrics

Instruction counts obtained by Perf and Callgrind tend to be very similar. Number of branches and number of branch misses, on the other hand, tend to differ to some extent. This is expected, since Perf uses CPU performance counters, while Callgrind uses internal simulation for these calculations, and, most likely, their very definition of branches is not the same. Interestingly enough, the percentage of branch misses are usually approximate for both tools.

### Size of Examined Executable

As noted before, Callgrind is capable of producing performance data for short-running executables, while Perf is not. Perf simply needs to reach certain reasonably large number of samples to be able to function.

### Source File Location

Callgrind provides information about the file where the source code of unction is located, unlike Perf, which doesn’t provide this kind of data.

Furthermore, it is possible (for example, in case a function contains some parts that are other functions inlined from other source files), that the actual source file of the functions spans over multiple source files. Callgrind makes this distinction, and reports such parts separately, while Perf reports just a single item for such function. The most notable example is `helper_lookup_tb_ptr()`.

### JIT-ed Code Execution

Perf provides highly granular data on JIT-ed code execution, while Callgrind sums all such cases into one item.

In previous example, these items can be found in Perf output:

```
   13       0.50%  0x00007f7b7004ef95         [JIT] tid 491
   14       0.50%  0x00007f7b70044e83         [JIT] tid 491
...
   17       0.30%  0x00007f7b700433f0         [JIT] tid 491
...
   21       0.25%  0x00007f7b7004c240         [JIT] tid 491
   22       0.25%  0x00007f7b70049900         [JIT] tid 491
   23       0.20%  0x00007f7b700496c0         [JIT] tid 491
   24       0.20%  0x00007f7b7004c000         [JIT] tid 491
   25       0.20%  0x00007f7b7004efbe         [JIT] tid 491
```

While in Callgrind, there is a single item:

```
    1     24.577%  0x00000000082db000         ???
```

Depending on what one wants to do with such data, it can be advantage or disadvantage. In next report, the fact that Callgrind sums up JIT-ed code execution will be used to extract some additional interesting performance metrics for QEMU.

### Percentages of Individual Items

Let's examine all items that surpassed 3% either in Perf or Calgrind results:

<style>
  /* Collapse multiple borders into one */
  table#compare {
    border-collapse: collapse;
  }
  /* Right align last two fields */
  table#compare td:nth-last-child(-n+2){
      text-align: right;
  }
  table#compare th:nth-last-child(-n+2){
      text-align: right;
  }
  /* Borders for table header */
  table#compare th {
    border: 1px solid black;
  }
  /* Borders for table data */
  table#compare td {
    border-right: 1px solid black;
    border-left: 1px solid black;
  }
  /* Borders for last table row */
  table#compare tr:nth-last-child(1) td{
    border-bottom: 1px solid black;
  }
</style>
<div style="overflow-x: auto;">
  <table id="compare">
    <thead>
      <tr>
        <th title="function-name">Function name</th>
        <th title="perf">Perf</th>
        <th title="callgrind">Callgrind</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>float64_mul</td>
        <td>16.25%</td>
        <td>20.467%</td>
      </tr>
      <tr>
        <td>float64_sub</td>
        <td>12.01%</td>
        <td>14.720%</td>
      </tr>
      <tr>
        <td>float64_add</td>
        <td>11.99%</td>
        <td>13.864%</td>
      </tr>
      <tr>
        <td>helper_mulsd</td>
        <td>5.69%</td>
        <td>4.876%</td>
      </tr>
      <tr>
        <td>helper_addsd</td>
        <td>4.68%</td>
        <td>3.549%</td>
      </tr>
      <tr>
        <td>helper_lookup_tb_ptr</td>
        <td>4.43%</td>
        <td>4.525%*</td>
      </tr>
      <tr>
        <td>helper_subsd</td>
        <td>4.28%</td>
        <td>3.767%</td>
      </tr>
    </tbody>
  </table>
</div>

_\* percentage for helper_lookup_tb_ptr for Callgrind is obtained by summing up several items._

It can be seen that the individual results are quite different. However it seems that relative relation between individual items is approximately the same for both tools.

For a performance engineer, the difference shown above does not make a significant problem. The performance improvement workflows, in general, usually focus on usage on only one tool, and both Perf and Callgrind can be used for such purpose.

In general, the more important factor while judging usability of a performance tool is its ability to provide the same or approximate results for multiple subsequent measurements of the identical scenarios. This factor, which is called stability for the purpose of this report, is examined in more depth in the following section.

## Stability of Perf and Callgrind Results

### Idea of the Experiment

Stability can be defined as the ability to provide nearly identical results with each run of the profiler.

A simple Python script is used to compare the stability of Valgrind vs Perf, but first, the [Coulomb](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/tree/master/benchmakrs/coulomb) program is executed with Callgrind once and with Perf three times.
This time, the `-r, --repeat <n>` Perf flag is utilized. It repeats Perf execution `n` times and prints the average of all events.

### Stability Experiment

This is a Bash script that performs an execution using `callgrind`, `perf`, `perf -r 10` and `perf -r 100` 20 times for the Coulomb benchmark:

```bash
mkdir output
for ((i = 0; i < 20; i++)); do
    valgrind --tool=callgrind <qemu-build>/x86_64-linux-user/qemu-x86_64 ./coulomb_double 2>>./output/out$i.txt &&
        sudo perf stat -e instructions <qemu-build>/x86_64-linux-user/qemu-x86_64 ./coulomb_double 2>>./output/out$i.txt &&
        sudo perf stat -e instructions -r 10 <qemu-build>/x86_64-linux-user/qemu-x86_64 ./coulomb_double 2>>./output/out$i.txt &&
        sudo perf stat -e instructions -r 100 <qemu-build>/x86_64-linux-user/qemu-x86_64 ./coulomb_double 2>>./output/out$i.txt
done
```

This is a Python script that extracts the instruction counts from each run and outputs a CSV file with the measurements, as well as, the average, standard deviation and coefficient of variation of all 20 executions for each of the four methods:

```python
from os import listdir
import csv
import statistics

output_files = listdir('output')
run = 1
results = []

for file in output_files:
    with open('output/' + file, "r") as target:
        lines = target.readlines()
        results.append([run,
                        lines[10].split()[3].replace(',', ' '),
                        lines[14].split()[0].replace(',', ' '),
                        lines[25].split()[0].replace(',', ' '),
                        lines[32].split()[0].replace(',', ' ')])
        run += 1

callgrind_results = [int(result[1].replace(' ', '')) for result in results]
callgrind_mean = statistics.mean(callgrind_results)
callgrind_stdev = statistics.stdev(callgrind_results)
callgrind_CV = (callgrind_stdev/callgrind_mean) * 100

perf_results = [int(result[2].replace(' ', '')) for result in results]
perf_mean = statistics.mean(perf_results)
perf_stdev = statistics.stdev(perf_results)
perf_CV = (perf_stdev / perf_mean) * 100

perf_10_results = [int(result[3].replace(' ', '')) for result in results]
perf_10_mean = statistics.mean(perf_10_results)
perf_10_stdev = statistics.stdev(perf_10_results)
perf_10_CV = (perf_10_stdev / perf_10_mean) * 100

perf_100_results = [int(result[4].replace(' ', '')) for result in results]
perf_100_mean = statistics.mean(perf_100_results)
perf_100_stdev = statistics.stdev(perf_100_results)
perf_100_CV = (perf_100_stdev/perf_100_mean) * 100

with open('output.csv', 'w') as csv_file:
    writer = csv.writer(csv_file)
    writer.writerow(["Run", "callgrind", "perf",
                     "perf -r 10", "perf -r 100"])
    for result in results:
        writer.writerow(result)
    writer.writerow(["Avg", callgrind_mean, perf_mean,
                     perf_10_mean, perf_100_mean])
    writer.writerow(["σ", callgrind_stdev, perf_stdev,
                     perf_10_stdev, perf_100_stdev])
    writer.writerow(["σ (%)", callgrind_CV, perf_CV,
                     perf_10_CV, perf_100_CV])
```

### Results of the Experiment

<style>
  /* Right align all table fields */
  table#results td:nth-last-child(-n + 4) {
    text-align: right;
  }
  table#results th:nth-last-child(-n + 4) {
    text-align: right;
  }
  /* Borders for table header */
  table#results th:nth-child(-n + 2) {
    border-right: 1px solid black;
  }
  table#results th:last-child {
    border-right: 1px solid black;
  }
  table#results th:first-child {
    border-left: 1px solid black;
  }
  table#results th {
    border-top: 1px solid black;
    border-bottom: 1px solid black;
  }
  /* Borders for table data */
  table#results td:nth-child(-n + 2) {
    border-right: 1px solid black;
  }
  table#results td:last-child {
    border-right: 1px solid black;
  }
  table#results td:first-child {
    border-left: 1px solid black;
    width: 10%;
  }
  /* Borders for last table row */
  table#results tr:nth-last-child(-n + 4) td {
    border-bottom: 1px solid black;
  }
</style>

<div style="overflow-x: auto;">
  <table id="results" style="width: 100%;">
    <thead>
      <tr>
        <th>Run</th>
        <th>callgrind</th>
        <th>perf</th>
        <th>perf -r 10</th>
        <th>perf -r 100</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>1</td>
        <td>8 197 479 927</td>
        <td>8 185 411 320</td>
        <td>8 185 345 224</td>
        <td>8 185 114 468</td>
      </tr>
      <tr>
        <td>2</td>
        <td>8 197 479 919</td>
        <td>8 185 195 094</td>
        <td>8 184 991 767</td>
        <td>8 184 923 223</td>
      </tr>
      <tr>
        <td>3</td>
        <td>8 197 479 842</td>
        <td>8 184 968 958</td>
        <td>8 185 206 200</td>
        <td>8 185 084 344</td>
      </tr>
      <tr>
        <td>4</td>
        <td>8 197 479 842</td>
        <td>8 185 162 965</td>
        <td>8 185 236 582</td>
        <td>8 184 992 738</td>
      </tr>
      <tr>
        <td>5</td>
        <td>8 197 479 919</td>
        <td>8 185 173 560</td>
        <td>8 185 230 524</td>
        <td>8 185 177 606</td>
      </tr>
      <tr>
        <td>6</td>
        <td>8 197 479 919</td>
        <td>8 185 107 760</td>
        <td>8 185 089 134</td>
        <td>8 184 985 563</td>
      </tr>
      <tr>
        <td>7</td>
        <td>8 197 479 927</td>
        <td>8 185 242 718</td>
        <td>8 185 067 722</td>
        <td>8 184 933 899</td>
      </tr>
      <tr>
        <td>8</td>
        <td>8 197 479 919</td>
        <td>8 185 259 192</td>
        <td>8 185 356 351</td>
        <td>8 185 202 083</td>
      </tr>
      <tr>
        <td>9</td>
        <td>8 197 479 927</td>
        <td>8 185 176 054</td>
        <td>8 184 955 923</td>
        <td>8 185 048 778</td>
      </tr>
      <tr>
        <td>10</td>
        <td>8 197 479 842</td>
        <td>8 185 062 738</td>
        <td>8 185 061 203</td>
        <td>8 185 148 177</td>
      </tr>
      <tr>
        <td>11</td>
        <td>8 197 479 842</td>
        <td>8 185 065 330</td>
        <td>8 184 952 823</td>
        <td>8 184 926 132</td>
      </tr>
      <tr>
        <td>12</td>
        <td>8 197 479 852</td>
        <td>8 185 370 367</td>
        <td>8 185 303 501</td>
        <td>8 185 125 697</td>
      </tr>
      <tr>
        <td>13</td>
        <td>8 197 479 927</td>
        <td>8 185 115 629</td>
        <td>8 184 950 032</td>
        <td>8 185 001 142</td>
      </tr>
      <tr>
        <td>14</td>
        <td>8 197 479 842</td>
        <td>8 186 555 638</td>
        <td>8 185 187 143</td>
        <td>8 185 035 473</td>
      </tr>
      <tr>
        <td>15</td>
        <td>8 197 481 000</td>
        <td>8 185 159 045</td>
        <td>8 185 238 812</td>
        <td>8 185 032 286</td>
      </tr>
      <tr>
        <td>16</td>
        <td>8 197 479 961</td>
        <td>8 185 170 493</td>
        <td>8 185 029 578</td>
        <td>8 185 120 424</td>
      </tr>
      <tr>
        <td>17</td>
        <td>8 197 479 842</td>
        <td>8 185 125 155</td>
        <td>8 184 990 595</td>
        <td>8 184 922 657</td>
      </tr>
      <tr>
        <td>18</td>
        <td>8 197 480 006</td>
        <td>8 187 323 418</td>
        <td>8 185 326 603</td>
        <td>8 185 092 509</td>
      </tr>
      <tr>
        <td>19</td>
        <td>8 197 479 919</td>
        <td>8 185 454 074</td>
        <td>8 185 320 982</td>
        <td>8 185 173 455</td>
      </tr>
      <tr>
        <td>20</td>
        <td>8 197 481 000</td>
        <td>8 185 164 000</td>
        <td>8 185 206 455</td>
        <td>8 185 044 700</td>
      </tr>
      <tr>
        <td><strong>Avg</strong></td>
        <td><strong>8 197 480 009</strong></td>
        <td><strong>8 185 363 175</strong></td>
        <td><strong>8 185 152 358</strong></td>
        <td><strong>8 185 054 268</strong></td>
      </tr>
      <tr>
        <td><strong>σ</strong></td>
        <td><strong>342.272</strong></td>
        <td><strong>565489.783</strong></td>
        <td><strong>144003.202</strong></td>
        <td><strong>89808.068</strong></td>
      </tr>
      <tr>
        <td><strong>σ (%)</strong></td>
        <td><strong>0.0000041</strong></td>
        <td><strong>0.0069085</strong></td>
        <td><strong>0.0017593</strong></td>
        <td><strong>0.0010972</strong></td>
      </tr>
    </tbody>
  </table>
</div>

From the previous experiment, it can be seen that σ (%) - Coefficient of Variation - is approx 0.0000041% for Callgrind. For Perf, it is 0.0069085% for `perf stat` used without any `-r` switch. It starts to decrease to 0.0017593% for `-r 10`, and further to 0.0010972% for `-r 100`. The maximal value that can be specified after `-r` is 100, so this is the maximal stability that can be achieved using Perf.

It can be concluded that despite (or perhaps, better said, because of) the slow execution time of Callgrind, it gives very stable results. The stability of Perf results increases with increasing the repetition count, but it still doesn’t reach a coefficient of variation as low as Callgrind even with the maximum possible repetitions (-r 100).

## Resources

If you want to learn more about Perf and Callgrind, please check the resources section below.

### Perf Resources

- [Official Perf Wiki](https://perf.wiki.kernel.org/index.php/Tutorial)

  The official Perf wiki offers a detailed step by step tutorial on the usage of Perf. It lists all of the measurable hardware and software events as well as multiple examples for Perf command usages.

- [Performance Lab: The Power of The Perf Tool - Arnaldo Melo and Jiri Olsa](https://youtu.be/qfmAMzCW3Mk)

  In this talk, the speakers show how to build Perf from source and how it can be used to detect and hunt down numerous performance issues. They also cover examples of some interesting Perf features and their favorite usage tips.

* [Linux Perf for Qt Developers - Milian Wolff](https://youtu.be/L4NClVxqdMw)

  In this talk, the speaker gives a detailed introduction to Perf showing how to use it to find CPU hotspots in the code, as well as some tricks to profile wait times for lock contention issues or disk I/O. He also dives into details on how it is applicable to Qt developers in particular.

### Callgrind Resources

- [Callgrind Official Manual](https://www.valgrind.org/docs/manual/cl-manual.html)

  The official Callgrind manual gives an overview of Callgrind. It provides a guide for basic and advanced usage of the tool as well as in detailed description of all command line arguments of the Callgrind.

- [Callgrind Output Format Manual](https://www.cs.cmu.edu/afs/cs.cmu.edu/project/cmt-40/Nice/RuleRefinement/bin/valgrind-3.2.0/docs/html/cl-format.html)

  This manual covers the internal structure of the Callgrind output file which callgrind_annotate was used to inspect in the report. It includes simple and extended examples of such files as well as the complete grammar of the format.

- [Stanford CS107 Callgrind Guide](https://web.stanford.edu/class/archive/cs/cs107/cs107.1174/guide_callgrind.html)

  This guide offers a quick introduction to get you up and running with Callgrind. It covers basic usage cases as well as some tips and tricks.

<br />
<hr />

## Appendix

### Installing Perf

**CentOS & RHEL**

```bash
sudo yum install perf
```

**Fedora**

```bash
sudo dnf install perf
```

**Arch**

```bash
sudo pacman -S perf
```

**Debian & Derivatives**

```bash
sudo apt update && sudo apt install linux-tools-$(uname -r) linux-tools-generic
```

The `uname -r` command is used to provide the Linux kernel version instead of manually writing it in the installation command.

### Installing Valgrind

**CentOS & RHEL**

```bash
sudo yum install valgrind
```

**Fedora**

```bash
sudo dnf install valgrind
```

**Arch**

```bash
sudo pacman -S valgrind
```

**Debian & Derivatives**

```bash
sudo apt install valgrind
```

<hr />

Apart from the procedures mentioned above, and for curious and advanced users, or simply those wishing the latest and greatest, both Perf and Valgrind can be also built and installed from their source code.

<hr />

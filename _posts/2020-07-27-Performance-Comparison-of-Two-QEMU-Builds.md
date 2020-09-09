---
layout: post
title: Performance Comparison of Two QEMU Builds (Compilers and QEMU Performance)
subtitle: GCC VS Clang
tags: [GCC, Clang, list_helpers, list_fn_callees]
excerpt: This report presents a performance comparison between two different QEMU builds, GCC and Clang. To provide a variety of test workloads, five new benchmarks are also introduced in the report. For each benchmark, the performance of QEMU is compared across the two builds for seventeen different targets.
---

## Intro

This report presents a performance comparison between two different QEMU builds, GCC and Clang. To provide a variety of test workloads, five new benchmarks are also introduced in the report. For each benchmark, the performance of QEMU is compared across the two builds for seventeen different targets.

The compiler versions used in the report are the default available versions in the Ubuntu 18.0 repositories which are 7.5.0 for GCC and 6.0.0 for Clang.

## Table of Contents

- [Benchmarks Overview](#benchmarks-overview)
- [Setup and Prerequisites](#setup-and-prerequisites)
- [Performance Tables](#performance-tables)
- [Analysis of Results](#analysis-of-results)
  - [Comparison Script](#comparison-script)
  - [Floating Point Benchmarks](#floating-point-benchmarks)
  - [Integer and String Benchmarks](#integer-and-string-benchmarks)
- [QEMU Binary Size](#qemu-binary-size)
- [Appendix](#appendix)

## Benchmarks Overview

This section gives a quick overview of the five new benchmarks used in the report. All benchmarks are available on the project [GitHub page](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/blob/master/benchmarks/).

**matmult_double:**

Standard matrix multiplication of an n\*n matrix of randomly generated double numbers from 0 to 100. The value of n is passed as an argument with the `-n` flag. The default value is 200.

**matmult_int32:**

Standard matrix multiplication of an n\*n matrix of randomly generated integer numbers from 0 to 100. The value of n is passed as an argument with the `-n` flag. The default value is 200.

**qsort_double:**

Quick sort of an array of n randomly generated double numbers from 0 to 1000. The value of n is passed as an argument with the `-n` flag. The default value is 300000.

**qsort_int32:**

Quick sort of an array of n randomly generated integer numbers from 0 to 50000000. The value of n is passed as an argument with the `-n` flag. The default value is 300000.

**qsort_string:**

Quick sort of an array of 10000 randomly generated strings of size 8 (including null terminating character). The sort process is repeated n number of times. The value of n is passed as an argument with the `-n` flag. The default value is 20.

## Setup and Prerequisites

All previous reports assumed a GCC build on top of QEMU 5.0.0. Given that QEMU 5.1.0-rc1 is now released, the report will be based on this newer QEMU version.

To download and make the GCC and Clang builds for QEMU, you can run the bash snippet below:

```bash
wget https://download.qemu.org/qemu-5.1.0-rc1.tar.xz
tar xfv qemu-5.1.0-rc1.tar.xz
cd qemu-5.1.0-rc1
mkdir build-gcc
cd build-gcc
../configure
make
cd ..
mkdir build-clang
../configure --cc=clang --cxx=clang++
make
```

The report will assume that the GCC build is at `<qemu-gcc-build>` and Clang build is at `<qemu-clang-build>`.

To measure the performance of the seventeen targets for all of the five benchmarks and the two QEMU builds, the Python script below is used. The script runs `dissect.py` (which was modified to print the number of instructions instead of percentages) on each of the five benchmarks for the two different builds. The results are ten tables, two for each benchmark.

The script assumes that all five benchmarks are in a `benchmarks` directory.

```python
import csv
import os
import subprocess


############### Script Options ###############
builds = {
    "gcc": "<qemu-gcc-build>",
    "clang": "<qemu-clang-build>"
}
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

benchmarks = sorted(os.listdir("benchmarks"))

csv_headers = ["Target", "Total Instructions",
               "Code Generation", "JIT Execution", "Helpers"]


for benchmark in benchmarks:
    # Remove file extension
    benchmark_name = os.path.splitext(benchmark)[0]

    for build_name, _ in builds.items():
        with open("tables/{}-{}.csv".format(benchmark_name, build_name), "w") as csv_file:
            writer = csv.writer(csv_file)
            writer.writerow(csv_headers)

    for target_name, target_compiler in targets.items():
        compile_target = subprocess.run([target_compiler,
                                         "-O2",
                                         "-static",
                                         "benchmarks/"+benchmark,
                                         "-o",
                                         "/tmp/" + benchmark_name])

        for build_name, build_path in builds.items():
            dissect_target = subprocess.run(["./dissect.py",
                                             "--",
                                             "{}/{}-linux-user/qemu-{}".
                                             format(build_path,
                                                    target_name,
                                                    target_name),
                                             "/tmp/" + benchmark_name],
                                            stdout=subprocess.PIPE)
            # Read the dissect output
            lines = dissect_target.stdout.decode("utf-8").split('\n')

            # Extract measurements
            total_instructions = lines[0].split()[-1]
            code_generation = lines[2].split()[-2]
            jit_execution = lines[3].split()[-2]
            helpers_execution = lines[4].split()[-2]

            # Save output to CSV
            with open("tables/{}-{}.csv".format(benchmark_name, build_name), "a") as csv_file:
                writer = csv.writer(csv_file)
                writer.writerow([target_name,
                                 total_instructions,
                                 code_generation,
                                 jit_execution,
                                 helpers_execution])
    # Remove temporary file
    os.unlink("/tmp/" + benchmark_name)
```

The ten resulting tables are shown in the next section. For Clang tables, numbers in green indicate a decrease in the number of instructions, and numbers in red indicate otherwise.

## Performance Tables

<style>
  /* Right align all table fields */
  table.results td:nth-last-child(-n + 4) {
    text-align: right;
  }
  table.results th:nth-last-child(-n + 4) {
    text-align: right;
  }
  /* Borders for table header */
  table.results th {
    border: 1px solid black;
  }
  /* Borders for table data */
  table.results td {
    border-right: 1px solid black;
  }
  table.results td:first-child {
    border-left: 1px solid black;
  }
  /* Borders for last table row */
  table.results tr:nth-last-child(-n + 1) td {
    border-bottom: 1px solid black;
  }
</style>

### matmult_double (GCC)

<div style="overflow-x: auto;">
  <table class="results" style="width: 100%;">
    <thead>
      <tr>
        <th title="Field #1">Target</th>
        <th title="Field #2">Total Instructions</th>
        <th title="Field #3">Code Generation</th>
        <th title="Field #4">JIT Execution</th>
        <th title="Field #5">Helpers</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td>1 411 957 967</td>
        <td>76 120 077</td>
        <td>611 067 999</td>
        <td>724 769 891</td>
      </tr>
      <tr>
        <td>alpha</td>
        <td>3 020 752 954</td>
        <td>56 143 853</td>
        <td>437 106 648</td>
        <td>2 527 502 453</td>
      </tr>
      <tr>
        <td>arm</td>
        <td>8 721 987 230</td>
        <td>285 174 602</td>
        <td>6 633 909 955</td>
        <td>1 802 902 673</td>
      </tr>
      <tr>
        <td>hppa</td>
        <td>3 345 168 851</td>
        <td>179 176 839</td>
        <td>348 961 284</td>
        <td>2 817 030 728</td>
      </tr>
      <tr>
        <td>m68k</td>
        <td>3 327 223 458</td>
        <td>65 872 033</td>
        <td>547 273 911</td>
        <td>2 714 077 514</td>
      </tr>
      <tr>
        <td>mips</td>
        <td>2 262 859 230</td>
        <td>79 312 482</td>
        <td>362 219 758</td>
        <td>1 821 326 990</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td>3 176 135 194</td>
        <td>79 192 739</td>
        <td>402 266 174</td>
        <td>2 694 676 281</td>
      </tr>
      <tr>
        <td>mips64</td>
        <td>2 276 881 008</td>
        <td>87 067 585</td>
        <td>364 561 337</td>
        <td>1 825 252 086</td>
      </tr>
      <tr>
        <td>mips64el</td>
        <td>3 189 604 541</td>
        <td>86 891 148</td>
        <td>404 111 884</td>
        <td>2 698 601 509</td>
      </tr>
      <tr>
        <td>ppc</td>
        <td>3 125 097 209</td>
        <td>82 035 165</td>
        <td>338 132 356</td>
        <td>2 704 929 688</td>
      </tr>
      <tr>
        <td>ppc64</td>
        <td>3 202 929 198</td>
        <td>88 654 395</td>
        <td>379 358 099</td>
        <td>2 734 916 704</td>
      </tr>
      <tr>
        <td>ppc64le</td>
        <td>3 202 302 439</td>
        <td>88 841 776</td>
        <td>378 540 677</td>
        <td>2 734 919 986</td>
      </tr>
      <tr>
        <td>riscv64</td>
        <td>1 222 310 471</td>
        <td>60 743 087</td>
        <td>305 444 092</td>
        <td>856 123 292</td>
      </tr>
      <tr>
        <td>s390x</td>
        <td>2 726 250 005</td>
        <td>57 481 812</td>
        <td>318 742 202</td>
        <td>2 350 025 991</td>
      </tr>
      <tr>
        <td>sh4</td>
        <td>3 341 872 364</td>
        <td>67 475 044</td>
        <td>602 524 473</td>
        <td>2 671 872 847</td>
      </tr>
      <tr>
        <td>sparc64</td>
        <td>3 205 825 118</td>
        <td>134 723 352</td>
        <td>501 019 705</td>
        <td>2 570 082 061</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td>1 249 941 832</td>
        <td>67 613 673</td>
        <td>364 007 574</td>
        <td>818 320 585</td>
      </tr>
    </tbody>
  </table>
</div>

### matmult_double (Clang)

<div style="overflow-x: auto;">
  <table class="results" style="width: 100%;">
    <thead>
      <tr>
        <th title="Field #1">Target</th>
        <th title="Field #2">Total Instructions</th>
        <th title="Field #3">Code Generation</th>
        <th title="Field #4">JIT Execution</th>
        <th title="Field #5">Helpers</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td style="color: #d7191c;">2 011 166 477</td>
        <td style="color: #4dac26;">71 984 566</td>
        <td>611 067 999</td>
        <td style="color: #d7191c;">1 328 113 912</td>
      </tr>
      <tr>
        <td>alpha</td>
        <td style="color: #d7191c;">4 673 820 741</td>
        <td style="color: #4dac26;">55 207 732</td>
        <td>437 106 648</td>
        <td style="color: #d7191c;">4 181 506 361</td>
      </tr>
      <tr>
        <td>arm</td>
        <td style="color: #d7191c;">8 746 454 169</td>
        <td style="color: #4dac26;">276 672 219</td>
        <td>6 633 909 955</td>
        <td style="color: #d7191c;">1 835 871 995</td>
      </tr>
      <tr>
        <td>hppa</td>
        <td style="color: #d7191c;">4 770 815 514</td>
        <td style="color: #4dac26;">175 521 902</td>
        <td>348 961 284</td>
        <td style="color: #d7191c;">4 246 332 328</td>
      </tr>
      <tr>
        <td>m68k</td>
        <td style="color: #d7191c;">3 542 927 301</td>
        <td style="color: #4dac26;">65 164 233</td>
        <td>547 273 911</td>
        <td style="color: #d7191c;">2 930 489 157</td>
      </tr>
      <tr>
        <td>mips</td>
        <td style="color: #d7191c;">3 738 305 471</td>
        <td style="color: #4dac26;">72 287 768</td>
        <td>362 219 758</td>
        <td style="color: #d7191c;">3 303 797 945</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td style="color: #d7191c;">5 239 048 897</td>
        <td style="color: #4dac26;">72 499 086</td>
        <td>402 266 174</td>
        <td style="color: #d7191c;">4 764 283 637</td>
      </tr>
      <tr>
        <td>mips64</td>
        <td style="color: #d7191c;">3 750 825 939</td>
        <td style="color: #4dac26;">77 896 420</td>
        <td>364 561 337</td>
        <td style="color: #d7191c;">3 308 368 182</td>
      </tr>
      <tr>
        <td>mips64el</td>
        <td style="color: #d7191c;">5 251 030 988</td>
        <td style="color: #4dac26;">78 065 089</td>
        <td>404 111 884</td>
        <td style="color: #d7191c;">4 768 854 015</td>
      </tr>
      <tr>
        <td>ppc</td>
        <td style="color: #d7191c;">4 109 476 668</td>
        <td style="color: #4dac26;">81 662 092</td>
        <td>338 132 356</td>
        <td style="color: #d7191c;">3 689 682 220</td>
      </tr>
      <tr>
        <td>ppc64</td>
        <td style="color: #d7191c;">4 170 489 431</td>
        <td style="color: #4dac26;">87 779 750</td>
        <td>379 358 099</td>
        <td style="color: #d7191c;">3 703 351 582</td>
      </tr>
      <tr>
        <td>ppc64le</td>
        <td style="color: #d7191c;">4 169 627 378</td>
        <td style="color: #4dac26;">87 731 465</td>
        <td>378 540 677</td>
        <td style="color: #d7191c;">3 703 355 236</td>
      </tr>
      <tr>
        <td>riscv64</td>
        <td style="color: #d7191c;">1 799 067 648</td>
        <td style="color: #4dac26;">59 198 155</td>
        <td>305 444 092</td>
        <td style="color: #d7191c;">1 434 425 401</td>
      </tr>
      <tr>
        <td>s390x</td>
        <td style="color: #d7191c;">3 924 840 571</td>
        <td style="color: #d7191c;">58 080 261</td>
        <td>318 742 202</td>
        <td style="color: #d7191c;">3 548 018 108</td>
      </tr>
      <tr>
        <td>sh4</td>
        <td style="color: #d7191c;">5 069 649 275</td>
        <td style="color: #4dac26;">65 326 027</td>
        <td>602 524 473</td>
        <td style="color: #d7191c;">4 401 798 775</td>
      </tr>
      <tr>
        <td>sparc64</td>
        <td style="color: #d7191c;">4 918 273 993</td>
        <td style="color: #4dac26;">131 200 185</td>
        <td>501 019 705</td>
        <td style="color: #d7191c;">4 286 054 103</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td style="color: #d7191c;">2 282 484 944</td>
        <td style="color: #4dac26;">66 749 639</td>
        <td>364 007 574</td>
        <td style="color: #d7191c;">1 851 727 731</td>
      </tr>
    </tbody>
  </table>
</div>

<br />
<hr />

### matmult_int32 (GCC)

<div style="overflow-x: auto;">
  <table class="results" style="width: 100%;">
    <thead>
      <tr>
        <th title="Field #1">Target</th>
        <th title="Field #2">Total Instructions</th>
        <th title="Field #3">Code Generation</th>
        <th title="Field #4">JIT Execution</th>
        <th title="Field #5">Helpers</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td>595 494 162</td>
        <td>69 380 056</td>
        <td>505 728 486</td>
        <td>20 385 620</td>
      </tr>
      <tr>
        <td>alpha</td>
        <td>370 047 513</td>
        <td>50 451 964</td>
        <td>307 209 296</td>
        <td>12 386 253</td>
      </tr>
      <tr>
        <td>arm</td>
        <td>735 549 496</td>
        <td>276 195 690</td>
        <td>410 431 931</td>
        <td>48 921 875</td>
      </tr>
      <tr>
        <td>hppa</td>
        <td>666 639 906</td>
        <td>171 974 055</td>
        <td>454 423 118</td>
        <td>40 242 733</td>
      </tr>
      <tr>
        <td>m68k</td>
        <td>406 939 070</td>
        <td>59 081 632</td>
        <td>329 037 444</td>
        <td>18 819 994</td>
      </tr>
      <tr>
        <td>mips</td>
        <td>497 096 141</td>
        <td>71 679 287</td>
        <td>416 946 656</td>
        <td>8 470 198</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td>497 010 030</td>
        <td>71 506 947</td>
        <td>417 032 765</td>
        <td>8 470 318</td>
      </tr>
      <tr>
        <td>mips64</td>
        <td>478 992 426</td>
        <td>78 298 258</td>
        <td>388 302 800</td>
        <td>12 391 368</td>
      </tr>
      <tr>
        <td>mips64el</td>
        <td>462 357 562</td>
        <td>78 074 494</td>
        <td>371 890 705</td>
        <td>12 392 363</td>
      </tr>
      <tr>
        <td>ppc</td>
        <td>338 417 174</td>
        <td>74 914 476</td>
        <td>255 198 622</td>
        <td>8 304 076</td>
      </tr>
      <tr>
        <td>ppc64</td>
        <td>390 129 095</td>
        <td>80 972 586</td>
        <td>297 023 711</td>
        <td>12 132 798</td>
      </tr>
      <tr>
        <td>ppc64le</td>
        <td>390 053 461</td>
        <td>80 990 281</td>
        <td>296 928 184</td>
        <td>12 134 996</td>
      </tr>
      <tr>
        <td>riscv64</td>
        <td>349 030 315</td>
        <td>54 578 504</td>
        <td>281 826 149</td>
        <td>12 625 662</td>
      </tr>
      <tr>
        <td>s390x</td>
        <td>491 822 152</td>
        <td>51 853 248</td>
        <td>375 436 514</td>
        <td>64 532 390</td>
      </tr>
      <tr>
        <td>sh4</td>
        <td>399 132 791</td>
        <td>59 096 149</td>
        <td>312 448 882</td>
        <td>27 587 760</td>
      </tr>
      <tr>
        <td>sparc64</td>
        <td>488 710 835</td>
        <td>123 407 698</td>
        <td>353 794 723</td>
        <td>11 508 414</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td>399 168 070</td>
        <td>61 136 986</td>
        <td>322 978 674</td>
        <td>15 052 410</td>
      </tr>
    </tbody>
  </table>
</div>

### matmult_int32 (Clang)

<div style="overflow-x: auto;">
  <table class="results" style="width: 100%;">
    <thead>
      <tr>
        <th title="Field #1">Target</th>
        <th title="Field #2">Total Instructions</th>
        <th title="Field #3">Code Generation</th>
        <th title="Field #4">JIT Execution</th>
        <th title="Field #5">Helpers</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td style="color: #4dac26;">593 751 025</td>
        <td style="color: #4dac26;">65 694 887</td>
        <td>505 728 486</td>
        <td style="color: #d7191c;">22 327 652</td>
      </tr>
      <tr>
        <td>alpha</td>
        <td style="color: #d7191c;">371 067 721</td>
        <td style="color: #4dac26;">50 014 259</td>
        <td>307 209 296</td>
        <td style="color: #d7191c;">13 844 166</td>
      </tr>
      <tr>
        <td>arm</td>
        <td style="color: #4dac26;">727 777 546</td>
        <td style="color: #4dac26;">267 935 229</td>
        <td>410 431 931</td>
        <td style="color: #d7191c;">49 410 386</td>
      </tr>
      <tr>
        <td>hppa</td>
        <td style="color: #d7191c;">664 473 385</td>
        <td style="color: #4dac26;">168 673 213</td>
        <td>454 423 118</td>
        <td style="color: #d7191c;">41 377 054</td>
      </tr>
      <tr>
        <td>m68k</td>
        <td style="color: #d7191c;">408 524 487</td>
        <td style="color: #4dac26;">58 641 545</td>
        <td>329 037 444</td>
        <td style="color: #d7191c;">20 845 498</td>
      </tr>
      <tr>
        <td>mips</td>
        <td style="color: #4dac26;">491 762 457</td>
        <td style="color: #4dac26;">65 531 396</td>
        <td>416 946 656</td>
        <td style="color: #d7191c;">9 284 405</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td style="color: #4dac26;">491 996 655</td>
        <td style="color: #4dac26;">65 679 356</td>
        <td>417 032 765</td>
        <td style="color: #d7191c;">9 284 534</td>
      </tr>
      <tr>
        <td>mips64</td>
        <td style="color: #4dac26;">472 458 134</td>
        <td style="color: #4dac26;">70 305 442</td>
        <td>388 302 800</td>
        <td style="color: #d7191c;">13 849 892</td>
      </tr>
      <tr>
        <td>mips64el</td>
        <td style="color: #4dac26;">456 176 576</td>
        <td style="color: #4dac26;">70 434 870</td>
        <td>371 890 705</td>
        <td style="color: #d7191c;">13 851 001</td>
      </tr>
      <tr>
        <td>ppc</td>
        <td style="color: #d7191c;">339 482 792</td>
        <td style="color: #d7191c;">75 166 396</td>
        <td>255 198 622</td>
        <td style="color: #d7191c;">9 117 774</td>
      </tr>
      <tr>
        <td>ppc64</td>
        <td style="color: #d7191c;">391 412 561</td>
        <td style="color: #4dac26;">80 800 275</td>
        <td>297 023 711</td>
        <td style="color: #d7191c;">13 588 575</td>
      </tr>
      <tr>
        <td>ppc64le</td>
        <td style="color: #d7191c;">391 115 113</td>
        <td style="color: #4dac26;">80 595 928</td>
        <td>296 928 184</td>
        <td style="color: #d7191c;">13 591 001</td>
      </tr>
      <tr>
        <td>riscv64</td>
        <td style="color: #d7191c;">349 721 940</td>
        <td style="color: #4dac26;">53 570 099</td>
        <td>281 826 149</td>
        <td style="color: #d7191c;">14 325 692</td>
      </tr>
      <tr>
        <td>s390x</td>
        <td style="color: #4dac26;">481 356 004</td>
        <td style="color: #d7191c;">52 685 433</td>
        <td>375 436 514</td>
        <td style="color: #4dac26;">53 234 057</td>
      </tr>
      <tr>
        <td>sh4</td>
        <td style="color: #d7191c;">399 736 614</td>
        <td style="color: #4dac26;">57 671 842</td>
        <td>312 448 882</td>
        <td style="color: #d7191c;">29 615 890</td>
      </tr>
      <tr>
        <td>sparc64</td>
        <td style="color: #4dac26;">486 470 314</td>
        <td style="color: #4dac26;">121 641 278</td>
        <td>353 794 723</td>
        <td style="color: #4dac26;">11 034 313</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td style="color: #d7191c;">399 194 875</td>
        <td style="color: #4dac26;">60 675 932</td>
        <td>322 978 674</td>
        <td style="color: #d7191c;">15 540 269</td>
      </tr>
    </tbody>
  </table>
</div>

<br />
<hr />

### qsort_double (GCC)

<div style="overflow-x: auto;">
  <table class="results" style="width: 100%;">
    <thead>
      <tr>
        <th title="Field #1">Target</th>
        <th title="Field #2">Total Instructions</th>
        <th title="Field #3">Code Generation</th>
        <th title="Field #4">JIT Execution</th>
        <th title="Field #5">Helpers</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td>2 658 154 250</td>
        <td>79 040 798</td>
        <td>1 265 579 424</td>
        <td>1 313 534 028</td>
      </tr>
      <tr>
        <td>alpha</td>
        <td>1 949 114 474</td>
        <td>57 399 955</td>
        <td>869 643 481</td>
        <td>1 022 071 038</td>
      </tr>
      <tr>
        <td>arm</td>
        <td>9 118 694 070</td>
        <td>850 895 346</td>
        <td>4 265 464 995</td>
        <td>4 002 333 729</td>
      </tr>
      <tr>
        <td>hppa</td>
        <td>3 138 372 515</td>
        <td>496 702 308</td>
        <td>1 023 062 954</td>
        <td>1 618 607 253</td>
      </tr>
      <tr>
        <td>m68k</td>
        <td>4 385 213 371</td>
        <td>63 926 269</td>
        <td>1 224 197 193</td>
        <td>3 097 089 909</td>
      </tr>
      <tr>
        <td>mips</td>
        <td>2 098 316 452</td>
        <td>82 146 339</td>
        <td>942 372 869</td>
        <td>1 073 797 244</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td>2 098 510 236</td>
        <td>81 980 330</td>
        <td>942 732 894</td>
        <td>1 073 797 012</td>
      </tr>
      <tr>
        <td>mips64</td>
        <td>1 970 640 902</td>
        <td>90 221 912</td>
        <td>787 224 479</td>
        <td>1 093 194 511</td>
      </tr>
      <tr>
        <td>mips64el</td>
        <td>1 968 095 838</td>
        <td>90 070 316</td>
        <td>784 830 991</td>
        <td>1 093 194 531</td>
      </tr>
      <tr>
        <td>ppc</td>
        <td>2 735 890 533</td>
        <td>115 892 326</td>
        <td>1 087 846 009</td>
        <td>1 532 152 198</td>
      </tr>
      <tr>
        <td>ppc64</td>
        <td>2 684 919 199</td>
        <td>122 371 298</td>
        <td>1 028 369 561</td>
        <td>1 534 178 340</td>
      </tr>
      <tr>
        <td>ppc64le</td>
        <td>2 641 863 052</td>
        <td>122 545 885</td>
        <td>985 137 467</td>
        <td>1 534 179 700</td>
      </tr>
      <tr>
        <td>riscv64</td>
        <td>1 589 964 563</td>
        <td>62 644 918</td>
        <td>643 559 932</td>
        <td>883 759 713</td>
      </tr>
      <tr>
        <td>s390x</td>
        <td>2 474 989 116</td>
        <td>58 933 857</td>
        <td>660 401 279</td>
        <td>1 755 653 980</td>
      </tr>
      <tr>
        <td>sh4</td>
        <td>2 562 375 399</td>
        <td>68 574 230</td>
        <td>1 138 226 176</td>
        <td>1 355 574 993</td>
      </tr>
      <tr>
        <td>sparc64</td>
        <td>3 917 963 038</td>
        <td>2 055 155 359</td>
        <td>1 298 625 002</td>
        <td>564 182 677</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td>1 986 765 860</td>
        <td>68 884 527</td>
        <td>804 594 605</td>
        <td>1 113 286 728</td>
      </tr>
    </tbody>
  </table>
</div>

### qsort_double (Clang)

<div style="overflow-x: auto;">
  <table class="results" style="width: 100%;">
    <thead>
      <tr>
        <th title="Field #1">Target</th>
        <th title="Field #2">Total Instructions</th>
        <th title="Field #3">Code Generation</th>
        <th title="Field #4">JIT Execution</th>
        <th title="Field #5">Helpers</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td style="color: #d7191c;">2 744 907 796</td>
        <td style="color: #4dac26;">74 594 511</td>
        <td>1 265 579 424</td>
        <td style="color: #d7191c;">1 404 733 861</td>
      </tr>
      <tr>
        <td>alpha</td>
        <td style="color: #d7191c;">2 070 204 037</td>
        <td style="color: #4dac26;">56 286 633</td>
        <td>869 643 481</td>
        <td style="color: #d7191c;">1 144 273 923</td>
      </tr>
      <tr>
        <td>arm</td>
        <td style="color: #d7191c;">9 163 293 622</td>
        <td style="color: #4dac26;">822 921 830</td>
        <td>4 265 464 995</td>
        <td style="color: #d7191c;">4 074 906 797</td>
      </tr>
      <tr>
        <td>hppa</td>
        <td style="color: #4dac26;">3 094 186 179</td>
        <td style="color: #4dac26;">484 467 463</td>
        <td>1 023 062 954</td>
        <td style="color: #4dac26;">1 586 655 762</td>
      </tr>
      <tr>
        <td>m68k</td>
        <td style="color: #4dac26;">4 114 339 886</td>
        <td style="color: #4dac26;">63 107 725</td>
        <td>1 224 197 193</td>
        <td style="color: #4dac26;">2 827 034 968</td>
      </tr>
      <tr>
        <td>mips</td>
        <td style="color: #d7191c;">2 240 757 989</td>
        <td style="color: #4dac26;">74 728 757</td>
        <td>942 372 869</td>
        <td style="color: #d7191c;">1 223 656 363</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td style="color: #d7191c;">2 241 264 168</td>
        <td style="color: #4dac26;">74 875 161</td>
        <td>942 732 894</td>
        <td style="color: #d7191c;">1 223 656 113</td>
      </tr>
      <tr>
        <td>mips64</td>
        <td style="color: #d7191c;">2 125 667 762</td>
        <td style="color: #4dac26;">80 578 997</td>
        <td>787 224 479</td>
        <td style="color: #d7191c;">1 257 864 286</td>
      </tr>
      <tr>
        <td>mips64el</td>
        <td style="color: #d7191c;">2 123 478 152</td>
        <td style="color: #4dac26;">80 782 855</td>
        <td>784 830 991</td>
        <td style="color: #d7191c;">1 257 864 306</td>
      </tr>
      <tr>
        <td>ppc</td>
        <td style="color: #d7191c;">3 265 774 088</td>
        <td style="color: #d7191c;">120 968 560</td>
        <td>1 087 846 009</td>
        <td style="color: #d7191c;">2 056 959 519</td>
      </tr>
      <tr>
        <td>ppc64</td>
        <td style="color: #d7191c;">3 198 309 630</td>
        <td style="color: #d7191c;">127 098 740</td>
        <td>1 028 369 561</td>
        <td style="color: #d7191c;">2 042 841 329</td>
      </tr>
      <tr>
        <td>ppc64le</td>
        <td style="color: #d7191c;">3 155 027 150</td>
        <td style="color: #d7191c;">127 046 877</td>
        <td>985 137 467</td>
        <td style="color: #d7191c;">2 042 842 806</td>
      </tr>
      <tr>
        <td>riscv64</td>
        <td style="color: #d7191c;">1 667 477 270</td>
        <td style="color: #4dac26;">60 857 834</td>
        <td>643 559 932</td>
        <td style="color: #d7191c;">963 059 504</td>
      </tr>
      <tr>
        <td>s390x</td>
        <td style="color: #d7191c;">2 404 573 110</td>
        <td style="color: #d7191c;">59 331 077</td>
        <td>660 401 279</td>
        <td style="color: #4dac26;">1 684 840 754</td>
      </tr>
      <tr>
        <td>sh4</td>
        <td style="color: #d7191c;">2 609 811 199</td>
        <td style="color: #4dac26;">66 186 107</td>
        <td>1 138 226 176</td>
        <td style="color: #d7191c;">1 405 398 916</td>
      </tr>
      <tr>
        <td>sparc64</td>
        <td style="color: #d7191c;">4 189 185 205</td>
        <td style="color: #d7191c;">2 351 778 131</td>
        <td>1 298 625 002</td>
        <td style="color: #4dac26;">538 782 072</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td style="color: #d7191c;">1 988 312 647</td>
        <td style="color: #4dac26;">67 801 893</td>
        <td>804 594 605</td>
        <td style="color: #d7191c;">1 115 916 149</td>
      </tr>
    </tbody>
  </table>
</div>

<br />
<hr />

### qsort_int32 (GCC)

<div style="overflow-x: auto;">
  <table class="results" style="width: 100%;">
    <thead>
      <tr>
        <th title="Field #1">Target</th>
        <th title="Field #2">Total Instructions</th>
        <th title="Field #3">Code Generation</th>
        <th title="Field #4">JIT Execution</th>
        <th title="Field #5">Helpers</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td>2 131 720 142</td>
        <td>72 399 376</td>
        <td>1 055 781 197</td>
        <td>1 003 539 569</td>
      </tr>
      <tr>
        <td>alpha</td>
        <td>1 460 366 380</td>
        <td>52 934 795</td>
        <td>796 943 218</td>
        <td>610 488 367</td>
      </tr>
      <tr>
        <td>arm</td>
        <td>3 372 434 621</td>
        <td>843 360 743</td>
        <td>1 078 158 662</td>
        <td>1 450 915 216</td>
      </tr>
      <tr>
        <td>hppa</td>
        <td>2 196 495 498</td>
        <td>491 615 054</td>
        <td>868 099 497</td>
        <td>836 780 947</td>
      </tr>
      <tr>
        <td>m68k</td>
        <td>1 779 675 824</td>
        <td>58 332 907</td>
        <td>966 478 982</td>
        <td>754 863 935</td>
      </tr>
      <tr>
        <td>mips</td>
        <td>1 499 858 843</td>
        <td>74 197 186</td>
        <td>842 808 363</td>
        <td>582 853 294</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td>1 502 516 358</td>
        <td>74 066 097</td>
        <td>845 597 211</td>
        <td>582 853 050</td>
      </tr>
      <tr>
        <td>mips64</td>
        <td>1 498 104 595</td>
        <td>84 972 133</td>
        <td>802 161 902</td>
        <td>610 970 560</td>
      </tr>
      <tr>
        <td>mips64el</td>
        <td>1 477 116 988</td>
        <td>84 679 402</td>
        <td>781 628 546</td>
        <td>610 809 040</td>
      </tr>
      <tr>
        <td>ppc</td>
        <td>1 668 038 700</td>
        <td>109 657 874</td>
        <td>975 751 506</td>
        <td>582 629 320</td>
      </tr>
      <tr>
        <td>ppc64</td>
        <td>1 779 152 045</td>
        <td>115 936 847</td>
        <td>1 072 790 643</td>
        <td>590 424 555</td>
      </tr>
      <tr>
        <td>ppc64le</td>
        <td>1 727 703 061</td>
        <td>115 826 109</td>
        <td>1 021 451 041</td>
        <td>590 425 911</td>
      </tr>
      <tr>
        <td>riscv64</td>
        <td>1 289 198 318</td>
        <td>57 502 772</td>
        <td>624 840 792</td>
        <td>606 854 754</td>
      </tr>
      <tr>
        <td>s390x</td>
        <td>2 114 306 901</td>
        <td>53 466 647</td>
        <td>692 707 638</td>
        <td>1 368 132 616</td>
      </tr>
      <tr>
        <td>sh4</td>
        <td>1 878 429 484</td>
        <td>61 060 371</td>
        <td>913 781 294</td>
        <td>903 587 819</td>
      </tr>
      <tr>
        <td>sparc64</td>
        <td>3 352 057 480</td>
        <td>2 022 774 129</td>
        <td>1 141 078 790</td>
        <td>188 204 561</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td>1 751 081 171</td>
        <td>63 007 727</td>
        <td>765 175 073</td>
        <td>922 898 371</td>
      </tr>
    </tbody>
  </table>
</div>

### qsort_int32 (Clang)

<div style="overflow-x: auto;">
  <table class="results" style="width: 100%;">
    <thead>
      <tr>
        <th title="Field #1">Target</th>
        <th title="Field #2">Total Instructions</th>
        <th title="Field #3">Code Generation</th>
        <th title="Field #4">JIT Execution</th>
        <th title="Field #5">Helpers</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td style="color: #4dac26;">2 223 343 530</td>
        <td style="color: #4dac26;">68 447 052</td>
        <td>1 055 781 197</td>
        <td style="color: #d7191c;">1 099 115 281</td>
      </tr>
      <tr>
        <td>alpha</td>
        <td style="color: #4dac26;">1 531 490 101</td>
        <td style="color: #4dac26;">52 235 802</td>
        <td>796 943 218</td>
        <td style="color: #d7191c;">682 311 081</td>
      </tr>
      <tr>
        <td>arm</td>
        <td style="color: #4dac26;">3 370 059 365</td>
        <td style="color: #4dac26;">815 660 694</td>
        <td>1 078 158 662</td>
        <td style="color: #d7191c;">1 476 240 009</td>
      </tr>
      <tr>
        <td>hppa</td>
        <td style="color: #d7191c;">2 212 542 138</td>
        <td style="color: #4dac26;">479 758 595</td>
        <td>868 099 497</td>
        <td style="color: #d7191c;">864 684 046</td>
      </tr>
      <tr>
        <td>m68k</td>
        <td style="color: #d7191c;">1 853 277 157</td>
        <td style="color: #4dac26;">57 802 833</td>
        <td>966 478 982</td>
        <td style="color: #d7191c;">828 995 342</td>
      </tr>
      <tr>
        <td>mips</td>
        <td style="color: #d7191c;">1 549 431 653</td>
        <td style="color: #4dac26;">67 726 753</td>
        <td>842 808 363</td>
        <td style="color: #d7191c;">638 896 537</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td style="color: #d7191c;">1 552 417 791</td>
        <td style="color: #4dac26;">67 924 305</td>
        <td>845 597 211</td>
        <td style="color: #d7191c;">638 896 275</td>
      </tr>
      <tr>
        <td>mips64</td>
        <td style="color: #d7191c;">1 559 572 362</td>
        <td style="color: #4dac26;">76 178 736</td>
        <td>802 161 902</td>
        <td style="color: #d7191c;">681 231 724</td>
      </tr>
      <tr>
        <td>mips64el</td>
        <td style="color: #d7191c;">1 539 491 325</td>
        <td style="color: #4dac26;">76 254 175</td>
        <td>781 628 546</td>
        <td style="color: #d7191c;">681 608 604</td>
      </tr>
      <tr>
        <td>ppc</td>
        <td style="color: #d7191c;">1 730 823 937</td>
        <td style="color: #d7191c;">115 323 201</td>
        <td>975 751 506</td>
        <td style="color: #d7191c;">639 749 230</td>
      </tr>
      <tr>
        <td>ppc64</td>
        <td style="color: #d7191c;">1 855 390 899</td>
        <td style="color: #d7191c;">121 324 924</td>
        <td>1 072 790 643</td>
        <td style="color: #d7191c;">661 275 332</td>
      </tr>
      <tr>
        <td>ppc64le</td>
        <td style="color: #d7191c;">1 803 732 493</td>
        <td style="color: #d7191c;">121 004 644</td>
        <td>1 021 451 041</td>
        <td style="color: #d7191c;">661 276 808</td>
      </tr>
      <tr>
        <td>riscv64</td>
        <td style="color: #d7191c;">1 369 588 518</td>
        <td style="color: #4dac26;">56 200 727</td>
        <td>624 840 792</td>
        <td style="color: #d7191c;">688 546 999</td>
      </tr>
      <tr>
        <td>s390x</td>
        <td style="color: #4dac26;">2 018 273 542</td>
        <td style="color: #d7191c;">54 160 605</td>
        <td>692 707 638</td>
        <td style="color: #4dac26;">1 271 405 299</td>
      </tr>
      <tr>
        <td>sh4</td>
        <td style="color: #d7191c;">1 943 201 025</td>
        <td style="color: #4dac26;">59 392 201</td>
        <td>913 781 294</td>
        <td style="color: #d7191c;">970 027 530</td>
      </tr>
      <tr>
        <td>sparc64</td>
        <td style="color: #d7191c;">3 626 612 511</td>
        <td style="color: #d7191c;">2 317 128 970</td>
        <td>1 141 078 790</td>
        <td style="color: #4dac26;">168 404 751</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td style="color: #d7191c;">1 759 411 677</td>
        <td style="color: #4dac26;">62 347 319</td>
        <td>765 175 073</td>
        <td style="color: #d7191c;">931 889 285</td>
      </tr>
    </tbody>
  </table>
</div>

<br />
<hr />

### qsort_string (GCC)

<div style="overflow-x: auto;">
  <table class="results" style="width: 100%;">
    <thead>
      <tr>
        <th title="Field #1">Target</th>
        <th title="Field #2">Total Instructions</th>
        <th title="Field #3">Code Generation</th>
        <th title="Field #4">JIT Execution</th>
        <th title="Field #5">Helpers</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td>2 530 266 115</td>
        <td>71 878 428</td>
        <td>1 449 639 434</td>
        <td>1 008 748 253</td>
      </tr>
      <tr>
        <td>alpha</td>
        <td>1 793 598 941</td>
        <td>52 114 495</td>
        <td>1 117 230 092</td>
        <td>624 254 354</td>
      </tr>
      <tr>
        <td>arm</td>
        <td>7 155 712 165</td>
        <td>3 673 959 444</td>
        <td>1 747 578 444</td>
        <td>1 734 174 277</td>
      </tr>
      <tr>
        <td>hppa</td>
        <td>4 596 662 435</td>
        <td>2 077 290 104</td>
        <td>1 188 975 849</td>
        <td>1 330 396 482</td>
      </tr>
      <tr>
        <td>m68k</td>
        <td>2 295 149 555</td>
        <td>58 023 587</td>
        <td>1 343 485 580</td>
        <td>893 640 388</td>
      </tr>
      <tr>
        <td>mips</td>
        <td>2 113 525 462</td>
        <td>74 968 779</td>
        <td>1 501 957 602</td>
        <td>536 599 081</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td>2 110 264 854</td>
        <td>74 820 076</td>
        <td>1 498 845 813</td>
        <td>536 598 965</td>
      </tr>
      <tr>
        <td>mips64</td>
        <td>1 968 388 319</td>
        <td>81 540 025</td>
        <td>1 278 967 522</td>
        <td>607 880 772</td>
      </tr>
      <tr>
        <td>mips64el</td>
        <td>1 950 703 742</td>
        <td>81 332 799</td>
        <td>1 261 490 179</td>
        <td>607 880 764</td>
      </tr>
      <tr>
        <td>ppc</td>
        <td>2 428 873 715</td>
        <td>269 273 748</td>
        <td>1 616 960 349</td>
        <td>542 639 618</td>
      </tr>
      <tr>
        <td>ppc64</td>
        <td>2 404 019 471</td>
        <td>273 403 312</td>
        <td>1 361 586 990</td>
        <td>769 029 169</td>
      </tr>
      <tr>
        <td>ppc64le</td>
        <td>2 385 236 674</td>
        <td>273 482 576</td>
        <td>1 342 609 438</td>
        <td>769 144 660</td>
      </tr>
      <tr>
        <td>riscv64</td>
        <td>1 563 526 901</td>
        <td>56 535 790</td>
        <td>880 912 912</td>
        <td>626 078 199</td>
      </tr>
      <tr>
        <td>s390x</td>
        <td>3 934 202 536</td>
        <td>52 728 806</td>
        <td>868 450 778</td>
        <td>3 013 022 952</td>
      </tr>
      <tr>
        <td>sh4</td>
        <td>2 097 991 097</td>
        <td>61 043 883</td>
        <td>1 155 615 736</td>
        <td>881 331 478</td>
      </tr>
      <tr>
        <td>sparc64</td>
        <td>4 130 814 212</td>
        <td>2 078 206 607</td>
        <td>1 572 858 282</td>
        <td>479 749 323</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td>2 864 486 422</td>
        <td>63 565 441</td>
        <td>1 160 432 349</td>
        <td>1 640 488 632</td>
      </tr>
    </tbody>
  </table>
</div>

### qsort_string (Clang)

<div style="overflow-x: auto;">
  <table class="results" style="width: 100%;">
    <thead>
      <tr>
        <th title="Field #1">Target</th>
        <th title="Field #2">Total Instructions</th>
        <th title="Field #3">Code Generation</th>
        <th title="Field #4">JIT Execution</th>
        <th title="Field #5">Helpers</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td style="color: #d7191c;">2 622 482 230</td>
        <td style="color: #4dac26;">68 022 754</td>
        <td>1 449 639 434</td>
        <td style="color: #d7191c;">1 104 820 042</td>
      </tr>
      <tr>
        <td>alpha</td>
        <td style="color: #d7191c;">1 866 463 476</td>
        <td style="color: #4dac26;">51 536 810</td>
        <td>1 117 230 092</td>
        <td style="color: #d7191c;">697 696 574</td>
      </tr>
      <tr>
        <td>arm</td>
        <td style="color: #4dac26;">7 056 218 429</td>
        <td style="color: #4dac26;">3 549 558 538</td>
        <td>1 747 578 444</td>
        <td style="color: #d7191c;">1 759 081 447</td>
      </tr>
      <tr>
        <td>hppa</td>
        <td style="color: #4dac26;">4 584 536 797</td>
        <td style="color: #4dac26;">2 023 729 867</td>
        <td>1 188 975 849</td>
        <td style="color: #d7191c;">1 371 831 081</td>
      </tr>
      <tr>
        <td>m68k</td>
        <td style="color: #d7191c;">2 391 766 444</td>
        <td style="color: #4dac26;">57 534 808</td>
        <td>1 343 485 580</td>
        <td style="color: #d7191c;">990 746 056</td>
      </tr>
      <tr>
        <td>mips</td>
        <td style="color: #d7191c;">2 158 611 852</td>
        <td style="color: #4dac26;">68 459 450</td>
        <td>1 501 957 602</td>
        <td style="color: #d7191c;">588 194 800</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td style="color: #d7191c;">2 155 675 997</td>
        <td style="color: #4dac26;">68 635 509</td>
        <td>1 498 845 813</td>
        <td style="color: #d7191c;">588 194 675</td>
      </tr>
      <tr>
        <td>mips64</td>
        <td style="color: #d7191c;">2 031 507 933</td>
        <td style="color: #4dac26;">73 143 579</td>
        <td>1 278 967 522</td>
        <td style="color: #d7191c;">679 396 832</td>
      </tr>
      <tr>
        <td>mips64el</td>
        <td style="color: #d7191c;">2 014 166 270</td>
        <td style="color: #4dac26;">73 279 267</td>
        <td>1 261 490 179</td>
        <td style="color: #d7191c;">679 396 824</td>
      </tr>
      <tr>
        <td>ppc</td>
        <td style="color: #d7191c;">2 516 388 621</td>
        <td style="color: #d7191c;">303 589 290</td>
        <td>1 616 960 349</td>
        <td style="color: #d7191c;">595 838 982</td>
      </tr>
      <tr>
        <td>ppc64</td>
        <td style="color: #d7191c;">2 384 468 962</td>
        <td style="color: #4dac26;">308 642 876</td>
        <td>1 361 586 990</td>
        <td style="color: #4dac26;">714 239 096</td>
      </tr>
      <tr>
        <td>ppc64le</td>
        <td style="color: #d7191c;">2 365 387 612</td>
        <td style="color: #4dac26;">308 506 861</td>
        <td>1 342 609 438</td>
        <td style="color: #4dac26;">714 271 313</td>
      </tr>
      <tr>
        <td>riscv64</td>
        <td style="color: #d7191c;">1 646 638 770</td>
        <td style="color: #4dac26;">55 367 615</td>
        <td>880 912 912</td>
        <td style="color: #d7191c;">710 358 243</td>
      </tr>
      <tr>
        <td>s390x</td>
        <td style="color: #4dac26;">3 475 927 973</td>
        <td style="color: #d7191c;">53 526 410</td>
        <td>868 450 778</td>
        <td style="color: #4dac26;">2 553 950 785</td>
      </tr>
      <tr>
        <td>sh4</td>
        <td style="color: #d7191c;">2 161 178 295</td>
        <td style="color: #4dac26;">59 427 892</td>
        <td>1 155 615 736</td>
        <td style="color: #d7191c;">946 134 667</td>
      </tr>
      <tr>
        <td>sparc64</td>
        <td style="color: #d7191c;">4 410 070 085</td>
        <td style="color: #d7191c;">2 377 799 055</td>
        <td>1 572 858 282</td>
        <td style="color: #4dac26;">459 412 748</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td style="color: #d7191c;">2 874 783 326</td>
        <td style="color: #4dac26;">62 956 885</td>
        <td>1 160 432 349</td>
        <td style="color: #d7191c;">1 651 394 092</td>
      </tr>
    </tbody>
  </table>
</div>

<br />
<hr />

## Analysis of Results

### Comparison Script

To facilitate the analysis, another Python script can be used to compare Clang performance to that of GCC for each benchmark. The result would be five tables (one for each benchmark) with the percentage of increase/decrease in Clang instructions compared to GCC.

The script assumes that all the tables from the previous section are in a `tables` directory.

```python
import os
import csv


def calculate_change(gcc_instructions, clang_instructions):
    # Calculate the percentage of change in Clang instructions compared to GCC
    percentage = round(((clang_instructions - gcc_instructions) /
                        gcc_instructions) * 100, 3)
    return "+" + str(percentage) + "%" if percentage > 0 else str(percentage) + "%"


tables = sorted(os.listdir("tables"))

csv_headers = ["Target", "Total Instructions %",
               "Code Generation %", "JIT Execution %", "Helpers %"]

for i in range(0, len(tables), 2):
    benchmark_name = tables[i].split("-")[0]

    # Extract data from tables
    clang_data, gcc_data = [], []
    with open("tables/" + tables[i], "r") as file:
        clang_data = file.readlines()
    with open("tables/" + tables[i+1], "r") as file:
        gcc_data = file.readlines()

    with open(benchmark_name + "-compare.csv", "w") as file:
        writer = csv.writer(file)
        writer.writerow(csv_headers)

    for l in range(1, len(gcc_data)):
        gcc_split = gcc_data[l].split('"')
        clang_split = clang_data[l].split('"')

        target_name = gcc_split[0][:-1]

        gcc_instructions = int(gcc_split[1].replace(",", ""))
        clang_instructions = int(clang_split[1].replace(",", ""))
        instructions_change = calculate_change(
            gcc_instructions, clang_instructions)

        gcc_code_generation = int(gcc_split[3].replace(",", ""))
        clang_code_generation = int(clang_split[3].replace(",", ""))
        code_generation_change = calculate_change(
            gcc_code_generation, clang_code_generation)

        gcc_jit = int(gcc_split[5].replace(",", ""))
        clang_jit = int(clang_split[5].replace(",", ""))
        jit_change = calculate_change(gcc_jit, clang_jit)

        gcc_helpers = int(gcc_split[7].replace(",", ""))
        clang_helpers = int(clang_split[7].replace(",", ""))
        helpers_change = calculate_change(gcc_helpers, clang_helpers)

        with open(benchmark_name + "-compare.csv", "a") as file:
            writer = csv.writer(file)
            writer.writerow([
                target_name,
                instructions_change,
                code_generation_change,
                jit_change,
                helpers_change
            ])
    i += 2
```

### matmult_double

<div style="overflow-x: auto;">
  <table class="results" style="width: 100%;">
    <thead>
      <tr>
        <th title="Field #1">Target</th>
        <th title="Field #2">Total Instructions %</th>
        <th title="Field #3">Code Generation %</th>
        <th title="Field #4">JIT Execution %</th>
        <th title="Field #5">Helpers %</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td>+42.438%</td>
        <td>-5.433%</td>
        <td>0.0%</td>
        <td>+83.246%</td>
      </tr>
      <tr>
        <td>alpha</td>
        <td>+54.724%</td>
        <td>-1.667%</td>
        <td>0.0%</td>
        <td>+65.44%</td>
      </tr>
      <tr>
        <td>arm</td>
        <td>+0.281%</td>
        <td>-2.981%</td>
        <td>0.0%</td>
        <td>+1.829%</td>
      </tr>
      <tr>
        <td>hppa</td>
        <td>+42.618%</td>
        <td>-2.04%</td>
        <td>0.0%</td>
        <td>+50.738%</td>
      </tr>
      <tr>
        <td>m68k</td>
        <td>+6.483%</td>
        <td>-1.075%</td>
        <td>0.0%</td>
        <td>+7.974%</td>
      </tr>
      <tr>
        <td>mips</td>
        <td>+65.203%</td>
        <td>-8.857%</td>
        <td>0.0%</td>
        <td>+81.395%</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td>+64.95%</td>
        <td>-8.452%</td>
        <td>0.0%</td>
        <td>+76.804%</td>
      </tr>
      <tr>
        <td>mips64</td>
        <td>+64.735%</td>
        <td>-10.533%</td>
        <td>0.0%</td>
        <td>+81.255%</td>
      </tr>
      <tr>
        <td>mips64el</td>
        <td>+64.63%</td>
        <td>-10.158%</td>
        <td>0.0%</td>
        <td>+76.716%</td>
      </tr>
      <tr>
        <td>ppc</td>
        <td>+31.499%</td>
        <td>-0.455%</td>
        <td>0.0%</td>
        <td>+36.406%</td>
      </tr>
      <tr>
        <td>ppc64</td>
        <td>+30.209%</td>
        <td>-0.987%</td>
        <td>0.0%</td>
        <td>+35.41%</td>
      </tr>
      <tr>
        <td>ppc64le</td>
        <td>+30.207%</td>
        <td>-1.25%</td>
        <td>0.0%</td>
        <td>+35.41%</td>
      </tr>
      <tr>
        <td>riscv64</td>
        <td>+47.186%</td>
        <td>-2.543%</td>
        <td>0.0%</td>
        <td>+67.549%</td>
      </tr>
      <tr>
        <td>s390x</td>
        <td>+43.965%</td>
        <td>+1.041%</td>
        <td>0.0%</td>
        <td>+50.978%</td>
      </tr>
      <tr>
        <td>sh4</td>
        <td>+51.701%</td>
        <td>-3.185%</td>
        <td>0.0%</td>
        <td>+64.746%</td>
      </tr>
      <tr>
        <td>sparc64</td>
        <td>+53.417%</td>
        <td>-2.615%</td>
        <td>0.0%</td>
        <td>+66.767%</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td>+82.607%</td>
        <td>-1.278%</td>
        <td>0.0%</td>
        <td>+126.284%</td>
      </tr>
    </tbody>
  </table>
</div>

### matmult_int32

<div style="overflow-x: auto;">
  <table class="results" style="width: 100%;">
    <thead>
      <tr>
        <th title="Field #1">Target</th>
        <th title="Field #2">Total Instructions %</th>
        <th title="Field #3">Code Generation %</th>
        <th title="Field #4">JIT Execution %</th>
        <th title="Field #5">Helpers %</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td>-0.293%</td>
        <td>-5.312%</td>
        <td>0.0%</td>
        <td>+9.526%</td>
      </tr>
      <tr>
        <td>alpha</td>
        <td>+0.276%</td>
        <td>-0.868%</td>
        <td>0.0%</td>
        <td>+11.77%</td>
      </tr>
      <tr>
        <td>arm</td>
        <td>-1.057%</td>
        <td>-2.991%</td>
        <td>0.0%</td>
        <td>+0.999%</td>
      </tr>
      <tr>
        <td>hppa</td>
        <td>-0.325%</td>
        <td>-1.919%</td>
        <td>0.0%</td>
        <td>+2.819%</td>
      </tr>
      <tr>
        <td>m68k</td>
        <td>+0.39%</td>
        <td>-0.745%</td>
        <td>0.0%</td>
        <td>+10.763%</td>
      </tr>
      <tr>
        <td>mips</td>
        <td>-1.073%</td>
        <td>-8.577%</td>
        <td>0.0%</td>
        <td>+9.613%</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td>-1.009%</td>
        <td>-8.15%</td>
        <td>0.0%</td>
        <td>+9.613%</td>
      </tr>
      <tr>
        <td>mips64</td>
        <td>-1.364%</td>
        <td>-10.208%</td>
        <td>0.0%</td>
        <td>+11.77%</td>
      </tr>
      <tr>
        <td>mips64el</td>
        <td>-1.337%</td>
        <td>-9.785%</td>
        <td>0.0%</td>
        <td>+11.77%</td>
      </tr>
      <tr>
        <td>ppc</td>
        <td>+0.315%</td>
        <td>+0.336%</td>
        <td>0.0%</td>
        <td>+9.799%</td>
      </tr>
      <tr>
        <td>ppc64</td>
        <td>+0.329%</td>
        <td>-0.213%</td>
        <td>0.0%</td>
        <td>+11.999%</td>
      </tr>
      <tr>
        <td>ppc64le</td>
        <td>+0.272%</td>
        <td>-0.487%</td>
        <td>0.0%</td>
        <td>+11.998%</td>
      </tr>
      <tr>
        <td>riscv64</td>
        <td>+0.198%</td>
        <td>-1.848%</td>
        <td>0.0%</td>
        <td>+13.465%</td>
      </tr>
      <tr>
        <td>s390x</td>
        <td>-2.128%</td>
        <td>+1.605%</td>
        <td>0.0%</td>
        <td>-17.508%</td>
      </tr>
      <tr>
        <td>sh4</td>
        <td>+0.151%</td>
        <td>-2.41%</td>
        <td>0.0%</td>
        <td>+7.352%</td>
      </tr>
      <tr>
        <td>sparc64</td>
        <td>-0.458%</td>
        <td>-1.431%</td>
        <td>0.0%</td>
        <td>-4.12%</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td>+0.007%</td>
        <td>-0.754%</td>
        <td>0.0%</td>
        <td>+3.241%</td>
      </tr>
    </tbody>
  </table>
</div>

### qsort_double

<div style="overflow-x: auto;">
  <table class="results" style="width: 100%;">
    <thead>
      <tr>
        <th title="Field #1">Target</th>
        <th title="Field #2">Total Instructions %</th>
        <th title="Field #3">Code Generation %</th>
        <th title="Field #4">JIT Execution %</th>
        <th title="Field #5">Helpers %</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td>+3.264%</td>
        <td>-5.625%</td>
        <td>0.0%</td>
        <td>+6.943%</td>
      </tr>
      <tr>
        <td>alpha</td>
        <td>+6.213%</td>
        <td>-1.94%</td>
        <td>0.0%</td>
        <td>+11.956%</td>
      </tr>
      <tr>
        <td>arm</td>
        <td>+0.489%</td>
        <td>-3.288%</td>
        <td>0.0%</td>
        <td>+1.813%</td>
      </tr>
      <tr>
        <td>hppa</td>
        <td>-1.408%</td>
        <td>-2.463%</td>
        <td>0.0%</td>
        <td>-1.974%</td>
      </tr>
      <tr>
        <td>m68k</td>
        <td>-6.177%</td>
        <td>-1.28%</td>
        <td>0.0%</td>
        <td>-8.72%</td>
      </tr>
      <tr>
        <td>mips</td>
        <td>+6.788%</td>
        <td>-9.03%</td>
        <td>0.0%</td>
        <td>+13.956%</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td>+6.803%</td>
        <td>-8.667%</td>
        <td>0.0%</td>
        <td>+13.956%</td>
      </tr>
      <tr>
        <td>mips64</td>
        <td>+7.867%</td>
        <td>-10.688%</td>
        <td>0.0%</td>
        <td>+15.063%</td>
      </tr>
      <tr>
        <td>mips64el</td>
        <td>+7.895%</td>
        <td>-10.311%</td>
        <td>0.0%</td>
        <td>+15.063%</td>
      </tr>
      <tr>
        <td>ppc</td>
        <td>+19.368%</td>
        <td>+4.38%</td>
        <td>0.0%</td>
        <td>+34.253%</td>
      </tr>
      <tr>
        <td>ppc64</td>
        <td>+19.121%</td>
        <td>+3.863%</td>
        <td>0.0%</td>
        <td>+33.155%</td>
      </tr>
      <tr>
        <td>ppc64le</td>
        <td>+19.424%</td>
        <td>+3.673%</td>
        <td>0.0%</td>
        <td>+33.155%</td>
      </tr>
      <tr>
        <td>riscv64</td>
        <td>+4.875%</td>
        <td>-2.853%</td>
        <td>0.0%</td>
        <td>+8.973%</td>
      </tr>
      <tr>
        <td>s390x</td>
        <td>-2.845%</td>
        <td>+0.674%</td>
        <td>0.0%</td>
        <td>-4.033%</td>
      </tr>
      <tr>
        <td>sh4</td>
        <td>+1.851%</td>
        <td>-3.483%</td>
        <td>0.0%</td>
        <td>+3.675%</td>
      </tr>
      <tr>
        <td>sparc64</td>
        <td>+6.923%</td>
        <td>+14.433%</td>
        <td>0.0%</td>
        <td>-4.502%</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td>+0.078%</td>
        <td>-1.572%</td>
        <td>0.0%</td>
        <td>+0.236%</td>
      </tr>
    </tbody>
  </table>
</div>

### qsort_int32

<div style="overflow-x: auto;">
  <table class="results" style="width: 100%;">
    <thead>
      <tr>
        <th title="Field #1">Target</th>
        <th title="Field #2">Total Instructions %</th>
        <th title="Field #3">Code Generation %</th>
        <th title="Field #4">JIT Execution %</th>
        <th title="Field #5">Helpers %</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td>+4.298%</td>
        <td>-5.459%</td>
        <td>0.0%</td>
        <td>+9.524%</td>
      </tr>
      <tr>
        <td>alpha</td>
        <td>+4.87%</td>
        <td>-1.32%</td>
        <td>0.0%</td>
        <td>+11.765%</td>
      </tr>
      <tr>
        <td>arm</td>
        <td>-0.07%</td>
        <td>-3.284%</td>
        <td>0.0%</td>
        <td>+1.745%</td>
      </tr>
      <tr>
        <td>hppa</td>
        <td>+0.731%</td>
        <td>-2.412%</td>
        <td>0.0%</td>
        <td>+3.335%</td>
      </tr>
      <tr>
        <td>m68k</td>
        <td>+4.136%</td>
        <td>-0.909%</td>
        <td>0.0%</td>
        <td>+9.82%</td>
      </tr>
      <tr>
        <td>mips</td>
        <td>+3.305%</td>
        <td>-8.721%</td>
        <td>0.0%</td>
        <td>+9.615%</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td>+3.321%</td>
        <td>-8.292%</td>
        <td>0.0%</td>
        <td>+9.615%</td>
      </tr>
      <tr>
        <td>mips64</td>
        <td>+4.103%</td>
        <td>-10.349%</td>
        <td>0.0%</td>
        <td>+11.5%</td>
      </tr>
      <tr>
        <td>mips64el</td>
        <td>+4.223%</td>
        <td>-9.95%</td>
        <td>0.0%</td>
        <td>+11.591%</td>
      </tr>
      <tr>
        <td>ppc</td>
        <td>+3.764%</td>
        <td>+5.166%</td>
        <td>0.0%</td>
        <td>+9.804%</td>
      </tr>
      <tr>
        <td>ppc64</td>
        <td>+4.285%</td>
        <td>+4.647%</td>
        <td>0.0%</td>
        <td>+12.0%</td>
      </tr>
      <tr>
        <td>ppc64le</td>
        <td>+4.401%</td>
        <td>+4.471%</td>
        <td>0.0%</td>
        <td>+12.0%</td>
      </tr>
      <tr>
        <td>riscv64</td>
        <td>+6.236%</td>
        <td>-2.264%</td>
        <td>0.0%</td>
        <td>+13.462%</td>
      </tr>
      <tr>
        <td>s390x</td>
        <td>-4.542%</td>
        <td>+1.298%</td>
        <td>0.0%</td>
        <td>-7.07%</td>
      </tr>
      <tr>
        <td>sh4</td>
        <td>+3.448%</td>
        <td>-2.732%</td>
        <td>0.0%</td>
        <td>+7.353%</td>
      </tr>
      <tr>
        <td>sparc64</td>
        <td>+8.191%</td>
        <td>+14.552%</td>
        <td>0.0%</td>
        <td>-10.52%</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td>+0.476%</td>
        <td>-1.048%</td>
        <td>0.0%</td>
        <td>+0.974%</td>
      </tr>
    </tbody>
  </table>
</div>

### qsort_string

<div style="overflow-x: auto;">
  <table class="results" style="width: 100%;">
    <thead>
      <tr>
        <th title="Field #1">Target</th>
        <th title="Field #2">Total Instructions %</th>
        <th title="Field #3">Code Generation %</th>
        <th title="Field #4">JIT Execution %</th>
        <th title="Field #5">Helpers %</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td>+3.645%</td>
        <td>-5.364%</td>
        <td>0.0%</td>
        <td>+9.524%</td>
      </tr>
      <tr>
        <td>alpha</td>
        <td>+4.062%</td>
        <td>-1.108%</td>
        <td>0.0%</td>
        <td>+11.765%</td>
      </tr>
      <tr>
        <td>arm</td>
        <td>-1.39%</td>
        <td>-3.386%</td>
        <td>0.0%</td>
        <td>+1.436%</td>
      </tr>
      <tr>
        <td>hppa</td>
        <td>-0.264%</td>
        <td>-2.578%</td>
        <td>0.0%</td>
        <td>+3.114%</td>
      </tr>
      <tr>
        <td>m68k</td>
        <td>+4.21%</td>
        <td>-0.842%</td>
        <td>0.0%</td>
        <td>+10.866%</td>
      </tr>
      <tr>
        <td>mips</td>
        <td>+2.133%</td>
        <td>-8.683%</td>
        <td>0.0%</td>
        <td>+9.615%</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td>+2.152%</td>
        <td>-8.266%</td>
        <td>0.0%</td>
        <td>+9.615%</td>
      </tr>
      <tr>
        <td>mips64</td>
        <td>+3.207%</td>
        <td>-10.297%</td>
        <td>0.0%</td>
        <td>+11.765%</td>
      </tr>
      <tr>
        <td>mips64el</td>
        <td>+3.253%</td>
        <td>-9.902%</td>
        <td>0.0%</td>
        <td>+11.765%</td>
      </tr>
      <tr>
        <td>ppc</td>
        <td>+3.603%</td>
        <td>+12.744%</td>
        <td>0.0%</td>
        <td>+9.804%</td>
      </tr>
      <tr>
        <td>ppc64</td>
        <td>-0.813%</td>
        <td>+12.889%</td>
        <td>0.0%</td>
        <td>-7.125%</td>
      </tr>
      <tr>
        <td>ppc64le</td>
        <td>-0.832%</td>
        <td>+12.807%</td>
        <td>0.0%</td>
        <td>-7.134%</td>
      </tr>
      <tr>
        <td>riscv64</td>
        <td>+5.316%</td>
        <td>-2.066%</td>
        <td>0.0%</td>
        <td>+13.462%</td>
      </tr>
      <tr>
        <td>s390x</td>
        <td>-11.648%</td>
        <td>+1.513%</td>
        <td>0.0%</td>
        <td>-15.236%</td>
      </tr>
      <tr>
        <td>sh4</td>
        <td>+3.012%</td>
        <td>-2.647%</td>
        <td>0.0%</td>
        <td>+7.353%</td>
      </tr>
      <tr>
        <td>sparc64</td>
        <td>+6.76%</td>
        <td>+14.416%</td>
        <td>0.0%</td>
        <td>-4.239%</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td>+0.359%</td>
        <td>-0.957%</td>
        <td>0.0%</td>
        <td>+0.665%</td>
      </tr>
    </tbody>
  </table>
</div>

### Floating Point Benchmarks

For all five benchmarks, most targets had a decrease in the number of instructions spent in code generation, however, there was a major increase in the number of instructions spent in the execution of helpers.

To find out the reason behind this increase, the `list_helpers.py` script can be used with the `matmul_double` benchmark (which had the biggest decrease in Clang performance) and any of the seventeen targets to list the executed helpers.

List helpers of ppc for matmult_double on GCC:

```bash
./list_helpers.py -- <qemu-gcc-build>/ppc-linux-user/qemu-ppc matmult_double-ppc
```

Results:

```
Executed QEMU Helpers:

 No.     Instructions  Percentage            Calls    Ins/Call  Helper Name                   Source File
----  ---------------  ----------  ---------------  ----------  -------------------------     ------------------------------
   1    2,088,642,242     66.832%        8,000,000         261  helper_fmadd                  <qemu>/target/ppc/fpu_helper.c
   2      420,240,000     13.447%        8,240,000          51  helper_compute_fprf_float64   <qemu>/target/ppc/fpu_helper.c
   3      139,760,120      4.472%        8,240,008          16  helper_float_check_status     <qemu>/target/ppc/fpu_helper.c
   4       16,480,024      0.527%        8,240,012           2  helper_reset_fpstatus         <qemu>/include/fpu/softfloat-helpers.h
   5       11,167,515      0.357%           80,000         139  helper_fmul                   <qemu>/target/ppc/fpu_helper.c
   6       10,320,000      0.330%           80,000         129  helper_fsub                   <qemu>/target/ppc/fpu_helper.c
   7       10,000,000      0.320%           80,000         125  helper_fdiv                   <qemu>/target/ppc/fpu_helper.c
   8        8,314,772      0.266%          162,603          51  helper_lookup_tb_ptr          <qemu>/accel/tcg/tcg-runtime.c
   9            2,618      0.000%               14         187  helper_dcbz                   <qemu>/target/ppc/mem_helper.c
  10            1,494      0.000%               18          83  helper_raise_exception_err    <qemu>/target/ppc/excp_helper.c
  11            1,012      0.000%                8         126  helper_fcmpu                  <qemu>/target/ppc/fpu_helper.c
```

List helpers of ppc for matmult_double on Clang:

```bash
./list_helpers.py -- <qemu-clang-build>/ppc-linux-user/qemu-ppc matmult_double-ppc
```

Results:

```
Executed QEMU Helpers:

 No.     Instructions  Percentage            Calls    Ins/Call  Helper Name                   Source File
----  ---------------  ----------  ---------------  ----------  -------------------------     ------------------------------
   1    3,040,716,864     73.990%        8,000,000         380  helper_fmadd                  <qemu>/target/ppc/fpu_helper.c
   2      403,760,000      9.825%        8,240,000          49  helper_compute_fprf_float64   <qemu>/target/ppc/fpu_helper.c
   3      164,480,144      4.002%        8,240,008          19  helper_float_check_status     <qemu>/target/ppc/fpu_helper.c
   4       18,800,000      0.457%           80,000         235  helper_fsub                   <qemu>/target/ppc/fpu_helper.c
   5       18,230,012      0.444%           80,000         227  helper_fmul                   <qemu>/target/ppc/fpu_helper.c
   6       18,080,000      0.440%           80,000         226  helper_fdiv                   <qemu>/target/ppc/fpu_helper.c
   7       16,480,024      0.401%        8,240,012           2  helper_reset_fpstatus         <qemu>/include/fpu/softfloat-helpers.h
   8        9,127,473      0.222%          162,603          56  helper_lookup_tb_ptr          <qemu>/include/exec/exec-all.h
   9            3,774      0.000%               18         209  helper_raise_exception_err    <qemu>/target/ppc/excp_helper.c
  10            2,492      0.000%               14         178  helper_dcbz                   <qemu>/target/ppc/mem_helper.c
  11            1,544      0.000%                8         193  helper_fcmpu                  <qemu>/target/ppc/fpu_helper.c
```

All floating point helpers had an increase in their number of instructions per call. The `list_fn_callees.py` script can be utilized to analyse one of these helpers.

List callees of helper_fdiv on GCC:

```bash
./list_fn_callees.py -f helper_fdiv-- <qemu-gcc-build>/ppc-linux-user/qemu-ppc matmult_double-ppc
```

Results:

```
Callees of helper_fdiv:

 No.     Instructions  Percentage            Calls    Ins/Call  Function Name              Source File
----  ---------------  ----------  ---------------  ----------  -------------------------  ------------------------------
   1        7,840,000      0.251%           80,000          98  float64_div                 <qemu>/include/qemu/bitops.h
```

List callees of helper_fdiv on Clang:

```bash
./list_fn_callees.py -f helper_fdiv-- <qemu-clang-build>/ppc-linux-user/qemu-ppc matmult_double-ppc
```

Results:

```
Callees of helper_fdiv:

 No.     Instructions  Percentage            Calls    Ins/Call  Function Name              Source File
----  ---------------  ----------  ---------------  ----------  -------------------------  ------------------------------
   1       15,200,000      0.370%           80,000         190  float64_div                 <qemu>/fpu/softfloat.c
```

List callees of float64_div on GCC:

```bash
./list_fn_callees.py -f float64_div-- <qemu-gcc-build>/ppc-linux-user/qemu-ppc matmult_double-ppc
```

Results:

```
Couldn't locate function: float64_div.
```

List callees of float64_div on Clang:

```bash
./list_fn_callees.py -f float64_div-- <qemu-clang-build>/ppc-linux-user/qemu-ppc matmult_double-ppc
```

```
Callees of float64_div:

 No.     Instructions  Percentage            Calls    Ins/Call  Function Name              Source File
----  ---------------  ----------  ---------------  ----------  -------------------------  ------------------------------
   1        5,760,000      0.140%           80,000          72  round_canonical             <qemu>/fpu/softfloat.c
```

The source code of the `float64_div` function which is the callee of `helper_fdiv` is attached in the [appendix](#appendix) section of the report. Notice how the `QEMU_FLATTEN` attribute is used in the function, but it seems that GCC was able to inline all functions within to the maximum depth. Unlike Clang, which failed to do so with `float64_round_pack_canonical` causing the `round_canonical` function to be explicitly called thus costing extra instructions.

This is the reason behind the increase in the number of instructions per call for all targets and for most of the helpers, which proves that GCC does a better job in optimizing functions with `__attribute__((flatten))`. Since this is a compiler optimization problem, it might be fixed in future versions of Clang.

**cpu_loop_exit:**

By following the same procedure to analyze the `helper_raise_exception_err`, it can be seen that `cpu_loop_exit` uses a different function for performing long jump in Clang. This function (`longjmp@GLIBC_2.2.5`) executes nearly 3 times the number of instruction per call of the function which is used in the GCC build (`__longjmp_chk`).

This `cpu_loop_exit` behavior also appears in most of the other targets.

### Integer and String benchmarks

These types of benchmarks (matmul_int32, qsort_int32 and qsort_string) use very few helpers compared to benchmarks involving floating point numbers. The change in performance is mainly attributed to code generation. Degradation in helpers still occurs and one of its reasons is the different long jump function used in the two builds which was discussed above.

## QEMU Binary Size

To conclude the report, a small Python script is used to measure the size of the QEMU target binary for each of GCC and Clang. The last column in the output table is the percentage of change in the Clang binary size compared to GCC.

```python
import os
import csv

def convert_bytes(n):
    for x in ['bytes', 'KB', 'MB', 'GB', 'TB']:
        if n < 1024.0:
            return "%3.1f %s" % (n, x)
        n /= 1024.0


builds = {
    "gcc": "<qemu-gcc-build>",
    "clang": "<qemu-clang-build>"
}

targets = ["aarch64", "alpha", "arm", "hppa", "m68k", "mips", "mipsel",
           "mips64", "mips64el", "ppc", "ppc64", "ppc64le", "riscv64",
           "s390x", "sh4", "sparc64", "x86_64"]

csv_headers = ["Target", "GCC", "Clang", "Difference %"]

with open("compare_exe.csv", "w") as file:
    writer = csv.writer(file)
    writer.writerow(csv_headers)

for target in targets:
    size = []

    for build_name, build_path in builds.items():
        size.append(os.path.getsize("{}/{}-linux-user/qemu-{}".format(build_path,
                                                                      target,
                                                                      target)))
    with open("compare_exe.csv", "a") as file:
        writer = csv.writer(file)
        writer.writerow([target, convert_bytes(size[0]), convert_bytes(
            size[1]), str(round(((size[1] - size[0]) / size[1]) * 100, 3))+"%"])
```

### Results

<style>
  table.results {
    table-layout:fixed;
  }
  table.results td:nth-child(1) {
    text-align: left;
  }
  table.results th:nth-child(1) {
    text-align: left;
  }
</style>

<div style="overflow-x: auto;">
  <table class="results" style="width: 100%;">
    <thead>
      <tr>
        <th title="Field #1">Target</th>
        <th title="Field #2">GCC</th>
        <th title="Field #3">Clang</th>
        <th title="Field #4">Difference %</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td>27.6 MB</td>
        <td>26.4 MB</td>
        <td>-4.722%</td>
      </tr>
      <tr>
        <td>alpha</td>
        <td>15.1 MB</td>
        <td>11.9 MB</td>
        <td>-26.789%</td>
      </tr>
      <tr>
        <td>arm</td>
        <td>20.1 MB</td>
        <td>17.7 MB</td>
        <td>-13.108%</td>
      </tr>
      <tr>
        <td>hppa</td>
        <td>15.4 MB</td>
        <td>12.4 MB</td>
        <td>-24.662%</td>
      </tr>
      <tr>
        <td>m68k</td>
        <td>16.1 MB</td>
        <td>12.9 MB</td>
        <td>-24.916%</td>
      </tr>
      <tr>
        <td>mips</td>
        <td>25.5 MB</td>
        <td>23.7 MB</td>
        <td>-7.304%</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td>25.3 MB</td>
        <td>23.7 MB</td>
        <td>-6.577%</td>
      </tr>
      <tr>
        <td>mips64</td>
        <td>25.8 MB</td>
        <td>24.2 MB</td>
        <td>-6.743%</td>
      </tr>
      <tr>
        <td>mips64el</td>
        <td>25.7 MB</td>
        <td>24.2 MB</td>
        <td>-6.067%</td>
      </tr>
      <tr>
        <td>ppc</td>
        <td>20.1 MB</td>
        <td>19.2 MB</td>
        <td>-4.539%</td>
      </tr>
      <tr>
        <td>ppc64</td>
        <td>20.4 MB</td>
        <td>19.7 MB</td>
        <td>-3.337%</td>
      </tr>
      <tr>
        <td>ppc64le</td>
        <td>20.2 MB</td>
        <td>19.7 MB</td>
        <td>-2.52%</td>
      </tr>
      <tr>
        <td>riscv64</td>
        <td>20.1 MB</td>
        <td>18.1 MB</td>
        <td>-10.943%</td>
      </tr>
      <tr>
        <td>s390x</td>
        <td>17.2 MB</td>
        <td>13.7 MB</td>
        <td>-25.374%</td>
      </tr>
      <tr>
        <td>sh4</td>
        <td>15.0 MB</td>
        <td>11.8 MB</td>
        <td>-27.197%</td>
      </tr>
      <tr>
        <td>sparc64</td>
        <td>15.8 MB</td>
        <td>12.7 MB</td>
        <td>-24.964%</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td>17.2 MB</td>
        <td>13.9 MB</td>
        <td>-23.38%</td>
      </tr>
    </tbody>
  </table>
</div>

## Appendix

### float64_div Implementation

```c
float64 QEMU_FLATTEN
float64_div(float64 a, float64 b, float_status *s)
{
    return float64_gen2(a, b, s, hard_f64_div, soft_f64_div,
                        f64_div_pre, f64_div_post);
}

static inline float64
float64_gen2(float64 xa, float64 xb, float_status *s,
              hard_f64_op2_fn hard, soft_f64_op2_fn soft,
              f64_check_fn pre, f64_check_fn post)
{
    union_float64 ua, ub, ur;

    ua.s = xa;
    ub.s = xb;

    if (unlikely(!can_use_fpu(s))) {
        goto soft;
    }

    float64_input_flush2(&ua.s, &ub.s, s);
    if (unlikely(!pre(ua, ub))) {
        goto soft;
    }

    ur.h = hard(ua.h, ub.h);
    if (unlikely(f64_is_inf(ur))) {
        s->float_exception_flags |= float_flag_overflow;
    } else if (unlikely(fabs(ur.h) <= DBL_MIN) && post(ua, ub)) {
        goto soft;
    }
    return ur.s;

  soft:
    return soft(ua.s, ub.s, s);
}

static float64 QEMU_SOFTFLOAT_ATTR
soft_f64_div(float64 a, float64 b, float_status *status)
{
    FloatParts pa = float64_unpack_canonical(a, status);
    FloatParts pb = float64_unpack_canonical(b, status);
    FloatParts pr = div_floats(pa, pb, status);

    return float64_round_pack_canonical(pr, status);
}

static float64 float64_round_pack_canonical(FloatParts p, float_status *s)
{
    return float64_pack_raw(round_canonical(p, s, &float64_params));
}
```

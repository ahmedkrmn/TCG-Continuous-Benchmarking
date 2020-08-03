---
layout: post
title: Measuring QEMU Emulation Efficiency
subtitle: Comparing Guest Instructions and QEMU Instructions
tags: [Guest Instructions, libinsn, Callgrind]
excerpt: In this report, a method for measuring the TCG emulation efficiency of QEMU is presented. This is achieved for seventeen different targets by comparing the number of guest instructions (running the program natively on the target) and the number of QEMU instructions (running the program through QEMU). For each target, the ratio between these two numbers presents a rough estimation of the emulation efficiency for that target.
---

## Intro

This reports presents a method for measuring the TCG emulation efficiency in QEMU. This is achieved for seventeen different targets by comparing the number of guest instructions (running the program natively on the target) and the number of QEMU instructions (running the program through QEMU). For each target, the ratio between these two numbers presents a rough estimation of the emulation efficiency for that target.

Beside the five newly introduced benchmarks in the previous report, the Coulomb benchmark is also reused in this report to provide a variety of workloads. This gives a total of six benchmark programs that can be categorized into two groups:

- Floating point operations (group 1):
  - coulomb_double
  - matmult_double
  - qsort_double
- Basic int and char operations (group 2):
  - matmult_int32
  - qsort_int32
  - qsort_string

All benchmarks are available on the project [GitHub page](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/tree/master/benchmarks).

## Table of Contents

- [Setup](#setup)
- [Measurements](#measurements)
- [Results (Benchmark Group 1)](#results-benchmarks-group-1)
- [Results (Benchmark Group 2)](#results-benchmarks-group-2)
- [Analysis](#analysis)

## Setup

All the measurements in this report are based on the newly released QEMU version [5.1.0-rc2](https://git.qemu.org/?p=qemu.git;a=tag;h=d56d36b15b4751bbfb573accf6edaf1f4baa0198).
To measure the number of guest instructions, the `libinsn` plugin is utilized which is available when QEMU is built with the `--enable-plugins` option. The general syntax of using the plugin is:

```bash
<qemu-executable> -plugin <qemu-plugins-build>/tests/plugin/libinsn.so -d plugin <test-program>
```

To measure the number of QEMU instructions, Callgrind is used. Please refer to the "[Measuring Basic Performance Metrics of QEMU](https://ahmedkrmn.github.io/TCG-Continuous-Benchmarking/Measuring-Basic-Performance-Metrics-of-QEMU/)" report for more details on setting up and using Callgrind.

To create a plugins build based on the latest QEMU version, this bash snippet is used:

```bash
wget https://download.qemu.org/qemu-5.1.0-rc2.tar.xz
tar xfv qemu-5.1.0-rc2.tar.xz
cd qemu-5.1.0-rc2
mkdir build-gcc-plugins
cd build-gcc-plugins
../configure --disable-system --disable-tools --enable-plugins
make
```

## Measurements

The Python script below creates a CSV table for each of the six benchmarks. Each table contains seventeen rows, one for each target. A row contains the target name, number of guest instructions, number of QEMU instructions and the ratio between the two numbers.

```python
import csv
import os
import subprocess
import sys
import tempfile

############### Script Options ###############
qemu_build = "<qemu-plugins-build>"
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


def measure_qemu_instructions(qemu_exe_path, program_exe_path):
    # Measure the number of QEMU instructions using Callgrind
    with tempfile.NamedTemporaryFile() as tmp_out:
        run_callgrind = subprocess.run(["valgrind",
                                        "--tool=callgrind",
                                        "--callgrind-out-file=" + tmp_out.name,
                                        qemu_exe_path,
                                        program_exe_path],
                                       stdout=subprocess.DEVNULL,
                                       stderr=subprocess.PIPE)
    callgrind_output = run_callgrind.stderr.decode("utf-8").split("\n")
    return int(callgrind_output[8].split(" ")[-1])


csv_header = ["Target", "Guest Instructions", "QEMU Instructions", "Ratio"]
benchmarks = os.listdir('benchmarks')
libinsn_path = os.path.join(qemu_build, "tests", "plugin", "libinsn.so")
os.mkdir("tables")

for benchmark in benchmarks:
    data = []
    benchmark_name = os.path.splitext(benchmark)[0]
    benchmark_path = os.path.join("benchmarks", benchmark)
    for target_name, target_compiler in targets.items():
        with tempfile.NamedTemporaryFile() as tmp_exe:
            # Compile target
            subprocess.run([target_compiler, "-O2", "-static",
                            benchmark_path, "-o", tmp_exe.name, "-lm"])
            # Run the libinsn plugin
            run_qemu_plugin = subprocess.run(["{}/{}-linux-user/qemu-{}".
                                              format(qemu_build,
                                                     target_name,
                                                     target_name),
                                              "-plugin",
                                              libinsn_path,
                                              "-d",
                                              "plugin",
                                              tmp_exe.name],
                                             stdout=subprocess.DEVNULL,
                                             stderr=subprocess.PIPE)
            # Measure the instructions
            guest_instructions = int(run_qemu_plugin.stderr.decode("utf-8").
                                     split()[-1])
            qemu_instruction = measure_qemu_instructions("{}/{}-linux-user/qemu-{}".
                                                         format(qemu_build,
                                                                target_name,
                                                                target_name),
                                                         tmp_exe.name)
        data.append([target_name,
                     format(guest_instructions, ","),
                     format(qemu_instruction, ","),
                     "1:" + str(round((qemu_instruction / guest_instructions), 3))])

    with open(os.path.join("tables", benchmark_name) + ".csv", "w") as file:
        writer = csv.writer(file)
        writer.writerow(csv_header)
        writer.writerows(data)

```

<style>
  /* Right align all table fields */
  table.results td:nth-last-child(-n + 3) {
    text-align: right;
  }
  table.results th:nth-last-child(-n + 3) {
    text-align: right;
  }
  /* Borders for table header */
  table.results th{
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

## Results (Benchmarks Group 1)

### coulomb_double

<div style="overflow-x: auto;">
  <table
    class="results"
    style="width: 83%; margin-right: auto; margin-left: auto;"
  >
    <thead>
      <tr>
        <th title="Field #1">Target</th>
        <th title="Field #2">Guest Instructions</th>
        <th title="Field #3">QEMU Instructions</th>
        <th title="Field #4">Ratio</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td>182 965 444</td>
        <td>4 424 319 223</td>
        <td>1:24.181</td>
      </tr>
      <tr>
        <td>alpha</td>
        <td>287 894 875</td>
        <td>10 720 832 859</td>
        <td>1:37.239</td>
      </tr>
      <tr>
        <td>arm</td>
        <td>4 353 433 161</td>
        <td>39 328 640 162</td>
        <td style="color: #4dac26; font-weight: bold;">1:9.034</td>
      </tr>
      <tr>
        <td>hppa</td>
        <td>290 299 145</td>
        <td>12 007 537 148</td>
        <td>1:41.363</td>
      </tr>
      <tr>
        <td>m68k</td>
        <td>55 464 791</td>
        <td>7 107 559 194</td>
        <td style="color: #d7191c; font-weight: bold;">1:128.145</td>
      </tr>
      <tr>
        <td>mips</td>
        <td>286 969 260</td>
        <td>9 957 633 056</td>
        <td>1:34.699</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td>300 313 870</td>
        <td>11 123 315 018</td>
        <td>1:37.039</td>
      </tr>
      <tr>
        <td>mips64</td>
        <td>255 992 742</td>
        <td>9 855 532 178</td>
        <td>1:38.499</td>
      </tr>
      <tr>
        <td>mips64el</td>
        <td>266 739 104</td>
        <td>11 004 724 703</td>
        <td>1:41.257</td>
      </tr>
      <tr>
        <td>ppc</td>
        <td>239 658 319</td>
        <td>13 031 944 195</td>
        <td>1:54.377</td>
      </tr>
      <tr>
        <td>ppc64</td>
        <td>228 263 889</td>
        <td>13 034 833 440</td>
        <td style="color: #d7191c;">1:57.104</td>
      </tr>
      <tr>
        <td>ppc64le</td>
        <td>220 968 816</td>
        <td>13 012 936 191</td>
        <td style="color: #d7191c;">1:58.890</td>
      </tr>
      <tr>
        <td>riscv64</td>
        <td>209 944 207</td>
        <td>4 069 430 554</td>
        <td style="color: #4dac26;">1:19.383</td>
      </tr>
      <tr>
        <td>s390x</td>
        <td>215 191 419</td>
        <td>11 013 187 596</td>
        <td>1:51.179</td>
      </tr>
      <tr>
        <td>sh4</td>
        <td>473 219 807</td>
        <td>12 728 861 129</td>
        <td>1:26.898</td>
      </tr>
      <tr>
        <td>sparc64</td>
        <td>263 295 373</td>
        <td>11 969 980 973</td>
        <td>1:45.462</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td>225 499 576</td>
        <td>4 643 073 756</td>
        <td style="color: #4dac26;">1:20.590</td>
      </tr>
    </tbody>
  </table>
</div>

### matmult_double

<div style="overflow-x: auto;">
  <table
    class="results"
    style="width: 83%; margin-right: auto; margin-left: auto;"
  >
    <thead>
      <tr>
        <th title="Field #1">Target</th>
        <th title="Field #2">Guest Instructions</th>
        <th title="Field #3">QEMU Instructions</th>
        <th title="Field #4">Ratio</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td>62 565 037</td>
        <td>1 412 678 042</td>
        <td>1:22.579</td>
      </tr>
      <tr>
        <td>alpha</td>
        <td>120 146 835</td>
        <td>3 021 375 794</td>
        <td>1:25.147</td>
      </tr>
      <tr>
        <td>arm</td>
        <td>917 721 514</td>
        <td>8 723 369 272</td>
        <td style="color: #4dac26; font-weight: bold;">1:9.505</td>
      </tr>
      <tr>
        <td>hppa</td>
        <td>63 330 121</td>
        <td>3 346 341 016</td>
        <td>1:52.840</td>
      </tr>
      <tr>
        <td>m68k</td>
        <td>62 270 262</td>
        <td>3 327 921 564</td>
        <td style="color: #d7191c;">1:53.443</td>
      </tr>
      <tr>
        <td>mips</td>
        <td>87 981 027</td>
        <td>2 263 506 435</td>
        <td>1:25.727</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td>95 981 109</td>
        <td>3 176 876 928</td>
        <td>1:33.099</td>
      </tr>
      <tr>
        <td>mips64</td>
        <td>80 557 580</td>
        <td>2 277 631 169</td>
        <td>1:28.273</td>
      </tr>
      <tr>
        <td>mips64el</td>
        <td>88 557 574</td>
        <td>3 190 361 616</td>
        <td>1:36.026</td>
      </tr>
      <tr>
        <td>ppc</td>
        <td>48 136 797</td>
        <td>3 125 669 697</td>
        <td style="color: #d7191c; font-weight: bold;">1:64.933</td>
      </tr>
      <tr>
        <td>ppc64</td>
        <td>64 408 551</td>
        <td>3 203 728 174</td>
        <td>1:49.741</td>
      </tr>
      <tr>
        <td>ppc64le</td>
        <td>64 289 333</td>
        <td>3 203 064 933</td>
        <td>1:49.823</td>
      </tr>
      <tr>
        <td>riscv64</td>
        <td>78 623 128</td>
        <td>1 222 950 784</td>
        <td style="color: #4dac26;">1:15.555</td>
      </tr>
      <tr>
        <td>s390x</td>
        <td>46 190 841</td>
        <td>2 726 829 922</td>
        <td style="color: #d7191c;">1:59.034</td>
      </tr>
      <tr>
        <td>sh4</td>
        <td>88 962 981</td>
        <td>3 342 515 085</td>
        <td>1:37.572</td>
      </tr>
      <tr>
        <td>sparc64</td>
        <td>79 003 237</td>
        <td>3 207 541 031</td>
        <td>1:40.600</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td>61 517 622</td>
        <td>1 250 647 935</td>
        <td style="color: #4dac26;">1:20.330</td>
      </tr>
    </tbody>
  </table>
</div>

### qsort_double

<div style="overflow-x: auto;">
  <table
    class="results"
    style="width: 83%; margin-right: auto; margin-left: auto;"
  >
    <thead>
      <tr>
        <th title="Field #1">Target</th>
        <th title="Field #2">Guest Instructions</th>
        <th title="Field #3">QEMU Instructions</th>
        <th title="Field #4">Ratio</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td>159 746 207</td>
        <td>2 658 877 440</td>
        <td>1:16.644</td>
      </tr>
      <tr>
        <td>alpha</td>
        <td>228 521 249</td>
        <td>1 949 737 992</td>
        <td style="color: #4dac26; font-weight: bold;">1:8.532</td>
      </tr>
      <tr>
        <td>arm</td>
        <td>662 068 324</td>
        <td>9 121 836 857</td>
        <td>1:13.778</td>
      </tr>
      <tr>
        <td>hppa</td>
        <td>247 113 645</td>
        <td>3 141 276 704</td>
        <td>1:12.712</td>
      </tr>
      <tr>
        <td>m68k</td>
        <td>203 935 507</td>
        <td>4 934 908 874</td>
        <td style="color: #d7191c; font-weight: bold;">1:24.198</td>
      </tr>
      <tr>
        <td>mips</td>
        <td>207 350 635</td>
        <td>2 099 043 136</td>
        <td style="color: #4dac26;">1:10.123</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td>207 350 618</td>
        <td>2 099 343 286</td>
        <td>1:10.125</td>
      </tr>
      <tr>
        <td>mips64</td>
        <td>188 086 328</td>
        <td>1 971 371 119</td>
        <td>1:10.481</td>
      </tr>
      <tr>
        <td>mips64el</td>
        <td>188 086 318</td>
        <td>1 968 839 700</td>
        <td>1:10.468</td>
      </tr>
      <tr>
        <td>ppc</td>
        <td>224 876 043</td>
        <td>2 736 474 437</td>
        <td>1:12.169</td>
      </tr>
      <tr>
        <td>ppc64</td>
        <td>203 809 886</td>
        <td>2 685 763 461</td>
        <td>1:13.178</td>
      </tr>
      <tr>
        <td>ppc64le</td>
        <td>193 040 770</td>
        <td>2 642 651 058</td>
        <td>1:13.690</td>
      </tr>
      <tr>
        <td>riscv64</td>
        <td>167 397 846</td>
        <td>1 590 611 459</td>
        <td style="color: #4dac26;">1:9.502</td>
      </tr>
      <tr>
        <td>s390x</td>
        <td>130 867 251</td>
        <td>2 475 571 654</td>
        <td style="color: #d7191c;">1:18.917</td>
      </tr>
      <tr>
        <td>sh4</td>
        <td>244 843 868</td>
        <td>2 563 068 375</td>
        <td>1:10.468</td>
      </tr>
      <tr>
        <td>sparc64</td>
        <td>190 084 290</td>
        <td>3 919 439 599</td>
        <td style="color: #d7191c;">1:20.619</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td>156 689 097</td>
        <td>1 987 553 774</td>
        <td>1:12.685</td>
      </tr>
    </tbody>
  </table>
</div>

## Results (Benchmarks Group 2)

### matmult_int32

<div style="overflow-x: auto;">
  <table
    class="results"
    style="width: 83%; margin-right: auto; margin-left: auto;"
  >
    <thead>
      <tr>
        <th title="Field #1">Target</th>
        <th title="Field #2">Guest Instructions</th>
        <th title="Field #3">QEMU Instructions</th>
        <th title="Field #4">Ratio</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td>62 555 845</td>
        <td>596 194 508</td>
        <td style="color: #d7191c;">1:9.531</td>
      </tr>
      <tr>
        <td>alpha</td>
        <td>96 215 385</td>
        <td>370 654 042</td>
        <td style="color: #4dac26; font-weight: bold;">1:3.852</td>
      </tr>
      <tr>
        <td>arm</td>
        <td>63 690 750</td>
        <td>736 994 597</td>
        <td style="color: #d7191c; font-weight: bold;">1:11.571</td>
      </tr>
      <tr>
        <td>hppa</td>
        <td>103 978 473</td>
        <td>667 790 898</td>
        <td>1:6.422</td>
      </tr>
      <tr>
        <td>m68k</td>
        <td>62 534 491</td>
        <td>407 647 521</td>
        <td>1:6.519</td>
      </tr>
      <tr>
        <td>mips</td>
        <td>88 083 941</td>
        <td>497 767 190</td>
        <td>1:5.651</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td>88 083 929</td>
        <td>497 780 326</td>
        <td>1:5.651</td>
      </tr>
      <tr>
        <td>mips64</td>
        <td>89 460 954</td>
        <td>479 725 676</td>
        <td>1:5.362</td>
      </tr>
      <tr>
        <td>mips64el</td>
        <td>89 460 943</td>
        <td>463 106 726</td>
        <td style="color: #4dac26;">1:5.177</td>
      </tr>
      <tr>
        <td>ppc</td>
        <td>55 843 156</td>
        <td>338 959 876</td>
        <td>1:6.070</td>
      </tr>
      <tr>
        <td>ppc64</td>
        <td>64 204 690</td>
        <td>390 884 485</td>
        <td>1:6.088</td>
      </tr>
      <tr>
        <td>ppc64le</td>
        <td>64 205 395</td>
        <td>390 743 122</td>
        <td>1:6.086</td>
      </tr>
      <tr>
        <td>riscv64</td>
        <td>86 448 202</td>
        <td>349 669 158</td>
        <td style="color: #4dac26;">1:4.045</td>
      </tr>
      <tr>
        <td>s390x</td>
        <td>62 614 807</td>
        <td>492 407 746</td>
        <td style="color: #d7191c;">1:7.864</td>
      </tr>
      <tr>
        <td>sh4</td>
        <td>72 780 143</td>
        <td>399 937 800</td>
        <td>1:5.495</td>
      </tr>
      <tr>
        <td>sparc64</td>
        <td>86 423 179</td>
        <td>489 936 356</td>
        <td>1:5.669</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td>61 590 922</td>
        <td>400 190 791</td>
        <td>1:6.498</td>
      </tr>
    </tbody>
  </table>
</div>

### qsort_int32

<div style="overflow-x: auto;">
  <table
    class="results"
    style="width: 83%; margin-right: auto; margin-left: auto;"
  >
    <thead>
      <tr>
        <th title="Field #1">Target</th>
        <th title="Field #2">Guest Instructions</th>
        <th title="Field #3">QEMU Instructions</th>
        <th title="Field #4">Ratio</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td>151 968 514</td>
        <td>2 132 112 102</td>
        <td>1:14.030</td>
      </tr>
      <tr>
        <td>alpha</td>
        <td>221 192 248</td>
        <td>1 460 982 497</td>
        <td style="color: #4dac26; font-weight: bold;">1:6.605</td>
      </tr>
      <tr>
        <td>arm</td>
        <td>160 875 621</td>
        <td>3 375 777 484</td>
        <td style="color: #d7191c; font-weight: bold;">1:20.984</td>
      </tr>
      <tr>
        <td>hppa</td>
        <td>201 401 936</td>
        <td>2 199 407 458</td>
        <td>1:10.920</td>
      </tr>
      <tr>
        <td>m68k</td>
        <td>169 894 134</td>
        <td>1 780 208 909</td>
        <td>1:10.478</td>
      </tr>
      <tr>
        <td>mips</td>
        <td>176 712 823</td>
        <td>1 501 040 830</td>
        <td>1:8.494</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td>176 712 809</td>
        <td>1 503 808 218</td>
        <td>1:8.510</td>
      </tr>
      <tr>
        <td>mips64</td>
        <td>176 020 831</td>
        <td>1 504 536 270</td>
        <td>1:8.547</td>
      </tr>
      <tr>
        <td>mips64el</td>
        <td>176 020 824</td>
        <td>1 483 550 240</td>
        <td>1:8.428</td>
      </tr>
      <tr>
        <td>ppc</td>
        <td>202 473 828</td>
        <td>1 668 592 063</td>
        <td style="color: #4dac26;">1:8.241</td>
      </tr>
      <tr>
        <td>ppc64</td>
        <td>198 918 772</td>
        <td>1 780 051 140</td>
        <td>1:8.949</td>
      </tr>
      <tr>
        <td>ppc64le</td>
        <td>188 749 603</td>
        <td>1 728 567 792</td>
        <td>1:9.158</td>
      </tr>
      <tr>
        <td>riscv64</td>
        <td>159 048 448</td>
        <td>1 289 755 584</td>
        <td style="color: #4dac26;">1:8.109</td>
      </tr>
      <tr>
        <td>s390x</td>
        <td>132 119 768</td>
        <td>2 114 840 292</td>
        <td style="color: #d7191c;">1:16.007</td>
      </tr>
      <tr>
        <td>sh4</td>
        <td>205 090 416</td>
        <td>1 879 285 254</td>
        <td>1:9.163</td>
      </tr>
      <tr>
        <td>sparc64</td>
        <td>185 195 979</td>
        <td>3 352 756 658</td>
        <td style="color: #d7191c;">1:18.104</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td>145 621 672</td>
        <td>1 751 799 973</td>
        <td>1:12.030</td>
      </tr>
    </tbody>
  </table>
</div>

### qsort_string

<div style="overflow-x: auto;">
  <table
    class="results"
    style="width: 83%; margin-right: auto; margin-left: auto;"
  >
    <thead>
      <tr>
        <th title="Field #1">Target</th>
        <th title="Field #2">Guest Instructions</th>
        <th title="Field #3">QEMU Instructions</th>
        <th title="Field #4">Ratio</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td>237 478 279</td>
        <td>2 530 968 853</td>
        <td>1:10.658</td>
      </tr>
      <tr>
        <td>alpha</td>
        <td>310 349 344</td>
        <td>1 794 207 498</td>
        <td style="color: #4dac26; font-weight: bold;">1:5.781</td>
      </tr>
      <tr>
        <td>arm</td>
        <td>277 491 839</td>
        <td>7 167 746 267</td>
        <td style="color: #d7191c; font-weight: bold;">1:25.830</td>
      </tr>
      <tr>
        <td>hppa</td>
        <td>286 010 885</td>
        <td>4 608 364 139</td>
        <td>1:16.113</td>
      </tr>
      <tr>
        <td>m68k</td>
        <td>242 574 561</td>
        <td>2 295 663 078</td>
        <td>1:9.464</td>
      </tr>
      <tr>
        <td>mips</td>
        <td>331 063 420</td>
        <td>2 114 226 632</td>
        <td style="color: #4dac26;">1:6.386</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td>331 063 408</td>
        <td>2 111 085 204</td>
        <td style="color: #4dac26;">1:6.377</td>
      </tr>
      <tr>
        <td>mips64</td>
        <td>304 640 414</td>
        <td>1 969 109 275</td>
        <td>1:6.464</td>
      </tr>
      <tr>
        <td>mips64el</td>
        <td>304 640 409</td>
        <td>1 951 425 342</td>
        <td>1:6.406</td>
      </tr>
      <tr>
        <td>ppc</td>
        <td>320 946 236</td>
        <td>2 429 421 810</td>
        <td>1:7.570</td>
      </tr>
      <tr>
        <td>ppc64</td>
        <td>272 956 914</td>
        <td>2 404 978 156</td>
        <td>1:8.811</td>
      </tr>
      <tr>
        <td>ppc64le</td>
        <td>273 392 915</td>
        <td>2 386 256 069</td>
        <td>1:8.728</td>
      </tr>
      <tr>
        <td>riscv64</td>
        <td>216 826 004</td>
        <td>1 564 149 511</td>
        <td>1:7.214</td>
      </tr>
      <tr>
        <td>s390x</td>
        <td>165 265 303</td>
        <td>4 189 211 923</td>
        <td style="color: #d7191c;">1:25.348</td>
      </tr>
      <tr>
        <td>sh4</td>
        <td>287 459 667</td>
        <td>2 098 659 130</td>
        <td>1:7.301</td>
      </tr>
      <tr>
        <td>sparc64</td>
        <td>304 142 262</td>
        <td>4 130 702 783</td>
        <td style="color: #d7191c;">1:13.581</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td>234 574 652</td>
        <td>2 865 446 064</td>
        <td>1:12.215</td>
      </tr>
    </tbody>
  </table>
</div>

## Analysis

The tables above are color coded to show the three best and worst emulation ratios for each benchmark. It can be noticed that within the same benchmark group, the ratios for all seventeen targets are nearly consistent.

It's also clear that the ratio depends on the type of the program being emulated. Benchmarks in group 1 have a considerably larger emulation ratio compared to benchmarks in group 2.

The Python script below averages the ratios across different tables for each target. The results give a very good overview of QEMU's **emulation efficiency** for each of the seventeen targets.

```python
import os
import csv

# Tables directory
tables = os.listdir("tables")
csv_headers = ["Target", "QEMU Efficiency"]

# Initialize target arrays
target_names, target_ratio_sums = [], []
with open(os.path.join("tables", tables[0]), "r") as file:
    # Skip headers line
    file.readline()
    lines = file.readlines()
    for line in lines:
        # Add target name
        target_names.append(line.split(",")[0])
        # Initialize sum to zero
        target_ratio_sums.append(0)

# Number of benchmarks and targets
no_benchmarks = len(tables)
no_targets = len(target_names)

for table in tables:
    with open(os.path.join("tables", table), "r") as file:
        file.readline()
        lines = file.readlines()
        for i in range(len(lines)):
            target_ratio_sums[i] += float(lines[i].split(",")
                                          [-1].split(":")[-1])


target_ratio_avgs = ["1:"+str(round((x / no_benchmarks), 3))
                     for x in target_ratio_sums]

with open("efficiency.csv", "w") as file:
    writer = csv.writer(file)
    writer.writerow(csv_headers)
    for i in range(no_targets):
        writer.writerow([target_names[i], target_ratio_avgs[i]])
```

The script can be ran three times to obtain three tables.

On the left is the table for averaging the three benchmarks in group 1. The table in the middle represents the average ratio for benchmarks in group 2. Lastly, the table on the right is the average of all six benchmarks.

<style>
  table#efficiency td:nth-child(1) {
    text-align: left;
  }
  table#efficiency th:nth-child(1) {
    text-align: left;
  }
</style>

<style>
  .wrap {
    display: flex;
    align-items: center;
    justify-content: center;
  }
  table#efficiency td:nth-child(1) {
    text-align: left;
  }
  table#efficiency th:nth-child(1) {
    text-align: left;
  }
</style>
<style>
  .wrap {
    display: flex;
    align-items: center;
    justify-content: center;
  }
  table#efficiency td:nth-child(1) {
    text-align: left;
  }
  table#efficiency th:nth-child(1) {
    text-align: left;
  }
</style>
<div class="wrap">
  <table id="efficiency" class="results" style="margin-right: 3%; width: 33%;">
    <thead>
      <tr>
        <th title="Field #1">Target</th>
        <th title="Field #2">QEMU Efficiency (group 1)</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td>1:21.135</td>
      </tr>
      <tr>
        <td>alpha</td>
        <td>1:23.639</td>
      </tr>
      <tr>
        <td>arm</td>
        <td style="color: #4dac26; font-weight: bold;">1:10.772</td>
      </tr>
      <tr>
        <td>hppa</td>
        <td>1:35.638</td>
      </tr>
      <tr>
        <td>m68k</td>
        <td style="color: #d7191c; font-weight: bold;">1:68.595</td>
      </tr>
      <tr>
        <td>mips</td>
        <td>1:23.516</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td>1:26.754</td>
      </tr>
      <tr>
        <td>mips64</td>
        <td>1:25.751</td>
      </tr>
      <tr>
        <td>mips64el</td>
        <td>1:29.250</td>
      </tr>
      <tr>
        <td>ppc</td>
        <td style="color: #d7191c;">1:43.826</td>
      </tr>
      <tr>
        <td>ppc64</td>
        <td>1:40.008</td>
      </tr>
      <tr>
        <td>ppc64le</td>
        <td>1:40.801</td>
      </tr>
      <tr>
        <td>riscv64</td>
        <td style="color: #4dac26;">1:14.813</td>
      </tr>
      <tr>
        <td>s390x</td>
        <td style="color: #d7191c;">1:43.043</td>
      </tr>
      <tr>
        <td>sh4</td>
        <td>1:24.979</td>
      </tr>
      <tr>
        <td>sparc64</td>
        <td>1:35.560</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td style="color: #4dac26;">1:17.868</td>
      </tr>
    </tbody>
  </table>

  <table id="efficiency" class="results" style="margin-right: 3%; width: 33%;">
    <thead>
      <tr>
        <th title="Field #1">Target</th>
        <th title="Field #2">QEMU Efficiency (group 2)</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td>1:11.406</td>
      </tr>
      <tr>
        <td>alpha</td>
        <td style="color: #4dac26; font-weight: bold;">1:5.413</td>
      </tr>
      <tr>
        <td>arm</td>
        <td style="color: #d7191c; font-weight: bold;">1:19.462</td>
      </tr>
      <tr>
        <td>hppa</td>
        <td>1:11.152</td>
      </tr>
      <tr>
        <td>m68k</td>
        <td>1:8.82</td>
      </tr>
      <tr>
        <td>mips</td>
        <td>1:6.844</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td>1:6.846</td>
      </tr>
      <tr>
        <td>mips64</td>
        <td>1:6.791</td>
      </tr>
      <tr>
        <td>mips64el</td>
        <td style="color: #4dac26;">1:6.670</td>
      </tr>
      <tr>
        <td>ppc</td>
        <td>1:7.294</td>
      </tr>
      <tr>
        <td>ppc64</td>
        <td>1:7.949</td>
      </tr>
      <tr>
        <td>ppc64le</td>
        <td>1:7.991</td>
      </tr>
      <tr>
        <td>riscv64</td>
        <td style="color: #4dac26;">1:6.456</td>
      </tr>
      <tr>
        <td>s390x</td>
        <td style="color: #d7191c;">1:16.406</td>
      </tr>
      <tr>
        <td>sh4</td>
        <td>1:7.32</td>
      </tr>
      <tr>
        <td>sparc64</td>
        <td style="color: #d7191c;">1:12.451</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td>1:10.248</td>
      </tr>
    </tbody>
  </table>

  <table id="efficiency" class="results" style="width: 32%;">
    <thead>
      <tr>
        <th title="Field #1">Target</th>
        <th title="Field #2">QEMU Efficiency (overall)</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td>1:16.270</td>
      </tr>
      <tr>
        <td>alpha</td>
        <td style="color: #4dac26; font-weight: bold;">1:14.526</td>
      </tr>
      <tr>
        <td>arm</td>
        <td style="color: #4dac26;">1:15.117</td>
      </tr>
      <tr>
        <td>hppa</td>
        <td>1:23.395</td>
      </tr>
      <tr>
        <td>m68k</td>
        <td style="color: #d7191c; font-weight: bold;">1:38.708</td>
      </tr>
      <tr>
        <td>mips</td>
        <td>1:15.180</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td>1:16.800</td>
      </tr>
      <tr>
        <td>mips64</td>
        <td>1:16.271</td>
      </tr>
      <tr>
        <td>mips64el</td>
        <td>1:17.960</td>
      </tr>
      <tr>
        <td>ppc</td>
        <td style="color: #d7191c;">1:25.560</td>
      </tr>
      <tr>
        <td>ppc64</td>
        <td>1:23.979</td>
      </tr>
      <tr>
        <td>ppc64le</td>
        <td>1:24.396</td>
      </tr>
      <tr>
        <td>riscv64</td>
        <td style="color: #4dac26;">1:10.635</td>
      </tr>
      <tr>
        <td>s390x</td>
        <td style="color: #d7191c;">1:29.725</td>
      </tr>
      <tr>
        <td>sh4</td>
        <td>1:16.149</td>
      </tr>
      <tr>
        <td>sparc64</td>
        <td>1:24.006</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td>1:14.058</td>
      </tr>
    </tbody>
  </table>
</div>

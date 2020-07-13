---
layout: post
title: Listing QEMU Helpers and Function Callees
subtitle: Using list_helpers.py and list_fn_callees.py for Performance Analysis
tags:
  [
    list_helpers,
    list_fn_callees,
    5.1-pre-soft-freeze,
    softfloat,
    ppc,
    hppa,
    sh4,
    mips,
  ]
excerpt: This report builds up on the previous one by presenting two new Python scripts that facilitates the process of displaying the executed QEMU helpers and function callees without the need of setting up KCachegrind. The PowerPC performance degradation is then re-analysed using the new scripts. The report also introduces the analysis of three other targets, hppa and sh4, explaining why they were not affected the same way as ppc, and mips, explaining why it showed an increase in performance.
---

## Intro

The previous report introduced a performance comparison between QEMU versions 5.0 and 5.1-pre-soft-freeze. The results showed an approximate 2.45% performance degradation in all PowerPC targets. To further analyze the results, the report introduced KCachegrind to compare the list of QEMU helpers executed in the two versions as well as to list the callees of these helpers.

This report presents two new Python scripts that facilitates the process of displaying the executed QEMU helpers and function callees without the need of setting up KCachegrind. The ppc/ppc64/ppc64le performance degradation is re-analysed using the new scripts. The report also introduces the analysis of three other targets, hppa and sh4, explaining why they were not affected the same way as ppc, and mips, explaining why it showed an increase in performance.

## Table of Contents

- [Introducing the Scripts](#introducing-the-scripts)
  - [Examples of Usage](#examples-of-usage)
  - [Principle of Operation](#principle-of-operation)
- [Re-analyzing ppc Performance 5.0 VS 5.1-pre-soft-freeze](#re-analyzing-ppc-performance-50-vs-51-pre-soft-freeze)
- [Analyzing hppa Performance 5.0 VS 5.1-pre-soft-freeze](#analyzing-hppa-performance-50-vs-51-pre-soft-freeze)
- [Analyzing sh4 Performance 5.0 VS 5.1-pre-soft-freeze](#analyzing-sh4-performance-50-vs-51-pre-soft-freeze)
- [Analyzing mips Performance 5.0 VS 5.1-pre-soft-freeze](#analyzing-mips-performance-50-vs-51-pre-soft-freeze)
- [Appendix](#appendix)
  - [Float Comparison Helper of ppc](#float-comparison-helper-of-ppc)
  - [Float Comparison Helper of hppa](#float-comparison-helper-of-hppa)
  - [Float Comparison Helpers of sh4](#float-comparison-helpers-of-sh4)
  - [Float Operation Helpers of mips](#float-operation-helpers-of-mips)

## Introducing the Scripts

The `list_helpers.py` script - as the name suggests - is used to list all helpers executed during a QEMU invocation. In the first part of its output, the script prints the total number of executed instructions. After that, it lists the executed helpers with the following info for each one:

1. Number of inclusive instructions
2. Overall percentage
3. Number of calls
4. Number of instructions per call
5. Function name
6. Source file

The `list_fn_callees.py` is a generalization of the `list_helpers.py` script. The `list_helpers.py` script works under the hood by listing the callees of the JIT call. The `list_fn_callees.py` script extends this by giving the user the ability to list the callees of any executed QEMU function(s). The script takes one required argument, `-f`, which is a list of space separated QEMU functions. For each function, the script prints the list of the function callees in a similar manner as `list_helpers.py` does.

Both scripts are available on the project [GitHub page](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/tree/master/tools/list_helpers_and_fn_callees).

### Examples of Usage

Compile the [coulomb_float](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/blob/master/benchmakrs/coulomb/coulomb_float.c) benchmark on an x86_64 Linux machine:

```bash
gcc -static -O2 coulomb_double.c -o coulomb_double -lm
```

To list the executed helpers, the `list_helpers.py` script can be invoked using:

```bash
./list_helpers.py -- <qemu-build>/x86_64-linux-user/qemu-x86_64 coulomb_double
```

Output:

```
Total number of instructions: 4,701,725,992

Executed QEMU Helpers:

 No.     Instructions  Percentage            Calls    Ins/Call  Helper Name                Source File
----  ---------------  ----------  ---------------  ----------  -------------------------  ------------------------------
   1    1,139,103,365     24.227%       21,490,812          53  helper_mulsd               <qemu>/target/i386/ops_sse.h
   2      906,697,049     19.284%       18,470,085          49  helper_addsd               <qemu>/target/i386/ops_sse.h
   3      858,124,043     18.251%       16,940,796          50  helper_subsd               <qemu>/target/i386/ops_sse.h
   4      211,982,677      4.509%        7,202,293          29  helper_ucomisd             <qemu>/target/i386/ops_sse.h
   5      154,316,493      3.282%        2,655,074          58  helper_lookup_tb_ptr       <qemu>/accel/tcg/tcg-runtime.c
   6       80,851,232      1.720%          459,382         176  helper_cvttsd2si           <qemu>/target/i386/ops_sse.h
   7       63,073,459      1.341%        1,261,468          50  helper_divsd               <qemu>/target/i386/ops_sse.h
   8       30,787,517      0.655%        2,646,130          11  helper_cc_compute_all      <qemu>/target/i386/cc_helper.c
   9       24,699,785      0.525%        4,939,957           5  helper_pand_xmm            <qemu>/target/i386/ops_sse.h
  10       14,266,055      0.303%        2,853,211           5  helper_pxor_xmm            <qemu>/target/i386/ops_sse.h
  11        8,885,615      0.189%        1,777,123           5  helper_por_xmm             <qemu>/target/i386/ops_sse.h
  12        5,714,358      0.122%            5,722         998  helper_divq_EAX            <qemu>/target/i386/int_helper.c
  13        2,435,265      0.052%           30,065          81  helper_pcmpeqb_xmm         <qemu>/target/i386/ops_sse.h
  14        1,900,764      0.040%          211,196           9  helper_pandn_xmm           <qemu>/target/i386/ops_sse.h
  15        1,200,024      0.026%           19,048          63  helper_pmovmskb_xmm        <qemu>/target/i386/ops_sse.h
  16          278,000      0.006%            2,000         139  helper_cvtsi2sd            <qemu>/target/i386/ops_sse.h
  17          260,732      0.006%           24,471          10  helper_cc_compute_c        <qemu>/target/i386/cc_helper.c
  18          225,270      0.005%            5,006          45  helper_punpcklbw_xmm       <qemu>/target/i386/ops_sse.h
  19           95,133      0.002%            5,007          19  helper_pshufd_xmm          <qemu>/target/i386/ops_sse.h
  20           75,090      0.002%            5,006          15  helper_punpcklwd_xmm       <qemu>/target/i386/ops_sse.h
  21           36,000      0.001%            1,000          36  helper_sqrtsd              <qemu>/target/i386/ops_sse.h
  22           28,000      0.001%            4,000           7  helper_movmskpd            <qemu>/target/i386/ops_sse.h
  23           20,028      0.000%            5,007           4  helper_movl_mm_T0_xmm      <qemu>/target/i386/ops_sse.h
  24           17,000      0.000%            1,000          17  helper_idivl_EAX           <qemu>/target/i386/int_helper.c
  25            8,000      0.000%            4,000           2  helper_fnstcw              <qemu>/target/i386/fpu_helper.c
  26            3,354      0.000%               43          78  helper_syscall             <qemu>/target/i386/seg_helper.c
  27            1,497      0.000%               13         115  helper_cpuid               <qemu>/target/i386/misc_helper.c
  28              775      0.000%               31          25  helper_pcmpgtl_xmm         <qemu>/target/i386/ops_sse.h
  29              720      0.000%                6         120  helper_pslldq_xmm          <qemu>/target/i386/ops_sse.h
  30              625      0.000%              125           5  helper_paddq_xmm           <qemu>/target/i386/ops_sse.h
  31              558      0.000%               62           9  helper_paddl_xmm           <qemu>/target/i386/ops_sse.h
  32              528      0.000%               16          33  helper_psubb_xmm           <qemu>/target/i386/ops_sse.h
  33              372      0.000%               62           6  helper_psllq_xmm           <qemu>/target/i386/ops_sse.h
  34              310      0.000%               62           5  helper_punpckhqdq_xmm      <qemu>/target/i386/ops_sse.h
  35              279      0.000%               31           9  helper_punpckhdq_xmm       <qemu>/target/i386/ops_sse.h
  36              248      0.000%               31           8  helper_pslld_xmm           <qemu>/target/i386/ops_sse.h
  37              217      0.000%               31           7  helper_punpckldq_xmm       <qemu>/target/i386/ops_sse.h
  38              216      0.000%                2         108  helper_psrldq_xmm          <qemu>/target/i386/ops_sse.h
  39              198      0.000%               66           3  helper_punpcklqdq_xmm      <qemu>/target/i386/ops_sse.h
  40              124      0.000%                2          62  helper_idivq_EAX           <qemu>/target/i386/int_helper.c
  41               25      0.000%                1          25  helper_pcmpeql_xmm         <qemu>/target/i386/ops_sse.h
  42               24      0.000%                1          24  helper_rdtsc               <qemu>/target/i386/misc_helper.c
```

To list the the callees of `helper_mulsd` and `helper_addsd`, the `list_fn_callees.py` script can be invoked using:

<pre class="highlight" style="font-size:14.6205px">
<code>./list_fn_callees.py <span style="color: #000080">-f</span> <b>helper_mulsd helper_addsd</b> -- &lt;qemu-build&gt;/x86_64-linux-user/qemu-x86_64 coulomb_double</code>
</pre>

Output:

```
Total number of instructions: 4,703,399,623

Callees of helper_mulsd:

 No.     Instructions  Percentage            Calls    Ins/Call  Function Name              Source File
----  ---------------  ----------  ---------------  ----------  -------------------------  ------------------------------
   1      924,195,245     19.650%       21,490,812          43  float64_mul                <qemu>/fpu/softfloat.c


Callees of helper_addsd:

 No.     Instructions  Percentage            Calls    Ins/Call  Function Name              Source File
----  ---------------  ----------  ---------------  ----------  -------------------------  ------------------------------
   1      721,996,199     15.351%       18,470,085          39  float64_add                <qemu>/fpu/softfloat.c
```

### Principle of Operation

The script executes the passed QEMU invocation command with Callgrind. It then uses `callgrind_annotate` with two new flags, `--tree=calling` and `--threshold=100`.

The `--tree=calling` flag is used to list the callees of each function, while the `--threshold=100` flag is used to set a threshold on the displayed cost percentage. `callgrind_annotate` stops printing functions when the sum of the cost percentage of the printed functions is bigger than or equal to the given threshold percentage. The default percentage is 99.

#### Understanding the "--tree=calling" Flag Output

To better understand the `--tree=calling` flag output, consider the excerpt below obtained from running `callgrind_annotate` on the Callgrind output of the x86_64 version of the Coulomb benchmark:

```
157    8,728,258  *  <qemu>/fpu/softfloat.c:float64_to_int32_scalbn [<qemu-build>/x86_64-linux-user/qemu-x86_64]
158   45,938,200  >  <qemu>/fpu/softfloat.c:round_to_int_and_pack (459382x) [<qemu-build>/x86_64-linux-user/qemu-x86_64]
159   15,618,988  >  <qemu>/fpu/softfloat.c:float64_unpack_canonical (459382x) [<qemu-build>/x86_64-linux-user/qemu-x86_64]
160
161    7,965,240  *  <qemu>/include/exec/tb-hash.h:helper_lookup_tb_ptr
162
163    7,350,112  *  <qemu>/target/i386/ops_sse.h:helper_cvttsd2si [<qemu-build>/x86_64-linux-user/qemu-x86_64]
164   72,122,974  >  <qemu>/fpu/softfloat.c:float64_to_int32_round_to_zero (459382x) [<qemu-build>/x86_64-linux-user/qemu-x86_64]
```

A line can come in two forms, either with a `*` after the number of instructions, or with a `>`.

A line with `*` indicates that it contains the measurements of a top-level function. All lines following it with a `>` are this function callees. The two line forms have different set of obtained measurements.

**Example of a top-level function (line number 157):**

- `8,728,258` - The number of **self** instructions of the function.
- `*` - Indicates that this is a top-level function.
- `<qemu>/fpu/softfloat.c:float64_to_int32_scalbn` - Function source file
- `[<qemu-build>/x86_64-linux-user/qemu-x86_64]` - Program executing the function

**Example of a function callee (line number 158):**

- `45,938,200` - The number of **inclusive** instructions of the function.
- `>` - Indicates that this is a callee of the top-level function.
- `<qemu>/fpu/softfloat.c:round_to_int_and_pack` - Function source file.
- `(459382x)` - Number of function calls.
- `[<qemu-build>/x86_64-linux-user/qemu-x86_64]` - Program executing the function.

The `list_helpers.py` and `list_fn_callees.py` scripts use the information above for searching and printing the callee details of the desired functions.

## Re-analyzing ppc Performance 5.0 VS 5.1-pre-soft-freeze

The previous report concluded that the changes made in SoftFloat by inlining the float64 compare specializations were the reason behind the PowerPC performance degradation. This section concludes the same, but this time by using the `list_helpers.py` and `list_fn_callees.py` scripts instead of KCachegrind.

Finding list of ppc helpers for QEMU 5.0:

```bash
./list_helpers.py -- <qemu-build>/ppc-linux-user/qemu-ppc coulomb_double-ppc
```

Results:

<pre class="highlight" style="font-size:14.6205px">
<code><b>Total number of instructions: 12,713,132,146</b>

Executed QEMU Helpers:

 No.     Instructions  Percentage            Calls    Ins/Call  Function Name               Source File
----  ---------------  ----------  ---------------  ----------  -------------------------   ------------------------------
   1    4,079,150,406     32.092%       14,765,516         276  helper_fmadd                &lt;qemu&gt;/qemu-5.0.0/target/ppc/fpu_helper.c
   2    2,019,505,224     15.888%       39,614,918          50  helper_compute_fprf_float64 &lt;qemu&gt;/qemu-5.0.0/target/ppc/fpu_helper.c
   3    1,136,334,017      8.940%        8,660,551         131  helper_fsub                 &lt;qemu&gt;/qemu-5.0.0/target/ppc/fpu_helper.c
   4    1,057,997,648      8.324%        8,110,271         130  helper_fadd                 &lt;qemu&gt;/qemu-5.0.0/target/ppc/fpu_helper.c
   5      773,545,891      6.086%        5,475,082         141  helper_fmul                 &lt;qemu&gt;/qemu-5.0.0/target/ppc/fpu_helper.c
   6      760,150,923      5.980%       46,826,121          16  helper_float_check_status   &lt;qemu&gt;/qemu-5.0.0/target/ppc/fpu_helper.c
   <b>7      632,546,190      4.976%        7,209,203          87  helper_fcmpu                &lt;qemu&gt;/qemu-5.0.0/target/ppc/fpu_helper.c</b>
   8      258,993,128      2.038%          913,858         283  helper_fnmadd               &lt;qemu&gt;/qemu-5.0.0/target/ppc/fpu_helper.c
   9      158,379,826      1.246%        1,261,466         125  helper_fdiv                 &lt;qemu&gt;/qemu-5.0.0/target/ppc/fpu_helper.c
  10      110,558,579      0.870%        2,167,299          51  helper_lookup_tb_ptr        &lt;qemu&gt;/qemu-5.0.0/accel/tcg/tcg-runtime.c
  11      109,016,868      0.858%          427,174         255  helper_fmsub                &lt;qemu&gt;/qemu-5.0.0/target/ppc/fpu_helper.c
  12       96,581,006      0.760%       48,290,503           2  helper_reset_fpstatus       &lt;qemu&gt;/qemu-5.0.0/include/fpu/softfloat-helpers.h
  13       79,473,086      0.625%          459,382         173  helper_fctiwz               &lt;qemu&gt;/qemu-5.0.0/target/ppc/fpu_helper.c
  14          266,000      0.002%            2,000         133  helper_store_fpscr          &lt;qemu&gt;/qemu-5.0.0/target/ppc/fpu_helper.c
  15          247,317      0.002%            1,000         247  helper_fnmsub               &lt;qemu&gt;/qemu-5.0.0/target/ppc/fpu_helper.c
  16           48,000      0.000%            2,000          24  helper_todouble             &lt;qemu&gt;/qemu-5.0.0/target/ppc/fpu_helper.c
  17            3,486      0.000%               42          83  helper_raise_exception_err  &lt;qemu&gt;/qemu-5.0.0/target/ppc/excp_helper.c
  18            2,380      0.000%               14         170  helper_dcbz                 &lt;qemu&gt;/qemu-5.0.0/target/ppc/mem_helper.c
  </code>
</pre>

Finding list of ppc helpers for QEMU 5.1-pre-soft-freeze:

```bash
./list_helpers.py -- <qemu-master-build>/ppc-linux-user/qemu-ppc coulomb_double-ppc
```

Results:

<pre class="highlight" style="font-size:14.6205px">
<code><b>Total number of instructions: 13,033,447,522</b>

Executed QEMU Helpers:

 No.     Instructions  Percentage            Calls    Ins/Call  Function Name               Source File
----  ---------------  ----------  ---------------  ----------  -------------------------   ------------------------------
   1    4,079,150,406     31.303%       14,765,516         276  helper_fmadd                &lt;qemu&gt;/qemu/target/ppc/fpu_helper.c
   2    2,019,505,224     15.498%       39,614,918          50  helper_compute_fprf_float64 &lt;qemu&gt;/qemu/target/ppc/fpu_helper.c
   3    1,136,334,017      8.720%        8,660,551         131  helper_fsub                 &lt;qemu&gt;/qemu/target/ppc/fpu_helper.c
   4    1,057,997,648      8.119%        8,110,271         130  helper_fadd                 &lt;qemu&gt;/qemu/target/ppc/fpu_helper.c
   <b>5      950,907,056      7.297%        7,209,203         131  helper_fcmpu                &lt;qemu&gt;/qemu/target/ppc/fpu_helper.c</b>
   6      773,545,891      5.936%        5,475,082         141  helper_fmul                 &lt;qemu&gt;/qemu/target/ppc/fpu_helper.c
   7      760,150,923      5.833%       46,826,121          16  helper_float_check_status   &lt;qemu&gt;/qemu/target/ppc/fpu_helper.c
   8      258,993,128      1.988%          913,858         283  helper_fnmadd               &lt;qemu&gt;/qemu/target/ppc/fpu_helper.c
   9      158,379,826      1.215%        1,261,466         125  helper_fdiv                 &lt;qemu&gt;/qemu/target/ppc/fpu_helper.c
  10      110,558,579      0.848%        2,167,299          51  helper_lookup_tb_ptr        &lt;qemu&gt;/qemu/accel/tcg/tcg-runtime.c
  11      109,016,868      0.837%          427,174         255  helper_fmsub                &lt;qemu&gt;/qemu/target/ppc/fpu_helper.c
  12       96,581,006      0.741%       48,290,503           2  helper_reset_fpstatus       &lt;qemu&gt;/qemu/include/fpu/softfloat-helpers.h
  13       80,391,850      0.617%          459,382         175  helper_fctiwz               &lt;qemu&gt;/qemu/target/ppc/fpu_helper.c
  14          266,000      0.002%            2,000         133  helper_store_fpscr          &lt;qemu&gt;/qemu/target/ppc/fpu_helper.c
  15          247,317      0.002%            1,000         247  helper_fnmsub               &lt;qemu&gt;/qemu/target/ppc/fpu_helper.c
  16           48,000      0.000%            2,000          24  helper_todouble             &lt;qemu&gt;/qemu/target/ppc/fpu_helper.c
  17            3,486      0.000%               42          83  helper_raise_exception_err  &lt;qemu&gt;/qemu/target/ppc/excp_helper.c
  18            2,618      0.000%               14         187  helper_dcbz                 &lt;qemu&gt;/qemu/target/ppc/mem_helper.c
  </code>
</pre>

To further inpsect `helper_fcmpu`. The `list_fn_callees.py` script is used to list the helper callees.

Finding list of helper_fcmpu callees for QEMU 5.0:

<pre class="highlight" style="font-size:14.6205px">
<code>./list_fn_callees.py <span style="color: #000080">-f</span> <b>helper_fcmpu</b> -- &lt;qemu-build&gt;/ppc-linux-user/qemu-ppc coulomb_double-ppc</code>
</pre>

Results:

<pre class="highlight" style="font-size:14.6205px">
<code>Total number of instructions: 12,713,132,146

Callees of helper_fcmpu:
 No.     Instructions  Percentage            Calls    Ins/Call  Function Name              Source File
----  ---------------  ----------  ---------------  ----------  -------------------------  ------------------------------
   <b>1      203,052,887      1.597%        7,209,203          28  float64_lt                 &lt;qemu&gt;/qemu-5.0.0/fpu/softfloat.c</b>
   <b>2      140,825,653      1.108%        4,856,057          29  float64_le                 &lt;qemu&gt;/qemu-5.0.0/fpu/softfloat.c</b>
  </code>
</pre>

Finding list of helper_fcmpu callees for QEMU 5.1-pre-soft-freeze:

<pre class="highlight" style="font-size:14.6205px">
<code>./list_fn_callees.py <span style="color: #000080">-f</span> <b>helper_fcmpu</b> -- &lt;qemu-master-build&gt;/ppc-linux-user/qemu-ppc coulomb_double-ppc</code>
</pre>

Results:

<pre class="highlight" style="font-size:14.6205px">
<code>Total number of instructions: 13,033,447,522:

Callees of helper_fcmpu:
 No.     Instructions  Percentage            Calls    Ins/Call  Function Name              Source File
----  ---------------  ----------  ---------------  ----------  -------------------------  ------------------------------
   <b>1      662,239,406      5.082%       12,065,260          54  float64_compare            &lt;qemu&gt;/qemu/fpu/softfloat.c</b>
  </code>
</pre>

This concludes - same as previous report- that replacing the float64 compare specializations with inline functions that call the standard float64_compare functions is the reason behind the PowerPC performance degradation.

Looking back on the summary of the performance comparison presented in the previous report, beside the performance degradation introduced in the PowerPC targets, other targets had no change in performance. The next two sections analyze two of such targets using the `list_helpers.py` and `list_fn_callees.py` scripts.

<style>
  /* Collapse multiple borders into one */
  table#compare {
    table-layout:fixed;
    width:80%;
    overflow:hidden;
    word-wrap:break-word;
    border-collapse: collapse;
    margin-right: auto;
    margin-left: auto;
  }
  /* Borders for table header */
  table#compare th {
    border: 1px solid black;
    text-align: center;
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
        <th>Decrease</th>
        <th>No Change</th>
        <th>Increase</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>
          <div style="float: left;">ppc</div>
          <div style="float: right;">2.458%</div>
        </td>
        <td>
          <div style="float: left;">alpha</div>
          <div style="float: right;">0.766%</div>
        </td>
        <td>
          <div style="float: left;">aarch64</div>
          <div style="float: right;">5.679%</div>
        </td>
      </tr>
      <tr>
        <td>
          <div style="float: left;">ppc64</div>
          <div style="float: right;">2.453%</div>
        </td>
        <td>
          <div style="float: left;">arm</div>
          <div style="float: right;">0.012%</div>
        </td>
        <td>
          <div style="float: left;">m68k</div>
          <div style="float: right;">4.572%</div>
        </td>
      </tr>
      <tr>
        <td>
          <div style="float: left;">ppc64le</div>
          <div style="float: right;">2.456%</div>
        </td>
        <td>
          <div style="float: left;">hppa</div>
          <div style="float: right;">0.026%</div>
        </td>
        <td>
          <div style="float: left;">mips</div>
          <div style="float: right;">4.614%</div>
        </td>
      </tr>
      <tr>
        <td></td>
        <td>
          <div style="float: left;">s390x</div>
          <div style="float: right;">0.599%</div>
        </td>
        <td>
          <div style="float: left;">mipsel</div>
          <div style="float: right;">5.043%</div>
        </td>
      </tr>
      <tr>
        <td></td>
        <td>
          <div style="float: left;">sh4</div>
          <div style="float: right;">0.015%</div>
        </td>
        <td>
          <div style="float: left;">mips64</div>
          <div style="float: right;">4.651%</div>
        </td>
      </tr>
      <tr>
        <td></td>
        <td>
          <div style="float: left;">sparc64</div>
          <div style="float: right;">0.057%</div>
        </td>
        <td>
          <div style="float: left;">mips64el</div>
          <div style="float: right;">5.087%</div>
        </td>
      </tr>
      <tr>
        <td></td>
        <td></td>
        <td>
          <div style="float: left;">riscv64</div>
          <div style="float: right;">2.155%</div>
        </td>
      </tr>
      <tr>
        <td></td>
        <td></td>
        <td>
          <div style="float: left;">x86_64</div>
          <div style="float: right;">1.609%</div>
        </td>
      </tr>
    </tbody>
  </table>
  <p style="text-align:center; color: gray; font-style:italic;">Report 3: QEMU 5.0 and 5.1-pre-soft-freeze Dissect Comparison</p>
</div>

## Analyzing hppa Performance 5.0 VS 5.1-pre-soft-freeze

Finding list of hppa helpers for QEMU 5.0:

```bash
./list_helpers.py -- <qemu-build>/hppa-linux-user/qemu-hppa coulomb_double-hppa
```

Results:

<pre class="highlight" style="font-size:14.6205px">
<code>Total number of instructions: 12,005,480,751

Executed QEMU Helpers:

 No.     Instructions  Percentage            Calls    Ins/Call  Helper Name                Source File
----  ---------------  ----------  ---------------  ----------  -------------------------  ------------------------------
   1    3,699,940,930     30.819%       21,503,828         172  helper_fmpy_d              &lt;qemu&gt;/target/hppa/op_helper.c
   2    3,104,012,400     25.855%       18,478,099         167  helper_fadd_d              &lt;qemu&gt;/target/hppa/op_helper.c
   3    2,649,102,324     22.066%       16,946,808         156  helper_fsub_d              &lt;qemu&gt;/target/hppa/op_helper.c
   <b>4      620,535,451      5.169%        7,578,184          81  helper_fcmp_d              &lt;qemu&gt;/target/hppa/op_helper.c</b>
   5      441,738,119      3.679%        7,605,380          58  helper_lookup_tb_ptr       &lt;qemu&gt;/accel/tcg/tcg-runtime.c
   6      195,195,906      1.626%        1,262,976         154  helper_fdiv_d              &lt;qemu&gt;/target/hppa/op_helper.c
   7       82,688,760      0.689%          459,382         180  helper_fcnv_t_d_w          &lt;qemu&gt;/target/hppa/op_helper.c
   8       15,120,000      0.126%        1,008,000          15  helper_loaded_fr0          &lt;qemu&gt;/target/hppa/op_helper.c
   9          604,725      0.005%            8,063          75  helper_excp                &lt;qemu&gt;/target/hppa/op_helper.c
  10          308,000      0.003%            2,000         154  helper_fcnv_w_d            &lt;qemu&gt;/target/hppa/op_helper.c
  11           71,424      0.001%           23,808           3  helper_tcond               &lt;qemu&gt;/target/hppa/op_helper.c
  </code>
</pre>

Finding list of hppa helpers for QEMU 5.1-pre-soft-freeze:

```bash
./list_helpers.py -- <qemu-master-build>/hppa-linux-user/qemu-hppa coulomb_double-hppa
```

Results:

<pre class="highlight" style="font-size:14.6205px">
<code>Total number of instructions: 12,008,544,996

Executed QEMU Helpers:

 No.     Instructions  Percentage            Calls    Ins/Call  Helper Name                Source File
----  ---------------  ----------  ---------------  ----------  -------------------------  ------------------------------
   1    3,699,940,930     30.811%       21,503,828         172  helper_fmpy_d              &lt;qemu-master&gt;/qemu/target/hppa/op_helper.c
   2    3,104,012,400     25.848%       18,478,099         167  helper_fadd_d              &lt;qemu-master&gt;/qemu/target/hppa/op_helper.c
   3    2,649,102,324     22.060%       16,946,808         156  helper_fsub_d              &lt;qemu-master&gt;/qemu/target/hppa/op_helper.c
   <b>4      620,535,451      5.167%        7,578,184          81  helper_fcmp_d              &lt;qemu-master&gt;/qemu/target/hppa/op_helper.c</b>
   5      441,738,119      3.679%        7,605,380          58  helper_lookup_tb_ptr       &lt;qemu-master&gt;/qemu/accel/tcg/tcg-runtime.c
   6      195,195,906      1.625%        1,262,976         154  helper_fdiv_d              &lt;qemu-master&gt;/qemu/target/hppa/op_helper.c
   7       83,607,524      0.696%          459,382         182  helper_fcnv_t_d_w          &lt;qemu-master&gt;/qemu/target/hppa/op_helper.c
   8       15,120,000      0.126%        1,008,000          15  helper_loaded_fr0          &lt;qemu-master&gt;/qemu/target/hppa/op_helper.c
   9          604,725      0.005%            8,063          75  helper_excp                &lt;qemu-master&gt;/qemu/target/hppa/op_helper.c
  10          294,000      0.002%            2,000         147  helper_fcnv_w_d            &lt;qemu-master&gt;/qemu/target/hppa/op_helper.c
  11           71,424      0.001%           23,808           3  helper_tcond               &lt;qemu-master&gt;/qemu/target/hppa/op_helper.c
  </code>
</pre>

Let's focus on the floating point comparison helper, `helper_fcmp_d`. It appears that its performance is totally unaffected by softfloat changes, as opposed to ppc/ppc64/ppc64le helper. Now, the `list_fn_callees.py` script can be used to list `helper_fcmp_d` callees for QEMU 5.0:

<pre class="highlight" style="font-size:14.6205px">
<code>./list_fn_callees.py <span style="color: #000080">-f</span> <b>helper_fcmp_d</b> -- &lt;qemu-build&gt;/hppa-linux-user/qemu-hppa coulomb_double-hppa</code>
</pre>

Results:

<pre class="highlight" style="font-size:14.6205px">
<code>Total number of instructions: 12,005,490,004

Callees of helper_fcmp_d:

 No.     Instructions  Percentage            Calls    Ins/Call  Function Name              Source File
----  ---------------  ----------  ---------------  ----------  -------------------------  ------------------------------
   1      234,512,818      1.953%        7,578,184          30  update_fr0_cmp.isra.6      &lt;qemu&gt;/target/hppa/op_helper.c
   <b>2       64,894,460      0.541%        4,514,339          14  float64_compare            &lt;qemu&gt;/fpu/softfloat.c</b>
   3       53,047,288      0.442%        7,578,184           7  update_fr0_op              &lt;qemu&gt;/target/hppa/op_helper.c
   <b>4       43,799,210      0.365%        3,063,845          14  float64_compare_quiet      &lt;qemu&gt;/fpu/softfloat.c</b>
  </code>
</pre>

Further, the same can be done for QEMU 5.1-pre-soft-freeze:

<pre class="highlight" style="font-size:14.6205px">
<code>./list_fn_callees.py <span style="color: #000080">-f</span> <b>helper_fcmp_d</b> -- &lt;qemu-master-build&gt;/hppa-linux-user/qemu-hppa coulomb_double-hppa</code>
</pre>

Results:

<pre class="highlight" style="font-size:14.6205px">
<code>Total number of instructions: 12,008,552,050

Callees of helper_fcmp_d:

 No.     Instructions  Percentage            Calls    Ins/Call  Function Name              Source File
----  ---------------  ----------  ---------------  ----------  -------------------------  ------------------------------
   1      234,512,818      1.953%        7,578,184          30  update_fr0_cmp.isra.6      &lt;qemu-master&gt;/qemu/target/hppa/op_helper.c
   <b>2       64,894,460      0.540%        4,514,339          14  float64_compare            &lt;qemu-master&gt;/qemu/fpu/softfloat.c</b>
   3       53,047,288      0.442%        7,578,184           7  update_fr0_op              &lt;qemu-master&gt;/qemu/target/hppa/op_helper.c
   <b>4       43,799,210      0.365%        3,063,845          14  float64_compare_quiet      &lt;qemu-master&gt;/qemu/fpu/softfloat.c</b>
  </code>
</pre>

From the results, it can be concluded that hppa performance remained the same since its float comparison helper `helper_fcmp_d` already used the standard `float64_compare` and `float64_compare_quiet` softfloat functions, unlike ppc/ppc64/ppc64le targets, that used the more specialized float64 compare specializations.

### Analyzing sh4 Performance 5.0 VS 5.1-pre-soft-freeze

Let's now similarly compare sh4 performance in QEMU 5.0 and QEMU 5.1-pre-soft-freeze.

Finding list of sh4 helpers for QEMU 5.0:

```bash
./list_helpers.py -- <qemu-build>/sh4-linux-user/qemu-sh4 coulomb_double-sh4
```

Results:

<pre class="highlight" style="font-size:14.6205px">
<code>Total number of instructions: 12,728,140,143

Executed QEMU Helpers:

 No.     Instructions  Percentage            Calls    Ins/Call  Helper Name                Source File
----  ---------------  ----------  ---------------  ----------  -------------------------  ------------------------------
   1    3,423,220,999     26.895%       21,503,828         159  helper_fmul_DT             &lt;qemu&gt;/target/sh4/op_helper.c
   2    3,059,134,203     24.034%       18,478,099         165  helper_fadd_DT             &lt;qemu&gt;/target/sh4/op_helper.c
   3    2,635,892,905     20.709%       16,946,808         155  helper_fsub_DT             &lt;qemu&gt;/target/sh4/op_helper.c
   4      561,171,205      4.409%        7,873,279          71  helper_lookup_tb_ptr       &lt;qemu&gt;/accel/tcg/tcg-runtime.c
   <b>5      192,918,908      1.516%        4,642,299          41  helper_fcmp_gt_DT          &lt;qemu&gt;/target/sh4/op_helper.c</b>
   6      188,881,026      1.484%        1,262,976         149  helper_fdiv_DT             &lt;qemu&gt;/target/sh4/op_helper.c
   <b>7      113,909,493      0.895%        2,760,445          41  helper_fcmp_eq_DT          &lt;qemu&gt;/target/sh4/op_helper.c</b>
   8       83,148,142      0.653%          459,382         181  helper_ftrc_DT             &lt;qemu&gt;/target/sh4/op_helper.c
   9          310,000      0.002%            2,000         155  helper_float_DT            &lt;qemu&gt;/target/sh4/op_helper.c
  10          143,000      0.001%           11,000          13  helper_ld_fpscr            &lt;qemu&gt;/target/sh4/op_helper.c
  11            3,528      0.000%               42          84  helper_trapa               &lt;qemu&gt;/target/sh4/op_helper.c
  </code>
</pre>

Finding list of sh4 helpers for QEMU 5.1-pre-soft-freeze:

```bash
./list_helpers.py -- <qemu-master-build>/sh4-linux-user/qemu-sh4 coulomb_double-sh4
```

Results:

<pre class="highlight" style="font-size:14.6205px">
<code>Total number of instructions: 12,730,023,780

Executed QEMU Helpers:

 No.     Instructions  Percentage            Calls    Ins/Call  Helper Name                Source File
----  ---------------  ----------  ---------------  ----------  -------------------------  ------------------------------
   1    3,423,220,999     26.891%       21,503,828         159  helper_fmul_DT             &lt;qemu-master&gt;/qemu/target/sh4/op_helper.c
   2    3,059,134,203     24.031%       18,478,099         165  helper_fadd_DT             &lt;qemu-master&gt;/qemu/target/sh4/op_helper.c
   3    2,635,892,905     20.706%       16,946,808         155  helper_fsub_DT             &lt;qemu-master&gt;/qemu/target/sh4/op_helper.c
   4      561,171,205      4.408%        7,873,279          71  helper_lookup_tb_ptr       &lt;qemu-master&gt;/qemu/accel/tcg/tcg-runtime.c
   <b>5      192,918,908      1.515%        4,642,299          41  helper_fcmp_gt_DT          &lt;qemu-master&gt;/qemu/target/sh4/op_helper.c</b>
   6      188,881,026      1.484%        1,262,976         149  helper_fdiv_DT             &lt;qemu-master&gt;/qemu/target/sh4/op_helper.c
   <b>7      113,909,493      0.895%        2,760,445          41  helper_fcmp_eq_DT          &lt;qemu-master&gt;/qemu/target/sh4/op_helper.c</b>
   8       84,066,906      0.660%          459,382         183  helper_ftrc_DT             &lt;qemu-master&gt;/qemu/target/sh4/op_helper.c
   9          296,000      0.002%            2,000         148  helper_float_DT            &lt;qemu-master&gt;/qemu/target/sh4/op_helper.c
  10          143,000      0.001%           11,000          13  helper_ld_fpscr            &lt;qemu-master&gt;/qemu/target/sh4/op_helper.c
  11            3,528      0.000%               42          84  helper_trapa               &lt;qemu-master&gt;/qemu/target/sh4/op_helper.c
  </code>
</pre>

Now, one can spot that helpers `helper_fcmp_gt_DT` and `helper_fcmp_eq_DT` - related to floating point number comparisons - didn't have any change in their number of instructions per call. To futher inspect the reason behind this, let's use the `list_fn_callees.py` script to find their callees.

For QEMU 5.0:

<pre class="highlight" style="font-size:14.6205px">
<code>./list_fn_callees.py <span style="color: #000080">-f</span> <b>helper_fcmp_gt_DT helper_fcmp_eq_DT</b> -- &lt;qemu-build&gt;/sh4-linux-user/qemu-sh4 coulomb_double-sh4</code>
</pre>

Results:

<pre class="highlight" style="font-size:14.6205px">
<code>Total number of instructions: 12,728,140,143

Callees of helper_fcmp_gt_DT:

 No.     Instructions  Percentage            Calls    Ins/Call  Function Name              Source File
----  ---------------  ----------  ---------------  ----------  -------------------------  ------------------------------
   <b>1       67,576,835      0.531%        4,642,299          14  float64_compare            &lt;qemu&gt;/fpu/softfloat.c</b>
   2       32,496,093      0.255%        4,642,299           7  update_fpscr               &lt;qemu&gt;/target/sh4/op_helper.c


Callees of helper_fcmp_eq_DT:

 No.     Instructions  Percentage            Calls    Ins/Call  Function Name              Source File
----  ---------------  ----------  ---------------  ----------  -------------------------  ------------------------------
   <b>1       39,377,478      0.309%        2,760,445          14  float64_compare            &lt;qemu&gt;/fpu/softfloat.c</b>
   2       19,323,115      0.152%        2,760,445           7  update_fpscr               &lt;qemu&gt;/target/sh4/op_helper.c
  </code>
</pre>

For QEMU 5.1-pre-soft-freeze:

<pre class="highlight" style="font-size:14.6205px">
<code>./list_fn_callees.py <span style="color: #000080">-f</span> <b>helper_fcmp_gt_DT helper_fcmp_eq_DT</b> -- &lt;qemu-master-build&gt;/sh4-linux-user/qemu-sh4 coulomb_double-sh4</code>
</pre>

Results:

<pre class="highlight" style="font-size:14.6205px">
<code>Total number of instructions: 12,730,023,815

Callees of helper_fcmp_gt_DT:

 No.     Instructions  Percentage            Calls    Ins/Call  Function Name              Source File
----  ---------------  ----------  ---------------  ----------  -------------------------  ------------------------------
   <b>1       67,576,835      0.531%        4,642,299          14  float64_compare            &lt;qemu-master&gt;/qemu/fpu/softfloat.c</b>
   2       32,496,093      0.255%        4,642,299           7  update_fpscr               &lt;qemu-master&gt;/qemu/target/sh4/op_helper.c


Callees of helper_fcmp_eq_DT:

 No.     Instructions  Percentage            Calls    Ins/Call  Function Name              Source File
----  ---------------  ----------  ---------------  ----------  -------------------------  ------------------------------
   <b>1       39,377,478      0.309%        2,760,445          14  float64_compare            &lt;qemu-master&gt;/qemu/fpu/softfloat.c</b>
   2       19,323,115      0.152%        2,760,445           7  update_fpscr               &lt;qemu-master&gt;/qemu/target/sh4/op_helper.c
  </code>
</pre>

From the results, it can be seen that for the same reason as hppa, the performance of sh4 remained the same despite the changes made in the softfloat implementation.

### Analyzing mips Performance 5.0 VS 5.1-pre-soft-freeze

Finding list of mips helpers for QEMU 5.0:

```bash
./list_helpers.py -- <qemu-build>/mips-linux-user/qemu-mips coulomb_double-mips
```

Results:

```
Total number of instructions: 10,438,170,014

Executed QEMU Helpers:

 No.     Instructions  Percentage            Calls    Ins/Call  Helper Name                 Source File
----  ---------------  ----------  ---------------  ----------  -------------------------   ------------------------------
   1    2,208,051,015     21.154%       14,936,914         147  helper_float_sub_d          <qemu>/target/mips/fpu_helper.c
   2    2,061,214,236     19.747%       11,626,196         177  helper_float_mul_d          <qemu>/target/mips/fpu_helper.c
   3    1,884,622,177     18.055%        7,860,734         239  helper_float_madd_d         <qemu>/target/mips/fpu_helper.c
   4    1,727,127,172     16.546%       10,609,351         162  helper_float_add_d          <qemu>/target/mips/fpu_helper.c
   5      468,482,823      4.488%        2,003,882         233  helper_float_msub_d         <qemu>/target/mips/fpu_helper.c
   6      249,775,460      2.393%        4,218,047          59  helper_cmp_d_lt             <qemu>/target/mips/fpu_helper.c
   7      207,577,312      1.989%        1,261,468         164  helper_float_div_d          <qemu>/target/mips/fpu_helper.c
   8      156,426,380      1.499%        3,055,498          51  helper_cmp_d_eq             <qemu>/target/mips/fpu_helper.c
   9      113,177,370      1.084%        2,169,025          52  helper_lookup_tb_ptr        <qemu>/accel/tcg/tcg-runtime.c
  10       80,391,850      0.770%          459,382         175  helper_float_trunc_w_d      <qemu>/target/mips/fpu_helper.c
  11       24,287,676      0.233%          419,248          57  helper_cmp_d_le             <qemu>/target/mips/fpu_helper.c
  12        4,016,000      0.038%        1,004,000           4  helper_cfc1                 <qemu>/target/mips/fpu_helper.c
  13          718,350      0.007%            1,000         718  helper_float_sqrt_d         <qemu>/target/mips/fpu_helper.c
  14          372,000      0.004%            4,000          93  helper_cmp_d_ule            <qemu>/target/mips/fpu_helper.c
  15          290,000      0.003%            2,000         145  helper_float_cvtd_w         <qemu>/target/mips/fpu_helper.c
  16          257,384      0.002%            2,962          86  helper_swl                  <qemu>/target/mips/op_helper.c
  17          176,000      0.002%            4,000          44  helper_cmp_d_un             <qemu>/target/mips/fpu_helper.c
  18           90,000      0.001%            1,000          90  helper_cmp_d_ult            <qemu>/target/mips/fpu_helper.c
  19            4,171      0.000%               43          97  helper_raise_exception_err  <qemu>/target/mips/op_helper.c
```

Finding list of mips helpers for QEMU 5.1-pre-soft-freeze:

```bash
./list_callees.py -- <qemu-master-build>/mips-linux-user/qemu-mips coulomb_double-mips
```

Results:

<pre class="highlight" style="font-size:14.6205px">
<code>Total number of instructions: 9,956,439,111

Executed QEMU Helpers:

 No.     Instructions  Percentage            Calls    Ins/Call  Helper Name                Source File
----  ---------------  ----------  ---------------  ----------  -------------------------  ------------------------------
   <b>1    2,122,805,507     21.321%       14,936,914         142  helper_float_sub_d         &lt;qemu-master&gt;/target/mips/fpu_helper.c</b>
   <b>2    1,943,156,724     19.517%       11,626,196         167  helper_float_mul_d         &lt;qemu-master&gt;/target/mips/fpu_helper.c</b>
   <b>3    1,792,488,279     18.003%        7,860,734         228  helper_float_madd_d        &lt;qemu-master&gt;/target/mips/fpu_helper.c</b>
   4    1,632,785,630     16.399%       10,609,351         153  helper_float_add_d         &lt;qemu-master&gt;/target/mips/fpu_helper.c
   5      444,449,149      4.464%        2,003,882         221  helper_float_msub_d        &lt;qemu-master&gt;/target/mips/fpu_helper.c
   6      207,914,606      2.088%        4,218,047          49  helper_cmp_d_lt            &lt;qemu-master&gt;/target/mips/fpu_helper.c
   7      192,439,696      1.933%        1,261,468         152  helper_float_div_d         &lt;qemu-master&gt;/target/mips/fpu_helper.c
   8      147,927,209      1.486%        3,055,498          48  helper_cmp_d_eq            &lt;qemu-master&gt;/target/mips/fpu_helper.c
   9      113,177,370      1.137%        2,169,025          52  helper_lookup_tb_ptr       &lt;qemu-master&gt;/accel/tcg/tcg-runtime.c
  10       80,391,850      0.807%          459,382         175  helper_float_trunc_w_d     &lt;qemu-master&gt;/target/mips/fpu_helper.c
  11       20,962,400      0.211%          419,248          50  helper_cmp_d_le            &lt;qemu-master&gt;/target/mips/fpu_helper.c
  12        4,016,000      0.040%        1,004,000           4  helper_cfc1                &lt;qemu-master&gt;/target/mips/fpu_helper.c
  13          706,350      0.007%            1,000         706  helper_float_sqrt_d        &lt;qemu-master&gt;/target/mips/fpu_helper.c
  14          300,000      0.003%            4,000          75  helper_cmp_d_ule           &lt;qemu-master&gt;/target/mips/fpu_helper.c
  15          272,000      0.003%            2,000         136  helper_float_cvtd_w        &lt;qemu-master&gt;/target/mips/fpu_helper.c
  16          257,384      0.003%            2,962          86  helper_swl                 &lt;qemu-master&gt;/target/mips/op_helper.c
  17          180,000      0.002%            4,000          45  helper_cmp_d_un            &lt;qemu-master&gt;/target/mips/fpu_helper.c
  18           72,000      0.001%            1,000          72  helper_cmp_d_ult           &lt;qemu-master&gt;/target/mips/fpu_helper.c
  19            4,171      0.000%               43          97  helper_raise_exception_err  &lt;qemu-master&gt;/target/mips/op_helper.c
  </code>
</pre>

Looking at the results, 14 out of the 19 helpers had a deacrease in their number of instructions per call. This implies that the change added to the source code in QEMU 5.1-pre-soft-freeze reflected in all of these helpers. To pin point this change, the `list_fn_callees.py` script will be used to display the callees of the top three helpers.

Finding the callees for QEMU 5.0:

<pre class="highlight" style="font-size:14.6205px">
<code>./list_fn_callees.py <span style="color: #000080">-f</span> <b>helper_float_sub_d helper_float_mul_d helper_float_madd_d</b> -- &lt;qemu-build&gt;/mips-linux-user/qemu-mips coulomb_double-mips</code>
</pre>

Results:

<pre class="highlight" style="font-size:14.6205px">
<code>Total number of instructions: 10,438,169,918

Callees of helper_float_sub_d:

 No.     Instructions  Percentage            Calls    Ins/Call  Function Name              Source File
----  ---------------  ----------  ---------------  ----------  -------------------------  ------------------------------
   1    1,756,973,417     16.832%       14,936,914         117  float64_sub                &lt;qemu&gt;/fpu/softfloat.c
   <b>2      116,280,528      1.114%        5,537,168          21  ieee_ex_to_mips.part.2     &lt;qemu&gt;/target/mips/fpu_helper.c</b>


Callees of helper_float_mul_d:

 No.     Instructions  Percentage            Calls    Ins/Call  Function Name              Source File
----  ---------------  ----------  ---------------  ----------  -------------------------  ------------------------------
   1    1,498,120,304     14.352%       11,626,196         128  float64_mul                &lt;qemu&gt;/fpu/softfloat.c
   <b>2      199,090,752      1.907%        9,480,512          21  ieee_ex_to_mips.part.2     &lt;qemu&gt;/target/mips/fpu_helper.c</b>


Callees of helper_float_madd_d:

 No.     Instructions  Percentage            Calls    Ins/Call  Function Name              Source File
----  ---------------  ----------  ---------------  ----------  -------------------------  ------------------------------
   1    1,009,704,817      9.673%        7,860,734         128  float64_mul                &lt;qemu&gt;/fpu/softfloat.c
   2      365,108,047      3.498%        7,860,734          46  float64_add                &lt;qemu&gt;/fpu/softfloat.c
   <b>3      160,466,103      1.537%        7,641,243          21  ieee_ex_to_mips.part.2     &lt;qemu&gt;/target/mips/fpu_helper.c</b>
  </code>
</pre>

Finding the callees for QEMU 5.1-pre-soft-freeze:

<pre class="highlight" style="font-size:14.6205px">
<code>./list_fn_callees.py <span style="color: #000080">-f</span> <b>helper_float_sub_d helper_float_mul_d helper_float_madd_d</b> -- &lt;qemu-master-build&gt;/mips-linux-user/qemu-mips coulomb_double-mips</code>
</pre>

Results:

```
Total number of instructions: 9,956,439,118

Callees of helper_float_sub_d:

 No.     Instructions  Percentage            Calls    Ins/Call  Function Name              Source File
----  ---------------  ----------  ---------------  ----------  -------------------------  ------------------------------
   1    1,756,973,417     17.647%       14,936,914         117  float64_sub                <qemu-master>/fpu/softfloat.c


Callees of helper_float_mul_d:

 No.     Instructions  Percentage            Calls    Ins/Call  Function Name              Source File
----  ---------------  ----------  ---------------  ----------  -------------------------  ------------------------------
   1    1,498,120,304     15.047%       11,626,196         128  float64_mul                <qemu-master>/fpu/softfloat.c


Callees of helper_float_madd_d:

 No.     Instructions  Percentage            Calls    Ins/Call  Function Name              Source File
----  ---------------  ----------  ---------------  ----------  -------------------------  ------------------------------
   1    1,009,704,817     10.141%        7,860,734         128  float64_mul                <qemu-master>/fpu/softfloat.c
   2      365,108,047      3.667%        7,860,734          46  float64_add                <qemu-master>/fpu/softfloat.c
```

From the results, it's seen that the function `ieee_ex_to_mips.part.2` dissapeard from the list of all three helper callees. Comparing the source code of both QEMU versions shows that the reason behind this is the inlining of the `ieee_ex_to_mips` function and renaming it to `ieee_to_mips_xcpt` in QEMU 5.1-pre-soft-freeze.

This concludes the four analysis sections in the report. ppc, hppa, sh4, and mips were selected as examples for the analysis because they cover all cases of performance change. The next logical step would be to find the commit that updated the implementation of softfloat - causing PowerPC performance degradation - as well as the commit that inlined `ieee_ex_to_mips` which caused the mips performance improvement.

This is the idea of next week's report which will introduce a new method for automtically locating commits that introduce performance improvements or degradations in QEMU.

<hr />

## Appendix

### Float Comparison Helper of ppc

```c
void helper_fcmpu(CPUPPCState *env, uint64_t arg1, uint64_t arg2,
                  uint32_t crfD)
{
    CPU_DoubleU farg1, farg2;
    uint32_t ret = 0;

    farg1.ll = arg1;
    farg2.ll = arg2;

    if (unlikely(float64_is_any_nan(farg1.d) ||
                 float64_is_any_nan(farg2.d))) {
        ret = 0x01UL;
    } else if (float64_lt(farg1.d, farg2.d, &env->fp_status)) {
        ret = 0x08UL;
    } else if (!float64_le(farg1.d, farg2.d, &env->fp_status)) {
        ret = 0x04UL;
    } else {
        ret = 0x02UL;
    }

    env->fpscr &= ~FP_FPCC;
    env->fpscr |= ret << FPSCR_FPCC;
    env->crf[crfD] = ret;
    if (unlikely(ret == 0x01UL
                 && (float64_is_signaling_nan(farg1.d, &env->fp_status) ||
                     float64_is_signaling_nan(farg2.d, &env->fp_status)))) {
        /* sNaN comparison */
        float_invalid_op_vxsnan(env, GETPC());
    }
}
```

### Float Comparison Helper of hppa

```c
void HELPER(fcmp_d)(CPUHPPAState *env, float64 a, float64 b,
                    uint32_t y, uint32_t c)
{
    FloatRelation r;
    if (c & 1) {
        r = float64_compare(a, b, &env->fp_status);
    } else {
        r = float64_compare_quiet(a, b, &env->fp_status);
    }
    update_fr0_op(env, GETPC());
    update_fr0_cmp(env, y, c, r);
}
```

### Float Comparison Helpers of sh4

```c
uint32_t helper_fcmp_eq_DT(CPUSH4State *env, float64 t0, float64 t1)
{
    int relation;

    set_float_exception_flags(0, &env->fp_status);
    relation = float64_compare(t0, t1, &env->fp_status);
    update_fpscr(env, GETPC());
    return relation == float_relation_equal;
}

uint32_t helper_fcmp_gt_DT(CPUSH4State *env, float64 t0, float64 t1)
{
    int relation;

    set_float_exception_flags(0, &env->fp_status);
    relation = float64_compare(t0, t1, &env->fp_status);
    update_fpscr(env, GETPC());
    return relation == float_relation_greater;
}
```

### Float Operation Helpers of mips

```c
uint64_t helper_float_ ## name ## _d(CPUMIPSState *env,            \
                                     uint64_t fdt0, uint64_t fdt1) \
{                                                                  \
    uint64_t dt2;                                                  \
                                                                   \
    dt2 = float64_ ## name(fdt0, fdt1, &env->active_fpu.fp_status);\
    update_fcr31(env, GETPC());                                    \
    return dt2;                                                    \
}

uint64_t helper_float_madd_d(CPUOpenRISCState *env, uint64_t a,
                             uint64_t b, uint64_t c)
{
    /* Note that or1ksim doesn't use fused operation.  */
    b = float64_mul(b, c, &env->fp_status);
    return float64_add(a, b, &env->fp_status);
}
```

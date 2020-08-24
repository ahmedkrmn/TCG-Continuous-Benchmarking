---
layout: post
title: Measuring QEMU Performance in System Mode
tags: ['System Mode', 'Perf', 'Top N', 'Basic Metrics']
subtitle: Comparing the Emulation Performance of Debian for Five Different Targets
excerpt: The final report of the TCG Continuos Benchmarking project introduces basic performance measurements for QEMU system mode emulation. Boot-up time and number of executed instructions are compared for the emulation of five different targets. The report also presents a new tool for finding the topN most executed functions in the emulation process.
---

## Intro

The final report of the TCG Continuos Benchmarking project introduces basic performance measurements for QEMU system mode emulation. Boot-up time and number of executed instructions are compared for the emulation of five different targets. The report also presents a new tool for finding the topN most executed functions in the emulation process.

## Table of Contents

- [Setup](#setup)
- [Starting System Emulation](#starting-system-emulation)
- [Boot-up Time](#boot-up-time)
- [The 25 Most Executed Functions](#the-25-most-executed-functions)
- [Conclusion](#conclusion)

## Setup

First, create a QEMU system build based on the latest version 5.1.0. For the purpose of this report, you'll only need to build for the five used targets.

```bash
wget https://download.qemu.org/qemu-5.1.0.tar.xz
tar xfv qemu-5.1.0.tar.xz
cd qemu-5.1.0
mkdir build-gcc-system
cd build-gcc-system
../configure --target-list=aarch64-softmmu,arm-softmmu,mips-softmmu,mipsel-softmmu,x86_64-softmmu
make
```

## Starting System Emulation

Debian provides support for a variety of architectures, that's why it's the OS of choice for testing the system mode emulation. The latest Debian version 15.0 is used.

For each of the five targets (aarch64, arm, mips, mipsel, and x86_64), the Debian image is booted up until the setup menu appears, then the emulation is manually stopped. Doing so assures that enough instructions are executed by QEMU for accurately comparing the results while at the same time avoids the unnecessary lengthy process of actually installing the OS.

Having said that, instead of downloading the .iso image of Debian which is around 350 MB for each target, it's sufficient to use the initial ramdisk and kernel files from the netboot version of the Debian distribution.

The below snippets download the required files from the Debian archives then starts the emulation for each target. All emulations are performed with a RAM size of 1024 MB.

**AArch64**:

```bash
# Initial ramdisk
wget http://ftp.nl.debian.org/debian/dists/buster/main/installer-arm64/current/images/cdrom/initrd.gz
# Linux kernel
wget http://ftp.nl.debian.org/debian/dists/buster/main/installer-arm64/current/images/cdrom/vmlinuz
# Emulate Debian
<qemu-system-build>/aarch64-softmmu/qemu-system-aarch64 \
-m 1024 -M virt -cpu cortex-a57 -kernel vmlinuz -initrd initrd.gz \
-append "root=/dev/ram"
```

**ARM**:

```bash
# Initial ramdisk
wget http://ftp.nl.debian.org/debian/dists/buster/main/installer-armhf/current/images/cdrom/initrd.gz
# Linux kernel
wget http://ftp.nl.debian.org/debian/dists/buster/main/installer-armhf/current/images/cdrom/vmlinuz
# Emulate Debian
<qemu-system-build>/arm-softmmu/qemu-system-arm \
-m 1024 -M virt -cpu cortex-a15 -kernel vmlinuz -initrd initrd.gz \
-append "root=/dev/ram"
```

**MIPS (Malta)**:

```bash
# Initial ramdisk
wget http://ftp.nl.debian.org/debian/dists/buster/main/installer-mips/current/images/malta/netboot/initrd.gz
# Linux kernel
wget http://ftp.nl.debian.org/debian/dists/buster/main/installer-mips/current/images/malta/netboot/vmlinux-4.19.0-10-4kc-malta
# Emulate Debian
<qemu-system-build>/mips-softmmu/qemu-system-mips \
-m 1024 -M Malta -kernel vmlinux-4.19.0-10-4kc-malta -initrd initrd.gz \
-append "root=/dev/ram"
```

**MIPSel (Malta)**:

```bash
# Initial ramdisk
wget http://ftp.nl.debian.org/debian/dists/buster/main/installer-mipsel/current/images/malta/netboot/initrd.gz
# Linux kernel
wget http://ftp.nl.debian.org/debian/dists/buster/main/installer-mipsel/current/images/malta/netboot/vmlinux-4.19.0-10-4kc-malta
# Emulate Debian
<qemu-system-build>/mipsel-softmmu/qemu-system-mipsel \
-m 1024 -M Malta -kernel vmlinux-4.19.0-10-4kc-malta -initrd initrd.gz \
-append "root=/dev/ram"
```

**x86_64**:

```bash
# Initial ramdisk
wget http://ftp.nl.debian.org/debian/dists/buster/main/installer-amd64/current/images/cdrom/initrd.gz
# Linux kernel
wget http://ftp.nl.debian.org/debian/dists/buster/main/installer-amd64/current/images/cdrom/vmlinuz
# Emulate Debian
<qemu-system-build>/x86_64-softmmu/qemu-system-x86_64 \
-m 1024 -kernel vmlinuz -initrd initrd.gz \
-append "root=/dev/ram"
```

**The demo below shows the successful emulation of the aarch64 target. All other targets behave the same way except the x86_64 in which the emulation output is shown on "VGA" instead of "serial0" like the other targets.**

![System Flow]({{ site.baseurl }}/images/system_mode/aarch64.gif)

## Boot-up Time

The Linux `time` command is used for measuring the boot-up time for the emulation of each target. The emulation is manually shutdown as soon as the setup menu appears.

<style>
  /* Right align all table fields */
  table.results td:nth-of-type(n+2) {
    text-align: right;
  }
  table.results th:nth-of-type(n+2) {
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
<div style="overflow-x: auto;">
  <table
    class="results"
    style="width: 83%; margin-right: auto; margin-left: auto;"
  >
    <thead>
      <tr>
        <th align="right">Target</th>
        <th>Boot-up Time</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td style="color: #4dac26;">16.76</td>
      </tr>
      <tr>
        <td>arm</td>
        <td style="color: #d7191c;">27.84</td>
      </tr>
      <tr>
        <td>mips</td>
        <td >20.76</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td >18.80</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td >17.97</td>
      </tr>
    </tbody>
  </table>
</div>

## The 25 Most Executed Functions

To find the top most executed QEMU functions as well as the total number of instructions, a new script, `topN_system.py`, is introduced which is similar to the topN scripts introduced in the "[Measuring Basic Performance Metrics of QEMU](https://ahmedkrmn.github.io/TCG-Continuous-Benchmarking/Measuring-Basic-Performance-Metrics-of-QEMU/)" report.

The script is based on Perf. Callgrind cannot be used in this case, because unlike user mode emulation, the system mode emulation is more resource intensive and since Callgrind is based on instrumentation, it would take an enormous amount of time to complete the measurements.

The script, which is available on the project [GitHub page](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/tree/master/tools/topN_system), takes only one optional argument `-n` to specify the number of top functions to display. If it's not provided, the script defaults to 25.

Usage example for aarch64 is shown below:

```bash
./topN_system.py -- <qemu-system-build>/aarch64-softmmu/qemu-system-aarch64 \
-m 1024 -M virt -cpu cortex-a57 -kernel images_debian/aarch64/vmlinuz -initrd images_debian/aarch64/initrd.gz \
-append "root=/dev/ram"
```

The script is executed in a similar manner for each of the five targets.

**AArch64**:

```
Number of instructions: 136,660,616,915

 No.  Percentage  Name
----  ----------  ------------------------------
   1      10.25%  helper_lookup_tb_ptr
   2       6.69%  liveness_pass_1
   3       4.71%  get_phys_addr_lpae
   4       3.65%  tcg_gen_code
   5       2.73%  tlb_flush_by_mmuidx_async_work
   6       2.57%  tlb_set_page_with_attrs
   7       2.48%  tcg_optimize
   8       2.24%  address_space_translate_internal
   9       2.14%  address_space_ldq_le
  10       1.76%  object_dynamic_cast_assert
  11       1.67%  tb_htable_lookup
  12       1.50%  flatview_do_translate
  13       1.44%  tcg_out_opc.isra.13
  14       1.07%  cpu_get_tb_cpu_state
  15       0.91%  get_phys_addr
  16       0.87%  get_page_addr_code_hostp
  17       0.80%  aa64_va_parameters
  18       0.78%  flatview_translate
  19       0.75%  S1_ptw_translate
  20       0.74%  victim_tlb_hit
  21       0.62%  arm_cpu_tlb_fill
  22       0.62%  object_class_dynamic_cast_assert
  23       0.62%  qht_lookup_custom
  24       0.62%  init_ts_info
  25       0.60%  regime_el
```

**ARM**:

```
Number of instructions: 236,734,647,205

 No.  Percentage  Name
----  ----------  ------------------------------
   1       9.10%  tlb_flush_by_mmuidx_async_work
   2       6.99%  helper_lookup_tb_ptr
   3       3.29%  tlb_set_page_with_attrs
   4       2.86%  liveness_pass_1
   5       2.79%  cpu_get_tb_cpu_state
   6       2.77%  get_phys_addr
   7       2.25%  tcg_gen_code
   8       1.78%  tcg_optimize
   9       1.41%  tb_htable_lookup
  10       1.25%  victim_tlb_hit
  11       1.03%  tcg_out_opc.isra.13
  12       0.91%  address_space_translate_internal
  13       0.85%  get_page_addr_code_hostp
  14       0.83%  object_class_dynamic_cast_assert
  15       0.80%  full_le_ldul_mmu
  16       0.77%  flatview_do_translate
  17       0.77%  object_dynamic_cast_assert
  18       0.73%  address_space_ldl_le
  19       0.72%  helper_uadd8
  20       0.68%  arm_ldl_ptw
  21       0.68%  arm_cpu_tlb_fill
  22       0.67%  find_next_bit
  23       0.60%  qht_lookup_custom
  24       0.56%  cpu_physical_memory_get_dirty.constprop.23
  25       0.54%  init_ts_info
```

**MIPS**:

```
Number of instructions: 180,841,164,833

 No.  Percentage  Name
----  ----------  ------------------------------
   1      16.91%  helper_lookup_tb_ptr
   2       8.08%  liveness_pass_1
   3       2.70%  tcg_gen_code
   4       2.26%  r4k_map_address
   5       1.87%  cpu_exec
   6       1.47%  tcg_optimize
   7       1.30%  object_class_dynamic_cast_assert
   8       1.04%  tcg_out_opc.isra.13
   9       0.77%  address_space_translate_internal
  10       0.74%  access_with_adjusted_size
  11       0.74%  tlb_set_page_with_attrs
  12       0.73%  tlb_flush_page_by_mmuidx_async_0
  13       0.71%  mips_cpu_do_interrupt
  14       0.63%  tb_htable_lookup
  15       0.61%  flatview_do_translate
  16       0.59%  tb_jmp_cache_clear_page
  17       0.49%  flatview_access_valid
  18       0.42%  tcg_out_sib_offset
  19       0.39%  init_ts_info
  20       0.38%  helper_eret
  21       0.36%  memory_region_dispatch_write
  22       0.35%  get_page_addr_code_hostp
  23       0.32%  object_dynamic_cast_assert
  24       0.31%  full_be_ldul_mmu
  25       0.30%  flatview_translate
```

**MIPSel**:

```
Number of instructions: 160,755,845,442

 No.  Percentage  Name
----  ----------  ------------------------------
   1       8.95%  liveness_pass_1
   2       6.29%  helper_lookup_tb_ptr
   3       2.99%  tcg_gen_code
   4       2.65%  r4k_map_address
   5       1.89%  cpu_exec
   6       1.60%  tcg_optimize
   7       1.50%  object_class_dynamic_cast_assert
   8       1.02%  tcg_out_opc.isra.13
   9       0.89%  access_with_adjusted_size
  10       0.86%  tlb_flush_page_by_mmuidx_async_0
  11       0.86%  tb_htable_lookup
  12       0.83%  flatview_do_translate
  13       0.82%  address_space_translate_internal
  14       0.82%  mips_cpu_do_interrupt
  15       0.73%  tlb_set_page_with_attrs
  16       0.72%  flatview_access_valid
  17       0.70%  tb_jmp_cache_clear_page
  18       0.51%  victim_tlb_hit
  19       0.48%  helper_eret
  20       0.43%  get_page_addr_code_hostp
  21       0.43%  memory_region_dispatch_write
  22       0.42%  tcg_out_sib_offset
  23       0.40%  flatview_translate
  24       0.39%  object_dynamic_cast_assert
  25       0.38%  init_ts_info
```

**x86_64**:

```
Number of instructions: 150,991,381,071

 No.  Percentage  Name
----  ----------  ------------------------------
   1      11.30%  helper_lookup_tb_ptr
   2       7.01%  liveness_pass_1
   3       4.48%  tcg_gen_code
   4       3.41%  tcg_optimize
   5       1.84%  tcg_out_opc.isra.13
   6       1.78%  helper_pcmpeqb_xmm
   7       1.20%  object_dynamic_cast_assert
   8       1.00%  cpu_exec
   9       0.99%  tcg_temp_new_internal
  10       0.88%  tb_htable_lookup
  11       0.84%  object_class_dynamic_cast_assert
  12       0.81%  init_ts_info
  13       0.80%  tlb_set_page_with_attrs
  14       0.77%  victim_tlb_hit
  15       0.75%  tcg_out_sib_offset
  16       0.62%  tcg_op_alloc
  17       0.61%  helper_pmovmskb_xmm
  18       0.58%  disas_insn.isra.50
  19       0.56%  helper_pcmpgtb_xmm
  20       0.56%  address_space_ldq
  21       0.49%  address_space_translate_internal
  22       0.49%  x86_cpu_tlb_fill
  23       0.46%  tb_gen_code
  24       0.45%  tcg_out_modrm_sib_offset
  25       0.43%  flatview_do_translate
```

## Conclusion

The results from the topN script and the manual boot-up time measurement are combined in the table below. The results quiet resemble those from the previous reports that compared the performance in the user mode.

<div style="overflow-x: auto;">
  <table
    class="results"
    style="width: 83%; margin-right: auto; margin-left: auto;"
  >
    <thead>
      <tr>
        <th>Target</th>
        <th>Boot-up Time</th>
        <th>Instructions</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>aarch64</td>
        <td style="color: #4dac26;">16.76</td>
        <td style="color: #4dac26;">136 660 616 915</td>
      </tr>
      <tr>
        <td>arm</td>
        <td style="color: #d7191c;">27.84</td>
        <td style="color: #d7191c;">236 734 647 205</td>
      </tr>
      <tr>
        <td>mips</td>
        <td >20.76</td>
        <td>180 841 164 833</td>
      </tr>
      <tr>
        <td>mipsel</td>
        <td >18.80</td>
        <td>160 755 845 442</td>
      </tr>
      <tr>
        <td>x86_64</td>
        <td >17.97</td>
        <td>150 991 381 071</td>
      </tr>
    </tbody>
  </table>
</div>

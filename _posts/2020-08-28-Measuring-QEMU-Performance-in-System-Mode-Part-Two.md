---
layout: post
title: Measuring QEMU Performance in System Mode - Part Two
tags: ['System Mode', 'Perf', 'Top N', 'Basic Metrics']
subtitle: Inspecting QEMU System Emulation Performance for Five Different Targets
excerpt: In part two of the final TCG Continuos Benchmarking report, the same procedures introduced in part one are used for inspecting the performance of QEMU system mode emulation. The only difference is instead of emulating the same OS for all targets, different images where selected from a Qemu-devel thread and the official QEMU documentation.
---

## Intro

In part two of the final TCG Continuos Benchmarking report, the same procedures introduced in [part one](https://ahmedkrmn.github.io/TCG-Continuous-Benchmarking/Measuring-QEMU-Performance-in-System-Mode/) are used for inspecting the performance of QEMU system mode emulation. The only difference is instead of emulating the same OS for all targets, different images where selected from this [Qemu-devel thread](https://www.mail-archive.com/qemu-devel@nongnu.org/msg604682.html) and the official [QEMU documentation](https://www.qemu.org/docs/master/system/).

For each of the five targets used in this report (arm, hppa, m68k, mipsel, and sh4), the source of the used emulation instructions is mentioned. This is followed by a snippet for fetching the required files and starting the system emulation.

The results of running the `time` command are then displayed for measuring the boot-up time until the login screen is reached where the emulation is stopped. The top 25 executed QEMU functions are also measured using the `topN_system.py` script introduced in part one of the report.

## List of Targets

- [ARM](#arm)
- [HPPA](#hppa)
- [M68k](#m68k)
- [MIPSel](#mipsel)
- [SH4](#sh4)

## ARM

**Source:**
[https://www.mail-archive.com/qemu-devel@nongnu.org/msg604838.html](https://www.mail-archive.com/qemu-devel@nongnu.org/msg604838.html)

**Emulation Setup:**

```bash
# Download spi.bin
wget https://github.com/Subbaraya-Sundeep/qemu-test-binaries/raw/master/spi.bin
# Download u-boot.bin
wget https://github.com/Subbaraya-Sundeep/qemu-test-binaries/raw/master/u-boot.bin
# Start system emulation
<qemu-system-build>/arm-softmmu/qemu-system-arm -M emcraft-sf2 -serial mon:stdio \
-kernel u-boot.bin -drive file=spi.bin,if=mtd,format=raw
```

**Emulation Demo:**

![ARM]({{ site.baseurl }}/images/system_mode/arm.gif)

**Boot-up Time:**

```
real	0m8.853s
user	0m7.543s
sys	0m0.156s
```

**Top 25 Functions:**

```
Number of instructions: 72,939,727,121

 No.  Percentage  Name
----  ----------  ------------------------------
   1       7.57%  access_with_adjusted_size
   2       6.79%  address_space_translate_internal
   3       4.46%  flatview_do_translate
   4       4.12%  memory_region_dispatch_read
   5       4.09%  flatview_access_valid
   6       3.78%  memory_region_read_with_attrs_accessor
   7       3.56%  flatview_read_continue
   8       3.09%  full_le_ldul_mmu
   9       2.90%  object_class_dynamic_cast_assert
  10       2.84%  iotlb_to_section
  11       2.63%  memory_region_access_valid
  12       2.50%  io_readx
  13       2.06%  flatview_translate
  14       2.04%  systick_read
  15       2.01%  tlb_set_page_with_attrs
  16       2.01%  object_dynamic_cast_assert
  17       1.72%  get_phys_addr
  18       1.62%  fifo8_pop
  19       1.62%  flatview_read
  20       1.42%  subpage_read
  21       1.22%  fifo8_push
  22       1.10%  victim_tlb_hit
  23       1.10%  adjust_endianness.isra.15
  24       1.00%  cpu_exec
  25       0.91%  memory_region_read_accessor
```

## HPPA

**Source:**
[https://www.mail-archive.com/qemu-devel@nongnu.org/msg604710.html](https://www.mail-archive.com/qemu-devel@nongnu.org/msg604710.html)

**Emulation Setup:**

```bash
# Download the image file
wget http://backup.parisc-linux.org/debian-cd/debian-8.0/hppa/debian-8.0-hppa-CD-1.iso
# Start system emulation
<qemu-system-build>/hppa-softmmu/qemu-system-hppa -nographic -m 256M -cdrom debian-8.0-hppa-CD-1.iso -boot d
```

**Emulation Demo:**

![HPPA]({{ site.baseurl }}/images/system_mode/hppa.gif)

**Boot-up Time:**

```
real	0m26.902s
user	0m27.643s
sys	0m0.205s
```

**Top 25 Functions:**

```
Number of instructions: 148,143,319,523

 No.  Percentage  Name
----  ----------  ------------------------------
   1      25.62%  helper_lookup_tb_ptr
   2      18.57%  tlb_flush_by_mmuidx_async_work
   3       7.55%  hppa_get_physical_address
   4       4.03%  tb_htable_lookup
   5       2.14%  tlb_set_page_with_attrs
   6       1.68%  get_page_addr_code_hostp
   7       1.61%  qht_lookup_custom
   8       1.19%  object_class_dynamic_cast_assert
   9       1.14%  liveness_pass_1
  10       1.07%  qemu_ram_block_from_host
  11       1.02%  helper_itlba
  12       0.94%  cpu_exec
  13       0.82%  tcg_gen_code
  14       0.72%  victim_tlb_hit
  15       0.55%  tb_lookup_cmp
  16       0.54%  helper_itlbp
  17       0.53%  artist_rop8
  18       0.53%  qemu_ram_addr_from_host
  19       0.51%  block_move
  20       0.51%  tcg_optimize
  21       0.45%  ptlb_work
  22       0.43%  find_next_bit
  23       0.42%  address_space_translate_for_iotlb
  24       0.38%  address_space_translate_internal
  25       0.35%  access_with_adjusted_size
```

## M68k

**Source:**
[https://www.mail-archive.com/qemu-devel@nongnu.org/msg605814.html](https://www.mail-archive.com/qemu-devel@nongnu.org/msg605814.html)

**Emulation Setup:**

```bash
# Download the image file
wget https://web.archive.org/web/20180303021225/http://www.uclinux.org/ports/coldfire/image-an5206-small-20000706.bin.gz
# Extract the archive content
gunzip image-an5206-small-20000706.bin.gz
# Start system emulation
<qemu-system-build>/m68k-softmmu/qemu-system-m68k -M an5206 -kernel image-an5206-small-20000706.bin -nographic
```

**Emulation Demo:**

![M68k]({{ site.baseurl }}/images/system_mode/m68k.gif)

**Boot-up Time:**

```
real	0m0.643s
user	0m0.607s
sys	0m0.029s
```

**Top 25 Functions:**

```
Number of instructions: 18,848,179,906

 No.  Percentage  Name
----  ----------  ------------------------------
   1      21.53%  cpu_exec
   2      14.52%  helper_lookup_tb_ptr
   3       8.26%  object_class_dynamic_cast_assert
   4       4.04%  cpu_m68k_get_ccr
   5       4.04%  cpu_m68k_set_sr
   6       3.57%  m68k_switch_sp
   7       0.67%  object_get_class
   8       0.62%  helper_get_ccr
   9       0.53%  helper_set_sr
  10       0.16%  tcg_gen_code
  11       0.13%  liveness_pass_1
  12       0.11%  tcg_out_opc.isra.13
  13       0.09%  page_trylock_add
  14       0.09%  page_collection_lock
  15       0.09%  tcg_optimize
  16       0.05%  notdirty_write.isra.15
  17       0.04%  tcg_op_alloc
  18       0.04%  tb_page_addr_cmp
  19       0.03%  tcg_opt_gen_mov
  20       0.03%  tcg_reg_alloc
  21       0.03%  find_next_bit
  22       0.03%  init_ts_info
  23       0.02%  tcg_out_modrm_sib_offset
  24       0.02%  tb_gen_code
  25       0.02%  bitmap_set_atomic
```

## MIPSel

**Source:**
[https://www.qemu.org/docs/master/system/target-mips.html](https://www.qemu.org/docs/master/system/target-mips.html#nanomips-system-emulator)

**Emulation Setup:**

```bash
# Download and extract the kernel file
wget https://mipsdistros.mips.com/LinuxDistro/nanomips/kernels/v4.15.18-432-gb2eb9a8b07a1-20180627102142/generic_nano32r6el_page16k.xz
unxz generic_nano32r6el_page16k.xz
# Download and extract the disk image file
wget https://mipsdistros.mips.com/LinuxDistro/nanomips/buildroot/nanomips32r6_le_sf_2017.05-03-59-gf5595d6.ext4.xz
unxz nanomips32r6_le_sf_2017.05-03-59-gf5595d6.ext4.xz
# Start system emulation
<qemu-system-build>/mipsel-softmmu/qemu-system-mipsel -cpu I7200 -kernel generic_nano32r6el_page16k \
-M malta -serial stdio -m 1024 -hda nanomips32r6_le_sf_2017.05-03-59-gf5595d6.ext4 \
-append "mem=256m@0x0 rw console=ttyS0 vga=cirrus vesa=0x111 root=/dev/sda"
```

**Emulation Demo:**

![MIPSel]({{ site.baseurl }}/images/system_mode/mipsel.gif)

**Boot-up Time:**

```
real	0m3.931s
user	0m3.183s
sys	0m0.131s
```

**Top 25 Functions:**

```
Number of instructions: 33,844,336,116

 No.  Percentage  Name
----  ----------  ------------------------------
   1       5.86%  address_space_translate_internal
   2       5.39%  access_with_adjusted_size
   3       5.08%  liveness_pass_1
   4       4.59%  flatview_do_translate
   5       3.52%  flatview_access_valid
   6       2.24%  memory_region_dispatch_write
   7       1.89%  flatview_read_continue
   8       1.84%  object_class_dynamic_cast_assert
   9       1.78%  memory_region_access_valid
  10       1.74%  flatview_translate
  11       1.72%  iotlb_to_section
  12       1.61%  tcg_gen_code
  13       1.56%  io_writex
  14       1.53%  helper_ret_stb_mmu
  15       1.49%  io_readx
  16       1.40%  cmos_ioport_read
  17       1.31%  memory_region_dispatch_read
  18       1.29%  helper_lookup_tb_ptr
  19       1.29%  flatview_write_continue
  20       1.22%  memory_region_read_with_attrs_accessor
  21       0.97%  memory_region_write_accessor
  22       0.96%  full_ldub_mmu
  23       0.92%  memory_region_write_with_attrs_accessor
  24       0.90%  memory_region_read_accessor
  25       0.89%  helper_rotx
```

## SH4

**Source:**
[https://www.mail-archive.com/qemu-devel@nongnu.org/msg604838.html](https://www.mail-archive.com/qemu-devel@nongnu.org/msg604838.html)

**Emulation Setup:**

```bash
# Download the kernel file
wget https://people.debian.org/~aurel32/qemu/sh4/vmlinuz-2.6.32-5-sh7751r
# Download the initial ramdisk file
wget https://people.debian.org/~aurel32/qemu/sh4/initrd.img-2.6.32-5-sh7751r
# Download the disk image file
wget https://people.debian.org/~aurel32/qemu/sh4/debian_sid_sh4_standard.qcow2
# Start system emulation
<qemu-system-build>/sh4-softmmu/qemu-system-sh4 -M r2d -kernel vmlinuz-2.6.32-5-sh7751r \
-initrd initrd.img-2.6.32-5-sh7751r -hda debian_sid_sh4_standard.qcow2 \
-append "root=/dev/sda1 console=tty0 noiotrap"
```

**Emulation Demo:**

![SH4]({{ site.baseurl }}/images/system_mode/sh4.gif)

**Boot-up Time:**

```
real	0m33.843s
user	0m20.976s
sys	0m3.179s
```

**Top 25 Functions:**

```
Number of instructions: 114,925,910,876

 No.  Percentage  Name
----  ----------  ------------------------------
   1       9.34%  find_tlb_entry
   2       4.97%  helper_lookup_tb_ptr
   3       3.66%  liveness_pass_1
   4       3.07%  tb_jmp_cache_clear_page
   5       2.85%  tb_htable_lookup
   6       2.37%  page_collection_lock
   7       2.23%  tlb_set_page_with_attrs
   8       1.90%  tlb_flush_page_by_mmuidx_async_0
   9       1.82%  page_trylock_add
  10       1.71%  object_class_dynamic_cast_assert
  11       1.62%  tcg_gen_code
  12       1.58%  get_page_addr_code_hostp
  13       1.56%  address_space_translate_internal
  14       1.52%  access_with_adjusted_size
  15       1.46%  cpu_exec
  16       1.19%  qht_lookup_custom
  17       0.91%  tcg_optimize
  18       0.90%  victim_tlb_hit
  19       0.86%  flatview_access_valid
  20       0.83%  tb_page_addr_cmp
  21       0.82%  flatview_do_translate
  22       0.72%  get_physical_address.isra.4
  23       0.68%  tcg_out_opc.isra.13
  24       0.66%  io_readx
  25       0.66%  iotlb_to_section
```

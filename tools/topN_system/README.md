### Finding the Most Executed Functions in System Mode

The script takes only one optional argument `-n` to specify the number of top functions to display. If it's not provided, the script defaults to 25.

Usage example for aarch64 with Debian:

```bash
./topN_system.py -- <qemu-system-build>/aarch64-softmmu/qemu-system-aarch64 \
-m 1024 -M virt -cpu cortex-a57 -kernel images_debian/aarch64/vmlinuz -initrd images_debian/aarch64/initrd.gz \
-append "root=/dev/ram"
```

Results:

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

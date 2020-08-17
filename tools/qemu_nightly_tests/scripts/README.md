### QEMU Nightly Tests

**Required settings:**

Update the [`GMAIL_USER`](https://github.com/ahmedkrmn/TCG-Continuous-Benchmarking/blob/242613d274ba96648f0f286367ca714166ba3a54/tools/qemu_nightly_tests/scripts/send_email.py#L31) object in `send_email.py` with your credentials.

**Running the System:**

```bash
python run_nightly_tests.py
```

The default reference version is v5.1.0. To specify a custom version, please use the `-r, --reference` flag.

For more details on how the system work, please check the [eighth report](https://ahmedkrmn.github.io/TCG-Continuous-Benchmarking/QEMU-Nightly-Performance-Tests/) of the "TCG Continuos Benchmarking" series.

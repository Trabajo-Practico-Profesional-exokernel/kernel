## x86 Kernel Testing

### Dependencies:

OpenSBI

```bash
curl -LO https://github.com/qemu/qemu/raw/v8.0.4/pc-bios/opensbi-riscv32-generic-fw_dynamic.bin
```

### How to Run:

```bash
make ARCH=x86 run
```

```bash
make ARCH=riscv run
```

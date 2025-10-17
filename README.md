## Kernel

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

### How to debug

```bash
make ARCH=x86 debug
```

In another console

```bash
gdb build/x86/kernel.elf
```

```bash
(gdb) target remote :26000
(gdb) symbol-file build/x86/kernel.elf
(gdb) c
```

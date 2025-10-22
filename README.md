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


## Utilidades

se puede usar run.sh para correr/hacer cleanups
```bash
./run.sh [trg: riscv]
```
compila el trg, ultimo parametro que no sea un flag. Como flags estan el
-c para hacer make clean antes, -uc para recompilar los programas de usuario.

```bash
./dump.sh [trg: riscv]
```

hace dump el trg .elf... se puede pasar el flag -u para hacer el dump de un programa de usuario.. en tal caso de tener el -u el default trg es shell.


## Kernel

### Dependencies:

[OpenSBI](https://github.com/riscv-software-src/opensbi)

```bash
curl -LO https://github.com/qemu/qemu/raw/v8.0.4/pc-bios/opensbi-riscv32-generic-fw_dynamic.bin
```

[GenISOImage](https://wiki.debian.org/genisoimage)

```bash
sudo apt-get install build-essential nasm genisoimage bochs bochs-sdl
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

## Utilidades

se puede usar run.sh para correr/hacer cleanups
```bash
./run.sh [trg: riscv]
```
compila el trg, ultimo parametro que no sea un flag. Como flags estan el
-c para hacer make clean antes, -uc para recompilar los programas de usuario.
se puede usar -d para correr make debug y posteriormente correr el debugger.sh para conectar con gdb y empezar el debugeo.

El uso del debugger es

```bash
./debugger.sh [trg: riscv] [setup_file:gdb_setup]
```
Por default asume el target es riscv y el setup file, comandos setup de breakpoints, etc. de gdb como gdb_setup. Este programa corre gdb basicamente para poder debugear el kernel. Que inicializo con el run.sh -d .. o con el make debug

Para el debugger para riscv se necesita gdb-multiarch
```bash
```

```bash
./dump.sh [trg: riscv]
```

hace dump el trg .elf... se puede pasar el flag -u para hacer el dump de un programa de usuario.. en tal caso de tener el -u el default trg es shell.



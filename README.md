## Kernel


## Dependencies:

Dependencias basicas 

Make
```bash
sudo apt-get install make
```

Para compilacion de programas de usuario y hacer dump con llvm-objdump y para los programas de usuario se usa llvm-objcopy
```bash
sudo apt-get install llvm
```

### X86:

Build essential installs make
[GenISOImage](https://wiki.debian.org/genisoimage)

```bash
sudo apt-get install build-essential nasm genisoimage bochs bochs-sdl
```

Para debugging
```bash
sudo apt-get install gdb
```

Qemu 
```bash
sudo apt-get install qemu-system-x86
```


### Riscv

Required for debugging with riscv
```bash
sudo apt-get install gdb-multiarch
```
Qemu 
```bash
sudo apt-get install qemu-system-misc
```


[OpenSBI](https://github.com/riscv-software-src/opensbi)

```bash
curl -LO https://github.com/qemu/qemu/raw/v8.0.4/pc-bios/opensbi-riscv32-generic-fw_dynamic.bin
```


You can run install_dep.sh to install dependencies, except the OpenSBI binaries.




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



#!/bin/bash


trg="${1:-riscv}"
setup_file="${2:-gdb_setup}" # mas setup especifico en un archivo

# Build gdb command in preferred order.
# We'll:
#  1) connect to the remote,
#  2) load symbols/file,
#  3) then run any user-provided setup file (so 'c' will continue after connect).

if [[ "$trg" == "riscv" ]]; then
  cmd=(gdb-multiarch -q)
  cmd+=("-ex" "set architecture riscv:rv32")
else
  cmd=(gdb -q)  
fi
cmd+=("-ex" "target remote :26000")
cmd+=("-ex" "symbol-file build/${trg}/kernel.elf")


# If a setup file was provided, read it after connecting
if [[ -n "$setup_file" ]]; then
  echo "Adding setup file $setup_file"

  cmd+=(-x "$setup_file")
fi

# finally pass the ELF as positional
cmd+=("build/${trg}/kernel.elf")

# execute
exec "${cmd[@]}"

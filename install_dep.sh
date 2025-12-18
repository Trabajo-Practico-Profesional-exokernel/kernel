

echo "###### First update apt-get just in case"
sudo apt-get update


echo "###### Installing essentials and basic dependencies for x86"
sudo apt-get install clang
sudo apt install lld

sudo apt-get install build-essential nasm genisoimage bochs bochs-sdl

echo "###### Installing qemu for both architectures"
sudo apt-get install qemu-system-x86 qemu-system-misc


echo "###### Installing gdb for both architectures"
sudo apt-get install gdb gdb-multiarch

echo "###### Installing llvm"
sudo apt-get install llvm

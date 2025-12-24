#!/bin/bash


clean_build=0
clean_user_apps=0
verbose=0
debug=0
trg=riscv

while [[ $# -gt 0 ]]; do
	case "$1" in
	        -c) clean_build=1; shift ;;
	        -v) verbose=1; shift ;;
	        -d) debug=1; shift ;;
	        -build_users) clean_user_apps=1; shift ;;
	        #-*) echo "Got flag $1 after - :${1#-}" ; shift;;  	
	*) trg=$1 ; shift ;;
	esac
done
cd src

if [[ $clean_user_apps -eq 1 ]]; then
	if [[ $verbose -eq 0 ]]; then
		clear
	fi

	echo "clean user apps build"
	rm -r apps/build

	# # Si es riscv compila los programas user space
	if [[ "$trg" == "riscv" ]]; then
		echo "-->Compiling user space apps!"
		cd user
		make
		cd ..
		
		clean_build=1 # Ensure no issues with vars
	fi
fi

if [[ "$trg" == "c" ]]; then
	if [[ $verbose -eq 0 ]]; then
		clear
	fi
	echo "make clean kernel build"
	make clean
	exit
fi


if [[ $clean_build -eq 1 ]]; then
	if [[ $verbose -eq 0 ]]; then
		clear
	fi
	echo "clean kernel build before make"
	make clean
fi
if [[ $verbose -eq 0 ]]; then
	clear
fi

echo "trg: $trg"

if [[ $debug -eq 1 ]]; then
	echo "-->Running make ARCH=$trg debug"
	make ARCH=$trg debug
else
	echo "-->Running make ARCH=$trg run"
	make ARCH=$trg run	
fi

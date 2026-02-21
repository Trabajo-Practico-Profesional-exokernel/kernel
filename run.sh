#!/bin/bash


clean_build=0
clean_user_apps=0
verbose=0
debug=0
trg=riscv
testing=0
only_build=0
disk=
level=-1

while [[ $# -gt 0 ]]; do
	case "$1" in
	        -c) clean_build=1; shift ;;
	        -v) verbose=1; shift ;;
	        -d) debug=1; shift ;;
	        -users) clean_user_apps=1; shift ;;
	        -t) testing=1; shift ;;
	        -b) only_build=1; shift ;;
			-disk) disk=$2; shift 2 ;;
			-level) level=$2; shift 2 ;;
	        #-*) echo "Got flag $1 after - :${1#-}" ; shift;;  	
	*) trg=$1 ; shift ;;
	esac
done

if [[ "$trg" == "riscv" || "$trg" == "x86" ]]; then
	NCPU=$1 # Next arg after arch is ncpus!
	if [ -z "$NCPU" ];then
		NCPU=1
	fi

else
	NCPU=$trg # first arg is ncpus!

	trg=$1
	if [ -z "$trg" ];then
		trg=riscv
	fi



fi

cd src

if [[ $clean_user_apps -eq 1 ]]; then
	if [[ $verbose -eq 0 ]]; then
		clear
	fi

	cd user
	
	if [[ $clean_build -eq 1 ]]; then
		make clean
	fi
	echo "clean user apps build"

	DISK_USER_ARG=""
	if [[ -n "$disk" ]]; then
		DISK_USER_ARG="TRG_DISK=$disk"
	fi

	if [[ "$trg" == "riscv" ]]; then
		echo "-->Compiling user space apps for riscv!"
		make $DISK_USER_ARG
	else
		echo "-->Compiling user space apps for x86!"
		make ARCH=x86 $DISK_USER_ARG
	fi

	if [[ $? -ne 0 ]]; then
		echo "Failed compile user program! $?"
		exit
	fi
	cd ..
	
	if [[ $only_build -eq 1 ]]; then
		echo "Built user apps. Finishing..."
		exit
	fi		
	clean_build=1 # Ensure no issues with vars
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

echo "trg: $trg with '$NCPU' cpus"

trg_action=run


if [[ $only_build -eq 1 ]]; then
	trg_action=
else
	if [[ $debug -eq 1 ]]; then
		trg_action=debug
	fi	
fi
	
DISK_ARG=""
if [[ -n "$disk" ]]; then
	DISK_ARG="KERNEL_DISK_PATH=$disk"
fi

echo "Running: make ARCH=$trg NCPU=$NCPU TESTING=$testing $trg_action VERBOSE=$verbose DEBUG_LEVEL=$level"
make ARCH=$trg $DISK_ARG NCPU=$NCPU TESTING=$testing $trg_action VERBOSE=$verbose DEBUG_LEVEL=$level

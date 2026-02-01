#!/bin/bash

clean_build=0
is_user_app=0
is_test=0
trg=
while [[ $# -gt 0 ]]; do
	case "$1" in
	        -c) clean_build=1; shift ;;
	        -u) is_user_app=1; shift ;;
	        -tests) is_test=1; shift ;;
	        #-*) echo "Got flag $1 after - :${1#-}" ; shift;;  	
	*) trg=$1 ; shift ;;
	esac
done


if [[ $is_user_app -eq 1 ]]; then
	if [[ "$trg" == "" ]]; then
		trg=shell
	fi
	
	# if [[ $clean_build -eq 1 ]]; then
	# 	echo "Rebuilding user app with trg $trg"
	# 	rm -r apps/build
	# 	./compile_user.sh
	# fi
	
	llvm-objdump -d user/build/$trg/app.elf

else
	if [[ "$trg" == "" ]]; then
		trg=riscv
	fi

	if [[ $clean_build -eq 1 ]]; then
		echo "Rebuilding kernel with trg $trg"
		make clean
		make ARCH=$trg TESTING=$is_test
	fi
	
	llvm-objdump -d build/$trg/kernel.elf
fi

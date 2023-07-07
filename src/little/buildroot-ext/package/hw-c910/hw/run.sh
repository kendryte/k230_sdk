if [ $# -lt 1 -o $# -gt 4 ] ; then
	echo "Usage: . run.sh <ip:port> [an/eg/eg2/ve/ice_evb/ice_fpga/ice_fpga2/th/k230] [s/m] noreset"
	echo "Usage: [an/eg/eg2/ve/ice_evb/ice_fpga/ice_fpga2/th/k2] is for board name"
	echo "Usage: [s/m] is for onecore/twocore"
	exit 1
fi

BOARD="eg"
NRCORE=2

GDBRESET="reset"
if [ "$4" == "noreset" ]; then
GDBRESET="noreset"
fi

for idx in "$@"
do
if [ $idx == "an" ]; then
	BOARD="an"
elif [ $idx == "k230" ]; then
	BOARD="k230"
	GDBRESET="reset"	
elif [ $idx == "d1_nfs" ]; then
	BOARD="d1_nfs"
elif [ $idx == "lw" ]; then
	BOARD="lw"
elif [ $idx == "lw_nfs" ]; then
	BOARD="lw_nfs"
elif [ $idx == "lw1" ]; then
	BOARD="lw1"
elif [ $idx == "lw2" ]; then
	BOARD="lw2"
elif [ $idx == "lw3" ]; then
	BOARD="lw3"
elif [ $idx == "lw_fm" ]; then
	BOARD="lw_fm"
elif [ $idx == "lw_fm1" ]; then
	BOARD="lw_fm1"
elif [ $idx == "lw_fm2" ]; then
	BOARD="lw_fm2"
elif [ $idx == "lw_fm3" ]; then
	BOARD="lw_fm3"
elif [ $idx == "ice_evb" ]; then
	BOARD="ice_evb"
	GDBRESET="noreset"
elif [ $idx == "ice_evb_nfs" ]; then
	BOARD="ice_evb_nfs"
	GDBRESET="noreset"
elif [ $idx == "ice_fpga" ]; then
	BOARD="ice_fpga"
	GDBRESET="noreset"
elif [ $idx == "ice_fpga2" ]; then
	BOARD="ice_fpga2"
	GDBRESET="noreset"
elif [ $idx == "ice2" ]; then
	BOARD="ice2"
	GDBRESET="noreset"
elif [ $idx == "th" ]; then
	BOARD="th"
elif [ $idx == "eg" ]; then
	BOARD="eg"
elif [ $idx == "eg2" ]; then
	BOARD="eg2"
elif [ $idx == "ve" ]; then
	BOARD="ve"
	GDBRESET="noreset"
elif [ $idx == "s" ]; then
	NRCORE=1
elif [ $idx == "m" ]; then
	NRCORE=2
fi
case "$idx" in
    [1-9] | [1-1][0-6])
		NRCORE=$idx
esac
done

set -ex

DDRINIT=ddrinit.$BOARD.txt
GDBINIT=gdbinit.$BOARD.txt
DTS=$BOARD.dts.txt

echo "Use config" $NRCORE $BOARD
if [ ! -f $GDBINIT -o ! -f $DTS ]; then
	echo "No support"
	exit 1
fi

enable_dts_cores() {
	let x=$1-1
	for ((i=1;i<=$x;i++)); do
		y=`awk '/cpu@'$i' \{/{getline a;print NR}' .hw.dts`
		let y=$y+2
		sed -i ''$y','$y's/fail/okay/g' .hw.dts
	done
}

cp $DTS .hw.dts
if [ $NRCORE -gt 1 ] ; then
enable_dts_cores $NRCORE
fi

ROOTFS_BASE=`cat .hw.dts | grep initrd-start | awk -F " " '{print $4}' | awk -F ">" '{print $1}'`
ROOTFS_SIZE=`ls -lt ../rootfs-final.cpio.gz | awk '{print $5}'`
((ROOTFS_END= $ROOTFS_BASE + $ROOTFS_SIZE))
ROOTFS_END=`printf "0x%x" $ROOTFS_END`
sed -i "s/linux,initrd-end = <0x0 .*/linux,initrd-end = <0x0 $ROOTFS_END>;/g" .hw.dts

./dtc -I dts -O dtb .hw.dts > hw.dtb

# reset
if [ $GDBRESET == "reset" ]; then
riscv64-unknown-linux-gnu-gdb -ex "tar remote $1" -ex "reset" -ex "set confirm off" -ex "q"
fi

# Init DDR
if [ $BOARD == "eg" -o $BOARD == "eg2" ]; then
riscv64-unknown-linux-gnu-gdb -ex "tar remote $1" -x $DDRINIT ddr_init_eg_elf -ex "c" -ex "q" > /dev/null
else
riscv64-unknown-linux-gnu-gdb -ex "tar remote $1" -x $DDRINIT -ex "q"
fi

# Run linux
#riscv64-unknown-linux-gnu-gdb -ex "tar remote $1" -x $GDBINIT -ex "c" -ex "q"
riscv64-unknown-linux-gnu-gdb -ex "tar remote $1" -x $GDBINIT   

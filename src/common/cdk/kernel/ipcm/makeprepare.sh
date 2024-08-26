#!/bin/bash

top_dir=$(basename $0)
config_file=${MODULE_DIR}/arch/"$2"/configs/"$1"
device_header=include/device_config.h
virt_tty_header=class/virt-tty/virt-tty_config.h
build_config=config.mk
node_name=

function cleanup()
{
  echo -e "cleanup "
  rm -f $device_header
  rm -f $virt_tty_header
  rm -f $build_config
  rm -f config.log
}

function usage()
{
  echo -e "\n usage:"
  echo -e "\t $conf_script conf_file"
  echo -e "\t $conf_script clean"
}

function parsing_config()
{
while read LINE
do
  case "$LINE" in
    platform*)
      echo "## case platform"
      echo -e "#define __platform_${LINE#platform=}__    1">>$device_header
      echo -e "export __PLATFORM__${LINE#platform}">>$build_config
      ;;
    node_id*)
      echo "## case node_id"
      echo -e "#define LOCAL_ID ${LINE#node_id=}">>$device_header
      echo -e "export LOCAL_ID${LINE#node_id}">>$build_config
      ;;
    arch_type*)
      echo "## case arch_type"
      echo -e "#define ARCH_TYPE ${LINE#arch_type=}">>$device_header
      echo -e "export ARCH_TYPE${LINE#arch_type}">>$build_config
      ;;
    os_type*)
      echo "## case os_type"
      echo -e "#define OS_TYPE ${LINE#os_type=}">>$device_header
      echo -e "export OS_TYPE${LINE#os_type}">>$build_config
      ;;
    node_name*)
      echo "## case node_name"
      echo -e "#define NODE_NAME ${LINE#node_name=}">>$device_header
      echo -e "export NODE_NAME${LINE#node_name}">>$build_config
	  node_name=${LINE#node_name=}
      ;;
    top_role*)
      echo "## case top_role"
      echo -e "#define TOP_ROLE ${LINE#top_role=}">>$device_header
      ;;
    ipcm_irq*)
      echo "## case ipcm_irq"
      echo -e "#define IRQ_NUM ${LINE#ipcm_irq=}">>$device_header
      ;;
    max_nodes*)
      echo "## case max_nodes"
      echo -e "#define MAX_NODES ${LINE#max_nodes=}">>$device_header
      ;;
    cdev*)
      echo "## case cdev"
      echo -e "export CDEV${LINE#cdev}">>$build_config
      ;;
    sharefs*)
      echo "## case sharefs"
      echo -e "export SHAREFS${LINE#sharefs}">>$build_config
      ;;
    shm_phys*)
      echo "## case shm_phys"
      echo -e "#define ${LINE%=*}" " ${LINE#*=}" >> $device_header
      ;;
    shm_size*)
      echo "## case shm_size"
      echo -e "#define ${LINE%=*}" " ${LINE#*=}" >> $device_header
      ;;
    cross_compile*)
      echo "## case cross_compile"
      echo -e "export CFG_COMPILE${LINE#cross_compile}">>$build_config
      ;;
    cc_flags*)
      echo "## case cc_flags"
      echo -e "export IPCM_CFLAGS${LINE#cc_flags}">> $build_config
      ;;
    ld_flags*)
      echo "## case ld_flags"
      echo -e "export IPCM_LDFLAGS${LINE#ld_flags}">> $build_config
      ;;
    kernel_dir*)
      echo "## case kernel_dir"
      if [ -d ${LINE#kernel_dir=} ]
      then
	cd ${LINE#kernel_dir=}
        fullpath=$(pwd)
	cd -
        echo -e "export KERNEL_DIR=${fullpath}">>$build_config
      else
        echo "<Err>kernel directory[${LINE#kernel_dir=}] not found!"
        exit 1
      fi
      ;;
  esac
done < $config_file
}

function parsing_virt_tty_config()
{
while read LINE
do
  case "$LINE" in
    platform*)
      echo "## case platform"
      echo -e "#define __platform_${LINE#platform=}__    1\n">>$virt_tty_header
      ;;
  virt_tty_role*)
      echo "## case virt_tty role"
      echo -e "export VIRT_TTY_ROLE${LINE#virt_tty_role}">> $build_config
      echo -e "#define VIRT_TTY_ROLE    ${LINE#*=}" >> $virt_tty_header
      echo -e "#define VIRT_TTY_NAME    $node_name" >> $virt_tty_header
      ;;
  virt_tty_phys*)
      echo "## case virt_tty phys"
      echo -e "#define VIRT_TTY_PHYS    ${LINE#*=}" >> $virt_tty_header
      ;;
  virt_tty_size*)
      echo "## case virt_tty size"
      echo -e "#define VIRT_TTY_SIZE    ${LINE#*=}" >> $virt_tty_header
      ;;
  virt_tty_server_id*)
      echo "## case virt_tty server id"
      echo -e "#define VIRT_TTY_SERVER_ID    ${LINE#*=}" >> $virt_tty_header
      ;;
  esac
done < $config_file
}

echo "prepare start..."
#[ $# -ne 2 ] && { usage; exit 1; }
[ $1 == "clean" ] && { echo -e "prepare clean"; cleanup; exit 0; }

if test -f ${config_file}; then
cleanup
echo "generating config headers..."
cat >> $device_header << _ACEOF
/*
 * This is an auto generated header, please do not edit it.
 */
#ifndef __IPCM_AUTO_GENERATED_DEVICE_HEADER__
#define __IPCM_AUTO_GENERATED_DEVICE_HEADER__

_ACEOF

parsing_config >> config.log

cat >> $device_header << _ACEOF

#endif
_ACEOF


echo "generating virt_tty config headers..."
cat >> $virt_tty_header << _ACEOF
/*
 * This is an auto generated header, please do not edit it.
 */
#ifndef __AUTO_GENERATED_VIRT_TTY_HEADER__
#define __AUTO_GENERATED_VIRT_TTY_HEADER__

_ACEOF

parsing_virt_tty_config >> config.log

cat >> $virt_tty_header << _ACEOF

#endif
_ACEOF

else
echo -e "<Error>config file ${config_file} not found!"
exit 1;
fi


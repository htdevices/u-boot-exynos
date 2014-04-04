#!/bin/bash
#
# Copyright (C) 2012 Insignal
# Copyright (C) 2014 High Technology Devices LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

################################################################################

TARGET_COMPANY="samsung"
TARGET_DEVICE="exshields"
TARGET_SOC="exynos5250"

#[ -z "${ANDROID_BUILD_TOP}" ] && ANDROID_BUILD_TOP="$(pwd)"
#[ -z "${ANDROID_PRODUCT_OUT}" ] && ANDROID_PRODUCT_OUT="${ANDROID_BUILD_TOP}/out/target/product/${TARGET_DEVICE}"

# unit: block(512 bytes)
BL1_OFFSET=1
BL1_SIZE=30 # 15KB
BL1_TYPE="prebuilt"
BL1_NAME="${TARGET_SOC}.bl1.bin"
BL1_PATH="."
BL1_FILE="${BL1_PATH}/${BL1_NAME}"

BL2_OFFSET=`expr $BL1_OFFSET + $BL1_SIZE`
BL2_SIZE=32 # 16KB
BL2_TYPE="bootloader"
BL2_NAME="bl2.bin"
BL2_PATH="."
BL2_FILE="${BL2_PATH}/${BL2_NAME}"

BL3_OFFSET=`expr $BL2_OFFSET + $BL2_SIZE`
BL3_SIZE=656 # 328KB
BL3_TYPE="bootloader"
BL3_NAME="u-boot.bin"
BL3_PATH="."
BL3_FILE="${BL3_PATH}/${BL3_NAME}"

TZSW_OFFSET=`expr $BL3_OFFSET + $BL3_SIZE`
TZSW_SIZE=312 # 156KB
TZSW_TYPE="prebuilt"
TZSW_NAME="${TARGET_SOC}.tzsw.bin"
TZSW_PATH="."
TZSW_FILE="${TZSW_PATH}/${TZSW_NAME}"

ENV_OFFSET=`expr $TZSW_OFFSET + $TZSW_SIZE`
ENV_SIZE=32 # 16KB

################################################################################

function usage()
{
	echo "Usage: $0 [device path]"
	echo "   ex: $0 /dev/sdx"
	exit 0
}

function error_print()
{
	local err
	if [ "$1" -ne "0" ]; then
		err=$1
		shift
		[ $err -ne 0 -a $# -ne 0 ] && echo "!!!!! ERROR !!!!! $@"
	fi
	return $err
}

function error_exit()
{
	local err
	if [ "$1" -ne "0" ]; then
		err=$1
		error_print $@
		umount_all
		exit $err
	fi
}

function check_continue()
{
	printf "continue (%s)? [Y/n]: " $1
	read answer
	case "$answer" in
	[Yy])
		return 0;;
	"")
		return 0;;
	*)
		return 1;;
	esac
}

################################################################################

function exec_cmd()
{
	[ $# -eq 0 ] && return 1
	sudo $@
	ret=$?
	error_print $ret $@
	return $ret
}

function umount_specific()
{
	local mnt
	mnt=$(cat /proc/mounts | grep "$1")
	[ ! -z "${mnt}" ] && exec_cmd umount "$1"
	return $?
}

function umount_all()
{
	for dev in "$(pwd)/${SRC_MOUNTPOINT}" "$(pwd)/${DST_MOUNTPOINT}" $@
	do
		umount_specific ${dev}
	done
	return 0
}

################################################################################

function do_dd()
{
	[ $# -ne 2 ] && return 1
	exec_cmd dd iflag=dsync oflag=dsync if="$1" of="${DEVICE}" bs=512 seek=$2
	return 0;
}

################################################################################

[ "$#" -ne "0" ] || usage

MAKE_START=`date +%s`
DEVICE="$1"
BASEPATH="$2"
[ -z "${BASEPATH}" ] && BASEPATH="$(pwd)"
CMD_IDX=0
SCRIPT_NAME="[MKSDCARD]"
STEP_IDX=1

################################################################################

echo ""
echo "STEP ${STEP_IDX}. check device"
STEP_IDX=`expr ${STEP_IDX} + 1`
ls -l ${DEVICE}
[ "$?" -eq "0" ] || usage
[ -e "${DEVICE}" ] || usage
[ -b "${DEVICE}" ] || usage
for dev in ${DEVICE}[0-9]
do
	cat /proc/mounts | grep $dev | awk '{ printf "device=\"%s\" mount-point=\"%s\" f/s-type=\"%s\"\n", $1, $2, $3 }'
done

################################################################################

echo ""
echo "STEP ${STEP_IDX}. confirm device"
STEP_IDX=`expr ${STEP_IDX} + 1`
check_continue
[ $? -eq 0 ] || exit 0

################################################################################

echo ""
echo "STEP ${STEP_IDX}. check file"
STEP_IDX=`expr ${STEP_IDX} + 1`
CMD_IDX=0

ls -l "${BL1_FILE}"; ret=$?; 
[ $ret -ne 0 ] && error_exit $ret "Can not found '${BL1_FILE}'"
ls -l "${BL2_FILE}"; ret=$?; 
[ $ret -ne 0 ] && error_exit $ret "Can not found '${BL2_FILE}'"
ls -l "${BL3_FILE}"; ret=$?; 
[ $ret -ne 0 ] && error_exit $ret "Can not found '${BL3_FILE}'"
ls -l "${TZSW_FILE}"; ret=$?; 
[ $ret -ne 0 ] && error_exit $ret "Can not found '${TZSW_FILE}'"

################################################################################

echo ""
echo "STEP ${STEP_IDX}. unmounting partitions"
STEP_IDX=`expr ${STEP_IDX} + 1`
umount_all ${DEVICE}[0-9]

################################################################################

echo ""
echo "STEP ${STEP_IDX}. clean up boot area"
STEP_IDX=`expr ${STEP_IDX} + 1`

#exec_cmd dd if=/dev/zero of=${DEVICE} bs=512 seek=$BL1_OFFSET count=$BL1_SIZE
#[ $ret -ne 0 ] && error_exit $ret "Can not access '${DEVICE}'"
#exec_cmd dd if=/dev/zero of=${DEVICE} bs=512 seek=$BL2_OFFSET count=$BL2_SIZE
#[ $ret -ne 0 ] && error_exit $ret "Can not access '${DEVICE}'"
#exec_cmd dd if=/dev/zero of=${DEVICE} bs=512 seek=$BL3_OFFSET count=$BL3_SIZE
#[ $ret -ne 0 ] && error_exit $ret "Can not access '${DEVICE}'"
#exec_cmd dd if=/dev/zero of=${DEVICE} bs=512 seek=$TZSW_OFFSET count=$TZSW_SIZE
#[ $ret -ne 0 ] && error_exit $ret "Can not access '${DEVICE}'"
exec_cmd dd if=/dev/zero of=${DEVICE} bs=512 seek=$ENV_OFFSET count=$ENV_SIZE
[ $ret -ne 0 ] && error_exit $ret "Can not access '${DEVICE}'"

################################################################################

echo ""
echo "STEP ${STEP_IDX}. copy/fusing files"
STEP_IDX=`expr ${STEP_IDX} + 1`

do_dd "${BL1_FILE}" "${BL1_OFFSET}"
[ $ret -ne 0 ] && error_exit $ret "Can not fuse '${BL1_FILE}'"
do_dd "${BL2_FILE}" "${BL2_OFFSET}"
[ $ret -ne 0 ] && error_exit $ret "Can not fuse '${BL2_FILE}'"
do_dd "${BL3_FILE}" "${BL3_OFFSET}"
[ $ret -ne 0 ] && error_exit $ret "Can not fuse '${BL3_FILE}'"
do_dd "${TZSW_FILE}" "${TZSW_OFFSET}"
[ $ret -ne 0 ] && error_exit $ret "Can not fuse '${TZSW_FILE}'"

################################################################################

echo "STEP ${STEP_IDX}. Completed."
STEP_IDX=`expr ${STEP_IDX} + 1`

MAKE_END=`date +%s`
[ $? == 0 ] && let "MAKE_ELAPSED=$MAKE_END-$MAKE_START" && echo "Fuse time = $MAKE_ELAPSED sec."

################################################################################


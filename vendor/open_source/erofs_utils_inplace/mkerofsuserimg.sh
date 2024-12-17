#!/bin/bash
#
# To call this script, make sure make_erofs is somewhere in PATH
# Copyright © Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
set -e

function usage() {
cat<<EOT
Usage:
mkuserimg.sh [-s] SRC_DIR OUTPUT_FILE EROFS_VARIANT MOUNT_POINT SIZE [-j <journal_size>]
             [-T TIMESTAMP] [-C FS_CONFIG] [-D PRODUCT_OUT] [-B BLOCK_LIST_FILE]
             [-d BASE_ALLOC_FILE_IN ] [-A BASE_ALLOC_FILE_OUT ] [-L LABEL]
             [-i INODES ] [-e ERASE_BLOCK_SIZE] [-o FLASH_BLOCK_SIZE] [FILE_CONTEXTS]
             [-N PATCH_DIR]
EOT
}

echo "$*"
ENABLE_SPARSE_IMAGE=""
if [ "$1" = "-s" ]; then
  ENABLE_SPARSE_IMAGE="-s"
  shift
fi

ENABLE_COMPRESS_IMAGE=""
if [ "$1" = "-z" ]; then
  ENABLE_COMPRESS_IMAGE="-z"
  shift
fi

if [ $# -lt 5 ]; then
  usage
  exit 1
fi

SRC_DIR="$1"
if [ ! -d "$SRC_DIR" ]; then
  echo "Can not find directory $SRC_DIR!"
  exit 2
fi

OUTPUT_FILE="$2"
EROFS_VARIANT="$3"
MOUNT_POINT="$4"
SIZE="$5"
shift; shift; shift; shift; shift

JOURNAL_FLAGS=""
if [ "$1" = "-j" ]; then
  if [ "$2" = "0" ]; then
    JOURNAL_FLAGS="-J"
  else
    JOURNAL_FLAGS="-j $2"
  fi
  shift; shift
fi

TIMESTAMP=-1
if [[ "$1" == "-T" ]]; then
  TIMESTAMP="$2"
  shift; shift
fi

FS_CONFIG=""
if [[ "$1" == "-C" ]]; then
  FS_CONFIG="$2"
  shift; shift
fi

PRODUCT_OUT=""
if [[ "$1" == "-D" ]]; then
  PRODUCT_OUT="$2"
  shift; shift
fi

BLOCK_LIST=""
if [[ "$1" == "-B" ]]; then
  BLOCK_LIST="$2"
  shift; shift
fi

BASE_ALLOC_FILE_IN=""
if [[ "$1" == "-d" ]]; then
  BASE_ALLOC_FILE_IN="$2"
  shift; shift
fi

BASE_ALLOC_FILE_OUT=""
if [[ "$1" == "-A" ]]; then
  BASE_ALLOC_FILE_OUT="$2"
  shift; shift
fi

LABEL=""
if [[ "$1" == "-L" ]]; then
  LABEL="$2"
  shift; shift
fi

INODES=""
if [[ "$1" == "-i" ]]; then
  INODES="$2"
  shift; shift
fi

ERASE_SIZE=""
if [[ "$1" == "-e" ]]; then
    ERASE_SIZE="$2"
    shift; shift
fi

FLASH_BLOCK_SIZE=""
if [[ "$1" == "-o" ]]; then
    FLASH_BLOCK_SIZE="$2"
    shift; shift
fi

PATCH_PATH=""
if [[ "$1" == "-N" ]]; then
    PATCH_PATH="$2"
    shift; shift
fi
erofs_image_version=kernel_v414
if [[ "$1" == "-E" ]]; then
    erofs_image_version="$2"
    shift; shift
fi

FC="$1"

case "$EROFS_VARIANT" in
  erofs) ;;
  *) echo "Only erofs is supported!"; exit 3 ;;
esac

if [ -z "$MOUNT_POINT" ]; then
  echo "Mount point is required"
  exit 2
fi

if [ -z "$SIZE" ]; then
  echo "Need size of filesystem"
  exit 2
fi

OPT=""
if [ -n "$FC" ]; then
  OPT="$OPT -S $FC"
fi
if [ -n "$FS_CONFIG" ]; then
  OPT="$OPT -C $FS_CONFIG"
fi
if [ -n "$BLOCK_LIST" ]; then
  OPT="$OPT -B $BLOCK_LIST"
fi
if [ -n "$BASE_ALLOC_FILE_IN" ]; then
  OPT="$OPT -d $BASE_ALLOC_FILE_IN"
fi
if [ -n "$BASE_ALLOC_FILE_OUT" ]; then
  OPT="$OPT -D $BASE_ALLOC_FILE_OUT"
fi
if [ -n "$LABEL" ]; then
  OPT="$OPT -L $LABEL"
fi
if [ -n "$INODES" ]; then
  OPT="$OPT -i $INODES"
fi
if [ -n "$ERASE_SIZE" ]; then
  OPT="$OPT -e $ERASE_SIZE"
fi
if [ -n "$FLASH_BLOCK_SIZE" ]; then
  OPT="$OPT -o $FLASH_BLOCK_SIZE"
fi

MAKE_EROFS_CMD_ELF=make_erofs
if [ X"${erofs_image_version}" = X"kernel_v49" ];then
  # erofs package tool is selected by MTK macro right now
  # here not active until CI script update finished
  # MAKE_EROFS_CMD_ELF=make_erofs_k49
  MAKE_EROFS_CMD_ELF=make_erofs_k49
fi

if [ -n "$PATCH_PATH" ]; then
  files=$(ls -A "$PATCH_PATH")
  if [ -n files ]; then
    OPT="$OPT -N $PATCH_PATH"
    MAKE_EROFS_CMD_ELF=make_erofs_patch
  fi
fi

MAKE_EROFS_CMD="$MAKE_EROFS_CMD_ELF $ENABLE_SPARSE_IMAGE $ENABLE_COMPRESS_IMAGE -T $TIMESTAMP $OPT -l $SIZE $JOURNAL_FLAGS -a $MOUNT_POINT ${OUTPUT_FILE}_unsparse $SRC_DIR $PRODUCT_OUT"

echo "$MAKE_EROFS_CMD"
$MAKE_EROFS_CMD
echo "EROFS:simg2img"
img2simg "${OUTPUT_FILE}"_unsparse "$OUTPUT_FILE"
rm "${OUTPUT_FILE}"_unsparse

if [ $? -ne 0 ]; then
  exit 4
fi
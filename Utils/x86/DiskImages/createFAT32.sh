#!/bin/bash
# create a disk image with a MBR and a partition formatted with FAT32

# check that a disk image path was passed
if [ $# -lt 1 ]
then
    echo "Usage: $0 <path/to/disk/image>"
    exit 1
fi

DISK_IMAGE_PATH=$1

# create approx 320 MiB disk image (63S x 16H x 650C x 512 byte blocks)
N_CYLINDERS=650
dd if=/dev/zero of="$DISK_IMAGE_PATH" bs=516096 count="$N_CYLINDERS"

# create a loopback device
LOOP_BACK_FILE=$(losetup -f)
LOOP_BACK_FILE_P1=$LOOP_BACK_FILE"p1"

# partition
echo $'o\nn\np\n1\n\n\nw\n' | fdisk -u -C$N_CYLINDERS -S63 -H16 $DISK_IMAGE_PATH
losetup -P $LOOP_BACK_FILE $DISK_IMAGE_PATH
mkdosfs -F32 -I $LOOP_BACK_FILE_P1 -s 8

# fix the hidden sectors value in fat32
echo "echo -ne \\\\x00\\\\x08" | bash | dd of="$LOOP_BACK_FILE_P1" bs=1 count=2 seek=28 conv=notrunc

# fix that mkdosfs doesn't update the partition flag in the MBR
echo "echo -ne \\\\x0B" | bash | dd of="$LOOP_BACK_FILE" bs=1 count=1 seek=450 conv=notrunc

# detach loopback device
losetup -d $LOOP_BACK_FILE
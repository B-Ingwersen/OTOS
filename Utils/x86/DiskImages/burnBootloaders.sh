# burn first and second stage bootloaders to the first sector of the MBR and
# first sector of the FAT32 partition respectively

# check that a disk image file and both bootloader paths were passed
if [ $# -lt 3 ]
then
    echo "Usage: $0 <path/to/disk/image> <path/to/bootloader1> <path/to/bootloader2>"
    exit 1
fi

# get input paths
DISK_IMAGE_PATH=$1
BOOTLOADER1_PATH=$2
BOOTLOADER2_PATH=$3

# define temporary files
TMP_FILE=".inputDisk_Bootloader.temporary"

# burn the MBR bootloader
dd if=$DISK_IMAGE_PATH of=$TMP_FILE bs=1 count=512
dd if=$BOOTLOADER1_PATH of=$TMP_FILE bs=1 count=440 conv=notrunc
dd if=$BOOTLOADER1_PATH of=$TMP_FILE bs=1 skip=510 seek=510 count=2 conv=notrunc
dd if=$TMP_FILE of=$DISK_IMAGE_PATH bs=1 count=512 conv=notrunc
rm $TMP_FILE

# setup the loopback device
LOOP_BACK_FILE=$(losetup -f)
losetup -o1048576 $LOOP_BACK_FILE $DISK_IMAGE_PATH

# write the FAT32 bootloader
dd if=$LOOP_BACK_FILE of=$TMP_FILE bs=1 count=512
dd if=$BOOTLOADER2_PATH of=$TMP_FILE bs=1 count=3 conv=notrunc
dd if=$BOOTLOADER2_PATH of=$TMP_FILE bs=1 skip=90 seek=90 count=420 conv=notrunc
dd if=$TMP_FILE of=$LOOP_BACK_FILE bs=1 count=512 conv=notrunc
rm $TMP_FILE

# detach loopback device
losetup -d $LOOP_BACK_FILE
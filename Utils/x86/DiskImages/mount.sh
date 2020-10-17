# mount the disk image onto the file system

# check that a disk image file, folder, and mount point were passed
if [ $# -lt 2 ]
then
    echo "Usage: $0 <disk/image> <mount/point>"
    exit 1
fi

# get input paths
DISK_IMAGE_PATH=$1
MOUNT_POINT=$2

LOOP_BACK_FILE=$(losetup -f)
LOOP_BACK_FILE_P1=$LOOP_BACK_FILE"p1"
losetup -P $LOOP_BACK_FILE $DISK_IMAGE_PATH
mount -tvfat $LOOP_BACK_FILE_PL $MOUNT_POINT


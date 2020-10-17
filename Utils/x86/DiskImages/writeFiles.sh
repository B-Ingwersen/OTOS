# copy files from the file system onto a disk image

# check that a disk image file, folder, and mount point were passed
if [ $# -lt 3 ]
then
    echo "Usage: $0 <disk/image> <folder/to/copy> <mount/point>"
    exit 1
fi

# get input paths
DISK_IMAGE_PATH=$1
FOLDER_TO_COPY=$2
MOUNT_POINT=$3

# setup the loopback device
LOOP_BACK_FILE=$(losetup -f)
losetup -o1048576 $LOOP_BACK_FILE $DISK_IMAGE_PATH

# mount the fat32 partition
mount -tvfat $LOOP_BACK_FILE $MOUNT_POINT

# clear current files and copy new ones over
echo "rm -rf $MOUNT_POINT/*" | bash
cp -r $FOLDER_TO_COPY/* $MOUNT_POINT

# unmount and detach loopback device
umount $MOUNT_POINT
losetup -d $LOOP_BACK_FILE
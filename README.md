# OTOS (Orange Tiger OS)

A hobbyist microkernel operating system project

## Building

To create the directory structure, build the test disk image, and build the cross comiler, run:

```
make x86-initializeAll
```

Note that building the gcc cross compiler takes a decent bit of time and needs to download about a hundred megabytes and takes up about a gigabyte on disk.

To actually build the code then run:

```
make x86-buildAll
```

## Testing

If you have qemu installed, you can run the default i386 emulator test with

```
make x86-test
```

If you want to test on real hardware, you could burn the disk image (located at `Build/x86/testImage.img`) to a usb disk and test on an x86 system with BIOS boot.
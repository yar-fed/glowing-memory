# Building kernel and rootfs for i386 architecture, and running it in qemu

## Building kernel
 - get dependencies
   
   on ubuntu install these
   ```
   sudo apt-get install libncurses-dev flex bison openssl libssl-dev dkms libelf-dev libudev-dev libpci-dev libiberty-dev autoconf build-dep
   ```
 - get kernel sources
   ```
   git clone git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable -b v5.4 --depth 1
   cd linux-stable
   ```
 - configure
   + prepare directory
     ```
     export BUILD_KERNEL=<your_build_path>
     make O=${BUILD_KERNEL} i386_defconfig
     cd $BUILD_KERNEL
     ```

   + configure kernel
     ```
     make menuconfig
     ```

   + build kernel
     ```
     make -j4 
     ```
     *(you can use your own value in -j to allocate cpu threads if you have more than 4)*

   The build result is `$BUILD_KERNEL/arch/i386/boot/bzImage`

## Building rootfs
 - get sources
   ```
   git clone git://git.buildroot.net/buildroot
   cd buildroot
   ```
 
 - configure
   + prepare directory
     ```
     export BUILD_ROOTFS=<your_build_path>
     make O=${BUILD_ROOTFS} qemu_x86_defconfig
     cd ${BUILD_ROOTFS}
     ```

   + make changes in configuration (or make sure they are set as specified below)

     + Target options:
       + Target Architecture = i386
       + Target Architecture Variant = i686
     + Toolchain:
       + Custom kernel headers series = 5.4.x (if you build another version of kernel than use it instead)
       + [\*] Enable WCHAR support
     + System configuration:
       + System hostname = mymachine (give it a name)
       + System banner = Welcome to myLinux (you can use your own value)
       + [*] Enable root login with password
       + Root password = <rootpass>
       + Path to the users tables = ${BUILD_ROOTFS}/users (replace variable with its value to avoid potential errors)
       + Root filesystem overlay directories = ${BUILD_ROOTFS}/root (replace variable)
     + Kernel:
       + [ ] Linux Kernel (we have built it already so uncheck this, otherwise it will build its own kernel)
     + Target packages:
       + [*] Show packages that are also provided by busybox
       + Development tools
         + [*] binutils
         + [*] binutils binaries
         + [*] findutils
         + [*] grep
         + [*] sed
         + [*] tree
       + Libraries:
         + Compression and decompression:
           + [*] zlib
       + Text and terminal handling:
         + [*] ncurses
         + [*] readline
       + Networking applications:
         + [*] dropbear (ssh server)
         + [*] wget
       + Shell and utilities:
         + [*] bash
         + [*] file
         + [*] sudo
         + [*] which
       + System tools
         + [*] kmod
         + [*] kmod utilities
         + [*] rsyslog
       + Text editors and viewers:
         + [*] <nano/vim/joe> (check any one of these, whichever you are more comfortable with)
         + [*] less
         + [*] mc
     + Filesystem images:
         + [*] ext2/3/4 root filesystem
         + ext2/3/4 variant = ext3
         + [*] tar the root filesystem
   + additional files

     + user record
       ```
       echo "user 1000 user 1000 =pass /home/user /bin/bash - Linux User" > ${BUILD_ROOTFS}/users
       ```
     + add user to sudoers
       ```
       mkdir -p ${BUILD_ROOTFS}/root/etc/sudoers.d
       echo "user ALL=(ALL) ALL" > ${BUILD_ROOTFS}/root/etc/sudoers.d/user
       ```
     + list shells for dropbear (ssh server)
       ```
       mkdir -p ${BUILD_ROOTFS}/root/etc
       echo "/bin/sh" > ${BUILD_ROOTFS}/root/etc/shells
       echo "/bin/bash" >> ${BUILD_ROOTFS}/root/etc/shells
       ```
   + build rootfs (it will take a lot of time, so make sure to use -j<n>)
     ```
     make -j4
     ```
     If you have `rsync ... Permission denied` error you will need to run make again with sudo `sudo make` and execute `sudo chown -R $(id -un) $BUILD_ROOTFS/images` after make is done

   The result is ${BUILD_ROOTFS}/images/rootfs.ext3

## Launching
 - launch qemu
   ```
   qemu-system-i386 -M pc \
   -kernel $BUILD_KERNEL/arch/i386/boot/bzImage \
   -append "root=/dev/sda" \
   -hda ${BUILD_ROOTFS}/images/rootfs.ext3 \
   -nic user,hostfwd=tcp::8022-:22
   ```

 - login to your system via ssh
   ```
   ssh -p 8022 user@localhost
   ```
   If you have trouble with login use `root@localhost` with password you created in buildroot config. Then you type `passwd user` to create password for user and relogin with it.

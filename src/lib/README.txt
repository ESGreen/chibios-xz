These libraries taken out of launchpad.net's ARM GCC distro:
https://launchpad.net/gcc-arm-embedded/4.9/4.9-2015-q3-update/+download/gcc-arm-none-eabi-4_9-2015q3-20150921-linux.tar.bz2
from the subdirectories arm-none-eabi/lib/armv7e-m/fpu/libc_nano.a
and lib/gcc/arm-none-eabi/4.9.3/armv7e-m/fpu

more libs you can get from here: https://developer.arm.com/downloads/-/gnu-rm/6-2016-q4-major
~/code/gcc-arm-none-eabi-6_2-2016q4/arm-none-eabi/lib/thumb/v7e-m/fpv4-sp/hard 
~/code/gcc-arm-none-eabi-6_2-2016q4/lib/gcc/arm-none-eabi/6.2.1/thumb/v7e-m/fpv4-sp/hard

Right so after getting the above libs extract them to the lib directory

Here is what worked.
lrwxrwxrwx 1 pi pi   78 Oct  9 10:21 armv7e-m-fpu-libgcc.a -> gcc-arm-none-eabi-4_9-2015q3/lib/gcc/arm-none-eabi/4.9.3/armv7e-m/fpu/libgcc.a
drwxr-xr-x 6 pi pi 4096 Oct  9 10:22 gcc-arm-none-eabi-4_9-2015q3/
drwxr-xr-x 6 pi pi 4096 Oct  9 10:20 gcc-arm-none-eabi-6_2-2016q4/
lrwxrwxrwx 1 pi pi   78 Oct  9 10:19 libc.a -> gcc-arm-none-eabi-6_2-2016q4/arm-none-eabi/lib/thumb/v7e-m/fpv4-sp/hard/libc.a
lrwxrwxrwx 1 pi pi   71 Oct  9 10:19 libc_nano.a -> gcc-arm-none-eabi-4_9-2015q3/arm-none-eabi/lib/armv7e-m/fpu/libc_nano.a
lrwxrwxrwx 1 pi pi   11 Oct  9 10:21 libc-nano-armv7e-m-fpu.a -> libc_nano.a
lrwxrwxrwx 1 pi pi   78 Oct  9 10:19 libg.a -> gcc-arm-none-eabi-6_2-2016q4/arm-none-eabi/lib/thumb/v7e-m/fpv4-sp/hard/libg.a
lrwxrwxrwx 1 pi pi   78 Oct  9 10:21 libgcc.a -> gcc-arm-none-eabi-4_9-2015q3/lib/gcc/arm-none-eabi/4.9.3/armv7e-m/fpu/libgcc.a
lrwxrwxrwx 1 pi pi   79 Oct  9 10:21 libgcov.a -> gcc-arm-none-eabi-4_9-2015q3/lib/gcc/arm-none-eabi/4.9.3/armv7e-m/fpu/libgcov.a
lrwxrwxrwx 1 pi pi   88 Oct  9 10:21 libgloss-linux.a -> gcc-arm-none-eabi-6_2-2016q4/arm-none-eabi/lib/thumb/v7e-m/fpv4-sp/hard/libgloss-linux.a
lrwxrwxrwx 1 pi pi   83 Oct  9 10:21 libg_nano.a -> gcc-arm-none-eabi-6_2-2016q4/arm-none-eabi/lib/thumb/v7e-m/fpv4-sp/hard/libg_nano.a
lrwxrwxrwx 1 pi pi   78 Oct  9 10:21 libm.a -> gcc-arm-none-eabi-6_2-2016q4/arm-none-eabi/lib/thumb/v7e-m/fpv4-sp/hard/libm.a
lrwxrwxrwx 1 pi pi   82 Oct  9 10:21 libnosys.a -> gcc-arm-none-eabi-6_2-2016q4/arm-none-eabi/lib/thumb/v7e-m/fpv4-sp/hard/libnosys.a
lrwxrwxrwx 1 pi pi   83 Oct  9 10:21 librdimon.a -> gcc-arm-none-eabi-6_2-2016q4/arm-none-eabi/lib/thumb/v7e-m/fpv4-sp/hard/librdimon.a
lrwxrwxrwx 1 pi pi   88 Oct  9 10:21 librdimon_nano.a -> gcc-arm-none-eabi-6_2-2016q4/arm-none-eabi/lib/thumb/v7e-m/fpv4-sp/hard/librdimon_nano.a
lrwxrwxrwx 1 pi pi   83 Oct  9 10:21 librdpmon.a -> gcc-arm-none-eabi-6_2-2016q4/arm-none-eabi/lib/thumb/v7e-m/fpv4-sp/hard/librdpmon.a
lrwxrwxrwx 1 pi pi   83 Oct  9 10:21 libstdc++.a -> gcc-arm-none-eabi-6_2-2016q4/arm-none-eabi/lib/thumb/v7e-m/fpv4-sp/hard/libstdc++.a
lrwxrwxrwx 1 pi pi   88 Oct  9 10:21 libstdc++_nano.a -> gcc-arm-none-eabi-6_2-2016q4/arm-none-eabi/lib/thumb/v7e-m/fpv4-sp/hard/libstdc++_nano.a
lrwxrwxrwx 1 pi pi   83 Oct  9 10:21 libsupc++.a -> gcc-arm-none-eabi-6_2-2016q4/arm-none-eabi/lib/thumb/v7e-m/fpv4-sp/hard/libsupc++.a
lrwxrwxrwx 1 pi pi   88 Oct  9 10:21 libsupc++_nano.a -> gcc-arm-none-eabi-6_2-2016q4/arm-none-eabi/lib/thumb/v7e-m/fpv4-sp/hard/libsupc++_nano.a

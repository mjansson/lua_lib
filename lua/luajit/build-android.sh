LUAJIT=.

BUILD_DIR=$LUAJIT/build

rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR
rm *.a 1>/dev/null 2>/dev/null

HOST_OS=darwin

NDK=/Users/mjansson/android-ndk
NDKABI=16

#NDKVER=$NDK/toolchains/arm-linux-androideabi-4.9
#NDKP=$NDKVER/prebuilt/$HOST_OS-x86_64/bin/arm-linux-androideabi-
#NDKF="--sysroot $NDK/platforms/android-$NDKABI/arch-arm"
#NDKARCH=" -march=armv5te -mtune=xscale -msoft-float -marm"
#make CC="gcc" HOST_CC="gcc -m32" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android clean
#make CC="gcc" HOST_CC="gcc -m32" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android amalg
#mv $LUAJIT/src/libluajit.a $BUILD_DIR/libluajitA6.a

#NDKVER=$NDK/toolchains/arm-linux-androideabi-4.9
#NDKP=$NDKVER/prebuilt/$HOST_OS-x86_64/bin/arm-linux-androideabi-
#NDKF="--sysroot $NDK/platforms/android-$NDKABI/arch-arm"
#NDKARCH="-march=armv7-a -mhard-float -mfpu=vfpv3-d16 -mfpu=neon -D_NDK_MATH_NO_SOFTFP=1 -marm -Wl,--fix-cortex-a8"
#make CC="gcc" HOST_CC="gcc -m32" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android clean
#make CC="gcc" HOST_CC="gcc -m32" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android amalg
#mv $LUAJIT/src/libluajit.a $BUILD_DIR/libluajitA7.a

#NDKVER=$NDK/toolchains/x86-4.9
#NDKP=$NDKVER/prebuilt/$HOST_OS-x86_64/bin/i686-linux-android-
#NDKF="--sysroot $NDK/platforms/android-$NDKABI/arch-x86"
#NDKARCH=""
#make CC="gcc" HOST_CC="gcc -m32" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android clean
#make CC="gcc" HOST_CC="gcc -m32" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android amalg
#mv $LUAJIT/src/libluajit.a $BUILD_DIR/libluajitx68.a

#NDKVER=$NDK/toolchains/mipsel-linux-android-4.9
#NDKP=$NDKVER/prebuilt/$HOST_OS-x86_64/bin/mipsel-linux-android-
#NDKF="--sysroot $NDK/platforms/android-$NDKABI/arch-mips"
#NDKARCH=""
#make CC="gcc" HOST_CC="gcc -m32" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android clean
#make CC="gcc" HOST_CC="gcc -m32" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android amalg
#mv $LUAJIT/src/libluajit.a $BUILD_DIR/libluajitmips.a

NDKABI=21
NDKVER=$NDK/toolchains/aarch64-linux-android-4.9
NDKP=$NDKVER/prebuilt/$HOST_OS-x86_64/bin/aarch64-linux-android-
NDKF="--sysroot $NDK/platforms/android-$NDKABI/arch-arm64"
NDKARCH=" -DLJ_ABI_SOFTFP=0 -DLJ_ARCH_HASFPU=1"
make CC="gcc" HOST_CC="gcc -m64" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android clean
make CC="gcc" HOST_CC="gcc -m64" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android amalg
mv $LUAJIT/src/libluajit.a $BUILD_DIR/libluajitA8.a

#NDKVER=$NDK/toolchains/x86_64-4.9
#NDKP=$NDKVER/prebuilt/$HOST_OS-x86_64/bin/x86_64-linux-android-
#NDKF="--sysroot $NDK/platforms/android-$NDKABI/arch-x86_64"
#NDKARCH=""
#make CC="gcc" HOST_CC="gcc -m64" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android clean
#make CC="gcc" HOST_CC="gcc -m64" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android amalg
#mv $LUAJIT/src/libluajit.a $BUILD_DIR/libluajitx68-64.a

#NDKVER=$NDK/toolchains/mips64el-linux-android-4.9
#NDKP=$NDKVER/prebuilt/$HOST_OS-x86_64/bin/mips64el-linux-android-
#NDKF="--sysroot $NDK/platforms/android-$NDKABI/arch-mips64"
#NDKARCH=""
#make CC="gcc" HOST_CC="gcc -m64" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android clean
#make CC="gcc" HOST_CC="gcc -m64" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android amalg
#mv $LUAJIT/src/libluajit.a $BUILD_DIR/libluajitmips.a

LUAJIT=.

BUILD_DIR=$LUAJIT/build

rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR
rm *.a 1>/dev/null 2>/dev/null

HOST_OS=darwin
NDK=/Users/mattias/projects/android-ndk
NDKABI=16

mkdir ../../lib
mkdir ../../lib/android

NDKVER=$NDK/toolchains/arm-linux-androideabi-4.9
NDKP=$NDKVER/prebuilt/$HOST_OS-x86_64/bin/arm-linux-androideabi-
NDKF="--sysroot $NDK/platforms/android-$NDKABI/arch-arm"
NDKARCH="-march=armv5te -mtune=xscale -msoft-float -marm"
make CC="gcc" HOST_CC="gcc -m32" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android clean
make CC="gcc" HOST_CC="gcc -m32" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android amalg
mv $LUAJIT/src/libluajit.a $BUILD_DIR/libluajitA6.a
mkdir ../../lib/android/armeabi
cp $BUILD_DIR/libluajitA6.a ../../lib/android/armeabi/libluajit.a

NDKVER=$NDK/toolchains/arm-linux-androideabi-4.9
NDKP=$NDKVER/prebuilt/$HOST_OS-x86_64/bin/arm-linux-androideabi-
NDKF="--sysroot $NDK/platforms/android-$NDKABI/arch-arm"
NDKARCH="-march=armv7-a -mhard-float -mfpu=vfpv3-d16 -mfloat-abi=hard -D_NDK_MATH_NO_SOFTFP=1 -marm -Wl,--fix-cortex-a8"
make CC="gcc" HOST_CC="gcc -m32" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android clean
make CC="gcc" HOST_CC="gcc -m32" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android amalg
mv $LUAJIT/src/libluajit.a $BUILD_DIR/libluajitA7.a
mkdir ../../lib/android/armeabi-v7a
cp $BUILD_DIR/libluajitA6.a ../../lib/android/armeabi-v7a/libluajit.a

NDKVER=$NDK/toolchains/x86-4.9
NDKP=$NDKVER/prebuilt/$HOST_OS-x86_64/bin/i686-linux-android-
NDKF="--sysroot $NDK/platforms/android-$NDKABI/arch-x86"
NDKARCH=""
make CC="gcc" HOST_CC="gcc -m32" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android clean
make CC="gcc" HOST_CC="gcc -m32" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android amalg
mv $LUAJIT/src/libluajit.a $BUILD_DIR/libluajitx68.a
mkdir ../../lib/android/x86
cp $BUILD_DIR/libluajitA6.a ../../lib/android/x86/libluajit.a

NDKVER=$NDK/toolchains/mipsel-linux-android-4.9
NDKP=$NDKVER/prebuilt/$HOST_OS-x86_64/bin/mipsel-linux-android-
NDKF="--sysroot $NDK/platforms/android-$NDKABI/arch-mips"
NDKARCH=""
make CC="gcc" HOST_CC="gcc -m32" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android clean
make CC="gcc" HOST_CC="gcc -m32" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android amalg
mv $LUAJIT/src/libluajit.a $BUILD_DIR/libluajitmips.a
mkdir ../../lib/android/mips
cp $BUILD_DIR/libluajitA6.a ../../lib/android/mips/libluajit.a

NDKABI=21
NDKVER=$NDK/toolchains/aarch64-linux-android-4.9
NDKP=$NDKVER/prebuilt/$HOST_OS-x86_64/bin/aarch64-linux-android-
NDKF="--sysroot $NDK/platforms/android-$NDKABI/arch-arm64"
NDKARCH="-DLJ_ABI_SOFTFP=0 -DLJ_ARCH_HASFPU=1 -DLUAJIT_ENABLE_GC64=1"
make CC="gcc" HOST_CC="gcc -m64" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android clean
make CC="gcc" HOST_CC="gcc -m64" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android amalg
mv $LUAJIT/src/libluajit.a $BUILD_DIR/libluajitA8.a
mkdir ../../lib/android/arm64-v8a
cp $BUILD_DIR/libluajitA6.a ../../lib/android/arm64-v8a/libluajit.a

NDKVER=$NDK/toolchains/x86_64-4.9
NDKP=$NDKVER/prebuilt/$HOST_OS-x86_64/bin/x86_64-linux-android-
NDKF="--sysroot $NDK/platforms/android-$NDKABI/arch-x86_64"
NDKARCH="-DLUAJIT_ENABLE_GC64=1"
make CC="gcc" HOST_CC="gcc -m64" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android clean
make CC="gcc" HOST_CC="gcc -m64" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android amalg
mv $LUAJIT/src/libluajit.a $BUILD_DIR/libluajitx68-64.a
mkdir ../../lib/android/x86-64
cp $BUILD_DIR/libluajitA6.a ../../lib/android/x86-64/libluajit.a

NDKVER=$NDK/toolchains/mips64el-linux-android-4.9
NDKP=$NDKVER/prebuilt/$HOST_OS-x86_64/bin/mips64el-linux-android-
NDKF="--sysroot $NDK/platforms/android-$NDKABI/arch-mips64"
NDKARCH="-DLUAJIT_ENABLE_GC64=1"
make CC="gcc" HOST_CC="gcc -m64" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android clean
make CC="gcc" HOST_CC="gcc -m64" CROSS=$NDKP TARGET_FLAGS="$NDKF $NDKARCH" TARGET_SYS=Android amalg
mv $LUAJIT/src/libluajit.a $BUILD_DIR/libluajitmips.a
mkdir ../../lib/android/mips64
cp $BUILD_DIR/libluajitA6.a ../../lib/android/mips64/libluajit.a

LUAJIT=.
DEVDIR=`xcode-select -print-path`
IOSSDKVER=9.0
IOSDIR=$DEVDIR/Platforms
IOSBIN=$DEVDIR/Toolchains/XcodeDefault.xctoolchain/usr/bin/

BUILD_DIR=$LUAJIT/build

rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR
rm *.a 1>/dev/null 2>/dev/null

ISDKF="-arch i386 -isysroot $IOSDIR/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk -miphoneos-version-min=8.0"
make -j -C $LUAJIT HOST_CC="clang -m32" CROSS=$IOSBIN TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS clean
make -j -C $LUAJIT HOST_CC="clang -m32" CROSS=$IOSBIN TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS amalg
mv $LUAJIT/src/libluajit.a $BUILD_DIR/libluajitx86.a

ISDKF="-arch x86_64 -isysroot $IOSDIR/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk -miphoneos-version-min=8.0"
make -j -C $LUAJIT HOST_CC="clang -m64" CROSS=$IOSBIN TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS clean
make -j -C $LUAJIT HOST_CC="clang -m64" CROSS=$IOSBIN TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS amalg
mv $LUAJIT/src/libluajit.a $BUILD_DIR/libluajitx86_64.a

ISDKF="-arch armv7 -isysroot $IOSDIR/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk -miphoneos-version-min=8.0"
make -j -C $LUAJIT HOST_CC="clang -m32" CROSS=$IOSBIN TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS clean
make -j -C $LUAJIT HOST_CC="clang -m32" CROSS=$IOSBIN TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS amalg
mv $LUAJIT/src/libluajit.a $BUILD_DIR/libluajitA7.a

ISDKF="-arch armv7s -isysroot $IOSDIR/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk -miphoneos-version-min=8.0"
make -j -C $LUAJIT HOST_CC="clang -m32 -arch i386" CROSS=$IOSBIN TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS clean
make -j -C $LUAJIT HOST_CC="clang -m32 -arch i386" CROSS=$IOSBIN TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS amalg
mv $LUAJIT/src/libluajit.a $BUILD_DIR/libluajitA7s.a

ISDKF="-arch arm64 -isysroot $IOSDIR/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk -miphoneos-version-min=8.0"
make -j -C $LUAJIT HOST_CC="xcrun clang -m64 -arch x86_64" CROSS=$IOSBIN TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS clean
make -j -C $LUAJIT HOST_CC="xcrun clang -m64 -arch x86_64" CROSS=$IOSBIN TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS amalg
mv $LUAJIT/src/libluajit.a $BUILD_DIR/libluajitA64.a

libtool -o ../../lib/ios/libluajit.a $BUILD_DIR/libluajit*.a 2> /dev/null


LUAJIT=.
DEVDIR=`xcode-select -print-path`
IOSSDKVER=8.0
IOSDIR=$DEVDIR/Platforms
IOSBIN=$DEVDIR/Toolchains/XcodeDefault.xctoolchain/usr/bin/

BUILD_DIR=$LUAJIT/build

rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR
rm *.a 1>/dev/null 2>/dev/null

#make CC="clang" HOST_CC="clang" HOST_CFLAGS="-arch i386" HOST_LDFLAGS="-arch i386" TARGET_SYS=iOS TARGET=i386 CROSS=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/usr/bin/ TARGET_FLAGS="-arch i386 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator7.1.sdk" TARGET_SYS=iOS TARGET_AR="/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ar rcus" clean
#make CC="clang" HOST_CC="clang" HOST_CFLAGS="-arch i386" HOST_LDFLAGS="-arch i386" TARGET_SYS=iOS TARGET=i386 CROSS=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/usr/bin/ TARGET_FLAGS="-arch i386 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator7.1.sdk" TARGET_SYS=iOS TARGET_AR="/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ar rcus" amalg
#mv $LUAJIT/src/libluajit.a $BUILD_DIR/libluajitx86.a

#make CC="clang" HOST_CC="clang" HOST_CFLAGS="-arch x86_64" HOST_LDFLAGS="-arch x86_64" TARGET_SYS=iOS TARGET=x86_64 CROSS=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/usr/bin/ TARGET_FLAGS="-arch x86_64 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator7.1.sdk" TARGET_SYS=iOS TARGET_AR="/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ar rcus" clean
#make CC="clang" HOST_CC="clang" HOST_CFLAGS="-arch x86_64" HOST_LDFLAGS="-arch x86_64" TARGET_SYS=iOS TARGET=x86_64 CROSS=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/usr/bin/ TARGET_FLAGS="-arch x86_64 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator7.1.sdk" TARGET_SYS=iOS TARGET_AR="/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ar rcus" amalg
#mv $LUAJIT/src/libluajit.a $BUILD_DIR/libluajitx64.a

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


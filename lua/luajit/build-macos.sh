BUILD_DIR=build

rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR
rm *.a 1>/dev/null 2>/dev/null

make clean
make amalg
mv src/libluajit.a $BUILD_DIR/libluajit32.a

#libtool -o ../../lib/macosx/libluajit.a $BUILD_DIR/libluajit32.a $BUILD_DIR/libluajit64.a 2> /dev/null
cp $BUILD_DIR/libluajit32.a ../../lib/macos/libluajit32.a

make clean
make amalg LUAJIT_ENABLE_GC64=1
mv src/libluajit.a $BUILD_DIR/libluajit64.a

cp $BUILD_DIR/libluajit64.a ../../lib/macos/libluajit.a

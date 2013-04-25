BUILD_DIR=build

rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR
rm *.a 1>/dev/null 2>/dev/null

make CC="gcc -m32" clean
make CC="gcc -m32" amalg
mv src/libluajit.a $BUILD_DIR/libluajit32.a

make clean
make amalg
mv src/libluajit.a $BUILD_DIR/libluajit64.a

libtool -o ../../lib/osx/libluajit.a $BUILD_DIR/*.a 2> /dev/null


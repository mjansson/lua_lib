REM call D:\dev\vs2017\vc\Auxiliary\Build\vcvarsall.bat amd64

cd src
call msvcbuild.bat gc64 static
copy /Y luajit.lib ..\..\..\lib\windows\x86-64\luajit.lib
copy /Y luajit.lib ..\..\..\..\..\..\lib\windows\luajit.lib

call msvcbuild.bat static
copy /Y luajit.lib ..\..\..\lib\windows\x86-64\luajit32.lib
copy /Y luajit.lib ..\..\..\..\..\..\lib\windows\luajit32.lib

cd ..

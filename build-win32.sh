#!/usr/bin/env bash

set -xe

# Switch to Bison 2.6 (2012-07-19)
cd /tools/bison-2.6
echo -e "\n\n>>> bison 2.6 <<<"
./configure && make && make install

echo -e "\n\n>>> rgbds 0.0.1 <<<"
export CURDIST=/dist/0.0.1
cd /work/rgbds-0.0.1
patch -p2 < ../patches/0.0.1-win32_string_funcs.diff
mkdir -p $CURDIST/bin
env PATH="/bin:$PATH" make
# "install" manually...
for i in rgb{asm,fix,lib,link}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.0.2 <<<"
export CURDIST=/dist/0.0.2
cd /work/rgbds-0.0.2
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.0.1-win32_string_funcs.diff
env PATH="/bin:$PATH" make
for i in rgb{asm,fix,lib,link}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.1.0 <<<"
export CURDIST=/dist/0.1.0
cd /work/rgbds-0.1.0
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.0.1-win32_string_funcs.diff
env PATH="/bin:$PATH" make
for i in rgb{asm,fix,lib,link}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.1.1 <<<"
export CURDIST=/dist/0.1.1
cd /work/rgbds-0.1.1
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.0.1-win32_string_funcs.diff
env PATH="/bin:$PATH" CFLAGS="$CFLAGS -D__progname=\\\"\\\"" make
for i in rgb{asm,fix,link}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.1.2 <<<"
export CURDIST=/dist/0.1.2
cd /work/rgbds-0.1.2
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.0.1-win32_string_funcs.diff
env PATH="/bin:$PATH" CFLAGS="$CFLAGS -D__progname=\\\"\\\"" make
for i in rgb{asm,fix,link}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.2.0 <<<"
export CURDIST=/dist/0.2.0
cd /work/rgbds-0.2.0
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.0.1-win32_string_funcs.diff
env PATH="/bin:$PATH" CFLAGS="$CFLAGS -D__progname=\\\"\\\"" make
for i in rgb{asm,fix,link}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.2.1 <<<"
export CURDIST=/dist/0.2.1
cd /work/rgbds-0.2.1
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.0.1-win32_string_funcs.diff
env PATH="/bin:$PATH" CFLAGS="$CFLAGS -D__progname=\\\"\\\"" make
for i in rgb{asm,fix,link}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.2.2 <<<"
export CURDIST=/dist/0.2.2
cd /work/rgbds-0.2.2
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.0.1-win32_string_funcs.diff
env PATH="/bin:$PATH" CFLAGS="$CFLAGS -D__progname=\\\"\\\"" make
for i in rgb{asm,fix,link}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.2.3 <<<"
export CURDIST=/dist/0.2.3
cd /work/rgbds-0.2.3
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.0.1-win32_string_funcs.diff
env PATH="/bin:$PATH" CFLAGS="$CFLAGS -D__progname=\\\"\\\"" make
for i in rgb{asm,fix,link}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.2.4 <<<"
export CURDIST=/dist/0.2.4
cd /work/rgbds-0.2.4
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.0.1-win32_string_funcs.diff
env PATH="/bin:$PATH" CFLAGS="$CFLAGS -D__progname=\\\"\\\"" make
for i in rgb{asm,fix,link}.exe; do cp $i $CURDIST/bin/$i; done


# Switch to zlib 1.2.10 (2017-01-02)
cd /tools/zlib-1.2.10
echo -e "\n\n>>> zlib 1.2.10 <<<"
env PATH="/bin:$PATH" sh -c './configure --static && make && make install'

# Switch to libpng 1.6.28 (2017-01-05)
cd /tools/libpng-1.6.28
echo -e "\n\n>>> libpng 1.6.28 <<<"
env PATH="/bin:$PATH" sh -c './configure --with-pic=yes --host=i686-w64-mingw32 && make && make install'

# rgbgfx = hell in a handbasket
echo -e "\n\n>>> rgbds 0.2.5 <<<"
export CURDIST=/dist/0.2.5
cd /work/rgbds-0.2.5
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.2.5_fix_multiple_definition.diff
patch -p2 < ../patches/0.2.5_fix_linking.diff
env PATH="/bin:$PATH" make
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

# Switch to flex 2.6.3 (2016-12-30)
cd /tools/flex-2.6.3
echo -e "\n\n>>> flex 2.6.3 <<<"
./configure && make && make install

echo -e "\n\n>>> rgbds 0.3.0 <<<"
export CURDIST=/dist/0.3.0
cd /work/rgbds-0.3.0
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.3.0_fix_multiple_definition.diff
patch -p2 < ../patches/0.2.5_fix_linking.diff
# warning: unknown conversion type character "F" in format
env PATH="/bin:$PATH" make WARNFLAGS="-Wall"
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.3.1 <<<"
export CURDIST=/dist/0.3.1
cd /work/rgbds-0.3.1
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.3.1_fix_multiple_definition.diff
env PATH="/bin:$PATH" make WARNFLAGS="-Wall"
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.3.2 <<<"
export CURDIST=/dist/0.3.2
cd /work/rgbds-0.3.2
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.3.1_fix_multiple_definition.diff
env PATH="/bin:$PATH" make WARNFLAGS="-Wall"
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.3.3 <<<"
export CURDIST=/dist/0.3.3
cd /work/rgbds-0.3.3
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.3.1_fix_multiple_definition.diff
env PATH="/bin:$PATH" make WARNFLAGS="-Wall"
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.3.4 <<<"
export CURDIST=/dist/0.3.4
cd /work/rgbds-0.3.4
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.3.4_fix_multiple_definition.diff
env PATH="/bin:$PATH" make WARNFLAGS="-Wall"
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.3.5 <<<"
export CURDIST=/dist/0.3.5
cd /work/rgbds-0.3.5
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.3.4_fix_multiple_definition.diff
env PATH="/bin:$PATH" make WARNFLAGS="-Wall"
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.3.6 <<<"
export CURDIST=/dist/0.3.6
cd /work/rgbds-0.3.6
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.3.4_fix_multiple_definition.diff
env PATH="/bin:$PATH" make WARNFLAGS="-Wall"
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.3.7 <<<"
export CURDIST=/dist/0.3.7
cd /work/rgbds-0.3.7
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.3.4_fix_multiple_definition.diff
env PATH="/bin:$PATH" make WARNFLAGS="-Wall"
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.3.8 <<<"
export CURDIST=/dist/0.3.8
cd /work/rgbds-0.3.8
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.3.4_fix_multiple_definition.diff
env PATH="/bin:$PATH" make WARNFLAGS="-Wall"
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.3.9 <<<"
export CURDIST=/dist/0.3.9
cd /work/rgbds-0.3.9
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.3.4_fix_multiple_definition.diff
env PATH="/bin:$PATH" make WARNFLAGS="-Wall"
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.3.10 <<<"
export CURDIST=/dist/0.3.10
cd /work/rgbds-0.3.10
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.3.4_fix_multiple_definition.diff
env PATH="/bin:$PATH" make WARNFLAGS="-Wall"
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.4.0 <<<"
export CURDIST=/dist/0.4.0
cd /work/rgbds-0.4.0
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.3.4_fix_multiple_definition.diff
patch -p2 < ../patches/0.4.0_fix_linking.diff
env PATH="/bin:$PATH" make
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.4.1 <<<"
export CURDIST=/dist/0.4.1
cd /work/rgbds-0.4.1
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.4.0_fix_linking.diff
env PATH="/bin:$PATH" make
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.4.2-pre <<<"
export CURDIST=/dist/0.4.2-pre
cd /work/rgbds-0.4.2-pre
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.4.0_fix_linking.diff
env PATH="/bin:$PATH" make
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.4.2 <<<"
export CURDIST=/dist/0.4.2
cd /work/rgbds-0.4.2
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.4.0_fix_linking.diff
env PATH="/bin:$PATH" make
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

# Bison version is officially too old
cd /tools/bison-2.6
make uninstall

# Switch to Bison 3.7.6 (2021-03-09)
cd /tools/bison-3.7.6
echo -e "\n\n>>> bison 3.7.6 <<<"
./configure && make && make install

echo -e "\n\n>>> rgbds 0.5.0-rc1 <<<"
export CURDIST=/dist/0.5.0-rc1
cd /work/rgbds-0.5.0-rc1
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.4.0_fix_linking.diff
env PATH="/bin:$PATH" make
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.5.0-rc2 <<<"
export CURDIST=/dist/0.5.0-rc2
cd /work/rgbds-0.5.0-rc2
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.4.0_fix_linking.diff
env PATH="/bin:$PATH" make
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.5.0-rcCar <<<"
export CURDIST=/dist/0.5.0-rcCar
cd /work/rgbds-0.5.0-rcCar
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.4.0_fix_linking.diff
env PATH="/bin:$PATH" make
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.5.0 <<<"
export CURDIST=/dist/0.5.0
cd /work/rgbds-0.5.0
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.4.0_fix_linking.diff
env PATH="/bin:$PATH" make
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.5.1 <<<"
export CURDIST=/dist/0.5.1
cd /work/rgbds-0.5.1
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.4.0_fix_linking.diff
env PATH="/bin:$PATH" make
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.5.2 <<<"
export CURDIST=/dist/0.5.2
cd /work/rgbds-0.5.2
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.4.0_fix_linking.diff
env PATH="/bin:$PATH" make
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

# enter stage left, g++
echo -e "\n\n>>> rgbds 0.6.0-rc1 <<<"
export CURDIST=/dist/0.6.0-rc1
cd /work/rgbds-0.6.0-rc1
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.4.0_fix_linking.diff
env PATH="/bin:$PATH" make
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.6.0-rc2 <<<"
export CURDIST=/dist/0.6.0-rc2
cd /work/rgbds-0.6.0-rc2
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.4.0_fix_linking.diff
env PATH="/bin:$PATH" make
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

# Absolute power move
echo -e "\n\n>>> rgbds 0.6.0-welease-cnyandidayte <<<"
export CURDIST=/dist/0.6.0-welease-cnyandidayte
cd /work/rgbds-0.6.0-welease-cnyandidayte
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.4.0_fix_linking.diff
env PATH="/bin:$PATH" make
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.6.0 <<<"
export CURDIST=/dist/0.6.0
cd /work/rgbds-0.6.0
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.4.0_fix_linking.diff
env PATH="/bin:$PATH" make
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.6.1 <<<"
export CURDIST=/dist/0.6.1
cd /work/rgbds-0.6.1
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.4.0_fix_linking.diff
env PATH="/bin:$PATH" make
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

# WELCOME TO NEXT LEVEL IN C++20 WORLD
echo -e "\n\n>>> rgbds 0.7.0 <<<"
export CURDIST=/dist/0.7.0
cd /work/rgbds-0.7.0
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.4.0_fix_linking.diff
env PATH="/bin:$PATH" make
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.8.0 <<<"
export CURDIST=/dist/0.8.0
cd /work/rgbds-0.8.0
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.4.0_fix_linking.diff
env PATH="/bin:$PATH" make
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done

echo -e "\n\n>>> rgbds 0.9.0-rc1 <<<"
export CURDIST=/dist/0.9.0-rc1
cd /work/rgbds-0.9.0-rc1
mkdir -p $CURDIST/bin
patch -p2 < ../patches/0.4.0_fix_linking.diff
env PATH="/bin:$PATH" make
for i in rgb{asm,fix,link,gfx}.exe; do cp $i $CURDIST/bin/$i; done
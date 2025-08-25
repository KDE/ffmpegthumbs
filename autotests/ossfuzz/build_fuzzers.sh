#!/bin/bash -eu
#
# SPDX-FileCopyrightText: 2025 Azhar Momin <azhar.momin@kdemail.net>
# SPDX-License-Identifier: LGPL-2.0-or-later
#
# This script must be run after kio-extras/thumbnail/autotests/ossfuzz/build_fuzzers.sh

export PATH="$WORK/bin:$WORK/libexec:$PATH"
export PKG_CONFIG="$(which pkg-config) --static"
export PKG_CONFIG_PATH="$WORK/lib/pkgconfig:$WORK/share/pkgconfig:$WORK/lib/x86_64-linux-gnu/pkgconfig"

# For FFMpegThumbnailer
cd $SRC/ffmpeg
if [ "$SANITIZER" = "memory" ]; then
  disable_asm="--disable-inline-asm --disable-x86asm"
else
  disable_asm=
fi
./configure \
  --cc="$CC" \
  --cxx="$CXX" \
  --ld="$CXX $CXXFLAGS -std=c++11" \
  --prefix=$WORK \
  $disable_asm \
  --enable-static \
  --disable-shared \
  --disable-doc \
  --disable-everything \
  --disable-programs \
  --disable-avdevice \
  --disable-swresample \
  --enable-avfilter \
  --enable-swscale \
  --enable-avformat \
  --enable-avcodec \
  --enable-avutil
make install -j$(nproc)

# Build FFMpegThumbs
cd $SRC/ffmpegthumbs
cmake -B build -G Ninja \
    -DCMAKE_PREFIX_PATH=$WORK \
    -DCMAKE_INSTALL_PREFIX=$WORK \
    -DBUILD_FUZZERS=ON \
    -DFUZZERS_USE_QT_MINIMAL_INTEGRATION_PLUGIN=ON \
    -DBUILD_SHARED_LIBS=OFF
ninja -C build -j$(nproc)

EXTENSIONS="ffmpegthumbs_fuzzer ogg wma wmv mp4 mkv webm avi mov flv m3u8"

echo "$EXTENSIONS" | while read fuzzer_name extensions; do
    # copy the fuzzer binary
    cp build/bin/fuzzers/$fuzzer_name $OUT

    # create seed corpus
    for extension in $extensions; do
        find . -name "*.$extension" -exec zip -q "$OUT/${fuzzer_name}_seed_corpus.zip" {} +
    done
done

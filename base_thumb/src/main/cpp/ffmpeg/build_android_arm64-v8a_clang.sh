#!/bin/bash

export NDK=~/Library/Android/sdk/ndk/20.1.5948944 #这里配置先你的 NDK 路径
TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/darwin-x86_64 #这里是MacOS的路径，有需要可改linux-x86_64


build_android() {

	./configure \
		--prefix=$PREFIX \
		--enable-neon  \
		--disable-asm \
		--enable-hwaccels  \
		--enable-gpl   \
		--disable-postproc \
		--disable-debug \
		--enable-small \
		--enable-jni \
		--enable-mediacodec \
		--enable-decoder=h264_mediacodec \
		--enable-static \
		--enable-shared \
		--disable-doc \
		--enable-ffmpeg \
		--disable-ffplay \
		--disable-ffprobe \
		--disable-avdevice \
		--disable-doc \
		--disable-symver \
		--cross-prefix=$CROSS_PREFIX \
		--target-os=android \
		--arch=$ARCH \
		--cpu=$CPU \
		--cc=$CC \
		--cxx=$CXX \
		--ld=$CC \
		--nm=$NM \
		--strip=$STRIP \
		--enable-cross-compile \
		--sysroot=$SYSROOT \
		--extra-cflags="-Os -fpic $OPTIMIZE_CFLAGS" \
		--extra-ldflags="$ADDI_LDFLAGS"

	make clean
	make -j16
	make install

	echo "============================ build android arm64-v8a success =========================="

}

#arm64-v8a
ARCH=aarch64
CPU=armv8-a
API=21
CC=$TOOLCHAIN/bin/aarch64-linux-android$API-clang
CXX=$TOOLCHAIN/bin/aarch64-linux-android$API-clang++
NM=$TOOLCHAIN/bin/aarch64-linux-android-nm
STRIP=$TOOLCHAIN/bin/aarch64-linux-android-strip
SYSROOT=$TOOLCHAIN/sysroot
CROSS_PREFIX=$TOOLCHAIN/bin/aarch64-linux-android-
PREFIX=$(pwd)/android/$CPU
OPTIMIZE_CFLAGS="-march=$CPU"
ADDI_LDFLAGS=""

build_android


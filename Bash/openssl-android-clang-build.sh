#! /bin/sh

# This script build openssl 1.1.x for android using toolchain 4.9 and clang under Linux
# Before start this environment variables must set:
# $ANDROID_NDK_ROOT = path to root of your android NDK
# $ANDROID_TARGET_ARCH = set to target architecture (arm64-v8a, armeabi-v7a, x86)
# 
# Example:
# export ANDROID_NDK_ROOT="/home/foo/android-ndk"
# export ANDROID_TARGET_ARCH="armeabi-v7a"
# ./openssl-android-clang-build.sh

TOOLCHAIN_VERSION=4.9
HOST_ARCH=linux-x86_64
case "$ANDROID_TARGET_ARCH" in
    arm64-v8a)
        API_VERSION=21
        ARCH_ID=android-arm64
        TOOLCHAIN=aarch64-linux-android-$TOOLCHAIN_VERSION
        ;;
    armeabi-v7a)
        API_VERSION=16
        ARCH_ID=android-arm
        TOOLCHAIN=arm-linux-android-$TOOLCHAIN_VERSION
        ;;
    x86)
        API_VERSION=16
        ARCH_ID=android-x86
        TOOLCHAIN=x86-$TOOLCHAIN_VERSION
        ;;
    *)
        echo "Unsupported ANDROID_TARGET_ARCH: $ANDROID_TARGET_ARCH"
        exit 1
        ;;
esac

export ANDROID_NDK="$ANDROID_NDK_ROOT"
export PATH=$ANDROID_NDK/toolchains/llvm/prebuilt/$HOST_ARCH/bin/:$ANDROID_NDK/toolchains/$TOOLCHAIN/prebuilt/$HOST_ARCH/bin:$PATH
./Configure $ARCH_ID shared no-ssl3 -D__ANDROID_API__=$API_VERSION
make SHLIB_VERSION_NUMBER= SHLIB_EXT=.so build_libs

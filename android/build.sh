#!/bin/bash

# Based on http://www.hanshq.net/command-line-android.html
#
# This expects $JAVA_HOME to be set and the JDK tools to be in $PATH.
# For example:
# $ export JAVA_HOME="${HOME}/jdk1.8.0_112"
# $ export PATH="${JAVA_HOME}/bin:${PATH}"

set -eu

SDK="${HOME}/android-sdk-linux"
NDK="${HOME}/android-ndk-r13b"

BUILD_TOOLS="${SDK}/build-tools/25.0.0"
PLATFORM="${SDK}/platforms/android-16"

ARM_TOOLCHAIN="${NDK}/toolchains/arm-linux-androideabi-4.9/prebuilt/"
ARM_TOOLCHAIN+="linux-x86_64/bin/arm-linux-androideabi-gcc"

ARM64_TOOLCHAIN="${NDK}/toolchains/aarch64-linux-android-4.9/prebuilt/"
ARM64_TOOLCHAIN+="linux-x86_64/bin/aarch64-linux-android-gcc"

X86_TOOLCHAIN="${NDK}/toolchains/x86-4.9/prebuilt/"
X86_TOOLCHAIN+="linux-x86_64/bin/i686-linux-android-gcc"

rm -rf build
mkdir -p build/gen build/obj
mkdir -p build/apk/lib/armeabi-v7a build/apk/lib/arm64-v8a build/apk/lib/x86

"${BUILD_TOOLS}/aapt" package -f -m -J build/gen/ -S res \
    -M AndroidManifest.xml -I "${PLATFORM}/android.jar"

javac -source 1.7 -target 1.7 -bootclasspath "${JAVA_HOME}/jre/lib/rt.jar" \
    -classpath "${PLATFORM}/android.jar" -d build/obj \
    build/gen/net/hanshq/othello/R.java java/net/hanshq/othello/*.java

"${BUILD_TOOLS}/dx" --dex --output=build/apk/classes.dex build/obj/

"${ARM_TOOLCHAIN}" --sysroot="${NDK}/platforms/android-16/arch-arm" \
    -march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=softfp -Wl,--fix-cortex-a8 \
    -fPIC -shared -o build/apk/lib/armeabi-v7a/libothello.so \
    -Wall -Wextra -O3 -DNDEBUG -std=c99 -pedantic -I.. \
    jni/othello_board.c ../othello.c

"${ARM64_TOOLCHAIN}" --sysroot="${NDK}/platforms/android-21/arch-arm64" \
    -fPIC -shared -o build/apk/lib/arm64-v8a/libothello.so \
    -Wall -Wextra -O3 -DNDEBUG -std=c99 -pedantic -I.. \
    jni/othello_board.c ../othello.c

"${X86_TOOLCHAIN}" --sysroot="${NDK}/platforms/android-16/arch-x86" \
    -fPIC -shared -o build/apk/lib/x86/libothello.so \
    -Wall -Wextra -O3 -DNDEBUG -std=c99 -pedantic -I.. \
    jni/othello_board.c ../othello.c

"${BUILD_TOOLS}/aapt" package -f -M AndroidManifest.xml -S res/ \
    -I "${PLATFORM}/android.jar" \
    -F build/Othello.unsigned.apk build/apk/

"${BUILD_TOOLS}/zipalign" -f -p 4 \
    build/Othello.unsigned.apk build/Othello.aligned.apk

if [ ! -f keystore.jks ]; then
        keytool -genkeypair -keystore keystore.jks -alias androidkey \
            -validity 10000 -keyalg RSA -keysize 2048 \
            -storepass android -keypass android
fi

if [ "${@-1}" = "release" ]; then
        echo Signing with the upload key.
        "${BUILD_TOOLS}/apksigner" sign --ks release_keystore.jks \
            --ks-key-alias uploadkey --out build/Othello.apk \
            build/Othello.aligned.apk
else
        "${BUILD_TOOLS}/apksigner" sign --ks keystore.jks \
            --ks-key-alias androidkey --ks-pass pass:android \
            --key-pass pass:android --out build/Othello.apk \
            build/Othello.aligned.apk
fi


# To run:
# $ export PATH="${PATH}:${SDK}/platform-tools"
# $ adb install -r build/Othello.apk
# $ adb shell am start -n net.hanshq.othello/.OthelloActivity

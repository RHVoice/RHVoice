# Compiling for Android

## Required tools

To compile RHVoice the following programs must be installed on your system:

* [GCC](https://gcc.gnu.org)
* [Pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/)
* [SCons](https://www.scons.org)

## Get sources

1. One line
    ```bash
    git clone --recursive https://github.com/RHVoice/RHVoice.git
    ```
2. Step by step
    ```bash
    git clone https://github.com/RHVoice/RHVoice.git
    cd RHVoice
    git submodule update --init
    ```

## Compilation

These compilation instructions are written for use on Linux to compile the apk for Android.
To start compilation, execute the command:

```bash
cd RHVoice/src/android
```
This is the directory in which the source code for the Android application is located.

```bash
./gradlew assemble
```

After the previous command has finished, the app will have successfully compiled.
```bash
cd RHVoice-core/build/outputs/apk/stable/release
```
This folder should contain RHVoice-core-stable-release-unsigned.apk

## Installation
To install the application [remote sources](https://www.maketecheasier.com/install-apps-from-unknown-sources-android/) or [ADB](https://developer.android.com/tools/adb) must be enabled.
Either install the application by transferring the APK to the device, or performing a streamed install via ADB.

To install RHVoice using ADB execute the following command:

```bash
adb install RHVoice-core-stable-release-unsigned.apk
```

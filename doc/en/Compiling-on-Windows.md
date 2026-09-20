# Compiling on Windows

## Required tools

To compile RHVoice on Windows, the following tools are required:

* [Python](https://www.python.org)
* [Wix toolset (3.14, strongly required)](https://github.com/wixtoolset/wix3/releases)

* [Nsis](https://nsis.sourceforge.io/Download)

Additional dependencies should be installed using the following command:

```bash
pip install scons lxml
```

## Getting the source code

After installing the dependencies, we can proceed to compiling.

To clone the source, we can go in two possible ways:

1. One line
    ```bash
    git clone --recursive https://github.com/RHVoice/RHVoice.git
    ```

2. Step by step:
    ```bash
    git clone https://github.com/RHVoice/RHVoice.git
    cd RHVoice
    git submodule update --init
    ```

## Compilation

To start compilation, execute the command:

```bash
scons
```

You can change some compilation options. For more information, run the command:

```bash
scons -h
```

The compiled packages are located in `RHVoice\build\packages\windows` directory.

## Building for Windows on ARM

By default the x86 and x64 libraries are built. Two more targets are available, both disabled by default:

* `enable_arm64` builds native ARM64 libraries. Only native ARM64 programs can load them, for example through SAPI5.
* `enable_arm64ec` builds [ARM64EC](https://learn.microsoft.com/en-us/windows/arm/arm64ec) libraries. They use the x64 compatible ABI, so emulated x64 processes can load them, but they run natively. The NVDA add-on needs these, because NVDA itself is an x64 program.

Both require the *MSVC ARM64/ARM64EC build tools* component of Visual Studio, which is not installed by default.

For example, to build the ARM64EC libraries only:

```bash
scons enable_x86=no enable_x64=no enable_arm64ec=yes
```

When `enable_arm64ec` is set, the ARM64EC library is added to the NVDA add-on. The driver prefers it on ARM64 machines, and falls back to the x64 library everywhere else.

## Further reading for language maintainers

[Understanding about MSI package versioning](https://github.com/rhvoice/msi)

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

## Further reading for language maintainers

[Understanding about MSI package versioning](https://github.com/rhvoice/msi)

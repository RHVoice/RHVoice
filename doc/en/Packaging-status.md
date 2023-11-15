# Packaging status

You can view Repology [project page](https://repology.org/project/rhvoice/versions).

Please note that versions of RHVoice in distribution repositories may lag (sometimes significantly lag) behind the version in the RHVoice repository.

RHVoice is available in the following Linux distributions (in alphabet order):

## ALT Linux

Can be installed from [Sisyphus](https://packages.altlinux.org/).

Installation commands:

```bash
sudo apt-get install RHVoice
```

## Arch Linux

Can be installed from [AUR](https://aur.archlinux.org/):

* [Latest version](https://aur.archlinux.org/packages/rhvoice/)
* [Development version](https://aur.archlinux.org/packages/rhvoice-git/)

Example installation commands:

```bash
sudo pacman -S rhvoice
sudo pacman -S rhvoice-language-english
sudo pacman -S rhvoice-voice-alan
```

## Debian

Packages are in the non-free components repository.

Example installation commands:

```bash
sudo apt install speech-dispatcher-rhvoice
sudo apt install rhvoice-english
```

## Fedora

Can be found in [lostmemories/RHVoice](https://copr.fedorainfracloud.org/coprs/lostmemories/)
Fedora Copr.

Example installation commands:

```bash
sudo dnf copr enable lostmemories/RHVoice
sudo dnf install rhvoice
sudo dnf install rhvoice-speech-dispatcher-plugin
```

## Manjaro

Example installation commands:

```bash
pamac install rhvoice
pamac install rhvoice-language-english
pamac install rhvoice-voice-alan
```

## RED OS

Can be installed from the repository starting with version 7.3 (update 7.3.3).

Installation commands:

```bash
sudo dnf install RHVoice
```

## Slint

RHVoice installed in Slint distribution out of the box.

## Solus

Installation commands:

```bash
sudo eopkg upgrade
sudo eopkg install rhvoice
```

## Ubuntu

Can be found in [linvinus/rhvoice](https://launchpad.net/~linvinus/+archive/ubuntu/rhvoice/)
PPA.

Installation commands:

```bash
sudo add-apt-repository ppa:linvinus/rhvoice
sudo apt-get update
sudo apt-get install rhvoice
```

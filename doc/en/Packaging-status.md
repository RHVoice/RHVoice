# Packaging status

## RHVoice in Snap Store

An alpha version of the RHVoice snap is now available in the Snap Store. Please help us test it and fix bugs.

The snap only installs the RHVoice TTS engine itself, including the module connecting Speech Dispatcher to RHVoice. Voices need to be installed via the newly implemented command-line voice manager.

Install the snap:

```bash
sudo snap install --edge rhvoice
```

The voice manager must be run as root.

List available voices:

```bash
sudo rhvoice.vm -a
```

Install a voice, for example, Alan:

```bash
sudo rhvoice.vm -i alan
```

List installed voices:

```bash
sudo rhvoice.vm -l
```

Test if it speaks:

```bash
echo hello|rhvoice.test
```

Unfortunately, we are not aware of any way we could register RHVoice with Speech Dispatcher automatically. You will have to manually either add a symlink to the module or define the module in Speech Dispatcher's configuration file. We aren't sure if the absolute path to the module will remain the same on all the systems supporting snap packages. On Ubuntu the path is /snap/rhvoice/current/bin/sd_rhvoice.

## Packages in Linux distributions

You can view Repology [project page](https://repology.org/project/rhvoice/versions).

Please note that versions of RHVoice in distribution repositories may lag (sometimes significantly lag) behind the version in the RHVoice repository.

RHVoice is available in the following Linux distributions (in alphabet order):

### ALT Linux

Can be installed from [Sisyphus](https://packages.altlinux.org/).

Installation commands:

```bash
sudo apt-get install RHVoice
```

### Arch Linux

Can be installed from [AUR](https://aur.archlinux.org/):

* [Latest version](https://aur.archlinux.org/packages/rhvoice/)
* [Development version](https://aur.archlinux.org/packages/rhvoice-git/)

Example installation commands:

```bash
sudo pacman -S rhvoice
sudo pacman -S rhvoice-language-english
sudo pacman -S rhvoice-voice-alan
```

### Debian

Packages are in the non-free components repository.

Example installation commands:

```bash
sudo apt install speech-dispatcher-rhvoice
sudo apt install rhvoice-english
```

### Fedora

Can be found in [lostmemories/RHVoice](https://copr.fedorainfracloud.org/coprs/lostmemories/)
Fedora Copr.

Example installation commands:

```bash
sudo dnf copr enable lostmemories/RHVoice
sudo dnf install rhvoice
sudo dnf install rhvoice-speech-dispatcher-plugin
```

### Gentoo

Can be installed from [GURU](https://wiki.gentoo.org/wiki/GURU).

You might want to enable nonfree (CC-BY-NC-ND 4.0) voices:

```bash
echo "app-accessibility/rhvoice redistributable" | sudo tee -a /etc/portage/package.use
```

Installation commands:

```bash
sudo eselect repository enable guru
sudo emaint sync -r guru
sudo emerge app-accessibility/rhvoice
```

### Manjaro

Example installation commands:

```bash
pamac install rhvoice
pamac install rhvoice-language-english
pamac install rhvoice-voice-alan
```

### RED OS

Can be installed from the repository starting with version 7.3 (update 7.3.3).

Installation commands:

```bash
sudo dnf install RHVoice
```

### Slint

RHVoice installed in Slint distribution out of the box.

### Solus

Installation commands:

```bash
sudo eopkg upgrade
sudo eopkg install rhvoice
```

### Ubuntu

Can be found in [linvinus/rhvoice](https://launchpad.net/~linvinus/+archive/ubuntu/rhvoice/)
PPA.

Installation commands:

```bash
sudo add-apt-repository ppa:linvinus/rhvoice
sudo apt-get update
sudo apt-get install rhvoice
```

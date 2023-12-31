# Стан пакетів

Ви можете переглянути [сторінку проєкту](https://repology.org/project/rhvoice/versions) на сервісі Repology.

Зверніть увагу, що версії RHVoice у репозиторіях дистрибутивів можуть відставати (іноді значно відставати) від версії у репозиторії RHVoice.

RHVoice доступний у нижченаведених дистрибутивах Linux (в алфавітному порядку):

## ALT Linux

Можна встановити з [Sisyphus](https://packages.altlinux.org/).

Команди встановлення:

```bash
sudo apt-get install RHVoice
```

## Arch Linux

Можна встановити з [AUR](https://aur.archlinux.org/):

* [Остання версія](https://aur.archlinux.org/packages/rhvoice/)
* [Версія в розробці](https://aur.archlinux.org/packages/rhvoice-git/)

Приклад команд встановлення:

```bash
sudo pacman -S rhvoice
sudo pacman -S rhvoice-language-ukrainian
sudo pacman -S rhvoice-voice-anatol
```

## Debian

Пакети перебувають у репозиторії невільних компонентів (non-free).

Приклад команд встановлення:

```bash
sudo apt install speech-dispatcher-rhvoice
sudo apt install rhvoice-ukrainian
```

## Fedora

Можна знайти в [lostmemories/RHVoice](https://copr.fedorainfracloud.org/coprs/lostmemories/)
Fedora Copr.

Приклад команд встановлення:

```bash
sudo dnf copr enable lostmemories/RHVoice
sudo dnf install rhvoice
sudo dnf install rhvoice-speech-dispatcher-plugin
sudo dnf install rhvoice-ukrainian
```

## Manjaro

Приклад команд встановлення:

```bash
pamac install rhvoice
pamac install rhvoice-language-ukrainian
pamac install rhvoice-voice-anatol
```

## RED OS

Можна встановити з репозиторію, починаючи з версії 7.3 (оновлення 7.3.3).

Команди встановлення:

```bash
sudo dnf install RHVoice
```

## Slint

RHVoice встановлюється у дистрибутив Slint з коробки.

## Solus

Команди встановлення:

```bash
sudo eopkg upgrade
sudo eopkg install rhvoice
```

## Ubuntu

Можна знайти в [linvinus/rhvoice](https://launchpad.net/~linvinus/+archive/ubuntu/rhvoice/)
PPA.

Команди встановлення:

```bash
sudo add-apt-repository ppa:linvinus/rhvoice
sudo apt-get update
sudo apt-get install rhvoice
```

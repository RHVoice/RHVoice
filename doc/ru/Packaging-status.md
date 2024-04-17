# Состояние пакетов

Вы можете просмотреть [страницу проекта](https://repology.org/project/rhvoice/versions) на сервисе Repology.

Обратите внимание, что версии RHVoice в репозиториях дистрибутивов могут отставать (иногда значительно отставать) от версии в репозитории RHVoice.

RHVoice доступен в следующих дистрибутивах Linux (в алфавитном порядке):

## ALT Linux

Можно установить из [Sisyphus](https://packages.altlinux.org/).

Команды установки:

```bash
sudo apt-get install RHVoice
```

## Arch Linux

Можно установить из [AUR](https://aur.archlinux.org/):

* [Последняя версия](https://aur.archlinux.org/packages/rhvoice/)
* [Разрабатываемая версия](https://aur.archlinux.org/packages/rhvoice-git/)

Пример команд установки:

```bash
sudo pacman -S rhvoice
sudo pacman -S rhvoice-language-russian
sudo pacman -S rhvoice-voice-aleksandr
```

## Debian

Пакеты находятся в репозитории несвободных компонентов (non-free).

Пример команд установки:

```bash
sudo apt install speech-dispatcher-rhvoice
sudo apt install rhvoice-russian
```

## Fedora

Можно найти в [lostmemories/RHVoice](https://copr.fedorainfracloud.org/coprs/lostmemories/)
Fedora Copr.

Пример команд установки:

```bash
sudo dnf copr enable lostmemories/RHVoice
sudo dnf install rhvoice
sudo dnf install rhvoice-speech-dispatcher-plugin
sudo dnf install rhvoice-russian
```

## Gentoo

Можно найти в [GURU](https://wiki.gentoo.org/wiki/GURU).

Несвободные (CC-BY-NC-ND 4.0) голоса не устанавливаются по умолчанию, выполните
следующую команду, чтобы их включить:

```bash
echo "app-accessibility/rhvoice redistributable" | sudo tee -a /etc/portage/package.use
```

Команды установки:

```bash
sudo eselect repository enable guru
sudo emaint sync -r guru
sudo emerge app-accessibility/rhvoice
```

## Manjaro

Пример команд установки:

```bash
pamac install rhvoice
pamac install rhvoice-language-russian
pamac install rhvoice-voice-aleksandr
```

## RED OS

Можно установить из репозитория, начиная с версии 7.3 (обновление 7.3.3).

Команды установки:

```bash
sudo dnf install RHVoice
```

## Slint

RHVoice устанавливается в дистрибутив Slint из коробки.

## Solus

Команды установки:

```bash
sudo eopkg upgrade
sudo eopkg install rhvoice
```

## Ubuntu

Можно найти в [linvinus/rhvoice](https://launchpad.net/~linvinus/+archive/ubuntu/rhvoice/)
PPA.

Команды установки:

```bash
sudo add-apt-repository ppa:linvinus/rhvoice
sudo apt-get update
sudo apt-get install rhvoice
```

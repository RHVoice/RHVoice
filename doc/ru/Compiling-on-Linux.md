# компиляция на Linux

## Необходимые инструменты

Для компиляции RHVoice в вашей системе должны быть установлены
следующие программы:

* [GCC](https://gcc.gnu.org)
* [Pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/)
* [SCons](https://www.scons.org)

## Дополнительные зависимости

Перед началом компиляции RHVoice убедитесь, что на вашей машине
установлен по крайней мере один из следующих компонентов,
обеспечивающих вывод звука:

* [PulseAudio](https://www.freedesktop.org/wiki/Software/PulseAudio/)
* [Libao](https://www.xiph.org/ao/)
* [PortAudio](http://www.portaudio.com) V19
* [Speech Dispatcher](https://freebsoft.org/speechd)

Обратите внимание, что многие дистрибутивы выделяют в отдельные пакеты
те файлы, которые необходимы только при компиляции, а не при
выполнении, программ, использующих ту или иную библиотеку. В вашей
системе должны быть установлены пакеты обоих типов.

## Получение исходников

1. Одной строкой
    ```bash
    git clone --recursive https://github.com/RHVoice/RHVoice.git
    ```
2. По шагам
    ```bash
    git clone https://github.com/RHVoice/RHVoice.git
    cd RHVoice
    git submodule update --init
    ```

## Компиляция

Для запуска компиляции выполните команду:

```bash
scons
```

Вы можете изменить параметры сборки, например, передать дополнительные
опции компилятору. Для получения более подробной информации
воспользуйтесь командой:

```bash
scons -h
```

## Установка

Для установки RHVoice выполните следующую команду:

```bash
# scons install
```

Теперь вы можете проверить работоспособность синтезатора:

```bash
echo test|RHVoice-test
```

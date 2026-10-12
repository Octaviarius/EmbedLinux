# Кросс-разработка — черновик

## Содержание

- [Статус материала](#статус-материала)
- [Что нужно в принципе](#что-нужно-в-принципе)
- [Подготовка платы и SSH](#подготовка-платы-и-ssh)
- [Кросс-компиляция](#кросс-компиляция)

## Статус материала

Материал выделен из прежнего общего туториала и сохранён для следующего этапа. Его предстоит согласовать с [рабочим окружением](../../environment/README.md) и проверить отдельно. Сейчас основной материал — [нативная разработка](../native/README.md).

Практическая платформа — [rpi0-2w](../../hardware/rpi0-2w/README.md), целевое пользовательское окружение — `arm64`. Сборка выполняется на компьютере в Linux/WSL, программа запускается на плате.

## Что нужно в принципе

- Raspberry Pi с ОС, питанием и подключением к сети.
- SSH-доступ к плате.
- Компилятор C/C++ и система сборки CMake.
- VS Code с расширением **C/C++** (`ms-vscode.cpptools`).
- Для отладки — GDB; при кросс-компиляции дополнительно нужен `gdbserver` на плате.
- Для кросс-компиляции — компилятор под архитектуру целевых приложений и **sysroot**: каталог заголовков и библиотек целевой системы.

## Подготовка платы и SSH

Установите актуальную стабильную Raspberry Pi OS. В Raspberry Pi Imager задайте пользователя, сеть и включите SSH. Далее используются примеры пользователя `dev` и адреса `192.168.1.50`; замените их своими.

Проверьте архитектуру приложений и ядра **на плате**:

```bash
dpkg --print-architecture
uname -m
```

Для приведённой кросс-сборки `dpkg --print-architecture` должен показать `arm64`. Эта команда показывает основную архитектуру пакетов Raspberry Pi OS; по ней выбираем компилятор для обычных приложений. `uname -m` показывает архитектуру работающего ядра: для AArch64 это `aarch64`.

Процессор определяет, какие архитектуры он поддерживает, но на Raspberry Pi с 64-битным процессором могут использоваться 32-битные приложения и библиотеки. Даже 64-битное ядро может работать с 32-битным пользовательским окружением, поэтому одного `uname -m` для выбора toolchain недостаточно. См. [документацию ядра Raspberry Pi](https://www.raspberrypi.com/documentation/computers/linux_kernel.html) и [описание dpkg](https://manpages.debian.org/trixie/dpkg/dpkg.1.en.html).

Если `dpkg --print-architecture` показывает `armhf`, используйте toolchain для 32-битного ARM и sysroot этой системы. Проверьте также поддерживаемый набор инструкций конкретной модели: произвольный `arm-linux-gnueabihf` toolchain не обязательно подходит старым Raspberry Pi с ARMv6. Нативная сборка использует установленные на плате инструменты и не требует приведённого AArch64 toolchain.

Подключение по IP и имени хоста, ключ и алиас настройте по [отдельному SSH-туториалу](../ssh/README.md). Для примеров ниже должен работать `ssh pi-dev` из WSL.

## Кросс-компиляция

### Подготовьте Linux или WSL

На Windows установите WSL с актуальной поддерживаемой Ubuntu и расширение **WSL** (`ms-vscode-remote.remote-wsl`). Откройте проект из WSL командой `code .`; окно VS Code должно показывать контекст WSL. Установите **C/C++** в это окружение. См. [работу в WSL](https://code.visualstudio.com/docs/remote/wsl).

Далее команды компьютера выполняются в **Bash Linux/WSL**, не в PowerShell. Проект и sysroot удобно хранить в домашнем каталоге Linux.

На компьютере (Ubuntu/Debian):

```bash
sudo apt update
sudo apt install cmake ninja-build gcc-aarch64-linux-gnu g++-aarch64-linux-gnu \
    gdb-multiarch rsync openssh-client file
aarch64-linux-gnu-gcc -dumpmachine
```

Ожидается `aarch64-linux-gnu`. Пакеты устанавливают версии из репозитория выбранного дистрибутива. Номер GCC не обязан совпадать с номером на плате; важны поддерживаемые процессором инструкции, совместимость ABI и библиотек, используемых при запуске. Для повторяемых сборок зафиксируйте версии инструментов, пакетов целевой ОС и соответствующий sysroot.

На Raspberry Pi:

```bash
sudo apt update
sudo apt install build-essential gdbserver rsync
```

Дополнительные библиотеки устанавливайте на плате вместе с пакетами разработки (`*-dev`), затем обновляйте sysroot. Например, заголовки libgpiod поставляет `libgpiod-dev`; API зависит от версии библиотеки.

### Скопируйте sysroot с платы

На компьютере:

```bash
export RPI_SYSROOT="$HOME/rpi/sysroot"
mkdir -p "$RPI_SYSROOT/usr" "$RPI_SYSROOT/lib"
rsync -rltzL --delete pi-dev:/usr/include/ "$RPI_SYSROOT/usr/include/"
rsync -rltzL --delete pi-dev:/usr/lib/ "$RPI_SYSROOT/usr/lib/"
rsync -rltzL --delete pi-dev:/lib/ "$RPI_SYSROOT/lib/"
```

`-L` заменяет символические ссылки копиями файлов или каталогов, на которые они указывают. Это предотвращает обращение по скопированным абсолютным ссылкам к файлам компьютера. Копирование `/lib/` сохраняет в sysroot расположение загрузчика и библиотек из этого каталога, в том числе когда `/lib` на плате является ссылкой на `/usr/lib`. Содержимое файлов, включая пути в скриптах линковщика, эта опция не меняет.

После установки, обновления или удаления библиотек повторите команды. По умолчанию rsync определяет необходимость обновления по размеру и времени изменения файла. `--delete` удаляет из соответствующего каталога sysroot файлы, которых больше нет в исходном каталоге платы; собственные файлы храните отдельно от этих каталогов. После смены ОС создайте новый sysroot в отдельном каталоге. См. [описание rsync](https://www.samba.org/ftp/rsync/rsync.1.html).

Чтобы сначала посмотреть план копирования и удаления, добавьте к нужной команде `--dry-run --itemize-changes`. Например:

```bash
rsync -rltzL --delete --dry-run --itemize-changes \
    pi-dev:/usr/include/ "$RPI_SYSROOT/usr/include/"
```

Этот вызов не изменяет файлы. Для выполнения синхронизации используйте команды выше без `--dry-run`.

Sysroot содержит файлы целевой системы, но GCC может дополнительно использовать библиотеки своего комплекта, например `libgcc` и libstdc++. Сообщения `GLIBC_* not found` или `GLIBCXX_* not found` означают, что загруженная при запуске библиотека не предоставляет требуемую версию символов. Проверьте, какие библиотеки использованы при сборке и какие загружаются на плате; затем согласуйте toolchain и библиотеки целевой системы.

### Создайте проект и укажите свои пути

На компьютере в Linux/WSL скопируйте **содержимое** `tutor/cross/example`, включая `.vscode`, в отдельную папку. Из корня репозитория EmbedLinux:

```bash
mkdir -p "$HOME/projects/hello-cross"
cp -R tutor/cross/example/. "$HOME/projects/hello-cross/"
cd "$HOME/projects/hello-cross"
code .
```

В открытом окне Linux/WSL откройте **файл проекта `.vscode/settings.json`**, например `/home/dev/projects/hello-cross/.vscode/settings.json`. Это настройки папки проекта, не глобальные настройки VS Code. Готовое содержимое файла приведено в конце этого раздела.

| Параметр в `.vscode/settings.json` | Что вписать |
|---|---|
| `rpi.sysroot` | Абсолютный путь **на компьютере** к скопированному sysroot. Например, если `echo "$HOME"` выводит `/home/alex`, укажите `/home/alex/rpi/sysroot` |
| `rpi.remoteDir` | Абсолютный путь **на Raspberry Pi**, куда загружать программу. Например, `/home/dev/pi_test`; пользователь SSH должен иметь право создавать каталог и записывать в него |
| `rpi.sshHost` | Проверенный SSH-алиас компьютера, например `pi-dev` |
| `rpi.cc`, `rpi.cxx` | Пути к кросс-компиляторам **на компьютере**. При установке пакетами выше оставьте готовые значения |

Проверьте `ssh pi-dev` из терминала того же окна. Сохраните `settings.json`.

- **Ctrl+Shift+B** — настроить CMake и собрать `build/cross/hello`.
- `Tasks: Run Task` → `Run on Pi` — собрать, загрузить и запустить программу на плате.
- Точка останова в `main.c`, **F5** → `Raspberry Pi: cross` — собрать, загрузить и отладить.

Переключать профиль не требуется: файлы этого примера уже настроены только для кросс-компиляции. Порт `rpi.gdbPort` должен быть свободен на компьютере и плате. Вывод программы появится в терминале задачи `Debug server`. Если после остановки отладки SSH/gdbserver-задача осталась работать, завершите её перед повторным запуском.

SSH-туннель защищает соединение с компьютера, но gdbserver не имеет аутентификации и слушает TCP-порт платы. Используйте его в доверенной сети или ограничьте доступ firewall. Задача ждёт строку `Listening on port` перед подключением отладчика. См. [задачи VS Code](https://code.visualstudio.com/docs/debugtest/tasks) и [документацию gdbserver](https://www.sourceware.org/gdb/current/onlinedocs/gdb.html/Server.html).

Для сборки без VS Code, из папки проекта на компьютере:

```bash
export RPI_CC=/usr/bin/aarch64-linux-gnu-gcc
export RPI_CXX=/usr/bin/aarch64-linux-gnu-g++
export RPI_SYSROOT="$HOME/rpi/sysroot"
cmake --preset cross
cmake --build build/cross --parallel 2
file build/cross/hello
ssh pi-dev 'mkdir -p /home/dev/pi_test'
scp build/cross/hello pi-dev:/home/dev/pi_test/hello
ssh pi-dev 'cd /home/dev/pi_test && ./hello'
```

### Альтернативные toolchain

Можно взять актуальный Linux AArch64 toolchain из [официального каталога Arm](https://gitlab.arm.com/tooling/gnu-toolchains-for-arm) и заменить `rpi.cc` и `rpi.cxx` полными путями к `gcc` и `g++`. Выбирайте пакет для архитектуры **компьютера** с target `aarch64-none-linux-gnu`. `aarch64_be` означает big-endian, `aarch64-none-elf` — bare-metal; они не подходят к описанному сценарию Raspberry Pi OS. См. [страницу Arm GNU Toolchain](https://developer.arm.com/Tools%20and%20Software/GNU%20Toolchain).

### Готовые файлы для кросс-компиляции

Полный пример находится в [example](example). Ниже приведено содержимое каждого файла. Если создаёте проект вручную, размещайте файлы относительно **`$HOME/projects/hello-cross` на компьютере в Linux/WSL**; каталог `.vscode` находится внутри этой папки. Это отдельный законченный пример, объединять его с файлами другого варианта не нужно.

**`main.c`** ([готовый файл](example/main.c)):

```c
#include <stdio.h>

int main(void)
{
    puts("Hello, Raspberry Pi!");
    return 0;
}
```

**`CMakeLists.txt`** ([готовый файл](example/CMakeLists.txt)):

```cmake
cmake_minimum_required(VERSION 3.21)
project(rpi_example LANGUAGES C CXX)

set(APP_NAME hello CACHE STRING "Executable name")
add_executable(${APP_NAME} main.c)
target_compile_features(${APP_NAME} PRIVATE c_std_11)
target_compile_options(${APP_NAME} PRIVATE -Wall -Wextra)
```

**`CMakePresets.json`** ([готовый файл](example/CMakePresets.json)):

```json
{
    "version": 3,
    "configurePresets": [
        {
            "name": "cross",
            "generator": "Ninja",
            "binaryDir": "${sourceDir}/build/cross",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug",
                "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
            },
            "toolchainFile": "${sourceDir}/cmake/rpi-toolchain.cmake"
        }
    ]
}
```

**`cmake/rpi-toolchain.cmake`** ([готовый файл](example/cmake/rpi-toolchain.cmake)):

```cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

foreach(name RPI_CC RPI_CXX RPI_SYSROOT)
    if("$ENV{${name}}" STREQUAL "")
        message(FATAL_ERROR "Set ${name} before configuring the cross preset")
    endif()
endforeach()

set(CMAKE_C_COMPILER "$ENV{RPI_CC}")
set(CMAKE_CXX_COMPILER "$ENV{RPI_CXX}")
set(CMAKE_SYSROOT "$ENV{RPI_SYSROOT}")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
```

**`.vscode/settings.json`** ([готовый файл](example/.vscode/settings.json)):

```json
{
    "rpi.program": "hello",
    "rpi.jobs": "2",
    "rpi.cc": "/usr/bin/aarch64-linux-gnu-gcc",
    "rpi.cxx": "/usr/bin/aarch64-linux-gnu-g++",
    "rpi.sysroot": "/home/dev/rpi/sysroot",
    "rpi.sshHost": "pi-dev",
    "rpi.remoteDir": "/home/dev/pi_test",
    "rpi.gdbPort": "1234",
    "rpi.crossGdb": "/usr/bin/gdb-multiarch",
    "C_Cpp.default.compileCommands": [
        "${workspaceFolder}/build/cross/compile_commands.json"
    ]
}
```

**`.vscode/tasks.json`** ([готовый файл](example/.vscode/tasks.json)):

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Configure",
            "type": "process",
            "command": "cmake",
            "args": [
                "--preset",
                "cross",
                "-DAPP_NAME=${config:rpi.program}"
            ],
            "options": {
                "cwd": "${workspaceFolder}",
                "env": {
                    "RPI_CC": "${config:rpi.cc}",
                    "RPI_CXX": "${config:rpi.cxx}",
                    "RPI_SYSROOT": "${config:rpi.sysroot}"
                }
            },
            "problemMatcher": []
        },
        {
            "label": "Build",
            "type": "process",
            "command": "cmake",
            "args": [
                "--build",
                "${workspaceFolder}/build/cross",
                "--parallel",
                "${config:rpi.jobs}"
            ],
            "dependsOn": "Configure",
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": "$gcc"
        },
        {
            "label": "Prepare Pi directory",
            "type": "process",
            "command": "ssh",
            "args": [
                "${config:rpi.sshHost}",
                "mkdir -p '${config:rpi.remoteDir}'"
            ],
            "problemMatcher": []
        },
        {
            "label": "Deploy",
            "type": "process",
            "command": "scp",
            "args": [
                "${workspaceFolder}/build/cross/${config:rpi.program}",
                "${config:rpi.sshHost}:${config:rpi.remoteDir}/${config:rpi.program}"
            ],
            "dependsOn": [
                "Build",
                "Prepare Pi directory"
            ],
            "dependsOrder": "sequence",
            "problemMatcher": []
        },
        {
            "label": "Run on Pi",
            "type": "process",
            "command": "ssh",
            "args": [
                "${config:rpi.sshHost}",
                "cd '${config:rpi.remoteDir}' && './${config:rpi.program}'"
            ],
            "dependsOn": "Deploy",
            "problemMatcher": []
        },
        {
            "label": "Debug server",
            "type": "process",
            "command": "ssh",
            "args": [
                "-o",
                "ExitOnForwardFailure=yes",
                "-L",
                "127.0.0.1:${config:rpi.gdbPort}:127.0.0.1:${config:rpi.gdbPort}",
                "${config:rpi.sshHost}",
                "cd '${config:rpi.remoteDir}' && LC_ALL=C gdbserver --once :${config:rpi.gdbPort} './${config:rpi.program}'"
            ],
            "dependsOn": "Deploy",
            "isBackground": true,
            "problemMatcher": {
                "owner": "rpi-gdbserver",
                "pattern": {
                    "regexp": "^(.+):(\\d+): (.+)$",
                    "file": 1,
                    "line": 2,
                    "message": 3
                },
                "background": {
                    "activeOnStart": true,
                    "beginsPattern": "^Process .* created; pid = ",
                    "endsPattern": "^Listening on port \\d+"
                }
            }
        }
    ]
}
```

**`.vscode/launch.json`** ([готовый файл](example/.vscode/launch.json)):

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Raspberry Pi: cross",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/cross/${config:rpi.program}",
            "cwd": "${workspaceFolder}",
            "MIMode": "gdb",
            "miDebuggerPath": "${config:rpi.crossGdb}",
            "miDebuggerServerAddress": "127.0.0.1:${config:rpi.gdbPort}",
            "preLaunchTask": "Debug server",
            "stopAtEntry": false,
            "externalConsole": false,
            "setupCommands": [
                {
                    "text": "set sysroot ${config:rpi.sysroot}"
                }
            ]
        }
    ]
}
```

[К списку туториалов](../README.md) · [К описанию проекта](../../README.md)

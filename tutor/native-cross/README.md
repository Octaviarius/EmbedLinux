# Нативная сборка и кросс-компиляция

## Содержание

- [Цель и окружение](#цель-и-окружение)
- [Что нужно в принципе](#что-нужно-в-принципе)
- [Подготовка платы и SSH](#подготовка-платы-и-ssh)
  - [Подключение по IP-адресу](#подключение-по-ip-адресу)
  - [Подключение по имени хоста](#подключение-по-имени-хоста)
  - [Один алиас вместо адресов в каждом файле](#один-алиас-вместо-адресов-в-каждом-файле)
- [Вариант A: нативная разработка](#вариант-a-нативная-разработка)
  - [Подготовьте инструменты на плате](#подготовьте-инструменты-на-плате)
  - [Откройте пример, соберите и отладьте](#откройте-пример-соберите-и-отладьте)
  - [Готовые файлы для нативной разработки](#готовые-файлы-для-нативной-разработки)
- [Вариант B: кросс-компиляция](#вариант-b-кросс-компиляция)
  - [Подготовьте Linux или WSL](#подготовьте-linux-или-wsl)
  - [Скопируйте sysroot с платы](#скопируйте-sysroot-с-платы)
  - [Создайте проект и укажите свои пути](#создайте-проект-и-укажите-свои-пути)
  - [Альтернативные toolchain](#альтернативные-toolchain)
  - [Готовые файлы для кросс-компиляции](#готовые-файлы-для-кросс-компиляции)
- [Изменения под свой проект](#изменения-под-свой-проект)
- [Если что-то не работает](#если-что-то-не-работает)
- [Проверка результата](#проверка-результата)

Соберём и запустим простое приложение на C двумя способами: непосредственно в целевой системе (**native**) и на рабочем компьютере для целевой системы (**cross**). Затем настроим отладку и готовые проекты VS Code.

## Цель и окружение

Результат туториала — программа `hello`, которая выводит `Hello, Raspberry Pi!` на целевом устройстве. В нативном варианте компиляция выполняется на плате; в cross-варианте — на компьютере, после чего исполняемый файл передаётся на плату.

Для практической части выбрана [Raspberry Pi Zero 2 W (`rpi0-2w`)](../../hardware/rpi0-2w/README.md). Пример кросс-компиляции рассчитан на Raspberry Pi OS с 64-битными приложениями и библиотеками (`arm64`). Рабочий компьютер — Linux или Windows с Ubuntu в WSL. Команды Linux выполняются в Bash; команды установки пакетов рассчитаны на системы семейства Debian.

Нужны установленная ОС, доступ к плате по SSH и доступ к репозиториям пакетов. Подготовка образа ОС не входит в этот туториал: используйте [инструкцию Raspberry Pi](https://www.raspberrypi.com/documentation/computers/getting-started.html). Для примеров требуется CMake 3.21 или новее; проверьте `cmake --version` после установки.

Общий принцип применим к другим платформам, но компилятор, sysroot и параметры целевой архитектуры нужно подобрать отдельно. Для `rpi0-2w` начните с одного процесса нативной сборки, как в готовых настройках ниже.

## Что нужно в принципе

- Raspberry Pi с ОС, питанием и подключением к сети.
- SSH-доступ к плате.
- Компилятор C/C++ и система сборки CMake.
- VS Code с расширением **C/C++** (`ms-vscode.cpptools`).
- Для отладки — GDB; при кросс-компиляции дополнительно нужен `gdbserver` на плате.
- Для кросс-компиляции — компилятор под архитектуру целевых приложений и **sysroot**: каталог заголовков и библиотек целевой системы.

| | Нативная разработка | Кросс-компиляция |
|---|---|---|
| Исходники и сборка | На Raspberry Pi | На компьютере в Linux/WSL |
| Среда работы | SSH-терминал; Remote SSH при достаточных ресурсах платы | VS Code в Linux или WSL |
| Компилятор | GCC на плате | AArch64 GCC на компьютере |
| Отдельный sysroot | Не нужен | Нужен из ОС платы |
| Отладка | GDB на плате | GDB на компьютере → SSH → gdbserver на плате |
| Когда выбирать | Простой старт, небольшие проекты | Большие проекты, частые сборки |

Начните с нативного варианта, если скорость сборки устраивает. Для каждого варианта ниже приведён самостоятельный готовый проект со своими настройками VS Code.

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

### Подключение по IP-адресу

Узнайте адрес платы в списке подключённых устройств роутера или выполните **на Raspberry Pi**:

```bash
hostname -I
```

Команда может вывести несколько адресов. Выберите адрес интерфейса Ethernet или Wi-Fi, доступного с компьютера. Затем подключитесь **с компьютера**:

```bash
ssh dev@192.168.1.50
```

Если роутер выдаёт адрес через DHCP, он может измениться. Для постоянного адреса можно настроить DHCP-резервирование для платы на роутере.

### Подключение по имени хоста

Имя платы задаётся в Raspberry Pi Imager. Посмотреть текущее имя можно командой `hostname` на плате, изменить — через `sudo raspi-config` → `System Options` → `Hostname`. Для нескольких плат задавайте разные имена.

Если имя платы `raspberrypi`, попробуйте подключиться с компьютера:

```bash
ssh dev@raspberrypi.local
```

Для имени `rpi-dev` команда будет `ssh dev@rpi-dev.local`. Суффикс `.local` используется для обнаружения имени через mDNS в локальной сети. Это требует работающего mDNS на плате и поддержки разрешения таких имён на компьютере; сеть должна пропускать соответствующий multicast-трафик.

Подключение без суффикса, например `ssh dev@raspberrypi`, работает, если это имя разрешается через DNS вашей сети или запись в файле `hosts`. Само наличие имени на плате не гарантирует его разрешение на компьютере.

Если `.local` не находится, проверьте подключение по IP. Если IP работает, а имя — нет, проблема в разрешении имени. В WSL проверьте имя отдельно: успешное подключение из Windows не гарантирует, что тот же адрес по имени доступен из WSL.

Оба способа используют тот же SSH-сервер и пользователя. См. [документацию удалённого доступа Raspberry Pi](https://www.raspberrypi.com/documentation/computers/remote-access.html).

### Один алиас вместо адресов в каждом файле

Добавьте в SSH-конфигурацию компьютера. В `HostName` укажите выбранный IP-адрес или имя хоста:

```sshconfig
Host pi-dev
    HostName 192.168.1.50
    User dev
    IdentityFile ~/.ssh/key_rpi
    IdentitiesOnly yes
```

Для подключения по имени замените строку на `HostName raspberrypi.local`. `Host pi-dev` — локальный алиас SSH-клиента, а `HostName` — адрес платы; они не обязаны совпадать с системным именем Raspberry Pi. После настройки используйте `ssh pi-dev`, выбирайте `pi-dev` в Remote SSH и оставьте `rpi.sshHost` равным `pi-dev` в настройках VS Code. При смене адреса достаточно обновить `HostName`.

В Linux/WSL файл находится в `~/.ssh/config`, в Windows — в `%USERPROFILE%\.ssh\config`. По умолчанию Windows и WSL читают SSH-конфигурации из своих домашних каталогов; пути к ключам тоже настраиваются отдельно. Для cross-сценария проверьте доступ именно из WSL.

Ключ можно создать и установить из Linux/WSL:

```bash
ssh-keygen -t ed25519 -f ~/.ssh/key_rpi
ssh-copy-id -i ~/.ssh/key_rpi.pub dev@192.168.1.50
chmod 600 ~/.ssh/config
ssh pi-dev
```

Если ключ уже существует, используйте его, не перезаписывайте. Для Remote SSH из Windows публичный ключ можно добавить при установке ОС через Imager; приватный ключ должен лежать по пути из Windows SSH-конфигурации. Перед автоматической загрузкой и отладкой проверьте, что `ssh pi-dev` работает без запроса пароля учётной записи. Ключ с парольной фразой загрузите в SSH-агент.

## Вариант A: нативная разработка

### Подготовьте инструменты на плате

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build gdb git
```

Для нативной сборки достаточно обычного SSH: подключитесь к плате командой `ssh pi-dev` и выполняйте команды сборки в её терминале.

VS Code Remote SSH запускает сервер редактора и расширения на плате. В [официальных требованиях](https://code.visualstudio.com/docs/remote/ssh#system-requirements) указан минимум 1 ГБ RAM; у `rpi0-2w` — 512 МБ. Поэтому этот вариант не соответствует указанному минимуму и его работа на данной плате здесь не подтверждена. Для нативного сценария используйте обычный SSH; cross-сценарий запускает VS Code на рабочем компьютере.

Если ваша платформа соответствует требованиям Remote SSH, установите на компьютере **Remote - SSH** (`ms-vscode-remote.remote-ssh`), подключитесь через `Remote-SSH: Connect to Host...` → `pi-dev` и установите **C/C++** в окружение `SSH: pi-dev`. После копирования примера откройте его папку на плате. Терминал, сборка и GDB такого окна работают в целевой системе.

### Откройте пример, соберите и отладьте

Скопируйте **содержимое** `tutor/native-cross/examples/native`, включая скрытую папку `.vscode`, в `/home/dev/projects/hello-native` на плате. Если репозиторий EmbedLinux уже находится на плате, выполните из его корня:

```bash
mkdir -p "$HOME/projects/hello-native"
cp -R tutor/native-cross/examples/native/. "$HOME/projects/hello-native/"
```

Если репозиторий есть только на компьютере, скопируйте пример из корня EmbedLinux в Bash Linux/WSL:

```bash
ssh pi-dev 'mkdir -p /home/dev/projects/hello-native'
scp -r tutor/native-cross/examples/native/. pi-dev:/home/dev/projects/hello-native/
```

Затем подключитесь к плате через `ssh pi-dev` и перейдите в каталог проекта: `cd "$HOME/projects/hello-native"`.

Для варианта с Remote SSH в окне VS Code `SSH: pi-dev` откройте `/home/dev/projects/hello-native` через `File → Open Folder`. В этой папке должны сразу находиться `main.c`, `CMakeLists.txt` и `.vscode`, а не дополнительный вложенный каталог `native`.

В готовых настройках VS Code используются установленный `/usr/bin/gdb` и имя программы `hello`; менять настройки для первого запуска не требуется. Если ваш домашний каталог отличается от `/home/dev`, откройте соответствующую папку внутри него.

Если используете Remote SSH и открыли папку примера в VS Code:

- **Ctrl+Shift+B** — настроить CMake и собрать `build/native/hello`.
- `./build/native/hello` в терминале VS Code — запустить программу.
- Поставьте точку останова в `main.c`, нажмите **F5**, выберите `Raspberry Pi: native`.

Через обычный SSH, из папки проекта на плате (это основной нативный путь для `rpi0-2w`):

```bash
cmake --preset native
cmake --build build/native --parallel 1
./build/native/hello
```

Для отладки в том же SSH-терминале:

```bash
gdb ./build/native/hello
```

В приглашении GDB выполните `break main`, `run`, затем `next`. Для завершения — `quit`. Такой способ не требует VS Code Server на плате.

### Готовые файлы для нативной разработки

Полный пример находится в [examples/native](examples/native). Ниже приведено содержимое каждого файла. Если создаёте проект вручную, размещайте файлы относительно **`/home/dev/projects/hello-native` на Raspberry Pi**; каталог `.vscode` находится внутри этой папки. Это отдельный законченный пример, объединять его с файлами другого варианта не нужно.

**`main.c`** ([готовый файл](examples/native/main.c)):

```c
#include <stdio.h>

int main(void)
{
    puts("Hello, Raspberry Pi!");
    return 0;
}
```

**`CMakeLists.txt`** ([готовый файл](examples/native/CMakeLists.txt)):

```cmake
cmake_minimum_required(VERSION 3.21)
project(rpi_example LANGUAGES C CXX)

set(APP_NAME hello CACHE STRING "Executable name")
add_executable(${APP_NAME} main.c)
target_compile_features(${APP_NAME} PRIVATE c_std_11)
target_compile_options(${APP_NAME} PRIVATE -Wall -Wextra)
```

**`CMakePresets.json`** ([готовый файл](examples/native/CMakePresets.json)):

```json
{
    "version": 3,
    "configurePresets": [
        {
            "name": "native",
            "generator": "Ninja",
            "binaryDir": "${sourceDir}/build/native",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug",
                "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
            }
        }
    ]
}
```

**`.vscode/settings.json`** ([готовый файл](examples/native/.vscode/settings.json)):

```json
{
    "rpi.program": "hello",
    "rpi.jobs": "1",
    "rpi.gdb": "/usr/bin/gdb",
    "C_Cpp.default.compileCommands": [
        "${workspaceFolder}/build/native/compile_commands.json"
    ]
}
```

**`.vscode/tasks.json`** ([готовый файл](examples/native/.vscode/tasks.json)):

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
                "native",
                "-DAPP_NAME=${config:rpi.program}"
            ],
            "options": {
                "cwd": "${workspaceFolder}"
            },
            "problemMatcher": []
        },
        {
            "label": "Build",
            "type": "process",
            "command": "cmake",
            "args": [
                "--build",
                "${workspaceFolder}/build/native",
                "--parallel",
                "${config:rpi.jobs}"
            ],
            "dependsOn": "Configure",
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": "$gcc"
        }
    ]
}
```

**`.vscode/launch.json`** ([готовый файл](examples/native/.vscode/launch.json)):

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Raspberry Pi: native",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/native/${config:rpi.program}",
            "cwd": "${workspaceFolder}",
            "args": [],
            "MIMode": "gdb",
            "miDebuggerPath": "${config:rpi.gdb}",
            "preLaunchTask": "Build",
            "stopAtEntry": false,
            "externalConsole": false
        }
    ]
}
```

## Вариант B: кросс-компиляция

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

На компьютере в Linux/WSL скопируйте **содержимое** `tutor/native-cross/examples/cross`, включая `.vscode`, в отдельную папку. Из корня репозитория EmbedLinux:

```bash
mkdir -p "$HOME/projects/hello-cross"
cp -R tutor/native-cross/examples/cross/. "$HOME/projects/hello-cross/"
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

Полный пример находится в [examples/cross](examples/cross). Ниже приведено содержимое каждого файла. Если создаёте проект вручную, размещайте файлы относительно **`$HOME/projects/hello-cross` на компьютере в Linux/WSL**; каталог `.vscode` находится внутри этой папки. Это отдельный законченный пример, объединять его с файлами другого варианта не нужно.

**`main.c`** ([готовый файл](examples/cross/main.c)):

```c
#include <stdio.h>

int main(void)
{
    puts("Hello, Raspberry Pi!");
    return 0;
}
```

**`CMakeLists.txt`** ([готовый файл](examples/cross/CMakeLists.txt)):

```cmake
cmake_minimum_required(VERSION 3.21)
project(rpi_example LANGUAGES C CXX)

set(APP_NAME hello CACHE STRING "Executable name")
add_executable(${APP_NAME} main.c)
target_compile_features(${APP_NAME} PRIVATE c_std_11)
target_compile_options(${APP_NAME} PRIVATE -Wall -Wextra)
```

**`CMakePresets.json`** ([готовый файл](examples/cross/CMakePresets.json)):

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

**`cmake/rpi-toolchain.cmake`** ([готовый файл](examples/cross/cmake/rpi-toolchain.cmake)):

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

**`.vscode/settings.json`** ([готовый файл](examples/cross/.vscode/settings.json)):

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

**`.vscode/tasks.json`** ([готовый файл](examples/cross/.vscode/tasks.json)):

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

**`.vscode/launch.json`** ([готовый файл](examples/cross/.vscode/launch.json)):

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

## Изменения под свой проект

В обоих примерах `rpi.*` — пользовательские параметры в `.vscode/settings.json`. Задачи читают их через `${config:...}`; VS Code может помечать такие параметры как неизвестные настройки. Вписывайте полные пути без `~` и вложенных `${...}`. В удалённых командах примера используйте имя программы и каталог без пробелов, кавычек и shell-символов. См. [переменные VS Code](https://code.visualstudio.com/docs/reference/variables-reference).

| Что изменить | Где |
|---|---|
| Имя программы | `rpi.program` в `.vscode/settings.json`; для сборки из терминала — `-DAPP_NAME=имя` при настройке CMake |
| Число процессов сборки | `rpi.jobs` в `.vscode/settings.json` |
| Исходники и библиотеки | `add_executable`, `target_link_libraries` в `CMakeLists.txt` |
| Каталоги заголовков | `target_include_directories` или импортированные targets в `CMakeLists.txt` |
| Debug / Release | `CMAKE_BUILD_TYPE` в `CMakePresets.json` соответствующего примера |
| Аргументы программы | В нативном примере — `args` в `.vscode/launch.json`; в cross-примере — после имени программы в команде gdbserver задачи `Debug server` |

Для C++ замените `main.c` на `main.cpp` в проекте и `CMakeLists.txt`, а `c_std_11` — на, например, `cxx_std_17`.

IntelliSense читает реальные параметры сборки из `build/native/compile_commands.json` или `build/cross/compile_commands.json`. Файл появляется после выполнения `Configure`; пути уже указаны в настройках каждого примера. `c_cpp_properties.json` не требуется. См. [настройки C/C++](https://code.visualstudio.com/docs/cpp/customize-cpp-settings).

В cross-примере CMake читает переменные `RPI_CC`, `RPI_CXX`, `RPI_SYSROOT`. Задача `Configure` задаёт их из `.vscode/settings.json`; при работе из терминала экспортируйте их сами. После смены компилятора или sysroot настройте профиль с чистым кэшем: `cmake --fresh --preset cross` (CMake 3.24+), предварительно экспортировав эти переменные. Для более старого CMake используйте новый каталог сборки. См. [toolchain в CMake](https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html).

Если проект использует pkg-config, задайте `PKG_CONFIG_SYSROOT_DIR` равным пути к sysroot, а `PKG_CONFIG_LIBDIR` — каталогам целевых `.pc`-файлов внутри него. Проверьте, что `PKG_CONFIG_PATH` не добавляет каталоги пакетов компьютера.

Для отладки нужна Debug-сборка с символами. При cross-отладке локальный бинарник и загруженная на плату копия должны совпадать; не применяйте `strip` к локальному файлу. См. [настройки отладки C/C++](https://code.visualstudio.com/docs/cpp/launch-json-reference).

## Если что-то не работает

| Симптом | Что проверить |
|---|---|
| `Exec format error` | Архитектуру бинарника (`file`), пакетов платы (`dpkg --print-architecture`) и поддержку этой архитектуры ядром |
| Файл существует, но запуск сообщает `No such file or directory` | Путь ELF-загрузчика: `readelf -l build/cross/hello`; есть ли он на плате |
| Не найдены заголовки, `crt*.o` или `-l...` | Пакеты `*-dev` на плате, полноту sysroot и совместимость toolchain |
| `GLIBC_*` / `GLIBCXX_* not found` | Runtime-библиотеки платы и библиотеки, использованные при линковке |
| IntelliSense ошибается при успешной сборке | Выполнить `Configure`, выбрать `compile_commands.json` нужного профиля |
| F5 ждёт или порт занят | Терминал `Debug server`, SSH-доступ, наличие gdbserver, оставшуюся задачу |
| Точки останова не срабатывают | Debug-сборку, совпадение локального и загруженного бинарника |

## Проверка результата

После сборки выполните программу на плате. Для обоих вариантов ожидается строка:

```text
Hello, Raspberry Pi!
```

В cross-варианте `file build/cross/hello` на компьютере должен показывать ELF для ARM AArch64. Проверка архитектуры файла не заменяет запуск: успешный запуск на плате проверяет также доступность загрузчика и runtime-библиотек. Для проверки отладки остановитесь в `main` и выполните код пошагово.

При подготовке проверены JSON-конфигурации, зависимости задач, ссылки и совпадение файлов с примерами в README. Оба проекта собраны в Ubuntu/WSL: native — для x86-64 с локальным запуском, cross — для AArch64 с библиотеками кросс-компилятора дистрибутива. В проверке использован генератор Unix Makefiles вместо отсутствующего Ninja; sysroot конкретной платы не проверен. Запуск и отладка на `rpi0-2w` пока не подтверждены.

Инструкции адаптированы из прежнего мануала по Raspberry Pi; официальные требования Remote SSH и CMake сверены 12 октября 2026 года.

[К списку туториалов](../README.md) · [К описанию проекта](../../README.md)

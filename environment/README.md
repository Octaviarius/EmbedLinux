# Рабочее окружение: WSL и Ubuntu/Debian

## Содержание

- [Назначение](#назначение)
- [Host, target и имена параметров](#host-target-и-имена-параметров)
- [Windows и WSL](#windows-и-wsl)
- [Ubuntu, Debian и близкие системы](#ubuntu-debian-и-близкие-системы)
- [VS Code и расположение проекта](#vs-code-и-расположение-проекта)
- [Пути, пользователи и SSH](#пути-пользователи-и-ssh)
- [Перед началом туториала](#перед-началом-туториала)

## Назначение

Общие настройки рабочего компьютера для туториалов. Основной вариант — **Windows, WSL 2 с Ubuntu и VS Code**. При работе в Ubuntu/Debian напрямую команды Bash и пакеты те же, а установка WSL не требуется.

Это окружение компьютера разработчика. Linux на целевом устройстве — отдельная система со своими пакетами, пользователями и путями.

## Host, target и имена параметров

**Host** — компьютер разработчика; в варианте с Windows команды выполняются в Ubuntu/WSL. **Target** — целевое Linux-устройство, на котором запускается приложение. При нативной сборке компилятор работает на target, при кросс-сборке — на host; роли систем от этого не меняются.

| Категория | Переменные Bash | Настройки VS Code |
|---|---|---|
| Целевое устройство | `TARGET_SSH_HOST`, `TARGET_WORK_DIR`, `TARGET_BUILD_DIR`, `TARGET_GDBSERVER` | `target.sshHost`, `target.workDir`, `target.buildDir`, `target.gdbserver` |
| Компьютер разработчика | `HOST_WORK_DIR`, `HOST_BUILD_DIR`, `HOST_SYSROOT`, `HOST_TOOLCHAIN_FILE`, `HOST_GDB` | `host.workDir`, `host.buildDir`, `host.sysroot`, `host.toolchainFile`, `host.gdb` |
| Исполняемый файл приложения на host | `HOST_APP_PATH` | `host.appPath` |
| Исполняемый файл приложения на target | `TARGET_APP_PATH` | `target.appPath` |
| Параметры сборки | `BUILD_TYPE`, `BUILD_JOBS` | `build.type`, `build.jobs` |

Префикс указывает, к какой системе относится настройка. Для путей это место хранения: `HOST_SYSROOT` находится на host, хотя содержит заголовки и библиотеки target. `TARGET_SSH_HOST` — адрес или алиас подключения к target; он настраивается на host.

Архитектура и компиляторы кросс-сборки задаются в toolchain-файле CMake, без дублирования в переменных окружения. В примерах используются только параметры, необходимые соответствующему варианту.

`TARGET_APP_PATH` — полный путь к исполняемому файлу на target. Он используется для запуска, передачи и отладки. Имя приложения фиксируется в CMake самого проекта; этот параметр его не меняет.

В Bash пути формируются последовательно и имеют абсолютные значения: `WORK_DIR → BUILD_DIR → APP_PATH`. В VS Code `target.workDir` — абсолютный путь на устройстве, `host.workDir` — относительно `${workspaceFolder}`; `host.buildDir` — относительно рабочего каталога host, `host.appPath` — относительно каталога сборки. В нативном примере `target.buildDir` — относительно `target.workDir`, `target.appPath` — относительно него. В кросс-примере `target.workDir` — каталог установки, `target.appPath` — относительно него; каталога сборки на target нет. `host.toolchainFile` — относительно `host.workDir`, а `host.sysroot` — абсолютный путь к sysroot на host. Полный путь соединяется непосредственно в `tasks.json` или `launch.json`, без вложенных `${config:...}` в настройках.

## Windows и WSL

В **PowerShell от имени администратора** для новой установки:

```powershell
wsl --install -d Ubuntu
```

Перезагрузите Windows, если установщик попросит. Запустите Ubuntu и создайте Linux-пользователя. Если WSL уже установлен, не переустанавливайте его; проверьте имеющиеся дистрибутивы:

```powershell
wsl --list --verbose
wsl --list --online
```

Для туториалов используем WSL 2. Указанное имя `Ubuntu` заменяйте именем своего дистрибутива, если оно отличается. См. [установку WSL](https://learn.microsoft.com/en-us/windows/wsl/install) и [команды WSL](https://learn.microsoft.com/en-us/windows/wsl/basic-commands).

## Ubuntu, Debian и близкие системы

В **терминале Ubuntu/WSL либо обычной Ubuntu/Debian**:

```bash
sudo apt update
sudo apt install git openssh-client rsync build-essential
```

Для нативного туториала GCC компьютера используется для подсказок редактора; приложение собирается компилятором на плате. CMake, Ninja и GDB на плате устанавливаются отдельно по инструкции туториала.

Проверьте окружение:

```bash
cat /etc/os-release
uname -m
command -v ssh rsync git gcc
```

Команды `apt` подходят Ubuntu, Debian и совместимым системам с такими пакетами. Для другого дистрибутива подберите пакеты через его менеджер; названия и версии могут отличаться. Виртуальная машина с Ubuntu/Debian тоже подходит, если из неё доступно целевое устройство по сети.

## VS Code и расположение проекта

В Windows установите VS Code и расширение **WSL** (`ms-vscode-remote.remote-wsl`). В терминале Ubuntu/WSL откройте папку проекта:

```bash
mkdir -p "$HOME/projects"
cd "$HOME/projects"
code .
```

Окно должно показывать контекст WSL. Расширение **C/C++** (`ms-vscode.cpptools`) устанавливайте в это окружение. Интерфейс редактора остаётся в Windows, а терминал, расширения и Linux-команды работают в WSL. См. [официальный туториал VS Code для WSL](https://code.visualstudio.com/docs/remote/wsl-tutorial).

Храните рабочие проекты в файловой системе Linux, например `/home/alex/projects`, когда работаете Linux-инструментами. Диск `C:` доступен из WSL как `/mnt/c`; существующий репозиторий можно читать и копировать оттуда. Пример для данного расположения EmbedLinux:

```bash
mkdir -p "$HOME/projects/hello-native"
cp -R /mnt/c/WORK/study/EmbedLinux/tutor/native/example/. "$HOME/projects/hello-native/"
cd "$HOME/projects/hello-native"
code .
```

При обычной Ubuntu/Debian установите VS Code для Linux и C/C++; WSL-расширение не требуется. Открывайте папку конкретного примера с `.vscode`, а не весь репозиторий: настройки вложенной папки не подхватываются автоматически.

## Пути, пользователи и SSH

| Окружение | Пример домашнего каталога | SSH-конфигурация |
|---|---|---|
| Windows | `C:\Users\alex` | `%USERPROFILE%\.ssh\config` |
| Ubuntu/WSL | `/home/alex` | `~/.ssh/config` внутри WSL |
| Целевое устройство | `/home/dev` | Собственные настройки пользователя устройства |

Имена пользователей компьютера и устройства могут отличаться. По умолчанию Windows и WSL читают разные SSH-конфигурации; рабочий SSH-алиас нужно проверять из терминала того окружения, где выполняются задачи VS Code.

Сетевое имя вида `raspberrypi.local` зависит от разрешения mDNS в выбранном окружении. Если имя работает из Windows, но не из WSL, попробуйте IP и затем разберитесь с разрешением имени. Настройка ключа и алиаса приведена в [туториале доступа по SSH](../tutor/ssh/README.md).

## Перед началом туториала

- Проверьте, что терминал VS Code работает в нужном Linux-окружении.
- Откройте папку примера, содержащую `.vscode` и `CMakeLists.txt`.
- Проверьте доступность устройства из этого терминала.
- Устанавливайте пакеты на компьютере или устройстве в соответствии с указанием в туториале.

[К описанию проекта](../README.md) · [К туториалам](../tutor/README.md)

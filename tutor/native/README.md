# Нативная разработка: VS Code в WSL, сборка на устройстве

## Содержание

- [Цель и схема работы](#цель-и-схема-работы)
- [Предварительные условия](#предварительные-условия)
- [Настройте SSH в WSL](#настройте-ssh-в-wsl)
- [Установите инструменты на плате](#установите-инструменты-на-плате)
- [Откройте проект в VS Code](#откройте-проект-в-vs-code)
- [Соберите, запустите и отладьте](#соберите-запустите-и-отладьте)
- [Готовые файлы проекта](#готовые-файлы-проекта)
- [Подсказки редактора и изменения проекта](#подсказки-редактора-и-изменения-проекта)
- [Если что-то не работает](#если-что-то-не-работает)
- [Границы проверки](#границы-проверки)

## Цель и схема работы

Соберём приложение на C на [Raspberry Pi Zero 2 W (`rpi0-2w`)](../../hardware/rpi0-2w/README.md), запустим его и настроим отладку из VS Code на компьютере.

**VS Code и исходники находятся в Ubuntu/WSL. GCC, CMake, исполняемый файл и GDB — на Raspberry Pi.** Это нативная сборка: компилятор работает в той же системе, где выполняется программа. Передача исходников по SSH не делает сборку кросс-компиляцией.

| Где | Что выполняется |
|---|---|
| Windows | Интерфейс VS Code |
| Ubuntu в WSL | Расширение C/C++, редактирование, задачи SSH и rsync |
| Raspberry Pi | Компиляция, линковка, запуск и GDB |

VS Code Server на плате для этого сценария не нужен. При обычной Ubuntu/Debian на компьютере используется тот же порядок действий без слоя Windows/WSL.

## Предварительные условия

Подготовьте компьютер по [инструкции рабочего окружения](../../environment/README.md). Установите Raspberry Pi OS на плату, настройте сеть и включите SSH по [инструкции производителя](https://www.raspberrypi.com/documentation/computers/getting-started.html).

Для резервного доступа при недоступности Wi-Fi настройте и проверьте [UART-консоль `rpi0-2w`](../../hardware/rpi0-2w/README.md#отладочная-консоль-через-uart).

В примере пользователь платы — `dev`, SSH-алиас — `pi-dev`. Замените пользователя и адрес своими. Нативная сборка использует инструменты и библиотеки установленной ОС платы; отдельный sysroot не нужен.

Все команды компьютера ниже выполняются в **Bash WSL**, команды платы — в SSH-терминале Raspberry Pi. CMake на плате должен быть версии 3.21 или новее.

## Настройте SSH в WSL

Подключиться можно по IP или имени хоста:

```bash
ssh dev@192.168.1.50
ssh dev@raspberrypi.local
```

Используйте доступный способ. IP можно посмотреть на плате командой `hostname -I` или в списке клиентов роутера. Имя `raspberrypi.local` подходит, если имя платы — `raspberrypi` и в сети работает mDNS; если имя не разрешается из WSL, используйте IP. Подробнее: [удалённый доступ Raspberry Pi](https://www.raspberrypi.com/documentation/computers/remote-access.html).

На компьютере **в WSL** создайте ключ, если ещё нет подходящего:

```bash
mkdir -p ~/.ssh
chmod 700 ~/.ssh
ssh-keygen -t ed25519 -f ~/.ssh/key_rpi
ssh-copy-id -i ~/.ssh/key_rpi.pub dev@192.168.1.50
```

Не перезаписывайте существующий ключ. Если у ключа есть парольная фраза, загрузите его в агент текущего сеанса WSL:

```bash
eval "$(ssh-agent -s)"
ssh-add ~/.ssh/key_rpi
```

В **`~/.ssh/config` внутри WSL** добавьте запись. Это готовый блок для данной платы, другие существующие записи оставьте:

```sshconfig
Host pi-dev
    HostName 192.168.1.50
    User dev
    IdentityFile ~/.ssh/key_rpi
    IdentitiesOnly yes
```

Вместо IP можно задать `HostName raspberrypi.local`. `pi-dev` — алиас клиента, он не обязан совпадать с именем платы. Проверьте подключение сначала интерактивно, затем без запросов пароля:

```bash
chmod 600 ~/.ssh/config
ssh pi-dev
```

После выхода из SSH командой `exit`, в WSL:

```bash
ssh -o BatchMode=yes pi-dev 'hostname'
```

Последняя команда должна завершиться без запроса ввода: так будут подключаться задачи и отладчик. Агент SSH, запущенный после VS Code, может быть недоступен уже открытому окну; запускайте `code .` из терминала, в котором работает агент.

## Установите инструменты на плате

Подключитесь из WSL командой `ssh pi-dev`. **На плате**:

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build gdb rsync
cmake --version
gcc --version
```

Для Zero 2 W начните с одного процесса сборки: у платы 512 МБ RAM. Завершите SSH-сеанс командой `exit`.

## Откройте проект в VS Code

Из корня репозитория EmbedLinux **в WSL**:

```bash
mkdir -p "$HOME/projects/hello-native"
cp -R tutor/native/example/. "$HOME/projects/hello-native/"
cd "$HOME/projects/hello-native"
code .
```

В VS Code должен отображаться контекст WSL. Установите расширение **C/C++** (`ms-vscode.cpptools`) в окружение WSL. Откройте именно папку `hello-native`, содержащую `.vscode` и `CMakeLists.txt`.

В локальном **`.vscode/settings.json`** замените:

| Параметр | Значение |
|---|---|
| `target.host` | Проверенный SSH-алиас WSL, например `pi-dev` |
| `target.remoteDir` | Отдельный каталог проекта **на плате**, например `/home/dev/projects/hello-native` |
| `target.program` | Имя исполняемого файла; для первого запуска оставьте `hello` |
| `target.jobs` | Число процессов сборки на плате; оставьте `1` |
| `target.gdb` | Путь к GDB **на плате**, обычно `/usr/bin/gdb` |

Задача синхронизирует содержимое локального проекта в `target.remoteDir` через rsync с `--delete`: удалённые файлы, исчезнувшие локально, будут удалены и на плате. Выберите отдельный каталог только для этого проекта. Каталоги `build`, `.git` и `.vscode` исключены из передачи и удаления; результат сборки остаётся на плате.

Для удалённых команд этого примера используйте пути и имя программы без пробелов, кавычек и shell-символов. Параметры `target.*` — пользовательские настройки для `${config:...}`; VS Code может отмечать их как неизвестные.

## Соберите, запустите и отладьте

Сохраните исходник перед запуском задачи.

- **Ctrl+Shift+B** — передать исходники, настроить CMake и собрать на плате.
- `Tasks: Run Task` → `Run on target` — собрать и запустить.
- Поставьте точку останова на строке `puts` в `main.c`, нажмите **F5**, выберите `Native: GDB on target via SSH`.

Ожидаемый вывод программы:

```text
Hello, Raspberry Pi!
```

GDB запускается на плате через SSH с помощью `pipeTransport`. Команда `set substitute-path` сопоставляет удалённые пути исходников с локальными файлами WSL. Отдельный `gdbserver` в этом варианте не нужен. См. [официальную настройку pipeTransport](https://code.visualstudio.com/docs/cpp/pipe-transport).

Для проверки через терминал **WSL**, из локальной папки проекта:

```bash
ssh pi-dev 'mkdir -p /home/dev/projects/hello-native'
rsync -rltz --delete --exclude=/build/ --exclude=/.git/ --exclude=/.vscode/ \
    ./ pi-dev:/home/dev/projects/hello-native/
ssh pi-dev 'cd /home/dev/projects/hello-native && cmake --preset native && cmake --build build/native --parallel 1'
ssh pi-dev 'cd /home/dev/projects/hello-native && ./build/native/hello'
```

## Готовые файлы проекта

Готовый проект: [example](example). Файлы ниже создаются **на компьютере в WSL**, относительно `$HOME/projects/hello-native`. Задачи сами передают необходимые исходники на плату.

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

**`.vscode/settings.json`** ([готовый файл](example/.vscode/settings.json)):

```json
{
    "target.host": "pi-dev",
    "target.remoteDir": "/home/dev/projects/hello-native",
    "target.program": "hello",
    "target.jobs": "1",
    "target.gdb": "/usr/bin/gdb",
    "C_Cpp.default.compilerPath": "/usr/bin/gcc",
    "C_Cpp.default.cStandard": "c11",
    "C_Cpp.default.cppStandard": "c++17"
}
```

**`.vscode/tasks.json`** ([готовый файл](example/.vscode/tasks.json)):

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Prepare target",
            "type": "process",
            "command": "ssh",
            "args": [
                "-o",
                "BatchMode=yes",
                "${config:target.host}",
                "mkdir -p '${config:target.remoteDir}'"
            ],
            "problemMatcher": []
        },
        {
            "label": "Sync sources",
            "type": "process",
            "command": "rsync",
            "args": [
                "-rltz",
                "--delete",
                "--exclude=/build/",
                "--exclude=/.git/",
                "--exclude=/.vscode/",
                "-e",
                "ssh -o BatchMode=yes",
                "${workspaceFolder}/",
                "${config:target.host}:${config:target.remoteDir}/"
            ],
            "dependsOn": "Prepare target",
            "problemMatcher": []
        },
        {
            "label": "Build on target",
            "type": "process",
            "command": "ssh",
            "args": [
                "-o",
                "BatchMode=yes",
                "${config:target.host}",
                "cd '${config:target.remoteDir}' && cmake --preset native '-DAPP_NAME=${config:target.program}' && cmake --build build/native --parallel '${config:target.jobs}'"
            ],
            "dependsOn": "Sync sources",
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": []
        },
        {
            "label": "Run on target",
            "type": "process",
            "command": "ssh",
            "args": [
                "-o",
                "BatchMode=yes",
                "${config:target.host}",
                "cd '${config:target.remoteDir}' && './build/native/${config:target.program}'"
            ],
            "dependsOn": "Build on target",
            "problemMatcher": []
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
            "name": "Native: GDB on target via SSH",
            "type": "cppdbg",
            "request": "launch",
            "program": "${config:target.remoteDir}/build/native/${config:target.program}",
            "cwd": "${config:target.remoteDir}",
            "args": [],
            "MIMode": "gdb",
            "preLaunchTask": "Build on target",
            "stopAtEntry": false,
            "externalConsole": false,
            "pipeTransport": {
                "pipeProgram": "/usr/bin/ssh",
                "pipeArgs": [
                    "-T",
                    "-o",
                    "BatchMode=yes",
                    "${config:target.host}"
                ],
                "pipeCwd": "${workspaceFolder}",
                "debuggerPath": "${config:target.gdb}"
            },
            "setupCommands": [
                {
                    "text": "set substitute-path \"${config:target.remoteDir}\" \"${workspaceFolder}\""
                }
            ]
        }
    ]
}
```

## Подсказки редактора и изменения проекта

В примере IntelliSense использует GCC и заголовки **WSL**. Это подходит для навигации по небольшому примеру, но не описывает точно целевую систему ARM: размеры типов, архитектурные определения и доступные библиотеки могут отличаться. Источник результата сборки — сообщения GCC на плате. Настройка точных подсказок по целевому окружению будет рассматриваться отдельно.

Новые исходники добавляйте в `add_executable`, библиотеки — через `target_link_libraries` в `CMakeLists.txt`. Для C++ замените `main.c` на `main.cpp` в проекте и списке исходников, а `c_std_11` — на `cxx_std_17`. Пакеты разработки зависимостей устанавливаются на плате.

Аргументы программы для отладки задаются в `args` файла `.vscode/launch.json`; для запуска через задачу — в удалённой команде `Run on target` после имени программы.

В репозитории используется один корневой `.gitignore`. Если копируете пример в отдельный Git-репозиторий, добавьте правило `build/` в его корневой `.gitignore`.

## Если что-то не работает

| Симптом | Проверка |
|---|---|
| SSH-задача завершается с ошибкой | Выполнить `ssh -o BatchMode=yes pi-dev 'hostname'` из терминала WSL; проверить ключ и агент |
| Не найден rsync | Проверить установку rsync в WSL и на плате |
| Ошибка создания каталога | Проверить пользователя SSH и права записи в `target.remoteDir` |
| CMake или Ninja не найдены | Проверить инструменты именно на плате |
| Подсказки редактора расходятся со сборкой | Заголовки WSL и платы могут различаться; проверить диагностику удалённого GCC |
| Точка останова не срабатывает | Проверить Debug-сборку и `set substitute-path`, сохранить файл и пересобрать |
| GDB не запускается | Выполнить `ssh pi-dev '/usr/bin/gdb --version'` и проверить `target.gdb` |

## Границы проверки

При подготовке проверены JSON, ссылки, зависимости задач и совпадение готовых файлов с текстом. В изолированных каталогах Ubuntu/WSL проверена синхронизация rsync: устаревший исходник удаляется, исключённые `build` и `.vscode` сохраняются. Команды настройки, сборки и запуска выполнены локально для x86-64 с генератором Unix Makefiles вместо отсутствующего Ninja.

Эта проверка не подтверждает доступ по SSH, запуск GDB из VS Code и выполнение на `rpi0-2w`. Эти шаги нужно проверить на целевом устройстве.

[К туториалам](../README.md) · [К описанию проекта](../../README.md)

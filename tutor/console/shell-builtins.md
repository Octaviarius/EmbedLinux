# Встроенные команды и ключевые слова оболочек

## Содержание

- [Как читать сравнение](#как-читать-сравнение)
- [Сводная таблица](#сводная-таблица)
- [Проверка и документация](#проверка-и-документация)

## Как читать сравнение

Оболочка содержит не только встроенные команды, но и собственную грамматику. Например, `echo` — команда, а `while` начинает цикл. Оба доступны без внешней программы, но относятся к разным типам.

Ниже — объединённый список встроенных имён и ключевых слов проверенных сборок Ubuntu 24.04. Добавлены обозначения группировки и арифметической конструкции. Это справочник, а не порядок изучения синтаксиса.

| Оболочка | Проверенная версия |
|---|---|
| Bash | 5.2.21 |
| dash | 0.5.12 |
| BusyBox ash | 1.36.1 |
| Zsh | 5.9 |
| Fish | 3.7.0 |

`sh` не выделен в отдельный столбец: под этим именем запускается конкретная реализация, например dash.

**Значения ячеек:**

- `builtin` — встроенная команда интерпретатора.
- `keyword` — ключевое слово, которое разбирается как часть языка.
- `syntax` — составная конструкция или контекстный разделитель, а не отдельная команда.
- `modifier` — модификатор запуска следующей команды в Fish.
- `function` — функция из стандартной поставки Fish, доступная под этим именем.
- `—` — в проверенном наборе такого имени или конструкции нет. Внешние программы и пользовательские настройки здесь не учитываются.

Тип указан для обычного использования имени. Некоторые имена также зарегистрированы в builtin-реестре, хотя участвуют в грамматике: здесь они обозначены как `keyword` или `modifier`. Например, `while` в Fish есть в `builtin --names`, но в таблице отмечен как `keyword`; `export` в Zsh — ключевое слово при обычном разборе и одновременно имеет встроенную реализацию.

`help` в Bash охватывает и команды, и конструкции языка. Поэтому число тем `help` не равно числу builtin-команд: в проверенном Bash их 61, в dash — 39. Запись `while` теперь явно показывает наличие цикла, а не отсутствие встроенной команды с таким именем.

В Zsh учтены встроенные имена, зарегистрированные без пользовательской конфигурации, включая автоматически загружаемые модули; дополнительные модули расширяют набор. В BusyBox ash список зависит от сборки. Остальные апплеты BusyBox, например `ls`, не включены. В Fish стандартные функции отмечены только для имён, уже вошедших в сравнение; это не каталог всех функций Fish.

Совпадение имени и типа не гарантирует одинаковые параметры или синтаксис между оболочками.

## Сводная таблица

| Имя | Bash | dash | BusyBox<br>ash | Zsh | Fish |
|---|---|---|---|---|---|
| `!` | keyword | keyword | keyword | keyword | keyword |
| `( ... )` | syntax | syntax | syntax | syntax | — |
| `(( ... ))` | syntax | — | — | syntax | — |
| `-` | — | — | — | builtin | — |
| `.` | builtin | builtin | builtin | builtin | builtin |
| `:` | builtin | builtin | builtin | builtin | builtin |
| `[` | builtin | builtin | builtin | builtin | builtin |
| `[[` | keyword | — | builtin | keyword | — |
| `]]` | keyword | — | — | syntax | — |
| `_` | — | — | — | — | builtin |
| `abbr` | — | — | — | — | builtin |
| `alias` | builtin | builtin | builtin | builtin | function |
| `and` | — | — | — | — | keyword |
| `argparse` | — | — | — | — | builtin |
| `autoload` | — | — | — | builtin | — |
| `begin` | — | — | — | — | keyword |
| `bg` | builtin | builtin | builtin | builtin | builtin |
| `bind` | builtin | — | — | — | builtin |
| `bindkey` | — | — | — | builtin | — |
| `block` | — | — | — | — | builtin |
| `break` | builtin | builtin | builtin | builtin | builtin |
| `breakpoint` | — | — | — | — | builtin |
| `builtin` | builtin | — | — | builtin | modifier |
| `bye` | — | — | — | builtin | — |
| `caller` | builtin | — | — | — | — |
| `case` | keyword | keyword | keyword | keyword | keyword |
| `cd` | builtin | builtin | builtin | builtin | builtin |
| `chdir` | — | builtin | builtin | builtin | — |
| `command` | builtin | builtin | builtin | builtin | modifier |
| `commandline` | — | — | — | — | builtin |
| `compadd` | — | — | — | builtin | — |
| `comparguments` | — | — | — | builtin | — |
| `compcall` | — | — | — | builtin | — |
| `compctl` | — | — | — | builtin | — |
| `compdescribe` | — | — | — | builtin | — |
| `compfiles` | — | — | — | builtin | — |
| `compgen` | builtin | — | — | — | — |
| `compgroups` | — | — | — | builtin | — |
| `complete` | builtin | — | — | — | builtin |
| `compopt` | builtin | — | — | — | — |
| `compquote` | — | — | — | builtin | — |
| `compset` | — | — | — | builtin | — |
| `comptags` | — | — | — | builtin | — |
| `comptry` | — | — | — | builtin | — |
| `compvalues` | — | — | — | builtin | — |
| `contains` | — | — | — | — | builtin |
| `continue` | builtin | builtin | builtin | builtin | builtin |
| `coproc` | keyword | — | — | keyword | — |
| `count` | — | — | — | — | builtin |
| `declare` | builtin | — | — | keyword | — |
| `dirs` | builtin | — | — | builtin | function |
| `disable` | — | — | — | builtin | — |
| `disown` | builtin | — | — | builtin | builtin |
| `do` | keyword | keyword | keyword | keyword | — |
| `done` | keyword | keyword | keyword | keyword | — |
| `echo` | builtin | builtin | builtin | builtin | builtin |
| `echotc` | — | — | — | builtin | — |
| `echoti` | — | — | — | builtin | — |
| `elif` | keyword | keyword | keyword | keyword | — |
| `else` | keyword | keyword | keyword | keyword | keyword |
| `emit` | — | — | — | — | builtin |
| `emulate` | — | — | — | builtin | — |
| `enable` | builtin | — | — | builtin | — |
| `end` | — | — | — | keyword | keyword |
| `esac` | keyword | keyword | keyword | keyword | — |
| `eval` | builtin | builtin | builtin | builtin | builtin |
| `exec` | builtin | builtin | builtin | builtin | modifier |
| `exit` | builtin | builtin | builtin | builtin | builtin |
| `export` | builtin | builtin | builtin | keyword | function |
| `false` | builtin | builtin | builtin | builtin | builtin |
| `fc` | builtin | — | — | builtin | — |
| `fg` | builtin | builtin | builtin | builtin | builtin |
| `fi` | keyword | keyword | keyword | keyword | — |
| `float` | — | — | — | keyword | — |
| `for` | keyword | keyword | keyword | keyword | keyword |
| `foreach` | — | — | — | keyword | — |
| `function` | keyword | — | keyword | keyword | keyword |
| `functions` | — | — | — | builtin | builtin |
| `getln` | — | — | — | builtin | — |
| `getopts` | builtin | builtin | builtin | builtin | — |
| `hash` | builtin | builtin | builtin | builtin | — |
| `help` | builtin | — | builtin | — | function |
| `history` | builtin | — | builtin | builtin | builtin |
| `if` | keyword | keyword | keyword | keyword | keyword |
| `in` | keyword | keyword | keyword | syntax | syntax |
| `integer` | — | — | — | keyword | — |
| `jobs` | builtin | builtin | builtin | builtin | builtin |
| `kill` | builtin | builtin | builtin | builtin | — |
| `let` | builtin | — | builtin | builtin | — |
| `limit` | — | — | — | builtin | — |
| `local` | builtin | builtin | builtin | keyword | — |
| `log` | — | — | — | builtin | — |
| `logout` | builtin | — | — | builtin | — |
| `mapfile` | builtin | — | — | — | — |
| `math` | — | — | — | — | builtin |
| `nocorrect` | — | — | — | keyword | — |
| `noglob` | — | — | — | builtin | — |
| `not` | — | — | — | — | keyword |
| `or` | — | — | — | — | keyword |
| `path` | — | — | — | — | builtin |
| `popd` | builtin | — | — | builtin | function |
| `print` | — | — | — | builtin | — |
| `printf` | builtin | builtin | builtin | builtin | builtin |
| `private` | — | — | — | builtin | — |
| `pushd` | builtin | — | — | builtin | function |
| `pushln` | — | — | — | builtin | — |
| `pwd` | builtin | builtin | builtin | builtin | builtin |
| `r` | — | — | — | builtin | — |
| `random` | — | — | — | — | builtin |
| `read` | builtin | builtin | builtin | builtin | builtin |
| `readarray` | builtin | — | — | — | — |
| `readonly` | builtin | builtin | builtin | keyword | — |
| `realpath` | — | — | — | — | builtin |
| `rehash` | — | — | — | builtin | — |
| `repeat` | — | — | — | keyword | — |
| `return` | builtin | builtin | builtin | builtin | builtin |
| `sched` | — | — | — | builtin | — |
| `select` | keyword | — | — | keyword | — |
| `set` | builtin | builtin | builtin | builtin | builtin |
| `set_color` | — | — | — | — | builtin |
| `setopt` | — | — | — | builtin | — |
| `shift` | builtin | builtin | builtin | builtin | — |
| `shopt` | builtin | — | — | — | — |
| `sleep` | — | — | builtin | — | — |
| `source` | builtin | — | builtin | builtin | builtin |
| `status` | — | — | — | — | builtin |
| `string` | — | — | — | — | builtin |
| `suspend` | builtin | — | — | builtin | function |
| `switch` | — | — | — | — | keyword |
| `test` | builtin | builtin | builtin | builtin | builtin |
| `then` | keyword | keyword | keyword | keyword | — |
| `time` | keyword | — | — | keyword | keyword |
| `times` | builtin | builtin | builtin | builtin | — |
| `trap` | builtin | builtin | builtin | builtin | function |
| `true` | builtin | builtin | builtin | builtin | builtin |
| `ttyctl` | — | — | — | builtin | — |
| `type` | builtin | builtin | builtin | builtin | builtin |
| `typeset` | builtin | — | — | keyword | — |
| `ulimit` | builtin | builtin | builtin | builtin | builtin |
| `umask` | builtin | builtin | builtin | builtin | function |
| `unalias` | builtin | builtin | builtin | builtin | — |
| `unfunction` | — | — | — | builtin | — |
| `unhash` | — | — | — | builtin | — |
| `unlimit` | — | — | — | builtin | — |
| `unset` | builtin | builtin | builtin | builtin | — |
| `unsetopt` | — | — | — | builtin | — |
| `until` | keyword | keyword | keyword | keyword | — |
| `vared` | — | — | — | builtin | function |
| `wait` | builtin | builtin | builtin | builtin | builtin |
| `whence` | — | — | — | builtin | — |
| `where` | — | — | — | builtin | — |
| `which` | — | — | — | builtin | — |
| `while` | keyword | keyword | keyword | keyword | keyword |
| `zcompile` | — | — | — | builtin | — |
| `zformat` | — | — | — | builtin | — |
| `zle` | — | — | — | builtin | — |
| `zmodload` | — | — | — | builtin | — |
| `zparseopts` | — | — | — | builtin | — |
| `zregexparse` | — | — | — | builtin | — |
| `zstyle` | — | — | — | builtin | — |
| `{` | keyword | keyword | keyword | keyword | — |
| `}` | keyword | keyword | keyword | keyword | — |

## Проверка и документация

Списки сверены с интерпретаторами в WSL:`compgen -b` и список ключевых слов Bash, `type` в dash и ash, реестры встроенных имён и ключевых слов Zsh, `builtin --names` и стандартные функции Fish. Классификация управляющих конструкций Fish дополнительно сверена с его грамматикой.

Документация: [встроенные команды Bash](https://www.gnu.org/software/bash/manual/html_node/Shell-Builtin-Commands.html), [dash](https://manpages.debian.org/testing/dash/dash.1.en.html), [BusyBox](https://busybox.net/FAQ.html), [команды Zsh](https://zsh.sourceforge.io/Doc/Release/Shell-Builtin-Commands.html) и [грамматика Zsh](https://zsh.sourceforge.io/Doc/Release/Shell-Grammar.html), [язык Fish](https://fishshell.com/docs/3.7/language.html) и [список встроенных имён Fish](https://fishshell.com/docs/3.7/cmds/builtin.html).

[К разделу об оболочках](syntax.md#оболочки-и-их-различия) · [К серии уроков](README.md).

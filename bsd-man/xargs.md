# macos xargs

## 名称

xargs -- 构造参数列表并执行命令

## 概要

xargs [-0oprt] [-E eofstr] [-I replstr [-R replacements] [-S replsize]] [-J replstr] [-L number] [-n number [-x]] [-P maxprocs] [-s size] [utility [argument ...]]

## 描述

xargs 命令从标准输入中读取空格、制表符、换行符和文件结尾分隔的字符串，并使用这些字符串作为参数执行命令。

命令行上指定的任何参数在每次调用时都会提供给命令，然后是从 xargs 的标准输入读取的一些参数。重复此操作，直到用尽标准输入。

空格、制表符和换行符可以使用单引号 ` (`` ' '') ` 或双引号 ` (``"'') ` 或反斜杠 ` (``\'') ` 嵌入到参数中。单引号转义所有非单引号字符，不包括换行符，直到匹配的单引号。双引号转义所有非双引号字符，不包括换行符，直到匹配的双引号。任何单个字符，包括换行符，都可以用反斜杠转义。

选项如下：

- -0

  将 xargs 更改为期望 NUL (``\0'') 字符作为分隔符，而不是空格和换行符。这预计将与 find(1) 中的 -print0 函数一起使用。(find -print0 指定文件列表以 null 分隔)

  ```
  find /path -type f -print0 | xargs -0 rm
  ```

- -E eofstr

  使用 eofstr 作为逻辑 EOF 标记。

- -I replstr

  为每个输入行执行命令，将一个或多个 replstr 替换为最多替换（如果未指定 -R 标志，则为 5 个）参数到命令的整行输入。替换完成后，生成的参数将不允许超过 replsize（如果未指定 -S 标志，则为 255）字节；这是通过将尽可能多的包含 replstr 的参数连接到命令的构造参数来实现的，最多为 replsize 字节。大小限制不适用于不包含 replstr 的命令参数，此外，命令本身不会进行替换。意味着 -x。

  ```
  cat foo.txt | xargs -I file sh -c 'echo file; mkdir file'
  ```

- J replstr

  如果指定了此选项，xargs 将使用从标准输入读取的数据来替换第一次出现的 replstr，而不是在所有其他参数之后附加该数据。此选项不会影响将从输入 (-n) 读取的参数数量，或 xargs 将生成的命令大小 (-s)。该选项只是移动这些参数将放置在执行的命令中的位置。replstr 必须显示为 xargs 的不同参数。例如，如果它位于带引号的字符串的中间，则不会被识别。此外，只会替换第一次出现的 replstr。例如，以下命令会将当前目录中以大写字母开头的文件和目录列表复制到 destdir：(ps: 感觉跟-I 差不多)

  ```
  ls -1d [A-Z]* | xargs -J % cp -Rp % destdir
  ```

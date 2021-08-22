# GN 简单的构建示例
来源：https://gn.googlesource.com/gn/
api文档：https://gn.googlesource.com/gn/+/HEAD/docs/reference.md#

这是一个用gcc来编译一些简单项目的示例。它用来展示如何配置一个简单的GN构建。因为故意简单所以目录结构更清晰，但不支持所有平台。

## 前言
GN是一个元构建系统，用来生成Ninja文件

## GN 构建指南
来源：https://gn.googlesource.com/gn/+/HEAD/docs/quick_start.md

### 配置构建
与其他一些构建系统不同，使用 GN 您可以使用所需的设置设置自己的构建目录。这使您可以根据需要并行维护尽可能多的不同构建。
一旦您设置了构建目录，当您在该目录中构建时，如果 Ninja 文件已过期，它们将自动重新生成，因此您不必重新运行 GN。
生成构建目录
```
gn gen out/my_build
```

### 传递构建参数
设置构建参数在运行构建目录上
```
gn args out/my_build
```
然后会打开一个编辑器，像这样在该文件输入
```
is_component_build = true
is_debug = false
```
可用变量将取决于您的构建（此示例来自 Chromium）。您可以通过键入来查看可用参数及其默认值的列表
```
gn args --list out/my_build
```
在命令行上。请注意，您必须为此命令指定构建目录，因为可用参数可能会根据构建而改变。
Chrome 开发人员还可以阅读 [Chrome 特定的构建配置说明](http://www.chromium.org/developers/gn-build-configuration)以获取更多信息。
### 交叉编译到目标操作系统或架构
运行 gn args out/Default （根据需要替换您的构建目录）并为常见的交叉编译选项添加以下一行或多行。
```
target_os = "chromeos"
target_os = "android"

target_cpu = "arm"
target_cpu = "x86"
target_cpu = "x64"
```
有关更多信息，请参阅 [GN 交叉编译](https://gn.googlesource.com/gn/+/HEAD/docs/cross_compiles.md)。

### 一步步来
转到目录gn_example。这是最小 GN 仓库。
在该目录中有一个 tutorial 目录。已经有一个 tutorial.cc 文件。在该目录中为我们的新目标创建一个新的 BUILD.gn 文件。
```
executable("tutorial") {
  sources = [
    "tutorial.cc",
  ]
}
```
现在我们只需要告诉构建这个新目标。打开父 (gn_example) 目录中的 BUILD.gn 文件。 GN 首先加载这个根文件，然后从这里加载我们所有的依赖项，所以我们只需要从这个文件中添加一个对我们新目标的引用。
您可以将我们的新目标添加为 gn_example/BUILD.gn 文件中现有目标之一的依赖项，但将可执行文件作为另一个可执行文件的依赖项通常没有多大意义（他们不能被链接）。因此，让我们创建一个“工具”组。在 GN "group" 只是未编译或链接的依赖项集合：
```
group("tools") {
  deps = [
    # This will expand to the name "//tutorial:tutorial" which is the full name
    # of our new target. Run "gn help labels" for more.
    "//tutorial",
  ]
}
```
### 测试您的添加
```
./bin/gn gen out
ninja -C out tutorial
out/tutorial
```
你应该看到“Hello from the tutorial.“输出到控制台。

旁注：GN 鼓励静态库的目标名称不是全局唯一的。要构建其中之一，您可以将标签及其路径（但没有前导“//”）传递给 ninja：
```
ninja -C out some/path/to/target:my_target
```
### 声明依赖
让我们看看在 gn_examples/BUILD.gn 中定义的目标。有一个静态库定义了一个函数 GetStaticText()
```
static_library("hello_static") {
  sources = [
    "hello_static.cc",
    "hello_static.h",
  ]
}
```
还有一个共享库定义了一个函数 GetSharedText()：
```
shared_library("hello_shared") {
  sources = [
    "hello_shared.cc",
    "hello_shared.h",
  ]

  defines = [ "HELLO_SHARED_IMPLEMENTATION" ]
}
```
这也说明了如何为目标设置预处理器定义。要设置多个或分配值，请使用以下形式：
```
defines = [
  "HELLO_SHARED_IMPLEMENTATION",
  "ENABLE_DOOM_MELON=0",
]
```
现在让我们看看依赖于这两个库的可执行文件：
```
executable("hello") {
  sources = [
    "hello.cc",
  ]

  deps = [
    ":hello_shared",
    ":hello_static",
  ]
}
```
此可执行文件包含一个源文件并依赖于前两个库。以冒号开头的标签指的是当前 BUILD.gn 文件中具有该名称的目标。
### 测试二进制库
从 gn_example 目录中的命令行：
```
ninja -C out hello
out/hello
```
请注意，您不需要重新运行 GN。当任何构建文件发生更改时，GN 将自动重建 ninja 文件。您知道当 ninja 在执行开始时打印 [1/1] 重新生成 ninja 文件时会发生这种情况。

### 将设置放入配置
库的用户通常需要编译器标志、定义和包含应用于他们的目录。为此，请将所有此类设置放入一个“config”，它是一个命名的设置集合（但不是源或依赖项）：
```
config("my_lib_config") {
  defines = [ "ENABLE_DOOM_MELON" ]
  include_dirs = [ "//third_party/something" ]
}
```
要将配置的设置应用于目标，请将其添加到配置列表：
```
static_library("hello_shared") {
  ...
  # Note "+=" here is usually required, see "default configs" below.
  configs += [
    ":my_lib_config",
  ]
}
```
通过将其标签放在 public_configs 列表中，可以将配置应用于所有依赖于当前目标的目标：
```
static_library("hello_shared") {
  ...
  public_configs = [
    ":my_lib_config",
  ]
}
```
public_configs 也适用于当前目标，因此无需在两个地方都列出配置。

### 默认配置
默认情况下，构建配置将设置一些适用于每个目标的设置。这些通常会被设置为默认的配置列表。您可以使用对调试很有用的“print”命令来查看这一点：
```
executable("hello") {
  print(configs)
}
```
运行 GN 将打印如下内容：
```
$ gn gen out
["//build:compiler_defaults", "//build:executable_ldconfig"]
Done. Made 5 targets from 5 files in 9ms
```
目标可以修改此列表以更改其默认值。例如，默认情况下，构建设置可能会通过添加 no_exceptions 配置来关闭异常，但目标可能会通过将其替换为不同的配置来重新启用它们：
```
executable("hello") {
  ...
  configs -= [ "//build:no_exceptions" ]  # Remove global default.
  configs += [ "//build:exceptions" ]  # Replace with a different one.
}
```
我们上面的打印命令也可以使用字符串插值来表达。这是一种将值转换为字符串的方法。它使用符号“$”来指代一个变量：
```
print("The configs for the target $target_name are $configs")
```

### 添加一个新的构建参数
您可以通过declare_args 声明您接受哪些参数并指定默认值。
```
declare_args() {
  enable_teleporter = true
  enable_doom_melon = false
}
```
请参阅 `gn help buildargs` 以了解其工作原理。有关声明它们的详细信息，请参阅 `gn help declare_args`。
在给定范围内多次声明给定参数是错误的，因此在定义范围和命名参数时应小心。

### 不知道是怎么回事？
您可以在详细模式下运行 GN 以查看有关其正在执行的操作的大量消息。为此使用 -v。
**“desc”命令**
您可以运行 gn desc <build_dir> <targetname> 来获取有关给定目标的信息：
```
gn desc out/Default //foo/bar:say_hello
```
将打印出许多令人兴奋的信息。您也可以只打印一个部分。假设您想知道您的 TWO_PEOPLE 定义来自 say_hello 目标的何处：
```
> gn desc out/Default //foo/bar:say_hello defines --blame
...lots of other stuff omitted...
  From //foo/bar:hello_config
       (Added by //foo/bar/BUILD.gn:12)
    TWO_PEOPLE
```
另一个特别有趣的变化：
```
gn desc out/Default //base:base_i18n deps --tree
```
有关更多信息，请参阅 gn 帮助说明。
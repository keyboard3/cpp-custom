# 简单的Makefile教程
来源：https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

## 前言
Makefile 是一种组织代码编译的简单方法。本教程甚至没有涉及到可以用 make 干什么，而是旨在作为入门指南，以便您可以快速轻松地为中小型项目创建自己的 makefile。

## 简单的示例
让我们从以下三个文件开始，[hellomake.c](./hellomake.c)、[hellofunc.c](./hellofunc.c) 和 [hellomake.h](./hellomake.h)，它们分别代表一个典型的主程序、单独文件中的一些功能代码和一个包含文件。

通常，您将通过执行以下命令来编译此代码集合：
```
gcc -o hellomake hellomake.c hellofunc.c -I.
```

这将编译两个 .c 文件并将可执行文件命名为 hellomake。 -I 包含在内，以便 gcc 在当前目录 (.) 中查找包含文件 hellomake.h。如果没有 makefile，测试/修改/调试周期的典型方法是使用终端中的向上箭头返回到上次编译命令，这样您就不必每次都键入它，尤其是在您添加之后再添加一些 .c 文件。

不幸的是，这种编译方法有两个缺点。首先，如果您丢失了编译命令或切换计算机，则必须从头开始重新键入，这充其量是低效的。其次，如果您只对一个 .c 文件进行更改，那么每次都重新编译所有这些文件也是费时且低效的。所以，是时候看看我们可以用 makefile 做什么了

您可以创建的最简单的 makefile 如下所示：
```Makefile
hellomake: hellomake.c hellofunc.c
	gcc -o hellomake hellomake.c hellofunc.c -I.
```
如果您将此规则放入名为 Makefile 或 makefile 的文件中，然后在命令行中键入 make，它将执行您在 makefile 中编写的编译命令。请注意，不带参数的 make 执行文件中的第一条规则。此外，通过将命令所依赖的文件列表放在 : 之后的第一行，make 知道如果这些文件中的任何一个发生更改，则需要执行规则 hellomake。立即，您已经解决了问题 #1，并且可以避免重复使用向上箭头，查找您的最后一个编译命令。但是，该系统在仅编译最新更改方面仍然效率不高。

需要注意的一件非常重要的事情是 makefile 中 gcc 命令之前有一个tab。任何命令的开头都必须有一个制表符，如果不存在，make 不会高兴。

为了提高效率，让我们尝试以下操作：
```Makefile
CC=gcc
CFLAGS=-I.

hellomake: hellomake.o hellofunc.o
     $(CC) -o hellomake hellomake.o hellofunc.o
```
所以现在我们已经定义了一些常量 CC 和 CFLAGS。事实证明，这些是特殊的常量，它们与我们希望如何编译文件 hellomake.c 和 hellofunc.c 进行通信。特别是，宏 CC 是要使用的 C 编译器，而 CFLAGS 是要传递给编译命令的标志列表。通过将目标文件——hellomake.o 和 hellofunc.o——放在依赖列表和规则中，make 知道它必须首先单独编译 .c 版本，然后构建可执行文件 hellomake。

对于大多数小型项目，使用这种形式的 makefile 就足够了。但是，缺少一件事：对include file的依赖。例如，如果您要对 hellomake.h 进行更改，make 将不会重新编译 .c 文件，即使它们需要重新编译。为了解决这个问题，我们需要告诉 make 所有 .c 文件都依赖于某些 .h 文件。我们可以通过编写一个简单的规则并将其添加到 makefile 中来做到这一点。
```Makefile
CC=gcc
CFLAGS=-I.
DEPS = hellomake.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

hellomake: hellomake.o hellofunc.o 
	$(CC) -o hellomake hellomake.o hellofunc.o 
```

此添加首先创建宏 DEPS，它是 .c 文件所依赖的 .h 文件集。然后我们定义一个规则，适用于所有以 .o 后缀结尾的文件。该规则表示 .o 文件取决于文件的 .c 版本和 DEPS 宏中包含的 .h 文件。然后规则说要生成 .o 文件，make 需要使用 CC 宏中定义的编译器编译 .c 文件。 -c 标志表示生成目标文件，-o `$@` 表示将编译的输出放在 : 左侧命名的文件中，$< 是依赖项列表中的第一项，而CFLAGS 宏定义如上。

作为最后的简化，让我们使用特殊的宏 `$@` 和 $^，它们分别是 : 的左侧和右侧，以使整体编译规则更加通用。在下面的示例中，所有include file都应列为宏 DEPS 的一部分，所有目标文件都应列为宏 OBJ 的一部分。
```Makefile
CC=gcc
CFLAGS=-I.
DEPS = hellomake.h
OBJ = hellomake.o hellofunc.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

hellomake: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
```
那么，如果我们想开始将 .h 文件放在 include 目录中，将源代码放在 src 目录中，将一些本地库放在 lib 目录中，该怎么办？另外，我们能否以某种方式隐藏那些无处不在的烦人的 .o 文件？答案当然是肯定的。以下 makefile 定义了 include 和 lib 目录的路径，并将目标文件放置在 src 目录中的 obj 子目录中。它还为您想要include的任何库定义了一个宏，例如数学库 -lm。此生成文件应位于 src 目录中。请注意，如果您键入 make clean，它还包括用于清理源目录和对象目录的规则。 .PHONY 规则阻止 make 对名为 clean 的文件执行某些操作。
```Makefile
IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=obj
LDIR =../lib

LIBS=-lm

_DEPS = hellomake.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = hellomake.o hellofunc.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

hellomake: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
```
所以现在你有一个完美的 makefile，你可以修改它来管理中小型软件项目。你可以在一个 makefile 中添加多个规则；您甚至可以创建调用其他规则的规则。有关 makefile 和 make 函数的更多信息，请查看 [GNU Make 手册](http://www.gnu.org/software/make/manual/make.html)，它会告诉您比您想知道的更多（真的）。

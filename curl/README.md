# libcurl 编程指南
## 目标

本文档试图描述使用 libcurl 进行编程时要考虑的一般原则和一些基本方法。主要关注 C 接口，但也可能适用于其他接口，因为它们通常非常接近 C 接口。

本文档将“用户”称为编写使用 libcurl 的源代码的人。那可能是您或您所在职位的某个人。通常称为“程序”的是您编写的使用 libcurl 进行传输的收集源代码。程序在 libcurl 之外，libcurl 在程序之外

要获得有关此处描述的所有选项和功能的更多详细信息，请参阅它们各自的手册页。
## 构建

有许多不同的方法来构建 C 程序。本章将假设一个 Unix 风格的构建过程。如果您使用不同的构建系统，您仍然可以阅读本文以获取可能适用于您的环境的一般信息。

### 编译程序

您的编译器需要知道 libcurl 头文件的位置。因此，您必须将编译器的包含路径设置为指向安装它们的目录。 'curl-config'[3] 工具可用于获取此信息：
``` shell
$ curl-config --cflags
```

### 将程序与 libcurl 链接

编译程序后，您需要链接目标文件以创建单个可执行文件。为此，您需要与 libcurl 以及 libcurl 本身所依赖的其他库进行链接。与 OpenSSL 库类似，但在命令行上甚至可能需要一些标准操作系统库。要确定要使用哪些标志，“curl-config”工具再次派上用场：
```
$ curl-config --libs
```

### SSL 与否

libcurl 可以通过多种方式构建和定制。不同库和构建的不同之处之一是支持基于 SSL 的传输，如 HTTPS 和 FTPS。如果在构建时正确检测到支持的 SSL 库，libcurl 将使用 SSL 支持构建。要确定已安装的 libcurl 是否是在启用 SSL 支持的情况下构建的，请使用“curl-config”，如下所示：

```
$ curl-config --feature
```

如果支持 SSL，关键字“SSL”将被写入标准输出，可能与一些其他功能一起写入，这些功能可以针对不同的 libcurl 打开或关闭。
另请参阅下面的“功能 libcurl 提供”。

### 自动配置宏

当您编写配置脚本来相应地检测 libcurl 和设置变量时，我们提供了一个预写的宏，它可能会完成您在这方面所需的一切。请参阅 docs/libcurl/libcurl.m4 文件 - 它包含有关如何使用它的文档。

## 可移植代码

libcurl 背后的人付出了相当大的努力，使 libcurl 能够在大量不同的操作系统和环境中运行。

您可以在运行 libcurl 的所有平台上以相同的方式对 libcurl 进行编程。只有很少的小考虑不同。如果您只是确保编写的代码具有足够的可移植性，那么您很可能会为自己创建一个非常可移植的程序。 libcurl 不应该阻止你这样做。

## 全局准备
该程序必须全局初始化一些 libcurl 功能。这意味着它应该只执行一次，无论您打算使用该库多少次。一次在您的程序的整个生命周期内。这是使用
```c++
curl_global_init()
```
它需要一个参数，它是一个位模式，告诉 libcurl 初始化什么。使用 CURL_GLOBAL_ALL 将使其初始化所有已知的内部子模块，并且可能是一个很好的默认选项。当前指定的两位是：

- CURL_GLOBAL_WIN32
它只在 Windows 机器上做任何事情。在 Windows 机器上使用时，它会使 libcurl 初始化 win32 socket内容。如果没有正确初始化，您的程序就无法正确使用socket。对于每个应用程序，您应该只执行一次此操作，因此如果您的程序已经执行此操作或正在使用的其他库执行此操作，则不应告诉 libcurl 也执行此操作。

- CURL_GLOBAL_SSL
它只对编译和构建启用 SSL 的 libcurl 执行任何操作。在这些系统上，这将使 libcurl 为此应用程序正确初始化 SSL 库。对于每个应用程序只需要执行一次，因此如果您的程序或其他库已经执行此操作，则不需要此位。

libcurl 有一个默认的保护机制，它检测 curl_easy_perform 被调用时 curl_global_init 是否没有被调用，如果是这种情况，libcurl 会使用猜测的位模式运行函数本身。请注意，仅仅依赖于这一点不被认为是好的也不是很好。

当程序不再使用 libcurl 时，它应该调用 curl_global_cleanup，这与 init 调用相反。然后它将执行相反的操作来清理 curl_global_init 调用初始化的资源。

应避免重复调用 curl_global_init 和 curl_global_cleanup。它们每个只能被调用一次。

## libcurl 提供的功能
在运行时而不是在构建时（当然，如果可能的话）确定 libcurl 功能被认为是最佳实践。通过调用 curl_version_info 并检查返回结构的详细信息，您的程序可以准确地确定当前运行的 libcurl 支持什么。

## 两个接口
libcurl 首先介绍了所谓的简单接口。 easy 接口中的所有操作都以“curl_easy”为前缀。easy接口让您可以通过同步和阻塞函数调用进行单次传输。

libcurl 还提供了另一个接口，允许在单个线程中同时进行多个传输，即所谓的多接口。有关该接口的更多信息，请参阅下面的单独章节。您仍然需要先了解easy接口，所以请继续阅读以获得更好的理解。

## 处理easy libcurl
要使用easy接口，您必须首先为自己创建一个easy句柄。对于要执行的每个简单会话，您都需要一个句柄。基本上，您应该为计划用于传输的每个线程使用一个句柄。您绝不能在多个线程中共享同一个句柄。

获得一个easy句柄
```c++
easyhandle = curl_easy_init();
```
它返回一个easy句柄。使用它继续下一步：设置您的首选操作。句柄只是即将进行的传输或一系列传输的逻辑实体。

您可以使用 curl_easy_setopt 为该句柄设置属性和选项。他们控制如何进行后续转移。选项在句柄中保持设置，直到再次设置为不同的内容。它们很 sticky。使用相同句柄的多个请求将使用相同的选项。

如果您在任何时候想要为单个 easy 句柄清除所有先前设置的选项，您可以调用 curl_easy_reset 并且您还可以使用 curl_easy_duphandle 制作一个 easy 句柄的克隆（及其所有设置选项）。

您在 libcurl 中设置的许多选项都是“字符串”，即指向以零字节结尾的数据的指针。当您使用 curl_easy_setopt 设置字符串时，libcurl 会制作自己的副本，以便在设置后无需将它们保留在您的应用程序中 [4]。

要在句柄中设置的最基本的属性之一是 URL。您将首选 URL 设置为使用 CURLOPT_URL 以类似于以下方式进行传输：
```c++
curl_easy_setopt(handle, CURLOPT_URL, "http://domain.com/");
```

让我们假设您想接收数据，因为 URL 标识了您想在此处获取的远程资源。由于您编写了一种需要这种传输的应用程序，我假设您希望将数据直接传递给您，而不是简单地将其传递给 stdout。因此，您编写自己的与此原型匹配的函数：

```c++
size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp);
```
您可以通过发出类似于以下内容的函数来告诉 libcurl 将所有数据传递给该函数：

```c++
curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, write_data);
```
您可以通过设置另一个属性来控制回调函数在第四个参数中获取的数据：
```c++
curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, &internal_struct);
```
使用该属性，您可以轻松地在应用程序和由 libcurl 调用的函数之间传递本地数据。 libcurl 本身不会触及您通过 CURLOPT_WRITEDATA 传递的数据。

libcurl 提供了自己的默认内部回调，如果您不使用 CURLOPT_WRITEFUNCTION 设置回调，它将处理数据。然后它会简单地将接收到的数据输出到标准输出。您可以通过将“FILE *”传递给使用 CURLOPT_WRITEDATA 选项打开以进行写入的文件，让默认回调将数据写入不同的文件句柄。

现在，我们需要退后一步，深呼吸。这是那些罕见的依赖于平台的挑剔之一。你发现了吗？在某些平台上[2]，libcurl 将无法对程序打开的文件进行操作。因此，如果您使用默认回调并使用 CURLOPT_WRITEDATA 传入打开的文件，它将崩溃。因此，您应该避免这种情况，以使您的程序几乎在任何地方都能正常运行。

（CURLOPT_WRITEDATA 以前称为 CURLOPT_FILE。这两个名称仍然有效并执行相同的操作）。

如果您使用 libcurl 作为 win32 DLL，则必须在设置 CURLOPT_WRITEDATA 时使用 CURLOPT_WRITEFUNCTION - 否则您将遇到崩溃。

当然，您还可以设置更多选项，我们稍后再讨论其中的几个。让我们继续进行实际的传输：
```c++
success = curl_easy_perform(easyhandle);
```

curl_easy_perform 将连接到远程站点，执行必要的命令并接收传输。每当它接收到数据时，它就会调用我们之前设置的回调函数。该函数可能一次获得一个字节，也可能一次获得许多千字节。 libcurl 尽可能多地提供尽可能多的服务。您的回调函数应该返回它“处理”的字节数。如果这与传递给它的字节数不完全相同，libcurl 将中止操作并返回错误代码。

传输完成后，该函数会返回一个返回code，通知您它是否成功完成了任务。如果返回code对您来说不够用，您可以使用 CURLOPT_ERRORBUFFER 将 libcurl 指向您的缓冲区，它也将存储人类可读的错误消息。

如果您随后想要传输另一个文件，则该句柄已准备好再次使用。请注意，如果您打算进行另一次转移，最好重新使用现有的句柄。 libcurl 然后将尝试重新使用以前的连接。

对于某些协议，下载文件可能涉及一个复杂的过程，包括登录、设置传输模式、更改当前目录以及最终传输文件数据。 libcurl 会为您处理所有这些复杂问题。简单地给出文件的 URL，libcurl 将处理将文件从一台机器移动到另一台机器所需的所有细节。

## 多线程问题
libcurl 是线程安全的，但有一些例外。有关更多信息，请参阅 libcurl-thread。

## 当它不起作用时
总会有一些时候由于某种原因传输失败。您可能设置了错误的 libcurl 选项或误解了 libcurl 选项的实际作用，或者远程服务器可能返回非标准回复，混淆了库，然后混淆了您的程序。

当这些事情发生时，有一个黄金法则：将 CURLOPT_VERBOSE 选项设置为 1。它会导致库喷出它发送的整个协议细节，一些内部信息和一些接收到的协议数据（尤其是在使用 FTP 时）。如果您使用的是 HTTP，则在接收到的输出中添加 headers 以进行研究也是一种聪明的方法，可以更好地了解服务器的行为方式。在 CURLOPT_HEADER 设置为 1 的正常正文输出中包含headers。

当然，还有一些bug。我们需要了解它们才能修复它们，因此我们非常依赖您的错误报告！当您报告 libcurl 中的可疑错误时，请尽可能多地提供详细信息：CURLOPT_VERBOSE 生成的协议转储、库版本、尽可能多的使用 libcurl 的代码、操作系统名称和版本、编译器名称和版本等等。

如果 CURLOPT_VERBOSE 不够用，您可以使用 CURLOPT_DEBUGFUNCTION 增加应用程序接收的调试数据的级别。

深入了解所涉及的协议永远不会错，如果您正在尝试做一些有趣的事情，那么如果您至少简要地研究适当的 RFC 文档，您可能会很好地了解 libcurl 以及如何更好地使用它。

## 上传数据到远程站点
libcurl 尝试对大多数传输保持独立于协议的方法，因此上传到远程 FTP 站点与使用 PUT 请求将数据上传到 HTTP 服务器非常相似。

当然，首先你要么创建一个 easy 句柄，要么重新使用一个现有的句柄。然后设置 URL 以像以前一样操作。这是我们现在将上传的远程 URL。

由于我们编写了一个应用程序，我们很可能希望 libcurl 通过要求我们来获取上传数据。为了做到这一点，我们设置了读取回调，自定义指针 libcurl 将传递给我们的读取回调。读取回调应具有类似于以下内容的原型：
```c++
size_t function(char *bufptr, size_t size, size_t nitems, void *userp);
```
其中 bufptr 是指向我们填充要上传的数据的缓冲区的指针，而 size*nitems 是缓冲区的大小，因此也是我们可以在此调用中返回给 libcurl 的最大数据量。 “userp”指针是我们设置为指向我们的结构的自定义指针，以在应用程序和回调之间传递私有数据。

```c++
curl_easy_setopt(easyhandle, CURLOPT_READFUNCTION, read_function);
curl_easy_setopt(easyhandle, CURLOPT_READDATA, &filedata);
```
告诉 libcurl 我们要上传：
```c++
curl_easy_setopt(easyhandle, CURLOPT_UPLOAD, 1L);
```
在没有任何预期文件大小的先验知识的情况下完成上传时，一些协议将无法正常运行。因此，使用 CURLOPT_INFILESIZE_LARGE 为所有已知文件大小设置上传文件大小，如下所示 [1]：
```c++
/* in this example, file_size must be an curl_off_t variable */
curl_easy_setopt(easyhandle, CURLOPT_INFILESIZE_LARGE, file_size);
```
当您这次调用 curl_easy_perform 时，它将执行所有必要的操作，当它调用上传时，它将调用您提供的回调以获取要上传的数据。程序应该在每次调用中返回尽可能多的数据，因为这可能会使上传尽可能快地执行。回调应该返回它在缓冲区中写入的字节数。返回 0 将表示上传结束。

## 密码
许多协议使用甚至要求提供用户名和密码才能下载或上传您选择的数据。 libcurl 提供了几种指定它们的方法。

大多数协议支持您在 URL 本身中指定名称和密码。 libcurl 将检测到这一点并相应地使用它们。这是这样写的：

```js
protocol://user:password@example.com/path/
```
如果您的用户名或密码中需要任何奇数字母，您应该输入它们的 URL 编码，如 %XX，其中 XX 是两位十六进制数。

libcurl 还提供了设置各种密码的选项。嵌入在 URL 中的用户名和密码可以改为使用 CURLOPT_USERPWD 选项设置。传递给 libcurl 的参数应该是 char * 到格式为“user:password”的字符串。以这样的方式：

```c++
curl_easy_setopt(easyhandle, CURLOPT_USERPWD, "myname:thesecret");
```

有时可能需要名称和密码的另一种情况是那些需要向他们使用的代理进行身份验证的用户。 libcurl 为此提供了另一个选项，即 CURLOPT_PROXYUSERPWD。它的使用与 CURLOPT_USERPWD 选项非常相似，如下所示：

```c++
curl_easy_setopt(easyhandle, CURLOPT_PROXYUSERPWD, "myname:thesecret");
```
存储 FTP 用户名和密码的 Unix“标准”方式由来已久，即在 $HOME/.netrc 文件中。该文件应设为私有，以便只有用户可以阅读它（另请参阅“安全注意事项”一章），因为它可能包含纯文本形式的密码。 libcurl 能够使用此文件来确定特定主机使用的用户名和密码集。作为对正常功能的扩展，libcurl 也支持非 FTP 协议（​​如 HTTP）的这个文件。要使 curl 使用此文件，请使用 CURLOPT_NETRC 选项：
```c++
curl_easy_setopt(easyhandle, CURLOPT_NETRC, 1L);
```
还有一个非常基本的示例，说明此类 .netrc 文件的外观：
```shell
machine myhost.mydomain.com
login userlogin
password secretword
```
所有这些例子都是密码是可选的，或者至少你可以忽略它并让 libcurl 尝试在没有它的情况下完成它的工作。有时密码不是可选的，例如当您使用 SSL 私钥进行安全传输时。

将已知的私钥密码传递给 libcurl：
```c++
curl_easy_setopt(easyhandle, CURLOPT_KEYPASSWD, "keypassword");
```

## HTTP 认证
上一章展示了如何设置用户名和密码以获取需要身份验证的 URL。使用 HTTP 协议时，客户端可以通过多种不同的方式向服务器提供这些凭据，您可以控制 libcurl 将（尝试）使用它们的方式。默认的 HTTP 身份验证方法称为“Basic”，它以 base64 编码的 HTTP 请求中的明文形式发送名称和密码。这是不安全的。

在撰写本文时，可以构建 libcurl 以使用：Basic、Digest、NTLM、Negotiate (SPNEGO)。您可以告诉 libcurl 将哪个与 CURLOPT_HTTPAUTH 一起使用，如下所示：

```c++
curl_easy_setopt(easyhandle, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);
```
当您向代理发送身份验证时，您还可以以相同的方式设置身份验证类型，但使用 CURLOPT_PROXYAUTH：
```c++
curl_easy_setopt(easyhandle, CURLOPT_PROXYAUTH, CURLAUTH_NTLM);
```
这两个选项都允许您设置多种类型（通过将它们组合在一起），以使 libcurl 从服务器/代理声称支持的类型中选择最安全的类型。但是，此方法确实添加了往返，因为 libcurl 必须首先询问服务器它支持什么：
```c++
curl_easy_setopt(easyhandle, CURLOPT_HTTPAUTH,  CURLAUTH_DIGEST|CURLAUTH_BASIC);
```
为方便起见，您可以使用 'CURLAUTH_ANY' 定义（而不是具有特定类型的列表），它允许 libcurl 使用它想要的任何方法。

当要求多种类型时，libcurl 会按照它自己的内部偏好顺序选择它认为“最佳”的可用类型。

## HTTP POST
关于如何以正确的方式使用 libcurl 发出 HTTP POST，我们收到了很多问题。因此，本章将包括使用 libcurl 支持的两种不同版本的 HTTP POST 的示例。

第一个版本是简单的 POST，最常见的版本，大多数使用 <form> 标签的 HTML 页面都使用它。我们提供一个指向数据的指针并告诉 libcurl 将其全部发布到远程站点：
```c++
char *data="name=daniel&project=curl";
curl_easy_setopt(easyhandle, CURLOPT_POSTFIELDS, data);
curl_easy_setopt(easyhandle, CURLOPT_URL, "http://posthere.com/");

curl_easy_perform(easyhandle); /* post away! */
```
够简单了吧？由于您使用 CURLOPT_POSTFIELDS 设置了 POST 选项，这会自动将句柄切换为在即将到来的请求中使用 POST。

好的，那么如果您想发布也需要设置 Content-Type: post 二进制数据呢？好吧，二进制 post 阻止 libcurl 能够对数据执行 strlen() 来计算大小，因此我们必须告诉 libcurl  post 数据的大小。在 libcurl 请求中设置Header以通用方式完成，通过构建我们自己的 header list，然后将该列表传递给 libcurl。
```c++
struct curl_slist *headers=NULL;
headers = curl_slist_append(headers, "Content-Type: text/xml");

/* post binary data */
curl_easy_setopt(easyhandle, CURLOPT_POSTFIELDS, binaryptr);

/* set the size of the postfields data */
curl_easy_setopt(easyhandle, CURLOPT_POSTFIELDSIZE, 23L);

/* pass our list of custom made headers */
curl_easy_setopt(easyhandle, CURLOPT_HTTPHEADER, headers);

curl_easy_perform(easyhandle); /* post away! */

curl_slist_free_all(headers); /* free the header list */
```
虽然上面的简单示例涵盖了大部分需要 HTTP POST 操作的情况，但它们不执行多部分表单发布。多部分模板作为发布（可能很大）二进制数据的更好方式被引入，并首次记录在 RFC 1867（在 RFC 2388 中更新）中。它们被称为多部分是因为它们由一连串的部分组成，每个部分都是一个单一的数据单元。每个部分都有自己的名称和内容。实际上，您可以使用上述常规 libcurl POST 支持创建和发布多部分表单，但这需要您自己构建表单并提供给 libcurl。为了更简单，libcurl 提供了一个包含多个函数的 MIME API：使用这些函数，您可以创建和填写一个多部分的表单。函数 curl_mime_init 创建一个多部分主体；然后，您可以使用 curl_mime_addpart 将新部分附加到多部分正文。零件有三种可能的数据源：使用 curl_mime_data 的内存、使用 curl_mime_filedata 的文件和使用 curl_mime_data_cb 的用户定义的数据读取回调。 curl_mime_name 设置部件的（即：表单字段）名称，而 curl_mime_filename 填写远程文件名。使用 curl_mime_type，您可以告诉零件的 MIME 类型， curl_mime_headers 允许定义零件的 headers。当不再需要多部分主体时，您可以使用 curl_mime_free 销毁它。

下面的例子设置了两个带有纯文本内容的简单文本部分，然后是一个带有二进制内容的文件并上传了整个内容。

```c++
 curl_mime *multipart = curl_mime_init(easyhandle);
 curl_mimepart *part = curl_mime_addpart(multipart);
 curl_mime_name(part, "name");
 curl_mime_data(part, "daniel", CURL_ZERO_TERMINATED);
 part = curl_mime_addpart(multipart);
 curl_mime_name(part, "project");
 curl_mime_data(part, "curl", CURL_ZERO_TERMINATED);
 part = curl_mime_addpart(multipart);
 curl_mime_name(part, "logotype-image");
 curl_mime_filedata(part, "curl.png");
 
 /* Set the form info */
 curl_easy_setopt(easyhandle, CURLOPT_MIMEPOST, multipart);
 
 curl_easy_perform(easyhandle); /* post away! */
 
 /* free the post data again */
 curl_mime_free(multipart);
```
要为单个表单字段发布多个文件，您必须在单独的部分中提供每个文件，所有文件都具有相同的字段名称。尽管函数 curl_mime_subparts 实现了嵌套的多部分，但这种多文件发布方式已被 RFC 7578 第 4.3 章弃用。

要从已打开的 FILE 指针设置数据源，请使用：

```c++
curl_mime_data_cb(part, filesize, (curl_read_callback) fread,
                   (curl_seek_callback) fseek, NULL, filepointer);
```
libcurl 中仍支持已弃用的 curl_formadd 函数。然而，它不应该再用于新的设计和程序，使用它应该转换为 MIME API。然而，这里将其描述为有助于转换。

使用 curl_formadd，您可以向表单添加部件。添加完部分后，您可以发布整个表单。

上面的 MIME API 示例使用此函数表示如下：

```c++
struct curl_httppost *post=NULL;
struct curl_httppost *last=NULL;
curl_formadd(&post, &last,
            CURLFORM_COPYNAME, "name",
            CURLFORM_COPYCONTENTS, "daniel", CURLFORM_END);
curl_formadd(&post, &last,
            CURLFORM_COPYNAME, "project",
            CURLFORM_COPYCONTENTS, "curl", CURLFORM_END);
curl_formadd(&post, &last,
            CURLFORM_COPYNAME, "logotype-image",
            CURLFORM_FILECONTENT, "curl.png", CURLFORM_END);

/* Set the form info */
curl_easy_setopt(easyhandle, CURLOPT_HTTPPOST, post);

curl_easy_perform(easyhandle); /* post away! */

/* free the post data again */
curl_formfree(post);
```
多部分模板是使用 MIME 样式分隔符和 header 的部分链。这意味着这些单独的部分中的每一个都有一些 headers，这些 headers描述了各个内容类型、大小等。为了使您的应用程序能够更多地制作此模板，libcurl 允许您将自己的自定义 headers提供给这样的一个个体形式部分。您当然可以为任意数量的部分提供 header ，但是这个小示例将显示当您将 header 添加到帖子句柄时如何将 header 设置为一个特定部分：

```c++
struct curl_slist *headers=NULL;
headers = curl_slist_append(headers, "Content-Type: text/xml");

curl_formadd(&post, &last,
            CURLFORM_COPYNAME, "logotype-image",
            CURLFORM_FILECONTENT, "curl.xml",
            CURLFORM_CONTENTHEADER, headers,
            CURLFORM_END);

curl_easy_perform(easyhandle); /* post away! */

curl_formfree(post); /* free post */
curl_slist_free_all(headers); /* free custom header list */
```
由于easyhandle 上的所有选项都是“sticky”，因此即使您确实调用了curl_easy_perform，它们在更改之前都保持不变，如果您打算将其作为下一个请求执行，则可能需要告诉curl 返回到普通的GET 请求。您可以使用 CURLOPT_HTTPGET 选项强制 easyhandle 返回到 GET：

```c++
curl_easy_setopt(easyhandle, CURLOPT_HTTPGET, 1L);
```
只需将 CURLOPT_POSTFIELDS 设置为 "" 或 NULL 将*不会*阻止 libcurl 执行 POST。它只会让它 POST 没有任何数据要发送！

## 从弃用的表单 API 转换为 MIME API
在构建多部分时必须遵守四个规则： 
- 必须在构建多部分之前创建 easy 句柄。 
- 多部分始终通过调用 curl_mime_init(easyhandle) 创建。 
- 每个部分都是通过调用 curl_mime_addpart(multipart) 创建的。 
- 完成后，必须使用 CURLOPT_MIMEPOST 而不是 CURLOPT_HTTPPOST 将多部分绑定到 easy 句柄。

以下是 curl_formadd 调用 MIME API 序列的一些示例：
```c++
curl_formadd(&post, &last,
              CURLFORM_COPYNAME, "id",
              CURLFORM_COPYCONTENTS, "daniel", CURLFORM_END);
              CURLFORM_CONTENTHEADER, headers,
              CURLFORM_END);
```
变成：
```c++
part = curl_mime_addpart(multipart);
curl_mime_name(part, "id");
curl_mime_data(part, "daniel", CURL_ZERO_TERMINATED);
curl_mime_headers(part, headers, FALSE);
```
将最后一个 curl_mime_headers 参数设置为 TRUE 会导致在销毁多部分时自动释放Header，从而节省对 curl_slist_free_all 的清理调用。

```c++
curl_formadd(&post, &last,
            CURLFORM_PTRNAME, "logotype-image",
            CURLFORM_FILECONTENT, "-",
            CURLFORM_END);
```
变成：
```c++
part = curl_mime_addpart(multipart);
curl_mime_name(part, "logotype-image");
curl_mime_data_cb(part, (curl_off_t) -1, fread, fseek, NULL, stdin);
```
curl_mime_name 始终复制字段名称。 curl_mime_file 不支持特殊文件名“-”：要读取打开的文件，请使用 fread() 使用回调源。由于数据大小未知，传输将被分块。
```c++
curl_formadd(&post, &last,
              CURLFORM_COPYNAME, "datafile[]",
              CURLFORM_FILE, "file1",
              CURLFORM_FILE, "file2",
              CURLFORM_END);
```
变成：
```c++
part = curl_mime_addpart(multipart);
curl_mime_name(part, "datafile[]");
curl_mime_filedata(part, "file1");
part = curl_mime_addpart(multipart);
curl_mime_name(part, "datafile[]");
curl_mime_filedata(part, "file2");
```
已弃用的 multipart/mixed 多个文件字段的实现被转换为具有相同名称的两个不同部分。
```c++
curl_easy_setopt(easyhandle, CURLOPT_READFUNCTION, myreadfunc);
curl_formadd(&post, &last,
            CURLFORM_COPYNAME, "stream",
            CURLFORM_STREAM, arg,
            CURLFORM_CONTENTLEN, (curl_off_t) datasize,
            CURLFORM_FILENAME, "archive.zip",
            CURLFORM_CONTENTTYPE, "application/zip",
            CURLFORM_END);
```
变成：
```c++
part = curl_mime_addpart(multipart);
curl_mime_name(part, "stream");
curl_mime_data_cb(part, (curl_off_t) datasize,
                myreadfunc, NULL, NULL, arg);
curl_mime_filename(part, "archive.zip");
curl_mime_type(part, "application/zip");
```
未使用 CURLOPT_READFUNCTION 回调：它通过直接设置回调读取函数中的部分源数据来替换。
```c++
curl_formadd(&post, &last,
            CURLFORM_COPYNAME, "memfile",
            CURLFORM_BUFFER, "memfile.bin",
            CURLFORM_BUFFERPTR, databuffer,
            CURLFORM_BUFFERLENGTH, (long) sizeof databuffer,
            CURLFORM_END);
```
变成：
```c++
part = curl_mime_addpart(multipart);
curl_mime_name(part, "memfile");
curl_mime_data(part, databuffer, (curl_off_t) sizeof databuffer);
curl_mime_filename(part, "memfile.bin");
```
curl_mime_data 总是复制初始数据：因此数据缓冲区可以立即重用。
```c++
curl_formadd(&post, &last,
            CURLFORM_COPYNAME, "message",
            CURLFORM_FILECONTENT, "msg.txt",
            CURLFORM_END);
```
变成：
```c++
part = curl_mime_addpart(multipart);
curl_mime_name(part, "message");
curl_mime_filedata(part, "msg.txt");
curl_mime_filename(part, NULL);
```
使用 curl_mime_filedata 将远程文件名设置为副作用：因此有必要为 CURLFORM_FILECONTENT 模拟清除它。

## 显示进度
由于历史和传统的原因，libcurl 有一个内置的进度表，可以打开它，然后让它在你的终端中显示一个进度表。

奇怪的是，通过将 CURLOPT_NOPROGRESS 设置为0来打开进度表。默认情况下，此选项设置为 1。

然而，对于大多数应用程序，内置的进度表是无用的，而有趣的是指定进度回调的能力。您传递给 libcurl 的函数指针将在不规则的时间间隔内被调用，并提供有关当前传输的信息。

使用 CURLOPT_PROGRESSFUNCTION 设置进度回调。并传递一个指向与此原型匹配的函数的指针：

```c++
int progress_callback(void *clientp,
                    double dltotal,
                    double dlnow,
                    double ultotal,
                    double ulnow);
```
如果任何输入参数未知，则将传递 0。第一个参数“clientp”是您使用 CURLOPT_PROGRESSDATA 传递给 libcurl 的指针。 libcurl 不会碰它。

## C++ 使用 libcurl
在连接 libcurl 时使用 C++ 而不是 C 时，基本上只需要记住一件事：

回调不能是非静态类成员函数

示例代码
```c++
class AClass {
    static size_t write_data(void *ptr, size_t size, size_t nmemb,
                             void *ourpointer)
    {
      /* do what you want with the data */
    }
 }
```

## 代理
根据Merriam-Webster的说法，“代理人”是什么意思：“被授权为他人行事的人”，但也包括“代理他人的代理机构、职能或办公室”。

如今，代理非常普遍。公司通常仅通过其代理向员工提供 Internet 访问权限。网络客户端或用户代理向代理请求文档，代理执行实际请求，然后返回它们。

libcurl 支持 SOCKS 和 HTTP 代理。当需要给定的 URL 时，libcurl 将向代理询问它，而不是尝试连接到 URL 中标识的实际主机。

如果您使用 SOCKS 代理，您可能会发现 libcurl 并不完全支持通过它进行的所有操作。

对于 HTTP 代理：代理是 HTTP 代理这一事实对实际发生的事情施加了某些限制。可能不是 HTTP URL 的请求 URL 仍将传递给 HTTP 代理以返回给 libcurl。这是透明发生的，应用程序可能不需要知道。我说“可能”，因为有时了解 HTTP 代理上的所有操作都使用 HTTP 协议非常重要。例如，您无法调用自己的自定义 FTP 命令，甚至无法调用正确的 FTP 目录列表。

### 代理选项
告诉 libcurl 在给定端口号使用代理：
```c++
curl_easy_setopt(easyhandle, CURLOPT_PROXY, "proxy-host.com:8080");
```
某些代理在允许请求之前需要用户身份验证，并且您传递类似于以下内容的信息：
```c++
curl_easy_setopt(easyhandle, CURLOPT_PROXYUSERPWD, "user:password");
```
如果需要，可以只在 CURLOPT_PROXY 选项中指定主机名，并用 CURLOPT_PROXYPORT 单独设置端口号。

用 CURLOPT_PROXYTYPE 告诉 libcurl 它是什么类型的代理（如果不是，它会默认假设一个 HTTP 代理）：
```c++
curl_easy_setopt(easyhandle, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4);
```
### 环境变量
libcurl 会自动检查并使用一组环境变量来了解特定协议使用的代理。变量的名称遵循古老的事实上的标准，并构建为“[protocol]_proxy”（注意小写）。当输入 URL 为 HTTP 时，这使得变量“http_proxy”检查要使用的代理名称。按照相同的规则，检查名为“ftp_proxy”的变量以查找 FTP URL。同样，代理始终是 HTTP 代理，变量的不同名称只是允许使用不同的 HTTP 代理。

代理环境变量内容的格式应为“[protocol://][user:password@]machine[:port]”。如果存在 protocol:// 部分，则会被简单地忽略（因此 http://proxy 和 bluerk://proxy 将执行相同的操作）并且可选端口号指定代理在主机上运行的端口。如果未指定，将使用内部默认端口号，这很可能*不是*您希望使用的端口号。

有两个特殊的环境变量。 'all_proxy' 是在未设置协议特定变量的情况下为任何 URL 设置代理，而 'no_proxy' 定义了不应使用代理的主机列表，即使变量可能会这样说。如果 'no_proxy' 是一个普通的星号（“*”），它匹配所有主机。

要显式禁用 libcurl 对代理环境变量的检查和使用，请将代理名称设置为 "" - 一个空字符串 - 使用 CURLOPT_PROXY。


### SSL 和代理
SSL 用于安全的点对点连接。这涉及到强加密和类似的东西，这有效地使代理不可能作为代理任务的“中间人”运行，如前所述。相反，让 SSL 通过 HTTP 代理工作的唯一方法是要求代理通过隧道传输所有内容，而不能检查或处理流量。

因此，通过 HTTP 代理打开 SSL 连接就是要求代理直接连接到指定端口上的目标主机。这是通过 HTTP 请求 CONNECT 实现的。 （“请代理先生，将我连接到该远程主机”）。

由于此操作的性质，代理不知道通过此隧道传入和传出的数据类型，这破坏了使用代理带来的一些极少数优势，例如缓存。许多组织阻止这种隧道连接到除 443（默认 HTTPS 端口号）之外的其他目标端口号。

### 通过代理隧道
正如上面所解释的，SSL 需要隧道才能工作，并且通常甚至仅限于用于 SSL 的操作； HTTPS。

然而，这并不是代理隧道可能为您或您的应用程序带来好处的唯一一次。

当隧道打开从您的应用程序到远程机器的直接连接时，它突然也重新引入了通过 HTTP 代理执行非 HTTP 操作的能力。实际上，您可以通过这种方式使用诸如 FTP 上传或 FTP 自定义命令之类的东西。

同样，这通常被代理管理员阻止并且很少被允许。

告诉 libcurl 像这样使用代理隧道：

```c++
curl_easy_setopt(easyhandle, CURLOPT_HTTPPROXYTUNNEL, 1L);
```
事实上，有时您甚至可能想要使用这样的隧道进行普通的 HTTP 操作，因为这样您就可以在远程服务器上进行操作，而不是要求代理这样做。 libcurl 也不会妨碍此类创新行动！

### 代理自动配置
Netscape 首先提出了这一点。它基本上是一个带有 Javascript 的网页（通常使用 .pac 扩展名），当浏览器以请求的 URL 作为输入执行时，向浏览器返回有关如何连接到 URL 的信息。返回的信息可能是“DIRECT”（这意味着不应该使用代理）、“PROXY host:port”（告诉浏览器这个特定 URL 的代理在哪里）或“SOCKS host:port”（引导浏览器到 SOCKS 代理）。

libcurl 无法解释或eval Javascript，因此它不支持这一点。如果您让自己面临这个令人讨厌的发明，那么过去曾提到并使用过以下建议：

- 根据 Javascript 的复杂性，编写一个脚本，将其翻译成另一种语言并执行。
- 阅读 Javascript 代码并用另一种语言重写相同的逻辑。
- 实现一个 Javascript 解释器；人们过去曾成功地使用过 Mozilla Javascript 引擎。
- 要求您的管理员停止此操作，以进行静态代理设置或类似操作。

## 持久是幸福之道
在执行多个请求时多次重复使用相同的 easy 句柄是可行的方法。

在每次 curl_easy_perform 操作之后，libcurl 将保持连接处于活动状态并打开。对同一主机使用相同 easy 句柄的后续请求可能只能使用已经打开的连接！这大大减少了网络影响。

即使连接断开，所有再次连接到同一主机的 SSL 连接也将受益于 libcurl 的会话 ID 缓存，这大大减少了重新连接的时间。

保持活动状态的 FTP 连接可以节省大量时间，因为命令-响应往返被跳过，而且您也不会冒着被阻止而未经许可再次登录的风险，就像在许多只允许 N 人登录的 FTP 服务器上一样同时。

libcurl 缓存 DNS 名称解析结果，以便更快地查找以前查找的名称。

将来也可能会添加其他有趣的细节，以提高后续请求的性能。

每个 easy 的句柄都会尝试将最后几个连接保持活动一段时间，以防它们再次被使用。您可以使用 CURLOPT_MAXCONNECTS 选项设置此“缓存”的大小。默认值为 5。更改此值很少有任何意义，如果您想更改此值，通常只需重新思考即可。

要强制您即将发出的请求不使用已经存在的连接（如果碰巧有一个连接到您要操作的同一主机上，它甚至会先关闭一个连接），您可以通过将 CURLOPT_FRESH_CONNECT 设置为 1 来实现。出于类似的精神，您还可以通过将 CURLOPT_FORBID_REUSE 设置为 1 来禁止即将到来的请求“撒谎”，并可能在请求后重新使用。

## libcurl 使用的 HTTP Heaers
当您使用 libcurl 执行 HTTP 请求时，它会自动传递一系列 headers。了解和理解这些可能对您有好处。您可以使用 CURLOPT_HTTPHEADER 选项替换或删除它们。
- Host

    HTTP 1.1 甚至许多 1.0 服务器都需要此 headers，并且应该是我们要与之通信的服务器的名称。这包括端口号（如果不是默认的）。

- Accept
    
    "\*/\*".

- Expect

    在执行 POST 请求时，libcurl 将此 headers 设置为“100-continue”，以在继续发送 POST 的数据部分之前向服务器询问“OK”消息。如果 POSTed 数据量被认为“small”，libcurl 将不使用此 headers 。

## 自定义操作
今天有一个持续的发展，越来越多的协议建立在 HTTP 上进行传输。这具有明显的好处，因为 HTTP 是一种经过测试且可靠的协议，广泛部署并具有出色的代理支持。

当您使用其中一种协议时，甚至在进行其他类型的编程时，您可能需要更改传统的 HTTP（或 FTP 或...）方式。您可能需要更改单词、 Header 或各种数据。

libcurl 也是你的朋友。

### 客户要求
如果只是更改实际的 HTTP 请求关键字就是您想要的，例如当 GET、HEAD 或 POST 对您来说不够好时，CURLOPT_CUSTOMREQUEST 就可以为您服务。使用非常简单：
```c++
curl_easy_setopt(easyhandle, CURLOPT_CUSTOMREQUEST, "MYOWNREQUEST");
```
使用自定义请求时，您可以更改正在执行的实际请求的请求关键字。因此，默认情况下您会发出 GET 请求，但您也可以进行 POST 操作（如前所述），然后根据需要替换 POST 关键字。你是老板。

### 修改 Headers
执行请求时，类似 HTTP 的协议会向服务器传递一系列 Headers，您可以随意传递任何您认为合适的额外 Headers。添加 header 就是这么简单：
```c++
struct curl_slist *headers=NULL; /* init to NULL is important */

headers = curl_slist_append(headers, "Hey-server-hey: how are you?");
headers = curl_slist_append(headers, "X-silly-content: yes");

/* pass our list of custom made headers */
curl_easy_setopt(easyhandle, CURLOPT_HTTPHEADER, headers);

curl_easy_perform(easyhandle); /* transfer http */

curl_slist_free_all(headers); /* free the header list */
```
...如果您认为某些内部生成的 header ，例如 Accept: 或 Host: 不包含您希望它们包含的数据，您也可以通过简单地设置它们来替换它们：
```c++
headers = curl_slist_append(headers, "Accept: Agent-007");
headers = curl_slist_append(headers, "Host: munged.host.line");
```
### 删除 Headers
如果您用没有内容的 Header 替换现有 Header ，您将阻止发送 Header 。例如，如果您想完全阻止发送“Accept:”Header，您可以使用类似于以下的代码禁用它：
```c++
headers = curl_slist_append(headers, "Accept:");
```
替换和取消内部 Header 都应该仔细考虑，并且您应该意识到这样做可能会违反 HTTP 协议。

### 强制分块传输编码
通过确保请求在执行非 GET HTTP 操作时使用自定义 Header “Transfer-Encoding: chunked”，libcurl 将切换到“chunked”上传，即使要上传的数据大小可能已知。默认情况下，如果上传数据大小未知，libcurl 通常会自动切换到分块上传。

### HTTP 版本
所有 HTTP 请求都包含版本号以告诉服务器我们支持哪个版本。 libcurl 默认使用 HTTP 1.1。一些非常旧的服务器不喜欢获得 1.1 请求，并且在处理诸如此类的顽固旧事物时，您可以通过执行以下操作告诉 libcurl 使用 1.0：
```c++
curl_easy_setopt(easyhandle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
```

### FTP 自定义命令
并非所有协议都类似于 HTTP，因此当您想要使 FTP 传输行为不同时，上述内容可能无济于事。

将自定义命令发送到 FTP 服务器意味着您需要完全按照 FTP 服务器的期望发送命令（RFC959 是这里的一个很好的指南），并且您只能使用单独在控制连接上工作的命令。各种需要数据交换从而需要数据连接的命令必须留给libcurl自己判断。另请注意，libcurl 会在进行任何传输之前尽最大努力将目录更改为目标目录，因此如果您更改目录（使用 CWD 或类似的），您可能会混淆 libcurl，然后它可能不会尝试以正确的方式传输文件远程目录。

在操作前删除给定文件的一个小例子：
```c++
headers = curl_slist_append(headers, "DELE file-to-remove");

/* pass the list of custom commands to the handle */
curl_easy_setopt(easyhandle, CURLOPT_QUOTE, headers);

curl_easy_perform(easyhandle); /* transfer ftp data! */

curl_slist_free_all(headers); /* free the header list */
```
如果您希望此操作（或操作链）在 _after_ 数据传输发生后发生，则 curl_easy_setopt 选项将被称为 CURLOPT_POSTQUOTE 并以完全相同的方式使用。

自定义 FTP 命令将按照添加到列表中的相同顺序向服务器发出，如果命令从服务器返回错误代码，则不会发出更多命令，libcurl 将退出并显示错误代码(CURLE_QUOTE_ERROR)。请注意，如果您在传输之前使用 CURLOPT_QUOTE 发送命令，则在引用命令失败时实际上不会发生任何传输。

如果您将 CURLOPT_HEADER 设置为 1，您将告诉 libcurl 获取有关目标文件的信息并输出有关它的“headers”。Header 将采用“HTTP style”，看起来就像它们在 HTTP 中一样。

启用 Header 或运行自定义 FTP 命令的选项与 CURLOPT_NOBODY 结合使用可能很有用。如果设置了此选项，则不会执行实际的文件内容传输。

### FTP 自定义 CUSTOMREQUEST
如果您确实想使用自己定义的 FTP 命令列出 FTP 目录的内容，CURLOPT_CUSTOMREQUEST 将做到这一点。 “NLST”是列出目录的默认选项，但您可以自由地传递您对一个好的替代方案的想法。

## Cookies Without Chocolate Chips
在 HTTP 意义上，cookie 是具有关联值的名称。服务器将名称和值发送给客户端，并期望它在每个后续请求中被发送回与特定条件集相匹配的服务器。条件包括域名和路径匹配并且cookie 没有变得太旧。

在实际情况下，服务器会发送新的 cookie 来替换现有的 cookie 来更新它们。服务器使用 cookie 来“跟踪”用户并保持“会话”。

Cookie 从服务器发送到客户端，带有头文件 Set-Cookie: 并且它们从客户端发送到带有 Cookie: 头的服务器。

要将您想要的任何 cookie 发送到服务器，您可以使用 CURLOPT_COOKIE 来设置这样的 cookie 字符串：

```c++
curl_easy_setopt(easyhandle, CURLOPT_COOKIE, "name1=var1; name2=var2;");
```

在许多情况下，这还不够。您可能希望动态保存远程服务器传递给您的任何 cookie，并确保这些 cookie 随后相应地用于以后的请求。

一种方法是将您收到的所有标头保存在一个普通文件中，当您发出请求时，您告诉 libcurl 读取先前的标头以确定要使用哪些 cookie。使用 CURLOPT_COOKIEFILE 设置头文件以从中读取 cookie。

CURLOPT_COOKIEFILE 选项还会自动启用 libcurl 中的 cookie 解析器。在启用 cookie 解析器之前，libcurl 不会解析或理解传入的 cookie，它们只会被忽略。但是，当启用解析器时，cookie 将被理解，cookie 将保存在内存中，并在使用相同句柄时在后续请求中正确使用。很多时候这已经足够了，您可能根本不必将 cookie 保存到磁盘。请注意，您指定给 CURLOPT_COOKIEFILE 的文件不一定要存在才能启用解析器，因此仅启用解析器而不读取任何 cookie 的常用方法是使用您知道不存在的文件的名称。

如果您更愿意使用以前通过 Netscape 或 Mozilla 浏览器收到的现有 cookie，您可以让 libcurl 使用该 cookie 文件作为输入。 CURLOPT_COOKIEFILE 也用于此目的，因为 libcurl 会自动找出它是什么类型的文件并采取相应的行动。

也许 libcurl 提供的最先进的 cookie 操作是将整个内部 cookie 状态保存回 Netscape/Mozilla 格式的 cookie 文件。我们称之为cookie-jar。当您使用 CURLOPT_COOKIEJAR 设置文件名时，将创建该文件名，并在调用 curl_easy_cleanup 时将所有收到的 cookie 存储在其中。这使 cookie 能够在多个句柄之间正确传递，而不会丢失任何信息。

## 我们需要的 FTP 特性
FTP 传输使用第二个 TCP/IP 连接进行数据传输。这通常是一个你可以忘记和忽略的事实，但有时这个事实会再次困扰你。 libcurl 提供了几种不同的方式来定制第二个连接的建立方式。

libcurl 可以再次连接到服务器或告诉服务器重新连接到它。第一个选项是默认选项，它也是防火墙、NAT 或 IP 伪装设置背后的所有人的最佳选择。 libcurl 然后告诉服务器打开一个新端口并等待第二个连接。默认情况下，首先尝试使用 EPSV，如果这不起作用，它会尝试使用 PASV。 （EPSV 是原始 FTP 规范的扩展，不存在也不适用于所有 FTP 服务器。）

您可以通过将 CURLOPT_FTP_USE_EPSV 设置为零来阻止 libcurl 首先尝试 EPSV 命令。

在某些情况下，您更愿意让服务器连接回您以进行第二次连接。这可能是当服务器可能位于防火墙或其他东西之后并且只允许在单个端口上进行连接时。 libcurl 然后通知远程服务器要连接的 IP 地址和端口号。这是通过 CURLOPT_FTPPORT 选项实现的。如果您将其设置为“-”，libcurl 将使用您系统的“默认 IP 地址”。如果您想使用特定的 IP，您可以设置完整的 IP 地址、要解析为 IP 地址的主机名，甚至是 libcurl 将从中获取 IP 地址的本地网络接口名称。

在执行“PORT”方法时，libcurl 将在尝试 PORT 之前尝试使用 EPRT 和 LPRT，因为它们适用于更多协议。您可以通过将 CURLOPT_FTP_USE_EPRT 设置为0来禁用此行为。

## 为 SMTP 和 IMAP 重新访问 MIME API
除了支持 HTTP 多部分表单字段之外，MIME API 还可用于构建结构化电子邮件消息并通过 SMTP 发送它们或将此类消息附加到 IMAP 目录。

结构化的电子邮件消息可能包含几个部分：一些由 MUA 内联显示，一些是附件。部分也可以构造为多部分，例如包含另一封电子邮件或提供多种文本格式替代。这可以嵌套到任何级别。

要构建这样的消息，您需要准备第 n 级多部分，然后使用函数 curl_mime_subparts 将其作为源包含在父多部分中。一旦它绑定到它的父 multi-part，第 n 级 multi-part 就属于它，不应显式释放。

电子邮件消息数据不应该是非 ascii 并且行长度是有限的：幸运的是，标准定义了一些传输编码以支持此类不兼容数据的传输。函数 curl_mime_encoder 告诉部件它的源数据在发送之前必须进行编码。它还为该部分生成相应的标题。如果您要发送的部分数据已经以这种方案编码，请不要使用此功能（这会对其进行过度编码），而是明确设置相应的部分标头。

发送这样的消息后，libcurl 将使用 CURLOPT_HTTPHEADER 设置的标头列表作为第 0 级 mime 部分标头。

下面是一个使用内联纯文本/html 文本替代和以 base64 编码的文件附件构建电子邮件消息的示例：

```c++
curl_mime *message = curl_mime_init(easyhandle);
 
 /* The inline part is an alternative proposing the html and the text
    versions of the e-mail. */
 curl_mime *alt = curl_mime_init(easyhandle);
 
 /* HTML message. */
 curl_mimepart *part = curl_mime_addpart(alt);
 curl_mime_data(part, "<html><body><p>This is HTML</p></body></html>",
                      CURL_ZERO_TERMINATED);
 curl_mime_type(part, "text/html");
 
 /* Text message. */
 part = curl_mime_addpart(alt);
 curl_mime_data(part, "This is plain text message",
                      CURL_ZERO_TERMINATED);
 
 /* Create the inline part. */
 part = curl_mime_addpart(message);
 curl_mime_subparts(part, alt);
 curl_mime_type(part, "multipart/alternative");
 struct curl_slist *headers = curl_slist_append(NULL,
                   "Content-Disposition: inline");
 curl_mime_headers(part, headers, TRUE);
 
 /* Add the attachment. */
 part = curl_mime_addpart(message);
 curl_mime_filedata(part, "manual.pdf");
 curl_mime_encoder(part, "base64");
 
 /* Build the mail headers. */
 headers = curl_slist_append(NULL, "From: me@example.com");
 headers = curl_slist_append(headers, "To: you@example.com");
 
 /* Set these into the easy handle. */
 curl_easy_setopt(easyhandle, CURLOPT_HTTPHEADER, headers);
 curl_easy_setopt(easyhandle, CURLOPT_MIMEPOST, mime);
```
应该注意的是，将消息附加到 IMAP 目录需要在上传之前知道消息大小。因此，在此上下文中不可能包含数据大小未知的部分。

## Headers 同样有趣
一些协议提供“Header”，即与普通数据分离的元数据。默认情况下，这些Header不包含在普通数据流中，但您可以通过将 CURLOPT_HEADER 设置为 1 来使它们出现在数据流中。

更有用的是 libcurl 能够将 Header 与数据分开，从而使回调不同。例如，您可以通过设置 CURLOPT_HEADERDATA 来设置不同的指针以传递给普通的写入回调。

或者，您可以使用 CURLOPT_HEADERFUNCTION 设置一个完全独立的函数来接收 Header。

Header 一个一个地传递给回调函数，您可以依赖这一事实。它使您可以更轻松地添加自定义 Header 解析器等。

FTP 传输的“Header”等于所有 FTP 服务器响应。它们实际上不是真正的Header，但在这种情况下，我们假装它们是！ ;-)

## POST 传输信息
看 curl_easy_getinfo.

## 多接口
本文档中详细描述的简单接口是一种同步接口，一次传输一个文件，直到完成才返回。

另一方面，多接口允许您的程序同时在两个方向上传输多个文件，而不必强迫您使用多个线程。这个名字可能让人觉得多接口是为多线程程序设计的，但事实几乎相反。多接口允许单线程应用程序执行多线程程序可以执行的相同类型的多个同时传输。它允许多线程传输的许多好处，而无需管理和同步多个线程的复杂性。

更复杂的是，多接口甚至有两个版本。基于事件的，也称为 multi_socket 和设计用于 select() 的“普通socket”。有关基于 multi_socket 事件的 API 的详细信息，请参阅 libcurl-multi.3 手册页，这里的描述是面向 select() 的。

要使用此接口，最好先了解如何使用简单接口的基础知识。多接口只是一种通过将多个 easy 句柄添加到“多堆栈”中来同时进行多次传输的方法。

你创建你想要的 easy 句柄，每个并发传输一个，你设置所有选项，就像你在上面学到的那样，然后你用 curl_multi_init 创建一个多句柄，并用 curl_multi_add_handle 将所有这些 easy 句柄添加到该多句柄。

当您添加了当前的句柄后（您仍然可以随时添加新句柄），您可以通过调用 curl_multi_perform 开始传输。

curl_multi_perform 是异步的。它只会执行现在可以完成的操作，然后将控制权返回给您的程序。它旨在永不阻塞。您需要不断调用该函数，直到所有传输完成。

此接口的最佳用法是在所有可能的文件描述符或 socket 上执行 select() 以了解何时再次调用 libcurl。这也使您可以轻松地等待和响应您自己的应用程序的 socket /句柄上的操作。您可以通过使用 curl_multi_fdset 来确定 select() 用于什么，它使用 libcurl 目前使用的特定文件描述符为您填充一组 fd_set 变量。

当您然后调用 select() 时，它将在文件之一处理信号操作时返回，然后您调用 curl_multi_perform 以允许 libcurl 执行它想做的事情。请注意，libcurl 还具有一些超时代码，因此我们建议您在再次调用 curl_multi_perform 之前不要在 select() 上使用很长的超时。提供 curl_multi_timeout 是为了帮助您获得合适的超时时间。

您应该使用的另一个预防措施：始终在 select() 调用之前立即调用 curl_multi_fdset，因为当前的文件描述符集可能会在任何 curl 函数调用中更改。

如果要停止传输堆栈中的某个 easy 句柄，可以使用 curl_multi_remove_handle 删除单个 easy 句柄。请记住，easy 句柄应该是 curl_easy_cleanuped。

当多堆栈中的传输完成时，运行传输的计数器（由 curl_multi_perform 填充）将减少。当数量达到0时，所有传输都完成。

curl_multi_info_read 可用于获取有关已完成传输的信息。然后它会返回每次轻松传输的 CURLcode，让您能够确定每个单独传输的成功与否。

## SSL、证书和其他技巧
[seeding、密码、密钥、证书、ENGINE、CA 证书]

## 在 easy 句柄 之间共享数据
使用简易接口时，您可以在 easy 句柄之间共享一些数据，使用多接口时会自动共享一些数据。

当您将 easy 句柄添加到多句柄时，这些 easy 句柄将自动共享大量数据，否则在使用简易接口时，这些数据将保留在每个 easy 句柄的基础上。

DNS 缓存在多句柄内的句柄之间共享，使后续名称解析更快，并且保留的连接池也可以共享以更好地允许持久连接和连接重用。如果您使用的是简单接口，您仍然可以使用共享接口在特定的 easy 句柄之间共享这些，请参阅 libcurl-share。

有些东西永远不会自动共享，不在多句柄中，例如 cookie，因此共享的唯一方法是使用共享接口。

## 脚注
- [1] libcurl 7.10.3 及更高版本能够在使用未知大小的数据完成 HTTP 上传的情况下切换到分块传输编码。
- [2] 当 libcurl 被构建并用作 DLL 时，这会发生在 Windows 机器上。但是，如果您与静态库链接，您仍然可以在 Windows 上执行此操作。
- [3] curl-config 工具是在构建时生成的（在类 Unix 系统上），应该使用“make install”或类似的指令来安装库、头文件、手册页等。
- [4] 此行为在 7.17.0 之前的版本中有所不同，其中字符串必须在 curl_easy_setopt 调用结束后保持有效。
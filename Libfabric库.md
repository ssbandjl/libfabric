# Libfabric 库

[Libfabric](https://ofiwg.github.io/libfabric/) 是 OpenFabrics Interfaces (OFI) 的核心组件，也是用于定义和导出 OFI 用户空间 API 的库。它通常是应用程序直接处理的唯一软件。



![img](https://pic1.zhimg.com/80/v2-75fb344eaf89d646650d7d9605d8af6c_720w.jpg)







# libfabric

Open Fabrics Interfaces (OFI) 是一个专注于将结构通信服务导出到应用程序的框架。

有关更多详细信息，请参阅[OFI 网站](http://libfabric.org/)，包括项目的描述和概述，以及 libfabric API 的详细文档。

## 安装预构建的 libfabric 包

在 OS X 上，可以使用 [Homebrew](https://github.com/Homebrew/homebrew)包管理器使用以下命令安装最新版本的 libfabric：

```
$ brew install libfabric
```

Libfabric 预构建的二进制文件可以从其他来源获得，例如 Linux 发行版。

## 从源代码构建和安装 libfabric

分发 tarball 可从 Github [发布](https://github.com/ofiwg/libfabric/releases)选项卡中获得。

如果您从开发人员 git clone 构建 libfabric，则必须首先运行该`autogen.sh`脚本。这将调用 GNU Autotools 来引导 libfabric 的配置和构建机制。如果您是从官方发行版 tarball 构建 libfabric，则无需运行`autogen.sh`；libfabric 分发 tarball 已经为您自举。

Libfabric 目前支持 GNU/Linux、Free BSD 和 OS X。

### 配置选项

该`configure`脚本有许多内置选项（请参阅 参考资料`./configure --help`）。一些有用的选项是：

```
--prefix=<directory>
```

默认情况下`make install`会将文件放在`/usr`树中。该`--prefix`选项指定应该将 libfabric 文件安装到由 named 指定的树中`<directory>`。可执行文件将位于 `<directory>/bin`.

```
--with-valgrind=<directory>
```

安装 valgrind 的目录。如果找到 valgrind，则启用 valgrind 注释。这可能会导致性能损失。

```
--enable-debug
```

启用调试代码路径。这启用了各种额外的检查，并允许使用通常在生产构建中编译的最高详细日志输出。

```
--enable-<provider>=[yes|no|auto|dl|<directory>]
--disable-<provider>
```

这将启用或禁用名为 的提供程序`<provider>`。有效的选项是：

- `--enable-<provider>`auto （如果未指定选项，这是默认设置）

  如果满足其所有要求，将启用提供程序。如果不能满足其中一项要求，则禁用提供程序。

- `--enable-<provider>`是（如果指定了选项，这是默认设置）

  如果无法启用提供程序（例如，由于其某些要求不可用，则配置脚本将中止。

- 不

  禁用提供程序。这是 的同义词`--disable-<provider>`。

- dl

  启用提供程序并将其构建为可加载库。

- <目录>

  启用提供程序并使用`<directory>`.

### 例子

考虑以下示例：

```
$ ./configure --prefix=/opt/libfabric --disable-sockets && make -j 32 && sudo make install
```

这将告诉 libfabric 禁用提供程序，并在树`sockets`中安装 libfabric 。`/opt/libfabric`如果可能，将启用所有其他提供程序，并禁用所有调试功能。

或者：

```
$ ./configure --prefix=/opt/libfabric --enable-debug --enable-psm=dl && make -j 32 && sudo make install
```

这将告诉 libfabric 将`psm`提供程序启用为可加载库，启用所有调试代码路径，并将 libfabric 安装到`/opt/libfabric` 树中。如果可能，将启用所有其他提供程序。

### mac

```
=> Downloading https://github.com/ofiwg/libfabric/releases/download/v1.11.1/libfabric-1.11.1.tar.bz2
==> Downloading from https://objects.githubusercontent.com/github-production-release-asset-2e65be/22996097/c1232400-0bf9-11eb-9365-b4a0526cc57
######################################################################## 100.0%
==> autoreconf -fiv
==> ./configure --prefix=/usr/local/Cellar/libfabric/1.11.1
==> make install
```



## 验证安装

fi_info 实用程序可用于验证 libfabric 和提供程序安装，并提供有关提供程序支持和可用接口的详细信息。有关使用 fi_info 实用程序的详细信息，请参见`fi_info(1)`手册页。fi_info 作为 libfabric 包的一部分安装。

更全面的测试包可通过 fabtests 包获得。

## 提供者

### gni

------

该`gni`提供程序在 Cray XC (TM) 系统上运行，使用用户空间通用网络接口 ( `uGNI`)，该接口提供对 Aries 互连的低级访问。Aries 互连专为低延迟单边消息传递而设计，还包括对常见原子操作和优化集合的直接硬件支持。

有关更多详细信息，请参见`fi_gni(7)`手册页。

#### 依赖项

- 提供`gni`程序需要`gcc`4.9 或更高版本。

### opx

------

OPX 提供程序是针对 Omni-Path HPC 结构的更新后的 Libfabric 提供程序。Omni-Path 的另一个提供程序是 PSM2。

OPX 提供程序最初是 libfabric BGQ 提供程序的一个分支，为 Omni-Path hfi1 结构接口卡重新编写了特定于硬件的部分。因此，OPX 继承了 BGQ 驱动程序的几个理想特性，对大多数 HPC 操作的指令计数和缓存行占用空间的分析表明，OPX 在主机软件堆栈上的重量比 PSM2 更轻，从而带来更好的整体性能。

有关更多详细信息，请参见`fi_opx(7)`手册页。有关支持信息，请参阅[Cornelis 客户中心。](https://customercenter.cornelisnetworks.com/)

### psm

------

该`psm`提供程序通过英特尔 TrueScale Fabric 当前支持的 PSM 1.x 接口运行。PSM 提供了针对 MPI 实现进行了优化的标记匹配消息队列功能。PSM 还具有有限的 Active Message 支持，它尚未正式发布，但相当稳定，并且在源代码（OFED 版本的一部分）中有详细记录。提供`psm` 者利用标签匹配消息队列函数和活动消息函数来支持各种 libfabric 数据传输 API，包括标记消息队列、消息队列、RMA 和原子操作。

提供`psm`者可以使用该`psm2-compat`库，该库在 Intel Omni-Path Fabric 上公开 PSM 1.x 接口。

有关更多详细信息，请参见`fi_psm(7)`手册页。

### psm2

------

提供`psm2`程序在英特尔 Omni-Path Fabric 支持的 PSM 2.x 接口上运行。PSM 2.x 具有 PSM 1.x 的所有特性以及一组具有增强功能的新功能。由于 PSM 1.x 和 PSM 2.x 不兼容 ABI，因此`psm2`提供程序仅适用于 PSM 2.x，不支持 Intel TrueScale Fabric。

有关更多详细信息，请参见`fi_psm2(7)`手册页。

### psm3

------

该`psm3`提供程序为大多数动词 UD 和套接字设备提供优化的性能和可扩展性。[`rv`](https://github.com/intel/iefs-kernel-updates)在 Intel 的 E810 以太网 NIC 上运行和/或使用 Intel 的 Rendezvous 内核模块 ( )时，可以启用其他功能和优化。PSM 3.x 完全集成了 OFI 提供程序和底层 PSM3 协议/实现，并且仅导出 OFI API。

更多细节参见[`fi_psm3`(7)](https://ofiwg.github.io/libfabric/main/man/fi_psm3.7.html)。

### rxm

------

提供`ofi_rxm`程序是一个实用程序提供程序，它支持在核心提供程序的 MSG 端点上模拟的 RDM 端点。

有关详细信息，请参阅[`fi_rxm`(7)](https://ofiwg.github.io/libfabric/main/man/fi_rxm.7.html)。

### sockets

------

套接字提供程序已被弃用，取而代之的是 tcp、udp 和实用程序提供程序，它们提供了改进的性能和稳定性。

提供`sockets`程序是一个通用的提供程序，可以在任何支持 TCP 套接字的系统上使用。该提供程序并非旨在提供对常规 TCP 套接字的性能改进，而是允许开发人员即使在没有高性能结构硬件的平台上也能编写、测试和调试应用程序代码。套接字提供程序支持所有 libfabric 提供程序要求和接口。

有关更多详细信息，请参见`fi_sockets(7)`手册页。

### tcp

------

tcp 提供程序是一个优化的基于套接字的提供程序，支持可靠连接的端点。它旨在由需要 MSG 端点支持的应用程序直接使用，或与 rxm 提供程序一起用于需要 RDM 端点的应用程序。tcp 提供程序旨在为使用标准网络硬件的应用程序替换套接字提供程序。

有关更多详细信息，请参见`fi_tcp(7)`手册页。

### UDP

------

提供`udp`程序是一个基本提供程序，可以在任何支持 UDP 套接字的系统上使用。提供程序的目的不是提供常规 UDP 套接字的性能改进，而是允许应用程序和提供程序开发人员编写、测试和调试他们的代码。该`udp`提供程序构成了一个实用程序提供程序的基础，它可以在任何硬件上实现 libfabric 功能。

有关更多详细信息，请参见`fi_udp(7)`手册页。

### usnic

------

该`usnic`提供商旨在通过 Cisco UCS 服务器上的 Cisco VIC（虚拟化 NIC）硬件运行。它利用 VIC 的 Cisco usnic（用户空间 NIC）功能在以太网网络上启用超低延迟和其他卸载功能。

有关更多详细信息，请参见`fi_usnic(7)`手册页。

#### 依赖项

- 提供`usnic`程序依赖于`libnl`版本 1（有时称为`libnl`or `libnl1`）或版本 3（有时称为 `libnl3`）的库文件。如果您从源代码编译 libfabric 并希望启用 usNIC 支持，您还需要匹配的`libnl`头文件（例如，如果您使用`libnl`版本 3 构建，则需要版本 3 的头文件和库文件）。

#### 配置选项

```
--with-libnl=<directory>
```

如果指定，请查找 libnl 支持。如果未找到，`usnic` 则不会构建提供程序。如果`<directory>`指定，则检查目录并检查`libnl`版本 3。如果未找到版本 3，则检查版本 1。如果未`<directory>`指定参数，则此选项与`--with-usnic`.

### 动词verbs

------

动词提供程序使使用 OFI 的应用程序能够在任何动词硬件（Infiniband、iWarp 和 RoCE）上运行。它使用 Linux Verbs API 进行网络传输，并将 OFI 调用转换为适当的动词 API 调用。它使用 librdmacm 进行通信管理，使用 libibverbs 进行其他控制和数据传输操作。

有关更多详细信息，请参见`fi_verbs(7)`手册页。

#### 依赖项

- 动词提供程序需要 libibverbs（v1.1.8 或更新版本）和 librdmacm（v1.0.16 或更新版本）。如果您从源代码编译 libfabric 并希望启用动词支持，您还需要上述两个库的匹配头文件。如果库和头文件不在默认路径中，请在 CFLAGS、LDFLAGS 和 LD_LIBRARY_PATH 环境变量中指定它们。

The verbs provider enables applications using OFI to be run over any verbs hardware (Infiniband, iWarp, and RoCE). It uses the Linux Verbs API for network transport and translates OFI calls to appropriate verbs API calls. It uses **librdmacm** for communication management and **libibverbs** for other control and data transfer operations.

See the `fi_verbs(7)` man page for more details.

#### Dependencies

- The verbs provider requires libibverbs (v1.1.8 or newer) and librdmacm (v1.0.16 or newer). If you are compiling libfabric from source and want to enable verbs support, you will also need the matching header files for the above two libraries. If the libraries and header files are not in default paths, specify them in CFLAGS, LDFLAGS and LD_LIBRARY_PATH environment variables.



### bgq

------

提供`bgq`者是原生提供者，直接利用 Blue Gene/Q 系统的硬件接口来实现 libfabric 接口的各个方面，以完全支持 MPICH3 CH4。

有关更多详细信息，请参见`fi_bgq(7)`手册页。

#### 依赖项

- 提供`bgq`程序依赖于 Blue Gene/Q 驱动程序安装中的系统编程接口 (SPI) 和硬件接口 (HWI)。此外，还需要开源 Blue Gene/Q 系统文件。

#### 配置选项

```
--with-bgq-progress=(auto|manual)
```

如果指定，设置在 FABRIC_DIRECT 中启用的进度模式（默认为 FI_PROGRESS_MANUAL）。

```
--with-bgq-mr=(basic|scalable)
```

如果指定，则设置内存注册模式（默认为 FI_MR_BASIC）。

### 网络直通

------

Network Direct 提供程序使使用 OFI 的应用程序能够在任何动词硬件（Infiniband、iWarp 和 RoCE）上运行。它使用 Microsoft Network Direct SPI 进行网络传输，并将 OFI 调用转换为适当的 Network Direct API 调用。Network Direct 提供商使基于 OFI 的应用程序能够利用应用程序之间的零拷贝数据传输、内核绕过 I/O 生成和 Microsoft Windows 操作系统上的单面数据传输操作。如果设备的硬件供应商为其硬件实现了 Network Direct 服务提供者接口 (SPI)，则应用程序可以将 OFI 与在 Windows 操作系统上启用的 Network Direct 提供程序一起使用以公开网络设备的功能。

有关更多详细信息，请参见`fi_netdir(7)`手册页。

#### 依赖项

- Network Direct 提供程序需要 Network Direct SPI。如果您从源代码编译 libfabric 并希望启用 Network Direct 支持，您还需要 Network Direct SPI 的匹配头文件。如果库和头文件不在默认路径中（默认路径是provier 目录的根目录，即\prov\netdir\NetDirect，其中NetDirect 包含头文件），请在VS 项目的配置属性中指定它们。

### shm共享内存share memory

------

shm 提供程序使使用 OFI 的应用程序能够在共享内存上运行。

有关更多详细信息，请参见`fi_shm(7)`手册页。

#### 依赖项

- 共享内存提供程序仅适用于 Linux 平台，并利用内核支持“跨内存附加”(CMA) 数据副本进行大型传输。

### efa 弹性网络适配器

------

该提供程序允许在[Amazon EC2 Elastic Fabric Adapter (EFA)](https://aws.amazon.com/hpc/efa/)`efa`上使用支持 libfabric 的应用程序，这是一个定制的操作系统绕过硬件接口，用于 EC2 上的实例间通信。

有关详细信息，请参阅[`fi_efa`(7)](https://ofiwg.github.io/libfabric/main/man/fi_efa.7.html)。

## WINDOWS 使用说明

尽管不完全支持 Windows，但可以编译和链接您的库。

- 1. 首先，您需要 NetDirect 提供程序： Network Direct SDK/DDK 可以作为 NuGet 包（首选）从以下位置获得：

  https://www.nuget.org/packages/NetworkDirect

  或从以下位置下载：

  https://www.microsoft.com/en-us/download/details.aspx?id=36043 在页面上按下载按钮并选择 NetworkDirect_DDK.zip。

  `\NetDirect\include\`从下载的 NetworkDirect_DDK.zip:文件中提取头文件`<libfabricroot>\prov\netdir\NetDirect\`，或将 NetDirect 头文件的路径添加到 VS 包含路径中

- 1. 编译：libfabric 有 6 个 Visual Studio 解决方案配置：

     1-2：调试/发布 ICC（仅限英特尔编译器 XE 15.0） 3-4：调试/发布 v140（VS 2015 工具集） 5-6：调试/发布 v141（VS 2017 工具集） 7-8：调试/发布 v142（VS 2019 工具集）

  确保选择适合编译器的正确目标。默认情况下，该库将被编译为`<libfabricroot>\x64\<yourconfigchoice>`

- 1. 链接你的图书馆

  - 右键单击您的项目并选择属性。
  - 选择 C/C++ > General 并添加`<libfabricroot>\include`到“Additional include Directories”
  - 选择链接器>输入并添加`<libfabricroot>\x64\<yourconfigchoice>\libfabric.lib`到“附加依赖项”
  - 根据您正在构建的内容，您可能还需要复制`libfabric.dll`到您自己项目的目标文件夹中。



[Libfabric Programmer's Manual](https://ofiwg.github.io/libfabric)



# fabric(7) Libfabric Programmer's Manual

# NAME

fabric - Fabric Interface Library

# SYNOPSIS

```
#include <rdma/fabric.h>
```

Libfabric is a high-performance fabric software library designed to provide low-latency interfaces to fabric hardware.

# OVERVIEW

Libfabric provides ‘process direct I/O’ to application software communicating across fabric software and hardware. Process direct I/O, historically referred to as RDMA, allows an application to directly access network resources without operating system interventions. Data transfers can occur directly to and from application memory.

There are two components to the libfabric software:

- *Fabric Providers*

  Conceptually, a fabric provider may be viewed as a local hardware NIC driver, though a provider is not limited by this definition. The first component of libfabric is a general purpose framework that is capable of handling different types of fabric hardware. All fabric hardware devices and their software drivers are required to support this framework. Devices and the drivers that plug into the libfabric framework are referred to as fabric providers, or simply providers. Provider details may be found in [`fi_provider`(7)](https://ofiwg.github.io/libfabric/v1.14.1/man/fi_provider.7.html).

- *Fabric Interfaces*

  The second component is a set of communication operations. Libfabric defines several sets of communication functions that providers can support. It is not required that providers implement all the interfaces that are defined; however, providers clearly indicate which interfaces they do support.

# FABRIC INTERFACES

The fabric interfaces are designed such that they are cohesive and not simply a union of disjoint interfaces. The interfaces are logically divided into two groups: control interfaces and communication operations. The control interfaces are a common set of operations that provide access to local communication resources, such as address vectors and event queues. The communication operations expose particular models of communication and fabric functionality, such as message queues, remote memory access, and atomic operations. Communication operations are associated with fabric endpoints.

Applications will typically use the control interfaces to discover local capabilities and allocate necessary resources. They will then allocate and configure a communication endpoint to send and receive data, or perform other types of data transfers, with remote endpoints.

# CONTROL INTERFACES

The control interfaces APIs provide applications access to network resources. This involves listing all the interfaces available, obtaining the capabilities of the interfaces and opening a provider.

- *fi_getinfo - Fabric Information*

  The fi_getinfo call is the base call used to discover and request fabric services offered by the system. Applications can use this call to indicate the type of communication that they desire. The results from fi_getinfo, fi_info, are used to reserve and configure fabric resources.fi_getinfo returns a list of fi_info structures. Each structure references a single fabric provider, indicating the interfaces that the provider supports, along with a named set of resources. A fabric provider may include multiple fi_info structures in the returned list.

- *fi_fabric - Fabric Domain*

  A fabric domain represents a collection of hardware and software resources that access a single physical or virtual network. All network ports on a system that can communicate with each other through the fabric belong to the same fabric domain. A fabric domain shares network addresses and can span multiple providers. libfabric supports systems connected to multiple fabrics.

- *fi_domain - Access Domains*

  An access domain represents a single logical connection into a fabric. It may map to a single physical or virtual NIC or a port. An access domain defines the boundary across which fabric resources may be associated. Each access domain belongs to a single fabric domain.

- *fi_endpoint - Fabric Endpoint*

  A fabric endpoint is a communication portal. An endpoint may be either active or passive. Passive endpoints are used to listen for connection requests. Active endpoints can perform data transfers. Endpoints are configured with specific communication capabilities and data transfer interfaces.

- *fi_eq - Event Queue*

  Event queues, are used to collect and report the completion of asynchronous operations and events. Event queues report events that are not directly associated with data transfer operations.

- *fi_cq - Completion Queue*

  Completion queues are high-performance event queues used to report the completion of data transfer operations.

- *fi_cntr - Event Counters*

  Event counters are used to report the number of completed asynchronous operations. Event counters are considered light-weight, in that a completion simply increments a counter, rather than placing an entry into an event queue.

- *fi_mr - Memory Region*

  Memory regions describe application local memory buffers. In order for fabric resources to access application memory, the application must first grant permission to the fabric provider by constructing a memory region. Memory regions are required for specific types of data transfer operations, such as RMA transfers (see below).

- *fi_av - Address Vector*

  Address vectors are used to map higher level addresses, such as IP addresses, which may be more natural for an application to use, into fabric specific addresses. The use of address vectors allows providers to reduce the amount of memory required to maintain large address look-up tables, and eliminate expensive address resolution and look-up methods during data transfer operations.

# DATA TRANSFER INTERFACES

Fabric endpoints are associated with multiple data transfer interfaces. Each interface set is designed to support a specific style of communication, with an endpoint allowing the different interfaces to be used in conjunction. The following data transfer interfaces are defined by libfabric.

- *fi_msg - Message Queue*

  Message queues expose a simple, message-based FIFO queue interface to the application. Message data transfers allow applications to send and receive data with message boundaries being maintained.

  消息队列向应用程序公开了一个简单的、基于消息的 FIFO 队列接口。 消息数据传输允许应用程序在维护消息边界的情况下发送和接收数据。

- *fi_tagged - Tagged Message Queues*

  Tagged message lists expose send/receive data transfer operations built on the concept of tagged messaging. The tagged message queue is conceptually similar to standard message queues, but with the addition of 64-bit tags for each message. Sent messages are matched with receive buffers that are tagged with a similar value.

  标记消息列表公开了基于标记消息概念的发送/接收数据传输操作。 标记消息队列在概念上类似于标准消息队列，但为每条消息添加了 64 位标记。 发送的消息与标记有相似值的接收缓冲区匹配。

- *fi_rma - Remote Memory Access*

  RMA transfers are one-sided operations that read or write data directly to a remote memory region. Other than defining the appropriate memory region, RMA operations do not require interaction at the target side for the data transfer to complete.

  fi_rma - 远程内存访问
  RMA 传输是一种将数据直接读取或写入远程内存区域的单向操作。 除了定义适当的内存区域外，RMA 操作不需要在目标端进行交互来完成数据传输。

- *fi_atomic - Atomic*

  Atomic operations can perform one of several operations on a remote memory region. Atomic operations include well-known functionality, such as atomic-add and compare-and-swap, plus several other pre-defined calls. Unlike other data transfer interfaces, atomic operations are aware of the data formatting at the target memory region.fi_atomic - 原子的
  原子操作可以对远程内存区域执行多种操作之一。 原子操作包括众所周知的功能，例如 atomic-add 和 compare-and-swap，以及其他几个预定义的调用。 与其他数据传输接口不同，原子操作知道目标内存区域的数据格式。

# LOGGING INTERFACE

Logging can be controlled using the FI_LOG_LEVEL, FI_LOG_PROV, and FI_LOG_SUBSYS environment variables.

- *FI_LOG_LEVEL*

  FI_LOG_LEVEL controls the amount of logging data that is output. The following log levels are defined.

- - *Warn*

    Warn is the least verbose setting and is intended for reporting errors or warnings.

- - *Trace*

    Trace is more verbose and is meant to include non-detailed output helpful to tracing program execution.

- - *Info*

    Info is high traffic and meant for detailed output.

- - *Debug*

    Debug is high traffic and is likely to impact application performance. Debug output is only available if the library has been compiled with debugging enabled.

- *FI_LOG_PROV*

  The FI_LOG_PROV environment variable enables or disables logging from specific providers. Providers can be enabled by listing them in a comma separated fashion. If the list begins with the ‘^’ symbol, then the list will be negated. By default all providers are enabled.Example: To enable logging from the psm and sockets provider: FI_LOG_PROV=”psm,sockets”Example: To enable logging from providers other than psm: FI_LOG_PROV=”^psm”

- *FI_LOG_SUBSYS*

  The FI_LOG_SUBSYS environment variable enables or disables logging at the subsystem level. The syntax for enabling or disabling subsystems is similar to that used for FI_LOG_PROV. The following subsystems are defined.

- - *core*

    Provides output related to the core framework and its management of providers.

- - *fabric*

    Provides output specific to interactions associated with the fabric object.

- - *domain*

    Provides output specific to interactions associated with the domain object.

- - *ep_ctrl*

    Provides output specific to endpoint non-data transfer operations, such as CM operations.

- - *ep_data*

    Provides output specific to endpoint data transfer operations.

- - *av*

    Provides output specific to address vector operations.

- - *cq*

    Provides output specific to completion queue operations.

- - *eq*

    Provides output specific to event queue operations.

- - *mr*

    Provides output specific to memory registration.

# PROVIDER INSTALLATION AND SELECTION

The libfabric build scripts will install all providers that are supported by the installation system. Providers that are missing build prerequisites will be disabled. Installed providers will dynamically check for necessary hardware on library initialization and respond appropriately to application queries.

Users can enable or disable available providers through build configuration options. See ‘configure –help’ for details. In general, a specific provider can be controlled using the configure option ‘–enable-'. For example, '--enable-udp' (or '--enable-udp=yes') will add the udp provider to the build. To disable the provider, '--enable-udp=no' can be used.

Providers can also be enable or disabled at run time using the FI_PROVIDER environment variable. The FI_PROVIDER variable is set to a comma separated list of providers to include. If the list begins with the ‘^’ symbol, then the list will be negated.

Example: To enable the udp and tcp providers only, set: FI_PROVIDER=”udp,tcp”

The fi_info utility, which is included as part of the libfabric package, can be used to retrieve information about which providers are available in the system. Additionally, it can retrieve a list of all environment variables that may be used to configure libfabric and each provider. See [`fi_info`(1)](https://ofiwg.github.io/libfabric/v1.14.1/man/fi_info.1.html) for more details.

# ENVIRONMENT VARIABLE CONTROLS 

```fi_info -env | -e```

Core features of libfabric and its providers may be configured by an administrator through the use of environment variables. Man pages will usually describe the most commonly accessed variables, such as those mentioned above. However, libfabric defines interfaces for publishing and obtaining environment variables. These are targeted for providers, but allow applications and users to obtain the full list of variables that may be set, along with a brief description of their use.

A full list of variables available may be obtained by running the fi_info application, with the -e or –env command line option.

libfabric 及其提供程序的核心功能可以由管理员通过使用环境变量进行配置。 手册页通常会描述最常访问的变量，例如上面提到的那些。 但是，libfabric 定义了用于发布和获取环境变量的接口。 这些是针对提供者的，但允许应用程序和用户获取可以设置的变量的完整列表，以及它们的使用的简要描述。

可以通过使用 -e 或 –env 命令行选项运行 fi_info 应用程序来获得可用变量的完整列表。

# NOTES

## System Calls

Because libfabric is designed to provide applications direct access to fabric hardware, there are limits on how libfabric resources may be used in conjunction with system calls. These limitations are notable for developers who may be familiar programming to the sockets interface. Although limits are provider specific, the following restrictions apply to many providers and should be adhered to by applications desiring portability across providers.

- *fork*

  Fabric resources are not guaranteed to be available by child processes. This includes objects, such as endpoints and completion queues, as well as application controlled data buffers which have been assigned to the network. For example, data buffers that have been registered with a fabric domain may not be available in a child process because of copy on write restrictions.

## CUDA deadlock

In some cases, calls to `cudaMemcpy` within libfabric may result in a deadlock. This typically occurs when a CUDA kernel blocks until a `cudaMemcpy` on the host completes. To avoid this deadlock, `cudaMemcpy` may be disabled by setting `FI_HMEM_CUDA_ENABLE_XFER=0`. If this environment variable is set and there is a call to `cudaMemcpy` with libfabric, a warning will be emitted and no copy will occur. Note that not all providers support this option.

Another mechanism which can be used to avoid deadlock is Nvidia’s gdrcopy. Using gdrcopy requires an external library and kernel module available at https://github.com/NVIDIA/gdrcopy. Libfabric must be configured with gdrcopy support using the `--with-gdrcopy` option, and be run with `FI_HMEM_CUDA_USE_GDRCOPY=1`. This may be used in conjunction with the above option to provide a method for copying to/from CUDA device memory when `cudaMemcpy` cannot be used. Again, this may not be supported by all providers.

# ABI CHANGES

libfabric releases maintain compatibility with older releases, so that compiled applications can continue to work as-is, and previously written applications will compile against newer versions of the library without needing source code changes. The changes below describe ABI updates that have occurred and which libfabric release corresponds to the changes.

Note that because most functions called by applications actually call static inline functions, which in turn reference function pointers in order to call directly into providers, libfabric only exports a handful of functions directly. ABI changes are limited to those functions, most notably the fi_getinfo call and its returned attribute structures.

The ABI version is independent from the libfabric release version.

libfabric 版本保持与旧版本的兼容性，因此编译的应用程序可以继续按原样工作，并且以前编写的应用程序将针对较新版本的库进行编译，而无需更改源代码。 下面的更改描述了已发生的 ABI 更新以及与更改对应的 libfabric 版本。

请注意，由于应用程序调用的大多数函数实际上调用的是静态内联函数，而后者又引用函数指针以便直接调用提供程序，因此 libfabric 仅直接导出少数函数。 ABI 更改仅限于这些函数，最值得注意的是 fi_getinfo 调用及其返回的属性结构。

ABI 版本独立于 libfabric 发布版本。

ABI 1.0

## ABI 1.0

The initial libfabric release (1.0.0) also corresponds to ABI version 1.0. The 1.0 ABI was unchanged for libfabric major.minor versions 1.0, 1.1, 1.2, 1.3, and 1.4.

## ABI 1.1

A number of external data structures were appended starting with libfabric version 1.5. These changes included adding the fields to the following data structures. The 1.1 ABI was exported by libfabric versions 1.5 and 1.6.

- *fi_fabric_attr*

  Added api_version

- *fi_domain_attr*

  Added cntr_cnt, mr_iov_limit, caps, mode, auth_key, auth_key_size, max_err_data, and mr_cnt fields. The mr_mode field was also changed from an enum to an integer flag field.

- *fi_ep_attr*

  Added auth_key_size and auth_key fields.

## ABI 1.2

The 1.2 ABI version was exported by libfabric versions 1.7 and 1.8, and expanded the following structure.

- *fi_info*

  The fi_info structure was expanded to reference a new fabric object, fid_nic. When available, the fid_nic references a new set of attributes related to network hardware details.

## ABI 1.3

The 1.3 ABI version was exported by libfabric versions 1.9, 1.10, and 1.11. Added new fields to the following attributes:

- *fi_domain_attr*

  Added tclass

- *fi_tx_attr*

  Added tclass

## ABI 1.4

The 1.4 ABI version was exported by libfabric 1.12. Added fi_tostr_r, a thread-safe (re-entrant) version of fi_tostr.

## ABI 1.5

ABI version starting with libfabric 1.13. Added new fi_open API call.

## ABI 1.6

ABI version starting with libfabric 1.14. Added fi_log_ready for providers.

# SEE ALSO

[`fi_info`(1)](https://ofiwg.github.io/libfabric/v1.14.1/man/fi_info.1.html), [`fi_provider`(7)](https://ofiwg.github.io/libfabric/v1.14.1/man/fi_provider.7.html), [`fi_getinfo`(3)](https://ofiwg.github.io/libfabric/v1.14.1/man/fi_getinfo.3.html), [`fi_endpoint`(3)](https://ofiwg.github.io/libfabric/v1.14.1/man/fi_endpoint.3.html), [`fi_domain`(3)](https://ofiwg.github.io/libfabric/v1.14.1/man/fi_domain.3.html), [`fi_av`(3)](https://ofiwg.github.io/libfabric/v1.14.1/man/fi_av.3.html), [`fi_eq`(3)](https://ofiwg.github.io/libfabric/v1.14.1/man/fi_eq.3.html), [`fi_cq`(3)](https://ofiwg.github.io/libfabric/v1.14.1/man/fi_cq.3.html), [`fi_cntr`(3)](https://ofiwg.github.io/libfabric/v1.14.1/man/fi_cntr.3.html), [`fi_mr`(3)](https://ofiwg.github.io/libfabric/v1.14.1/man/fi_mr.3.html)





核心提供者直接在低级硬件和软件接口上实现 libfabric 接口。它们旨在支持特定类别的硬件，并且可能仅限于支持单个 NIC。核心提供者通常只支持有效映射到其底层硬件的 libfabric 特性和接口。

实用程序提供程序与核心提供程序的不同之处在于它们不与特定类别的设备相关联。相反，他们与核心提供者合作以扩展其功能，并在内部通过 libfabric 接口与核心提供者交互。实用程序提供程序通常用于支持特定端点类型而不是更简单的端点类型。例如，RXD 提供者在不可靠的数据报端点上实现可靠性。除非明确请求，否则实用程序提供程序不会覆盖套接字提供程序。

实用程序提供者在核心提供者的组件列表中显示为一个组件。请参见 fi_fabric(3)。对于不支持应用程序请求的功能集的核心提供程序，会自动启用实用程序提供程序。





Libfabric 提供了一个通用框架来支持多种类型的结构对象及其相关接口。 Fabric 提供商在根据特定的硬件限制选择他们能够并愿意支持的组件方面具有很大的灵活性。提供程序开发人员应参考 docs/provider 以获取有关框架提供的功能的信息，以协助提供程序实施。为了帮助开发应用程序，libfabric 指定了任何结构提供者必须满足的以下要求（如果应用程序提出请求）。

请注意，特定结构对象的实例化受应用程序配置参数的影响，不需要满足这些要求。

结构提供者必须支持至少一种端点类型。
所有端点都必须支持消息队列数据传输接口 (fi_ops_msg)。
宣布支持特定端点功能的端点必须支持相应的数据传输接口。
FI_ATOMIC - fi_ops_atomic
FI_RMA - fi_ops_rma
FI_TAGGED - fi_ops_tagged
端点必须支持它们支持的任何数据传输接口的所有传输和接收操作。
例外：如果操作仅可用于提供者不支持的操作，并且使用其他机制传达对该操作的支持，则操作可能会返回
FI_ENOSYS。例如，如果提供者不支持注入数据，它可以设置属性 inject_size = 0，并且使所有 fi_inject 操作失败。
该框架提供了可以使用的“msg”操作的包装器。例如，框架通过调用 sendmsg() 来实现 sendv() msg 操作。提供者可以参考一般操作，并提供 sendmsg() 实现。
提供者必须将所有操作设置为实现。函数指针不得为 NULL 或未初始化。该框架提供了返回 -FI_ENOSYS 的空函数，可用于此目的。
端点必须支持 CM 接口，如下所示：
FI_EP_MSG 端点必须支持所有 CM 操作。
FI_EP_DGRAM 端点必须支持 CM getname 和 setname。
FI_EP_RDM 端点必须支持 CM getname 和 setname。
支持无连接端点的提供者必须支持所有 AV 操作 (fi_ops_av)。
支持内存注册的提供者，必须支持所有的 MR 操作（fi_ops_mr）。
提供者应该支持完成队列和计数器。
如果不支持 FI_RMA_EVENT，则计数器支持仅限于本地事件。
完成队列必须支持 FI_CQ_FORMAT_CONTEXT 和 FI_CQ_FORMAT_MSG。
支持 FI_REMOTE_CQ_DATA 的提供者应支持 FI_CQ_FORMAT_DATA。
支持 FI_TAGGED 的提供者应支持 FI_CQ_FORMAT_TAGGED。
提供者应该是向前兼容的，并且必须能够针对扩展的 fi_xxx_ops 结构进行编译，这些结构定义了在编写提供者之后添加的新函数。任何未知函数都必须设置为 NULL。
提供者应在其手册页中记录他们支持的功能以及任何缺失的要求。
libfabric 的未来版本将自动为提供者启用更完整的功能集，这些提供者将其实现集中在 libfabric 功能的一小部分子集上。
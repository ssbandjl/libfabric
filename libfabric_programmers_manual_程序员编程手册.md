[Libfabric 程序员手册](https://ofiwg.github.io/libfabric)



# Libfabric OpenFabrics

[![在 GitHub 上FORK](https://camo.githubusercontent.com/652c5b9acfaddf3a9c326fa6bde407b87f7be0f4/68747470733a2f2f73332e616d617a6f6e6177732e636f6d2f6769746875622f726962626f6e732f666f726b6d655f72696768745f6f72616e67655f6666373630302e706e67)](https://github.com/ofiwg/libfabric)

![OpenFabrics 接口概述](https://ofiwg.github.io/libfabric/images/openfabric-interfaces-overview.png)

# 最新版本

- libfabric 库、单元测试和文档：[libfabric v1.19.0](https://github.com/ofiwg/libfabric/releases/tag/v1.19.0)（或[查看所有先前版本](https://github.com/ofiwg/libfabric/releases/)）。

Libfabric 的目标是每年发布 3 个主要版本，遵循以下时间表：3 月、7 月和 11 月。具体发布的时间会根据底层提供商的准备情况进行调整。

# 概述

Libfabric 也称为开放结构接口 (OFI)，为高性能并行和分布式应用程序定义了通信 API。它是一个低级通信库，抽象了各种网络技术。Libfabric 由 OFI 工作组（OFIWG，发音为“o-fee-wig”）开发，该工作组是 OpenFabrics[联盟 - OFA](http://www.openfabrics.org/)的一个子组。OFIWG 向任何人开放，不仅限于 OFA 成员。

libfabric 的目标是定义接口，以在应用程序和底层结构服务之间实现紧密的语义映射。具体来说，libfabric软件接口是与fabric硬件提供商和应用程序开发商共同设计的，重点关注HPC用户的需求。Libfabric 支持多种通信语义，与结构和硬件实现无关，并利用和扩展现有的 RDMA 开源社区。

Libfabric 旨在最大限度地减少应用程序之间的阻抗失配，包括 MPI、SHMEM、数据存储和 PGAS 等中间件以及结构通信硬件。其接口针对高带宽、低延迟的 NIC，目标是扩展到数万个节点。

Libfabric 的目标是支持 Linux、Free BSD、Windows 和 OS X。我们尽力支持所有主要的现代 Linux 发行版；但是，验证仅限于 Red Hat Enterprise Linux (RHEL) 和 SUSE Linux Enterprise Server (SLES) 的最新 2-3 个版本。对特定操作系统版本或发行版的支持是特定于供应商的。例外情况是基于 tcp 和 udp 的套接字提供程序在所有平台上都可用。

# 开发者资源

手册页中包含一份全面的开发人员指南。

- [开发者指南](https://ofiwg.github.io/libfabric/main/man/fi_guide.7.html)

我们精心编写了一组手册页来指定 libfabric API。

- v1.19.0 的手册页
  - 旧版本：[v1.18.2 的手册页](https://ofiwg.github.io/libfabric/v1.18.2/man/)
  - 旧版本：[v1.18.1 的手册页](https://ofiwg.github.io/libfabric/v1.18.1/man/)
  - 旧版本：[v1.18.0 的手册页](https://ofiwg.github.io/libfabric/v1.18.0/man/)
  - 旧版本：[v1.17.1 的手册页](https://ofiwg.github.io/libfabric/v1.17.1/man/)
  - 旧版本：[v1.17.0 的手册页](https://ofiwg.github.io/libfabric/v1.17.0/man/)
  - 旧版本：[v1.16.1 的手册页](https://ofiwg.github.io/libfabric/v1.16.1/man/)
  - 旧版本：[v1.16.0 的手册页](https://ofiwg.github.io/libfabric/v1.16.0/man/)
  - 旧版本：[v1.15.2 的手册页](https://ofiwg.github.io/libfabric/v1.15.2/man/)
  - 旧版本：[v1.15.1 的手册页](https://ofiwg.github.io/libfabric/v1.15.1/man/)
  - 旧版本：[v1.15.0 的手册页](https://ofiwg.github.io/libfabric/v1.15.0/man/)
  - 旧版本：[v1.14.1 的手册页](https://ofiwg.github.io/libfabric/v1.14.1/man/)
  - 旧版本：[v1.14.0 的手册页](https://ofiwg.github.io/libfabric/v1.14.0/man/)
  - 旧版本：[v1.13.2 的手册页](https://ofiwg.github.io/libfabric/v1.13.2/man/)
  - 旧版本：[v1.13.1 的手册页](https://ofiwg.github.io/libfabric/v1.13.1/man/)
  - 旧版本：[v1.13.0 的手册页](https://ofiwg.github.io/libfabric/v1.13.0/man/)
  - 旧版本：[v1.12.1 的手册页](https://ofiwg.github.io/libfabric/v1.12.1/man/)
  - 旧版本：[v1.12.0 的手册页](https://ofiwg.github.io/libfabric/v1.12.0/man/)
  - 旧版本：[v1.11.2 的手册页](https://ofiwg.github.io/libfabric/v1.11.2/man/)
  - 旧版本：[v1.11.1 的手册页](https://ofiwg.github.io/libfabric/v1.11.1/man/)
  - 旧版本：[v1.11.0 的手册页](https://ofiwg.github.io/libfabric/v1.11.0/man/)
  - 旧版本：[v1.10.1 的手册页](https://ofiwg.github.io/libfabric/v1.10.1/man/)
  - 旧版本：[v1.10.0 的手册页](https://ofiwg.github.io/libfabric/v1.10.0/man/)
  - 旧版本：[v1.9.1 的手册页](https://ofiwg.github.io/libfabric/v1.9.1/man/)
  - 旧版：[v1.9.0 的手册页](https://ofiwg.github.io/libfabric/v1.9.0/man/)
  - 旧版本：[v1.8.1 的手册页](https://ofiwg.github.io/libfabric/v1.8.1/man/)
  - 旧版本：[v1.8.0 的手册页](https://ofiwg.github.io/libfabric/v1.8.0/man/)
  - 旧版：[v1.7.2 的手册页](https://ofiwg.github.io/libfabric/v1.7.2/man/)
  - 旧版本：[v1.7.1 的手册页](https://ofiwg.github.io/libfabric/v1.7.1/man/)
  - 旧版本：[v1.7.0 的手册页](https://ofiwg.github.io/libfabric/v1.7.0/man/)
  - 旧版本：[v1.6.2 的手册页](https://ofiwg.github.io/libfabric/v1.6.2/man/)
  - 旧版本：[v1.6.1 的手册页](https://ofiwg.github.io/libfabric/v1.6.1/man/)
  - 旧版本：[v1.6.0 的手册页](https://ofiwg.github.io/libfabric/v1.6.0/man/)
  - 旧版本：[v1.5.4 的手册页](https://ofiwg.github.io/libfabric/v1.5.4/man/)
  - 旧版本：[v1.5.3 的手册页](https://ofiwg.github.io/libfabric/v1.5.3/man/)
  - 旧版本：[v1.5.2 的手册页](https://ofiwg.github.io/libfabric/v1.5.2/man/)
  - 旧版：[v1.5.1 的手册页](https://ofiwg.github.io/libfabric/v1.5.1/man/)
  - 旧版本：[v1.5.0 的手册页](https://ofiwg.github.io/libfabric/v1.5.0/man/)
  - 旧版本：[v1.4.2 的手册页](https://ofiwg.github.io/libfabric/v1.4.2/man/)
  - 旧版：[v1.4.1 的手册页](https://ofiwg.github.io/libfabric/v1.4.1/man/)
  - 旧版本：[v1.4.0 的手册页](https://ofiwg.github.io/libfabric/v1.4.0/man/)
  - 旧版本：[v1.3.0 的手册页](https://ofiwg.github.io/libfabric/v1.3.0/man/)
  - 旧版本：[v1.2.0 的手册页](https://ofiwg.github.io/libfabric/v1.2.0/man/)
  - 旧版本：[v1.1.1 的手册页](https://ofiwg.github.io/libfabric/v1.1.1/man/)
  - 旧版本：[v1.1.0 的手册页](https://ofiwg.github.io/libfabric/v1.1.0/man/)
  - 旧版本：[v1.0.0 的手册页](https://ofiwg.github.io/libfabric/v1.0.0/man/)
- [现任开发主管的手册页](https://ofiwg.github.io/libfabric/main/man/)

[测试应用程序集](https://github.com/ofiwg/libfabric/tree/main/fabtests)- 这些测试侧重于验证 libfabric 开发，但也强调应用程序如何使用 libfabric 的各个方面。

此外，开发人员可能会发现下面列出的文档有助于更详细地理解 libfabric 架构和目标。

- [2019 年 2 月 OFA 网络研讨会：libfabric 概述](https://www.slideshare.net/seanhefty/ofi-overview-2019-webinar)- libfabic 的一般介绍。
- [libfabric 架构简短介绍](https://www.slideshare.net/seanhefty/ofi-overview)- 推荐给 libfabric 新手。
- [开发人员教程 - 来自 HOTI '17](https://www.slideshare.net/seanhefty/2017-ofihotitutorial) - 介绍设计指南、架构，然后介绍中间件用例（PGAS 和 MPICH）
- [开发人员教程 - 来自 SC '15](https://www.slideshare.net/dgoodell/ofi-libfabric-tutorial) - 介绍低级接口详细信息，然后是使用 API 的应用程序和中间件（MPI、SHMEM）示例。
- [写入 libfabric 的入门指南](https://www.slideshare.net/JianxinXiong/getting-started-with-libfabric)

# 开放合作

libfabric 代码库正在[主 OFIWG libfabric GitHub 存储库](https://github.com/ofiwg/libfabric)中开发。有两个用于 OFIWG 讨论的邮件列表：

- [Libfabric 用户邮件列表](http://lists.openfabrics.org/mailman/listinfo/libfabric-users)- 旨在解决有关 Libfabric 库的一般用户问题，包括尝试在其应用程序中使用 libfabric 的开发人员提出的问题。
- [OFI 工作组邮件列表](http://lists.openfabrics.org/mailman/listinfo/ofiwg)- 用于 OFI API 本身的讨论和开发，以及 libfabric 库的持续开发。

每隔一个星期二的 OFIWG Webexe 通知将发送到 OFIWG 邮件列表。任何人都可以加入来聆听并参与 Libfabric 的设计。Webex 信息可从 OpenFabrice 联盟日历中获取。

------

[© 2023 OpenFabrics 接口工作组在Jekyll Bootstrap](http://jekyllbootstrap.com/) 和[Twitter Bootstrap](http://twitter.github.com/bootstrap/)的帮助下



# EN

[Libfabric Programmer's Manual](https://ofiwg.github.io/libfabric)



# Libfabric OpenFabrics

[![Fork me on GitHub](https://camo.githubusercontent.com/652c5b9acfaddf3a9c326fa6bde407b87f7be0f4/68747470733a2f2f73332e616d617a6f6e6177732e636f6d2f6769746875622f726962626f6e732f666f726b6d655f72696768745f6f72616e67655f6666373630302e706e67)](https://github.com/ofiwg/libfabric)

![OpenFabrics Interface Overview](https://ofiwg.github.io/libfabric/images/openfabric-interfaces-overview.png)

# Latest releases

- The libfabric library, unit tests, and documentation: [libfabric v1.19.0](https://github.com/ofiwg/libfabric/releases/tag/v1.19.0) (or [see all prior releases](https://github.com/ofiwg/libfabric/releases/)).

Libfabric targets 3 major releases per year, following this schedule: March, July, and November. The timing of a specific release is adjusted based on the readiness of the underlying providers.

# Overview

Libfabric, also known as Open Fabrics Interfaces (OFI), defines a communication API for high-performance parallel and distributed applications. It is a low-level communication library that abstracts diverse networking technologies. Libfabric is developed by the OFI Working Group (OFIWG, pronounced “o-fee-wig”), a subgroup of the [OpenFabrics Alliance - OFA](http://www.openfabrics.org/). Participation in the OFIWG is open to anyone, and not restricted to members of OFA.

The goal of libfabric is to define interfaces that enable a tight semantic map between applications and underlying fabric services. Specifically, libfabric software interfaces have been co-designed with fabric hardware providers and application developers, with a focus on the needs of HPC users. Libfabric supports multiple communication semantics, is fabric and hardware implementation agnostic, and leverages and expands the existing RDMA open source community.

Libfabric is designed to minimize the impedance mismatch between applications, including middleware such as MPI, SHMEM, data storage, and PGAS, and fabric communication hardware. Its interfaces target high-bandwidth, low-latency NICs, with a goal to scale to tens of thousands of nodes.

Libfabric targets support for the Linux, Free BSD, Windows, and OS X. A reasonable effort is made to support all major, modern Linux distributions; however, validation is limited to the most recent 2-3 releases of Red Hat Enterprise Linux (RHEL) and SUSE Linux Enterprise Server (SLES). Support for a particular operating system version or distribution is vendor specific. The exceptions are the tcp and udp based socket providers are available on all platforms.

# Developer Resources

A comprehensive developer’s guide is included with the man pages.

- [Developer Guide](https://ofiwg.github.io/libfabric/main/man/fi_guide.7.html)

A set of man pages have been carefully written to specify the libfabric API.

- Man pages for v1.19.0
  - Older: [Man pages for v1.18.2](https://ofiwg.github.io/libfabric/v1.18.2/man/)
  - Older: [Man pages for v1.18.1](https://ofiwg.github.io/libfabric/v1.18.1/man/)
  - Older: [Man pages for v1.18.0](https://ofiwg.github.io/libfabric/v1.18.0/man/)
  - Older: [Man pages for v1.17.1](https://ofiwg.github.io/libfabric/v1.17.1/man/)
  - Older: [Man pages for v1.17.0](https://ofiwg.github.io/libfabric/v1.17.0/man/)
  - Older: [Man pages for v1.16.1](https://ofiwg.github.io/libfabric/v1.16.1/man/)
  - Older: [Man pages for v1.16.0](https://ofiwg.github.io/libfabric/v1.16.0/man/)
  - Older: [Man pages for v1.15.2](https://ofiwg.github.io/libfabric/v1.15.2/man/)
  - Older: [Man pages for v1.15.1](https://ofiwg.github.io/libfabric/v1.15.1/man/)
  - Older: [Man pages for v1.15.0](https://ofiwg.github.io/libfabric/v1.15.0/man/)
  - Older: [Man pages for v1.14.1](https://ofiwg.github.io/libfabric/v1.14.1/man/)
  - Older: [Man pages for v1.14.0](https://ofiwg.github.io/libfabric/v1.14.0/man/)
  - Older: [Man pages for v1.13.2](https://ofiwg.github.io/libfabric/v1.13.2/man/)
  - Older: [Man pages for v1.13.1](https://ofiwg.github.io/libfabric/v1.13.1/man/)
  - Older: [Man pages for v1.13.0](https://ofiwg.github.io/libfabric/v1.13.0/man/)
  - Older: [Man pages for v1.12.1](https://ofiwg.github.io/libfabric/v1.12.1/man/)
  - Older: [Man pages for v1.12.0](https://ofiwg.github.io/libfabric/v1.12.0/man/)
  - Older: [Man pages for v1.11.2](https://ofiwg.github.io/libfabric/v1.11.2/man/)
  - Older: [Man pages for v1.11.1](https://ofiwg.github.io/libfabric/v1.11.1/man/)
  - Older: [Man pages for v1.11.0](https://ofiwg.github.io/libfabric/v1.11.0/man/)
  - Older: [Man pages for v1.10.1](https://ofiwg.github.io/libfabric/v1.10.1/man/)
  - Older: [Man pages for v1.10.0](https://ofiwg.github.io/libfabric/v1.10.0/man/)
  - Older: [Man pages for v1.9.1](https://ofiwg.github.io/libfabric/v1.9.1/man/)
  - Older: [Man pages for v1.9.0](https://ofiwg.github.io/libfabric/v1.9.0/man/)
  - Older: [Man pages for v1.8.1](https://ofiwg.github.io/libfabric/v1.8.1/man/)
  - Older: [Man pages for v1.8.0](https://ofiwg.github.io/libfabric/v1.8.0/man/)
  - Older: [Man pages for v1.7.2](https://ofiwg.github.io/libfabric/v1.7.2/man/)
  - Older: [Man pages for v1.7.1](https://ofiwg.github.io/libfabric/v1.7.1/man/)
  - Older: [Man pages for v1.7.0](https://ofiwg.github.io/libfabric/v1.7.0/man/)
  - Older: [Man pages for v1.6.2](https://ofiwg.github.io/libfabric/v1.6.2/man/)
  - Older: [Man pages for v1.6.1](https://ofiwg.github.io/libfabric/v1.6.1/man/)
  - Older: [Man pages for v1.6.0](https://ofiwg.github.io/libfabric/v1.6.0/man/)
  - Older: [Man pages for v1.5.4](https://ofiwg.github.io/libfabric/v1.5.4/man/)
  - Older: [Man pages for v1.5.3](https://ofiwg.github.io/libfabric/v1.5.3/man/)
  - Older: [Man pages for v1.5.2](https://ofiwg.github.io/libfabric/v1.5.2/man/)
  - Older: [Man pages for v1.5.1](https://ofiwg.github.io/libfabric/v1.5.1/man/)
  - Older: [Man pages for v1.5.0](https://ofiwg.github.io/libfabric/v1.5.0/man/)
  - Older: [Man pages for v1.4.2](https://ofiwg.github.io/libfabric/v1.4.2/man/)
  - Older: [Man pages for v1.4.1](https://ofiwg.github.io/libfabric/v1.4.1/man/)
  - Older: [Man pages for v1.4.0](https://ofiwg.github.io/libfabric/v1.4.0/man/)
  - Older: [Man pages for v1.3.0](https://ofiwg.github.io/libfabric/v1.3.0/man/)
  - Older: [Man pages for v1.2.0](https://ofiwg.github.io/libfabric/v1.2.0/man/)
  - Older: [Man pages for v1.1.1](https://ofiwg.github.io/libfabric/v1.1.1/man/)
  - Older: [Man pages for v1.1.0](https://ofiwg.github.io/libfabric/v1.1.0/man/)
  - Older: [Man pages for v1.0.0](https://ofiwg.github.io/libfabric/v1.0.0/man/)
- [Man pages for current head of development](https://ofiwg.github.io/libfabric/main/man/)

[Set of test applications](https://github.com/ofiwg/libfabric/tree/main/fabtests) - These tests focus on validating libfabric development, but also highlight how an application might use various aspects of libfabric.

Additionally, developers may find the documents listed below useful in understanding the libfabric architecture and objectives in more detail.

- [Feb 2019 OFA Webinar: Overview of libfabric](https://www.slideshare.net/seanhefty/ofi-overview-2019-webinar) - general introduction to libfabic.
- [A Short Introduction to the libfabric Architecture](https://www.slideshare.net/seanhefty/ofi-overview) - recommended for anyone new to libfabric.
- [Developer Tutorial - from HOTI ‘17](https://www.slideshare.net/seanhefty/2017-ofihotitutorial) - walks through design guidelines, architecture, followed by middleware use cases (PGAS and MPICH)
- [Developer Tutorial - from SC ‘15](https://www.slideshare.net/dgoodell/ofi-libfabric-tutorial) - walks through low-level interface details, followed by examples of application and middleware (MPI, SHMEM) using the APIs.
- [Starting Guide for Writing to libfabric](https://www.slideshare.net/JianxinXiong/getting-started-with-libfabric)

# Open Collaboration

The libfabric code base is being developed in [the main OFIWG libfabric GitHub repository](https://github.com/ofiwg/libfabric). There are two mailing lists for OFIWG discussions:

- [The Libfabric users mailing list](http://lists.openfabrics.org/mailman/listinfo/libfabric-users) - intended for general user questions about the Libfabric library, to include questions from developers trying to use libfabric in their applications.
- [The OFI working group mailing list](http://lists.openfabrics.org/mailman/listinfo/ofiwg) - used for the discussion and development of the OFI APIs themselves, and by extension, the continued development of the libfabric library.

Notices of the every-other-Tuesday OFIWG Webexes are sent to the OFIWG mailing list. Anyone can join the calls to listen and participate in the design of Libfabric. Webex information is available from the OpenFabrice Alliance calendar.

------

© 2023 OpenFabrics Interfaces Working Group with help from [Jekyll Bootstrap](http://jekyllbootstrap.com/) and [Twitter Bootstrap](http://twitter.github.com/bootstrap/)
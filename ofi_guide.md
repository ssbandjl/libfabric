---
layout: page
title: High Performance Network Programming with OFI
tagline: Libfabric (v1.4) Programmer's Guide
---
{% include JB/setup %}

参考: 

https://ofiwg.github.io/libfabric/v1.15.2/man/fi_av.3.html



# OFI指导

# Introduction OFI简介 

OpenFabrics Interfaces, or OFI, is a framework focused on exporting fabric communication services to applications.  OFI is specifically designed to meet the performance and scalability requirements of high-performance computing (HPC) applications, such as MPI, SHMEM, PGAS, DBMS, and enterprise applications, running in a tightly coupled network environment.  The key components of OFI are: **application interfaces, provider libraries, kernel services, daemons, and test applications.**

Libfabric is a core component of OFI. It is the library that defines and exports the user-space API of OFI, and is typically the only software that applications deal with directly. Libfabric is agnostic to the underlying networking protocols, as well as the implementation of the networking devices. 

The goal of OFI, and libfabric specifically, is to define interfaces that enable a tight semantic map between applications and underlying fabric services. Specifically, libfabric software interfaces have been co-designed with fabric hardware providers and application developers, with a focus on the needs of HPC users.

This guide describes the libfabric architecture and interfaces.  It provides insight into the motivation for its design, and aims to instruct developers on how the features of libfabric may best be employed.

OpenFabrics 接口或 OFI 是一个专注于将结构通信服务导出到应用程序的框架。 OFI 专为满足在紧密耦合的网络环境中运行的高性能计算 (HPC) 应用程序（例如 MPI、SHMEM、PGAS、DBMS 和企业应用程序）的性能和可扩展性要求而设计。 OFI 的关键组件是：应用程序接口、提供程序库、内核服务、守护程序和测试应用程序(如ping_pong)。

Libfabric 是 OFI 的核心组件。它是定义和导出 OFI 的用户空间 API 的库，通常是应用程序直接处理的唯一软件。 Libfabric 与底层网络协议以及网络设备的实现无关。

OFI 的目标，特别是 libfabric，是定义接口，在应用程序和底层结构服务之间实现紧密的语义映射。具体来说，**libfabric 软件接口是与 Fabric 硬件提供商和应用程序开发人员共同设计的，重点是 HPC 用户的需求**。

本指南描述了 libfabric 架构和接口。它提供了对其设计动机的洞察，旨在指导开发人员如何最好地利用 libfabric 的特性。

# Review of Sockets Communication

The sockets API is a widely used networking API.  This guide assumes that a reader has a working knowledge of programming to sockets.  It makes reference to socket based communications throughout in an effort to help explain libfabric concepts and how they relate or differ from the socket API. To be clear, there is no intent to criticize the socket API.  The objective is to use sockets as a starting reference point in order to explain certain network features or limitations.  The following sections provide a high-level overview of socket semantics for reference.

套接字 API 是一种广泛使用的网络 API。 本指南假定读者具有套接字编程的工作知识。 它在整个过程中都引用了基于套接字的通信，以帮助解释 libfabric 概念以及它们与套接字 API 的关系或不同之处。 需要明确的是，没有批评套接字 API 的意图。 目的是使用套接字作为起始参考点，以解释某些网络功能或限制。 以下部分提供了套接字语义的高级概述以供参考。

## Connected (TCP) Communication

The most widely used type of socket is SOCK_STREAM.  This sort of socket usually runs over TCP/IP, and as a result is often referred to as a 'TCP' socket.  TCP sockets are connection-oriented, requiring an explicit connection setup before data transfers can occur.  **A TCP socket can only transfer data to a single peer socket**.

Applications using TCP sockets are typically labeled as either a client or server.  Server applications listen for connection request, and accept them when they occur.  Clients, on the other hand, initiate connections to the server.  After a connection has been established, data transfers between a client and server are similar.  The following code segments highlight the general flow for a sample client and server.  Error handling and some subtleties of the socket API are omitted for brevity.

最广泛使用的套接字类型是 SOCK_STREAM。 这种套接字通常在 TCP/IP 上运行，因此通常被称为“TCP”套接字。 TCP 套接字是面向连接的，在发生数据传输之前需要明确的连接设置。 TCP 套接字只能将数据传输到单个对等套接字。

使用 TCP 套接字的应用程序通常被标记为客户端或服务器。 服务器应用程序侦听连接请求，并在它们发生时接受它们。 另一方面，客户端启动与服务器的连接。 建立连接后，客户端和服务器之间的数据传输是相似的。 以下代码段突出显示了示例客户端和服务器的一般流程。 为简洁起见，省略了错误处理和套接字 API 的一些细微之处。

```
/* Example server code flow to initiate listen 服务端 */
struct addrinfo *ai, hints;
int listen_fd;

memset(&hints, 0, sizeof hints);
hints.ai_socktype = SOCK_STREAM;
hints.ai_flags = AI_PASSIVE;
getaddrinfo(NULL, "7471", &hints, &ai);

listen_fd = socket(ai->ai_family, SOCK_STREAM, 0);
bind(listen_fd, ai->ai_addr, ai->ai_addrlen);
freeaddrinfo(ai);

fcntl(listen_fd, F_SETFL, O_NONBLOCK);
listen(listen_fd, 128);
```

In this example, the server will listen for connection requests on port 7471 across all addresses in the system.  The call to getaddrinfo() is used to form the local socket address.  The node parameter is set to NULL, which result in a wild card IP address being returned.  The port is hard-coded to 7471.  The AI_PASSIVE flag signifies that the address will be used by the listening side of the connection.

This example will work with both IPv4 and IPv6.  The getaddrinfo() call abstracts the address format away from the server, improving its portability.  Using the data returned by getaddrinfo(), the server allocates a socket of type SOCK_STREAM, and binds the socket to port 7471.

In practice, most enterprise-level applications make use of non-blocking sockets.  The fcntl() command sets the listening socket to non-blocking mode.  This will affect how the server processes connection requests (shown below).  Finally, the server starts listening for connection requests by calling listen.  Until listen is called, connection requests that arrive at the server will be rejected by the operating system.

在此示例中，服务器将在端口 7471 上侦听系统中所有地址的连接请求。对 getaddrinfo() 的调用用于形成本地套接字地址。 node 参数设置为 NULL，这将导致返回通配符 IP 地址。该端口被硬编码为 7471。AI_PASSIVE 标志表示该地址将由连接的侦听方使用。

此示例适用于 IPv4 和 IPv6。 getaddrinfo() 调用将地址格式从服务器中抽象出来，提高了它的可移植性。使用 getaddrinfo() 返回的数据，服务器分配一个 SOCK_STREAM 类型的套接字，并将套接字绑定到端口 7471。

实际上，大多数企业级应用程序都使用非阻塞套接字。 fcntl() 命令将侦听套接字设置为非阻塞模式。这将影响服务器处理连接请求的方式（如下所示）。最后，服务器通过调用listen开始监听连接请求。在调用listen之前，到达服务器的连接请求将被操作系统拒绝。

```
/* Example client code flow to start connection 客户端 */
struct addrinfo *ai, hints;
int client_fd;

memset(&hints, 0, sizeof hints);
hints.ai_socktype = SOCK_STREAM;
getaddrinfo("10.31.20.04", "7471", &hints, &ai);

client_fd = socket(ai->ai_family, SOCK_STREAM, 0);
fcntl(client_fd, F_SETFL, O_NONBLOCK);

connect(client_fd, ai->ai_addr, ai->ai_addrlen);
freeaddrinfo(ai);
```

Similar to the server, the client makes use of getaddrinfo().  Since the AI_PASSIVE flag is not specified, the given address is treated as that of the destination.  The client expects to reach the server at IP address 10.31.20.04, port 7471.  For this example the address is hard-coded into the client.  More typically, the address will be given to the client via the command line, through a configuration file, or from a service.  Often the port number will be well-known, and the client will find the server by name, with DNS (domain name service) providing the name to address resolution.  Fortunately, the getaddrinfo call can be used to convert host names into IP addresses.

Whether the client is given the server's network address directly or a name which must be translated into the network address, the mechanism used to provide this information to the client varies widely.  A simple mechanism that is commonly used is for users to provide the server's address using a command line option.  The problem of telling applications where its peers are located increases significantly for applications that communicate with hundreds to millions of peer processes, often requiring a separate, dedicated application to solve.  For a typical client-server socket application, this is not an issue, so we will defer more discussion until later.

Using the getaddrinfo() results, the client opens a socket, configures it for non-blocking mode, and initiates the connection request.  At this point, the network stack has sent a request to the server to establish the connection.  Because the socket has been set to non-blocking, the connect call will return immediately and not wait for the connection to be established.  As a result any attempt to send data at this point will likely fail.

与服务器类似，客户端使用 getaddrinfo()。由于未指定 **AI_PASSIVE** 标志，因此将给定地址视为目标地址。客户端希望到达 IP 地址为 10.31.20.04 的服务器，端口为 7471。对于此示例，该地址被硬编码到客户端中。更典型的是，该地址将通过命令行、配置文件或服务提供给客户端。通常端口号是众所周知的，客户端将通过名称找到服务器，DNS（域名服务）提供名称到地址的解析。幸运的是，getaddrinfo 调用可用于将主机名转换为 IP 地址。

无论是直接为客户端提供服务器的网络地址，还是必须将名称转换为网络地址，用于向客户端提供此信息的机制各不相同。一个常用的简单机制是用户使用命令行选项提供服务器地址。对于与数以亿计的对等进程进行通信的应用程序，告诉应用程序其对等点所在位置的问题显着增加，通常需要单独的专用应用程序来解决。对于典型的客户端-服务器套接字应用程序，这不是问题，因此我们将稍后再讨论。

使用 getaddrinfo() 结果，客户端打开一个套接字，将其配置为非阻塞模式，并启动连接请求。至此，网络栈已经向服务器发送了建立连接的请求。因为socket已经设置为非阻塞，**connect调用会立即返回**，不会等待连接建立。因此，此时任何发送数据的尝试都可能失败。

```
/* Example server code flow to accept a connection */
struct pollfd fds;
int server_fd;

fds.fd = listen_fd;
fds.events = POLLIN;

poll(&fds, -1);

server_fd = accept(listen_fd, NULL, 0);
fcntl(server_fd, F_SETFL, O_NONBLOCK);
```

Applications that use non-blocking sockets use select() or poll() to receive notification of when a socket is ready to send or receive data.  In this case, the server wishes to know when the listening socket has a connection request to process.  It adds the listening socket to a poll set, then waits until a connection request arrives (i.e. POLLIN is true).  The poll() call blocks until POLLIN is set on the socket.  POLLIN indicates that the socket has data to accept.  Since this is a listening socket, the data is a connection request.  The server accepts the request by calling accept().  That returns a new socket to the server, which is ready for data transfers.

The server sets the new socket to non-blocking mode.  Non-blocking support is particularly important to applications that manage communication with multiple peers.

使用非阻塞套接字的应用程序使用 select() 或 poll() 来接收有关套接字何时准备好发送或接收数据的通知。 在这种情况下，服务器希望知道侦听套接字何时有连接请求要处理。 它将侦听套接字添加到轮询集poll set，然后等待**连接请求到达**（即 POLLIN 为真）。 poll() 调用阻塞，直到在套接字上设置 POLLIN。 POLLIN 表示套接字有数据要接受。 由于这是一个监听套接字，因此数据是一个连接请求。 服务器通过调用accept() 接受请求。 **这会向服务器返回一个新的套接字**，该套接字已准备好进行数据传输。

服务器将新套接字设置为非阻塞模式。 非阻塞支持对于管理与多个对等点的通信的应用程序尤其重要。

```
/* Example client code flow to establish a connection */
struct pollfd fds;
int err;
socklen_t len;

fds.fd = client_fd;
fds.events = POLLOUT;

poll(&fds, -1);

len = sizeof err;
getsockopt(client_fd, SOL_SOCKET, SO_ERROR, &err, &len);
```

The client is notified that its connection request has completed when its connecting socket is 'ready to send data' (i.e. POLLOUT is true).  The poll() call blocks until POLLOUT is set on the socket, indicating the connection attempt is done.  Note that the connection request may have completed with an error, and the client still needs to check if the connection attempt was successful.  That is not conveyed to the application by the poll() call.  The getsockopt() call is used to retrieve the result of the connection attempt.  If err in this example is set to 0, then the connection attempt succeeded.  The socket is now ready to send and receive data.

After a connection has been established, the process of sending or receiving data is the same for both the client and server.  The examples below differ only by name of the socket variable used by the client or server application.

当其连接套接字“准备好发送数据”（即 POLLOUT 为真）时，客户端会收到其连接请求已完成的通知。 poll() 调用阻塞，直到在套接字上设置 POLLOUT，表示连接尝试已完成。 请注意，连接请求可能已经完成但有错误，客户端仍然需要检查连接尝试是否成功。 poll() 调用不会将其传达给应用程序。 getsockopt() 调用用于检索连接尝试的结果。 如果此示例中的 err 设置为 0，则连接尝试成功。 套接字现在已准备好发送和接收数据。

建立连接后，客户端和服务器发送或接收数据的过程是相同的。 下面的示例仅在客户端或服务器应用程序使用的套接字变量的名称上有所不同。

```
/* Example of client sending data to server */
struct pollfd fds;
size_t offset, size, ret;
char buf[4096];

fds.fd = client_fd;
fds.events = POLLOUT;

size = sizeof(buf);
for (offset = 0; offset < size; ) {
    poll(&fds, -1);
    
    ret = send(client_fd, buf + offset, size - offset, 0);
    offset += ret;
}
```

Network communication involves buffering of data at both the sending and receiving sides of the connection. TCP uses a credit based scheme to manage flow control to ensure that there is sufficient buffer space at the receive side of a connection to accept incoming data.  This flow control is hidden from the application by the socket API.  As a result, stream based sockets may not transfer all the data that the application requests to send as part of a single operation.

In this example, the client maintains an offset into the buffer that it wishes to send.  As data is accepted by the network, the offset increases.  The client then waits until the network is ready to accept more data before attempting another transfer.  The poll() operation supports this.  When the client socket is ready for data, it sets POLLOUT to true.  This indicates that send will transfer some additional amount of data.  The client issues a send() request for the remaining amount of buffer that it wishes to transfer.  If send() transfers less data than requested, the client updates the offset, waits for the network to become ready, then tries again.

网络通信涉及在连接的发送端和接收端缓冲数据。 TCP 使用基于信用(credit)的方案来管理流量控制，以确保连接的接收端有足够的缓冲区空间来接收传入数据。套接字 API 对应用程序隐藏了这种流控制。因此，基于流的套接字可能不会传输应用程序请求作为单个操作的一部分发送的所有数据。

在此示例中，客户端在它希望发送的缓冲区中维护一个偏移量。随着网络接受数据，偏移量增加。然后，客户端等待网络准备好接受更多数据，然后再尝试另一次传输。 poll() 操作支持这一点。当客户端套接字准备好接收数据时，它将 POLLOUT 设置为 true。这表明发送将传输一些额外的数据量。客户端针对它希望传输的剩余缓冲区量发出 send() 请求。如果 send() 传输的数据少于请求的数据，客户端会更新偏移量，等待网络准备好，然后重试。

```
/* Example of server receiving data from client */
struct pollfd fds;
size_t offset, size, ret;
char buf[4096];

fds.fd = server_fd;
fds.events = POLLIN;

size = sizeof(buf);
for (offset = 0; offset < size; ) {
    poll(&fds, -1);

    ret = recv(client_fd, buf + offset, size - offset, 0);
    offset += ret;
}
```

The flow for receiving data is similar to that used to send it.  Because of the streaming nature of the socket, there is no guarantee that the receiver will obtain all of the available data as part of a single call.  The server instead must wait until the socket is ready to receive data (POLLIN), before calling receive to obtain what data is available.  In this example, the server knows to expect exactly 4 KB of data from the client.

It is worth noting that the previous two  examples are written so that they are simple to understand.  They are poorly constructed when considering performance.  In both cases, the application always precedes a data transfer call (send or recv) with poll().  The impact is even if the network is ready to transfer data or has data queued for receiving, the application will always experience the latency and processing overhead of poll().  A better approach is to call send() or recv() prior to entering the for() loops, and only enter the loops if needed.

接收数据的流程与发送数据的流程相似。 由于套接字的流式传输特性，不能保证接收方将获得所有可用数据作为单个调用的一部分。 相反，服务器必须等到套接字准备好接收数据（POLLIN），然后再调用接收来获取可用的数据。 在此示例中，服务器知道从客户端准确地期望 4 KB 的数据。

值得注意的是，前面两个例子都是为了便于理解而编写的。 在考虑性能时，它们的构造很差。 在这两种情况下，应用程序总是在数据传输调用（send 或 recv）之前使用 poll()。 其影响是，即使网络已准备好传输数据或有数据排队等待接收，应用程序也将始终经历 poll() 的延迟和处理开销。 更好的方法是在进入 for() 循环之前调用 send() 或 recv()，并且仅在需要时才进入循环。

## Connection-less (UDP) Communication 无连接(连接更少)的UDP

TODO

## Advantages 优点

The socket API has two significant advantages.  First, it is available on a wide variety of operating systems and platforms, and works over the vast majority of available networking hardware.  It is easily the de facto networking API.  This by itself makes it appealing to use.

The second key advantage is that it is relatively easy to program to.  The importance of this should not be overlooked.  Networking APIs that offer access to higher performing features, but are difficult to program to correctly or well, often result in lower application performance.  This is not unlike coding an application in a higher-level language such as C or C++, versus assembly.  Although writing directly to assembly language offers the promise of being better performing, for the vast majority of developers, their applications will perform better if written in C or C++, and using an optimized compiler.  Applications should have a clear need for high-performance networking before selecting an alternative API to sockets.

套接字 API 有两个显着的优点。首先，它适用于各种操作系统和平台，并且适用于绝大多数可用的网络硬件。它很容易成为事实上的网络 API。这本身就使其具有吸引力。

第二个关键优势是相对容易编程。不应忽视这一点的重要性。提供对更高性能功能的访问但难以正确或良好编程的网络 API 通常会导致较低的应用程序性能。这与使用 C 或 C++ 等高级语言对应用程序进行编码与汇编没有什么不同。尽管直接用汇编语言编写提供了性能更好的承诺，但对于绝大多数开发人员来说，如果用 C 或 C++ 编写并使用优化的编译器，他们的应用程序将性能更好。在选择套接字的替代 API 之前，应用程序应该明确需要高性能网络。

## Disadvantages 不足

When considering the problems with the socket API, we limit our discussion to the two most common sockets types: streaming (TCP) and datagram (UDP).

Most applications require that network data be sent reliably.  This invariably means using a connection-oriented TCP socket.  TCP sockets transfer data as a stream of bytes.  However, many applications operate on messages.  The result is that applications often insert headers that are simply used to convert to/from a byte stream.  These headers consume additional network bandwidth and processing.  The streaming nature of the interface also results in the application using loops as shown in the examples above to send and receive larger messages.  The complexity of those loops can be significant if the application is managing sockets to hundreds or thousands of peers.

Another issue highlighted by the above examples deals with the asynchronous nature of network traffic.  When using a reliable transport, it is not enough to place an application's data onto the network.  If the network is busy, it could drop the packet, or the data could become corrupted during a transfer.  The data must be kept until it has been acknowledged by the peer, so that it can be resent if needed.  The socket API is defined such that the application owns the contents of its memory buffers after a socket call returns.

As an example, if we examine the socket send() call, once send() returns the application is free to modify its buffer.  The network implementation has a couple of options.  One option is for the send call to place the data directly onto the network.  The call must then block before returning to the user until the peer acknowledges that it received the data, at which point send() can then return.  The obvious problem with this approach is that the application is blocked in the send() call until the network stack at the peer can process the data and generate an acknowledgment.  This can be a significant amount of time where the application is blocked and unable to process other work, such as responding to messages from other clients.

A better option is for the send() call to copy the application's data into an internal buffer.  The data transfer is then issued out of that buffer, which allows retrying the operation in case of a failure.  The send() call in this case is not blocked, but all data that passes through the network will result in a memory copy to a local buffer, even in the absence of any errors.

Allowing immediate re-use of a data buffer helps keep the socket API simple.  However, such a feature can potentially have a negative impact on network performance.  For network or memory limited applications, an alternative API may be attractive.

Because the socket API is often considered in conjunction with TCP and UDP, that is, with protocols, it is intentionally detached from the underlying network hardware implementation, including NICs, switches, and routers.  Access to available network features is therefore constrained by what the API can support.

在考虑套接字 API 的问题时，我们将讨论限制在两种最常见的套接字类型：流 (TCP) 和数据报 (UDP)。

大多数应用程序要求可靠地发送网络数据。这总是意味着使用面向连接的 TCP 套接字。 TCP 套接字以字节流的形式传输数据。但是，许多应用程序对消息进行操作。结果是应用程序经常插入仅用于转换为/从字节流转换的标头。这些标头消耗额外的网络带宽和处理。接口的流式传输特性还导致应用程序使用如上例所示的循环来发送和接收更大的消息。如果应用程序正在管理数百或数千个对等点的套接字，那么这些循环的复杂性可能会很大。

上述示例强调的另一个问题涉及网络流量的异步性质。使用可靠传输时，仅将应用程序的数据放到网络上是不够的。如果网络繁忙，它可能会丢弃数据包，或者数据可能会在传输过程中损坏。数据必须保存到对方确认为止，以便在需要时可以重新发送。套接字 API 被定义为在套接字调用返回后应用程序拥有其内存缓冲区的内容。

例如，如果我们检查套接字 send() 调用，一旦 send() 返回，应用程序就可以自由修改其缓冲区。网络实现有几个选项。一种选择是发送调用将数据直接放到网络上。然后调用必须在返回给用户之前阻塞，直到对等方确认它接收到数据，此时 send() 可以返回。这种方法的明显问题是应用程序在 send() 调用中被阻塞，直到对端的网络堆栈可以处理数据并生成确认。这可能是应用程序被阻止并且无法处理其他工作（例如响应来自其他客户端的消息）的大量时间。

更好的选择是调用 send() 将应用程序的数据复制到内部缓冲区中。然后从该缓冲区发出数据传输，这允许在失败的情况下重试操作。在这种情况下，send() 调用不会被阻塞，但是所有通过网络的数据都会导致内存复制到本地缓冲区，即使没有任何错误。

允许立即重用数据缓冲区有助于保持套接字 API 简单。但是，这样的功能可能会对网络性能产生负面影响。对于网络或内存受限的应用程序，替代 API 可能很有吸引力。

由于套接字 API 经常被考虑与 TCP 和 UDP 结合，即与协议结合，因此有意将其与底层网络硬件实现分离，包括 NIC、交换机和路由器。因此，对可用网络功能的访问受限于 API 可以支持的内容。

# High-Performance Networking 高性能网络

By analyzing the socket API in the context of high-performance networking, we can start to see some features that are desirable for a network API.

通过在高性能网络环境中分析套接字 API，我们可以开始看到网络 API 所需要的一些特性。

## Avoiding Memory Copies 避免内存拷贝

The socket API implementation usually results in data copies occurring at both the sender and the receiver.  This is a trade-off between keeping the interface easy to use, versus providing reliability.  Ideally, all memory copies would be avoided when transferring data over the network.  There are techniques and APIs that can be used to avoid memory copies, but in practice, the cost of avoiding a copy can often be more than the copy itself, in particular for small transfers (measured in bytes, versus kilobytes or more).

To avoid a memory copy at the sender, we need to place the application data directly onto the network.  If we also want to avoid blocking the sending application, we need some way for the network layer to communicate with the application when the buffer is safe to re-use.  This would allow the buffer to be re-used in case the data needs to be re-transmitted.  This leads us to crafting a network interface that behaves asynchronously.  The application will need to issue a request, then receive some sort of notification when the request has completed.

Avoiding a memory copy at the receiver is more challenging.  When data arrives from the network, it needs to land into an available memory buffer, or it will be dropped, resulting in the sender re-transmitting the data.  If we use socket recv() semantics, the only way to avoid a copy at the receiver is for the recv() to be called before the send().  Recv() would then need to block until the data has arrived.  Not only does this block the receiver, it is impractical to use outside of an application with a simple request-reply protocol.

Instead, what is needed is a way for the receiving application to provide one or more buffers to the network for received data to land.  The network then needs to notify the application when data is available.  This sort of mechanism works well if the receiver does not care where in its memory space the data is located; it only needs to be able to process the incoming message.

As an alternative, it is possible to reverse this flow, and have the network layer hand its buffer to the application.  The application would then be responsible for returning the buffer to the network layer when it is done with its processing.  While this approach can avoid memory copies, it suffers from a few drawbacks.  First, the network layer does not know what size of messages to expect, which can lead to inefficient memory use.  Second, many would consider this a more difficult programming model to use.  And finally, the network buffers would need to be mapped into the application process' memory space, which negatively impacts performance.

In addition to processing messages, some applications want to receive data and store it in a specific location in memory.  For example, a database may want to merge received data records into an existing table.  In such cases, even if data arriving from the network goes directly into an application's receive buffers, it may still need to be copied into its final location.  It would be ideal if the network supported placing data that arrives from the network into a specific memory buffer, with the buffer determined based on the contents of the data.

套接字 API 实现通常会导致发送方和接收方都发生数据副本。这是保持界面易于使用与提供可靠性之间的权衡。理想情况下，通过网络传输数据时会避免所有内存副本。有一些技术和 API 可用于避免内存复制，但在实践中，避免复制的成本通常可能超过复制本身，特别是对于小传输（以字节为单位，而不是千字节或更多）。

为了避免发送方的内存复制，我们需要将应用程序数据直接放到网络上。如果我们还想避免阻塞发送应用程序，我们需要一些方法让网络层在缓冲区可以安全重用时与应用程序通信。这将允许在需要重新传输数据的情况下重新使用缓冲区。这导致我们设计了一个异步行为的网络接口。应用程序需要发出请求，然后在请求完成时收到某种通知。

在接收器处避免内存复制更具挑战性。当数据从网络到达时，需要降落到可用的内存缓冲区中，否则会被丢弃，导致发送方重新传输数据。如果我们使用套接字 recv() 语义，避免在接收方复制的唯一方法是在 send() 之前调用 recv()。然后，Recv() 将需要阻塞，直到数据到达。这不仅会阻塞接收器，而且在应用程序之外使用简单的请求-回复协议也是不切实际的。

相反，需要一种方法让接收应用程序向网络提供一个或多个缓冲区，以便接收到的数据到达。然后，网络需要在数据可用时通知应用程序。如果接收方不关心数据在其内存空间中的位置，这种机制就可以很好地工作。它只需要能够处理传入的消息。

作为替代方案，可以反转此流程，让网络层将其缓冲区交给应用程序。然后，应用程序将负责在完成处理后将缓冲区返回给网络层。虽然这种方法可以避免内存复制，但它也有一些缺点。首先，网络层不知道预期的消息大小，这会导致内存使用效率低下。其次，许多人会认为这是一个更难使用的编程模型。最后，网络缓冲区需要映射到应用程序进程的内存空间，这会对性能产生负面影响。

除了处理消息之外，一些应用程序还希望接收数据并将其存储在内存中的特定位置。例如，数据库可能希望将接收到的数据记录合并到现有表中。在这种情况下，即使来自网络的数据直接进入应用程序的接收缓冲区，也可能仍需要将其复制到其最终位置。如果网络支持将来自网络的数据放置到特定的内存缓冲区中，那将是理想的，缓冲区根据数据的内容确定。

### Network Buffers

Based on the problems described above, we can start to see that avoiding memory copies depends upon the ownership of the memory buffers used for network traffic.  With socket based transports, the network buffers are owned and managed by the networking stack.  This is usually handled by the operating system kernel.  However, this results in the data 'bouncing' between the application buffers and the network buffers.  By putting the application in control of managing the network buffers, we can avoid this overhead.  The cost for doing so is additional complexity in the application.

Note that even though we want the application to own the network buffers, we would still like to avoid the situation where the application implements a complex network protocol.  The trade-off is that the app provides the data buffers to the network stack, but the network stack continues to handle things like flow control, reliability, and segmentation and reassembly.

基于上述问题，我们可以开始看到避免内存复制取决于用于网络流量的内存缓冲区的所有权。 使用基于套接字的传输，网络缓冲区由网络堆栈拥有和管理。 这通常由操作系统内核处理。 但是，这会导致数据在应用程序缓冲区和网络缓冲区之间“反弹”。 通过让应用程序控制管理网络缓冲区，我们可以避免这种开销。 这样做的代价是应用程序的额外复杂性。

请注意，即使我们希望应用程序拥有网络缓冲区，我们仍然希望避免应用程序实现复杂网络协议的情况。 权衡是应用程序向网络堆栈提供数据缓冲区，但网络堆栈继续处理流量控制、可靠性以及分段和重组等事情。

### Resource Management

We define resource management to mean properly allocating network resources in order to avoid overrunning data buffers or queues.  Flow control is a common aspect of resource management.  Without proper flow control, a sender can overrun a slow or busy receiver.  This can result in dropped packets, re-transmissions, and increased network congestion.  Significant research and development has gone into implementing flow control algorithms.  Because of its complexity, it is not something that an application developer should need to deal with.  That said, there are some applications where flow control simply falls out of the network protocol.  For example, a request-reply protocol naturally has flow control built in.

For our purposes, we expand the definition of resource management beyond flow control.  Flow control typically only deals with available network buffering at a peer.  We also want to be concerned about having available space in outbound data transfer queues.  That is, as we issue commands to the local NIC to send data, that those commands can be queued at the NIC.  When we consider reliability, this means tracking outstanding requests until they have been acknowledged.  Resource management will need to ensure that we do not overflow that request queue.

Additionally, supporting asynchronous operations (described in detail below) will introduce potential new queues.  Those queues must not overflow as well.

我们将资源管理定义为正确分配网络资源以避免超出数据缓冲区或队列。流控制是资源管理的一个常见方面。如果没有适当的流量控制，发送方可能会超出慢速或繁忙的接收方。这可能导致丢包、重传和网络拥塞增加。大量的研究和开发已经进入实施流控制算法。由于它的复杂性，它不是应用程序开发人员应该需要处理的事情。也就是说，在某些应用程序中，流量控制完全脱离了网络协议。例如，请求-回复协议自然具有内置的流量控制。

出于我们的目的，我们将资源管理的定义扩展到流控制之外。流控制通常只处理对等点的可用网络缓冲。我们还希望关注出站数据传输队列中的可用空间。也就是说，当我们向本地 NIC 发出命令以发送数据时，这些命令可以在 NIC 处排队。当我们考虑可靠性时，这意味着跟踪未完成的请求，直到它们被确认。资源管理需要确保我们不会溢出该请求队列。

此外，支持异步操作（下面详细描述）将引入潜在的新队列。这些队列也不能溢出。

## Asynchronous Operations 异步操作

Arguably, the key feature of achieving high-performance is supporting asynchronous operations.  The socket API supports asynchronous transfers with its non-blocking mode.  However, because the API itself operates synchronously, the result is additional data copies.  For an API to be asynchronous, an application needs to be able to submit work, then later receive some sort of notification that the work is done.  In order to avoid extra memory copies, the application must agree not to modify its data buffers until the operation completes.

There are two main ways to notify an application that it is safe to re-use its data buffers.  One mechanism is for the network layer to invoke some sort of callback or send a signal to the application that the request is done.  Some asynchronous APIs use this mechanism.  The drawback of this approach is that signals interrupt an application's processing.  This can negatively impact the CPU caches, plus requires interrupt processing.  Additionally, it is often difficult to develop an application that can handle processing a signal that can occur at anytime.

An alternative mechanism for supporting asynchronous operations is to write events into some sort of completion queue when an operation completes.  This provides a way to indicate to an application when a data transfer has completed, plus gives the application control over when and how to process completed requests.  For example, it can process requests in batches to improve code locality and performance.

可以说，实现高性能的关键特性是支持异步操作。套接字 API 以非阻塞模式支持异步传输。但是，由于 API 本身是同步操作的，因此结果是额外的数据副本。对于异步的 API，应用程序需要能够提交工作，然后接收某种通知，表明工作已完成。为了避免额外的内存副本，应用程序必须同意在操作完成之前不修改其数据缓冲区。

有两种主要方法可以通知应用程序可以安全地重用其数据缓冲区。一种机制是网络层调用某种回调或向应用程序发送请求完成的信号。一些异步 API 使用这种机制。这种方法的缺点是信号会中断应用程序的处理。这会对 CPU 缓存产生负面影响，并且需要中断处理。此外，开发一个可以处理随时可能发生的信号的应用程序通常很困难。

支持异步操作的另一种机制是在操作完成时将事件写入某种完成队列。这提供了一种在数据传输完成时向应用程序指示的方法，并让应用程序控制何时以及如何处理已完成的请求。例如，它可以批量处理请求以提高代码局部性和性能。

### Interrupts and Signals 中断和轮训

Interrupts are a natural extension to supporting asynchronous operations.  However, when dealing with an asynchronous API, they can negatively impact performance.  Interrupts, even when directed to a kernel agent, can interfere with application processing.

If an application has an asynchronous interface with completed operations written into a completion queue, it is often sufficient for the application to simply check the queue for events.  As long as the application has other work to perform, there is no need for it to block.  This alleviates the need for interrupt generation.  A NIC merely needs to write an entry into the completion queue and update a tail pointer to signal that a request is done.

If we follow this argument, then it can be beneficial to give the application control over when interrupts should occur and when to write events to some sort of wait object.  By having the application notify the network layer that it will wait until a completion occurs, we can better manage the number and type of interrupts that are generated.

中断是支持异步操作的自然扩展。但是，在处理异步 API 时，它们会对性能产生负面影响。中断，即使指向内核代理，也会干扰应用程序处理。

如果应用程序有一个将已完成操作写入完成队列的异步接口，则应用程序通常只需检查队列中的事件就足够了。只要应用程序有其他工作要执行，就没有必要阻塞。这减轻了对中断生成的需求。 NIC 只需将一个条目写入完成队列并更新尾指针以表示请求已完成。

如果我们遵循这个论点，那么让应用程序控制何时应该发生中断以及何时将事件写入某种等待对象可能是有益的。通过让应用程序通知网络层它将等到完成发生，我们可以更好地管理生成的中断的数量和类型。

### Event Queues 事件队列

As outlined above, there are performance advantages to having an API that reports completions or provides other types of notification using an event queue.  A very simple type of event queue merely tracks completed operations.  As data is received or a send completes, an entry is written into the event queue.

如上所述，使用事件队列报告完成或提供其他类型通知的 API 具有性能优势。 一种非常简单的事件队列仅跟踪已完成的操作。 当数据被接收或发送完成时，一个条目被写入事件队列。

## Direct Hardware Access 直接内存访问

When discussing the network layer, most software implementations refer to kernel modules responsible for implementing the necessary transport and network protocols.  However, if we want network latency to approach sub-microsecond speeds, then we need to remove as much software between the application and its access to the hardware as possible.  One way to do this is for the application to have direct access to the network interface controller's command queues.  Similarly, the NIC requires direct access to the application's data buffers and control structures, such as the above mentioned completion queues.

Note that when we speak about an application having direct access to network hardware, we're referring to the application process.  Naturally, an application developer is highly unlikely to code for a specific hardware NIC.  That work would be left to some sort of network library specifically targeting the NIC.  The actual network layer, which implements the network transport, could be part of the network library or offloaded onto the NIC's hardware or firmware.

在讨论网络层时，大多数软件实现都是指负责实现必要的传输和网络协议的内核模块。但是，如果我们希望网络延迟接近亚微秒级的速度，那么我们需要在应用程序与其对硬件的访问之间尽可能多地删除软件。一种方法是让应用程序直接访问网络接口控制器的命令队列。同样，NIC 需要直接访问应用程序的数据缓冲区和控制结构，例如上面提到的完成队列。

请注意，当我们谈到应用程序可以直接访问网络硬件时，我们指的是应用程序进程。自然，应用程序开发人员极不可能为特定的硬件 NIC 编写代码。这项工作将留给某种专门针对 NIC 的网络库。实现网络传输的实际网络层可以是网络库的一部分，也可以卸载到 NIC 的硬件或固件上。

### Kernel Bypass

Kernel bypass is a feature that allows the application to avoid calling into the kernel for data transfer operations.  This is possible when it has direct access to the NIC hardware.  Complete kernel bypass is impractical because of security concerns and resource management constraints.  However, it is possible to avoid kernel calls for what are called 'fast-path' operations, such as send or receive.

For security and stability reasons, operating system kernels cannot rely on data that comes from user space applications.  As a result, even a simple kernel call often requires acquiring and releasing locks, coupled with data verification checks.  If we can limit the effects of a poorly written or malicious application to its own process space, we can avoid the overhead that comes with kernel validation without impacting system stability.

内核绕过是一项允许应用程序避免调用内核进行数据传输操作的功能。 当它可以直接访问 NIC 硬件时，这是可能的。 由于安全问题和资源管理限制，完全绕过内核是不切实际的。 但是，可以避免内核调用所谓的“快速路径”操作，例如发送或接收。

出于安全和稳定性的原因，操作系统内核不能依赖来自用户空间应用程序的数据。 因此，即使是简单的内核调用也经常需要获取和释放锁，再加上数据验证检查。 如果我们可以将编写不佳或恶意应用程序的影响限制在它自己的进程空间中，我们就可以避免内核验证带来的开销，而不会影响系统稳定性。

### Direct Data Placement 直接内存放置

Direct data placement means avoiding data copies when sending and receiving data, plus placing received data into the correct memory buffer where needed.  On a broader scale, it is part of having direct hardware access, with the application and NIC communicating directly with shared memory buffers and queues.

Direct data placement is often thought of by those familiar with RDMA - remote direct memory access.  RDMA is a technique that allows reading and writing memory that belongs to a peer process that is running on a node across the network.  Advanced RDMA hardware is capable of accessing the target memory buffers without involving the execution of the peer process.  RDMA relies on offloading the network transport onto the NIC in order to avoid interrupting the target process.

The main advantages of supporting direct data placement is avoiding memory copies and minimizing processing overhead.

直接数据放置意味着在发送和接收数据时避免数据复制，并在需要时将接收到的数据放入正确的内存缓冲区。 在更广泛的范围内，它是直接硬件访问的一部分，应用程序和 NIC 直接与共享内存缓冲区和队列通信。

熟悉 RDMA（远程直接内存访问）的人通常会想到直接数据放置。 RDMA 是一种允许读写属于在网络上的节点上运行的对等进程的内存的技术。 高级 RDMA 硬件能够访问目标内存缓冲区，而不涉及对等进程的执行。 RDMA 依赖于将网络传输卸载到 NIC 上以避免中断目标进程。

支持直接数据放置的主要优点是避免内存复制和最小化处理开销。

# Designing Interfaces for Performance

We want to design a network interface that can meet the requirements outlined above.  Moreover, we also want to take into account the performance of the interface itself.  It is often not obvious how an interface can adversely affect performance, versus performance being a result of the underlying implementation.  The following sections describe how interface choices can impact performance.  Of course, when we begin defining the actual APIs that an application will use, we will need to trade off raw performance for ease of use where it makes sense.

When considering performance goals for an API, we need to take into account the target application use cases.  For the purposes of this discussion, we want to consider applications that communicate with thousands to millions of peer processes.  Data transfers will include millions of small messages per second per peer, and large transfers that may be up to gigabytes of data.  At such extreme scales, even small optimizations are measurable, in terms of both performance and power.  If we have a million peers sending a millions messages per second, eliminating even a single instruction from the code path quickly multiplies to saving billions of instructions per second from the overall execution, when viewing the operation of the entire application.

We once again refer to the socket API as part of this discussion in order to illustrate how an API can affect performance.

我们想设计一个能够满足上述要求的网络接口。而且，我们还要考虑到接口本身的性能。接口如何对性能产生不利影响通常并不明显，而性能是底层实现的结果。以下部分描述了接口选择如何影响性能。当然，当我们开始定义应用程序将使用的实际 API 时，我们需要在合理的情况下权衡原始性能以换取易用性。

在考虑 API 的性能目标时，我们需要考虑目标应用程序用例。出于本次讨论的目的，我们希望考虑与成千上万个对等进程进行通信的应用程序。数据传输将包括每个对等方每秒数百万条小消息，以及可能高达千兆字节数据的大传输。在如此极端的规模下，就性能和功率而言，即使是很小的优化也是可以衡量的。如果我们有一百万对等方每秒发送数百万条消息，那么在查看整个应用程序的操作时，即使从代码路径中消除一条指令也会迅速成倍地从整体执行中节省数十亿条指令。

我们再次将套接字 API 作为讨论的一部分，以说明 API 如何影响性能。

```
/* Notable socket function prototypes */
/* "control" functions */
int socket(int domain, int type, int protocol);
int bind(int socket, const struct sockaddr *addr, socklen_t addrlen);
int listen(int socket, int backlog);
int accept(int socket, struct sockaddr *addr, socklen_t *addrlen);
int connect(int socket, const struct sockaddr *addr, socklen_t addrlen);
int shutdown(int socket, int how);
int close(int socket); 

/* "fast path" data operations - send only (receive calls not shown) */
ssize_t send(int socket, const void *buf, size_t len, int flags);
ssize_t sendto(int socket, const void *buf, size_t len, int flags,
    const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t sendmsg(int socket, const struct msghdr *msg, int flags);
ssize_t write(int socket, const void *buf, size_t count);
ssize_t writev(int socket, const struct iovec *iov, int iovcnt);

Send 和 Write 之间的主要区别在于，套接字编程中的两个函数在它们都存在多个标志方面有所不同。 众所周知，套接字编程中的函数 Send 仅在作为套接字描述符的更专业的函数上起作用。 而众所周知，Write 在这个问题上是通用的，因为它可以处理所有类型的描述符。

/* "indirect" data operations */
int poll(struct pollfd *fds, nfds_t nfds, int timeout);
int select(int nfds, fd_set *readfds, fd_set *writefds,
    fd_set *exceptfds, struct timeval *timeout); 
```

Examining this list, there are a couple of features to note.  First, there are multiple calls that can be used to send data, as well as multiple calls that can be used to wait for a non-blocking socket to become ready.  This will be discussed in more detail further on.  Second, the operations have been split into different groups (terminology is ours).  Control operations are those functions that an application seldom invokes during execution.  They often only occur as part of initialization.

Data operations, on the other hand, may be called hundreds to millions of times during an application's lifetime.  They deal directly or indirectly with transferring or receiving data over the network.  Data operations can be split into two groups.  Fast path calls interact with the network stack to immediately send or receive data.  In order to achieve high bandwidth and low latency, those operations need to be as fast as possible.  Non-fast path operations that still deal with data transfers are those calls, that while still frequently called by the application, are not as performance critical.  For example, the select() and poll() calls are used to block an application thread until a socket becomes ready.  Because those calls suspend the thread execution, performance is a lesser concern.  (Performance of those operations is still of a concern, but the cost of executing the operating system scheduler often swamps any but the most substantial performance gains.)

检查此列表，有几个功能需要注意。首先，有多个调用可用于发送数据，以及多个调用可用于等待非阻塞套接字就绪。这将在后面更详细地讨论。其次，操作被分成不同的组（术语是我们的）。控制操作是应用程序在执行期间很少调用的那些功能。它们通常仅作为初始化的一部分出现。

另一方面，数据操作在应用程序的生命周期中可能会被调用数百到数百万次。它们直接或间接地处理通过网络传输或接收数据。数据操作可以分为两组。快速路径调用与网络堆栈交互以立即发送或接收数据。为了实现高带宽和低延迟，这些操作需要尽可能快。仍然处理数据传输的非快速路径操作是那些调用，虽然仍然经常被应用程序调用，但对性能的要求并不高。例如，select() 和 poll() 调用用于阻塞应用程序线程，直到套接字准备好。因为这些调用会暂停线程执行，所以性能是一个不太关心的问题。 （这些操作的性能仍然是一个问题，但是执行操作系统调度程序的成本通常会超过除了最实质性的性能提升之外的任何东西。）

## Call Setup Costs

The amount of work that an application needs to perform before issuing a data transfer operation can affect performance, especially message rates.  Obviously, the more parameters an application must push on the stack to call a function increases its instruction count.  However, replacing stack variables with a single data structure does not help to reduce the setup costs.

Suppose that an application wishes to send a single data buffer of a given size to a peer.  If we examine the socket API, the best fit for such an operation is the write() call.  That call takes only those values which are necessary to perform the data transfer.  The send() call is a close second, and send() is a more natural function name for network communication, but send() requires one extra argument over write().  Other functions are even worse in terms of setup costs.  The sendmsg() function, for example, requires that the application format a data structure, the address of which is passed into the call.  This requires significantly more instructions from the application if done for every data transfer.

Even though all other send functions can be replaced by sendmsg(), it is useful to have multiple ways for the application to issue send requests.  Not only are the other calls easier to read and use (which lower software maintenance costs), but they can also improve performance.

应用程序在发出数据传输操作之前需要执行的工作量会影响性能，尤其是消息速率。显然，应用程序必须压入堆栈以调用函数的参数越多，它的指令数就会增加。但是，用单个数据结构替换堆栈变量并不能帮助降低设置成本。

假设应用程序希望将给定大小的单个数据缓冲区发送到对等点。如果我们检查套接字 API，最适合这种操作的是 write() 调用。该调用仅采用执行数据传输所需的那些值。 send() 调用紧随其后，send() 是用于网络通信的更自然的函数名称，但 send() 需要一个比 write() 额外的参数。就设置成本而言，其他功能甚至更糟。例如，sendmsg() 函数要求应用程序格式化一个数据结构，并将其地址传递给调用。如果每次数据传输都完成，这需要来自应用程序的更多指令。

尽管所有其他发送函数都可以用 sendmsg() 代替，但有多种方式让应用程序发出发送请求还是很有用的。其他调用不仅更易于阅读和使用（这降低了软件维护成本），而且还可以提高性能。

## Branches and Loops

When designing an API, developers rarely consider how the API impacts the underlying implementation.  However, the selection of API parameters can require that the underlying implementation add branches or use control loops.  Consider the difference between the write() and writev() calls.  The latter passes in an array of I/O vectors, which may be processed using a loop such as this:

在设计 API 时，开发人员很少考虑 API 如何影响底层实现。 但是，API 参数的选择可能需要底层实现添加分支或使用控制循环。 考虑 write() 和 writev() 调用之间的区别。 后者传入一个 I/O 向量数组，可以使用如下循环进行处理：

```
/* Sample implementation for processing an array */
for (i = 0; i < iovcnt; i++) {
    ...
}
```

In order to process the iovec array, the natural software construct would be to use a loop to iterate over the entries.  Loops result in additional processing.  Typically, a loop requires initializing a loop control variable (e.g. i = 0), adds ALU operations (e.g. i++), and a comparison (e.g. i < iovcnt).  This overhead is necessary to handle an arbitrary number of iovec entries.  If the common case is that the application wants to send a single data buffer, write() is a better option.

In addition to control loops, an API can result in the implementation needing branches.  Branches can change the execution flow of a program, impacting processor pipe-lining techniques.  Processor branch prediction helps alleviate this issue.  However, while branch prediction can be correct nearly 100% of the time while running a micro-benchmark, such as a network bandwidth or latency test, with more realistic network traffic, the impact can become measurable.

We can easily see how an API can introduce branches into the code flow if we examine the send() call.  Send() takes an extra flags parameter over the write() call.  This allows the application to modify the behavior of send().  From the viewpoint of implementing send(), the flags parameter must be checked.  In the best case, this adds one additional check (flags are non-zero).  In the worst case, every valid flag may need a separate check, resulting in potentially dozens of checks.

Overall, the sockets API is well designed considering these performance implications.  It provides complex calls where they are needed, with simpler functions available that can avoid some of the overhead inherent in other calls.

为了处理 iovec 数组，自然的软件构造将使用循环来迭代条目。循环导致额外的处理。通常，循环需要初始化循环控制变量（例如 i = 0）、添加 ALU 操作（例如 i++）和比较（例如 i < iovcnt）。此开销对于处理任意数量的 iovec 条目是必需的。如果常见情况是应用程序想要发送单个数据缓冲区，那么 write() 是一个更好的选择。

除了控制循环之外，API 还可能导致实现需要分支。分支可以改变程序的执行流程，影响处理器流水线技术。处理器分支预测有助于缓解这个问题。然而，虽然在运行微基准测试（例如网络带宽或延迟测试）时，分支预测几乎 100% 的时间都是正确的，但网络流量更真实，其影响可以变得可衡量。

如果我们检查 send() 调用，我们可以很容易地看到 API 如何将分支引入代码流。 Send() 在 write() 调用上采用额外的标志参数。这允许应用程序修改 send() 的行为。从实现 send() 的角度来看，必须检查 flags 参数。在最好的情况下，这会增加一项额外的检查（标志非零）。在最坏的情况下，每个有效标志都可能需要单独检查，从而可能导致数十次检查。

总体而言，考虑到这些性能影响，套接字 API 设计得很好。它在需要的地方提供复杂的调用，并提供更简单的功能，可以避免其他调用中固有的一些开销。

## Command Formatting

The ultimate objective of invoking a network function is to transfer or receive data from the network.  In this section, we're dropping to the very bottom of the software stack to the component responsible for directly accessing the hardware.  This is usually referred to as the network driver, and its implementation is often tied to a specific piece of hardware, or a series of NICs by a single hardware vendor.

In order to signal a NIC that it should read a memory buffer and copy that data onto the network, the software driver usually needs to write some sort of command to the NIC.  To limit hardware complexity and cost, a NIC may only support a couple of command formats.  This differs from the software interfaces that we've been discussing, where we can have different APIs of varying complexity in order to reduce overhead.  There can be significant costs associated with formatting the command and posting it to the hardware.

With a standard NIC, the command is formatted by a kernel driver.  That driver sits at the bottom of the network stack servicing requests from multiple applications.  It must typically format each command only after a request has passed through the network stack.

With devices that are directly accessible by a single application, there are opportunities to use pre-formatted command structures.  The more of the command that can be initialized prior to the application submitting a network request, the more streamlined the process, and the better the performance.

As an example, a NIC needs to have the destination address as part of a send operation.  If an application is sending to a single peer, that information can be cached and be part of a pre-formatted network header.  This is only possible if the NIC driver knows that the destination will not change between sends.  The closer that the driver can be to the application, the greater the chance for optimization.  An optimal approach is for the driver to be part of a library that executes entirely within the application process space.

调用网络函数的最终目标是从网络传输或接收数据。在本节中，我们将下降到软件堆栈的最底层，即负责直接访问硬件的组件。这通常称为网络驱动程序，其实现通常与特定硬件或单个硬件供应商的一系列 NIC 相关联。

为了向 NIC 发出信号，它应该读取内存缓冲区并将该数据复制到网络上，软件驱动程序通常需要向 NIC 写入某种命令。为了限制硬件复杂性和成本，一个 NIC 可能只支持几种命令格式。这与我们一直在讨论的软件接口不同，我们可以拥有不同复杂度的不同 API 以减少开销。格式化命令并将其发布到硬件可能会产生大量成本。

对于标准 NIC，命令由内核驱动程序格式化。该驱动程序位于网络堆栈的底部，为来自多个应用程序的请求提供服务。它通常必须仅在请求通过网络堆栈后才格式化每个命令。

对于可由单个应用程序直接访问的设备，有机会使用预先格式化的命令结构。在应用程序提交网络请求之前可以初始化的命令越多，流程就越精简，性能也越好。

例如，NIC 需要将目标地址作为发送操作的一部分。如果应用程序正在发送到单个对等点，则该信息可以被缓存并成为预先格式化的网络标头的一部分。这只有在 NIC 驱动程序知道目标在发送之间不会改变的情况下才有可能。驱动程序离应用程序越近，优化的机会就越大。一种最佳方法是让驱动程序成为完全在应用程序进程空间内执行的库的一部分。

## Memory Footprint

Memory footprint concerns are most notable among high-performance computing (HPC) applications that communicate with thousands of peers.  Excessive memory consumption impacts application scalability, limiting the number of peers that can operate in parallel to solve problems.  There is often a trade-off between minimizing the memory footprint needed for network communication, application performance, and ease of use of the network interface.

As we discussed with the socket API semantics, part of the ease of using sockets comes from the network layering copying the user's buffer into an internal buffer belonging to the network stack.  The amount of internal buffering that's made available to the application directly correlates with the bandwidth that an application can achieve.  In general, larger internal buffering increases network performance, with a cost of increasing the memory footprint consumed by the application.  This memory footprint exists independent of the amount of memory allocated directly by the application.  Eliminating network buffering not only helps with performance, but also scalability, by reducing the memory footprint needed to support the application.

While network memory buffering increases as an application scales, it can often be configured to a fixed size.  The amount of buffering needed is dependent on the number of active communication streams being used at any one time.  That number is often significantly lower than the total number of peers that an application may need to communicate with.  The amount of memory required to _address_ the peers, however, usually has a linear relationship with the total number of peers.

With the socket API, each peer is identified using a struct sockaddr.  If we consider a UDP based socket application using IPv4 addresses, a peer is identified by the following address.

内存占用问题在与数千个对等点进行通信的高性能计算 (HPC) 应用程序中最为显着。过多的内存消耗会影响应用程序的可扩展性，从而限制可以并行运行以解决问题的对等点的数量。通常在最小化网络通信所需的内存占用、应用程序性能和网络接口的易用性之间进行权衡。

正如我们对套接字 API 语义所讨论的，使用套接字的部分易用性来自网络分层，将用户的缓冲区复制到属于网络堆栈的内部缓冲区。应用程序可用的内部缓冲量与应用程序可以实现的带宽直接相关。通常，较大的内部缓冲会提高网络性能，但代价是会增加应用程序消耗的内存占用。这种内存占用独立于应用程序直接分配的内存量。通过减少支持应用程序所需的内存占用，消除网络缓冲不仅有助于提高性能，还有助于提高可扩展性。

虽然网络内存缓冲随着应用程序的扩展而增加，但通常可以将其配置为固定大小。所需的缓冲量取决于任何时候使用的活动通信流的数量。该数字通常远低于应用程序可能需要与之通信的对等点的总数。然而，_寻址_对等点所需的内存量通常与对等点的总数呈线性关系。

使用套接字 API，每个对等点都使用 struct sockaddr 进行标识。如果我们考虑使用 IPv4 地址的基于 UDP 的套接字应用程序，则对等点由以下地址标识。

```
/* IPv4 socket address - with typedefs removed */
struct sockaddr_in {
    uint16_t sin_family; /* AF_INET */
    uint16_t sin_port;
    struct {
        uint32_t sin_addr;
    } in_addr;
};
```

In total, the application requires 8-bytes of addressing for each peer.  If the app communicates with a million peers, that explodes to roughly 8 MB of memory space that is consumed just to maintain the address list.  If IPv6 addressing is needed, then the requirement increases by a factor of 4.

Luckily, there are some tricks that can be used to help reduce the addressing memory footprint, though doing so will introduce more instructions into code path to access the network stack.  For instance, we can notice that all addresses in the above example have the same sin_family value (AF_INET).  There's no need to store that for each address.  This potentially shrinks each address from 8 bytes to 6.  (We may be left with unaligned data, but that's a trade-off to reducing the memory consumption).  Depending on how the addresses are assigned, further reduction may be possible.  For example, if the application uses the same set of port addresses at each node, then we can eliminate storing the port, and instead calculate it from some base value.  This type of trick can be applied to the IP portion of the address if the app is lucky enough to run across sequential IP addresses.

The main issue with this sort of address reduction is that it is difficult to achieve.  It requires that each application check for and handle address compression, exposing the application to the addressing format used by the networking stack.  It should be kept in mind that TCP/IP and UDP/IP addresses are logical addresses, not physical.  When running over Ethernet, the addresses that appear at the link layer are MAC addresses, not IP addresses.  The IP to MAC address association is managed by the network software.  We would like to provide addressing that is simple for an application to use, but at the same time can provide a minimal memory footprint.

总的来说，该应用程序需要为每个对等方提供 8 字节的寻址。如果应用程序与一百万个对等点进行通信，那么这会爆发到大约 8 MB 的内存空间，仅用于维护地址列表。如果需要 IPv6 寻址，则要求增加 4 倍。

幸运的是，有一些技巧可以用来帮助减少寻址内存占用，尽管这样做会在代码路径中引入更多指令来访问网络堆栈。例如，我们可以注意到上例中的所有地址都具有相同的 sin_family 值 (AF_INET)。无需为每个地址存储它。这可能会将每个地址从 8 个字节缩小到 6 个。（我们可能会留下未对齐的数据，但这是减少内存消耗的权衡）。根据地址的分配方式，可能会进一步减少。例如，如果应用程序在每个节点使用相同的端口地址集，那么我们可以消除存储端口，而是从某个基值计算它。如果应用程序足够幸运地跨连续 IP 地址运行，则可以将这种类型的技巧应用于地址的 IP 部分。

这种地址减少的主要问题是难以实现。它要求每个应用程序检查并处理地址压缩，将应用程序暴露给网络堆栈使用的寻址格式。应该记住，TCP/IP 和 UDP/IP 地址是逻辑地址，而不是物理地址。在以太网上运行时，出现在链路层的地址是 MAC 地址，而不是 IP 地址。 IP 到 MAC 地址的关联由网络软件管理。我们希望为应用程序提供简单易用的寻址，但同时可以提供最小的内存占用。

## Communication Resources

We need to take a brief detour in the discussion in order to delve deeper into the network problem and solution space.  Instead of continuing to think of a socket as a single entity, with both send and receive capabilities, we want to consider its components separately. A network socket can be viewed as three basic constructs: a transport level address, a send or transmit queue, and a receive queue.  Because our discussion will begin to pivot away from pure socket semantics, we will refer to our network 'socket' as an endpoint.

In order to reduce an application's memory footprint, we need to consider features that fall outside of the socket API.  So far, much of the discussion has been around sending data to a peer.  We now want to focus on the best mechanisms for receiving data.

With sockets, when an app has data to receive (indicated, for example, by a POLLIN event), we call recv().  The network stack copies the receive data into its buffer and returns.  If we want to avoid the data copy on the receive side, we need a way for the application to post its buffers to the network stack _before_ data arrives.

Arguably, a natural way of extending the socket API to support this feature is to have each call to recv() simply post the buffer to the network layer.  As data is received, the receive buffers are removed in the order that they were posted.  Data is copied into the posted buffer and returned to the user.  It would be noted that the size of the posted receive buffer may be larger (or smaller) than the amount of data received.  If the available buffer space is larger, hypothetically, the network layer could wait a short amount of time to see if more data arrives.  If nothing more arrives, the receive completes with the buffer returned to the application.

This raises an issue regarding how to handle buffering on the receive side.  So far, with sockets we've mostly considered a streaming protocol.  However, many applications deal with messages which end up being layered over the data stream.  If they send an 8 KB message, they want the receiver to receive an 8 KB message.  Message boundaries need to be maintained.

If an application sends and receives a fixed sized message, buffer allocation becomes trivial.  The app can post X number of buffers each of an optimal size.  However, if there is a wide mix in message sizes, difficulties arise.  It is not uncommon for an app to have 80% of its messages be a couple hundred of bytes or less, but 80% of the total data that it sends to be in large transfers that are, say, a megabyte or more.  Pre-posting receive buffers in such a situation is challenging.

A commonly used technique used to handle this situation is to implement one application level protocol for smaller messages, and use a separate protocol for transfers that are larger than some given threshold.  This would allow an application to post a bunch of smaller messages, say 4 KB, to receive data.  For transfers that are larger than 4 KB, a different communication protocol is used, possibly over a different socket or endpoint.

我们需要在讨论中绕道而行，以便更深入地研究网络问题和解决方案空间。我们不想继续将套接字视为具有发送和接收功能的单个实体，而是要单独考虑其组件。网络套接字可以被视为三个基本结构：传输层地址、发送或传输队列以及接收队列。因为我们的讨论将开始脱离纯套接字语义，我们将把我们的网络“套接字”称为端点。

为了减少应用程序的内存占用，我们需要考虑不属于套接字 API 的特性。到目前为止，大部分讨论都是围绕向对等点发送数据进行的。我们现在要关注接收数据的最佳机制。

对于套接字，当应用程序有数据要接收（例如，由 POLLIN 事件指示）时，我们调用 recv()。网络堆栈将接收到的数据复制到其缓冲区并返回。如果我们想避免接收端的数据复制，我们需要一种方法让应用程序在数据到达之前将其缓冲区发布到网络堆栈。

可以说，扩展套接字 API 以支持此功能的一种自然方式是让对 recv() 的每次调用都简单地将缓冲区发布到网络层。当接收到数据时，接收缓冲区会按照它们发布的顺序被移除。数据被复制到发布的缓冲区并返回给用户。应注意，发布的接收缓冲区的大小可能大于（或小于）接收的数据量。如果可用缓冲区空间更大，假设网络层可以等待很短的时间来查看是否有更多数据到达。如果没有其他内容到达，则接收完成，缓冲区返回给应用程序。

这引发了一个关于如何在接收端处理缓冲的问题。到目前为止，对于套接字，我们主要考虑的是流式协议。然而，许多应用程序处理的消息最终被分层覆盖在数据流上。如果他们发送一个 8 KB 的消息，他们希望接收者接收一个 8 KB 的消息。需要维护消息边界。

如果应用程序发送和接收固定大小的消息，则缓冲区分配变得微不足道。该应用程序可以发布 X 个缓冲区，每个缓冲区都具有最佳大小。但是，如果消息大小混杂，就会出现困难。应用程序有 80% 的消息是几百字节或更少的情况并不少见，但它发送的总数据的 80% 是大型传输，例如 1 兆字节或更多。在这种情况下预先发布接收缓冲区是具有挑战性的。

用于处理这种情况的常用技术是为较小的消息实现一个应用程序级协议，并为大于某个给定阈值的传输使用单独的协议。这将允许应用程序发布一堆较小的消息，例如 4 KB，以接收数据。对于大于 4 KB 的传输，使用不同的通信协议，可能通过不同的套接字或端点。

### Shared Receive Queues

If an application pre-posts receive buffers to a network queue, it needs to balance the size of each buffer posted, the number of buffers that are posted to each queue, and the number of queues that are in use.  With a socket like approach, each socket would maintain an independent receive queue where data is placed.  If an application is using 1000 endpoints and posts 100 buffers, each 4 KB, that results in 400 MB of memory space being consumed to receive data.  (We can start to realize that by eliminating memory copies, one of the trade offs is increased memory consumption.)  While 400 MB seems like a lot of memory, there is less than half a megabyte allocated to a single receive queue.  At today's networking speeds, that amount of space can be consumed within milliseconds.  The result is that if only a few endpoints are in use, the application will experience long delays where flow control will kick in and back the transfers off.

There are a couple of observations that we can make here.  The first is that in order to achieve high scalability, we need to move away from a connection-oriented protocol, such as streaming sockets.  Secondly, we need to reduce the number of receive queues that an application uses.

A shared receive queue is a network queue that can receive data for many different endpoints at once.  With shared receive queues, we no longer associate a receive queue with a specific transport address.  Instead network data will target a specific endpoint address.  As data arrives, the endpoint will remove an entry from the shared receive queue, place the data into the application's posted buffer, and return it to the user.  Shared receive queues can greatly reduce the amount of buffer space needed by an applications.  In the previous example, if a shared receive queue were used, the app could post 10 times the number of buffers (1000 total), yet still consume 100 times less memory (4 MB total).  This is far more scalable.  The drawback is that the application must now be aware of receive queues and shared receive queues, rather than considering the network only at the level of a socket.

如果应用程序将接收缓冲区预先发布到网络队列，它需要平衡每个发布的缓冲区的大小、发布到每个队列的缓冲区数量以及正在使用的队列数量。使用类似套接字的方法，每个套接字将维护一个放置数据的独立接收队列。如果应用程序使用 1000 个端点并发布 100 个缓冲区，每个 4 KB，这将导致 400 MB 的内存空间用于接收数据。 （我们可以开始意识到，通过消除内存副本，权衡之一是增加了内存消耗。）虽然 400 MB 似乎是很多内存，但分配给单个接收队列的内存不到 0.5 兆字节。以今天的网络速度，可以在几毫秒内消耗掉这么多的空间。结果是，如果只有少数端点在使用，应用程序将经历长时间的延迟，此时流量控制将启动并停止传输。

我们可以在这里进行一些观察。首先是为了实现高可扩展性，我们需要远离面向连接的协议，例如流式套接字。其次，我们需要减少应用程序使用的接收队列数量。

共享接收队列是一个网络队列，可以同时接收许多不同端点的数据。使用共享接收队列，我们不再将接收队列与特定传输地址相关联。相反，网络数据将针对特定的端点地址。当数据到达时，端点将从共享接收队列中删除一个条目，将数据放入应用程序的发布缓冲区，并将其返回给用户。共享接收队列可以大大减少应用程序所需的缓冲区空间量。在前面的示例中，如果使用共享接收队列，应用程序可以发布 10 倍的缓冲区（总共 1000 个），但仍然消耗 100 倍的内存（总共 4 MB）。这更具可扩展性。缺点是应用程序现在必须知道接收队列和共享接收队列，而不是仅在套接字级别考虑网络。

### Multi-Receive Buffers

Shared receive queues greatly improve application scalability; however, it still results in some inefficiencies as defined so far.  We've only considered the case of posting a series of fixed sized memory buffers to the receive queue.  As mentioned, determining the size of each buffer is challenging.  Transfers larger than the fixed size require using some other protocol in order to complete.  If transfers are typically much smaller than the fixed size, then the extra buffer space goes unused.

Again referring to our example, if the application posts 1000 buffers, then it can only receive 1000 messages before the queue is emptied.  At data rates measured in millions of messages per second, this will introduce stalls in the data stream.  An obvious solution is to increase the number of buffers posted.  The problem is dealing with variable sized messages, including some which are only a couple hundred bytes in length.  For example, if the average message size in our case is 256 bytes or less, then even though we've allocated 4 MB of buffer space, we only make use of 6% of that space.  The rest is wasted in order to handle messages which may only occasionally be up to 4 KB.

A second optimization that we can make is to fill up each posted receive buffer as messages arrive.  So, instead of a 4 KB buffer being removed from use as soon as a single 256 byte message arrives, it can instead receive up to 16, 256 byte, messages.  We refer to such a feature as 'multi-receive' buffers.

With multi-receive buffers, instead of posting a bunch of smaller buffers, we instead post a single larger buffer, say the entire 4 MB, at once.  As data is received, it is placed into the posted buffer.  Unlike TCP streams, we still maintain message boundaries.  The advantages here are twofold.  Not only is memory used more efficiently, allowing us to receive more smaller messages at once and larger messages overall, but we reduce the number of function calls that the application must make to maintain its supply of available receive buffers.

When combined with shared receive queues, multi-receive buffers help support optimal receive side buffering and processing.  The main drawback to supporting multi-receive buffers are that the application will not necessarily know up front how many messages may be associated with a single posted memory buffer.  This is rarely a problem for applications.

共享接收队列极大地提高了应用程序的可扩展性；但是，它仍然会导致迄今为止定义的一些低效率。我们只考虑了将一系列固定大小的内存缓冲区发布到接收队列的情况。如前所述，确定每个缓冲区的大小具有挑战性。大于固定大小的传输需要使用其他协议才能完成。如果传输通常比固定大小小得多，则额外的缓冲区空间将未被使用。

再次参考我们的示例，如果应用程序发布 1000 个缓冲区，那么在队列清空之前它只能接收 1000 条消息。在以每秒数百万条消息测量的数据速率下，这将在数据流中引入停顿。一个明显的解决方案是增加发布的缓冲区数量。问题在于处理可变大小的消息，包括一些只有几百字节长度的消息。例如，如果我们案例中的平均消息大小为 256 字节或更小，那么即使我们分配了 4 MB 的缓冲区空间，我们也只使用了该空间的 6%。其余的被浪费以处理可能只是偶尔达到 4 KB 的消息。

我们可以进行的第二个优化是在消息到达时填满每个发布的接收缓冲区。因此，不是在单个 256 字节消息到达时立即从使用中删除 4 KB 缓冲区，而是可以接收多达 16、256 字节的消息。我们将这种特性称为“多接收”缓冲区。

对于多接收缓冲区，我们不是发布一堆较小的缓冲区，而是一次发布一个更大的缓冲区，比如整个 4 MB。接收到数据后，会将其放入已发布的缓冲区中。与 TCP 流不同，我们仍然维护消息边界。这里的优势是双重的。不仅内存使用效率更高，允许我们一次接收更多较小的消息和整体较大的消息，而且我们减少了应用程序为维持其可用接收缓冲区的供应而必须进行的函数调用的数量。

当与共享接收队列结合使用时，多接收缓冲区有助于支持最佳接收端缓冲和处理。支持多接收缓冲区的主要缺点是应用程序不一定预先知道有多少消息可能与单个发布的内存缓冲区相关联。这对于应用程序来说很少是一个问题。

## Optimal Hardware Allocation

As part of scalability considerations, we not only need to consider the processing and memory resources of the host system, but also the allocation and use of the NIC hardware.  We've referred to network endpoints as combination of transport addressing, transmit queues, and receive queues.  The latter two queues are often implemented as hardware command queues.  Command queues are used to signal the NIC to perform some sort of work.  A transmit queue indicates that the NIC should transfer data.  A transmit command often contains information such as the address of the buffer to transmit, the length of the buffer, and destination addressing data.  The actual format and data contents vary based on the hardware implementation.

NICs have limited resources.  Only the most scalable, high-performance applications likely need to be concerned with utilizing NIC hardware optimally.  However, such applications are an important and specific focus of OFI.  Managing NIC resources is often handled by a resource manager application, which is responsible for allocating systems to competing applications, among other activities.

Supporting applications that wish to make optimal use of hardware requires that hardware related abstractions be exposed to the application.  Such abstractions cannot require a specific hardware implementation, and care must be taken to ensure that the resulting API is still usable by developers unfamiliar with dealing with such low level details.  Exposing concepts such as shared receive queues is an example of giving an application more control over how hardware resources are used.

作为可扩展性考虑的一部分，我们不仅需要考虑主机系统的处理和内存资源，还要考虑网卡硬件的分配和使用。我们将网络端点称为传输寻址、传输队列和接收队列的组合。后两个队列通常实现为硬件命令队列。命令队列用于向 NIC 发出信号以执行某种工作。传输队列指示 NIC 应该传输数据。传输命令通常包含诸如要传输的缓冲区地址、缓冲区长度和目标寻址数据等信息。实际格式和数据内容因硬件实现而异。

NIC 的资源有限。只有最具可扩展性的高性能应用程序才可能需要关注以最佳方式利用 NIC 硬件。然而，此类应用是 OFI 的一个重要且具体的重点。管理 NIC 资源通常由资源管理器应用程序处理，该应用程序负责将系统分配给竞争应用程序以及其他活动。

支持希望充分利用硬件的应用程序需要向应用程序公开与硬件相关的抽象。这种抽象不需要特定的硬件实现，必须注意确保生成的 API 仍然可供不熟悉处理此类低级细节的开发人员使用。公开诸如共享接收队列之类的概念是让应用程序更好地控制硬件资源使用方式的一个示例。

### Sharing Command Queues

By exposing the transmit and receive queues to the application, we open the possibility for the application that makes use of multiple endpoints to determine how those queues might be shared.  We talked about the benefits of sharing a receive queue among endpoints.  The benefits of sharing transmit queues are not as obvious.

An application that uses more addressable endpoints than there are transmit queues will need to share transmit queues among the endpoints.  By controlling which endpoint uses which transmit queue, the application can prioritize traffic.  A transmit queue can also be configured to optimize for a specific type of data transfer, such as large transfers only.

From the perspective of a software API, sharing transmit or receive queues implies exposing those constructs to the application, and allowing them to be associated with different endpoint addresses.

通过向应用程序公开传输和接收队列，我们为应用程序打开了可能性，该应用程序利用多个端点来确定如何共享这些队列。 我们讨论了在端点之间共享接收队列的好处。 共享传输队列的好处并不那么明显。

使用比传输队列更多的可寻址端点的应用程序将需要在端点之间共享传输队列。 通过控制哪个端点使用哪个传输队列，应用程序可以优先处理流量。 传输队列也可以配置为优化特定类型的数据传输，例如仅大型传输。

从软件 API 的角度来看，共享传输或接收队列意味着将这些构造暴露给应用程序，并允许它们与不同的端点地址相关联。

### Multiple Queues

The opposite of a shared command queue are endpoints that have multiple queues.  An application that can take advantage of multiple transmit or receive queues can increase parallel handling of messages without synchronization constraints.  Being able to use multiple command queues through a single endpoint has advantages over using multiple endpoints.  Multiple endpoints require separate addresses, which increases memory use.  A single endpoint with multiple queues can continue to expose a single address, while taking full advantage of available NIC resources.

与共享命令队列相反的是具有多个队列的端点。 可以利用多个传输或接收队列的应用程序可以增加对消息的并行处理而没有同步限制。 能够通过单个端点使用多个命令队列比使用多个端点具有优势。 多个端点需要单独的地址，这会增加内存使用。 具有多个队列的单个端点可以继续公开单个地址，同时充分利用可用的 NIC 资源。

## Progress Model Considerations 模型注意事项

One aspect of the sockets programming interface that developers often don't consider is the location of the protocol implementation.  This is usually managed by the operating system kernel.  The network stack is responsible for handling flow control messages, timing out transfers, re-transmitting unacknowledged transfers, processing received data, and sending acknowledgments.  This processing requires that the network stack consume CPU cycles.  Portions of that processing can be done within the context of the application thread, but much must be handled by kernel threads dedicated to network processing.

By moving the network processing directly into the application process, we need to be concerned with how network communication makes forward progress.  For example, how and when are acknowledgments sent?  How are timeouts and message re-transmissions handled?  The progress model defines this behavior, and it depends on how much of the network processing has been offloaded onto the NIC.

More generally, progress is the ability of the underlying network implementation to complete processing of an asynchronous request.  In many cases, the processing of an asynchronous request requires the use of the host processor.  For performance reasons, it may be undesirable for the provider to allocate a thread for this purpose, which will compete with the application thread(s).  We can avoid thread context switches if the application thread can be used to make forward progress on requests -- check for acknowledgments, retry timed out operations, etc.  Doing so requires that the application periodically call into the network stack.

开发人员通常不考虑的套接字编程接口的一个方面是协议实现的位置。这通常由操作系统内核管理。网络堆栈负责处理流控制消息、超时传输、重新传输未确认的传输、处理接收到的数据以及发送确认。此处理要求网络堆栈消耗 CPU 周期。该处理的一部分可以在应用程序线程的上下文中完成，但许多必须由专用于网络处理的内核线程处理。

通过将网络处理直接移动到应用程序进程中，我们需要关注网络通信如何向前推进。例如，如何以及何时发送确认？如何处理超时和消息重传？进度模型定义了这种行为，它取决于有多少网络处理已卸载到 NIC 上。

更一般地说，进度是底层网络实现完成异步请求处理的能力。在许多情况下，异步请求的处理需要使用主机处理器。出于性能原因，提供者可能不希望为此目的分配一个线程，这将与应用程序线程竞争。如果应用程序线程可用于对请求进行前向处理，我们可以避免线程上下文切换——检查确认、重试超时操作等。这样做需要应用程序定期调用网络堆栈。

## Ordering

Network ordering is a complex subject.  With TCP sockets, data is sent and received in the same order.  Buffers are re-usable by the application immediately upon returning from a function call.  As a result, ordering is simple to understand and use.  UDP sockets complicate things slightly.  With UDP sockets, messages may be received out of order from how they were sent.  In practice, this often doesn't occur, particularly, if the application only communicates over a local area network, such as Ethernet.

With our evolving network API, there are situations where exposing different order semantics can improve performance.  These details will be discussed further below.

网络排序是一个复杂的主题。 使用 TCP 套接字，数据以相同的顺序发送和接收。 从函数调用返回后，应用程序可以立即重用缓冲区。 因此，订购很容易理解和使用。 UDP 套接字会使事情稍微复杂化。 使用 UDP 套接字，接收到的消息可能与发送方式不同。 实际上，这通常不会发生，尤其是当应用程序仅通过局域网（例如以太网）进行通信时。

随着我们不断发展的网络 API，在某些情况下公开不同的顺序语义可以提高性能。 这些细节将在下面进一步讨论。

### Messages

UDP sockets allow messages to arrive out of order because each message is routed from the sender to the receiver independently.  This allows packets to take different network paths, to avoid congestion or take advantage of multiple network links for improved bandwidth.  We would like to take advantage of the same features in those cases where the application doesn't care in which order messages arrive.

Unlike UDP sockets, however, our definition of message ordering is more subtle.  UDP messages are small, MTU sized packets.  In our case, messages may be gigabytes in size.  We define message ordering to indicate whether the start of each message is processed in order or out of order.  This is related to, but separate from the order of how the message payload is received.

An example will help clarify this distinction.  Suppose that an application has posted two messages to its receive queue.  The first receive points to a 4 KB buffer.  The second receive points to a 64 KB buffer.  The sender will transmit a 4 KB message followed by a 64 KB message.  If messages are processed in order, then the 4 KB send will match with the 4 KB received, and the 64 KB send will match with the 64 KB receive.  However, if messages can be processed out of order, then the sends and receives can mismatch, resulting in the 64 KB send being truncated.

In this example, we're not concerned with what order the data is received in.  The 64 KB send could be broken in 64 1-KB transfers that take different routes to the destination.  So, bytes 2k-3k could be received before bytes 1k-2k.  Message ordering is not concerned with ordering _within_ a message, only _between_ messages.  With ordered messages, the messages themselves need to be processed in order.

The more relaxed message ordering can be the more optimizations that the network stack can use to transfer the data.  However, the application must be aware of message ordering semantics, and be able to select the desired semantic for its needs.  For the purposes of this section, messages refers to transport level operations, which includes RDMA and similar operations (some of which have not yet been discussed).

UDP 套接字允许消息无序到达，因为每条消息都是从发送方独立路由到接收方的。这允许数据包采用不同的网络路径，以避免拥塞或利用多个网络链接来提高带宽。在应用程序不关心消息到达的顺序的情况下，我们希望利用相同的功能。

然而，与 UDP 套接字不同，我们对消息顺序的定义更加微妙。 UDP 消息是 MTU 大小的小数据包。在我们的例子中，消息的大小可能是千兆字节。我们定义消息排序来指示每条消息的开始是按顺序处理还是乱序处理。这与接收消息有效负载的顺序有关，但与之不同。

一个例子将有助于阐明这种区别。假设应用程序已将两条消息发布到其接收队列。第一个接收指向一个 4 KB 的缓冲区。第二个接收指向一个 64 KB 的缓冲区。发送者将发送一个 4 KB 的消息，然后是一个 64 KB 的消息。如果消息按顺序处理，则 4 KB 发送将与 4 KB 接收匹配，64 KB 发送将与 64 KB 接收匹配。但是，如果可以乱序处理消息，则发送和接收可能会不匹配，从而导致 64 KB 发送被截断。

在此示例中，我们不关心接收数据的顺序。64 KB 发送可能会在 64 个 1 KB 传输中中断，这些传输采用不同的路由到达目的地。因此，可以在字节 1k-2k 之前接收字节 2k-3k。消息排序不关心消息的_within_ 排序，只关心_between_ 消息的排序。对于有序消息，消息本身需要按顺序处理。

消息排序越宽松，网络堆栈可以用来传输数据的优化就越多。但是，应用程序必须了解消息排序语义，并且能够根据需要选择所需的语义。就本节而言，消息指的是传输层操作，包括 RDMA 和类似操作（其中一些尚未讨论）。

### Data

Data ordering refers to the receiving and placement of data both _within and between_ messages.  Data ordering is most important to messages that can update the same target memory buffer.  For example, imagine an application that writes a series of database records directly into a peer memory location.  Data ordering, combined with message ordering, ensures that the data from the second write updates memory after the first write completes.  The result is that the memory location will contain the records carried in the second write.

Enforcing data ordering between messages requires that the messages themselves be ordered.  Data ordering can also apply within a single message, though this level of ordering is usually less important to applications.  Intra-message data ordering indicates that the data for a single message is received in order.  Some applications use this feature to 'spin' reading the last byte of a receive buffer.  Once the byte changes, the application knows that the operation has completed and all earlier data has been received.  (Note that while such behavior is interesting for benchmark purposes, using such a feature in this way is strongly discouraged.  It is not portable between networks or platforms.) 

数据排序是指在消息内和消息之间接收和放置数据。数据排序对于可以更新相同目标内存缓冲区的消息来说是最重要的。例如，想象一个将一系列数据库记录直接写入对等内存位置的应用程序。数据排序与消息排序相结合，可确保来自第二次写入的数据在第一次写入完成后更新内存。结果是内存位置将包含第二次写入时携带的记录。

强制消息之间的数据排序要求消息本身是有序的。数据排序也可以应用在单个消息中，尽管这种排序级别通常对应用程序不太重要。消息内数据排序表示按顺序接收单个消息的数据。一些应用程序使用此功能来“旋转”读取接收缓冲区的最后一个字节。一旦字节发生变化，应用程序就知道操作已经完成并且所有之前的数据都已经收到。 （请注意，虽然这种行为对于基准测试来说很有趣，但强烈建议不要以这种方式使用这种功能。它不能在网络或平台之间移植。）

### Completions

Completion ordering refers to the sequence that asynchronous operations report their completion to the application.  Typically, unreliable data transfer will naturally complete in the order that they are submitted to a transmit queue.  Each operation is transmitted to the network, with the completion occurring immediately after.  For reliable data transfers, an operation cannot complete until it has been acknowledged by the peer.  Since ack packets can be lost or possibly take different paths through the network, operations can be marked as completed out of order.  Out of order acks is more likely if messages can be processed out of order.

Asynchronous interfaces require that the application track their outstanding requests.  Handling out of order completions can increase application complexity, but it does allow for optimizing network utilization.

完成顺序是指异步操作向应用程序报告完成的顺序。 通常，不可靠的数据传输自然会按照它们提交到传输队列的顺序完成。 每个操作都被传输到网络，然后立即完成。 对于可靠的数据传输，操作只有在对等方确认后才能完成。 由于 ack 数据包可能会丢失或可能通过网络采用不同的路径，因此可以将操作标记为无序完成。 如果消息可以乱序处理，则更有可能出现乱序确认。

异步接口要求应用程序跟踪其未完成的请求。 处理乱序完成会增加应用程序的复杂性，但它确实可以优化网络利用率。

# OFI Architecture 开放Fabric接口架构

Libfabric is well architected to support the previously discussed features, with specific focus on exposing direct network access to an application.  Direct network access, sometimes referred to as RDMA, allows an application to access network resources without operating system intervention. Data transfers can occur between networking hardware and application memory with minimal software overhead. Although libfabric supports scalable network solutions, it does not mandate any implementation.  And the APIs have been defined specifically to allow multiple implementations.

The following diagram highlights the general architecture of the interfaces exposed by libfabric. For reference, the diagram shows libfabric in reference to a NIC.

Libfabric 的架构很好，可以支持前面讨论的功能，特别关注公开对应用程序的直接网络访问。 直接网络访问（有时称为 RDMA）允许应用程序访问网络资源而无需操作系统干预。 数据传输可以在网络硬件和应用程序内存之间以最小的软件开销进行。 尽管 libfabric 支持可扩展的网络解决方案，但它并不强制要求任何实现。 并且 API 已被专门定义为允许多种实现。

下图突出了 libfabric 公开的接口的一般架构。 作为参考，该图显示了参考 NIC 的 libfabric。

![Architecture](assets/libfabric-arch.png)

## Framework versus Provider 框架和提供者(实用程序+底层提供者)

OFI is divided into two separate components. The main component is the OFI framework, which defines the interfaces that applications use. The OFI framework provides some generic services; however, the bulk of the OFI implementation resides in the providers. Providers plug into the framework and supply access to fabric hardware and services. Providers are often associated with a specific hardware device or NIC. Because of the structure of the OFI framework, applications access the provider implementation directly for most operations, in order to ensure the lowest possible software latency.

One important provider is referred to as the **sockets provider**.  This provider implements the libfabric API over TCP sockets.  A primary objective of the sockets provider is to support development efforts.  Developers can write and test their code over the sockets provider on a small system, possibly even a laptop, before debugging on a larger cluster.  The sockets provider can also be used as a fallback mechanism for applications that wish to target libfabric features for high-performance networks, but which may still need to run on small clusters connected, for example, by Ethernet.

The UDP provider has a similar goal, but implements a much smaller feature set than the sockets provider.  The UDP provider is implemented over UDP sockets.  It only implements those features of libfabric which would be most useful for applications wanting unreliable, unconnected communication.  The primary goal of the UDP provider is to provide a simple building block upon which the framework can construct more complex features, such as reliability.  As a result, a secondary objective of the UDP provider is to improve application scalability when restricted to using native operating system sockets.

The final generic (not associated with a specific network technology) provider is often referred to as the **utility provider**.  The utility provider is a collection of software modules that can be used to extend the feature coverage of any provider.  For example, the utility provider layers over the UDP provider to implement connection-oriented and reliable endpoint types.  It can similarly layer over a provider that only supports connection-oriented communication to expose reliable, connection-less (aka reliable datagram) semantics.

Other providers target specific network technologies and systems, such as InfiniBand, Cray Aries networks, or Intel Omni-Path Architecture.

OFI 分为两个独立的组件。主要组件是 OFI 框架，它定义了应用程序使用的接口。 OFI 框架提供了一些通用服务；然而，大部分的 OFI 实现都存在于提供程序中。提供商插入框架并提供对结构硬件和服务的访问。提供程序通常与特定的硬件设备或 NIC 相关联。由于 OFI 框架的结构，应用程序直接访问提供程序实现以进行大多数操作，以确保尽可能低的软件延迟。

一个重要的提供者被称为套接字提供者。此提供程序通过 TCP 套接字实现 libfabric API。套接字提供者的主要目标是支持开发工作。开发人员可以在小型系统（甚至可能是笔记本电脑）上通过套接字提供程序编写和测试他们的代码，然后再在更大的集群上进行调试。对于希望针对高性能网络的 libfabric 功能但可能仍需要在连接的小型集群（例如通过以太网）上运行的应用程序，套接字提供程序也可以用作后备机制。

UDP 提供者具有类似的目标，但实现的功能集比套接字提供者小得多。 UDP 提供程序是通过 UDP 套接字实现的。它只实现了 libfabric 的那些特性，这些特性对于需要不可靠、未连接通信的应用程序最有用。 UDP 提供者的主要目标是提供一个简单的构建块，框架可以在该构建块上构建更复杂的特性，例如可靠性。因此，UDP 提供程序的次要目标是在仅限于使用本机操作系统套接字时提高应用程序的可伸缩性。

最终的通用（与特定网络技术无关）提供商通常被称为公用事业提供商。实用程序提供程序是一组软件模块，可用于扩展任何提供程序的功能覆盖范围。例如，实用程序提供程序在 UDP 提供程序之上分层，以实现面向连接和可靠的端点类型。它可以类似地覆盖仅支持面向连接的通信的提供者，以公开可靠的、无连接（也称为可靠数据报）语义。

其他供应商针对特定的网络技术和系统，例如 InfiniBand、Cray Aries 网络或英特尔 Omni-Path 架构。

## Control services

Control services are used by applications to discover information about the types of communication services available in the system. For example, discovery will indicate what fabrics are reachable from the local node, and what sort of communication each provides.

In terms of implementation, control services are handled primarily by a single API, **fi_getinfo()**.  Modeled very loosely on getaddrinfo(), it is used not just to discover what features are available in the system, but also how they might best be used by an application desiring maximum performance.

Control services themselves are not considered performance critical.  However, the information exchanged between an application and the providers must be expressive enough to indicate the most performant way to access the network.  Those details must be balanced with ease of use.  As a result, the fi_getinfo() call provides the ability to access complex network details, while allowing an application to ignore them if desired.

应用程序使用控制服务来发现有关系统中可用的通信服务类型的信息。 例如，发现将指示可以从本地节点访问哪些结构，以及每个结构提供什么样的通信。

在实现方面，控制服务主要由单个 API fi_getinfo() 处理。 在 getaddrinfo() 上非常松散地建模，它不仅用于发现系统中可用的功能，而且还用于发现需要最大性能的应用程序如何最好地使用它们。

控制服务本身不被视为性能关键。 但是，应用程序和提供者之间交换的信息必须具有足够的表达性，以指示访问网络的最佳性能方式。 这些细节必须与易用性相平衡。 因此，fi_getinfo() 调用提供了访问复杂网络详细信息的能力，同时允许应用程序在需要时忽略它们。

## Communication Services

Communication interfaces are used to setup communication between nodes. It includes calls to establish connections (connection management), as well as functionality used to address connection-less endpoints (address vectors).

The best match to socket routines would be connect(), bind(), listen(), and accept().  In fact the connection management calls are modeled after those functions, but with improved support for the asynchronous nature of the calls.  For performance and scalability reasons, connection-less endpoints use a unique model, that is not based on sockets or other network interfaces.  Address vectors are discussed in detail later, but target applications needing to talk with potentially thousands to millions of peers.  For applications communicating with a handful of peers, address vectors can slightly complicate initialization for connection-less endpoints.  (Connection-oriented endpoints may be a better option for such applications).

通信接口用于建立节点之间的通信。 它包括建立连接的调用（连接管理），以及用于寻址无连接端点的功能（地址向量）。

与套接字例程的最佳匹配是 connect()、bind()、listen() 和 accept()。 事实上，连接管理调用是根据这些功能建模的，但改进了对调用的异步特性的支持。 出于性能和可扩展性的原因，无连接端点使用独特的模型，它不基于套接字或其他网络接口。 地址向量将在稍后详细讨论，但目标应用程序需要与潜在的数千到数百万个对等方进行通信。 对于与少数对等方通信的应用程序，地址向量可能会使无连接端点的初始化稍微复杂化。 （对于此类应用程序，面向连接的端点可能是更好的选择）。

## Completion Services

OFI exports asynchronous interfaces. Completion services are used to report the results of submitted data transfer operations. Completions may be reported using the cleverly named completions queues, which provide details about the operation that completed. Or, completions may be reported using lower-impact counters that simply return the number of operations that have completed.

Completion services are designed with high-performance, low-latency in mind.  The calls map directly into the providers, and data structures are defined to minimize memory writes and cache impact.  Completion services do not have corresponding socket APIs.  (For Windows developers, they are similar to IO completion ports).

OFI 导出异步接口。 完成服务用于报告提交的数据传输操作的结果。 可以使用巧妙命名的完成队列来报告完成，该队列提供有关已完成操作的详细信息。 或者，可以使用影响较小的计数器报告完成，该计数器仅返回已完成的操作数。

完成服务的设计考虑了高性能、低延迟。 调用直接映射到提供程序，并定义数据结构以最小化内存写入和缓存影响。 完成服务没有相应的套接字 API。 （对于 Windows 开发人员来说，它们类似于 IO 完成端口）。

## Data Transfer Services

Applications have needs of different data transfer semantics.  The data transfer services in OFI are designed around different communication paradigms. Although shown outside the data transfer services, triggered operations are strongly related to the data transfer operations.

There are four basic data transfer interface sets. Message queues expose the ability to send and receive data with message boundaries being maintained. Message queues act as FIFOs, with sent messages matched with receive buffers in the order that messages are received.  The message queue APIs are derived from the socket data transfer APIs, such as send(). sendto(), sendmsg(), recv(), recvmsg(), etc.

Tag matching is similar to message queues in that it maintains message boundaries. Tag matching differs from message queues in that received messages are directed into buffers based on small steering tags that are carried in the sent message.  This allows a receiver to post buffers labeled 1, 2, 3, and so forth, with sends labeled respectively.  The benefit is that send 1 will match with receive buffer 1, independent of how send operations may be transmitted or re-ordered by the network.

RMA stands for remote memory access. RMA transfers allow an application to write data directly into a specific memory location in a target process, or to read memory from a specific address at the target process and return the data into a local buffer.  RMA is essentially equivalent to RDMA; the exception being that RDMA originally defined a specific transport implementation of RMA.

Atomic operations are often viewed as a type of extended RMA transfer. They permit direct access to the memory on the target process. The benefit of atomic operations is that they allow for manipulation of the memory, such as incrementing the value found at the target buffer.  So, where RMA can write the value X to a remote memory buffer, atomics can change the value of the remote memory buffer, say Y, to Y + 1. Because RMA and atomic operations provide direct access to a process’s memory buffers, additional security synchronization is needed.

应用程序需要不同的数据传输语义。 OFI 中的数据传输服务是围绕不同的通信范式设计的。尽管显示在数据传输服务之外，但触发操作与数据传输操作密切相关。

有四个基本的数据传输接口集。消息队列公开了在维护消息边界的情况下发送和接收数据的能力。消息队列充当 FIFO，发送的消息按照接收消息的顺序与接收缓冲区匹配。消息队列 API 派生自套接字数据传输 API，例如 send()。 sendto()、sendmsg()、recv()、recvmsg() 等

标签匹配类似于消息队列，因为它维护消息边界。标签匹配与消息队列的不同之处在于，接收到的消息根据发送消息中携带的小导向标签被定向到缓冲区中。这允许接收者发布标记为 1、2、3 等的缓冲区，并分别标记发送。好处是发送 1 将与接收缓冲区 1 匹配，而与网络如何传输或重新排序发送操作无关。

RMA 代表远程内存访问。 RMA 传输允许应用程序将数据直接写入目标进程中的特定内存位置，或者从目标进程的特定地址读取内存并将数据返回到本地缓冲区。 RMA 本质上等同于 RDMA；例外是 RDMA 最初定义了 RMA 的特定传输实现。

原子操作通常被视为一种扩展 RMA 传输。它们允许直接访问目标进程的内存。原子操作的好处是它们允许对内存进行操作，例如增加在目标缓冲区中找到的值。因此，在 RMA 可以将值 X 写入远程内存缓冲区的情况下，原子可以将远程内存缓冲区的值（例如 Y）更改为 Y + 1。因为 RMA 和原子操作提供对进程内存缓冲区的直接访问，所以额外的安全性需要同步。

## Memory Registration

Memory registration is the security mechanism used to grant a remote peer access to local memory buffers.  Registered memory regions associate memory buffers with permissions granted for access by fabric resources. A memory buffer must be registered before it can be used as the target of an RMA or atomic data transfer.  Memory registration supports a simple protection mechanism.  After a memory buffer has been registered, that registration request (buffer's address, buffer length, and access permission) is given a registration key.  Peers that issue RMA or atomic operations against that memory buffer must provide this key as part of their operation.  This helps protects against unintentional accesses to the region. (Memory registration can help guard against malicious access, but it is often too weak by itself to ensure system isolation.  Other, fabric specific, mechanisms protect against malicious access.  Those mechanisms are currently outside of the scope of the libfabric API.)

Memory registration often plays a secondary role with high-performance networks.  In order for a NIC to read or write application memory directly, it must access the physical memory pages that back the application's address space.  Modern operating systems employ page files that swap out virtual pages from one process with the virtual pages from another.  As a result, a physical memory page may map to different virtual addresses depending on when it is accessed.  Furthermore, when a virtual page is swapped in, it may be mapped to a new physical page.  If a NIC attempts to read or write application memory without being linked into the virtual address manager, it could access the wrong data, possibly corrupting an application's memory.  Memory registration can be used to avoid this situation from occurring.  For example, registered pages can be marked such that the operating system locks the virtual to physical mapping, avoiding any possibility of the virtual page being paged out or remapped.

内存注册是用于授予远程对等方访问本地内存缓冲区的安全机制。已注册的内存区域将内存缓冲区与授予结构资源访问权限相关联。必须先注册内存缓冲区，然后才能将其用作 RMA 或原子数据传输的目标。内存注册支持简单的保护机制。注册内存缓冲区后，该注册请求（缓冲区地址、缓冲区长度和访问权限）将获得注册密钥。对该内存缓冲区发出 RMA 或原子操作的对等方必须提供此密钥作为其操作的一部分。这有助于防止无意访问该区域。 （内存注册可以帮助防止恶意访问，但它本身通常太弱而无法确保系统隔离。其他特定于结构的机制可以防止恶意访问。这些机制目前超出了 libfabric API 的范围。）

内存注册通常在高性能网络中扮演次要角色。为了让 NIC 直接读取或写入应用程序内存，它必须访问支持应用程序地址空间的物理内存页。现代操作系统使用页面文件将一个进程的虚拟页面与另一个进程的虚拟页面交换出来。因此，物理内存页可能会根据访问时间映射到不同的虚拟地址。此外，当一个虚拟页面被换入时，它可能被映射到一个新的物理页面。如果 NIC 尝试在未链接到虚拟地址管理器的情况下读取或写入应用程序内存，它可能会访问错误的数据，可能会损坏应用程序的内存。可以使用内存注册来避免这种情况的发生。例如，可以标记已注册的页面，以便操作系统锁定虚拟到物理的映射，避免虚拟页面被调出或重新映射的任何可能性。

# Object Model 对象模型

Interfaces exposed by OFI are associated with different objects. The following diagram shows a high-level view of the parent-child relationships. 

OFI 公开的接口与不同的对象相关联。 下图显示了父子关系的高级视图。

![Object Model](assets/libfabric-objmod.png)

## Fabric

A fabric represents a collection of hardware and software resources that access a single physical or virtual network. For example, a fabric may be a single network subnet or cluster. All network ports on a system that can communicate with each other through the fabric belong to the same fabric. A fabric shares network addresses and can span multiple providers.

Fabrics are the top level object from which other objects are allocated.

fabric表示访问单个物理或虚拟网络的硬件和软件资源的集合。 例如，结构可以是单个网络子网或集群。 一个系统上所有可以通过Fabric进行通信的网络端口都属于同一个Fabric。 一个结构共享网络地址并且可以跨越多个提供商。

Fabrics 是分配其他对象的顶级对象。

## Domain

A domain represents a logical connection into a fabric. For example, a domain may correspond to a physical or virtual NIC. Because domains often correlate to a single NIC, a domain defines the boundary within which other resources may be associated.  Objects such as completion queues and active endpoints must be part of the same domain in order to be related to each other.

域代表与fabric的逻辑连接。 例如，域可能对应于物理或虚拟 NIC。 由于域通常与单个 NIC 相关联，因此域定义了其他资源可能关联的边界。 完成队列和活动端点等对象必须属于同一域才能相互关联。

## Passive Endpoint 被动端点

Passive endpoints are used by connection-oriented protocols to listen for incoming connection requests. Passive endpoints often map to software constructs and may span multiple domains.  They are best represented by a listening socket.  Unlike the socket API, however, in which an allocated socket may be used with either a connect() or listen() call, a passive endpoint may only be used with a listen call.

面向连接的协议使用被动端点来侦听传入的连接请求。 被动端点通常映射到软件结构并且可能跨越多个域。 它们最好由侦听套接字表示。 然而，与套接字 API 不同，其中分配的套接字可以与 connect() 或 listen() 调用一起使用，被动端点只能与 listen 调用一起使用。

## Event Queues

EQs are used to collect and report the completion of asynchronous operations and events. Event queues handle _control_ events, which are not directly associated with data transfer operations. The reason for separating control events from data transfer events is for performance reasons.  Control events usually occur during an application's initialization phase, or at a rate that's several orders of magnitude smaller than data transfer events. Event queues are most commonly used by connection-oriented protocols for notification of connection request or established events.  A single event queue may combine multiple hardware queues with a software queue and expose them as a single abstraction.

事件队列用于收集和报告异步操作和事件的完成情况。 事件队列处理_control_ 事件，这些事件与数据传输操作没有直接关联。 将控制事件与数据传输事件分开的原因是出于性能原因。 控制事件通常发生在应用程序的初始化阶段，或者以比数据传输事件小几个数量级的速率发生。 面向连接的协议最常使用事件队列来通知连接请求或已建立的事件。 单个事件队列可以将多个硬件队列与软件队列结合起来，并将它们公开为单个抽象。

## Wait Sets

The intended objective of a wait set is to reduce system resources used for signaling events. For example, a wait set may allocate a single file descriptor.  All fabric resources that are associated with the wait set will signal that file descriptor when an event occurs. The advantage is that the number of opened file descriptors is greatly reduced.   The closest operating system semantic would be the Linux epoll construct.  The difference is that a wait set does not merely multiplex file descriptors to another file descriptor, but allows for their elimination completely.  Wait sets allow a single underlying wait object to be signaled whenever a specified condition occurs on an associated event queue, completion queue, or counter.

等待集的预期目标是减少用于信令事件的系统资源。 例如，等待集可以分配单个文件描述符。 当事件发生时，与等待集相关联的所有结构资源都会向该文件描述符发出信号。 优点是打开的文件描述符的数量大大减少。 最接近的操作系统语义是 Linux epoll 结构。 不同之处在于等待集不仅将文件描述符多路复用到另一个文件描述符，而且允许完全消除它们。 等待集允许在关联的事件队列、完成队列或计数器上发生指定条件时发出单个底层等待对象的信号。

## Active Endpoint

Active endpoints are data transfer communication portals.  Active endpoints are used to perform data transfers, and are conceptually similar to a connected TCP or UDP socket. Active endpoints are often associated with a single hardware NIC,  with the data transfers partially or fully offloaded onto the NIC.

活动端点是数据传输通信门户。 活动端点用于执行数据传输，在概念上类似于连接的 TCP 或 UDP 套接字。 活动端点通常与单个硬件 NIC 相关联，数据传输部分或全部卸载到 NIC 上。

## Completion Queue

Completion queues are high-performance queues used to report the completion of data transfer operations. Unlike event queues, completion queues are often associated with a single hardware NIC, and may be implemented entirely in hardware.  Completion queue interfaces are designed to minimize software overhead.

完成队列是用于报告数据传输操作完成的高性能队列。 与事件队列不同，完成队列通常与单个硬件 NIC 相关联，并且可以完全在硬件中实现。 完成队列接口旨在最大限度地减少软件开销。

## Completion Counter

Completion queues are used to report information about which request has completed.  However, some applications use this information simply to track how many requests have completed.  Other details are unnecessary.  Completion counters are optimized for this use case.  Rather than writing entries into a queue, completion counters allow the provider to simply increment a count whenever a completion occurs.

完成队列用于报告有关哪个请求已完成的信息。 但是，某些应用程序仅使用此信息来跟踪已完成的请求数。 其他细节是不必要的。 完成计数器已针对此用例进行了优化。 完成计数器不是将条目写入队列，而是允许提供者在完成时简单地增加计数。

## Poll Set

OFI allows providers to use an application’s thread to process asynchronous requests. This can provide performance advantages for providers that use software to progress the state of a data transfer. Poll sets allow an application to group together multiple objects, such that progress can be driven across all associated data transfers. In general, poll sets are used to simplify applications where a manual progress model is employed.

OFI 允许提供者使用应用程序的线程来处理异步请求。 这可以为使用软件来推进数据传输状态的提供商提供性能优势。 轮询集允许应用程序将多个对象组合在一起，以便可以跨所有关联的数据传输推动进度。 通常，轮询集用于简化采用手动进度模型的应用程序。

## Memory Region

Memory regions describe application’s local memory buffers. In order for fabric resources to access application memory, the application must first grant permission to the fabric provider by constructing a memory region. Memory regions are required for specific types of data transfer operations, such as RMA and atomic operations.

内存区域描述应用程序的本地内存缓冲区。 为了让fabric资源访问应用程序内存，应用程序必须首先通过构造一个内存区域向fabric提供者授予权限。 特定类型的数据传输操作（例如 RMA 和原子操作）需要内存区域。

## Address Vectors 地址向量

Address vectors are used by connection-less endpoints. They map higher level addresses, such as IP addresses, which may be more natural for an application to use, into fabric specific addresses. The use of address vectors allows providers to reduce the amount of memory required to maintain large address look-up tables, and eliminate expensive address resolution and look-up methods during data transfer operations. 

地址向量由无连接端点使用。 它们将更高级别的地址（例如 IP 地址，对于应用程序使用起来可能更自然）映射到特定于结构的地址。 地址向量的使用允许提供商减少维护大型地址查找表所需的内存量，并在数据传输操作期间消除昂贵的地址解析和查找方法。

# Communication Model 通信模型

OFI supports three main communication endpoint types: reliable-connected, unreliable datagram, and reliable-unconnected. (The fourth option, unreliable-connected is unused by applications, so is not included as part of the current implementation). Communication setup is based on whether the endpoint is connected or unconnected.  Reliability is a feature of the endpoint's data transfer protocol.

## Connected Communications

The following diagram highlights the general usage behind connection-oriented communication. Connected communication is based on the flow used to connect TCP sockets, with improved asynchronous support.

下图突出了面向连接的通信背后的一般用法。 连接通信基于用于连接 TCP 套接字的流，并具有改进的异步支持。

![Connecting](assets/connections.PNG)

Connections require the use of both passive and active endpoints. In order to establish a connection, an application must first create a passive endpoint and associate it with an event queue. The event queue will be used to report the connection management events. The application then calls listen on the passive endpoint. A single passive endpoint can be used to form multiple connections.

The connecting peer allocates an active endpoint, which is also associated with an event queue. Connect is called on the active endpoint, which results in sending a connection request (CONNREQ) message to the passive endpoint. The CONNREQ event is inserted into the passive endpoint’s event queue, where the listening application can process it.

Upon processing the CONNREQ, the listening application will allocate an active endpoint to use with the connection. The active endpoint is bound with an event queue. Although the diagram shows the use of a separate event queue, the active endpoint may use the same event queue as used by the passive endpoint. Accept is called on the active endpoint to finish forming the connection. It should be noted that the OFI accept call is different than the accept call used by sockets. The differences result from OFI supporting process direct I/O.

OFI does not define the connection establishment protocol, but does support a traditional three-way handshake used by many technologies. After calling accept, a response is sent to the connecting active endpoint. That response generates a CONNECTED event on the remote event queue. If a three-way handshake is used, the remote endpoint will generate an acknowledgment message that will generate a CONNECTED event for the accepting endpoint. Regardless of the connection protocol, both the active and passive sides of the connection will receive a CONNECTED event that signals that the connection has been established.

连接需要使用被动和主动端点。为了建立连接，应用程序必须首先创建一个**被动端点**并将其与事件队列相关联。事件队列将用于报告连接管理事件。然后应用程序在被动端点上调用监听。单个被动端点可用于形成多个连接。

连接对等方分配一个活动端点，该端点也与一个事件队列相关联。在主动端点上调用 Connect，这会导致向被动端点发送连接请求 (CONNREQ) 消息。 CONNREQ 事件被插入到被动端点的事件队列中，监听应用程序可以在其中处理它。

在处理 CONNREQ 后，监听应用程序将分配一个活动端点以用于连接。活动端点与事件队列绑定。尽管该图显示了使用单独的事件队列，但主动端点可以使用与被动端点相同的事件队列。在活动端点上调用 Accept 以完成连接的形成。需要注意的是，OFI 接受调用不同于套接字使用的接受调用。差异源于 OFI 支持进程直接 I/O。

OFI 没有定义连接建立协议，但支持许多技术使用的传统三次握手。调用accept 后，会向连接的活动端点发送响应。该响应在远程事件队列上生成一个 CONNECTED 事件。如果使用三次握手，远程端点将生成一个确认消息，该消息将为接受端点生成一个 CONNECTED 事件。不管连接协议如何，连接的主动和被动侧都将收到一个 CONNECTED 事件，表明连接已建立。

## Connection-less Communications

Connection-less communication allows data transfers between active endpoints without going through a connection setup process. The diagram below shows the basic components needed to setup connection-less communication. Connection-less communication setup differs from UDP sockets in that it requires that the remote addresses be stored with libfabric.

无连接通信允许活动端点之间的数据传输，而无需经过连接设置过程。 下图显示了设置无连接通信所需的基本组件。 无连接通信设置与 UDP 套接字的不同之处在于它要求**远程地址与 libfabric 一起存储**。

![Connection-less](assets/connectionless.PNG)

OFI requires the addresses of peer endpoints be inserted into a local addressing table, or address vector, before data transfers can be initiated against the remote endpoint. Address vectors abstract fabric specific addressing requirements and avoid long queuing delays on data transfers when address resolution is needed. For example, IP addresses may need to be resolved into Ethernet MAC addresses. Address vectors allow this resolution to occur during application initialization time. OFI does not define how an address vector be implemented, only its conceptual model.

Because address vector setup is considered a control operation, and often occurs during an application's initialization phase, they may be used both synchronously and asynchronously.  When used synchronously, calls to insert new addresses into the AV block until the resolution completes.  When an address vector is used asynchronously, it must be associated with an event queue.  With the asynchronous model, after an address has been inserted into the AV and the fabric specific details have been resolved, a completion event is generated on the event queue.  Data transfer operations against that address are then permissible on active endpoints that are associated with the address vector.

All connection-less endpoints that transfer data must be associated with an address vector. 

OFI 要求将**对等端点**的地址插入到本地寻址表或地址向量中，然后才能针对远程端点启动数据传输。地址向量抽象结构特定的寻址要求，并在需要地址解析时避免数据传输的长时间排队延迟。例如，可能需要将 IP 地址解析为以太网 MAC 地址。地址向量允许在应用程序初始化期间进行此解析。 OFI 没有定义地址向量是如何实现的，只定义了它的概念模型。

因为地址向量设置被认为是一种控制操作，并且经常发生在应用程序的初始化阶段，所以它们可以同步和异步使用。同步使用时，调用将新地址插入 AV 块，直到解析完成。当异步使用地址向量时，它必须与事件队列相关联。使用异步模型，在将地址插入 AV 并解析结构特定细节后，在事件队列上生成完成事件。然后在与地址向量关联的活动端点上允许针对该地址的数据传输操作。

所有传输数据的无连接端点都必须与地址向量相关联。

# Endpoints

Endpoints represent communication portals, and all data transfer operations are initiated on endpoints. OFI defines the conceptual model for how endpoints are exposed to applications, as demonstrated in the diagrams below.

端点代表通信入口，所有数据传输操作都是在端点上发起的。 OFI 定义了端点如何向应用程序公开的概念模型，如下图所示。

![Endpoint](assets/libfabric-ep.png)

Endpoints are usually associated with a transmit context and a receive context. Transmit and receive contexts are often implemented using hardware queues that are mapped directly into the process’s address space, though OFI does not require this implementation. Although not shown, an endpoint may be configured only to transmit or receive data. Data transfer requests are converted by the underlying provider into commands that are inserted into transmit and/or receive contexts.

Endpoints are also associated with completion queues. Completion queues are used to report the completion of asynchronous data transfer operations. An endpoint may direct completed transmit and receive operations to separate completion queues, or the same queue (not shown)

端点通常与发送上下文和接收上下文相关联。 发送和接收上下文通常使用直接映射到进程地址空间的硬件队列来实现，尽管 OFI 不需要这种实现。 尽管未示出，端点可以被配置为仅发送或接收数据。 底层提供者将数据传输请求转换为插入传输和/或接收上下文的命令。

端点也与完成队列相关联。 完成队列用于报告异步数据传输操作的完成情况。 端点可以将完成的发送和接收操作引导到单独的完成队列或同一个队列（未显示）

## Shared Contexts

A more advanced usage model of endpoints that allows for resource sharing is shown below.

下面显示了允许资源共享的更高级的端点使用模型。

![Shared Contexts](assets/libfabric-shared-ctx.png)

Because transmit and receive contexts may be associated with limited hardware resources, OFI defines mechanisms for sharing contexts among multiple endpoints. The diagram above shows two endpoints each sharing transmit and receive contexts. However, endpoints may share only the transmit context or only the receive context or neither. Shared contexts allow an application or resource manager to prioritize where resources are allocated and how shared hardware resources should be used.

Completions are still associated with the endpoints, with each endpoint being associated with their own completion queue(s).

因为发送和接收上下文可能与有限的硬件资源相关联，OFI 定义了在多个端点之间共享上下文的机制。 上图显示了两个端点，每个端点共享传输和接收上下文。 然而，端点可以只共享传输上下文或只共享接收上下文，或者两者都不共享。 共享上下文允许应用程序或资源管理器优先考虑分配资源的位置以及应如何使用共享硬件资源。

完成仍然与端点相关联，每个端点都与它们自己的完成队列相关联。

### Receive Contexts

TODO

### Transmit Contexts

TODO

## Scalable Endpoints

The final endpoint model is known as a scalable endpoint. Scalable endpoints allow a single endpoint to take advantage of multiple underlying hardware resources.

最终端点模型称为可扩展端点。 可扩展端点允许单个端点利用多个底层硬件资源。

![Scalable Endpoints](assets/libfabric-scal-ep.png)

Scalable endpoints have multiple transmit and/or receive contexts. Applications can direct data transfers to use a specific context, or the provider can select which context to use. Each context may be associated with its own completion queue. Scalable contexts allow applications to separate resources to avoid thread synchronization or data ordering restrictions.

可扩展端点具有多个传输和/或接收上下文。 应用程序可以指导数据传输使用特定的上下文，或者提供者可以选择使用哪个上下文。 每个上下文可能与它自己的完成队列相关联。 可扩展上下文允许应用程序分离资源以避免线程同步或数据排序限制。

# Data Transfers

Obviously, the goal of network communication is to transfer data between systems. In the same way that sockets defines different data transfer semantics for TCP versus UDP sockets (streaming versus datagram messages), OFI defines different data transfer semantics. However, unlike sockets, OFI allows different semantics over a single endpoint, even when communicating with the same peer.

OFI defines separate API sets for the different data transfer semantics; although, there are strong similarities between the API sets.  The differences are the result of the parameters needed to invoke each type of data transfer.

显然，网络通信的目标是在系统之间传输数据。 就像套接字为 TCP 与 UDP 套接字（流与数据报消息）定义不同的数据传输语义一样，OFI 定义了不同的数据传输语义。 然而，与套接字不同，OFI 允许在单个端点上使用不同的语义，即使在与同一个对等点通信时也是如此。

OFI 为不同的数据传输语义定义了单独的 API 集； 不过，API 集之间有很强的相似性。 不同之处在于调用每种类型的数据传输所需的参数。

## Message transfers 消息传输

Message transfers are most similar to UDP datagram transfers.  The sender requests that data be transferred as a single transport operation to a peer.  Even if the data is referenced using an I/O vector, it is treated as a single logical unit.  The data is placed into a waiting receive buffer at the peer.  Unlike UDP sockets, message transfers may be reliable or unreliable, and many providers support message transfers that are gigabytes in size.

Message transfers are usually invoked using API calls that contain the string "send" or "recv".  As a result they may be referred to simply as sends or receives.

Message transfers involve the target process posting memory buffers to the receive context of its endpoint.  When a message arrives from the network, a receive buffer is removed from the Rx context, and the data is copied from the network into the receive buffer.  Messages are matched with posted receives in the order that they are received.  Note that this may differ from the order that messages are sent, depending on the transmit side's ordering semantics.  Furthermore, received messages may complete out of order.  For instance, short messages could complete before larger messages, especially if the messages originate from different peers.  Completion ordering semantics indicate the order that posted receive operations complete.

Conceptually, on the transmit side, messages are posted to a transmit context.  The network processes messages from the Tx context, packetizing the data into outbound messages.  Although many implementations process the Tx context in order (i.e. the Tx context is a true queue), ordering guarantees determine the actual processing order.  For example, sent messages may be copied to the network out of order if targeting different peers.

In the default case, OFI defines ordering semantics such that messages 1, 2, 3, etc. from the sender are received in the same order at the target.  Relaxed ordering semantics is an optimization technique that applications can opt into in order to improve network performance and utilization.

消息传输与 UDP 数据报传输最相似。发送方请求将数据作为单个传输操作传输到对等方。即使使用 I/O 向量引用数据，它也被视为单个逻辑单元。数据被放置在对等端的等待接收缓冲区中。与 UDP 套接字不同，消息传输可能是可靠的，也可能是不可靠的，并且许多提供商支持千兆字节大小的消息传输。

消息传输通常使用包含字符串“send”或“recv”的 API 调用来调用。因此，它们可以简称为发送或接收。

消息传输涉及目标进程将内存缓冲区发布到其端点的接收上下文。当消息从网络到达时，接收缓冲区会从 Rx 上下文中删除，并且数据会从网络复制到接收缓冲区中。消息按接收顺序与已发布的接收相匹配。请注意，这可能与发送消息的顺序不同，具体取决于发送方的排序语义。此外，接收到的消息可能会乱序完成。例如，短消息可以在较大消息之前完成，特别是如果消息来自不同的对等方。完成排序语义指示发布的接收操作完成的顺序。

从概念上讲，在传输端，消息被发布到传输上下文。网络处理来自 Tx 上下文的消息，将数据打包成出站消息。尽管许多实现按顺序处理 Tx 上下文（即 Tx 上下文是一个真正的队列），但排序保证决定了实际的处理顺序。例如，如果针对不同的对等点，发送的消息可能会乱序复制到网络。

在默认情况下，OFI 定义了排序语义，使得来自发送者的消息 1、2、3 等在目标处以相同的顺序接收。宽松排序语义(Relaxed ordering)是一种优化技术，应用程序可以选择使用它来提高网络性能和利用率。

## Tagged messages

Tagged messages are similar to message transfers except that the messages carry one additional piece of information, a message tag.  Tags are application defined values that are part of the message transfer protocol and are used to route packets at the receiver.  At a high level, they are roughly similar to sequence numbers or message ids.  The difference is that tag values are set by the application, may be any value, and duplicate tag values are allowed.

Each sent message carries a single tag value, which is used to select a receive buffer into which the data is copied.  On the receiving side, message buffers are also marked with a tag.  Messages that arrive from the network search through the posted receive messages until a matching tag is found.  Tags allow messages to be placed into overlapping groups.

Tags are often used to identify virtual communication groups or roles.  For example, one tag value may be used to identify a group of systems that contain input data into a program.  A second tag value could identify the systems involved in the processing of the data.  And a third tag may identify systems responsible for gathering the output from the processing.  (This is purely a hypothetical example for illustrative purposes only).  Moreover, tags may carry additional data about the type of message being used by each group.  For example, messages could be separated based on whether the context carries control or data information.

In practice, message tags are typically divided into fields.  For example, the upper 16 bits of the tag may indicate a virtual group, with the lower 16 bits identifying the message purpose.  The tag message interface in OFI is designed around this usage model.  Each sent message carries exactly one tag value, specified through the API.  At the receiver, buffers are associated with both a tag value and a mask.  The mask is applied to both the send and receive tag values (using a bit-wise AND operation).  If the resulting values match, then the tags are said to match.  The received data is then placed into the matched buffer.

For performance reasons, the mask is specified as 'ignore' bits. Although this is backwards from how many developers think of a mask (where the bits that are valid would be set to 1), the definition ends up mapping well with applications.  The actual operation performed when matching tags is:

带标签的消息与消息传输类似，只是消息带有一条附加信息，即消息标签。标签是应用程序定义的值，它们是消息传输协议的一部分，用于在接收器处路由数据包。在高层次上，它们与序列号或消息 ID 大致相似。不同之处在于标签值由应用程序设置，可以是任何值，并且允许重复的标签值。

每个发送的消息都带有一个标签值，用于选择将数据复制到的接收缓冲区。在接收端，消息缓冲区也标有标签。从网络到达的消息通过发布的接收消息进行搜索，直到找到匹配的标签。标签允许将消息放入重叠的组中。

标签通常用于标识虚拟通信组或角色。例如，一个标签值可用于标识包含程序输入数据的一组系统。第二个标签值可以识别涉及数据处理的系统。第三个标签可以识别负责收集处理输出的系统。 （这纯粹是一个假设示例，仅用于说明目的）。此外，标签可以携带有关每个组正在使用的消息类型的附加数据。例如，可以根据上下文是否携带控制信息或数据信息来分离消息。

在实践中，消息标签通常分为字段。例如，标签的高 16 位可以指示虚拟组，而低 16 位标识消息目的。 OFI 中的标签消息接口就是围绕这种使用模型设计的。每条发送的消息都只携带一个标签值，通过 API 指定。在接收方，缓冲区与标记值和掩码相关联。掩码应用于发送和接收标记值（使用按位与运算）。如果结果值匹配，则称标签匹配。然后将接收到的数据放入匹配的缓冲区中。

出于性能原因，掩码被指定为“忽略”位。尽管这与许多开发人员对掩码（其中有效位将设置为 1）的想法相反，但该定义最终与应用程序很好地映射。匹配标签时实际执行的操作是：

```
send_tag | ignore == recv_tag | ignore
/* this is equivalent to:
 * send_tag & ~ignore == recv_tag & ~ignore
 */
```

Tagged messages are equivalent of message transfers if a single tag value is used.  But tagged messages require that the receiver perform the matching operation at the target, which can impact performance versus untagged messages.

如果使用单个标记值，则标记消息等效于消息传输。 但是标记消息要求接收者在目标处执行匹配操作，这可能会影响与未标记消息相比的性能。

## RMA

RMA operations are architected such that they can require no processing at the RMA target.  NICs which offload transport functionality can perform RMA operations without impacting host processing.  RMA write operations transmit data from the initiator to the target.  The memory location where the data should be written is carried within the transport message itself.

RMA read operations fetch data from the target system and transfer it back to the initiator of the request, where it is copied into memory.  This too can be done without involving the host processor at the target system when the NIC supports transport offloading.

The advantage of RMA operations is that they decouple the processing of the peers.  Data can be placed or fetched whenever the initiator is ready without necessarily impacting the peer process.

Because RMA operations allow a peer to directly access the memory of a process, additional protection mechanisms are used to prevent unintentional or unwanted access.  RMA memory that is updated by a write operation or is fetched by a read operation must be registered for access with the correct permissions specified.

RMA 操作的架构使得它们不需要在 RMA 目标上进行处理。卸载传输功能的 NIC 可以在不影响主机处理的情况下执行 RMA 操作。 RMA 写操作将数据从发起方传输到目标方。应该写入数据的内存位置由传输消息本身携带。

RMA 读取操作从目标系统获取数据并将其传输回请求的发起者，然后将其复制到内存中。当 NIC 支持传输卸载时，这也可以在不涉及目标系统的主机处理器的情况下完成。

RMA 操作的优势在于它们将对等点的处理解耦。只要发起者准备好，就可以放置或获取数据，而不必影响对等进程。

因为 RMA 操作允许对等方直接访问进程的内存，所以使用额外的保护机制来防止无意或不需要的访问。通过写操作更新或通过读操作获取的 RMA 内存必须注册以使用指定的正确权限进行访问。

## Atomic operations

Atomic transfers are used to read and update data located in remote memory regions in an atomic fashion. Conceptually, they are similar to local atomic operations of a similar nature (e.g. atomic increment, compare and swap, etc.).  The benefit of atomic operations is they enable offloading basic arithmetic capabilities onto a NIC.  Unlike other data transfer operations, atomics require knowledge of the format of the data being accessed.

A single atomic function may operate across an array of data, applying an atomic operation to each entry, but the atomicity of an operation is limited to a single data type or entry.  OFI defines a wide variety of atomic operations across all common data types.  However support for a given operation is dependent on the provider implementation.

原子传输用于以原子方式读取和更新位于远程内存区域的数据。 从概念上讲，它们类似于性质相似的局部原子操作（例如原子增量、比较和交换等）。 原子操作的好处是它们可以将基本的算术能力卸载到 NIC 上。 与其他数据传输操作不同，原子操作需要了解所访问数据的格式。

单个原子函数可以跨数据数组进行操作，将原子操作应用于每个条目，但操作的原子性仅限于单个数据类型或条目。 OFI 定义了所有常见数据类型的各种原子操作。 但是，对给定操作的支持取决于提供者的实现。

# Fabric Interfaces

A full description of the libfabric API is documented in the relevant man pages.  This section provides an introduction to select interfaces, including how they may be used.  It does not attempt to capture all subtleties or use cases, nor describe all possible data structures or fields.

libfabric API 的完整描述记录在相关手册页中。 本节介绍选择接口，包括如何使用它们。 它不会试图捕捉所有的细微之处或用例，也不会描述所有可能的数据结构或字段。

## Using fi_getinfo

https://ofiwg.github.io/libfabric/v1.13.2/man/fi_getinfo.3.html

The fi_getinfo() call is one of the first calls that most applications will invoke.  It is designed to be easy to use for simple applications, but extensible enough to configure a network for optimal performance.  It serves several purposes. First, it abstracts away network implementation and addressing details.  Second, it allows an application to specify which features they require of the network.  Last, it provides a mechanism for a provider to report how an application can use the network in order to achieve the best performance.

fi_getinfo() 调用是大多数应用程序将首先调用的调用之一。 它旨在易于用于简单的应用程序，但可扩展性足以配置网络以获得最佳性能。 它有几个目的。 首先，它抽象出网络实现和寻址细节。 其次，它允许应用程序指定他们需要网络的哪些功能。 最后，它为提供商提供了一种机制来报告应用程序如何使用网络以获得最佳性能。

fi_getinfo, fi_freeinfo - Obtain / free fabric interface information

fi_allocinfo, fi_dupinfo - Allocate / duplicate an fi_info structure 分配/复制一个 fi_info 结构

```
/* API prototypes */
struct fi_info *fi_allocinfo(void);

int fi_getinfo(int version, const char *node, const char *service,
    uint64_t flags, struct fi_info *hints, struct fi_info **info);
```

```
/* Sample initialization code flow */
struct fi_info *hints, *info;

hints = fi_allocinfo();

/* hints will point to a cleared fi_info structure
 * Initialize hints here to request specific network capabilities
 */

fi_getinfo(FI_VERSION(1, 4), NULL, NULL, 0, hints, &info);
fi_freeinfo(hints);

/* Use the returned info structure to allocate fabric resources */
```

The hints parameter is the key for requesting fabric services.  The fi_info structure contains several data fields, plus pointers to a wide variety of attributes.  The fi_allocinfo() call simplifies the creation of an fi_info structure.  In this example, the application is merely attempting to get a list of what providers are available in the system and the features that they support.  Note that the API is designed to be extensible.  Versioning information is provided as part of the fi_getinfo() call.  The version is used by libfabric to determine what API features the application is aware of.  In this case, the application indicates that it can properly handle any feature that was defined for the 1.4 release (or earlier).

Applications should _always_ hard code the version that they are written for into the fi_getinfo() call.  This ensures that newer versions of libfabric will provide backwards compatibility with that used by the application.

Typically, an application will initialize the hints parameter to list the features that it will use.

hints(提示/示意) 参数是请求fabric 服务的关键。 fi_info 结构包含几个数据字段，以及指向各种属性的指针。 fi_allocinfo() 调用简化了 fi_info 结构的创建。在此示例中，应用程序仅尝试获取系统中可用的提供程序及其支持的功能的列表。请注意，API 设计为可扩展的。版本信息作为 fi_getinfo() 调用的一部分提供。 libfabric 使用该版本来确定应用程序知道哪些 API 功能。在这种情况下，应用程序表明它可以正确处理为 1.4 版本（或更早版本）定义的任何功能。

应用程序应该_总是_将它们所编写的版本硬编码到 fi_getinfo() 调用中。这确保了较新版本的 libfabric 将提供与应用程序使用的向后兼容性。

通常，应用程序将初始化提示参数以列出它将使用的功能。

```
/* Taking a peek at the contents of fi_info */
struct fi_info {
    struct fi_info *next;
    uint64_t caps;
    uint64_t mode;
    uint32_t addr_format;
    size_t src_addrlen;
    size_t dest_addrlen;
    void *src_addr;
    void *dest_addr;
    fid_t handle;
    struct fi_tx_attr *tx_attr;
    struct fi_rx_attr *rx_attr;
    struct fi_ep_attr *ep_attr;
    struct fi_domain_attr *domain_attr;
    struct fi_fabric_attr *fabric_attr;
};
```

The fi_info structure references several different attributes, which correspond to the different OFI objects that an application allocates.  Details of the various attribute structures are defined below.  For basic applications, modifying or accessing most attribute fields are unnecessary.  Many applications will only need to deal with a few fields of fi_info, most notably the capability (caps) and mode bits.

On success, the fi_getinfo() function returns a linked list of fi_info structures. Each entry in the list will meet the conditions specified through the hints parameter. The returned entries may come from different network providers, or may differ in the returned attributes. For example, if hints does not specify a particular endpoint type, there may be an entry for each of the three endpoint types.  As a general rule, libfabric returns the list of fi_info structures in order from most desirable to least.  High-performance network providers are listed before more generic providers, such as the socket or UDP providers.

fi_info 结构引用了几个不同的属性，这些属性对应于应用程序分配的不同 OFI 对象。各种属性结构的细节定义如下。对于基本应用程序，不需要修改或访问大多数属性字段。许多应用程序只需要处理 fi_info 的几个字段，最值得注意的是能力（上限）和模式位。

成功时，fi_getinfo() 函数返回 fi_info 结构的链表。列表中的每个条目都将满足通过hints 参数指定的条件。返回的条目可能来自不同的网络提供商，或者返回的属性可能不同。例如，如果提示没有指定特定的端点类型，则三种端点类型中的每一种都可能有一个条目。作为一般规则，libfabric 按从最理想到最不理想的顺序返回 fi_info 结构列表。高性能网络提供程序列在更通用的提供程序之前，例如套接字或 UDP 提供程序。



### Capabilities

The fi_info caps field is used to specify the features and services that the application requires of the network.  This field is a bit-mask of desired capabilities.  There are capability bits for each of the data transfer services mentioned above: FI_MSG, FI_TAGGED, FI_RMA, and FI_ATOMIC.  Applications should set each bit for each set of operations that it will use.  These bits are often the only bits set by an application.

In some cases, additional bits may be used to limit how a feature will be used.  For example, an application can use the FI_SEND or FI_RECV bits to indicate that it will only send or receive messages, respectively.  Similarly, an application that will only initiate RMA writes, can set the FI_WRITE bit, leaving FI_REMOTE_WRITE unset.  The FI_SEND and FI_RECV bits can be used to restrict the supported message and tagged operations.  By default, if FI_MSG or FI_TAGGED are set, the resulting endpoint will be enabled to both send and receive messages.  Likewise, FI_READ, FI_WRITE, FI_REMOTE_READ, FI_REMOTE_WRITE can restrict RMA and atomic operations.

Capabilities are grouped into two general categories: primary and secondary. Primary capabilities must explicitly be requested by an application, and a provider must enable support for only those primary capabilities which were selected. Secondary capabilities may optionally be requested by an application. If requested, a provider must support a capability if it is asked for or fail the fi_getinfo request. A provider may optionally report non-selected secondary capabilities if doing so would not compromise performance or security.  That is, a provider may grant an application a secondary capability, whether the application requests it or not.

All of the capabilities discussed so far are primary.  Secondary capabilities mostly deal with features desired by highly scalable, high-performance applications.  For example, the FI_MULTI_RECV secondary capability indicates if the provider can support the multi-receive buffers feature described above.

Because different providers support different sets of capabilities, applications that desire optimal network performance may need to code for a capability being either present or absent.  When present, such capabilities can offer a scalability or performance boost.  When absent, an application may prefer to adjust its protocol or implementation to work around the network limitations.  Although providers can often emulate features, doing so can impact overall performance, including the performance of data transfers that otherwise appear unrelated to the feature in use.  For example, if a provider needs to insert protocol headers into the message stream in order to implement a given capability, the appearance of that header could negatively impact the performance of all transfers. By exposing such limitations to the application, the app has better control over how to best emulate the feature or work around its absence.

It is recommended that applications code for only those capabilities required to achieve the best performance.  If a capability would have little to no effect on overall performance, developers should avoid using such features as part of an initial implementation. This will allow the application to work well across the widest variety of hardware.  Application optimizations can then add support for less common features.  To see which features are supported by which providers, see the libfabric [Provider Feature Maxtrix](https://github.com/ofiwg/libfabric/wiki/Provider-Feature-Matrix) for the relevant release.

fi_info caps 字段用于指定应用程序需要的网络功能和服务。该字段是所需功能的位掩码。上面提到的每个数据传输服务都有能力位：FI_MSG、FI_TAGGED、FI_RMA 和 FI_ATOMIC。应用程序应为其将使用的每组操作设置每个位。这些位通常是应用程序设置的唯一位。

在某些情况下，可能会使用额外的位来限制功能的使用方式。例如，应用程序可以使用 FI_SEND 或 FI_RECV 位来分别指示它将仅发送或接收消息。同样，仅启动 RMA 写入的应用程序可以设置 FI_WRITE 位，而 FI_REMOTE_WRITE 未设置。 FI_SEND 和 FI_RECV 位可用于限制支持的消息和标记操作。默认情况下，如果设置了 FI_MSG 或 FI_TAGGED，则生成的端点将启用发送和接收消息。同样，FI_READ、FI_WRITE、FI_REMOTE_READ、FI_REMOTE_WRITE 可以限制 RMA 和原子操作。

能力分为两大类：主要的和次要的。主要功能必须由应用程序明确请求，并且提供者必须仅启用对那些被选择的主要功能的支持。辅助能力可以由应用程序可选地请求。如果请求，提供者必须在请求或失败 fi_getinfo 请求时支持功能。如果这样做不会损害性能或安全性，提供者可以选择性地报告未选择的辅助功能。也就是说，提供者可以授予应用程序次要能力，无论应用程序是否请求它。

到目前为止讨论的所有功能都是主要的。辅助功能主要处理高度可扩展的高性能应用程序所需的功能。例如，FI_MULTI_RECV 辅助功能指示提供程序是否可以支持上述多接收缓冲区功能。

因为不同的供应商支持不同的功能集，所以需要最佳网络性能的应用程序可能需要针对存在或不存在的功能进行编码。如果存在，此类功能可以提供可扩展性或性能提升。如果不存在，应用程序可能更愿意调整其协议或实现以解决网络限制。尽管提供商通常可以模拟功能，但这样做会影响整体性能，包括在其他情况下看起来与使用的功能无关的数据传输的性能。例如，如果提供者需要将协议标头插入消息流中以实现给定的功能，则该标头的出现可能会对所有传输的性能产生负面影响。通过将这些限制暴露给应用程序，应用程序可以更好地控制如何最好地模拟该功能或解决它的缺失。

建议应用程序仅针对实现最佳性能所需的功能进行编码。如果一项功能对整体性能几乎没有影响，开发人员应避免将此类功能用作初始实现的一部分。这将允许应用程序在最广泛的硬件上运行良好。然后，应用程序优化可以添加对不太常见的功能的支持。要查看哪些提供程序支持哪些功能，请参阅相关版本的 libfabric [Provider Feature Maxtrix](https://github.com/ofiwg/libfabric/wiki/Provider-Feature-Matrix)。

### Mode Bits

Where capability bits represent features desired by applications, mode bits correspond to behavior requested by the provider.  That is, capability bits are top down requests, whereas mode bits are bottom up restrictions.  Mode bits are set by the provider to request that the application use the API in a specific way in order to achieve optimal performance.  Mode bits often imply that the additional work needed by the application will be less overhead than forcing that same implementation down into the provider.  Mode bits arise as a result of hardware implementation restrictions.

An application developer decides which mode bits they want to or can easily support as part of their development process.  Each mode bit describes a particular behavior that the application must follow to use various interfaces.  Applications set the mode bits that they support when calling fi_getinfo().  If a provider requires a mode bit that isn't set, that provider will be skipped by fi_getinfo().  If a provider does not need a mode bit that is set, it will respond to the fi_getinfo() call, with the mode bit cleared.  This indicates that the application does not need to perform the action required by the mode bit.

One of the most common mode bits needed by providers is FI_CONTEXT.  This mode bit requires that applications pass in a libfabric defined data structure (struct fi_context) into any data transfer function.  That structure must remain valid and unused by the application until the data transfer operation completes.  The purpose behind this mode bit is that the struct fi_context provides "scratch" space that the provider can use to track the request.  For example, it may need to insert the request into a linked list, or track the number of times that an outbound transfer has been retried.  Since many applications already track outstanding operations with their own data structure, by embedding the struct fi_context into that same structure, overall performance can be improved.  This avoids the provider needing to allocate and free internal structures for each request.

Continuing with this example, if an application does not already track outstanding requests, then it would leave the FI_CONTEXT mode bit unset.  This would indicate that the provider needs to get and release its own structure for tracking purposes.  In this case, the costs would essentially be the same whether it were done by the application or provider.

For the broadest support of different network technologies, applications should attempt to support as many mode bits as feasible.  Most providers attempt to support applications that cannot support any mode bits, with as small an impact as possible.  However, implementation of mode bit avoidance in the provider will often impact latency tests.

能力位代表应用程序所需的功能，模式位对应于提供者请求的行为。也就是说，能力位是自上而下的请求，而模式位是自下而上的限制。模式位由提供程序设置以请求应用程序以特定方式使用 API 以获得最佳性能。模式位通常意味着应用程序所需的额外工作将比将相同的实现强制到提供程序中更少的开销。模式位是硬件实现限制的结果。

作为开发过程的一部分，应用程序开发人员决定他们想要或可以轻松支持哪些模式位。每个模式位都描述了应用程序使用各种接口必须遵循的特定行为。应用程序在调用 fi_getinfo() 时设置它们支持的模式位。如果提供程序需要未设置的模式位，则 fi_getinfo() 将跳过该提供程序。如果提供者不需要设置模式位，它将响应 fi_getinfo() 调用，并清除模式位。这表明应用程序不需要执行模式位所需的操作。

提供者需要的最常见的模式位之一是 FI_CONTEXT。此模式位要求应用程序将 libfabric 定义的数据结构 (struct fi_context) 传递到任何数据传输函数中。在数据传输操作完成之前，该结构必须保持有效且未被应用程序使用。此模式位背后的目的是 struct fi_context 提供提供者可以用来跟踪请求的“临时”空间。例如，它可能需要将请求插入到链表中，或跟踪出站传输已重试的次数。由于许多应用程序已经使用自己的数据结构跟踪未完成的操作，通过将结构 fi_context 嵌入到相同的结构中，可以提高整体性能。这避免了提供者需要为每个请求分配和释放内部结构。

继续此示例，如果应用程序尚未跟踪未完成的请求，则它将保留 FI_CONTEXT 模式位未设置。这表明提供者需要获取和发布其自己的结构以进行跟踪。在这种情况下，无论是由应用程序还是提供商完成，成本基本上是相同的。

为了最广泛地支持不同的网络技术，应用程序应尝试支持尽可能多的模式位。大多数提供商都试图以尽可能小的影响来支持无法支持任何模式位的应用程序。但是，在提供程序中实现模式位避免通常会影响延迟测试。

# FIDs

FID stands for fabric identifier.  It is the conceptual equivalent to a file descriptor.  All fabric resources are represented by a fid structure, and all fid's are derived from a base fid type.  In object-oriented terms, a fid would be the parent class.  The contents of a fid are visible to the application.

FID 代表fabric标识符。 它在概念上等同于文件描述符。 所有的结构资源都由一个fid 结构表示，所有的fid 都派生自一个基本的fid 类型。 在面向对象的术语中，fid 将是父类。 fid 的内容对应用程序是可见的。

```
/* Base FID definition */
enum {
    FI_CLASS_UNSPEC,
    FI_CLASS_FABRIC,
    FI_CLASS_DOMAIN,
    ...
};

struct fi_ops {
    size_t size;
    int (*close)(struct fid *fid);
    ...
};

/* All fabric interface descriptors must start with this structure */
struct fid {
    size_t fclass;
    void *context;
    struct fi_ops *ops;
};

```

The fid structure is designed as a trade-off between minimizing memory footprint versus software overhead.  Each fid is identified as a specific object class.  Examples are given above (e.g. FI_CLASS_FABRIC).  The context field is an application defined data value.  The context field is usually passed as a parameter into the call that allocates the fid structure (e.g. fi_fabric() or fi_domain()).  The use of the context field is application specific.  Applications often set context to a corresponding structure that they've allocated.  The context field is the only field that applications are recommended to access directly.  Access to other fields should be done using defined function calls.

The ops field points to a set of function pointers.  The fi_ops structure defines the operations that apply to that class.  The size field in the fi_ops structure is used for extensibility, and allows the fi_ops structure to grow in a backward compatible manner as new operations are added.  The fid deliberately points to the fi_ops structure, rather than embedding the operations directly.  This allows multiple fids to point to the same set of ops, which minimizes the memory footprint of each fid. (Internally, providers usually set ops to a static data structure, with the fid structure dynamically allocated.)

Although it's possible for applications to access function pointers directly, it is strongly recommended that the static inline functions defined in the man pages be used instead.  This is required by applications that may be built using the FABRIC_DIRECT library feature.  (FABRIC_DIRECT is a compile time option that allows for highly optimized builds by tightly coupling an application with a specific provider.  See the man pages for more details.)

Other OFI classes are derived from this structure, adding their own set of operations.

fid 结构被设计为最小化内存占用与软件开销之间的权衡。每个fid 都被标识为一个特定的对象类。上面给出了示例（例如 FI_CLASS_FABRIC）。上下文字段是应用程序定义的数据值。上下文字段通常作为参数传递给分配 fid 结构的调用（例如 fi_fabric() 或 fi_domain()）。上下文字段的使用是特定于应用程序的。应用程序通常将上下文设置为它们已分配的相应结构。上下文字段是唯一建议应用程序直接访问的字段。应该使用定义的函数调用来访问其他字段。

ops 字段指向一组函数指针。 fi_ops 结构定义了适用于该类的操作。 fi_ops 结构中的 size 字段用于可扩展性，并允许 fi_ops 结构在添加新操作时以向后兼容的方式增长。 fid 故意指向 fi_ops 结构，而不是直接嵌入操作。这允许多个 fid 指向同一组操作，从而最大限度地减少每个 fid 的内存占用。 （在内部，提供者通常将 ops 设置为静态数据结构，并动态分配 fid 结构。）

尽管应用程序可以直接访问函数指针，但强烈建议改用手册页中定义的静态内联函数。这是可能使用 FABRIC_DIRECT 库功能构建的应用程序所必需的。 （FABRIC_DIRECT 是一个编译时选项，它允许通过将应用程序与特定提供程序紧密耦合来进行高度优化的构建。有关更多详细信息，请参见手册页。）

其他 OFI 类都是从这种结构派生的，添加了自己的一组操作。

```
/* Example of deriving a new class for a fabric object */
struct fi_ops_fabric {
    size_t size;
    int (*domain)(struct fid_fabric *fabric, struct fi_info *info,
        struct fid_domain **dom, void *context);
    ...
};

struct fid_fabric {
    struct fid fid;
    struct fi_ops_fabric *ops;
};
```

Other fid classes follow a similar pattern as that shown for fid_fabric.  The base fid structure is followed by zero or more pointers to operation sets.

其他 fid 类遵循与 fid_fabric 类似的模式。 基本 fid 结构后跟零个或多个指向操作集的指针。

# Fabric

The top-level object that applications open is the fabric identifier.  The fabric can mostly be viewed as a container object by applications, though it does identify which provider applications use. (Future extensions are likely to expand methods that apply directly to the fabric object.  An example is adding topology data to the API.)

Opening a fabric is usually a straightforward call after calling fi_getinfo().

应用程序打开的顶级对象是结构标识符。 Fabric 主要可以被应用程序视为一个容器对象，尽管它确实确定了哪些提供程序应用程序使用。 （未来的扩展可能会扩展直接应用于结构对象的方法。一个例子是向 API 添加拓扑数据。）

在调用 fi_getinfo() 之后打开一个结构通常是一个简单的调用。

```
int fi_fabric(struct fi_fabric_attr *attr, struct fid_fabric **fabric, void *context);
```

The fabric attributes can be directly accessed from struct fi_info. The newly opened fabric is returned through the 'fabric' parameter.  The 'context' parameter appears in many operations.  It is a user-specified value that is associated with the fabric.  It may be used to point to an application specific structure and is retrievable from struct fid_fabric.

结构属性可以直接从 struct fi_info 访问。 通过'fabric'参数返回新打开的fabric。 'context' 参数出现在许多操作中。 它是与结构关联的用户指定值。 它可用于指向特定于应用程序的结构，并可从 struct fid_fabric 中检索。

## Attributes

The fabric attributes are straightforward.

```
struct fi_fabric_attr {
    struct fid_fabric *fabric;
    char *name;
    char *prov_name;
    uint32_t prov_version;
};
```

The only field that applications are likely to use directly is the prov_name.  This is a string value that can be used by hints to select a specific provider for use.  On most systems, there will be multiple providers available.  Only one is likely to represent the high-performance network attached to the system.  Others are generic providers that may be available on any system, such as the TCP socket and UDP providers.

The fabric field is used to help applications manage open fabric resources.  If an application has already opened a fabric that can support the returned fi_info structure, this will be set to that fabric. The contents of struct fid_fabric is visible to applications.  It contains a pointer to the application's context data that was provided when the fabric was opened.

应用程序可能直接使用的唯一字段是 prov_name。 这是一个字符串值，提示可以使用它来选择要使用的特定提供程序。 在大多数系统上，将有多个提供程序可用。 只有一个可能代表连接到系统的高性能网络。 其他是通用提供程序，可以在任何系统上使用，例如 TCP 套接字和 UDP 提供程序。

Fabric 字段用于帮助应用程序管理开放的 Fabric 资源。 如果应用程序已经打开了可以支持返回的 fi_info 结构的结构，则会将其设置为该结构。 struct fid_fabric 的内容对应用程序可见。 它包含一个指向应用程序上下文数据的指针，该数据在打开结构时提供。

## Environment Variables

Environment variables are used by providers to configure internal options for optimal performance or memory consumption.  Libfabric provides an interface for querying which environment variables are usable, along with an application to display the information to a command window.  Although environment variables are usually configured by an administrator, an application can query for variables programmatically.

提供者使用环境变量来配置内部选项以实现最佳性能或内存消耗。 Libfabric 提供了一个用于查询哪些环境变量可用的接口，以及一个将信息显示到命令窗口的应用程序。 尽管环境变量通常由管理员配置，但应用程序可以通过编程方式查询变量。

```
/* APIs to query for supported environment variables */
enum fi_param_type {
    FI_PARAM_STRING,
    FI_PARAM_INT,
    FI_PARAM_BOOL
};

struct fi_param {
    /* The name of the environment variable */
    const char *name;
    /* What type of value it stores: string, integer, or boolean */
    enum fi_param_type type;
    /* A description of how the variable is used */
    const char *help_string;
    /* The current value of the variable */
    const char *value;
};

int fi_getparams(struct fi_param **params, int *count);
void fi_freeparams(struct fi_param *params);
```

The modification of environment variables is typically a tuning activity done on larger clusters.  However there are a few values that are useful for developers.  These can be seen by executing the fi_info command.

环境变量的修改通常是在较大的集群上进行的调整活动。 但是，有一些值对开发人员有用。 这些可以通过执行 fi_info 命令来查看。

```
$ fi_info -e
# FI_LOG_LEVEL: String
# Specify logging level: warn, trace, info, debug (default: warn)

# FI_LOG_PROV: String
# Specify specific provider to log (default: all)

# FI_LOG_SUBSYS: String
# Specify specific subsystem to log (default: all)

# FI_PROVIDER: String
# Only use specified provider (default: all available)
```

Full documentation for these variables is available in the man pages.  Variables beyond these may only be documented directly in the library itself, and available using the 'fi_info -e' command.

The FI_LOG_LEVEL can be used to increase the debug output from libfabric and the providers.  Note that in the release build of libfabric, debug output from data path operations (transmit, receive, and completion processing) may not be available.  The FI_PROVIDER variable can be used to enable or disable specific providers.  This is useful to ensure that a given provider will be used.

手册页中提供了这些变量的完整文档。 超出这些的变量只能直接记录在库本身中，并且可以使用“fi_info -e”命令获得。

FI_LOG_LEVEL 可用于增加 libfabric 和提供程序的调试输出。 请注意，在 libfabric 的发布版本中，数据路径操作（传输、接收和完成处理）的调试输出可能不可用。 FI_PROVIDER 变量可用于启用或禁用特定提供程序。 这对于确保使用给定的提供程序很有用。

# Domains

Domains usually map to a specific local network interface adapter.  A domain may either refer to the entire NIC, a port on a multi-port NIC, or a virtual device exposed by a NIC.  From the viewpoint of the application, a domain identifies a set of resources that may be used together.

Similar to a fabric, opening a domain is straightforward after calling fi_getinfo().

域通常映射到特定的本地网络接口适配器。 域可以指整个 NIC、多端口 NIC 上的端口或 NIC 公开的虚拟设备。 从应用程序的角度来看，域标识了一组可以一起使用的资源。

与结构类似，调用 fi_getinfo() 后打开域很简单。

```
int fi_domain(struct fid_fabric *fabric, struct fi_info *info,
    struct fid_domain **domain, void *context);
```

The fi_info structure returned from fi_getinfo() can be passed directly to fi_domain() to open a new domain.

从 fi_getinfo() 返回的 fi_info 结构可以直接传递给 fi_domain() 以打开一个新域。

## Attributes

A domain defines the relationship between data transfer services (endpoints) and completion services (completion queues and counters).  Many of the domain attributes describe that relationship and its impact to the application.

域定义了数据传输服务（端点）和完成服务（完成队列和计数器）之间的关系。 许多域属性描述了这种关系及其对应用程序的影响。

```
struct fi_domain_attr {
    struct fid_domain *domain;
    char *name;
    enum fi_threading threading;
    enum fi_progress control_progress;
    enum fi_progress data_progress;
    enum fi_resource_mgmt resource_mgmt;
    enum fi_av_type av_type;
    enum fi_mr_mode mr_mode;
    size_t mr_key_size;
    size_t cq_data_size;
    size_t cq_cnt;
    size_t ep_cnt;
    size_t tx_ctx_cnt;
    size_t rx_ctx_cnt;
    size_t max_ep_tx_ctx;
    size_t max_ep_rx_ctx;
    size_t max_ep_stx_ctx;
    size_t
```

Details of select attributes and their impact to the application are described below.

选择属性的详细信息及其对应用程序的影响如下所述。

## Threading

OFI defines a unique threading model.  The libfabric design is heavily influenced by object-oriented programming concepts.  A multi-threaded application must determine how libfabric objects (domains, endpoints, completion queues, etc.) will be allocated among its threads, or if any thread can access any object.  For example, an application may spawn a new thread to handle each new connected endpoint.  The domain threading field provides a mechanism for an application to identify which objects may be accessed simultaneously by different threads.  This in turn allows a provider to optimize or, in some cases, eliminate internal synchronization and locking around those objects.

The threading is best described as synchronization levels.  As threading levels increase, greater potential parallelism is achieved.  For example, an application can indicate that it will only access an endpoint from a single thread.  This allows the provider to avoid acquiring locks around data transfer calls, knowing that there cannot be two simultaneous calls to send data on the same endpoint.  The provider would only need to provide serialization if separate endpoints accessed the same shared software or hardware resources.

Threading defines where providers could optimize synchronization primitives.  However, providers may still implement more serialization than is needed by the application.  (This is usually a result of keeping the provider implementation simpler).

Various threading models are described in detail in the man pages.  Developers should study the fi_domain man page and available threading options, and select a mode that is best suited for how the application was designed.  If an application leaves the value undefined, providers will report the highest (most parallel) threading level that they support.

FI 定义了一个独特的线程模型。 libfabric 设计深受面向对象编程概念的影响。多线程应用程序必须确定如何在其线程之间分配 libfabric 对象（域、端点、完成队列等），或者任何线程是否可以访问任何对象。例如，应用程序可能会产生一个新线程来处理每个新连接的端点。域线程字段为应用程序提供了一种机制来识别哪些对象可以被不同的线程同时访问。这反过来又允许提供者优化，或者在某些情况下，消除围绕这些对象的内部同步和锁定。

最好将线程描述为同步级别。随着线程级别的增加，实现了更大的潜在并行性。例如，应用程序可以指示它将仅从单个线程访问端点。这允许提供者避免获取围绕数据传输调用的锁，因为它知道不能有两个同时调用来在同一端点上发送数据。如果单独的端点访问相同的共享软件或硬件资源，提供者只需要提供序列化。

线程定义了提供者可以在哪里优化同步原语。但是，提供者仍可能实现比应用程序所需的更多的序列化。 （这通常是保持提供者实现更简单的结果）。

手册页中详细描述了各种线程模型。开发人员应研究 fi_domain 手册页和可用的线程选项，并选择最适合应用程序设计方式的模式。如果应用程序未定义该值，提供者将报告他们支持的最高（最并行）线程级别。

## Progress

As previously discussed, progress models are a result of using the host processor in order to perform some portion of the transport protocol.  In order to simplify development, OFI defines two progress models: automatic or manual.  It does not attempt to identify which specific interface features may be offloaded, or what operations require additional processing by the application's thread.

Automatic progress means that an operation initiated by the application will eventually complete, even if the application makes no further calls into the libfabric API.  The operation is either offloaded entirely onto hardware, the provider uses an internal thread, or the operating system kernel may perform the task.  The use of automatic progress may increase system overhead and latency in the latter two cases.  For control operations, this is usually acceptable.  However, the impact to data transfers may be measurable, especially if internal threads are required to provide automatic progress.

The manual progress model can avoid this overhead for providers that do not offload all transport features into hardware.  With manual progress the provider implementation will handle transport operations as part of specific libfabric functions.  For example, a call to fi_cq_read() which retrieves a list of completed operations may also be responsible for sending ack messages to notify peers that a message has been received.  Since reading the completion queue is part of the normal operation of an application, there is little impact to the application and additional threads are avoided.

Applications need to take care when using manual progress, particularly if they link into libfabric multiple times through different code paths or library dependencies.  If application threads are used to drive progress, such as responding to received data with ACKs, then it is critical that the application thread call into libfabric in a timely manner.

OFI defines wait and poll set objects that are specifically designed to assist with driving manual progress.

如前所述，进度模型是使用主机处理器来执行传输协议的某些部分的结果。为了简化开发，OFI 定义了两种进度模型：自动或手动。它不会尝试识别哪些特定的接口功能可以卸载，或者哪些操作需要应用程序线程的额外处理。

自动进度意味着应用程序启动的操作最终将完成，即使应用程序没有进一步调用 libfabric API。该操作要么完全卸载到硬件上，要么提供程序使用内部线程，要么操作系统内核可以执行任务。在后两种情况下，使用自动进度可能会增加系统开销和延迟。对于控制操作，这通常是可以接受的。但是，对数据传输的影响可能是可衡量的，尤其是在需要内部线程来提供自动进度的情况下。

对于不将所有传输功能卸载到硬件中的提供商，手动进度模型可以避免这种开销。随着手动进度，提供程序实现将处理传输操作作为特定 libfabric 功能的一部分。例如，调用 fi_cq_read() 检索已完成操作的列表也可能负责发送 ack 消息以通知对等方已收到消息。由于读取完成队列是应用程序正常操作的一部分，因此对应用程序的影响很小，并且避免了额外的线程。

应用程序在使用手动进度时需要小心，尤其是当它们通过不同的代码路径或库依赖项多次链接到 libfabric 时。如果应用程序线程用于推动进度，例如使用 ACK 响应接收到的数据，那么应用程序线程及时调用 libfabric 至关重要。

OFI 定义了专门用于帮助推动手动进度的等待和轮询集对象。

## Memory Registration

RMA and atomic operations can both read and write memory that is owned by a peer process, and neither require the involvement of the target processor.  Because the memory can be modified over the network, an application must opt into exposing its memory to peers.  This is handled by the memory registration process.  Registered memory regions associate memory buffers with permissions granted for access by fabric resources. A memory buffer must be registered before it can be used as the target of a remote RMA or atomic data transfer. Additionally, a fabric provider may require that data buffers be registered before being used in local transfers.  The latter is necessary to ensure that the virtual to physical page mappings do not change.

Although there are a few different attributes that apply to memory registration, OFI groups those attributes into one of two different modes (for application simplicity).

RMA 和原子操作都可以读取和写入由对等进程拥有的内存，并且都不需要目标处理器的参与。 因为可以通过网络修改内存，所以应用程序必须选择将其内存暴露给对等方。 这是由内存注册过程处理的。 已注册的内存区域将内存缓冲区与授予结构资源访问权限相关联。 必须先注册内存缓冲区，然后才能将其用作远程 RMA 或原子数据传输的目标。 此外，结构提供者可能要求在用于本地传输之前注册数据缓冲区。 后者对于确保虚拟到物理页面的映射不会改变是必要的。

尽管有一些不同的属性适用于内存注册，但 OFI 将这些属性分组为两种不同模式之一（为了应用程序的简单性）。

### Basic Memory Registration Mode

Basic memory registration mode is defined around supporting the InfiniBand, RoCE, and iWarp architectures, which maps well to a wide variety of RMA capable hardware.  In basic mode, registration occurs on allocated memory buffers, and the MR attributes are selected by the provider.  The application must only register allocated memory, and the protection keys that are used to access the memory are assigned by the provider.  The impact of using basic registration is that the application must inform any peer that wishes to access the region the local virtual address of the memory buffer, along with the key to use when accessing it.  Peers must provide both the key and the target's virtual address as part of the RMA operation.

Although not part of the basic memory registration mode definition, hardware that supports this mode frequently requires that all data buffers used for network communication also be registered.  This includes buffers posted to send or receive messages, _source_ RMA and atomic buffers, and tagged message buffers.  This restriction is indicated using the FI_LOCAL_MR mode bit.  This restriction is needed to ensure that the virtual to physical address mappings do not change between a request being submitted and the hardware processing it.

基本内存注册模式是围绕支持 InfiniBand、RoCE 和 iWarp 架构定义的，这些架构很好地映射到各种支持 RMA 的硬件。在基本模式下，注册发生在分配的内存缓冲区上，MR 属性由提供者选择。应用程序必须只注册分配的内存，并且用于访问内存的保护密钥由提供程序分配。使用基本注册的影响是应用程序必须通知任何希望访问该区域的对等方内存缓冲区的本地虚拟地址，以及访问它时使用的密钥。对等方必须同时提供密钥和目标的虚拟地址作为 RMA 操作的一部分。

虽然不是基本内存注册模式定义的一部分，但支持此模式的硬件经常要求也注册用于网络通信的所有数据缓冲区。这包括为发送或接收消息而发布的缓冲区、_source_ RMA 和原子缓冲区以及标记的消息缓冲区。此限制使用 FI_LOCAL_MR 模式位指示。需要此限制以确保虚拟到物理地址的映射不会在提交的请求和处理它的硬件之间发生变化。

### Scalable Memory Registration Mode

Scalable memory registration targets highly parallel, high-performance applications.  Such applications often have an additional level of security that allows the peers to operate in a more trusted environment where memory registration is employed.  In scalable mode, registration occurs on memory address ranges, and the MR attributes are selected by the user. There are two notable differences with scalable mode.

First is that the address ranges do not need to map to allocated memory buffers at the time the registration call is made.  (Virtual memory must back the ranges before they are accessed as part of any data transfer operation.)  This allows, for example, for an application to expose all or a significant portion of its address space to peers.  When combined with a symmetric memory allocator, this feature can eliminate a process from needing to store the target addresses of its peers.  Second, the application selects the protection key for the region.  Target addresses and keys can be hard-coded or determined algorithmically, reducing the memory footprint and avoiding network traffic associated with registration.

可扩展内存注册针对高度并行、高性能的应用程序。此类应用程序通常具有额外的安全级别，允许对等方在使用内存注册的更受信任的环境中运行。在可扩展模式下，注册发生在内存地址范围内，MR 属性由用户选择。可扩展模式有两个显着差异。

首先是地址范围不需要在注册调用时映射到分配的内存缓冲区。 （在作为任何数据传输操作的一部分访问范围之前，虚拟内存必须支持这些范围。）这允许，例如，应用程序将其地址空间的全部或大部分公开给对等方。当与对称内存分配器结合使用时，此功能可以消除进程需要存储其对等方的目标地址。其次，应用程序选择区域的保护密钥。目标地址和密钥可以硬编码或通过算法确定，从而减少内存占用并避免与注册相关的网络流量。

### Memory Region APIs

The following APIs highlight how to allocate and access a registered memory region.  Note that this is not a complete list of memory region (MR) calls, and for full details on each API, readers should refer directly to the man pages.

以下 API 重点介绍了如何分配和访问已注册的内存区域。 请注意，这不是内存区域 (MR) 调用的完整列表，有关每个 API 的完整详细信息，读者应直接参考手册页。

```
int fi_mr_reg(struct fid_domain *domain, const void *buf, size_t len,
    uint64_t access, uint64_t offset, uint64_t requested_key, uint64_t flags,
    struct fid_mr **mr, void *context);

void * fi_mr_desc(struct fid_mr *mr);
uint64_t fi_mr_key(struct fid_mr *mr);
```

By default, memory regions are associated with a domain.  A MR is accessible by any endpoint that is opened on that domain.  A region starts at the address specified by 'buf', and is 'len' bytes long.  The 'access' parameter are permission flags that are OR'ed together.  The permissions indicate which type of operations may be invoked against the region (e.g. FI_READ, FI_WRITE, FI_REMOTE_READ, FI_REMOTE_WRITE).  The 'buf' parameter must point to allocated virtual memory when using basic registration mode.

If scalable registration is used, the application can specify the desired MR key through the 'requested_key' parameter.  The 'offset' and 'flags' parameters are not used and reserved for future use.

A MR is associated with local and remote protection keys.  The local key is referred to as a memory descriptor and may be retrieved by calling fi_mr_desc().  This call is only needed if the FI_LOCAL_MR mode bit has been set.  The memory descriptor is passed directly into data transfer operations, for example:

默认情况下，内存区域与域相关联。在该域上打开的任何端点都可以访问 MR。区域从“buf”指定的地址开始，长度为“len”字节。 'access' 参数是 OR'ed 在一起的权限标志。权限指示可以针对区域调用哪种类型的操作（例如 FI_READ、FI_WRITE、FI_REMOTE_READ、FI_REMOTE_WRITE）。使用基本注册模式时，“buf”参数必须指向分配的虚拟内存。

如果使用可扩展注册，应用程序可以通过“requested_key”参数指定所需的 MR 密钥。 'offset' 和 'flags' 参数未使用，保留供将来使用。

MR 与本地和远程保护密钥相关联。本地密钥称为内存描述符，可以通过调用 fi_mr_desc() 来检索。仅当已设置 FI_LOCAL_MR 模式位时才需要此调用。内存描述符直接传递给数据传输操作，例如：

```
/* fi_mr_desc() example using fi_send() */
fi_send(ep, buf, len, fi_mr_desc(mr), 0, NULL);
```

The remote key, or simply MR key, is used by the peer when targeting the MR with an RMA or atomic operation. If scalable registration is used, the MR key will be the same as the 'requested_key'.  Otherwise, it is a provider selected value.  The key must be known to the peer.  If basic registration is used, this means that the key will need to be sent in a separate message to the initiating peer.  (Some applications exchange the key as part of connection setup).

The API is designed to handle MR keys that are at most 64-bits long.  The size of the actual key is reported as a domain attribute.  Typical sizes are either 32 or 64 bits, depending on the underlying fabric.  Support for keys larger than 64-bits is possible but requires using extended calls not discussed here.

当使用 RMA 或原子操作瞄准 MR 时，对等方使用远程密钥或简称为 MR 密钥。 如果使用可扩展注册，则 MR 密钥将与“requested_key”相同。 否则，它是提供者选择的值。 对等方必须知道密钥。 如果使用基本注册，这意味着需要在单独的消息中将密钥发送给发起对等方。 （一些应用程序交换密钥作为连接设置的一部分）。

该 API 旨在处理最多 64 位长的 MR 密钥。 实际密钥的大小被报告为域属性。 典型大小为 32 位或 64 位，具体取决于底层结构。 支持大于 64 位的密钥是可能的，但需要使用此处未讨论的扩展调用。

# Endpoints 端点

https://ofiwg.github.io/libfabric/v1.14.1/man/fi_endpoint.3.html

Endpoints are transport level communication portals. Opening an endpoint is trivial after calling fi_getinfo(), however, there are different open calls, depending on the type of endpoint to allocate.  There are separate calls to open active, passive, and scalable endpoints.

端点是传输级通信门户。 在调用 fi_getinfo() 后打开端点是微不足道的，但是，根据要分配的端点的类型，有不同的打开调用。 对打开主动、被动和可扩展端点有单独的调用。

## Active

Active endpoints may be connection-oriented or connection-less.  The data transfer interfaces – messages (fi_msg), tagged messages (fi_tagged), RMA (fi_rma), and atomics (fi_atomic) – are associated with active endpoints. In basic configurations, an active endpoint has transmit and receive queues. In general, operations that generate traffic on the fabric are posted to the transmit queue. This includes all RMA and atomic operations, along with sent messages and sent tagged messages. Operations that post buffers for receiving incoming data are submitted to the receive queue.

Active endpoints are created in the disabled state. They must transition into an enabled state before accepting data transfer operations, including posting of receive buffers. The fi_enable call is used to transition an active endpoint into an enabled state. The fi_connect and fi_accept calls will also transition an endpoint into the enabled state, if it is not already enabled.  An endpoint may immediately be allocated after opening a domain, using the same fi_info structure that was returned from fi_getinfo().

活动端点可能是面向连接的或无连接的。数据传输接口——消息（fi_msg）、标记消息（fi_tagged）、RMA（fi_rma）和原子（fi_atomic）——与活动端点相关联。在基本配置中，**活动端点具有发送和接收队列**。通常，在结构上生成流量的操作会发布到传输队列。这包括所有 RMA 和原子操作，以及发送的消息和发送的标记消息。为接收传入数据而发布缓冲区的操作将提交到接收队列。

活动端点是在禁用状态下创建的。在接受数据传输操作（包括接收缓冲区的发布）之前，它们必须转换为启用状态。 fi_enable 调用用于将活动端点转换为启用状态。 fi_connect 和 fi_accept 调用还将端点转换为启用状态（如果尚未启用）。端点可以在打开域后立即分配，使用从 fi_getinfo() 返回的相同 fi_info 结构。


```
int fi_endpoint(struct fid_domain *domain, struct fi_info *info,
    struct fid_ep **ep, void *context);
```

### Enabling

In order to transition an endpoint into an enabled state, it must be bound to one or more fabric resources. An endpoint that will generate asynchronous completions, either through data transfer operations or communication establishment events, must be bound to appropriate completion queues or event queues, respectively, before being enabled. Unconnected endpoints must be bound to an address vector.

为了将端点转换为启用状态，它必须绑定到一个或多个结构资源。 将通过数据传输操作或通信建立事件生成异步完成的端点必须在启用之前分别绑定到适当的完成队列或事件队列。 无连接的端点必须绑定到地址向量。

```
/* Example to enable an unconnected endpoint */

/* Allocate an address vector and associated it with the endpoint */
fi_av_open(domain, &av_attr, &av, NULL);
fi_ep_bind(ep, &av->fid, 0);

/* Allocate and associate completion queues with the endpoint */
fi_cq_open(domain, &tx_cq_attr, &tx_cq, NULL);
fi_ep_bind(ep, &tx_cq->fid, FI_TRANSMIT);

fi_cq_open(domain, &rx_cq_attr, &rx_cq, NULL);
fi_ep_bind(ep, &rx_cq->fid, FI_RECV);

fi_enable(ep);
```

In the above example, we allocate an address vector and transmit and receive completion queues.  The attributes for the address vector and completion queue are omitted (additional discussion below).  Those are then associated with the endpoint through the fi_ep_bind() call.  After all necessary resources have been assigned to the endpoint, we enable it.  Enabling the endpoint indicates to the provider that it should allocate any hardware and software resources and complete the initialization for the endpoint.

The fi_enable() call is always called for unconnected endpoints.  Connected endpoints may be able to skip calling fi_enable(), since fi_connect() and fi_accept() will enable the endpoint automatically.  However, applications may still call fi_enable() prior to calling fi_connect() or fi_accept().  Doing so allows the application to post receive buffers to the endpoint, which ensures that they are available to receive data in the case where the peer endpoint sends messages immediately after it establishes the connection.

在上面的例子中，我们分配了一个地址向量和发送和接收完成队列。地址向量和完成队列的属性被省略（下面有更多讨论）。然后通过 fi_ep_bind() 调用将它们与端点相关联。在将所有必要的资源分配给端点之后，我们启用它。启用端点向提供者表明它应该分配任何硬件和软件资源并完成端点的初始化。

始终为未连接的端点调用 fi_enable() 调用。连接的端点可以跳过调用 fi_enable()，因为 fi_connect() 和 fi_accept() 将自动启用端点。但是，应用程序仍然可以在调用 fi_connect() 或 fi_accept() 之前调用 fi_enable()。这样做允许应用程序将接收缓冲区发布到端点，从而确保在对等端点在建立连接后立即发送消息的情况下它们可以接收数据。

## Passive

Passive endpoints are used to listen for incoming connection requests.  Passive endpoints are of type FI_EP_MSG, and may not perform any data transfers.  An application wishing to create a passive endpoint typically calls fi_getinfo() using the FI_SOURCE flag, often only specifying a 'service' address.  The service address corresponds to a TCP port number.

Passive endpoints are associated with event queues.  Event queues report connection requests from peers.  Unlike active endpoints, passive endpoints are not associated with a domain.  This allows an application to listen for connection requests across multiple domains.

被动端点用于侦听传入的连接请求。 被动端点的类型为 FI_EP_MSG，并且可能不执行任何数据传输。 希望创建被动端点的应用程序通常使用 FI_SOURCE 标志调用 fi_getinfo()，通常只指定“服务”地址。 服务地址对应一个 TCP 端口号。

被动端点与事件队列相关联。 事件队列报告来自对等方的连接请求。 与主动端点不同，被动端点不与域相关联。 这允许应用程序跨多个域侦听连接请求。

```
/* Example passive endpoint listen */
fi_passive_ep(fabric, info, &pep, NULL);

fi_eq_open(fabric, &eq_attr, &eq, NULL);
fi_pep_bind(pep, &eq->fid, 0);

fi_listen(pep);
```

A passive endpoint must be bound to an event queue before calling listen.  This ensures that connection requests can be reported to the application.  To accept new connections, the application waits for a request, allocates a new active endpoint for it, and accepts the request.

在调用listen 之前，被动端点必须绑定到事件队列。 这确保可以将连接请求报告给应用程序。 为了接受新连接，应用程序等待请求，为其分配一个新的活动端点，然后接受请求。

```
/* Example accepting a new connection */

/* Wait for a CONNREQ event */
fi_eq_sread(eq, &event, &cm_entry, sizeof cm_entry, -1, 0);
assert(event == FI_CONNREQ);

/* Allocate an new endpoint for the connection */
if (!cm_entry.info->domain_attr->domain)
    fi_domain(fabric, cm_entry.info, &domain, NULL);
fi_endpoint(domain, cm_entry.info, &ep, NULL);

/* See the resource binding section below for details on associated fabric objects */
fi_ep_bind(ep, &eq->fid, 0);
fi_cq_open(domain, &tx_cq_attr, &tx_cq, NULL);
fi_ep_bind(ep, &tx_cq->fid, FI_TRANSMIT);
fi_cq_open(domain, &rx_cq_attr, &rx_cq, NULL);
fi_ep_bind(ep, &rx_cq->fid, FI_RECV);

fi_enable(ep);
fi_recv(ep, rx_buf, len, NULL, 0, NULL);

fi_accept(ep, NULL, 0);
fi_eq_sread(eq, &event, &cm_entry, sizeof cm_entry, -1, 0);
assert(event == FI_CONNECTED);
```

The connection request event (FI_CONNREQ) includes information about the type of endpoint to allocate, including default attributes to use.  If a domain has not already been opened for the endpoint, one must be opened.  Then the endpoint and related resources can be allocated.  Unlike the unconnected endpoint example above, a connected endpoint does not have an AV, but does need to be bound to an event queue.  In this case, we use the same EQ as the listening endpoint.  Once the other EP resources (e.g. CQs) have been allocated and bound, the EP can be enabled.

To accept the connection, the application calls fi_accept().  Note that because of thread synchronization issues, it is possible for the active endpoint to receive data even before fi_accept() can return.  The posting of receive buffers prior to calling fi_accept() handles this condition, which avoids network flow control issues occurring immediately after connecting.

The fi_eq_sread() calls are blocking (synchronous) read calls to the event queue.  These calls wait until an event occurs, which in this case are connection request and establishment events.

连接请求事件 (FI_CONNREQ) 包括有关要分配的端点类型的信息，包括要使用的默认属性。如果尚未为端点打开域，则必须打开一个域。然后可以分配端点和相关资源。与上面未连接的端点示例不同，已连接的端点没有 AV，但需要绑定到事件队列。在这种情况下，我们使用与监听端点相同的 EQ。一旦其他 EP 资源（例如 CQ）已经分配和绑定，EP 就可以启用。

为了接受连接，应用程序调用 fi_accept()。请注意，由于线程同步问题，活动端点甚至可能在 fi_accept() 返回之前接收数据。在调用 fi_accept() 之前发布接收缓冲区可以处理这种情况，从而避免在连接后立即发生网络流量控制问题。

fi_eq_sread() 调用是对事件队列的阻塞（同步）读取调用。这些调用一直等到事件发生，在这种情况下是连接请求和建立事件。

## Scalable 弹性(与SEP有关)

For most applications, an endpoint consists of a transmit and receive context associated with a single address.  The transmit and receive contexts often map to hardware command queues.  For multi-threaded applications, access to these hardware queues requires serialization, which can lead to them becoming bottlenecks.  Scalable endpoints were created to address this.

A scalable endpoint is an endpoint that has multiple transmit and/or receive contexts associated with it.  As an example, consider an application that allocates a total of four processing threads.  By assigning each thread its own transmit context, the application can avoid serializing (i.e. locking) access to hardware queues.

The advantage of using a scalable endpoint over allocating multiple traditional endpoints is reduced addressing footprint.  A scalable endpoint has a single address, regardless of how many transmit or receive contexts it may have.

Support for scalable endpoints is provider specific, with support indicated by the domain attributes:

对于大多数应用程序，端点由与单个地址关联的发送和接收上下文组成。发送和接收上下文通常映射到硬件命令队列。对于多线程应用程序，访问这些硬件队列需要序列化，这可能导致它们成为瓶颈。创建了可扩展的端点来解决这个问题。

可扩展端点是具有与其关联的多个发送和/或接收上下文的端点。例如，考虑一个总共分配四个处理线程的应用程序。通过为每个线程分配自己的传输上下文，应用程序可以避免对硬件队列的序列化（即锁定）访问。

与分配多个传统端点相比，使用可扩展端点的优势在于减少了寻址空间。一个可扩展的端点有一个地址，不管它可能有多少发送或接收上下文。

对可扩展端点的支持是特定于提供者的，支持由域属性指示：

```
struct fi_domain_attr {
    ...
    size_t max_ep_tx_ctx;
    size_t max_ep_rx_ctx;
    ...
```

The above fields indicates the maximum number of transmit and receive contexts, respectively, that may be associated with a single endpoint.  One or both of these values will be greater than one if scalable endpoints are supported.  Applications can configure and allocate a scalable endpoint using the fi_scalable_ep call:

上述字段分别指示可能与单个端点相关联的发送和接收上下文的最大数量。 如果支持可扩展端点，则这些值中的一个或两个都将大于 1。 应用程序可以使用 fi_scalable_ep 调用配置和分配可扩展端点：

```
/* Set the required number of transmit of receive contexts
 * These must be <= the domain maximums listed above.
 * This will usually be set prior to calling fi_getinfo
 */
struct fi_info *hints, *info;
struct fid_domain *domain;
struct fid_ep *scalable_ep, *tx_ctx[4], *rx_ctx[2];

hints = fi_allocinfo();
...

/* A scalable endpoint requires > 1 Tx or Rx queue */
hints->ep_attr->tx_ctx_cnt = 4;
hints->ep_attr->rx_ctx_cnt = 2;

/* Call fi_getinfo and open fabric, domain, etc. */
...

fi_scalable_ep(domain, info, &sep, NULL);
```

The above example opens an endpoint with four transmit and two receive contexts.  However, a scalable endpoint only needs to be scalable in one dimension -- transmit or receive.  For example, it could use multiple transmit contexts, but only require a single receive context.  It could even use a shared context, if desired.

Submitting data transfer operations to a scalable endpoint is more involved.  First, if the endpoint only has a single transmit context, then all transmit operations are posted directly to the scalable endpoint, the same as if a traditional endpoint were used.  Likewise, if the endpoint only has a single receive context, then all receive operations are posted directly to the scalable endpoint.  An additional step is needed before posting operations to one of many contexts, that is, the 'scalable' portion of the endpoint.  The desired context must first be retrieved:

上面的示例打开了一个具有四个发送和两个接收上下文的端点。 然而，一个可扩展的端点只需要在一个维度上是可扩展的——发送或接收。 例如，它可以使用多个传输上下文，但只需要一个接收上下文。 如果需要，它甚至可以使用共享上下文。

将数据传输操作提交到可扩展的端点更为复杂。 首先，如果端点只有一个传输上下文，那么所有传输操作都直接发布到可扩展端点，就像使用传统端点一样。 同样，如果端点只有一个接收上下文，则所有接收操作都直接发布到可扩展端点。 在将操作发布到许多上下文之一之前，需要一个额外的步骤，即端点的“可扩展”部分。 必须首先检索所需的上下文：

```
/* Retrieve the first (index 0) transmit and receive contexts */
fi_tx_context(scalable_ep, 0, info->tx_attr, &tx_ctx[0], &tx_ctx[0]);
fi_rx_context(scalable_ep, 0, info->rx_attr, &rx_ctx[0], &rx_ctx[0]);
```

Data transfer operations are then posted to the tx_ctx or rx_ctx.  It should be noted that although the scalable endpoint, transmit context, and receive context are all of type fid_ep, attempting to submit a data transfer operation against the wrong object will result in an error.

By default all transmit and receive contexts belonging to a scalable endpoint are similar with respect to other transmit and receive contexts.  However, applications can request that a context have fewer capabilities than what was requested for the scalable endpoint.  This allows the provider to configure its hardware resources for optimal performance.  For example, suppose a scalable endpoint has been configured for tagged message and RMA support.  An application can open a transmit context with only tagged message support, and another context with only RMA support.

然后将数据传输操作发布到 tx_ctx 或 rx_ctx。 需要注意的是，虽然可扩展端点、传输上下文和接收上下文都是 fid_ep 类型，但尝试针对错误对象提交数据传输操作会导致错误。

默认情况下，属于可扩展端点的所有发送和接收上下文与其他发送和接收上下文相似。 但是，应用程序可以请求上下文具有比可扩展端点请求的更少的功能。 这允许提供商配置其硬件资源以获得最佳性能。 例如，假设已为标记消息和 RMA 支持配置了可扩展端点。 应用程序可以打开仅支持标记消息的传输上下文，以及仅支持 RMA 的另一个上下文。

## Resource Bindings

Before an endpoint can be used for data transfers, it must be associated with other resources, such as completion queues, counters, address vectors, or event queues. Resource bindings must be done prior to enabling an endpoint.  All active endpoints must be bound to completion queues.  Unconnected endpoints must be associated with an address vector.  Passive and connection-oriented endpoints must be bound to an event queue.  The resource binding requirements are cumulative: for example, an RDM endpoint must be bound to completion queues and address vectors.

As shown in previous examples, resources are associated with endpoints using a bind operation:

在端点可以用于数据传输之前，它必须与其他资源相关联，例如完成队列、计数器、地址向量或事件队列。 资源绑定必须在启用端点之前完成。 所有活动端点必须绑定到完成队列。 未连接的端点必须与地址向量相关联。 被动和面向连接的端点必须绑定到事件队列。 资源绑定要求是累积的：例如，RDM 端点必须绑定到完成队列和地址向量。

如前面的示例所示，资源使用绑定操作与端点相关联：

```
int fi_ep_bind(struct fid_ep *ep, struct fid *fid, uint64_t flags);
int fi_scalable_ep_bind(struct fid_ep *sep, struct fid *fid, uint64_t flags);
int fi_pep_bind(struct fid_pep *pep, struct fid *fid, uint64_t flags);
```

The bind functions are similar to each other (and map to the same fi_bind call internally).  Flags are used to indicate how the resources should be associated.  The passive endpoint section above shows an example of binding passive and active endpoints to event and completion queues.

绑定函数彼此相似（并在内部映射到相同的 fi_bind 调用）。 标志用于指示资源应如何关联。 上面的被动端点部分显示了将被动和主动端点绑定到事件和完成队列的示例。

## EP Attributes

The properties of an endpoint are specified using endpoint attributes.  These may be set as hints passed into the fi_getinfo call.  Unset values will be filled out by the provider.

端点的属性是使用端点属性指定的。 这些可以设置为传递给 fi_getinfo 调用的提示。 未设置的值将由提供者填写。

```
struct fi_ep_attr {
    enum fi_ep_type type;
    uint32_t        protocol;
    uint32_t        protocol_version;
    size_t          max_msg_size;
    size_t          msg_prefix_size;
    size_t          max_order_raw_size;
    size_t          max_order_war_size;
    size_t          max_order_waw_size;
    uint64_t        mem_tag_format;
    size_t          tx_ctx_cnt;
    size_t          rx_ctx_cnt;
};
```

A full description of each field is available in the libfabric man pages, with selected details listed below.

libfabric 手册页中提供了每个字段的完整描述，下面列出了选定的详细信息。

### Endpoint Type

This indicates the type of endpoint: reliable datagram (FI_EP_RDM), reliable-connected (FI_EP_MSG), or unreliable datagram (DGRAM).  Nearly all applications will need to specify the endpoint type as a hint passed into fi_getinfo, as most applications will only be coded to support a single endpoint type.

这表示端点的类型：可靠数据报rdm (FI_EP_RDM) mercury(hg)默认使用的端点类型为可靠数据报、**可靠连接 (FI_EP_MSG)** 或不可靠数据报 (DGRAM)。 几乎所有应用程序都需要将端点类型指定为传递给 fi_getinfo 的提示，因为大多数应用程序只会被编码为支持单一端点类型。

### Maximum Message Size

This size is the maximum size for any data transfer operation that goes over the endpoint. For unreliable datagram endpoints, this is often the MTU of the underlying network. For reliable endpoints, this value is often a restriction of the underlying transport protocol. Applications that require transfers larger than the maximum reported size are required to break up a single, large transfer into multiple operations.

Providers expose their hardware or network limits to the applications, rather than segmenting large transfers internally, in order to minimize completion overhead. For example, for a provider to support large message segmentation internally, it would need to emulate all completion mechanisms (queues and counters) in software, even if transfers that are larger than the transport supported maximum were never used.

此大小是通过端点的任何数据传输操作的最大大小。 对于不可靠的数据报端点，这通常是底层网络的 MTU。 对于可靠的端点，这个值通常是底层传输协议的限制。 需要传输大于最大报告大小的应用程序需要将单个大传输分解为多个操作。

提供者将他们的硬件或网络限制暴露给应用程序，而不是在内部分割大量传输，以最大限度地减少完成开销。 例如，对于要在内部支持大型消息分段的提供程序，它需要在软件中模拟所有完成机制（队列和计数器），即使从未使用过大于传输支持的最大值的传输。

### Message Order Size

This field specifies data ordering. It defines the delivery order of transport data into target memory for RMA and atomic operations. Data ordering requires message ordering.

For example, suppose that an application issues two RMA write operations to the same target memory location. (The application may be writing a time stamp value every time a local condition is met, for instance).  Message ordering indicates that the first write as initiated by the sender is the first write processed by the receiver. Data ordering indicates whether the _data_ from the first write updates memory before the second write updates memory.

The max_order_xxx_size fields indicate how large a message may be while still achieving data ordering. If a field is 0, then no data ordering is guaranteed. If a field is the same as the max_msg_size, then data order is guaranteed for all messages.

It is common for providers to support data ordering up to max_msg_size for back to back operations that are the same. For example, an RMA write followed by an RMA write may have data ordering regardless of the size of the data transfer (max_order_waw_size = max_msg_size). Mixed operations, such as a read followed by a write, are often more restricted. This is because RMA read operations may require acknowledgments from the _initiator_, which impacts the re-transmission protocol.

For example, consider an RMA read followed by a write. The target will process the read request, retrieve the data, and send a reply. While that is occurring, a write is received that wants to update the same memory location accessed by the read. If the target processes the write, it will overwrite the memory used by the read. If the read response is lost, and the read is retried, the target will be unable to re-send the data. To handle this, the target either needs to: defer handling the write until it receives an acknowledgment for the read response, buffer the read response so it can be re-transmitted, or indicate that data ordering is not guaranteed.

Because the read or write operation may be gigabytes in size, deferring the write may add significant latency, and buffering the read response may be impractical. The max_order_xxx_size fields indicate how large back to back operations may be with ordering still maintained. In many cases, read after write and write and read ordering may be significantly limited, but still usable for implementing specific algorithms, such as a global locking mechanism.

### 消息排序大小

此字段指定数据排序。它为 RMA 和原子操作定义了传输数据到目标内存的传递顺序。数据排序需要消息排序。

例如，假设一个应用程序向同一个目标内存位置发出两个 RMA 写操作。 （例如，应用程序可能会在每次满足本地条件时写入时间戳值）。消息排序表明发送方发起的第一个写入是接收方处理的第一个写入。数据排序指示第一次写入的 _data_ 是否在第二次写入更新内存之前更新内存。

max_order_xxx_size 字段指示在仍实现数据排序的同时消息可能有多大。如果字段为 0，则不保证数据排序。如果某个字段与 max_msg_size 相同，则保证所有消息的数据顺序。

对于相同的背靠背操作，提供者通常支持最大为 max_msg_size 的数据排序。例如，RMA 写入后跟 RMA 写入可能具有数据排序，而与数据传输的大小无关 (max_order_waw_size = max_msg_size)。混合操作，例如先读后写，通常受到更多限制。这是因为 RMA 读取操作可能需要来自 _initiator_ 的确认，这会影响重新传输协议。

例如，考虑 RMA 读取，然后是写入。目标将处理读取请求、检索数据并发送回复。在发生这种情况时，会收到一个写入，该写入想要更新读取访问的相同内存位置。如果目标处理写入，它将覆盖读取使用的内存。如果读取响应丢失，并且重新尝试读取，则目标将无法重新发送数据。为了处理这个问题，目标要么需要：推迟处理写入，直到它收到对读取响应的确认，缓冲读取响应以便可以重新传输，或者表明数据顺序不能保证。

因为读取或写入操作的大小可能是千兆字节，所以延迟写入可能会增加显着的延迟，并且缓冲读取响应可能是不切实际的。 max_order_xxx_size 字段指示在仍保持排序的情况下背靠背操作可能有多大。在许多情况下，写后读以及写和读顺序可能会受到很大限制，但仍可用于实现特定算法，例如全局锁定机制。

## Rx/Tx Context Attributes

The endpoint attributes define the overall abilities for the endpoint; however, attributes that apply specifically to receive or transmit contexts are defined by struct fi_rx_attr and fi_tx_attr, respectively:

端点属性定义了端点的整体能力； 但是，专门应用于接收或传输上下文的属性分别由 struct fi_rx_attr 和 fi_tx_attr 定义：

```
struct fi_rx_attr {
    uint64_t caps;
    uint64_t mode;
    uint64_t op_flags;
    uint64_t msg_order;
    uint64_t comp_order;
    size_t total_buffered_recv;
    size_t size;
    size_t iov_limit;
};

struct fi_tx_attr {
    uint64_t caps;
    uint64_t mode;
    uint64_t op_flags;
    uint64_t msg_order;
    uint64_t comp_order;
    size_t inject_size;
    size_t size;
    size_t iov_limit;
    size_t rma_iov_limit;
};
```

Context capabilities must be a subset of the endpoint capabilities. For many applications, the default attributes returned by the provider will be sufficient, with the application only needing to specify endpoint attributes.

Both context attributes include an op_flags field. This field is used by applications to specify the default operation flags to use with any call. For example, by setting the transmit context’s op_flags to FI_INJECT, the application has indicated to the provider that all transmit operations should assume ‘inject’ behavior is desired. (I.e. the buffer provided to the call must be returned to the application upon return from the function). The op_flags applies to all operations that do not provide flags as part of the call (e.g. fi_sendmsg).  A common use of op_flags is to specify the default completion semantic desired (discussed next) by the application.

It should be noted that some attributes are dependent upon the peer endpoint having supporting attributes in order to achieve correct application behavior. For example, message order must be the compatible between the initiator’s transmit attributes and the target’s receive attributes. Any mismatch may result in incorrect behavior that could be difficult to debug.

上下文能力必须是端点能力的子集。对于许多应用程序，提供者返回的默认属性就足够了，应用程序只需要指定端点属性。

两个上下文属性都包含一个 op_flags 字段。应用程序使用此字段来指定用于任何调用的默认操作标志。例如，通过将传输上下文的 op_flags 设置为 FI_INJECT，应用程序已向提供者指示所有传输操作都应假定需要“注入”行为。 （即，提供给调用的缓冲区必须在从函数返回时返回给应用程序）。 op_flags 适用于在调用中不提供标志的所有操作（例如 fi_sendmsg）。 op_flags 的一个常见用途是指定应用程序所需的默认完成语义（接下来讨论）。

应该注意的是，一些属性依赖于具有支持属性的对等端点，以实现正确的应用程序行为。例如，消息顺序必须在发起者的发送属性和目标的接收属性之间兼容。任何不匹配都可能导致难以调试的错误行为。

# Completions 完成

Data transfer operations complete asynchronously. Libfabric defines two mechanism by which an application can be notified that an operation has completed: completion queues and counters.

Regardless of which mechanism is used to notify the application that an operation is done, developers must be aware of what a completion indicates.

In all cases, a completion indicates that it is safe to reuse the buffer(s) associated with the data transfer. This completion mode is referred to as inject complete and corresponds to the operational flags FI_INJECT_COMPLETE. However, a completion may also guarantee stronger semantics.

Although libfabric does not define an implementation, a provider can meet the requirement for inject complete by copying the application’s buffer into a network buffer before generating the completion. Even if the transmit operation is lost and must be retried, the provider can resend the original data from the copied location. For large transfers, a provider may not mark a request as inject complete until the data has been acknowledged by the target. Applications, however, should only infer that it is safe to reuse their data buffer for an inject complete operation.

Transmit complete is a completion mode that provides slightly stronger guarantees to the application. The meaning of transmit complete depends on whether the endpoint is reliable or unreliable. For an unreliable endpoint (FI_EP_DGRAM), a transmit completion indicates that the request has been delivered to the network. That is, the message has left the local NIC. For reliable endpoints, a transmit complete occurs when the request has reached the target endpoint. Typically, this indicates that the target has acked the request. Transmit complete maps to the operation flag FI_TRANSMIT_COMPLETE.

A third completion mode is defined to provide guarantees beyond transmit complete. With transmit complete, an application knows that the message is no longer dependent on the local NIC or network (e.g. switches). However, the data may be buffered at the remote NIC and has not necessarily been written to the target memory. As a result, data sent in the request may not be visible to all processes. The third completion mode is delivery complete.

Delivery complete indicates that the results of the operation are available to all processes on the fabric. The distinction between transmit and delivery complete is subtle, but important. It often deals with _when_ the target endpoint generates an acknowledgment to a message. For providers that offload transport protocol to the NIC, support for transmit complete is common. Delivery complete guarantees are more easily met by providers that implement portions of their protocol on the host processor. Delivery complete corresponds to the FI_DELIVERY_COMPLETE operation flag.

Applications can request a default completion mode when opening an endpoint by setting one of the above mentioned complete flags as an op_flags for the context’s attributes. However, it is usually recommended that application use the provider’s default flags for best performance, and amend its protocol to achieve its completion semantics. For example, many applications will perform a ‘finalize’ or ‘commit’ procedure as part of their operation, which synchronizes the processing of all peers and guarantees that all previously sent data has been received.

数据传输操作异步完成。 Libfabric 定义了两种机制，通过它可以通知应用程序操作已完成：完成队列和计数器。

无论使用哪种机制来通知应用程序操作已完成，开发人员都必须了解完成指示的内容。

在所有情况下，完成都表明重用与数据传输相关的缓冲区是安全的。这种完成模式称为注入完成，对应于操作标志 FI_INJECT_COMPLETE。然而，补全也可以保证更强的语义。

尽管 libfabric 没有定义实现，但提供者可以通过在生成完成之前将应用程序的缓冲区复制到网络缓冲区来满足注入完成的要求。即使传输操作丢失并且必须重试，提供者也可以从复制的位置重新发送原始数据。对于大型传输，在目标确认数据之前，提供者可能不会将请求标记为注入完成。然而，应用程序应该只推断重用它们的数据缓冲区来完成注入操作是安全的。

传输完成是一种完成模式，它为应用程序提供了稍微更强的保证。传输完成的含义取决于端点是可靠的还是不可靠的。对于不可靠的端点 (FI_EP_DGRAM)，传输完成指示请求已被传递到网络。即消息已离开本地网卡。对于可靠端点，当请求到达目标端点时会发生传输完成。通常，这表明目标已确认请求。传输完成映射到操作标志 FI_TRANSMIT_COMPLETE。

定义了第三种完成模式以提供传输完成之外的保证。传输完成后，应用程序知道消息不再依赖于本地 NIC 或网络（例如交换机）。但是，数据可能会在远程 NIC 处缓冲，并且不一定已写入目标内存。因此，请求中发送的数据可能对所有进程都不可见。第三种完成模式是交付完成。

交付完成表示操作的结果可用于结构上的所有进程。传输和交付完成之间的区别是微妙的，但很重要。它通常处理_何时_目标端点生成对消息的确认。对于将传输协议卸载到 NIC 的提供商，支持传输完成是常见的。在主机处理器上实现部分协议的提供商更容易满足交付完全保证。交付完成对应于 FI_DELIVERY_COMPLETE 操作标志。

通过将上述完成标志之一设置为上下文属性的 op_flags，应用程序可以在打开端点时请求默认完成模式。但是，通常建议应用程序使用提供者的默认标志以获得最佳性能，并修改其协议以实现其完成语义。例如，许多应用程序将执行“finalize”或“commit”过程作为其操作的一部分，这会同步所有对等点的处理并保证已接收到所有先前发送的数据。

## CQs 完成队列

Completion queues often map directly to provider hardware mechanisms, and libfabric is designed around minimizing the software impact of accessing those mechanisms. Unlike other objects discussed so far (fabrics, domains, endpoints), completion queues are not part of the fi_info structure or involved with the fi_getinfo() call.

All active endpoints must be bound with one or more completion queues. This is true even if completions will be suppressed by the application (e.g. using the FI_SELECTIVE_COMPLETION flag). Completion queues are needed to report operations that complete in error.

Transmit and receive contexts are each associated with their own completion queue. An endpoint may direct transmit and receive completions to separate CQs or to the same CQ. For applications, using a single CQ reduces system resource utilization. While separating completions to different CQs could simplify code maintenance or improve multi-threading execution. A CQ may be shared among multiple endpoints.

CQs are allocated separately from endpoints and are associated with endpoints through the fi_ep_bind() function. 

完成队列通常直接映射到提供者硬件机制，而 libfabric 的设计目的是最大限度地减少访问这些机制对软件的影响。与目前讨论的其他对象（结构、域、端点）不同，完成队列不是 fi_info 结构的一部分，也不涉及 fi_getinfo() 调用。

所有活动端点必须绑定一个或多个完成队列。即使完成将被应用程序抑制（例如使用 FI_SELECTIVE_COMPLETION 标志）也是如此。需要完成队列来报告错误完成的操作。

发送和接收上下文都与它们自己的完成队列相关联。端点可以将发送和接收完成定向到单独的 CQ 或同一个 CQ。对于应用程序，使用单个 CQ 会降低系统资源利用率。虽然将完成分离到不同的 CQ 可以简化代码维护或改进多线程执行。一个 CQ 可以在多个端点之间共享。

CQ 与端点分开分配，并通过 fi_ep_bind() 函数与端点关联。

### Attributes

The properties of a completion queue are specified using the fi_cq_attr structure:

```
struct fi_cq_attr {
    size_t                size;
    uint64_t              flags;
    enum fi_cq_format     format;
    enum fi_wait_obj      wait_obj;
    int                   signaling_vector;
    enum fi_cq_wait_cond  wait_cond;
    struct fid_wait       *wait_set;
};
```

Select details are described below.

#### CQ Size

The CQ size is the number of entries that the CQ can store before being overrun. If resource management is disabled, then the application is responsible for ensuring that it does not submit more operations than the CQ can store. When selecting an appropriate size for a CQ, developers should consider the size of all transmit and receive contexts that insert completions into the CQ.

Because CQs often map to hardware constructs, their size may be limited to a pre-set maximum.  Applications should be prepared to allocate multiple CQs if they make use of a lot of endpoints –- a connection-oriented server application, for example.  Applications should size the CQ correctly to avoid wasting system resources, while still protecting against queue overruns.

#### CQ Format

In order to minimize the amount of data that a provider must report, the type of completion data written back to the application is select-able. This limits the number of bytes the provider writes to memory, and allows necessary completion data to fit into a compact structure. Each CQ format maps to a specific completion structure. Developers should analyze each structure, select the smallest structure that contains all of the data it requires, and specify the corresponding enum value as the CQ format.

For example, if an application only needs to know which request completed, along with the size of a received message, it can select the following:

#### CQ 大小

CQ 大小是 CQ 在溢出之前可以存储的条目数。如果资源管理被禁用，那么应用程序负责确保它不会提交超过 CQ 可以存储的操作。在为 CQ 选择合适的大小时，开发人员应考虑将完成插入 CQ 的所有发送和接收上下文的大小。

因为 CQ 经常映射到硬件结构，所以它们的大小可能会被限制在一个预设的最大值。如果应用程序使用大量端点——例如面向连接的服务器应用程序，则应准备分配多个 CQ。应用程序应正确调整 CQ 的大小以避免浪费系统资源，同时仍防止队列溢出。

#### CQ 格式

为了最小化提供者必须报告的数据量，写回应用程序的完成数据类型是可选择的。这限制了提供程序写入内存的字节数，并允许必要的完成数据适合紧凑的结构。每个 CQ 格式映射到一个特定的完成结构。开发者应该分析每个结构，选择包含它需要的所有数据的最小结构，并将对应的枚举值指定为CQ格式。

例如，如果应用程序只需要知道完成了哪个请求，以及收到的消息的大小，它可以选择以下内容：

```
cq_attr->format = FI_CQ_FORMAT_MSG;

struct fi_cq_msg_entry {
    void      *op_context;
    uint64_t  flags;
    size_t    len;
};
```

Once the format has been selected, the underlying provider will assume that read operations against the CQ will pass in an array of the corresponding structure.  The CQ data formats are designed such that a structure that reports more information can be cast to one that reports less.

选择格式后，底层提供程序将假定针对 CQ 的读取操作将传入相应结构的数组。 CQ 数据格式的设计使得报告更多信息的结构可以转换为报告更少的结构。

#### CQ Wait Object

Wait objects are a way for an application to suspend execution until it has been notified that a completion is ready to be retrieved from the CQ. The use of wait objects is recommended over busy waiting (polling) techniques for most applications. CQs include calls (fi_cq_sread() – for synchronous read) that will block until a completion occurs. Applications that will only use the libfabric blocking calls should select FI_WAIT_UNSPEC as their wait object. This allows the provider to select an object that is optimal for its implementation.

Applications that need to wait on other resources, such as open file handles or sockets, can request that a specific wait object be used. The most common alternative to FI_WAIT_UNSPEC is FI_WAIT_FD. This associates a file descriptor with the CQ. The file descriptor may be retrieved from the CQ using an fi_control() operation, and can be passed to standard operating system calls, such as select() or poll().

等待对象是应用程序暂停执行的一种方式，直到它被通知已准备好从 CQ 检索完成。 对于大多数应用程序，建议使用等待对象而不是忙等待（轮询）技术。 CQ 包括调用（fi_cq_sread() – 用于同步读取），这些调用将阻塞直到完成。 仅使用 libfabric 阻塞调用的应用程序应选择 FI_WAIT_UNSPEC 作为其等待对象。 这允许提供者选择最适合其实现的对象。

需要等待其他资源（例如打开的文件句柄或套接字）的应用程序可以请求使用特定的等待对象。 FI_WAIT_UNSPEC 最常见的替代方法是 FI_WAIT_FD。 这将文件描述符与 CQ 相关联。 可以使用 fi_control() 操作从 CQ 检索文件描述符，并且可以将其传递给标准操作系统调用，例如 select() 或 poll()。

### Reading Completions

Completions may be read from a CQ by using one of the non-blocking calls, fi_cq_read / fi_cq_readfrom, or one of the blocking calls, fi_cq_sread / fi_cq_sreadfrom. Regardless of which call is used, applications pass in an array of completion structures based on the selected CQ format. The CQ interfaces are optimized for batch completion processing, allowing the application to retrieve multiple completions from a single read call.  The difference between the read and readfrom calls is that readfrom returns source addressing data, if available. The readfrom derivative of the calls is only useful for unconnected endpoints, and only if the corresponding endpoint has been configured with the FI_SOURCE capability.

FI_SOURCE requires that the provider use the source address available in the raw completion data, such as the packet's source address, to retrieve a matching entry in the endpoint’s address vector. Applications that carry some sort of source identifier as part of their data packets can avoid the overhead associated with using FI_SOURCE.

可以使用非阻塞调用之一 fi_cq_read / fi_cq_readfrom 或阻塞调用之一 fi_cq_sread / fi_cq_sreadfrom 从 CQ 读取完成。无论使用哪个调用，应用程序都会根据所选的 CQ 格式传入完成结构数组。 CQ 接口针对批处理完成进行了优化，允许应用程序从单个读取调用中检索多个完成。 read 和 readfrom 调用之间的区别在于 readfrom 返回源寻址数据（如果可用）。调用的 readfrom 派生仅对未连接的端点有用，并且仅当相应的端点已配置有 FI_SOURCE 功能时。

FI_SOURCE 要求提供者使用原始完成数据中可用的源地址（例如数据包的源地址）来检索端点地址向量中的匹配条目。将某种源标识符作为其数据包的一部分的应用程序可以避免与使用 FI_SOURCE 相关的开销。

### Retrieving Errors

Because the selected completion structure is insufficient to report all data necessary to debug or handle an operation that completes in error, failed operations are reported using a separate fi_cq_readerr() function.  This call takes as input a CQ error entry structure, which allows the provider to report more information regarding the reason for the failure.

由于所选的完成结构不足以报告调试或处理错误完成的操作所需的所有数据，因此使用单独的 fi_cq_readerr() 函数报告失败的操作。 此调用将 CQ 错误条目结构作为输入，它允许提供者报告有关失败原因的更多信息。

```
/* read error prototype */
fi_cq_readerr(struct fid_cq *cq, struct fi_cq_err_entry *buf, uint64_t flags);

/* error data structure */
struct fi_cq_err_entry {
    void      *op_context;
    uint64_t  flags;
    size_t    len;
    void      *buf;
    uint64_t  data;
    uint64_t  tag;
    size_t    olen;
    int       err;
    int       prov_errno;
    void     *err_data;
};

/* Sample error handling */
struct fi_cq_msg_entry entry;
struct fi_cq_err_entry err_entry;
int ret;

ret = fi_cq_read(cq, &entry, 1);
if (ret == -FI_EAVAIL)
    ret = fi_cq_readerr(cq, &err_entry, 0);
```

As illustrated, if an error entry has been inserted into the completion queue, then attempting to read the CQ will result in the read call returning -FI_EAVAIL (error available).  This indicates that the application must use the fi_cq_readerr() call to remove the failed operation's completion information before other completions can be reaped from the CQ.

A fabric error code regarding the failure is reported as the err field in the fi_cq_err_entry structure.  A provider specific error code is also available through the prov_errno field.  This field can be decoded into a displayable string using the fi_cq_strerror() routine. The err_data field is provider specific data that assists the provider in decoding the reason for the failure.

如图所示，如果错误条目已插入完成队列，则尝试读取 CQ 将导致读取调用返回 -FI_EAVAIL（错误可用）。 这表明应用程序必须使用 fi_cq_readerr() 调用来删除失败操作的完成信息，然后才能从 CQ 获取其他完成信息。

有关故障的结构错误代码报告为 fi_cq_err_entry 结构中的 err 字段。 提供者特定的错误代码也可通过 prov_errno 字段获得。 可以使用 fi_cq_strerror() 例程将此字段解码为可显示的字符串。 err_data 字段是提供者特定的数据，可帮助提供者解码失败的原因。

## Counters

Completion counters are conceptually very simple completion mechanisms that return the number of completions that have occurred on an endpoint.  No other details about the completion is available.  Counters work well for connection-oriented applications that make use of strict completion ordering (rx/tx attribute comp_order = FI_ORDER_STRICT), or applications that need to collect a specific number of responses from peers.

An endpoint has more flexibility with how many counters it can use relative to completion queues.  Different types of operations can update separate counters.  For instance, sent messages can update one counter, while RMA writes can update another.  This allows for simple, yet powerful usage models, as control message completions can be tracked independently from large data transfers.  Counters are associated with active endpoints using the fi_ep_bind() call:

完成计数器在概念上是非常简单的完成机制，它返回在端点上发生的完成次数。 没有其他关于完成的详细信息。 计数器适用于使用严格完成排序（rx/tx 属性 comp_order = FI_ORDER_STRICT）的面向连接的应用程序，或需要从对等方收集特定数量的响应的应用程序。

端点相对于完成队列可以使用多少计数器具有更大的灵活性。 不同类型的操作可以更新不同的计数器。 例如，发送的消息可以更新一个计数器，而 RMA 写入可以更新另一个。 这允许简单但功能强大的使用模型，因为可以独立于大数据传输跟踪控制消息的完成。 计数器使用 fi_ep_bind() 调用与活动端点相关联：

```
/* Example binding a counter to an endpoint.
 * The counter will update on completion of any transmit operation.
 */
fi_ep_bind(ep, cntr, FI_SEND | FI_WRITE | FI_READ);
```

Counters are defined such that they can be implemented either in hardware or in software, by layering over a hardware completion queue.  Even when implemented in software, counter use can improve performance by reducing the amount of completion data that is reported.  Additionally, providers may be able to optimize how a counter is updated, relative to an application counting the same type of events.  For example, a provider may be able to compare the head and tail pointers of a queue to determine the total number of completions that are available, allowing a single write to update a counter, rather than repeatedly incrementing a counter variable once for each completion.

通过在硬件完成队列上分层，计数器被定义为可以在硬件或软件中实现。 即使在软件中实现，计数器的使用也可以通过减少报告的完成数据量来提高性能。 此外，相对于计数相同类型事件的应用程序，提供者可能能够优化计数器的更新方式。 例如，提供者可能能够比较队列的头指针和尾指针以确定可用的完成总数，从而允许单次写入更新计数器，而不是每次完成时重复递增计数器变量一次。

### Attributes

Most counter attributes are a subset of the CQ attributes:

```
struct fi_cntr_attr {
    enum fi_cntr_events  events;
    enum fi_wait_obj     wait_obj;
    struct fid_wait      *wait_set;
    uint64_t             flags;
};
```

The sole exception is the events field, which must be set to FI_CNTR_EVENTS_COMP, indicating that completion events are being counted.  (This field is defined for future extensibility).  A completion counter is updated according to the completion model that was selected by the endpoint.  For example, if an endpoint is configured to for transmit complete, the counter will not be updated until the transfer has been received by the target endpoint.

唯一的例外是 events 字段，它必须设置为 FI_CNTR_EVENTS_COMP，表示正在计算完成事件。 （该字段是为将来的可扩展性而定义的）。 根据端点选择的完成模型更新完成计数器。 例如，如果端点配置为传输完成，则在目标端点接收到传输之前不会更新计数器。

### Counter Values

A completion counter is actually comprised of two different values.  One represents the number of operations that complete successfully.  The other indicates the number of operations which complete in error.  Counters do not provide any additional information about the type of error, nor indicate which operation failed.  Details of errors must be retrieved from a completion queue.

Reading a counter’s values is straightforward:

完成计数器实际上由两个不同的值组成。 1 表示成功完成的操作数。 另一个表示错误完成的操作数。 计数器不提供有关错误类型的任何附加信息，也不指示哪个操作失败。 必须从完成队列中检索错误的详细信息。

读取计数器的值很简单：

```
uint64_t fi_cntr_read(struct fid_cntr *cntr);
uint64_t fi_cntr_readerr(struct fid_cntr *cntr);
```

# Address Vectors 地址向量

A primary goal of address vectors is to allow applications to communicate with thousands to millions of peers while minimizing the amount of data needed to store peer addressing information. It pushes fabric specific addressing details away from the application to the provider. This allows the provider to optimize how it converts addresses into routing data, and enables data compression techniques that may be difficult for an application to achieve without being aware of low-level fabric addressing details. For example, providers may be able to algorithmically calculate addressing components, rather than storing the data locally. Additionally, providers can communicate with resource management entities or fabric manager agents to obtain quality of service or other information about the fabric, in order to improve network utilization.

An equally important objective is ensuring that the resulting interfaces, particularly data transfer operations, are fast and easy to use. Conceptually, an address vector converts an endpoint address into an fi_addr_t. The fi_addr_t (fabric interface address datatype) is a 64-bit value that is used in all ‘fast-path’ operations – data transfers and completions.

Address vectors are associated with domain objects. This allows providers to implement portions of an address vector, such as quality of service mappings, in hardware.

**地址向量**的主要目标是允许应用程序与数千到数百万个对等点进行通信，同时最大限度地减少存储对等点寻址信息所需的数据量。它将特定于结构的寻址细节从应用程序推送到提供者。这允许提供商优化将地址转换为路由数据的方式，并启用应用程序在不了解低级结构寻址细节的情况下可能难以实现的数据压缩技术。例如，提供者可能能够通过算法计算寻址组件，而不是在本地存储数据。此外，提供商可以与资源管理实体或结构管理器代理进行通信，以获得服务质量或有关结构的其他信息，以提高网络利用率。

一个同样重要的目标是确保生成的接口，特别是数据传输操作，快速且易于使用。从概念上讲，地址向量将端点地址转换为 fi_addr_t。 fi_addr_t（结构接口地址数据类型）是一个 **64 位值**，用于所有“快速路径”操作——数据传输和完成。

地址向量与域对象相关联。这允许提供商在硬件中实现地址向量的一部分，例如服务质量映射。

地址向量用于将更高级别的地址（对于应用程序使用起来可能更自然）映射到结构特定地址。地址的映射是特定于结构和提供商的，但可能涉及冗长的地址解析和结构管理协议。AV 操作默认是同步的，但可以通过指定FI_EVENT 标志为fi_av_open的参数来设置为异步操作。当请求异步操作时，应用程序必须先将事件队列绑定到 AV 才能插入地址。有关重复地址的 AV 限制，请参阅注释部分

## AV Attributes

Address vectors are created with the following attributes. Select attribute details are discussed below:

地址向量是使用以下属性创建的。 下面讨论选择属性的详细信息：

```
struct fi_av_attr {
    enum fi_av_type type;
    int rx_ctx_bits;
    size_t count;
    size_t ep_per_node;
    const char *name;
    void *map_addr;
    uint64_t flags;
};
```

### AV Type

There are two types of address vectors. The type refers to the format of the returned fi_addr_t values for addresses that are inserted into the AV. With type FI_AV_TABLE, returned addresses are simple indices, and developers may think of the AV as an array of addresses. Each address that is inserted into the AV is mapped to the index of the next free array slot. The advantage of FI_AV_TABLE is that applications can refer to peers using a simple index, eliminating an application’s need to store any addressing data. I.e. the application can generate the fi_addr_t values themselves.  This type maps well to applications, such as MPI, where a peer is referenced by rank.

The second type is FI_AV_MAP. This type does not define any specific format for the fi_addr_t value. Applications that use type map are required to provide the correct fi_addr_t for a given peer when issuing a data transfer operation. The advantage of FI_AV_MAP is that a provider can use the fi_addr_t to encode the target’s address, which avoids retrieving the data from memory. As a simple example, consider a fabric that uses TCP/IPv4 based addressing. An fi_addr_t is large enough to contain the address, which allows a provider to copy the data from the fi_addr_t directly into an outgoing packet.

有两种类型的地址向量。类型是指插入到 AV 中的地址的返回 fi_addr_t 值的格式。对于 FI_AV_TABLE 类型，返回的地址是简单的索引，开发人员可能会将 AV 视为地址数组。插入 AV 的每个地址都映射到下一个空闲数组槽的索引。 FI_AV_TABLE 的优点是应用程序可以使用简单的索引来引用对等点，从而消除了应用程序存储任何寻址数据的需要。 IE。应用程序可以自己生成 fi_addr_t 值。这种类型很好地映射到应用程序，例如 MPI，其中对等点按等级引用。

第二种类型是 FI_AV_MAP。此类型没有为 fi_addr_t 值定义任何特定格式。使用类型映射的应用程序需要在发出数据传输操作时为给定对等方提供正确的 fi_addr_t。 FI_AV_MAP 的优点是提供者可以使用 fi_addr_t 对目标地址进行编码，从而避免从内存中检索数据。作为一个简单的例子，考虑一个使用基于 TCP/IPv4 寻址的结构。 fi_addr_t 足够大以包含地址，这允许提供者将数据从 fi_addr_t 直接复制到传出数据包中。

### AV Rx Context Bits

The rx_ctx_bits field is only used with scalable endpoints with named received contexts, and is best described using an example. A peer process has allocated a scalable endpoint with two receive contexts. The first receive context will be used for control message, with data messages targeting the second context. Named contexts is a feature that allows the initiator to select which context will receive a message. If the initiating application wishes to send a data message, it must indicate that the message should be steered to the second context.

The rx_ctx_bits allocates a specific number of bits out of the fi_addr_t value that will be used to indicate which context a given operation will target. Applications should reserve a number of bits large enough to indicate any context at the target. For example, two bits is sufficient to target one of four different receive contexts.

The function fi_rx_addr() converts a target fi_addr_t address, along with the requested receive context index, into a new fi_addr_t value that may be used to transfer data to a scalable endpoint’s receive context.

rx_ctx_bits 字段仅用于具有命名接收上下文的可扩展端点，最好使用示例进行描述。一个对等进程分配了一个具有两个接收上下文的可扩展端点。第一个接收上下文将用于控制消息，数据消息针对第二个上下文。命名上下文是一项功能，它允许发起者选择哪个上下文将接收消息。如果启动应用程序希望发送数据消息，它必须指示该消息应该被引导到第二个上下文。

rx_ctx_bits 从 fi_addr_t 值中分配特定数量的位，用于指示给定操作将针对哪个上下文。应用程序应保留足够大的比特数以指示目标处的任何上下文。例如，两个比特足以针对四个不同的接收上下文之一。

函数 fi_rx_addr() 将目标 fi_addr_t 地址以及请求的接收上下文索引转换为新的 fi_addr_t 值，该值可用于将数据传输到可扩展端点的接收上下文。

```
fi_addr_t fi_rx_addr(fi_addr_t fi_addr, int rx_index, int rx_ctx_bits);
```

This call is simply a wrapper that writes the rx_index into the space that was reserved in the fi_addr_t value.

### Sharing AVs Between Processes

Large scale parallel programs typically run with multiple processes allocated on each node. Because these processes communicate with the same set of peers, the addressing data needed by each process is the same. Libfabric defines a mechanism by which processes running on the same node may share their address vectors. This allows a system to maintain a single copy of addressing data, rather than one copy per process.

Although libfabric does not require any implementation for how an address vector is shared, the interfaces map well to using shared memory. Address vectors which will be shared are given an application specific name. How an application selects a name that avoid conflicts with unrelated processes, or how it communicates the name with peer processes is outside the scope of libfabric.

In addition to having a name, a shared AV also has a base map address -- map_addr. Use of map_addr is only important for address vectors that are of type FI_AV_MAP, and allows applications to share fi_addr_t values. From the viewpoint of the application, the map_addr is the base value for all fi_addr_t values. A common use for map_addr is for the process that creates the initial address vector to request a value from the provider, exchange the returned map_addr with its peers, and for the peers to open the shared AV using the same map_addr. This allows the fi_addr_t values to be stored in shared memory that is 

accessible by all peers.

大规模并行程序通常在每个节点上分配多个进程运行。因为这些进程与同一组对等体进行通信，所以每个进程所需的寻址数据是相同的。 Libfabric 定义了一种机制，通过该机制运行在同一节点上的进程可以共享它们的地址向量。这允许系统维护寻址数据的单个副本，而不是每个进程一个副本。

尽管 libfabric 不需要任何实现地址向量的共享方式，但接口很好地映射到使用共享内存。将被共享的地址向量被赋予一个特定于应用程序的名称。应用程序如何选择一个名称以避免与无关进程发生冲突，或者它如何与对等进程通信名称超出了 libfabric 的范围。

共享 AV 除了有名字之外，还有一个基本的地图地址——map_addr。 map_addr 的使用仅对 FI_AV_MAP 类型的地址向量很重要，并允许应用程序共享 fi_addr_t 值。从应用程序的角度来看，map_addr 是所有 fi_addr_t 值的基值。 map_addr 的一个常见用途是用于创建初始地址向量以向提供者请求值、与其对等方交换返回的 map_addr 以及让对等方使用相同的 map_addr 打开共享 AV 的进程。这允许将 fi_addr_t 值存储在共享内存中，即所有同行都可以访问。

# Wait and Poll Sets

As mentioned, most libfabric operations involve asynchronous processing, with completions reported to event queues, completion queues, and counters. Wait sets and poll sets were created to help manage and optimize checking for completed requests across multiple objects.

A poll set is a collection of event queues, completion queues, and counters. Applications use a poll set to check if a new completion event has arrived on any of its associated objects. When events occur infrequently or to one of several completion reporting objects, using a poll set can improve application efficiency by reducing the number of calls that the application makes into the libfabric provider. The use of a poll set should be considered by apps that use at least two completion reporting structures, and it is likely that checking them will find that no new events have occurred.

A wait set is similar to a poll set, and is often used in conjunction with one. In ideal implementations, a wait set is associated with a single wait object, such as a file descriptor. All event / completion queues and counters associated with the wait set will be configured to signal that wait object when an event occurs. This minimizes the system resources that are necessary to support applications waiting for events.

如前所述，大多数 libfabric 操作都涉及异步处理，完成报告给事件队列、完成队列和计数器。创建等待集和轮询集是为了帮助管理和优化对跨多个对象的已完成请求的检查。

轮询集是事件队列、完成队列和计数器的集合。应用程序使用轮询集来检查新的完成事件是否已到达其任何关联对象。当事件不经常发生或发生在几个完成报告对象之一时，使用轮询集可以通过减少应用程序对 libfabric 提供程序的调用次数来提高应用程序效率。使用至少两个完成报告结构的应用程序应该考虑使用轮询集，并且检查它们很可能会发现没有发生新事件。

等待集类似于轮询集，并且经常与其中之一结合使用。在理想的实现中，等待集与单个等待对象相关联，例如文件描述符。与等待集相关的所有事件/完成队列和计数器将被配置为在事件发生时向该等待对象发出信号。这最大限度地减少了支持等待事件的应用程序所需的系统资源。

## Poll Set

The poll set API is fairly straightforward.

```
int fi_poll_open(struct fid_domain *domain, struct fi_poll_attr *attr,
    struct fid_poll **pollset);
int fi_poll_add(struct fid_poll *pollset, struct fid *event_fid,
    uint64_t flags);
int fi_poll_del(struct fid_poll *pollset, struct fid *event_fid,
    uint64_t flags);
int fi_poll(struct fid_poll *pollset, void **context, int count);
```

Applications call fi_poll_open() to allocate a poll set. The attribute structure is a placeholder for future extensions and contains a single flags field, which is reserved. To add and remove event queues, completion queues, and counters, the fi_poll_add() and fi_poll_del() calls are used. As with open, the flags parameter is for extensibility and should be 0. Once objects have been associated with the poll set, an app may call fi_poll() to retrieve a list of objects that may have new events available.

应用程序调用 fi_poll_open() 来分配轮询集。 属性结构是未来扩展的占位符，并包含一个保留的标志字段。 要添加和删除事件队列、完成队列和计数器，请使用 fi_poll_add() 和 fi_poll_del() 调用。 与 open 一样，flags 参数用于可扩展性，应为 0。一旦对象与轮询集相关联，应用程序可能会调用 fi_poll() 来检索可能具有可用新事件的对象列表。

```
struct my_cq *tx_cq, *rx_cq; /* embeds fid_cq, configured separately */
struct fid_poll *pollset;
struct fi_poll_attr attr = {};
void *cq_context;

/* Allocate and add CQs to poll set */
fi_poll_open(domain, &attr, &pollset);
fi_poll_add(pollset, &tx_cq->cq.fid, 0);
fi_poll_add(pollset, &rx_cq->cq.fid, 0);

/* Check for events */
ret = fi_poll(pollset, &cq_context, 1);
if (ret == 1) {
    /* CQ had an event */
    struct my_cq *cq = cq_context;
    struct fi_cq_msg_entry entry;
    fi_cq_read(&cq->cq, &entry, 1);
}

```

It’s worth noting that fi_poll() returns a list of objects that have experienced some level of activity since they were last checked. However, an object appearing in the poll output does not guarantee that an event is actually available. For example, fi_poll() may return the context associated with a completion queue, but an app may find that queue empty when reading it. This behavior is permissible by the API and is the result of potential provider implementation details.

One reason this can occur is if an entry is added to the completion queue, but that entry should not be reported to the application. For example, the completion may correspond to a message sent or received as part the provider’s protocol, and may not correspond to an application operation. The fi_poll() routine simply reports whether the queue is empty or not, and is not intended for event processing, which is deferred until the queue can be read in order to avoid additional software queuing overhead.

值得注意的是，fi_poll() 返回一个对象列表，这些对象自上次检查以来经历了某种程度的活动。 但是，出现在轮询输出中的对象并不能保证事件实际上是可用的。 例如，fi_poll() 可能会返回与完成队列关联的上下文，但应用程序可能会在读取该队列时发现该队列为空。 这种行为是 API 允许的，并且是潜在的提供者实现细节的结果。

发生这种情况的一个原因是，如果一个条目被添加到完成队列中，但该条目不应该报告给应用程序。 例如，完成可能对应于作为提供商协议的一部分发送或接收的消息，并且可能不对应于应用程序操作。 fi_poll() 例程仅报告队列是否为空，并且不用于事件处理，事件处理被推迟到可以读取队列时，以避免额外的软件排队开销。

## Wait Sets

The wait set API is actually smaller than the poll set.

```
int fi_wait_open(struct fid_fabric *fabric, struct fi_wait_attr *attr,
    struct fid_wait **waitset);
int fi_wait(struct fid_wait *waitset, int timeout);

struct fi_wait_attr {
    enum fi_wait_obj wait_obj;
    uint64_t flags;
};

```

The type of wait object that the wait set should use is specified through the wait attribute structure. Unlike poll sets, a wait set is associated with event queues, completion queues, and counters during their creation. This is necessary so that system resources, such as file descriptors, can be properly allocated and configured. Applications can block until the wait set’s wait object is signaled using the fi_wait() call. Or an application can use an fi_control() call to retrieve the native wait object for use directly with system calls, such as poll() and select().

A wait set is signaled whenever an event is added to one of its associated objects which would trigger the signal. In many cases, the wait set is signaled when any new event occurs; however, some objects will delay signaling the wait object until a threshold is crossed.

Because wait sets are responsible for linking completion reporting objects with wait objects, they can only indicate when a wait object has been signaled. A wait set cannot identify which object was responsible for signaling the wait object. Once a wait has been satisfied, applications are responsible for checking all completion structures for events. One simple way to accomplish this is to place all objects sharing a wait set into a peer poll set.

等待集应该使用的等待对象的类型是通过等待属性结构指定的。与轮询集不同，等待集在创建期间与事件队列、完成队列和计数器相关联。这是必要的，以便可以正确分配和配置系统资源，例如文件描述符。应用程序可以阻塞，直到使用 fi_wait() 调用通知等待集的等待对象。或者应用程序可以使用 fi_control() 调用来检索本机等待对象，以便直接与系统调用一起使用，例如 poll() 和 select()。

每当将事件添加到将触发信号的关联对象之一时，就会发出等待集的信号。在许多情况下，当任何新事件发生时都会发出等待集信号；但是，某些对象会延迟向等待对象发出信号，直到超过阈值。

因为等待集负责将完成报告对象与等待对象联系起来，所以它们只能指示等待对象何时发出信号。等待集无法识别哪个对象负责向等待对象发出信号。一旦满足等待，应用程序负责检查事件的所有完成结构。实现此目的的一种简单方法是将共享等待集的所有对象放入对等轮询集。

## Using Native Wait Objects: TryWait 使用原生等待对象:尝试等待

There is an important difference between using libfabric completion objects, versus sockets, that may not be obvious from the discussions so far. With sockets, the object that is signaled is the same object that abstracts the queues, namely the file descriptor. When data is received on a socket, that data is placed in a queue associated directly with the fd. Reading from the fd retrieves that data. If an application wishes to block until data arrives on a socket, it calls select() or poll() on the fd. The fd is signaled when a message is received, which releases the blocked thread, allowing it to read the fd.

By associating the wait object with the underlying data queue, applications are exposed to an interface that is easy to use and race free. If data is available to read from the socket at the time select() or poll() is called, those calls simply return that the fd is readable.

There are a couple of significant disadvantages to this approach, which have been discussed previously, but from different perspectives. The first is that every socket must be associated with its own fd. There is no way to share a wait object among multiple sockets. (This is a main reason for the development of epoll semantics). The second is that the queue is maintained in the kernel, so that the select() and poll() calls can check them.

Libfabric separates the wait object from the queues. For applications that use libfabric interfaces to wait for events, such as fi_cq_sread and fi_wait, this separation is mostly hidden from the application. The exception is that applications may receive a signal (e.g. fi_wait() returns success), but no events are retrieved when a queue is read.  This separation allows the queues to reside in the application's memory space, while wait objects may still use kernel components.  A reason for the latter is that wait objects may be signaled as part of system interrupt processing, which would go through a kernel driver.

Applications that want to use native wait objects (e.g. file descriptors) directly in operating system calls must perform an additional step in their processing. In order to handle race conditions that can occur between inserting an event into a completion or event object and signaling the corresponding wait object, libfabric defines a ‘trywait’ function. The fi_trywait implementation is responsible for handling potential race conditions which could result in an application either losing events or hanging. The following example demonstrates the use of fi_trywait.

使用 libfabric 完成对象与使用套接字之间有一个重要的区别，这在目前的讨论中可能并不明显。对于套接字，发出信号的对象与抽象队列的对象相同，即文件描述符。当在套接字上接收到数据时，该数据被放置在与 fd 直接关联的队列中。从 fd 读取会检索该数据。如果应用程序希望阻塞直到数据到达套接字，它会调用 fd 上的 select() 或 poll()。当收到一条消息时，fd 会发出信号，这会释放阻塞的线程，允许它读取 fd。

通过将等待对象与底层数据队列相关联，应用程序暴露于易于使用且无竞争的接口。如果在调用 select() 或 poll() 时可以从套接字读取数据，则这些调用仅返回 fd 可读。

这种方法有几个明显的缺点，前面已经讨论过，但是从不同的角度。首先是每个套接字都必须与它自己的 fd 相关联。无法在多个套接字之间共享等待对象。 （这是epoll语义发展的一个主要原因）。第二个是队列在内核中维护，以便select()和poll()调用可以检查它们。

Libfabric 将等待对象与队列分开。对于使用 libfabric 接口来等待事件的应用程序，例如 fi_cq_sread 和 fi_wait，这种分离大部分对应用程序是隐藏的。例外情况是应用程序可能会收到一个信号（例如 fi_wait() 返回成功），但在读取队列时不会检索到任何事件。这种分离允许队列驻留在应用程序的内存空间中，而等待对象仍可能使用内核组件。后者的一个原因是等待对象可以作为系统中断处理的一部分发出信号，这将通过内核驱动程序。

想要直接在操作系统调用中使用本机等待对象（例如文件描述符）的应用程序必须在其处理中执行额外的步骤。为了处理在将事件插入完成或事件对象和发出相应等待对象的信号之间可能发生的竞争条件，libfabric 定义了一个“trywait”函数。 fi_trywait 实现负责处理可能导致应用程序丢失事件或挂起的潜在竞争条件。以下示例演示了 fi_trywait 的使用。

```
/* Get the native wait object -- an fd in this case */
fi_control(&cq->fid, FI_GETWAIT, (void *) &fd);
FD_ZERO(&fds);
FD_SET(fd, &fds);

while (1) {
    ret = fi_trywait(fabric, &cq->fid, 1);
    if (ret == FI_SUCCESS) {
        /* It’s safe to block on the fd */
        select(fd + 1, &fds, NULL, &fds, &timeout);
    } else if (ret == -FI_EAGAIN) {
        /* Read and process all completions from the CQ */
        do {
            ret = fi_cq_read(cq, &comp, 1);
        } while (ret > 0);
    } else {
        /* something really bad happened */
    }
}
```

In this example, the application has allocated a CQ with an fd as its wait object. It calls select() on the fd. Before calling select(), the application must call fi_trywait() successfully (return code of FI_SUCCESS). Success indicates that a blocking operation can now be invoked on the native wait object without fear of the application hanging or events being lost. If fi_trywait() returns –FI_EAGAIN, it usually indicates that there are queued events to process.

在这个例子中，应用程序分配了一个以 fd 作为其等待对象的 CQ。 它在 fd 上调用 select()。 在调用 select() 之前，应用程序必须成功调用 fi_trywait()（返回码 FI_SUCCESS）。 成功表示现在可以在本机等待对象上调用阻塞操作，而不必担心应用程序挂起或事件丢失。 如果 fi_trywait() 返回 –FI_EAGAIN，通常表示有排队的事件要处理。

# Putting It All Together
## MSG EP pingpong
## RDM EP pingpong




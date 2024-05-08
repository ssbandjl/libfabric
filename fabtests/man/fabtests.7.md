---
layout: page
title: fabtests(7)
tagline: Fabtests Programmer's Manual
---
{% include JB/setup %}

# NAME

Fabtests

# SYNOPSIS

Fabtests is a set of examples for fabric providers that demonstrates
various features of libfabric- high-performance fabric software library.

# OVERVIEW

Libfabric defines sets of interface that fabric providers can support.
The purpose of Fabtests examples is to demonstrate some of the major features.
The goal is to familiarize users with different functionalities libfabric
offers and how to use them.  Although most tests report performance numbers,
they are designed to test functionality and not performance.  The exception
are the benchmarks and ubertest.

The tests are divided into the following categories. Except the unit tests
all of them are client-server tests.  Not all providers will support each test.

The test names try to indicate the type of functionality each test is
verifying.  Although some tests work with any endpoint type, many are
restricted to verifying a single endpoint type.  These tests typically
include the endpoint type as part of the test name, such as dgram, msg, or
rdm.

## Functional

These tests are a mix of very basic functionality tests that show major
features of libfabric.

*fi_av_xfer*
: Tests communication for connectionless endpoints, as addresses
  are inserted and removed from the local address vector.

*fi_cm_data*
: Verifies exchanging CM data as part of connecting endpoints.

*fi_cq_data*
: Tranfers messages with CQ data.

*fi_dgram*
: A basic datagram endpoint example.

*fi_dgram_waitset*
: Transfers datagrams using waitsets for completion notification.

*fi_inj_complete*
: Sends messages using the FI_INJECT_COMPLETE operation flag.

*fi_mcast*
: A simple multicast test.

*fi_msg*
: A basic message endpoint example.

*fi_msg_epoll*
: Transfers messages with completion queues configured to use file
  descriptors as wait objects.  The file descriptors are retrieved
  by the program and used directly with the Linux epoll API.

*fi_msg_sockets*
: Verifies that the address assigned to a passive endpoint can be
  transitioned to an active endpoint.  This is required applications
  that need socket API semantics over RDMA implementations (e.g. rsockets).

*fi_multi_ep*
: Performs data transfers over multiple endpoints in parallel.

*fi_multi_mr*
: Issues RMA write operations to multiple memory regions, using
  completion counters of inbound writes as the notification
  mechanism.

*fi_poll*
: Exchanges data over RDM endpoints using poll sets to drive
  completion notifications.

*fi_rdm*
: A basic RDM endpoint example.

*fi_rdm_atomic*
: Test and verifies atomic operations over an RDM endpoint.

*fi_rdm_deferred_wq*
: Test triggered operations and deferred work queue support.

*fi_rdm_multi_domain*
: Performs data transfers over multiple endpoints, with each
  endpoint belonging to a different opened domain.

*fi_rdm_multi_recv*
: Transfers multiple messages over an RDM endpoint that are received
  into a single buffer, posted using the FI_MULTI_RECV flag.

*fi_rdm_rma_event*
: An RMA write example over an RDM endpoint that uses RMA events
  to notify the peer that the RMA transfer has completed.

*fi_rdm_rma_trigger*
: A basic example of queuing an RMA write operation that is initiated
  upon the firing of a triggering completion. Works with RDM endpoints.

*fi_rdm_shared_av*
: Spawns child processes to verify basic functionality of using a shared
  address vector with RDM endpoints.

*fi_rdm_stress*
: A multi-process, multi-threaded stress test of RDM endpoints handling
  transfer errors.

*fi_rdm_tagged_peek*
: Basic test of using the FI_PEEK operation flag with tagged messages.
  Works with RDM endpoints.

*fi_recv_cancel*
: Tests canceling posted receives for tagged messages.

*fi_resmgmt_test*
: Tests the resource management enabled feature.  This verifies that the
  provider prevents applications from overrunning local and remote command
  queues and completion queues.  This corresponds to setting the domain
  attribute resource_mgmt to FI_RM_ENABLED.

*fi_scalable_ep*
: Performs data transfers over scalable endpoints, endpoints associated
  with multiple transmit and receive contexts.

*fi_shared_ctx*
: Performs data transfers between multiple endpoints, where the endpoints
  share transmit and/or receive contexts.

*fi_unexpected_msg*
: Tests the send and receive handling of unexpected tagged messages.

*fi_unmap_mem*
: Tests data transfers where the transmit buffer is mmapped and
  unmapped between each transfer, but the virtual address of the transmit
  buffer tries to remain the same.  This test is used to validate the
  correct behavior of memory registration caches.

*fi_bw*
: Performs a one-sided bandwidth test with an option for data verification.
  A sleep time on the receiving side can be enabled in order to allow
  the sender to get ahead of the receiver.

*fi_rdm_multi_client*
: Tests a persistent server communicating with multiple clients, one at a
  time, in sequence.

## Benchmarks

The client and the server exchange messages in either a ping-pong manner,
for pingpong named tests, or transfer messages one-way, for bw named tests.
These tests can transfer various messages sizes, with controls over which
features are used by the test, and report performance numbers.  The tests
are structured based on the benchmarks provided by OSU MPI.  They are not
guaranteed to provide the best latency or bandwidth performance numbers a
given provider or system may achieve.

*fi_dgram_pingpong*
: Latency test for datagram endpoints

*fi_msg_bw*
: Message transfer bandwidth test for connected (MSG) endpoints.

*fi_msg_pingpong*
: Message transfer latency test for connected (MSG) endpoints.

*fi_rdm_cntr_pingpong*
: Message transfer latency test for reliable-datagram (RDM) endpoints
  that uses counters as the completion mechanism.

*fi_rdm_pingpong*
: Message transfer latency test for reliable-datagram (RDM) endpoints.

*fi_rdm_tagged_bw*
: Tagged message bandwidth test for reliable-datagram (RDM) endpoints.

*fi_rdm_tagged_pingpong*
: Tagged message latency test for reliable-datagram (RDM) endpoints.

*fi_rma_bw*
: An RMA read and write bandwidth test for reliable (MSG and RDM) endpoints.

*fi_rma_pingpong*
: An RMA write and writedata latency test for reliable-datagram (RDM) endpoints.

## Unit

These are simple one-sided unit tests that validate basic behavior of the API.
Because these are single system tests that do not perform data transfers their
testing scope is limited.

*fi_av_test*
: Verify address vector interfaces.

*fi_cntr_test*
: Tests counter creation and destruction.

*fi_cq_test*
: Tests completion queue creation and destruction.

*fi_dom_test*
: Tests domain creation and destruction.

*fi_eq_test*
: Tests event queue creation, destruction, and capabilities.

*fi_getinfo_test*
: Tests provider response to fi_getinfo calls with varying hints.

*fi_mr_test*
: Tests memory registration.

*fi_mr_cache_evict*
: Tests provider MR cache eviction capabilities.

## Multinode

This test runs a series of tests over multiple formats and patterns to help
validate at scale. The patterns are an all to all, one to all, all to one and
a ring. The tests also run across multiple capabilities, such as messages, rma,
atomics, and tagged messages. Currently, there is no option to run these
capabilities and patterns independently, however the test is short enough to be
all run at once.

## Ubertest

This is a comprehensive latency, bandwidth, and functionality test that can
handle a variety of test configurations.  The test is able to run a large
number of tests by iterating over a large number of test variables.  As a
result, a full ubertest run can take a significant amount of time.  Because
ubertest iterates over input variables, it relies on a test configuration
file for control, rather than extensive command line options that are used
by other fabtests.  A configuration file must be constructed for each
provider.  Example test configurations are at test_configs.

*fi_ubertest*
: This test takes a configure file as input.  The file contains a list of
  variables and their values to iterate over.  The test will run a set of
  latency, bandwidth, and functionality tests over a given provider.  It
  will perform one execution for every possible combination of all variables.
  For example, if there are 8 test variables, with 6 having 2 possible
  values and 2 having 3 possible values, ubertest will execute 576 total
  iterations of each test.

# EFA provider specific tests

Beyond libfabric defined functionalities, EFA provider defines its
specific features/functionalities. These EFA provider specific fabtests
show users how to correctly use them.

*fi_efa_rnr_read_cq_error*
: This test modifies the RNR retry count (rnr_retry) to 0 via
  fi_setopt, and then runs a simple program to test if the error cq entry
  (with error FI_ENORX) can be read by the application, if RNR happens.

*fi_efa_rnr_queue_resend*
: This test modifies the RNR retry count (rnr_retry) to 0 via fi_setopt,
  and then tests RNR queue/re-send logic for different packet types.
  To run the test, one needs to use `-c` option to specify the category
  of packet types.

## Component tests

These stand-alone tests don't test libfabric functionalities. Instead,
they test some components that libfabric depend on. They are not called
by runfabtests.sh, either, and don't follow the fabtests coventions for
naming, config file, and command line options.

### Dmabuf RDMA tests

These tests check the functionality or performance of dmabuf based GPU
RDMA mechanism. They use oneAPI level-zero API to allocate buffer from
device memory, get dmabuf handle, and perform some device memory related
operations. Run with the *-h* option to see all available options for
each of the tests.

*xe_rdmabwe*
: This Verbs test measures the bandwidth of RDMA operations. It runs in
  client-server mode. It has options to choose buffer location, test type
  (write, read, send/recv), device unit(s), NIC unit(s), message size, and
  the number of iterations per message size.

*fi_xe_rdmabw*
: This test is similar to *xe_rdmabw*, but uses libfabric instead of Verbs.

*xe_mr_reg*
: This Verbs test tries to register a buffer with the RDMA NIC.

*fi_xe_mr_reg*
: This test is similar to *xe_mr_reg*, but uses libfabric instead of Verbs.

*xe_memcopy*
: This test measures the performance of memory copy operations between
  buffers. It has options for buffer locations, as well as memory copying
  methods to use (memcpy, mmap + memcpy, copy with device command queue, etc).

### Other component tests

*sock_test*
: This client-server test establishes socket connections and tests the
  functionality of select/poll/epoll with different set sizes.

## Config file options

The following keys and respective key values may be used in the config file.

*prov_name*
: Identify the provider(s) to test.  E.g. udp, tcp, verbs,
  ofi_rxm;verbs, ofi_rxd;udp.

*test_type*
: FT_TEST_LATENCY, FT_TEST_BANDWIDTH, FT_TEST_UNIT

*test_class*
: FT_CAP_MSG, FT_CAP_TAGGED, FT_CAP_RMA, FT_CAP_ATOMIC

*class_function*
: For FT_CAP_MSG and FT_CAP_TAGGED: FT_FUNC_SEND, FT_FUNC_SENDV, FT_FUNC_SENDMSG,
  FT_FUNC_INJECT, FT_FUNC_INJECTDATA, FT_FUNC_SENDDATA

  For FT_CAP_RMA: FT_FUNC_WRITE, FT_FUNC_WRITEV, FT_FUNC_WRITEMSG,
  FT_FUNC_WRITEDATA, FT_FUNC_INJECT_WRITE, FT_FUNC_INJECT_WRITEDATA,
  FT_FUNC_READ, FT_FUNC_READV, FT_FUNC_READMSG

  For FT_CAP_ATOMIC: FT_FUNC_ATOMIC, FT_FUNC_ATOMICV, FT_FUNC_ATOMICMSG,
  FT_FUNC_INJECT_ATOMIC, FT_FUNC_FETCH_ATOMIC, FT_FUNC_FETCH_ATOMICV,
  FT_FUNC_FETCH_ATOMICMSG, FT_FUNC_COMPARE_ATOMIC, FT_FUNC_COMPARE_ATOMICV,
  FT_FUNC_COMPARE_ATOMICMSG

*constant_caps - values OR'ed together*
: FI_RMA, FI_MSG, FI_SEND, FI_RECV, FI_READ,
  FI_WRITE, FI_REMOTE_READ, FI_REMOTE_WRITE, FI_TAGGED, FI_DIRECTED_RECV

*mode - values OR'ed together*
: FI_CONTEXT, FI_RX_CQ_DATA

*ep_type*
: FI_EP_MSG, FI_EP_DGRAM, FI_EP_RDM

*comp_type*
: FT_COMP_QUEUE, FT_COMP_CNTR, FT_COMP_ALL

*av_type*
: FI_AV_MAP, FI_AV_TABLE

*eq_wait_obj*
: FI_WAIT_NONE, FI_WAIT_UNSPEC, FI_WAIT_FD, FI_WAIT_MUTEX_COND

*cq_wait_obj*
: FI_WAIT_NONE, FI_WAIT_UNSPEC, FI_WAIT_FD, FI_WAIT_MUTEX_COND

*cntr_wait_obj*
: FI_WAIT_NONE, FI_WAIT_UNSPEC, FI_WAIT_FD, FI_WAIT_MUTEX_COND

*threading*
: FI_THREAD_UNSPEC, FI_THREAD_SAFE, FI_THREAD_FID, FI_THREAD_DOMAIN,
  FI_THREAD_COMPLETION, FI_THREAD_ENDPOINT

*progress*
: FI_PROGRESS_MANUAL, FI_PROGRESS_AUTO, FI_PROGRESS_UNSPEC

*mr_mode*
: (Values OR'ed together) FI_MR_LOCAL, FI_MR_VIRT_ADDR, FI_MR_ALLOCATED,
  FI_MR_PROV_KEY

*op*
: For FT_CAP_ATOMIC: FI_MIN, FI_MAX, FI_SUM, FI_PROD, FI_LOR, FI_LAND, FI_BOR,
  FI_BAND, FI_LXOR, FI_BXOR, FI_ATOMIC_READ, FI_ATOMIC_WRITE, FI_CSWAP,
  FI_CSWAP_NE, FI_CSWAP_LE, FI_CSWAP_LT, FI_CSWAP_GE, FI_CSWAP_GT, FI_MSWAP

*datatype*
: For FT_CAP_ATOMIC: FI_INT8, FI_UINT8, FI_INT16, FI_UINT16, FI_INT32,
  FI_UINT32, FI_INT64, FI_UINT64, FI_FLOAT, FI_DOUBLE, FI_FLOAT_COMPLEX,
  FI_DOUBLE_COMPLEX, FI_LONG_DOUBLE, FI_LONG_DOUBLE_COMPLEX

*msg_flags - values OR'ed together*
: For FT_FUNC_[SEND,WRITE,READ,ATOMIC]MSG: FI_REMOTE_CQ_DATA, FI_COMPLETION

*rx_cq_bind_flags - values OR'ed together*
: FI_SELECTIVE_COMPLETION

*tx_cq_bind_flags - values OR'ed together*
: FI_SELECTIVE_COMPLETION

*rx_op_flags - values OR'ed together*
: FI_COMPLETION

*tx_op_flags - values OR'ed together*
: FI_COMPLETION

*test_flags - values OR'ed together*
: FT_FLAG_QUICKTEST

# HOW TO RUN TESTS

(1) Fabtests requires that libfabric be installed on the system, and at least one provider be usable.

(2) Install fabtests on the system. By default all the test executables are installed in /usr/bin directory unless specified otherwise.

(3) All the client-server tests have the following usage model:

	fi_<testname> [OPTIONS]		start server
	fi_<testname> <host>		connect to server

# COMMAND LINE OPTIONS

Tests share command line options where appropriate.  The following
command line options are available for one or more test.  To see which
options apply for a given test, you can use the '-h' help option to see
the list available for that test.

*-h*
: Displays help output for the test.

*-f <fabric>*
: Restrict test to the specified fabric name.

*-d <domain>*
: Restrict test to the specified domain name.

*-p <provider>*
: Restrict test to the specified provider name.

*-e <ep_type>*
: Use the specified endpoint type for the test.  Valid options are msg,
  dgram, and rdm.  The default endpoint type is rdm.

*-D <device_name>*
: Allocate data buffers on the specified device, rather than in host
  memory.  Valid options are ze and cuda.

*-a <address vector name>*
: The name of a shared address vector.  This option only applies to tests
  that support shared address vectors.

*-B <src_port>*
: Specifies the port number of the local endpoint, overriding the default.

*-C <num_connections>*
: Specifies the number of simultaneous connections or communication
  endpoints to the server.

*-P <dst_port>*
: Specifies the port number of the peer endpoint, overriding the default.

*-s <address>*
: Specifies the address of the local endpoint.

*-F <address_format>
: Specifies the address format.

*-K
: Fork a child process after initializing endpoint.

*-b[=oob_port]*
: Enables out-of-band (via sockets) address exchange and test
  synchronization.  A port for the out-of-band connection may be specified
  as part of this option to override the default.  When specified, the
  input src_addr and dst_addr values are relative to the OOB socket
  connection, unless the -O option is also specified.

*-E[=oob_port]*
: Enables out-of-band (via sockets) address exchange only. A port for the
  out-of-band connection may be specified as part of this option to override
  the default. Cannot be used together with the '-b' option.  When specified,
  the input src_addr and dst_addr values are relative to the OOB socket
  connection, unless the -O option is also specified.

*-U*
: Run fabtests with FI_DELIVERY_COMPLETE.

*-I <number>*
: Number of data transfer iterations.

*-Q*
: Associated any EQ with the domain, rather than directly with the EP.

*-w <number>*
: Number of warm-up data transfer iterations.

*-S <size>*
: Data transfer size or 'all' for a full range of sizes.  By default a
  select number of sizes will be tested.

*-l*
: If specified, the starting address of transmit and receive buffers will
  be aligned along a page boundary.

*-m*
: Use machine readable output.  This is useful for post-processing the test
  output with scripts.

*-t <comp_type>*
: Specify the type of completion mechanism to use.  Valid values are queue
  and counter.  The default is to use completion queues.

*-c <comp_method>*
: Indicate the type of processing to use checking for completed operations.
  Valid values are spin, sread, and fd.  The default is to busy wait (spin)
  until the desired operation has completed.  The sread option indicates that
  the application will invoke a blocking read call in libfabric, such as
  fi_cq_sread.  Fd indicates that the application will retrieve the native
  operating system wait object (file descriptor) and use either poll() or
  select() to block until the fd has been signaled, prior to checking for
  completions.

*-o <op>*
: For RMA based tests, specify the type of RMA operation to perform.  Valid
  values are read, write, and writedata.  Write operations are the default.
  For message based, tests, specify whether msg (default) or tagged transfers
  will be used.

*-M <mcast_addr>*
: For multicast tests, specifies the address of the multicast group to join.

*-u <test_config_file>*
: Specify the input file to use for test control.  This is specified at the
  client for fi_ubertest and fi_rdm_stress and controls the behavior of the
  testing.

*-v*
: Add data verification check to data transfers.

*-O <addr>*
: Specify the out of band address to use, mainly useful if the address is not
  an IP address.  If given, the src_addr and dst_addr address parameters will
  be passed through to the libfabric provider for interpretation.

# USAGE EXAMPLES

## A simple example

	run server: <test_name> -p <provider_name> -s <source_addr>
		e.g.	fi_msg_rma -p sockets -s 192.168.0.123
	run client: <test_name> <server_addr> -p <provider_name>
		e.g.	fi_msg_rma 192.168.0.123 -p sockets

## An example with various options

	run server: fi_rdm_atomic -p psm3 -s 192.168.0.123 -I 1000 -S 1024
	run client: fi_rdm_atomic 192.168.0.123 -p psm3 -I 1000 -S 1024

This will run "fi_rdm_atomic" for all atomic operations with

	- PSM3 provider
	- 1000 iterations
	- 1024 bytes message size
	- server node as 123.168.0.123

## Run multinode tests

	Server and clients are invoked with the same command:
		fi_multinode -n <number of processes> -s <server_addr> -C <mode>

	A process on the server must be started before any of the clients can be started
	succesfully. -C lists the mode that the tests will run in. Currently the options are
  for rma and msg. If not provided, the test will default to msg.

## Run fi_rdm_stress

  run server: fi_rdm_stress
  run client: fi_rdm_stress -u fabtests/test_configs/rdm_stress/stress.json 127.0.0.1

## Run fi_ubertest

	run server: fi_ubertest
	run client: fi_ubertest -u fabtests/test_configs/tcp/all.test 127.0.0.1

This will run "fi_ubertest" with

	- tcp provider
	- configurations defined in fabtests/test_configs/tcp/all.test
	- server running on the same node

Usable config files are provided in fabtests/test_configs/<provider_name>.

For more usage options: fi_ubertest -h

## Run the whole fabtests suite

A runscript scripts/runfabtests.sh is provided that runs all the tests
in fabtests and reports the number of pass/fail/notrun.

	Usage: runfabtests.sh [OPTIONS] [provider] [host] [client]

By default if none of the options are provided, it runs all the tests using

	- sockets provider
	- 127.0.0.1 as both server and client address
	- for small number of optiond and iterations

Various options can be used to choose provider, subset tests to run,
level of verbosity etc.

	runfabtests.sh -vvv -t all psm3 192.168.0.123 192.168.0.124

This will run all fabtests using

	- psm3 provider
	- for different options and larger iterations
	- server node as 192.168.0.123 and client node as 192.168.0.124
	- print test output for all the tests

For detailed usage options: runfabtests.sh -h




Libfabric 程序员手册
fabtests(7) Fabtests 程序员手册
姓名
最佳测试

概要
Fabtests 是面向结构提供商的一组示例，演示了 libfabric-高性能结构软件库的各种功能。

概述
Libfabric 定义了结构提供商可以支持的接口集。Fabtests 示例的目的是演示一些主要功能。目标是让用户熟悉 libfabric 提供的不同功能以及如何使用它们。尽管大多数测试都会报告性能数据，但它们旨在测试功能而不是性能。基准测试和 ubertest 是例外。

测试分为以下几类。除了单元测试之外，所有测试都是客户端-服务器测试。并非所有提供商都会支持每项测试。

测试名称试图表明每个测试正在验证的功能类型。尽管某些测试适用于任何端点类型，但许多测试仅限于验证单个端点类型。这些测试通常包括端点类型作为测试名称的一部分，例如 dgram、msg 或 rdm。

功能性
这些测试是非常基本的功能测试的组合，显示了 libfabric 的主要功能。

fi_av_xfer
当在本地地址向量中插入和删除地址时，测试未连接端点的通信。
fi_cm_数据
验证交换 CM 数据作为连接端点的一部分。
fi_cq_数据
使用 CQ 数据传输消息。
fi_dgram
基本数据报端点示例。
fi_dgram_waitset
使用等待集传输数据报以获取完成通知。
fi_inj_complete
使用 FI_INJECT_COMPLETE 操作标志发送消息。
广播电视
一个简单的多播测试。
fi_msg
基本消息端点示例。
fi_msg_epoll
使用配置为使用文件描述符作为等待对象的完成队列传输消息。文件描述符由程序检索并直接与 Linux epoll API 一起使用。
fi_msg_sockets
验证分配给无源端点的地址是否可以转换为主动端点。这是需要 RDMA 实现上的套接字 API 语义（例如 rsockets）的应用程序所必需的。
fi_multi_ep
在多个端点上并行执行数据传输。
fi_multi_mr
使用入站写入的完成计数器作为通知机制，向多个内存区域发出 RMA 写入操作。
网络投票
使用轮询集通过 RDM 端点交换数据以驱动完成通知。
fi_rdm
基本 RDM 端点示例。
fi_rdm_atomic
测试和验证 RDM 端点上的原子操作。
fi_rdm_deferred_wq
测试触发操作和延迟工作队列支持。
fi_rdm_多域
在多个端点上执行数据传输，每个端点属于不同的开放域。
fi_rdm_multi_recv
通过 RDM 端点传输多个消息，这些消息接收到单个缓冲区中，并使用 FI_MULTI_RECV 标志发布。
fi_rdm_rma_simple
通过 RDM 端点的简单 RMA 写入示例。
fi_rdm_rma_trigger
对 RMA 写入操作进行排队的基本示例，该操作在触发完成时启动。与 RDM 端点配合使用。
fi_rdm_shared_av
生成子进程来验证与 RDM 端点一起使用共享地址向量的基本功能。
fi_rdm_tagged_peek
使用带有标记消息的 FI_PEEK 操作标志的基本测试。与 RDM 端点配合使用。
fi_recv_取消
测试取消已标记消息的已发布接收。
fi_resmgmt_测试
测试资源管理启用功能。这将验证提供程序是否可以防止应用程序超出本地和远程命令队列以及完成队列。这对应于将域属性resource_mgmt设置为FI_RM_ENABLED。
fi_scalable_ep
在可扩展端点、与多个传输和接收上下文关联的端点上执行数据传输。
fi_shared_ctx
在多个端点之间执行数据传输，其中端点共享传输和/或接收上下文。
fi_unexpected_msg
测试意外标记消息的发送和接收处理。
fi_unmap_mem
测试数据传输，其中传输缓冲区在每次传输之间进行映射和取消映射，但传输缓冲区的虚拟地址尝试保持相同。此测试用于验证内存注册缓存的正确行为。
fi_bw
执行单侧带宽测试，并提供数据验证选项。可以启用接收方的睡眠时间，以便发送方领先于接收方。
基准测试
客户端和服务器以 ping-pong 方式交换消息（对于 pingpong 命名测试），或者以单向传输消息（对于 bw 命名测试）。这些测试可以传输各种大小的消息，控制测试使用哪些功能，并报告性能数据。测试是根据 OSU MPI 提供的基准构建的。他们不保证提供给定提供商或系统可能实现的最佳延迟或带宽性能数字。

fi_dgram_pingpong
数据报端点的延迟测试
fi_msg_bw
连接 (MSG) 端点的消息传输带宽测试。
fi_msg_pingpong
连接 (MSG) 端点的消息传输延迟测试。
fi_rdm_cntr_pingpong
使用计数器作为完成机制的可靠数据报 (RDM) 端点的消息传输延迟测试。
fi_rdm_乒乓球
可靠数据报 (RDM) 端点的消息传输延迟测试。
fi_rdm_tagged_bw
可靠数据报 (RDM) 端点的标记消息带宽测试。
fi_rdm_tagged_pingpong
可靠数据报 (RDM) 端点的标记消息延迟测试。
fi_rma_bw
针对可靠（MSG 和 RDM）端点的 RMA 读写带宽测试。
单元
这些是简单的单方面单元测试，用于验证 API 的基本行为。由于这些是不执行数据传输的单一系统测试，因此其测试范围受到限制。

fi_av_测试
验证地址向量接口。
fi_cntr_测试
测试对抗创造和破坏。
fi_cq_测试
测试完成队列的创建和销毁。
fi_dom_测试
测试域的创建和销毁。
fi_eq_测试
测试事件队列的创建、销毁和功能。
fi_getinfo_test
使用不同的提示测试提供程序对 fi_getinfo 调用的响应。
fi_mr_测试
测试内存注册。
fi_资源_释放
分配并关闭结构资源以检查是否进行了正确的清理。
多节点
此测试对多种格式和模式运行一系列测试，以帮助大规模验证。这些模式是全对全、一对全、全对一和环。这些测试还跨多种功能运行，例如消息、rma、原子和标记消息。目前，还没有独立运行这些功能和模式的选项，但测试时间足够短，可以一次全部运行。

Ubertest
这是一项全面的延迟、带宽和功能测试，可以处理各种测试配置。该测试能够通过迭代大量测试变量来运行大量测试。因此，完整的 ubertest 运行可能需要大量时间。由于 ubertest 迭代输入变量，因此它依赖于测试配置文件进行控制，而不是其他 fabtest 使用的大量命令行选项。必须为每个提供者构建一个配置文件。示例测试配置位于 test_configs。

fi_uber测试
此测试采用配置文件作为输入。该文件包含要迭代的变量及其值的列表。该测试将针对给定的提供商运行一组延迟、带宽和功能测试。它将对所有变量的每一种可能的组合执行一次执行。例如，如果有 8 个测试变量，其中 6 个有 2 个可能值，2 个有 3 个可能值，则 ubertest 将执行每个测试的总共 576 次迭代。
配置文件选项
配置文件中可以使用以下键和相应的键值。

提供者名
确定要测试的提供商。例如 udp、tcp、动词、ofi_rxm；verbs；ofi_rxd；udp。
测试类型
FT_TEST_LATENCY、FT_TEST_BANDWIDTH、FT_TEST_UNIT
测试类
FT_CAP_MSG、FT_CAP_TAGGED、FT_CAP_RMA、FT_CAP_ATOMIC
类函数
对于 FT_CAP_MSG 和 FT_CAP_TAGGED：FT_FUNC_SEND、FT_FUNC_SENDV、FT_FUNC_SENDMSG、FT_FUNC_INJECT、FT_FUNC_INJECTDATA、FT_FUNC_SENDDATA
对于 FT_CAP_RMA：FT_FUNC_WRITE、FT_FUNC_WRITEV、FT_FUNC_WRITEMSG、FT_FUNC_WRITEDATA、FT_FUNC_INJECT_WRITE、FT_FUNC_INJECT_WRITEDATA FT_FUNC_READ、FT_FUNC_READV、FT_FUNC_READMSG

对于 FT_CAP_ATOMIC：FT_FUNC_ATOMIC、FT_FUNC_ATOMICV、FT_FUNC_ATOMICMSG、FT_FUNC_INJECT_ATOMIC、FT_FUNC_FETCH_ATOMIC、FT_FUNC_FETCH_ATOMICV、FT_FUNC_FETCH_ATOMICMSG、FT_FUNC_COMPARE_ATOMIC、FT_FUNC_COMPARE _ATOMICV，FT_FUNC_COMPARE_ATOMICMSG

Constant_caps - 值或运算在一起
FI_RMA、FI_MSG、FI_SEND、FI_RECV、FI_READ、FI_WRITE、FI_REMOTE_READ、FI_REMOTE_WRITE、FI_TAGGED、FI_DIRECTED_RECV
mode - 值或运算在一起
FI_上下文、FI_RX_CQ_数据
EP_类型
FI_EP_MSG、FI_EP_DGRAM、FI_EP_RDM
补偿类型
FT_COMP_QUEUE、FT_COMP_CNTR、FT_COMP_ALL
AV类型
FI_AV_MAP、FI_AV_TABLE
eq_wait_obj
FI_WAIT_NONE、FI_WAIT_UNSPEC、FI_WAIT_FD、FI_WAIT_MUTEX_COND
cq_wait_obj
FI_WAIT_NONE、FI_WAIT_UNSPEC、FI_WAIT_FD、FI_WAIT_MUTEX_COND
cntr_等待_obj
FI_WAIT_NONE、FI_WAIT_UNSPEC、FI_WAIT_FD、FI_WAIT_MUTEX_COND
线程
FI_THREAD_UNSPEC、FI_THREAD_SAFE、FI_THREAD_FID、FI_THREAD_DOMAIN、FI_THREAD_COMPLETION、FI_THREAD_ENDPOINT
进步
FI_PROGRESS_MANUAL、FI_PROGRESS_AUTO、FI_PROGRESS_UNSPEC
先生模式
（值“或”在一起）FI_MR_LOCAL、FI_MR_VIRT_ADDR、FI_MR_ALLOCATED、FI_MR_PROV_KEY
操作
对于 FT_CAP_ATOMIC：FI_MIN、FI_MAX、FI_SUM、FI_PROD、FI_LOR、FI_LAND、FI_BOR、FI_BAND、FI_LXOR、FI_BXOR、FI_ATOMIC_READ、FI_ATOMIC_WRITE、FI_CSWAP、FI_CSWAP_NE、FI_CSWAP_LE、FI_CSWAP_LT、 FI_CSWAP_GE、FI_CSWAP_GT、FI_MSWAP
数据类型
对于 FT_CAP_ATOMIC：FI_INT8、FI_UINT8、FI_INT16、FI_UINT16、FI_INT32、FI_UINT32、FI_INT64、FI_UINT64、FI_FLOAT、FI_DOUBLE、FI_FLOAT_COMPLEX、FI_DOUBLE_COMPLEX、FI_LONG_DOUBLE、FI_LONG_DOU BLE_COMPLE
msg_flags - 值或运算在一起
对于 FT_FUNC_XXXMSG：FI_REMOTE_CQ_DATA、FI_COMPLETION
rx_cq_bind_flags - 值或运算在一起
FI_SELECTIVE_COMPLETION
tx_cq_bind_flags - 值或运算在一起
FI_SELECTIVE_COMPLETION
rx_op_flags - 值或运算在一起
FI_COMPLETION
tx_op_flags - 值或运算在一起
FI_COMPLETION
test_flags - 值或运算在一起
FT_FLAG_QUICKTEST
如何运行测试
(1) Fabtests 要求系统上安装 libfabric，并且至少有一个可用的提供程序。

(2) 在系统上安装fabtests。默认情况下，除非另有指定，所有测试可执行文件都安装在 /usr/bin 目录中。

(3) 所有客户端-服务器测试都有以下使用模型：

fi_<testname> [OPTIONS]		start server
fi_<testname> <host>		connect to server
命令行选项
测试在适当的情况下共享命令行选项。以下命令行选项可用于一项或多项测试。要查看哪些选项适用于给定测试，您可以使用“-h”帮助选项来查看可用于该测试的列表。

-H
显示测试的帮助输出。
*-F*
将测试限制为指定的结构名称。
*-d*
将测试限制到指定的域名。
*-p*
将测试限制为指定的提供者名称。
*-e*
使用指定的端点类型进行测试。有效选项包括 msg、dgram 和 rdm。默认端点类型是 rdm。
-a <地址向量名称>
共享地址向量的名称。此选项仅适用于支持共享地址向量的测试。
*-B*
指定本地端点的端口号，覆盖默认值。
*-P*
指定对等端点的端口号，覆盖默认值。
-s <地址>
指定本地端点的地址。
-b[=oob_端口]
启用带外（通过套接字）地址交换和测试同步。带外连接的端口可以指定为该选项的一部分以覆盖默认值。
-E[=oob_端口]
仅启用带外（通过套接字）地址交换。带外连接的端口可以指定为该选项的一部分以覆盖默认值。不能与“-b”选项一起使用。
*-我*
数据传输迭代次数。
*-w*
预热数据传输迭代次数。
*-S*
数据传输大小或全范围大小的“全部”。默认情况下，将测试选定数量的尺寸。
-l
如果指定，发送和接收缓冲区的起始地址将沿页边界对齐。
-m
使用机器可读的输出。这对于使用脚本对测试输出进行后处理非常有用。
*-t*
指定要使用的完成机制的类型。有效值为队列和计数器。默认是使用完成队列。
*-C*
指示用于检查已完成操作的处理类型。有效值为 spin、sread 和 fd。默认设置是忙等待（旋转），直到所需的操作完成。sread 选项指示应用程序将调用 libfabric 中的阻塞读取调用，例如 fi_cq_sread。Fd 指示应用程序将检索本机操作系统等待对象（文件描述符），并使用 poll() 或 select() 进行阻塞，直到收到 fd 信号为止，然后再检查完成情况。
*-o*
对于基于 RMA 的测试，指定要执行的 RMA 操作的类型。有效值为读、写和写数据。写操作是默认操作。
*-M*
对于多播测试，指定要加入的多播组的地址。
-v
在数据传输中添加数据验证检查。
使用示例
一个简单的例子
run server: <test_name> -p <provider_name> -s <source_addr>
	e.g.	fi_msg_rma -p sockets -s 192.168.0.123
run client: <test_name> <server_addr> -p <provider_name>
	e.g.	fi_msg_rma 192.168.0.123 -p sockets
具有各种选项的示例
run server: fi_rdm_atomic -p psm -s 192.168.0.123 -I 1000 -S 1024
run client: fi_rdm_atomic 192.168.0.123 -p psm -I 1000 -S 1024
这将为所有原子操作运行“fi_rdm_atomic”

- PSM provider
- 1000 iterations
- 1024 bytes message size
- server node as 123.168.0.123
运行多节点测试
Server and clients are invoked with the same command: 
	fi_multinode -n <number of processes> -s <server_addr> -C <mode>

A process on the server must be started before any of the clients can be started 
succesfully. -C lists the mode that the tests will run in. Currently the options are   for rma and msg. If not provided, the test will default to msg. 
运行 fi_ubertest
run server: fi_ubertest
run client: fi_ubertest -u /usr/share/fabtests/test_configs/sockets/quick.test 192.168.0.123
这将运行“fi_ubertest”

- sockets provider
- configurations defined in /usr/share/fabtests/test_configs/sockets/quick.test
- server node as 192.168.0.123
/test_configs 中为套接字、动词、udp 和 usnic 提供程序提供了配置文件，并随 fabtests 安装一起分发。

有关更多使用选项：fi_ubertest -h

运行整个 fabtests 套件
提供了一个 runscript scripts/runfabtests.sh，用于运行 fabtests 中的所有测试并报告通过/失败/未运行的数量。

Usage: runfabtests.sh [OPTIONS] [provider] [host] [client]
默认情况下，如果未提供任何选项，它将使用以下命令运行所有测试

- sockets provider
- 127.0.0.1 as both server and client address
- for small number of optiond and iterations
可以使用各种选项来选择提供程序、要运行的子集测试、详细级别等。

runfabtests.sh -vvv -t all psm 192.168.0.123 192.168.0.124
这将使用以下命令运行所有 fabtests

- psm provider
- for different options and larger iterations
- server node as 192.168.0.123 and client node as 192.168.0.124
- print test output for all the tests
有关详细使用选项：runfabtests.sh -h

© 2023 OpenFabrics 接口工作组在Jekyll Bootstrap 和Twitter Bootstrap的帮助下

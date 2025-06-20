/*
 *
 * This software is available to you under the BSD license
 * below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <rdma/fabric.h>
#include <rdma/fi_domain.h>
#include <rdma/fi_endpoint.h>
#include <rdma/fi_cm.h>
#include <shared.h>

//Build with
//gcc -o example_msg example_msg.c -L<path to libfabric lib> -I<path to libfabric include> -lfabric
//gcc -o example_msg example_msg.c -L/home/xb/project/libfabric/libfabric/build/lib -I/home/xb/project/libfabric/libfabric/build/include -I/home/xb/project/libfabric/libfabric/build/include -lfabric

#define BUF_SIZE 64

char *dst_addr = NULL;
char *port = "9228";
struct fi_info *hints, *info, *fi_pep;
struct fid_fabric *fabric = NULL;
struct fid_domain *domain = NULL;
struct fid_ep *ep = NULL;
struct fid_pep *pep = NULL;
struct fid_cq *cq = NULL;
struct fid_eq *eq = NULL;
struct fi_cq_attr cq_attr = {0};
struct fi_eq_attr eq_attr = {
	.wait_obj = FI_WAIT_UNSPEC
};
//const struct sockaddr_in *sin;
char str_addr[INET_ADDRSTRLEN];
int ret;
char buffer[BUF_SIZE];
fi_addr_t fi_addr = FI_ADDR_UNSPEC;
struct fi_eq_cm_entry entry;
uint32_t event;
ssize_t rd;

/* Initializes all basic OFI resources to allow for a server/client to exchange a message */
static int start_client(void)
{

	ret = fi_getinfo(FI_VERSION(1,9), dst_addr, port, dst_addr ? 0 : FI_SOURCE,
			hints, &info);
	if (ret) {
	        printf("fi_getinfo: %d\n", ret);
	        return ret;
	}

	ret = fi_fabric(info->fabric_attr, &fabric, NULL);
	if (ret) {
		printf("fi_fabric: %d\n", ret);
        	return ret;
	}

	ret = fi_eq_open(fabric, &eq_attr, &eq, NULL);
	if (ret) {
	        printf("fi_eq_open: %d\n", ret);
	        return ret;
	}

	ret = fi_domain(fabric, info, &domain, NULL);
	if (ret) {
        	printf("fi_domain: %d\n", ret);
        	return ret;
        }

	/* Initialize our completion queue. Completion queues are used to report events associated
	 * with data transfers. In this example, we use one CQ that tracks sends and receives, but
	 * often times there will be separate CQs for sends and receives.
	 */
	cq_attr.size = 128;
	cq_attr.format = FI_CQ_FORMAT_MSG;
	ret = fi_cq_open(domain, &cq_attr, &cq, NULL);
	if (ret) {
		printf("fi_cq_open error (%d)\n", ret);
		return ret;
	}

	/* Bind our CQ to our endpoint to track any sends and receives that come in or out on that endpoint.
	 * A CQ can be bound to multiple endpoints but one EP can only have one send CQ and one receive CQ
	 * (which can be the same CQ).
	 */

	ret = fi_endpoint(domain, info, &ep, NULL);
	if (ret) {
	        printf("fi_endpoint: %d\n", ret);
	        return ret;
	}

	ret = fi_ep_bind(ep, &cq->fid, FI_SEND | FI_RECV);
        if (ret) {
            printf("fi_ep_bind cq error (%d)\n", ret);
            return ret;
        }

	ret = fi_ep_bind((ep), &(eq)->fid, 0);
	if (ret) {
		printf("fi_ep_bind: %d\n", ret);
		return ret;
	}

	ret = fi_enable(ep);
        if (ret) {
		printf("fi_enable: %d\n", ret);
	        return ret;
	}

	ret = fi_connect(ep, info->dest_addr, NULL, 0);
	if (ret) {
 	        printf("fi_connect: %d\n", ret);
 	        return ret;
	}

    rd = fi_eq_sread(eq, &event, &entry, sizeof(entry), -1, 0);
    if (rd != sizeof(entry)) {
        ret = (int) rd;
        printf("fi_eq_sread: %d\n", ret);
        return ret;
	}

	return 0;
}

static int start_server(void)
{
	// const struct sockaddr_in *sin;

	/* The first OFI call to happen for initialization is fi_getinfo which queries libfabric
	 * and returns any appropriate providers that fulfill the hints requirements. Any applicable
	 * providers will be returned as a list of fi_info structs (&info). Any info can be selected.
	 * In this test we select the first fi_info struct. Assuming all hints were set appropriately,
	 * the first fi_info should be most appropriate.
	 * The flag FI_SOURCE is set for the server to indicate that the address/port refer to source
	 * information. This is not set for the client because the fields refer to the server, not
	 * the caller (client). */
	/* 初始化时发生的第一个 OFI 调用是 fi_getinfo，它查询 libfabric 并返回满足提示要求的任何适当的提供程序。 任何适用的提供程序都将作为 fi_info 结构 (&info) 列表返回。 可以选择任何信息。 在此测试中，我们选择第一个 fi_info 结构。 假设所有提示均已正确设置，第一个 fi_info 应该是最合适的。 为服务器设置标志FI_SOURCE以指示地址/端口引用源信息。 这不是为客户端设置的，因为这些字段引用服务器，而不是调用者（客户端） */
	ret = fi_getinfo(FI_VERSION(1,9), dst_addr, port, dst_addr ? 0 : FI_SOURCE,
			 		hints, &fi_pep);
	if (ret) {
		printf("fi_getinfo error (%d)\n", ret);
		return ret;
	}

	/* Initialize our fabric. The fabric network represents a collection of hardware and software
	 * resources that access a single physical or virtual network. All network ports on a system
	 * that can communicate with each other through their attached networks belong to the same fabric.
	 */
	ret = fi_fabric(fi_pep->fabric_attr, &fabric, NULL); // 打开fabric, 初始化任何资源前需要打开fabric
	if (ret) {
		printf("fi_fabric error (%d)\n", ret);
		return ret;
	}

	/* Initialize our endpoint. Endpoints are transport level communication portals which are used to
	 * initiate and drive communication. There are three main types of endpoints:
	 * FI_EP_MSG - connected, reliable
	 * FI_EP_RDM - unconnected, reliable
	 * FI_EP_DGRAM - unconnected, unreliable
	 * The type of endpoint will be requested in hints/fi_getinfo. Different providers support different
	 * types of endpoints. TCP supports only FI_EP_MSG but when used with RxM, can support FI_EP_RDM.
	 * In this application, we requested TCP and FI_EP_MSG.
	 */

    ret = fi_eq_open(fabric, &eq_attr, &eq, NULL); // 打开事件队列EQ, 一般用于建连, 收发数据产生的事件
    if (ret) {
            printf("fi_eq_open: %d\n", ret);
            return ret;
    }

    ret = fi_passive_ep(fabric, fi_pep, &pep, NULL); // 打开被动端点, 常用与服务端监听端口, 支持多个客户端domain连接进来
    if (ret) {
        printf("fi_passive_ep: %d\n", ret);
        return ret;
    }

    ret = fi_pep_bind(pep, &eq->fid, 0); // 为端点绑定事件队列
    if (ret) {
        printf("fi_pep_bind %d", ret);
        return ret;
    }

    ret = fi_listen(pep); // 监听端点, 等待客户端连接请求
    if (ret) {
        printf("fi_listen %d", ret);
        return ret;
    }

	return 0;
}

static int complete_connection(void)
{

	rd = fi_eq_sread(eq, &event, &entry, sizeof(entry), -1, 0); // 等待读取客户端触发的服务端事件, 读取事件, 推动进展(驱动程序运转)
    if (rd != sizeof entry) {
	ret = (int) rd;
            printf("fi_eq_sread: %d", ret);
		if (ret)
			goto err;
    }

	ret = fi_domain(fabric, info, &domain, NULL); // domain域用于将资源分组, 可基于域来做管理
	if (ret) {
		printf("fi_domain: %d\n", ret);
	        return ret;
        }

	ret = fi_domain_bind(domain, &eq->fid, 0);
	if (ret) {
	        printf("fi_domain_bind: %d\n", ret);
	        return ret;
	}

	/* 
	 * Initialize our completion queue. Completion queues are used to report events associated
         * with data transfers. In this example, we use one CQ that tracks sends and receives, but
         * often times there will be separate CQs for sends and receives.
	 */
    cq_attr.size = 128;
    cq_attr.format = FI_CQ_FORMAT_MSG;
    ret = fi_cq_open(domain, &cq_attr, &cq, NULL);
    if (ret) {
        printf("fi_cq_open error (%d)\n", ret);
        return ret;
    }

    /* Bind our CQ to our endpoint to track any sends and receives that 
	 * come in or out on that endpoint. A CQ can be bound to multiple
	 * endpoints but one EP can only have one send CQ and one receive CQ
     * (which can be the same CQ).
	*/

	ret = fi_endpoint(domain, info, &ep, NULL); // 用于客户端, 主动端点, 发起建连
        if (ret) {
                printf("fi_endpoint: %d\n", ret);
                return ret;
        }

	ret = fi_ep_bind(ep, &cq->fid, FI_SEND | FI_RECV);
        if (ret) {
                printf("fi_ep_bind cq error (%d)\n", ret);
                return ret;
        }

	ret = fi_ep_bind((ep), &(eq)->fid, 0);
        if (ret) {
                printf("fi_ep_bind: %d\n", ret);
                return ret;
        }

	ret = fi_enable(ep);
        if (ret) {
			printf("fi_enable: %d", ret);
            return ret;
		}

	ret = fi_accept(ep, NULL, 0);
	if (ret) {
	        printf("fi_accept: %d\n", ret);
	        return ret;
	}

	rd = fi_eq_sread(eq, &event, &entry, sizeof(entry), -1, 0);
	if (rd != sizeof(entry)) {
		ret = (int) rd;
		printf("fi_eq_read: %d\n", ret);
		return ret;
	}
    return 0;

err:
    if (info)
    	fi_reject(pep, info->handle, NULL, 0);
    return ret;

}

static void cleanup(void)
{
	int ret;
	
	/* All OFI resources are cleaned up using the same fi_close(fid) call. */
	if (ep) {
		ret = fi_close(&ep->fid);
		if (ret)
			printf("warning: error closing EP (%d)\n", ret);
	}
	if (pep) {
		ret = fi_close(&pep->fid);
		if (ret)
			printf("warning: error closing PEP (%d)\n", ret);
	}

	ret = fi_close(&cq->fid);
	if (ret)
		printf("warning: error closing CQ (%d)\n", ret);
	
	ret = fi_close(&domain->fid);
	if (ret)
		printf("warning: error closing domain (%d)\n", ret);

	ret = fi_close(&eq->fid);
        if (ret)
            printf("warning: error closing EQ (%d)\n", ret);

	ret = fi_close(&fabric->fid);
        if (ret)
            printf("warning: error closing fabric (%d)\n", ret);

	if (info)
		fi_freeinfo(info);

	if (fi_pep)
        fi_freeinfo(fi_pep);
}

/* Post a receive buffer. This call does not ensure a message has been received, just
 * that a buffer has been passed to OFI for the next message the provider receives.
 * Receives may be directed or undirected using the address parameter. Here, we
 * pass in the fi_addr but note that the server has not inserted the client's
 * address into its AV, so the address is still FI_ADDR_UNSPEC, indicating that
 * this buffer may receive incoming data from any address. An application may
 * set this to a real fi_addr if the buffer should only receive data from a certain
 * peer.
 * When posting a buffer, if the provider is not ready to process messages (because
 * of connection initialization for example), it may return -FI_EAGAIN. This does
 * not indicate an error, but rather that the application should try again later.
 * This is why we almost always wrap sends and receives in a do/while. Some providers
 * may need the application to drive progress in order to get out of the -FI_EAGAIN
 * loop. To drive progress, the application needs to call fi_cq_read (not necessarily
 * reading any completion entries).
 */
static int post_recv(void)
{
	int ret;

	do {
		ret = fi_recv(ep, buffer, BUF_SIZE, NULL, fi_addr, NULL);
		if (ret && ret != -FI_EAGAIN) {
			printf("error posting recv buffer (%d\n", ret);
			return ret;
		}
		if (ret == -FI_EAGAIN)
			(void) fi_cq_read(cq, NULL, 0);
	} while (ret);

	return 0;
}

/* Post a send buffer. This call does not ensure a message has been sent, just that
 * a buffer has been submitted to OFI to be sent. Unlike a receive buffer, a send
 * needs a valid fi_addr as input to tell the provider where to send the message.
 * Similar to the receive buffer posting porcess, when posting a send buffer, if the
 * provider is not ready to process messages, it may return -FI_EAGAIN. This does not
 * indicate an error, but rather that the application should try again later. Just like
 * the receive, we drive progress with fi_cq_read if this is the case.
 */
static int post_send(void)
{
	char *msg = "Hello, server! I am the client you've been waiting for!\0";
	int ret;

	(void) snprintf(buffer, BUF_SIZE, "%s", msg);

	do {
		ret = fi_send(ep, buffer, BUF_SIZE, NULL, fi_addr, NULL);
		if (ret && ret != -FI_EAGAIN) {
			printf("error posting send buffer (%d)\n", ret);
			return ret;
		}
		if (ret == -FI_EAGAIN)
			(void) fi_cq_read(cq, NULL, 0);
	} while (ret);

	return 0;
}

/* Wait for the message to be sent/received using the CQ. fi_cq_read not only drives progress
 * but also returns any completed events to notify the application that it can reuse
 * the send/recv buffer. The returned completion entry will have fields set to let the application
 * know what operation completed. Not all fields will be valid. The fields set will be indicated
 * by the cq format (when creating the CQ). In this example, we use FI_CQ_FORMAT_MSG in order to
 * use the flags field.
 */
static int wait_cq(void)
{
	struct fi_cq_err_entry comp;
	int ret;

	do {
		ret = fi_cq_read(cq, &comp, 1);
		if (ret < 0 && ret != -FI_EAGAIN) {
			printf("error reading cq (%d)\n", ret);
			return ret;
		}
	} while (ret != 1);

	if (comp.flags & FI_RECV)
		printf("I received a message!\n");
	else if (comp.flags & FI_SEND)
		printf("My message got sent!\n");

	return 0;
}

static int run(void)
{
	int ret;

	if (dst_addr) {
		printf("Client: send to server %s\n", dst_addr);

		ret = post_send();
		if (ret)
			return ret;

		ret = wait_cq();
		if (ret)
			return ret;

	} else {
		printf("Server: post buffer and wait for message from client\n");

		ret = post_recv();
		if (ret)
			return ret;

		ret = wait_cq();
		if (ret)
			return ret;

		printf("This is the message I received: %s\n", buffer);
	}
	return 1;
}

int main(int argc, char **argv)
{
	int ret;

	/* Hints are used to request support for specific features from a provider */
	hints = fi_allocinfo(); // 
	if (!hints)
		return EXIT_FAILURE;

	/* Server run with no args, client has server's address as an argument */
	dst_addr = argv[1];


//Set anything in hints that the application needs

	/* Request FI_EP_MSG (reliable datagram) endpoint which will allow us
	 * to reliably send messages to peers without having to listen/connect/accept. */
	hints->ep_attr->type = FI_EP_MSG; // 可靠数据报端点, 类似socket, 但无须执行listen/connect/accept

	/* Request basic messaging capabilities from the provider (no tag matching,
	 * no RMA, no atomic operations) */
	hints->caps = FI_MSG;

	/* Specifically request the tcp provider for the simple test */
	// hints->fabric_attr->prov_name = "tcp"; // 类似socket的, 面向连接的消息类型端点
 	hints->fabric_attr->prov_name = "ofi_rxm;verbs";

	/* Specifically request SOCKADDR_IN address format to simplify addressing for test */
	hints->addr_format = FI_SOCKADDR_IN;

	/* Default to FI_DELIVERY_COMPLETE which will make sure completions do not get generated
	 * until our message arrives at the destination. Otherwise, the client might get a completion
	 * and exit before the server receives the message. This is to make the test simpler */
	/* 默认为 FI_DELIVERY_COMPLETE，这将确保在我们的消息到达目的地之前不会生成完成(等待)。 否则，客户端可能会在服务器收到消息之前完成并退出。 这是为了让测试更简单 */
	hints->tx_attr->op_flags = FI_DELIVERY_COMPLETE;

//Done setting hints

	if (!dst_addr) {
		ret = start_server();
		if (ret) {
			goto out;
			return ret;
		}
	}

	ret = dst_addr ? start_client() : complete_connection();
	if (ret) {
		goto out;
		return ret;
	}

	ret = run();
out:
	cleanup();
	return ret;
}

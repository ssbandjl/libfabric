/*
 * Copyright (c) 2015-2016 Intel Corporation. All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * BSD license below:
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

#include "rxm.h"

#define RXM_EP_CAPS (FI_MSG | FI_RMA | FI_TAGGED | FI_DIRECTED_RECV |	\
		     FI_READ | FI_WRITE | FI_RECV | FI_SEND |		\
		     FI_REMOTE_READ | FI_REMOTE_WRITE | FI_SOURCE)

struct fi_tx_attr rxm_tx_attr = {
	.caps = RXM_EP_CAPS,
	.comp_order = FI_ORDER_STRICT,
	.inject_size = RXM_TX_DATA_SIZE,
	.size = SIZE_MAX,
	.iov_limit = RXM_IOV_LIMIT,
};

struct fi_rx_attr rxm_rx_attr = {
	.caps = RXM_EP_CAPS,
	.comp_order = FI_ORDER_STRICT,
	.size = 1024,
	.iov_limit= RXM_IOV_LIMIT,
};

struct fi_ep_attr rxm_ep_attr = {
	.type = FI_EP_RDM,
	.protocol = FI_PROTO_RXM,
	.protocol_version = 1,
	.max_msg_size = SIZE_MAX,
	.tx_ctx_cnt = 1,
	.rx_ctx_cnt = 1
};

struct fi_domain_attr rxm_domain_attr = {
	.threading = FI_THREAD_SAFE,
	.control_progress = FI_PROGRESS_AUTO,
	.data_progress = FI_PROGRESS_MANUAL,
	.resource_mgmt = FI_RM_ENABLED,
	.av_type = FI_AV_UNSPEC,
	/* Advertise support for FI_MR_BASIC so that ofi_check_info call
	 * doesn't fail at RxM level. If an app requires FI_MR_BASIC, it
	 * would be passed down to core provider. */
	.mr_mode = FI_MR_BASIC,
	.cq_data_size = sizeof_field(struct ofi_op_hdr, data),
	.cq_cnt = (1 << 16),
	.ep_cnt = (1 << 15),
	.tx_ctx_cnt = 1,
	.rx_ctx_cnt = 1,
	.max_ep_tx_ctx = 1,
	.max_ep_rx_ctx = 1,
	.mr_iov_limit = 1,
};

struct fi_fabric_attr rxm_fabric_attr = {
	.prov_version = FI_VERSION(RXM_MAJOR_VERSION, RXM_MINOR_VERSION),
};

struct fi_info rxm_info = {
	.caps = RXM_EP_CAPS,
	.addr_format = FI_SOCKADDR,
	.tx_attr = &rxm_tx_attr,
	.rx_attr = &rxm_rx_attr,
	.ep_attr = &rxm_ep_attr,
	.domain_attr = &rxm_domain_attr,
	.fabric_attr = &rxm_fabric_attr
};

struct util_prov rxm_util_prov = {
	.prov = &rxm_prov,
	.info = &rxm_info,
	.flags = 0,
};

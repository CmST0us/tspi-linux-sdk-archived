/*
 * net/tipc/subscr.h: Include file for TIPC network topology service
 *
 * Copyright (c) 2003-2017, Ericsson AB
 * Copyright (c) 2005-2007, 2012-2013, Wind River Systems
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the names of the copyright holders nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _TIPC_SUBSCR_H
#define _TIPC_SUBSCR_H

#include "topsrv.h"

#define TIPC_MAX_SUBSCR         65535
#define TIPC_MAX_PUBL           65535

struct tipc_subscription;
struct tipc_conn;

/**
 * struct tipc_subscription - TIPC network topology subscription object
 * @kref: reference count for this subscription
 * @net: network namespace associated with subscription
 * @timer: timer governing subscription duration (optional)
 * @service_list: adjacent subscriptions in name sequence's subscription list
 * @sub_list: adjacent subscriptions in subscriber's subscription list
 * @evt: template for events generated by subscription
 * @conid: connection identifier of topology server
 * @inactive: true if this subscription is inactive
 * @lock: serialize up/down and timer events
 */
struct tipc_subscription {
	struct kref kref;
	struct net *net;
	struct timer_list timer;
	struct list_head service_list;
	struct list_head sub_list;
	struct tipc_event evt;
	int conid;
	bool inactive;
	spinlock_t lock;
};

struct tipc_subscription *tipc_sub_subscribe(struct net *net,
					     struct tipc_subscr *s,
					     int conid);
void tipc_sub_unsubscribe(struct tipc_subscription *sub);

int tipc_sub_check_overlap(struct tipc_name_seq *seq, u32 found_lower,
			   u32 found_upper);
void tipc_sub_report_overlap(struct tipc_subscription *sub,
			     u32 found_lower, u32 found_upper,
			     u32 event, u32 port, u32 node,
			     u32 scope, int must);

int __net_init tipc_topsrv_init_net(struct net *net);
void __net_exit tipc_topsrv_exit_net(struct net *net);

void tipc_sub_put(struct tipc_subscription *subscription);
void tipc_sub_get(struct tipc_subscription *subscription);

#define TIPC_FILTER_MASK (TIPC_SUB_PORTS | TIPC_SUB_SERVICE | TIPC_SUB_CANCEL)

/* tipc_sub_read - return field_ of struct sub_ in host endian format
 */
#define tipc_sub_read(sub_, field_)					\
	({								\
		struct tipc_subscr *sub__ = sub_;			\
		u32 val__ = (sub__)->field_;				\
		int swap_ = !((sub__)->filter & TIPC_FILTER_MASK);	\
		(swap_ ? swab32(val__) : val__);			\
	})

/* tipc_sub_write - write val_ to field_ of struct sub_ in user endian format
 */
#define tipc_sub_write(sub_, field_, val_)				\
	({								\
		struct tipc_subscr *sub__ = sub_;			\
		u32 val__ = val_;					\
		int swap_ = !((sub__)->filter & TIPC_FILTER_MASK);	\
		(sub__)->field_ = swap_ ? swab32(val__) : val__;	\
	})

/* tipc_evt_write - write val_ to field_ of struct evt_ in user endian format
 */
#define tipc_evt_write(evt_, field_, val_)				\
	({								\
		struct tipc_event *evt__ = evt_;			\
		u32 val__ = val_;					\
		int swap_ = !((evt__)->s.filter & (TIPC_FILTER_MASK));	\
		(evt__)->field_ = swap_ ? swab32(val__) : val__;	\
	})

#endif

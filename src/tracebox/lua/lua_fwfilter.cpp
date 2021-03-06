/**
 * Tracebox -- A middlebox detection tool
 *
 *  Copyright 2013-2015 by its authors.
 *  Some rights reserved. See LICENSE, AUTHORS.
 */

#include "lua_fwfilter.h"
#include "lua_packet.hpp"

using namespace Crafter;

FWFilter::FWFilter(int src, int dst)
	: src(src), dst(dst), id(src^dst), closed(false)
{
#ifdef __APPLE__
	std::string cmd = "ipfw add " + StrPort(id) + " deny tcp from any " \
		+ StrPort(dst) + " to any " + StrPort(src) + " in";
#else /* Assume Linux */
	std::string cmd = "iptables -A INPUT -p tcp --sport " \
				+ StrPort(dst) + " --dport " + StrPort(src) + " -j DROP";
#endif
	system(cmd.c_str());
}

void FWFilter::close() {
	if (closed)
		return;

#ifdef __APPLE__
	std::string cmd = "ipfw del " + StrPort(id);
#else /* Assume Linux */
	std::string cmd = "iptables -D INPUT -p tcp --sport " \
				+ StrPort(dst) + " --dport " + StrPort(src) + " -j DROP";
#endif
	system(cmd.c_str());
	closed = true;
}
/***
 * A simple firewall rule on the host machine. Create one with @{Globals.filter}
 * @classmod FWFilter
 */
/***Close the Firewall
 * @function close
 */
/*** Same than @{close}
 * @function __gc
 * @see close
 */
int l_fwfilter_ref::l_FWFilter_close(lua_State *l)
{
	FWFilter *fw = l_fwfilter_ref::extract(l, 1);
	fw->close();
	return 0;
}

int l_fwfilter_ref::l_TraceboxFilter(lua_State *l)
{
	int dstPort, srcPort;
	Packet *pkt = l_packet_ref::extract(l, 1);
	if (!pkt)
		return 0;
	TCP *tcp = pkt->GetLayer<TCP>();
	UDP *udp = pkt->GetLayer<UDP>();

	if (tcp) {
		dstPort = tcp->GetDstPort();
		srcPort = tcp->GetSrcPort();
	} else if (udp) {
		dstPort = udp->GetDstPort();
		srcPort = udp->GetSrcPort();
	} else {
		const char* msg = lua_pushfstring(l,
			"filter only works with TCP or UDP");
		luaL_argerror(l, -1, msg);
		return 0;
	}

	FWFilter *f = new FWFilter(srcPort, dstPort);
	new l_fwfilter_ref(f, l);

	return 1;
}

void l_fwfilter_ref::register_members(lua_State *l)
{
	l_ref<FWFilter>::register_members(l);
	meta_bind_func(l, "close", l_FWFilter_close);
}


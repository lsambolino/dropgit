
/*

Copyright (c) 2013, Sergio Mangialardi (sergio@reti.dist.unige.it)
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list 
of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this 
list of conditions and the following disclaimer in the documentation and/or 
other materials provided with the distribution.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "arp_request.hpp"

#include <string>

#include "event/network/arp.hpp"

#include "message/network/arp.hpp"

#include "protocol/protocol.hpp"

#include "router/route.hpp"

#include "util/arp_cache.hpp"

#include "ip_address.hpp"
#include "mac_address.hpp"

namespace drop {
namespace activity {

ArpRequest::ArpRequest(const std::shared_ptr<event::ArpRequest>& event): event_{ event } {}

void ArpRequest::operator()()
{
	/*auto dst = tnt::ip::Address::from_net_order_ulong(event_->ip());
	const auto& proto = event_->protocol();

	try
	{
		std::string address;

		ce::ControlElement::router()->for_each_route([&] (const auto& r)
		{
			if (r->destination() == dst)
			{
				auto mac = tnt::hw_from_arp(r->gateway().to_string());
				tnt::Log::info(colors::blue, "Got an ARP request for ip ", dst, " => ", r->gateway(), " (", mac, ")");
				proto->send(std::make_unique<message::ArpReply>(event_->ip(), mac));

				return;
			}
		});
	}
	catch (const std::invalid_argument&)
	{
		// The IP is not in the ARP cache
		// TODO: Send an ARP request for the IP event->ip() and send back the reply.

		if (!proto.unique())
		{
			proto->send(std::make_unique<message::ArpReply>(event_->ip(), tnt::MacAddress("00:01:02:03:05:00")));
		}
	}
	catch (const std::exception& ex)
	{
		tnt::Log::error("ArpRequest::run error: ", ex.what());

		return;
	}*/
}

} // namespace activity
} // namespace drop


/*

Copyright (c) 2013, Giulio Scancarello (giulioscanca@hotmail.it), Sergio Mangialardi (sergio@reti.dist.unige.it)
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

#include "openflow.hpp"

#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <cstdint>
#include <cassert>

#include "io/tcp_io.hpp"

#include "protocol/openflow/utils.hpp"
#include "protocol/openflow/openflow_version.hpp"
#include "protocol/openflow/flow.hpp"

#include "util/random.hpp"

#include "ip_socket_address.hpp"
#include "endianness.hpp"
#include "application.hpp"
#include "log.hpp"
#include "dump.hpp"

using namespace std::literals;

namespace drop {
namespace protocol {
namespace {

enum class ofp_type: uint8_t
{
    HELLO
};

struct ofp_header
{
    uint8_t version;    // OFP_VERSION.
    ofp_type type;      // One of the  constants.
    uint16_t length;    // Length including this ofp_header.
    uint32_t xid;       // Transaction id associated with this packet. Replies use the same id as was in the request to facilitate pairing.
};

} // namespace

Openflow::Openflow(const std::shared_ptr<tnt::IO>& io): AsyncProtocol(io) {}
Openflow::~Openflow() {}

void Openflow::write(const std::string& data)
{
    AsyncProtocol::write(data);
}

void Openflow::invoke_message(const std::string& message)
{
    auto pkt = reinterpret_cast<const ofp_header*>(message.data());

    if (pkt->type == ofp_type::HELLO)
    {
        auto version = pkt->version;
        protocol_ = OpenflowVersion::create(version, this);

        if (!protocol_)
        {
            version = 0x1;
            protocol_ = OpenflowVersion::create(version, this);
            assert(protocol_);
        }

        ofp_header packet;
        packet.version = version;
        packet.type = ofp_type::HELLO;
        packet.length = htons(sizeof(packet));
        packet.xid = pkt->xid;

        write(to_mem_buffer(packet));

        std::this_thread::sleep_for(50ms); // Needed to avoid the packets to be sent in the same tcp packet.

        /* In caso di supporto di piu versioni è necessario implementare la gestione
        dell'HELLO inviato e ricevuto.
        Se la versione ricevuta è piu piccola di quella del controller e questo la supporta
        non serve mandare un altro hello corretto, basta correggere la versione nei pacchetti
        che si invieranno e continuare normalmente con FEATURES_REQUEST e SET_CONFIG.
        Se la versione ricevuta è piu piccola di quella del controller ma non è supportata,
        probabilmente (non testato) è necessario mandare un ERROR e attendere un hello corretto.
        */

        protocol_->init();
    }
	else
	{
        assert(protocol_);
        protocol_->packet(message);
	}
}

std::vector<std::string> Openflow::parse(std::string& raw_input)
{
    std::vector<std::string> messages;

    while (true)
    {
		auto len = raw_input.size();

		if (len < sizeof(ofp_header))
		{
			// Got only a partial header of a message, wait for other data from IO class.
			break;
		}

		auto hdr = reinterpret_cast<const ofp_header*>(raw_input.data());
		auto packet_len = ntohs(hdr->length);

		if (len < packet_len)
		{
			// Got only a part of a message, wait for other data from IO class.
			break;
		}

		if (len == packet_len)
		{
			// Got one single message.
			messages.push_back(raw_input);
			raw_input.clear();

			break;
		}

		// Got more than one message.
		messages.push_back(raw_input.substr(0, packet_len));
		raw_input = raw_input.substr(packet_len);
    }

    return messages;
}

void Openflow::add(const Flow& flow)
{
    std::this_thread::sleep_for(50ms); // Needed to avoid the packets to be sent in the same tcp packet.
    protocol_->add(flow);
}

void Openflow::remove(const Flow& flow)
{
    std::this_thread::sleep_for(50ms); // Needed to avoid the packets to be sent in the same tcp packet.
    protocol_->remove(flow);
}

void Openflow::packet_out(const std::string& buffer, uint16_t port)
{
    std::this_thread::sleep_for(50ms); // Needed to avoid the packets to be sent in the same tcp packet.
    protocol_->send_packet(buffer, port);
}

void Openflow::request_port_stats(uint16_t port)
{
    std::this_thread::sleep_for(50ms); // Needed to avoid the packets to be sent in the same tcp packet.
    protocol_->request_port_stats(port);
}

} // namespace protocol
} // namespace drop

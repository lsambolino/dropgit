
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

#ifndef DROP_PROTOCOL_OPENFLOW_1_3_HPP_
#define DROP_PROTOCOL_OPENFLOW_1_3_HPP_

#include "protocol/openflow/openflow_version.hpp"
#include "protocol/openflow/v_1_3/values.hpp"

#include "dispatch.hpp"

namespace std {

template<> struct hash<::drop::protocol::ofp_type_1_3>
{
    using argument_type = ::drop::protocol::ofp_type_1_3;
    using underlying_type = std::underlying_type<argument_type>::type;
    using result_type = std::hash<underlying_type>::result_type;

    result_type operator()(const argument_type& arg) const
    {
        std::hash<underlying_type> hasher;

        return hasher(static_cast<underlying_type>(arg));
    }
};

template<> struct hash<::drop::protocol::ofp_error_type_1_3>
{
    using argument_type = ::drop::protocol::ofp_error_type_1_3;
    using underlying_type = std::underlying_type<argument_type>::type;
    using result_type = std::hash<underlying_type>::result_type;

    result_type operator()(const argument_type& arg) const
    {
        std::hash<underlying_type> hasher;

        return hasher(static_cast<underlying_type>(arg));
    }
};

} // namespace std

namespace drop {
namespace protocol {

class Openflow;
struct ofp_header_1_3;
struct ofp_error_msg_1_3;

class Openflow_1_3: public RegisterOpenflowVersion<Openflow_1_3, 0x4>
{
public:
    explicit Openflow_1_3(Openflow* parent);

    virtual void init() override;
    virtual void remove_all() override;

    virtual void add(const Flow& flow) override;
    virtual void remove(const Flow& flow) override;

    virtual void packet(const std::string& message) override;

    virtual void send_packet(const std::string& buffer, uint16_t port) override;

    virtual void request_port_stats(uint16_t port) override;
private:
    void features(const std::string& message);
    void error(const std::string& message);
    void echo(const std::string& message);
    void port_status(const std::string& message);
    void get_config(const std::string& message);
    void stats(const std::string& message);
    void flow_removed(const std::string& message);
    void packet_in(const std::string& message);

    void unmanaged_packet(const ofp_header_1_3* pkt);

    void error_hello_failed(const ofp_error_msg_1_3* pkt, const std::string& message);
    void error_bad_request(const ofp_error_msg_1_3* pkt, const std::string& message);
    void error_bad_action(const ofp_error_msg_1_3* pkt, const std::string& message);
    void error_flow_mod_failed(const ofp_error_msg_1_3* pkt, const std::string& message);
    void error_port_mod_failed(const ofp_error_msg_1_3* pkt, const std::string& message);
    void error_queue_op_failed(const ofp_error_msg_1_3* pkt, const std::string& message);
    void error_bad_instruction(const ofp_error_msg_1_3* pkt, const std::string& message);
    void error_bad_match(const ofp_error_msg_1_3* pkt, const std::string& message);
    void error_group_mod_failed(const ofp_error_msg_1_3* pkt, const std::string& message);
    void error_table_mod_failed(const ofp_error_msg_1_3* pkt, const std::string& message);
    void error_switch_config_failed(const ofp_error_msg_1_3* pkt, const std::string& message);
    void error_role_request_failed(const ofp_error_msg_1_3* pkt, const std::string& message);
    void error_meter_mod_failed(const ofp_error_msg_1_3* pkt, const std::string& message);
    void error_table_features_failed(const ofp_error_msg_1_3* pkt, const std::string& message);
    void error_experimenter(const ofp_error_msg_1_3* pkt, const std::string& message);

    void register_handlers();
private:
    Openflow* parent_;

    tnt::KeyDispatch<ofp_type_1_3, void, const std::string&> message_dispatcher_;
    tnt::KeyDispatch<ofp_error_type_1_3, void, const ofp_error_msg_1_3*, const std::string&> error_dispatcher_;
};

} // namespace protocol
} // namespace drop


#endif

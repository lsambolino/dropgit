
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

#ifndef DROP_PROTOCOL_DROP_VALUES_HPP_
#define DROP_PROTOCOL_DROP_VALUES_HPP_

#include <limits>
#include <cstdint>

namespace drop {
namespace protocol {

// Following enums values must be unique.
enum class DropMessage: uint32_t
{
    // Generic messages
    Ack,
    Heartbeat,

    // Association messages
    ServiceElementData,

    // Network management commands from CE to FE
	InterfaceList,
    AddKernelRoute,
    AddUserspaceRoute,
    DelRoute,
	ArpRequest,
	ArpReply,

    // Network management replies from FE to CE
    AddRouteSuccess,
    AddRouteFailure,
    DelRouteSuccess,
    DelRouteFailure,

    // Network management events
    PortAdminUp,
    PortAdminDown,
    
    // GAL messages
    GalDiscoveryRequest,
    GalDiscoveryReply,
    GalMonitorStateRequest,
    GalMonitorStateReply,
    GalMonitorHistoryRequest,
    GalMonitorHistoryReply,
    GalMonitorSensorRequest,
    GalMonitorSensorReply,
    GalProvisioningRequest,
    GalRollbackRequest,
    GalCommitRequest,
    GalReleaseRequest,

    // Openflow messages
    ControllerConnection,
    PortModify,
    DatapathJoin,			  	  
    DatapathJoinAck, 		      
    DatapathLeave, 				  
    DatapathLeaveAck,             
    AggregateStats,               
    AggregateStatsResponse,       
    BarrierReply,				  
    BarrierReplyResponse,         
    BootstrapComplete,            
    BootstrapCompleteResponse,    
    DescStatsIn,                  
    DescStatsInResponse,          
    EchoRequest,                  
    EchoRequestResponse,          
    FlowAdd,					  
    FlowModify,                   
    FlowModifyResponse,           
    FlowRemove,                   
    FlowRemoveResponse,           
    FlowStatsRequest,
	FlowStatsActivityStart,
    FlowStats,                    
    FlowStatsAck,		          
    FlowStatsReply,             
    OfmpConfigUpdateAck,          
    OfmpConfigUpdateAckResponse,  
    OfmpConfigUpdate,             
    OfmpConfigUpdateResponse,     
    OfmpResourcesUpdate,          
    OfmpResourcesUpdateResponse,  
    OpenflowMsg,                  
    OperatorMsgResponse,          
    PacketIn,                     
    PacketOut,
    PacketsOut,
    PortStatsRequest,             
    PortStatsReply,                    
    PortStatsAck,
    PortStatusChange,
    QueueConfigIn,                
    QueueConfigInResponse,        
    QueueStatsIn,                 
    QueueStatsInResponse,         
    SwitchMgrJoin,                
    SwitchMgrJoinResponse,        
    SwitchMgrLeave,               
    SwitchMgrLeaveResponse,       
    TableStatsIn,                 
    TableStatsInResponse,         
    StopProtocol
};

} // namespace protocol
} // namespace drop

#endif

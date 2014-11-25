
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

#ifndef DROP_EVENT_DATA_UPDATE_HPP_
#define DROP_EVENT_DATA_UPDATE_HPP_

#include <memory>
#include <map>
#include <vector>
#include <string>
#include <cstdint>

#include "event/gal/data.hpp"

namespace drop {

struct InterfaceData
{
    InterfaceData(int i, uint64_t rp, uint64_t rb, uint64_t tp, uint64_t tb) : iface(i), rxpkts(rp), rxbytes(rb), txpkts(tp), txbytes(tb) {}

    int iface;

    uint64_t rxpkts;
    uint64_t rxbytes;
    uint64_t txpkts;
    uint64_t txbytes;
};

namespace event {

class ExternalInterfacesDataUpdate
{
public:
    explicit ExternalInterfacesDataUpdate(const std::vector<InterfaceData>& data) : data_(data) {}
    const std::vector<InterfaceData>& data() const { return data_; }
private:
    std::vector<InterfaceData> data_;
};

}  // namespace event
}  // namespace drop

#endif

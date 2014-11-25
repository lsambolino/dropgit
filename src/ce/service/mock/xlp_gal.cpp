
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

#include "xlp_gal.hpp"

#include <unordered_map>
#include <tuple>

#include "exception/exception.hpp"

#include "event/http/http_response.hpp"
#include "event/gal/slot_change.hpp"

#include "message/http/http_request.hpp"

#include "network/service_element.hpp"

#include "protocol/http/http_protocol.hpp"

#include "io/io.hpp"

#include "gal_drop/load_power_states.hpp"
#include "gal_drop/load_sensors.hpp"

#include "util/path.hpp"
#include "util/pugixml.hpp"
#include "util/exec.hpp"
#include "util/string.hpp"
#include "util/random.hpp"

#include "application.hpp"

namespace drop {
namespace service {
namespace {

std::unordered_map<int, std::tuple<int, int, int, int, int, int, int, int, double>> frequencies_map = 
{
    { 1, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 2, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 3, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 4, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 5, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 6, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 7, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 8, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 9, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 10, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 11, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 12, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 13, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 14, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 15, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 16, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 17, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 18, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 19, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 20, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 21, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 22, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 23, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 24, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 25, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 26, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 27, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 28, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 29, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 30, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 31, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 32, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 33, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 34, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 35, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 36, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 37, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 38, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 39, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 40, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 41, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 42, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 43, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 44, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 45, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 46, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 47, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 48, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 49, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 50, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 51, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 52, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 53, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 54, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 55, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 56, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 57, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 58, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 59, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 60, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 61, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 62, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 63, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 64, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 65, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 66, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 67, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 68, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 69, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 70, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 71, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 72, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 73, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 74, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 75, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 76, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 77, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 78, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 79, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 80, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 81, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 82, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 83, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 84, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 85, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 86, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 87, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 88, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 89, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 90, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 91, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 92, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 93, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 94, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 95, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 96, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 97, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 98, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 99, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 100, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 101, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 102, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 103, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 104, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 105, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 106, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 107, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 108, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 109, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 110, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 111, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 112, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 113, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 114, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 115, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 116, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 117, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 118, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 119, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 120, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 121, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 122, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 123, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 124, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 125, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 126, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 127, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 128, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 129, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 130, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 131, std::make_tuple(429, 429, 429, 429, 429, 429, 429, 429, 0.941) },
    { 132, std::make_tuple(429, 429, 429, 429, 429, 429, 429, 429, 0.941) },
    { 133, std::make_tuple(429, 429, 429, 429, 429, 429, 429, 429, 0.941) },
    { 134, std::make_tuple(429, 429, 429, 429, 429, 429, 429, 429, 0.941) },
    { 135, std::make_tuple(429, 429, 429, 429, 429, 429, 429, 429, 0.941) },
    { 136, std::make_tuple(429, 429, 429, 429, 429, 429, 429, 429, 0.941) },
    { 137, std::make_tuple(429, 429, 429, 429, 429, 429, 429, 429, 0.941) },
    { 138, std::make_tuple(429, 429, 429, 429, 429, 429, 429, 429, 0.941) },
    { 139, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 140, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 141, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 142, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 143, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 144, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 145, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 146, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 147, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 148, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 149, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 150, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 151, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 152, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 153, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 154, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 155, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 156, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 157, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 158, std::make_tuple(500, 500, 500, 500, 500, 500, 500, 500, 0.941) },
    { 159, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 160, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 161, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 162, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 163, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 164, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 165, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 166, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 167, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 168, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 169, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 170, std::make_tuple(600, 600, 600, 600, 600, 600, 600, 600, 0.941) },
    { 171, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 172, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 173, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 174, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 175, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 176, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 177, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 178, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 179, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 180, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 181, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 182, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 183, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 184, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 185, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 186, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 187, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 188, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 189, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 190, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 191, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 192, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 193, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 194, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 195, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 196, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 197, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 198, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 199, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 200, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 201, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 202, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 203, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 204, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 205, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 206, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 207, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 208, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 209, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 210, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 211, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 212, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 213, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 214, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 215, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 216, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 217, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 218, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 219, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 220, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 221, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 222, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 223, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 224, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 225, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 226, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 227, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 228, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 229, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 230, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 231, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 232, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 233, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 234, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 235, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 236, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 237, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 238, std::make_tuple(1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500, 1.021) },
    { 239, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 240, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 241, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 242, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 243, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 244, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 245, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 246, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 247, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 248, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 249, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 250, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 251, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 252, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 253, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 254, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 255, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 256, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 257, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 258, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 259, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 260, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 261, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 262, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 263, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 264, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 265, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 266, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 267, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 268, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 269, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 270, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 271, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 272, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 273, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 274, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 275, std::make_tuple(1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 0.941) },
    { 276, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 277, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 278, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 279, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 280, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 281, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 282, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 283, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 284, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 285, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 286, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 287, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 288, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 289, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 290, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 291, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 292, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 293, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 294, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 295, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 296, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 297, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 298, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 299, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) },
    { 300, std::make_tuple(750, 750, 750, 750, 750, 750, 750, 750, 0.941) }
};

} // namespace

MockXLPGal::MockXLPGal(ce::ServiceElement* parent) : parent_(parent), current_slot_{ 1 }
{
    load_configuration();

    tnt::Application::subscribe([&] (const event::SlotChange& event)
    {
        current_slot_ = event.num();
    });
}

std::ostream& MockXLPGal::print(std::ostream& os) const
{
    os << R"({ "Name": "MockXLPGal" })";

    return os;
}

gal::ReturnCode MockXLPGal::discovery(const std::string& resource_id,
                                      bool /*committed*/,
                                      gal::LogicalResource* resource,
                                      gal::LogicalResourceContainer* logical_resources,
                                      gal::PhysicalResourceContainer* /*physical_resources*/,
                                      gal::SensorResourceContainer* sensor_resources,
                                      gal::PowerStateContainer* power_states,
                                      gal::OptimalConfigContainer* /*edl*/)
{
    if (resource_id != "0")
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    if (resource)
    {
        *resource = resource_;
    }
    
    if (logical_resources)
    {
        *logical_resources = logical_resources_;
    }
    
    if (sensor_resources)
    {
        for (const auto& s : sensor_resources_)
        {
            sensor_resources->push_back(s.second);
        }
    }

    if (power_states)
    {
        *power_states = power_states_;
    }
    
    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode MockXLPGal::provisioning(const std::string& /*resource_id*/, int /*power_state_id*/)
{
    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode MockXLPGal::release(const std::string& /*resource_id*/)
{
    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode MockXLPGal::monitor_state(const std::string& resource_id, bool /*committed*/, gal::PowerState::PowerStateId& power_state_id)
{
    if (resource_id != "0")
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    if (power_states_.empty())
    {
        return gal::ReturnCode::GalNotImplemented;
    }

    power_state_id = power_states_[0].id;
    
    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode MockXLPGal::monitor_history(const std::string& /*resource_id*/, gal::PowerStateHistoryContainer* /*history*/)
{
    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode MockXLPGal::monitor_sensor(const std::string& resource_id, gal::EntitySensorStatus& oper_status, gal::EntitySensorValue& sensor_value, gal::EntitySensorTimeStamp& value_timestamp)
{
    auto it = sensor_resources_.find(resource_id);

    if (it == std::end(sensor_resources_))
    {
        return gal::ReturnCode::GalResourceNotFound;
    }

    oper_status = gal::EntitySensorStatus::ok;
    value_timestamp = std::chrono::seconds(std::time(nullptr)).count() * 1000;

    const auto& t = frequencies_map[current_slot_];

    if (resource_id.find("freq") == 0)
    {
        switch (std::stoi(resource_id.substr(4)))
        {
        case 0:
            sensor_value = std::get<0>(t);
            break;
        case 1:
            sensor_value = std::get<1>(t);
            break;
        case 2:
            sensor_value = std::get<2>(t);
            break;
        case 3:
            sensor_value = std::get<3>(t);
            break;
        case 4:
            sensor_value = std::get<4>(t);
            break;
        case 5:
            sensor_value = std::get<5>(t);
            break;
        case 6:
            sensor_value = std::get<6>(t);
            break;
        case 7:
            sensor_value = std::get<7>(t);
            break;
        default:
            return gal::ReturnCode::GalResourceNotFound;
        }

        return gal::ReturnCode::GalSuccess;
    }
    else if (resource_id == "voltage")
    {
        sensor_value = std::get<8>(t) * 1000;

        return gal::ReturnCode::GalSuccess;
    }
    else if (resource_id == "cpuLoad")
    {
        auto sum = std::get<0>(t) + std::get<1>(t) + std::get<2>(t) + std::get<3>(t) + std::get<4>(t) + std::get<5>(t) + std::get<6>(t) + std::get<7>(t);
        sensor_value = static_cast<gal::EntitySensorValue>(sum / (1600.0 * 8.0) * 100.0);

        return gal::ReturnCode::GalSuccess;
    }

    return gal::ReturnCode::GalResourceNotFound;
}

gal::ReturnCode MockXLPGal::monitor_sensor_history(const std::string& /*resource_id*/, gal::SensorHistoryContainer* /*history*/)
{
    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode MockXLPGal::commit(const std::string& /*resource_id*/)
{
    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode MockXLPGal::rollback(const std::string& /*resource_id*/)
{
    return gal::ReturnCode::GalNotImplemented;
}

void MockXLPGal::load_configuration()
{
    auto path = util::full_path(util::combine_path("config", parent_->display_name(), "gal.xml"));

    pugi::xml_document doc;
    auto result = doc.load_file(path.c_str());

    if (!result)
    {
        throw tnt::ConfigurationError(std::string("MockXLPGal: ") + result.description());
    }

    const auto& root = doc.child("Gal");

    assert(root);

    resource_.id = parent_->name();
    resource_.type = root.child("Type").child_value();
    resource_.description = root.child("Description").child_value();

    auto lr = root.child("LogicalResources");

    if (lr)
    {
        for (auto r : lr)
        {
            gal::LogicalResource res;

            res.id = r.attribute("id").as_string();
            res.type = r.child("Type").child_value();
            res.description = r.child("Description").child_value();

            logical_resources_.push_back(res);
        }
    }

    auto pr = root.child("PhysicalResources");

    if (pr)
    {
        for (auto r : pr)
        {
            gal::PhysicalResource res;

            res.id = r.attribute("id").as_string();
            res.type = r.child("Type").child_value();
            res.description = r.child("Description").child_value();

            physical_resources_.push_back(res);
        }
    }

    auto sr = root.child("SensorResources");

    if (sr)
    {
        sensor_resources_ = load_sensors(sr);
    }

    power_states_ = load_power_states(root.child("EnergyAwareStates"));
}

} // namespace service
} // namespace drop

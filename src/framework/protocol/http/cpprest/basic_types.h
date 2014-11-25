/***
* ==++==
*
* Copyright (c) Microsoft Corporation. All rights reserved. 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* ==--==
* =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
*
* basic_types.h
*
* Platform-dependent type definitions
*
* For the latest on this and related APIs, please see http://casablanca.codeplex.com.
*
* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
****/

#ifndef CPPREST_BASIC_TYPES_H_
#define CPPREST_BASIC_TYPES_H_

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdint>

namespace utility {

// We should be using a 64-bit size type for most situations that do
// not involve specifying the size of a memory allocation or buffer.
typedef uint64_t size64_t;

//
// On POSIX platforms, all strings are narrow
//
typedef char char_t;
typedef std::string string_t;
#define _XPLATSTR(x) x
typedef std::ostringstream ostringstream_t;
typedef std::ofstream ofstream_t;
typedef std::ostream ostream_t;
typedef std::istream istream_t;
typedef std::ifstream ifstream_t;
typedef std::istringstream istringstream_t;
typedef std::stringstream stringstream_t;
#define ucout std::cout
#define ucin std::cin
#define ucerr std::cerr

#ifndef _TURN_OFF_PLATFORM_STRING
#define U(x) _XPLATSTR(x)
#endif // !_TURN_OFF_PLATFORM_STRING

#define _ASYNCRTIMP

} // namespace utility

#endif

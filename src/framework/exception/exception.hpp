
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

#ifndef TNT_EXCEPTION_HPP_
#define TNT_EXCEPTION_HPP_

#include <system_error>
#include <stdexcept>
#include <string>

namespace tnt {

std::error_code system_error_code();

struct ConfigurationError: public std::logic_error
{
    explicit ConfigurationError(const std::string& message): std::logic_error(message) {}
};

struct IOReset: public std::runtime_error
{
    explicit IOReset(const std::string& msg): std::runtime_error(msg) {}
};

struct IOConnectionError: public std::runtime_error
{
    explicit IOConnectionError(const std::string& msg): std::runtime_error(msg) {}
};

struct IODataError: public std::system_error
{
    explicit IODataError(): std::system_error(system_error_code()) {}
};

struct OperationNotSupported: public std::system_error
{
    explicit OperationNotSupported(std::error_code ec = system_error_code()): std::system_error(ec) { }
};

struct EmptyArgument: public std::invalid_argument
{
    explicit EmptyArgument(const std::string& parameter): std::invalid_argument(std::string(R"("The parameter ")") + parameter + R"(" cannot be empty.)") { }
};

struct FileOperationError: public std::system_error
{
    explicit FileOperationError(std::error_code ec): std::system_error(ec) { }
};

struct OpenFileError: public FileOperationError
{
    explicit OpenFileError(std::error_code ec = system_error_code()): FileOperationError(ec) { }
};

struct NonExistentFile: public OpenFileError
{
    NonExistentFile(): OpenFileError(std::make_error_code(std::errc::no_such_file_or_directory)) {}
};

struct ReadFileError: public FileOperationError
{
    explicit ReadFileError(std::error_code ec = system_error_code()): FileOperationError(ec) { }
};

struct WriteFileError: public FileOperationError
{
    explicit WriteFileError(std::error_code ec = system_error_code()): FileOperationError(ec) { }
};

struct SyntaxError: public std::runtime_error
{
    explicit SyntaxError(const std::string& msg): std::runtime_error(msg) { }
};

struct ProtocolException: public std::runtime_error
{
    explicit ProtocolException(const std::string& msg) : std::runtime_error(msg) {}
};

struct OperationError: public std::system_error
{
    explicit OperationError(std::error_code ec = system_error_code()): std::system_error(ec) { }
};

} // namespace tnt

#endif

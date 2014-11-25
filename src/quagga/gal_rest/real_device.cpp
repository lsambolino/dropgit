
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

#include "real_device.hpp"

#include <atomic>

#include "event/quit.hpp"
#include "event/gsi_request.hpp"



//IPC Common Part: I need these for making Inter Process Communications working
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

//Function needed for socket error
    void error(const char *msg)
{
    perror(msg);
    exit(0);
}













                                      gal::ReturnCode RealDevice::discovery(const std::string& /*resource_id*/,
									  bool /*committed*/,
									  gal::LogicalResource* resource,
									  gal::LogicalResourceContainer* /*logical_resources*/,
									  gal::PhysicalResourceContainer* /*physical_resources*/,
									  gal::SensorResourceContainer* /*sensor_resources*/,
									  gal::PowerStateContainer* /*power_states*/,
									  gal::OptimalConfigContainer* /*edl*/)
{
    // TODO: Add implementation
    std::cout << "Implementazione del discovery" << std::endl;



    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

    portno = 81;
    sockfd = socket(AF_INET, SOCK_STREAM, 0); //gives -1 if error
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    printf("Ready to send the message \n");
    bzero(buffer,256);
    //fgets(buffer,255,stdin);
    
    std::string tmp = "Send me secret message!\n";
	strcpy(buffer, tmp.c_str());
    n = write(sockfd,buffer,strlen(buffer)); //Write the above sentence in the socket
    printf("Secret message request sent \n");
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    printf("Here is the secrete message: %s\n",buffer);
    /*if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer);*/
    
    close(sockfd);







 
    if (resource)
    {
        resource->id = "0";
        resource->description = "bbb";
        resource->type = "ccc";
    }

    return gal::ReturnCode::GalSuccess;
}

gal::ReturnCode RealDevice::provisioning(const std::string& /*resource_id*/, int /*power_state_id*/)
{
	// TODO: Add implementation

    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode RealDevice::release(const std::string& /*resource_id*/)
{
	// TODO: Add implementation

    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode RealDevice::monitor_state(const std::string& /*resource_id*/, bool /*committed*/, int& /*power_state_id*/)
{
    // TODO: Add implementation

    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode RealDevice::monitor_history(const std::string& /*resource_id*/, gal::PowerStateHistoryContainer* /*history*/)
{
	// TODO: Add implementation

    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode RealDevice::monitor_sensor(const std::string& /*resource_id*/, 
											gal::EntitySensorStatus& /*oper_status*/, 
											gal::EntitySensorValue& /*sensor_value*/, 
											gal::EntitySensorTimeStamp& /*value_timestamp*/)
{
	// TODO: Add implementation.

	return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode RealDevice::monitor_sensor_history(const std::string& /*resource_id*/, gal::SensorHistoryContainer* /*history*/)
{
    // TODO: Add implementation.

	return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode RealDevice::commit(const std::string& /*resource_id*/)
{
	// TODO: Add implementation

    return gal::ReturnCode::GalNotImplemented;
}

gal::ReturnCode RealDevice::rollback(const std::string& /*resource_id*/)
{
	// TODO: Add implementation

    return gal::ReturnCode::GalNotImplemented;
}

void RealDevice::operator()()
{
    try
    {
        std::cout << "Starting operator.." << std::endl;
        std::atomic_bool running{ true }; 
        
        register_handler([&] (const tnt::event::Quit& /*event*/) // QUIT command
        {
			std::cout << "register handler for Quit.." << std::endl;
            running = false;
        });
        register_handler([&] (std::shared_ptr<drop::event::GSIRequest> event)
        {
			std::cout << "register handler for Event" << std::endl;
            event->exec(this);
        });

        while (running)
        {
			std::cout << "while running wait event.." << std::endl;
            wait_event();
        }
    }
    catch (std::exception& ex)
    {
        tnt::Log::error("ServiceElementLCP::run error: ", ex.what());
    }
    catch (...)
    {
        tnt::Log::error("ServiceElementLCP::run: unknown exception raised.");
    }
}

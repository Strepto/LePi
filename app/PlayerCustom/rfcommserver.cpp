#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#include "sdpregister.h"
#include "rfcommserver.h"

RfcommServer::RfcommServer(){
	std::cout << "RfCommServer instantiated" << std::endl;
}

int RfcommServer::btServer()
{
    std::cout << "Starting service" << std::endl;
	SdpRegisterer service;
	sdp_session_t* session = service.register_service();
    std::cout << "Service running" << std::endl;
	
	std::cout << "allocate socket\n" << std::endl;
	struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
	char buf[1024] = { 0 };
	int s, client, bytes_read;
	unsigned int opt = sizeof(rem_addr);
	std::cout << "allocate socket\n" << std::endl;
	s = session->sock;
	bdaddr_t BDADDR_ANY_CPY = {{0,0,0,0,0,0}};
	std::cout << "bind socket to port 1 of the first available bluetooth adapter "<< std::endl;
	loc_addr.rc_family = AF_BLUETOOTH;
	loc_addr.rc_bdaddr = BDADDR_ANY_CPY;
	loc_addr.rc_channel = 1;
	bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));

	std::cout << "put socket into listening mode\n" << std::endl;
	listen(s, 1);

	std::cout << "accept one connection\n"<< std::endl;
	client = accept(s, (struct sockaddr *)&rem_addr, &opt);

	ba2str( &rem_addr.rc_bdaddr, buf );
	fprintf(stderr, "accepted connection from %s\n", buf);
	memset(buf, 0, sizeof(buf));

	std::cout << "read data from the client\n" << std::endl;
	bytes_read = recv(client, buf, sizeof(buf), 0);
	if( bytes_read > 0 ) {
		printf("received [%s]\n", buf);
	}

	std::cout << "close connection\n" << std::endl;
	close(client);
	close(s);
	return 0;
}

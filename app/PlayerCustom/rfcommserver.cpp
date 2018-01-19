#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>


#include "sdpregister.h"
#include "rfcommserver.h"

RfcommServer::RfcommServer(){
	std::cout << "RfCommServer instantiated" << std::endl;
}


int RfcommServer::sdpSearch()
{
	    uint8_t svc_uuid_int[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
        0, 0, 0xab, 0xcd };
	bdaddr_t BDADDR_ANY_CPY = {{0,0,0,0,0,0}};

    int status;
    bdaddr_t target = {{0x54,0x27,0x1e,0xd4,0xac,0xbe}};
    uuid_t svc_uuid;
    sdp_list_t *response_list, *search_list, *attrid_list;
    sdp_session_t *session = 0;
    uint32_t range = 0x0000ffff;
    uint8_t port = 0;

    //~ if(argc < 2)
    //~ {
        //~ fprintf(stderr, "usage: %s <bt_addr>\n", argv[0]);
        //~ exit(2);
    //~ }
    //~ str2ba( argv[1], &target );

    // connect to the SDP server running on the remote machine
    session = sdp_connect( &BDADDR_ANY_CPY, &target, 0 );
	std::cout << "Was here " << 43 << std::endl;
    sdp_uuid128_create( &svc_uuid, &svc_uuid_int );
    search_list = sdp_list_append( 0, &svc_uuid );
    attrid_list = sdp_list_append( 0, &range );

    // get a list of service records that have UUID 0xabcd
	std::cout << "get a list of service records that have UUID 0xabcd" << std::endl;
    
    response_list = NULL;
    std::cout << "Was ere" << std::endl;
    status = sdp_service_search_attr_req( session, search_list,
            SDP_ATTR_REQ_RANGE, attrid_list, &response_list);
    std::cout << "sdp_service_search_attr_req" << status << std::endl;

    if( status == 0 ) {
        sdp_list_t *proto_list = NULL;
        sdp_list_t *r = response_list;
        
        // go through each of the service records
		std::cout << "go through each of the service records " << std::endl;
        for (; r; r = r->next ) {
            sdp_record_t *rec = (sdp_record_t*) r->data;
            
            // get a list of the protocol sequences
	std::cout << "get a list of the protocol sequences "  << std::endl;
            if( sdp_get_access_protos( rec, &proto_list ) == 0 ) {

                // get the RFCOMM port number
	std::cout << "et the RFCOMM port number "<< std::endl;
                port = sdp_get_proto_port( proto_list, RFCOMM_UUID );

                sdp_list_free( proto_list, 0 );
            }
            sdp_record_free( rec );
        }
    }
    sdp_list_free( response_list, 0 );
    sdp_list_free( search_list, 0 );
    sdp_list_free( attrid_list, 0 );
    sdp_close( session );

    if( port != 0 ) {
        printf("found service running on RFCOMM port %d\n", port);
    }
    
    return 0;
}


int RfcommServer::btServer2()
{	
	
    std::cout << "Starting service" << std::endl;
	SdpRegisterer service;
	sdp_session_t* session = service.register_service();
    std::cout << "Service running" << std::endl;
	//~ bt_security btsec = BT_SECURITY_SDP;
	bdaddr_t BDADDR_ANY_CPY = {{0,0,0,0,0,0}};
	struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
    char buf[1024] = { 0 };
    int s, client, bytes_read, status;
    unsigned int opt = sizeof(rem_addr);
    fd_set readfds, writefds;
    int maxfd, sock_flags;
    struct timeval timeout;

    // allocate socket
    s = session->sock;//socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    // bind socket to port 1 of the first available bluetooth adapter
    loc_addr.rc_family = AF_BLUETOOTH;
    loc_addr.rc_bdaddr = BDADDR_ANY_CPY;
    loc_addr.rc_channel = 11;
    bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));

    // put server socket into listening mode
    listen(s, 1);

    // put server socket into nonblocking mode
    sock_flags = fcntl( s, F_GETFL, 0 );
    fcntl( s, F_SETFL, sock_flags | O_NONBLOCK );

    // accept one connection
    while(1) { 
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_SET(s, &readfds);
        maxfd = s;

        printf("waiting for connection\n");
        status = select(maxfd + 1, &readfds, &writefds, 0, &timeout);
        if( status > 0 && FD_ISSET( s, &readfds ) ) {
            // incoming connection
            client = accept( s, (struct sockaddr*)&rem_addr, &opt );
            if( client >= 0 ) break;
        }
    }
    // close the server socket, leaving only the client socket open
    close(s);

    ba2str( &rem_addr.rc_bdaddr, buf );
    fprintf(stderr, "accepted connection from %s\n", buf);
    memset(buf, 0, sizeof(buf));

    // put client socket into nonblocking mode
    sock_flags = fcntl( client, F_GETFL, 0 );
    fcntl( client, F_SETFL, sock_flags | O_NONBLOCK );

    // read data from the client
    while(1) {
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_SET(client, &readfds);
        FD_SET(client, &writefds);
        maxfd = client;

        status = select(maxfd + 1, &readfds, &writefds, 0, &timeout);
        if( status > 0 && FD_ISSET( client, &readfds ) ) {
            // incoming data
            bytes_read = recv(client, buf, sizeof(buf), 0);
            if( 0 == bytes_read ) break;
            printf("received [%s]\n", buf);
        }
        if( status > 0 && FD_ISSET( client, &writefds ) ) {
            // can send data
            printf("socket is writable\n");
            send( client, "hello!", 6, 0 );
        }

        printf("doing other stuff\n");
	}
	service.cleanup();
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

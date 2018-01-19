/**
 * This file is part of the LePi Project:
 * https://github.com/cosmac/LePi
 *
 * MIT License
 *
 * Copyright (c) 2017 Andrei Claudiu Cosma
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// Nils Model
#include "Messages.pb.h"
#include "Messages.pb.cc"

// LePi
#include "ConnectionCustom.h"
//~ #include <ConnectionCommon.h>
#include <LeptonCommon.h>
#include <LeptonCamera.h>

// C/C++
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>


using namespace google::protobuf::io;

/**
 * Sample Server app for streaming video over the local network (TCP)
 */
int main() {
    while(true){
		
    // Create socket
    const int kPortNumber{5995};
    const std::string kIPAddress{""}; // If empty, local IP address is used
    int socket_connection;
    if (!ConnectPublisher(kPortNumber, kIPAddress, socket_connection)) {
        std::cerr << "Unable to create connection." << std::endl;
        exit(EXIT_FAILURE);
    }

    // Open camera connection
    LeptonCamera lePi;
    lePi.start();

    // Intermediary buffers
    std::vector<uint8_t> imgU8(lePi.width() * lePi.height());
    std::vector<uint16_t> imgU16(lePi.width() * lePi.height());
	uint frames_sent = 0;
	uint identical_frames_sent_total = 0;
	auto start_time = std::chrono::system_clock::now();

    bool force_exit{false};
    while (!force_exit) {
    
        //  Receive Request
        RequestMessage req_msg;
        req_msg.set_request_cmd(CMD_FRAME_U8);
        req_msg.set_request_type(REQUEST_FRAME);
        //~ ReceiveMessage(socket_connection, req_msg, 1337);
        uint32_t expectedSize = 1;
        uint8_t req_int;
		size_t data_size{0};
		do
		{ // wait for data to be available
			
			if(ioctl(socket_connection, FIONREAD, &data_size) < 0){
				std::cerr << "Error: " << strerror(errno) << std::endl;
				force_exit = true;
				break;
			}

		} while (data_size < expectedSize);
		auto rc = recv(socket_connection, &req_int, expectedSize, 0);

		// Check if connection is still open
		if (rc == -1) {
			std::cerr << "Error: " << strerror(errno) << std::endl;
			force_exit = true;			
			break;
//throw std::runtime_error("Connection lost.");
		}

		if(req_int == 1){
			std::cout << "Received byte with 1 Sending Data" << std::endl;
		}else{
			std::cout << "Received incorrect byte, waiting for new byte." << std::endl;
			continue;
		}

        // Frame request msg
        ResponseMessage resp_msg;
        switch (req_msg.request_type()) {

            case REQUEST_FRAME: {
                resp_msg.set_request_type(REQUEST_FRAME);
                resp_msg.set_sensor_temperature(lePi.SensorTemperature());
                if (req_msg.request_cmd() == CMD_FRAME_U8) {
					if (lePi.hasFrame()) {
						lePi.getFrameU8(imgU8);
						++identical_frames_sent_total;
					}
                    //~ resp_msg.set_frame(imgU8.data(), imgU8.size());
                    //~ resp_msg.set_bpp(1);
                } else {
                    lePi.getFrameU16(imgU16);
                    resp_msg.set_frame(imgU16.data(), imgU16.size() *2);
                    resp_msg.set_bpp(2);
                }
                resp_msg.set_request_status(STATUS_FRAME_READY);
                resp_msg.set_height(lePi.height());
                resp_msg.set_width(lePi.width());
                break;
            }
            case REQUEST_I2C: {
				// Disabled for now.
                //~ resp_msg.set_request_type(REQUEST_I2C);
                //~ if (lePi.sendCommand(static_cast<LeptonI2CCmd>(req_msg.request_cmd()),
                                     //~ resp_msg.frame()) {
                    //~ resp_msg.set_request_status(STATUS_I2C_SUCCEDED);
                //~ }
                //~ else {
                    //~ resp_msg.set_request_status(STATUS_I2C_FAILED);
                //~ }
                break;
            }
            case REQUEST_EXIT: {
                force_exit = true;
                break;
            }
            default : {
                resp_msg.set_request_type(REQUEST_UNKNOWN);
                resp_msg.set_request_status(STATUS_RESEND);
                break;
            }
        }
		//~ std::cout<< "Size after serializing is :" << (resp_msg.ByteSize()) << std::endl;

		//~ uint32_t siz = resp_msg.ByteSize();
		//~ char *pkt = new char [siz];
		//~ std::string data;
		//~ google::protobuf::io::ArrayOutputStream aos = aos(pkt, siz);
		//~ google::protobuf::io::CodedOutputStream *coded_output = new google::protobuf::io::CodedOutputStream(&aos);
		//~ coded_output->WriteVarint32(resp_msg.ByteSize());
		//~ resp_msg.SerializeToString(&data);
		std::cout<< imgU8.size() << std::endl;

        // Send response
        auto sd = send(socket_connection, imgU8.data(), imgU8.size(), 0);
		if (sd == -1) {
			std::cerr << "Error: " << strerror(errno) << std::endl;
			throw std::runtime_error("Connection lost.");
		}
		++frames_sent;
		// Runtime
        auto end_time = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast <std::chrono::seconds>(end_time - start_time);
        if (elapsed.count() > 1.0) {
            double fps = static_cast<double>(frames_sent) / static_cast<double>(elapsed.count());
            std::cout << "FPS: " << fps << std::endl;            
            std::cout << "Idential Frames Sent total: " << identical_frames_sent_total << std::endl;

            start_time = end_time;
            frames_sent = 0;
        }
		//~ std::cout<< "Data sent" << data.length() << std::endl;
        //~ SendMessage(socket_connection, resp_msg);
   }

    // Release sensors
    lePi.stop();

    // Close connection
    close(socket_connection);
}
    return EXIT_SUCCESS;

}

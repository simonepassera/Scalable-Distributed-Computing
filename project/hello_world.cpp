/*
 * Simple "hello world" client-server example.
 * To run the server:	./hello_world 0 server client
 * To run the client:	./hello_world 1 client server
 */

#include <iostream>
#include <csignal>
#include <chrono>
#include <kcl.hpp>

static volatile std::sig_atomic_t stop = 0;
void signal_handler(int) { stop = 1; }

void Server(std::string source) {
	if (KCL::Comm::Listen() != 0) {
		std::cerr << "[SERVER]: ERROR Listen()" << std::endl;
		return;
	}

    std::string msg;
    
	while(!stop) {
		KCL::Comm::Receive(source, msg);

		if (msg == "bye") {
			std::cout << "[SERVER]: The client sent the bye message!" << std::endl;
			stop = 1;
		} else {
			std:: cout << msg << std::endl;
		}
	}
	
	std::cout << "[SERVER]: Goodbye!" << std::endl;
}

void Client(std::string dest) {
	std::string msg;
	int err;

	const char spinner[] = "|/-\\";
	int i = 0;
	
	while (!stop) {
		std::cout << "\nEnter a message [" << dest << "] (\"bye\" for exit):" << std::endl;
		std::getline(std::cin, msg);

		do {
			std::cout << "\r[CLIENT]: Sending... " << spinner[i % 4] << std::flush;

			if ((err = KCL::Comm::Send(dest, msg)) == 1)
				std::cerr << "[CLIENT]: ERROR Send()" << std::endl;

			i++;
			
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		} while (err == 2);
		
		if (!err && (msg == "bye"))
			stop = 1;
	}

	std::cout << "\n[CLIENT]: Closed!" << std::endl;	
}

int main(int argc, char** argv) {
    if(argc < 4) {
		std::cerr << "Usage: " << argv[0] << " <0|1> <processName> <processSource|processDest>" << std::endl;
        return -1;
    }
    
	std::signal(SIGHUP, signal_handler);	

    KCL::Comm::Init(argv[2]);
    
    if (std::stol(argv[1]) == 0)
		Server(argv[3]);
    else
		Client(argv[3]);	
		
    KCL::Comm::Finalize();
	
    return 0;
}

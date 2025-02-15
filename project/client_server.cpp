/*
 * Simple client-server example.
 * To run the server: ./client_server 0 server client
 * To run the client: ./client_server 1 client server
 */

#include <iostream>
#include <csignal>
#include <chrono>
#include <kcl.hpp>

static volatile std::sig_atomic_t stop = 0;

void signal_handler(int) {
	stop = 1;
}

void Server(std::string source) {
	if (KCL::Comm::Listen() != 0) {
		std::cerr << "[\033[31mSERVER\033[0m]: \033[31mERROR Listen()\033[0m\n";
		exit(1);
	} else {
		std::cout << "[\033[34mSERVER\033[0m]: \033[34mListening...\033[0m\n";	
	}

    std::string msg;
    std::string process;
    
	while(!stop) {
		if (source == "ANY_SOURCE")
			KCL::Comm::Receive(KCL::KCL_ANY_SOURCE, msg, &process);
		else
			KCL::Comm::Receive(source, msg, &process);

		if (msg == "bye") {
			std::cout << "[\033[32mSERVER\033[0m]: \033[32mThe client sent the bye message!\033[0m\n";
			stop = 1;
		} else {
			std::cout << "[\033[33m" << process << "\033[0m]: \033[33m" << msg << "\033[0m\n";
		}
	}
}

void Client(std::string dest) {
	std::string msg;
	int err;

	const char spinner[] = "|/-\\";
	int i = 0;

	std::cout << "[\033[34mCLIENT\033[0m]: \033[34mEnter a message for server (" << dest << ")\n";
	std::cout << "\033[33mType 'bye' to exit." << std::flush;
	
	while (!stop) {
		std::cout << "\n\033[33m> " << std::flush;
		std::getline(std::cin, msg);
		std::cout << "\033[0m\n";
		
		while (!stop) {
		     std::cout << "\033[1A[\033[32mCLIENT\033[0m]: \033[32mSending... " << spinner[i % 4] << "\033[0m\n";
		
		     err = KCL::Comm::Send(dest, msg);

		     if (err == 1) {
		     	std::cerr << "\033[1A[\033[31mCLIENT\033[0m]: \033[31mMessage sending failed!\033[0m" << std::flush;
		        break;
		     }
		
		     if (err == 0) {
		     	std::cout << "\033[1A[\033[32mCLIENT\033[0m]: \033[32mMessage sent successfully!\033[0m" << std::flush;
		        break;
		     }
		
		     std::this_thread::sleep_for(std::chrono::milliseconds(250));
		     i++;
		}

		if (msg == "bye")
			stop = 1;
	}
	
	std::cout << "\n[\033[34mCLIENT\033[0m]: \033[34mClosed!\033[0m\n";	
}

int main(int argc, char** argv) {
    if(argc < 4) {
		std::cerr << "Usage: " << argv[0] << " <0|1> <processName> <processSource|processDest|ANY_SOURCE>\n";
        return 1;
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

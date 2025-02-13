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

void clearScreen() {
	std::cout << "\033[2J\033[H";
}

void moveCursor(int x, int y) {
	std::cout << "\033[" << y << ";" << x << "H";
}

void Server(std::string source) {
	if (KCL::Comm::Listen() != 0) {
		std::cerr << "[\033[31mSERVER\033[0m]: \033[31mERROR Listen()\033[0m\n";
		return;
	} else {
		std::cout << "[\033[34mSERVER\033[0m]: \033[34mListening...\033[0m\n";	
	}

    std::string msg;
    
	while(!stop) {
		KCL::Comm::Receive(source, msg);

		if (msg == "bye") {
			std::cout << "[\033[32mSERVER\033[0m]: \033[32mThe client sent the bye message!\033[0m\n";
			stop = 1;
		} else {
			std::cout << "[\033[33m" << source << "\033[0m]: \033[33m" << msg << "\033[0m\n";
		}
	}
}

void Client(std::string dest) {
	std::string msg;
	int err;

	const char spinner[] = "|/-\\";
	int i = 0;
	
	while (!stop) {
		clearScreen();
		moveCursor(1, 1);
		std::cout << "[\033[34mCLIENT\033[0m]: \033[34mEnter a message for server (" << dest << ")\n";
		std::cout << "\033[33mType 'bye' to exit.\n\033[33m> " << std::flush;
		
		std::getline(std::cin, msg);
		
		while (!stop) {
			 moveCursor(1, 3);
		     std::cout << "[\033[32mCLIENT\033[0m]: \033[32mSending... " << spinner[i % 4] << "\033[0m" << std::flush;
		
		     err = KCL::Comm::Send(dest, msg);

		     if (err == 1) {
		     	std::cerr << "[\033[31mCLIENT\033[0m]: \033[31mERROR Send()\033[0m" << std::flush;
		     	std::this_thread::sleep_for(std::chrono::milliseconds(500));
		        break;
		     }
		
		     if (err == 0) {
		     	moveCursor(1, 3);
		     	std::cout << "[\033[32mCLIENT\033[0m]: \033[32mMessage sent successfully!\033[0m" << std::flush;
		     	std::this_thread::sleep_for(std::chrono::milliseconds(500));
		        break;
		     }
		
		     std::this_thread::sleep_for(std::chrono::milliseconds(250));
		     i++;
		}

		if (msg == "bye")
			stop = 1;
	}
	
	moveCursor(1, 3);
	std::cout << "\033[2K[\033[32mCLIENT\033[0m]: \033[32mClosed!\033[0m\n";	
}

int main(int argc, char** argv) {
    if(argc < 4) {
		std::cerr << "Usage: " << argv[0] << " <0|1> <processName> <processSource|processDest>\n";
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

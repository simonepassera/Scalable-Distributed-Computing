#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <unistd.h>
#include <sys/wait.h>
#include <kcl.hpp>

const int WAIT_FOR_START_SECONDS = 5;

int main(int argc, char** argv) {
    int numProcesses = 2;
    int numMessages  = 10;

    if (argc >= 3) {
        numProcesses = std::stoi(argv[1]);
        numMessages  = std::stoi(argv[2]);
    } else {
        std::cout << "Usage: " << argv[0] << " <numProcesses> <numMessagesPerLink>\n";
        std::cout << "Using default values: numProcesses = " << numProcesses 
                  << ", numMessagesPerLink = " << numMessages << "\n";
    }

    std::vector<pid_t> children;

    for (int i = 0; i < numProcesses; i++) {
        pid_t pid = fork();
        
        if (pid == 0) {
            std::string processName = "process_" + std::to_string(i);

            KCL::Comm::Init(processName);

            std::cout << "[" << processName << "]: Waiting " 
                      << WAIT_FOR_START_SECONDS << " seconds...\n";

            if (KCL::Comm::Listen() != 0) {
            		std::cerr << "[\033[31m" << processName << "\033[0m]: \033[31mERROR Listen()\033[0m\n";
            		exit(1);
            }
            
            sleep(WAIT_FOR_START_SECONDS);

            auto start = std::chrono::steady_clock::now();

            for (int j = 0; j < numProcesses; j++) {
                if (j == i) continue;
                
                std::string targetName = "process_" + std::to_string(j);

                for (int m = 0; m < numMessages; m++) {
                    std::string msgContent = "Hello from " + processName + " to " + targetName + ", msg " + std::to_string(m);
                    
                    while (KCL::Comm::Send(targetName, msgContent) != 0)
                        usleep(100000); // 100 ms
                }
            }

            int expectedMessages = (numProcesses - 1) * numMessages;
            int received = 0;
            std::string msg;

            while (received < expectedMessages) {
                KCL::Comm::Receive(KCL::KCL_ANY_SOURCE, msg, nullptr);
                received++;
            }

            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed = end - start;

            std::cout << "[" << processName << "]: Total time: " 
                      << elapsed.count() << " seconds.\n";

            KCL::Comm::Finalize();
            exit(0);
        } else if (pid > 0) {
            children.push_back(pid);
        } else {
            std::cerr << "\033[31mERROR fork()\033[0m\n";
            return 1;
        }
    }

    for (pid_t child : children) {
        int status;
        waitpid(child, &status, 0);
    }
    
    std::cout << "Test finished!\n";
    return 0;
}

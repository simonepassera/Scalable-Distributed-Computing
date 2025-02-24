#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <unistd.h>
#include <sys/wait.h>
#include <kcl.hpp>

const int WAIT_FOR_START_SECONDS = 5;

int main(int argc, char* argv[]) {
    int numProcesses = 2;
    int M = 1000;

    if (argc >= 3) {
        numProcesses = std::stoi(argv[1]);
        M = std::stoi(argv[2]);

        if (numProcesses <= 1 || M <= 0) {
            std::cerr << "Usage: " << argv[0] << " <numProcesses> <totalMessages>\n";
            exit(1);
        }
    } else {
        std::cout << "Usage: " << argv[0] << " <numProcesses> <totalMessages>\n";
        std::cout << "Using default values: numProcesses = " << numProcesses 
                  << ", totalMessages = " << M << "\n";
    }

    std::vector<pid_t> children;

    for (int i = 0; i < numProcesses; i++) {
        pid_t pid = fork();
        
        if (pid == 0) {
            std::string processName = "process_" + std::to_string(i);

            KCL::Comm::Init(processName);

            if (KCL::Comm::Listen() != 0) {
            		std::cerr << "[\033[31m" << processName << "\033[0m]: \033[31mERROR Listen()\033[0m\n";
            		exit(1);
            }

            std::cout << "[" << processName << "]: Waiting " 
                      << WAIT_FOR_START_SECONDS << " seconds...\n";
            
            sleep(WAIT_FOR_START_SECONDS);

            auto start = std::chrono::steady_clock::now();

            int numMessages = M / numProcesses;
            int numTargets = numProcesses - 1;
            int base = numMessages / numTargets;
            int r = numMessages % numTargets;

            for (int j = 0; j < numTargets; j++) {
                int target = (i + 1 + j) % numProcesses;
                int messagesToSend = (j < r) ? (base + 1) : base;
                
                for (int m = 0; m < messagesToSend; m++) {
                    std::string msgContent = "Hello from " + processName +
                                             " to process_" + std::to_string(target) +
                                             ", msg " + std::to_string(m);
                    
                    while (KCL::Comm::Send("process_" + std::to_string(target), msgContent) != 0)
                        usleep(1000); // 1 ms
                }
            }

            int received = 0;
            std::string msg;

            while (received < numMessages) {
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
            exit(1);
        }
    }

    for (pid_t child : children) {
        int status;
        waitpid(child, &status, 0);
    }
    
    std::cout << "Test finished!\n";
    exit(0);
}

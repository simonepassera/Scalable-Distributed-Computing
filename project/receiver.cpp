#include "kcl.hpp"
#include <iostream>

int main() {
  KCL::Comm::Init("receiver");
  KCL::Comm::Listen();
  std::string msg, sender;
  
  KCL::Comm::Receive("sender",
                     msg,
                     &sender);
  std::cout << "Ricevuto da " << sender
            << ": " << msg << std::endl;
  KCL::Comm::Receive("sender",
                     msg,
                     &sender);
  std::cout << "Ricevuto da " << sender
            << ": " << msg << std::endl;

  KCL::Comm::Finalize();
  return 0;
}

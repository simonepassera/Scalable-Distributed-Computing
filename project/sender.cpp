#include "kcl.hpp"

int main() {
  KCL::Comm::Init("sender");

  size_t ret;
  
  do
  {
    ret = KCL::Comm::Send("receiver",
                          "Messaggio-1");
  } while (ret != 0);

  do 
  {
    ret = KCL::Comm::Send("receiver",
                          "Messaggio-2");
  } while (ret != 0);

  KCL::Comm::Finalize();
  return 0;
}

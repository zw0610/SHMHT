#include <iostream> 
#include <cstring>
#include "shared_mem_portal.h"
  
int main(int argc, char const *argv[])
{
    shared_mem_portal smp;

    std::string temp = "";
    while (!(temp=="exit"))
    {
        std::cout << "Type content to shared memory" << std::endl;
        std::cin >> temp;
        std::cout << "Writing " << temp << std::endl;
        smp.write(temp);
        std::cout << "Set " << temp << " to shared memory" << std::endl;
    }

    return 0;
}

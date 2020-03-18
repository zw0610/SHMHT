#include <iostream> 
#include <cstring>
#include "shared_mem_portal.h"

int main(int argc, char const *argv[])
{
    shared_mem_portal smp;

    std::string temp = "";
    std::string useless = "";
    while (!(useless=="exit"))
    {
        std::cout << "Press anything and Enter to read" << std::endl;
        std::cin >> useless;
        smp.read(temp);
        std::cout << "Got " << temp << " from shared memory" << std::endl;
    }
    
    return 0; 
}
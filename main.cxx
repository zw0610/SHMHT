#include <iostream>

#include "resource.h"

int main(int argc, char const *argv[])
{
    RNM rnm{};
    rnm.print_rnodes();
    rnm.print_gmem();
    std::cout << "---------------- preview ----------------" << std::endl;
    std::cout << "adding resource into rnm" << std::endl;
    rnm.add_gmem(123, 947428394, 3947572930, 1024);
    rnm.print_rnodes();
    rnm.print_gmem();
    return 0;
}

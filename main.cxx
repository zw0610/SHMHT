#include <iostream>

#include "resource.h"

int main(int argc, char const *argv[])
{
    RNM rnm{};
    std::cout << "adding resource into rnm" << std::endl;
    rnm.add_resource(123, 947428394, 3947572930, 1024);
    return 0;
}

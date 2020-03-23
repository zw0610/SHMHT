#include <stdio.h>

#include "resource_c_wrapper.hpp"

int main(int argc, char const *argv[])
{
    print_rnodes();
    print_gmem();
    printf("%s\n", "---------------- preview ----------------");
    printf("%s\n", "adding resource into rnm");
    add_gmem(123, 947428394, 3947572930, 1024);
    print_rnodes();
    print_gmem();
    return 0;
}
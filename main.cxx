#include <iostream>
#include "resource.h"

int main(int argc, const char * argv[]) {

    RNM rnm{};
    rnm.insert_rnode(std::make_tuple<int32_t, uint64_t>(312, 9475628405), RNode(312, 9475628405, 2));
    rnm.insert_rnode(std::make_tuple<int32_t, uint64_t>(36452, 2646593046), RNode(36452, 2646593046, 7)); // later removed
    rnm.print_rnode();
    
    std::cout << std::endl << "Deleting the second" << std::endl;
    rnm.delete_rnode(std::make_tuple<int32_t, uint64_t>(36452, 2646593046));
    rnm.print_rnode();
    
    std::cout << std::endl << "Adding more and trying to find" << std::endl;
    rnm.insert_rnode(std::make_tuple<int32_t, uint64_t>(12, 74653047), RNode(12, 74653047, 65));
    //rnm.print_rnode();
    rnm.insert_rnode(std::make_tuple<int32_t, uint64_t>(6385, 26465930467), RNode(6385, 26465930467, 77));
    rnm.print_rnode();
    auto x = rnm.find_rnode(std::make_tuple<int32_t, uint64_t>(12, 74653047));
    if (x != nullptr) {
        std::cout << "found val: " << *x << " associated with key: (12, 74653047)" << std::endl;
    } else {
        std::cout << "no element associated with (12, 74653047)" << std::endl;
    }
    auto y = rnm.find_rnode(std::make_tuple<int32_t, uint64_t>(36452, 2646593046));
    if (y != nullptr) {
        std::cout << "found val: " << *x << " associated with key: (36452, 2646593046)" << std::endl;
    } else {
        std::cout << "no element associated with (36452, 2646593046)" << std::endl;
    }

    return 0;
}

//
//  resource_c_wrapper.c
//  shmht
//
//  Created by Wang Zhang on 3/23/20.
//  Copyright © 2020 Wang Zhang. All rights reserved.
//

#include "resource_c_wrapper.hpp"
#include "resource.hpp"

static RNM rnm;

extern "C" void RNM_init(const int len_rnode, const int len_gmem) {
    rnm = RNM(len_rnode, len_gmem);
}

extern "C" void print_rnodes(void) {
    rnm.print_rnodes();
}

extern "C" void print_gmem(void) {
    rnm.print_gmem();
}

extern "C" void add_gmem(const int pid, const long stime, unsigned long long dptr, const unsigned int bytes) {
    rnm.add_gmem(pid, stime, dptr, bytes);
}
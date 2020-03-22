//
//  gmem.h
//  shmht
//
//  Created by Wang Zhang on 3/22/20.
//  Copyright © 2020 Wang Zhang. All rights reserved.
//
#include <iostream>

#ifndef gmem_h
#define gmem_h

typedef unsigned int CUdeviceptr;

struct GMem
{
    CUdeviceptr ptr;
    std::size_t bytes;
    int next;
    
    GMem()
    {
        initialize();
    }
    
    GMem(CUdeviceptr new_ptr, const std::size_t new_bytes) {
        set_res(new_ptr, new_bytes);
        set_next(-1);
    }
    
    void initialize(void)
    {
        ptr = 0;
        bytes = 0;
        next = -1;
    }
    
    void set_next(const int idx)
    {
        next = idx;
    }
    
    void set_res(CUdeviceptr new_ptr, const std::size_t new_bytes)
    {
        ptr = new_ptr;
        bytes = new_bytes;
    }
    
    bool empty(void) {
        return ptr == 0;
    }
    
    bool is_tail(void) {
        // ptr == 0: empty, neither head nor tail, return false
        // ptr != 1 && next != -1, not tail, return false
        return (ptr!=0) && (next==-1);
    }
    
    void shallow_delete(void) {
        ptr = 0;
        next = -1;
    }
    
    friend std::ostream &operator<<(std::ostream &os, const GMem &gm);
};

std::ostream &operator<<(std::ostream &os, const GMem &rn);

#endif /* gmem_h */

#pragma once

#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <iostream>
#include <tuple>
#include <assert.h>

#include "shmap.h"

typedef unsigned int CUdeviceptr;
typedef std::tuple<int32_t, uint64_t> pid_st;

enum RNodeStatus
{
    NONEXIST,
    DELETED,
    EXIST
};

struct Resource
{
    CUdeviceptr ptr;
    std::size_t bytes;
    int next;

    Resource()
    {
        initialize();
    }

    Resource(CUdeviceptr new_ptr, const std::size_t new_bytes) {
        set_res(new_ptr, new_bytes);
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
};

struct RNode
{
    int pid;
    unsigned long t;
    int res_entry_idx;
    RNodeStatus stat;

    RNode()
    {
        this->initialize();
    }

    // Copy Constructor
    RNode(const RNode &rn)
    {
        this->pid = rn.pid;
        this->t = rn.t;
        this->res_entry_idx = rn.res_entry_idx;
        this->stat = rn.stat;
    }

    RNode(const int pid, const unsigned long t, const int res_entry_idx)
    {
        this->pid = pid;
        this->t = t;
        this->res_entry_idx = res_entry_idx;
        this->stat = RNodeStatus::EXIST;
    }

    RNode &operator=(const RNode &rn)
    {
        this->pid = rn.pid;
        this->t = rn.t;
        this->res_entry_idx = rn.res_entry_idx;
        this->stat = rn.stat;
        return *this;
    }

    // need a copy operator here;

    void initialize(void)
    {
        pid = -1;
        t = 0;
        res_entry_idx = -1;
        stat = RNodeStatus::NONEXIST;
    }

    void set_key(const int pid, const unsigned long t)
    {
        this->pid = pid;
        this->t = t;
    }

    void set_record(const int idx)
    {
        res_entry_idx = idx;
    }

    void print(void)
    {
        std::cout << "PID: " << pid << " time: " << t << " res_entry_idx: " << res_entry_idx << std::endl;
    }

    RNodeStatus get_stat(void)
    {
        return stat;
    }

    bool same_key(const int pid, const unsigned long t) const
    {
        return pid == this->pid && t == this->t;
    }

    void shallow_delete(void)
    {
        stat = RNodeStatus::DELETED;
    }

    // resource related APIs
    void add_resource(const std::size_t r_idx, const SharedMemoryArrayPortal<Resource>& rap) {
        int local_idx = this->res_entry_idx;

        if (local_idx == -1)
        {
            res_entry_idx = r_idx;
            return;
        }
        
        while (rap[local_idx].next != -1)
        {
            local_idx = rap[local_idx].next;
        }
        
        rap[local_idx].next = r_idx;
        assert(rap[r_idx].next == -1);
    }

    bool remove_resource_by_dptr(const CUdeviceptr dptr, SharedMemoryArrayPortal<Resource>& rap) {
        int r_idx = this->res_entry_idx;
        if (rap[r_idx].ptr == dptr)
        {
            remove_resource_by_index(r_idx, rap);
            res_entry_idx = -1;
            return true;
        }
        
        do
        {
            int parent_r_idx = r_idx;
            r_idx = rap[r_idx].next;
            if (rap[r_idx].ptr == dptr)
            {
                remove_resource_by_index(r_idx, rap);
                rap[parent_r_idx].next = rap[r_idx].next;
                return true;
            }
            
        } while (r_idx != -1);
        
        return false;
    }

    void remove_resource_by_index(const std::size_t r_idx, SharedMemoryArrayPortal<Resource>& rap) {
        rap[r_idx].ptr = 0;
    }

    void remove_all_resource(SharedMemoryArrayPortal<Resource>& rap) {
        for (int i = this->res_entry_idx; i != -1; i = rap[i].next)
        {
            rap[i].ptr = 0;
        }
        
    }

    friend std::ostream &operator<<(std::ostream &os, const RNode &rn);
};

std::ostream &operator<<(std::ostream &os, const RNode &rn);

class RNM
{
    key_t key_rna;
    int shm_id_rna;
    SharedMemoryArrayPortal<RNode> rnap;
    std::size_t hash_d;

    key_t key_ra;
    int shm_id_ra;
    SharedMemoryArrayPortal<Resource> rap;
    
private:
    void init_rna(const std::size_t bytes);

    void init_ra(const std::size_t bytes);

    void init(const std::size_t len_rna, const std::size_t len_ra);

    void init_ptrs(void);

    const std::size_t pre_hash(const pid_st &key) const;

    const std::size_t hash(const pid_st &key, const int i) const;

    RNode *insert_rnode(const pid_st &key, const RNode &val);

    RNode *find_rnode(const pid_st &key) const;

    void delete_rnode(const pid_st &key);

    const int push_resource(const Resource& r);

    void remove_resource(const std::size_t idx);

public:
    RNM();

    RNM(const std::size_t len_rna, const std::size_t len_ra);

    RNM(const RNM &rnm);

    bool contain(const pid_st &key) const;

    void print_rnodes(void) const;

    void add_resource(const int32_t pid, const uint64_t stime, CUdeviceptr dptr, const std::size_t bytes);
    
    void remove_resource(const int32_t pid, const uint64_t stime, CUdeviceptr dptr);
};
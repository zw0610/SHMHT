#pragma once

#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <iostream>
#include <tuple>

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
    CUdeviceptr *ptr;
    std::size_t bytes;
    int next;

    Resource()
    {
        ptr = NULL;
        bytes = 0;
        next = -1;
    }

    void initialize(void)
    {
        ptr = NULL;
        bytes = 0;
        next = -1;
    }

    void set_next(const int idx)
    {
        next = idx;
    }

    void set_res(CUdeviceptr *new_ptr, std::size_t new_bytes)
    {
        ptr = new_ptr;
        bytes = new_bytes;
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

public:
    RNM();

    RNM(const std::size_t len_rna, const std::size_t len_ra);

    RNM(const RNM &rnm);

    void init(const std::size_t len_rna, const std::size_t len_ra);

    void init_ptrs(void);

    const std::size_t pre_hash(const pid_st &key) const;

    const std::size_t hash(const pid_st &key, const int i) const;

    RNode *insert_rnode(const pid_st &key, const RNode &val);

    RNode *find_rnode(const pid_st &key) const;

    void delete_rnode(const pid_st &key);

    void print_rnodes(void) const;

    void add_resource(const int32_t pid, const uint64_t stime, const CUdeviceptr dptr, const std::size_t bytes);
    
    void remove_resource(const int32_t pid, const uint64_t stime, const CUdeviceptr dptr, const std::size_t bytes);
};
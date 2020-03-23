//
//  rnode.h
//  shmht
//
//  Created by Wang Zhang on 3/22/20.
//  Copyright © 2020 Wang Zhang. All rights reserved.
//

#include <string>
#include <iostream>
#include <tuple>

#ifndef rnode_h
#define rnode_h

typedef std::tuple<int32_t, uint64_t> pid_st;

enum RNodeStatus
{
    NONEXIST=0,
    DELETED,
    EXIST
};

struct RNode
{
    int pid;
    unsigned long t;
    int res_entry_idx;
    RNodeStatus stat;
    
    RNode()
    {
        initialize();
    }
    
    // Copy Constructor
    RNode(const RNode &rn)
    {
        this->pid = rn.pid;
        this->t = rn.t;
        this->res_entry_idx = rn.res_entry_idx;
        this->stat = rn.stat;
    }
    
    RNode(const int npid, const unsigned long nt, const int nres_entry_idx)
    {
        this->pid = npid;
        this->t = nt;
        this->res_entry_idx = nres_entry_idx;
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
    
    void set_key(const pid_st& key)
    {
        pid = std::get<0>(key);
        t = std::get<1>(key);
    }
    
    void set_rec_entry(const int idx)
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
    
    bool same_key(const pid_st& key) const
    {
        return pid == std::get<0>(key) && t == std::get<1>(key);
    }
    
    void shallow_delete(void)
    {
        stat = RNodeStatus::DELETED;
    }
    
    const int get_entry(void) {
        return res_entry_idx;
    }
    
    friend std::ostream &operator<<(std::ostream &os, const RNode &rn);
};

std::ostream &operator<<(std::ostream &os, const RNode &rn);

#endif /* rnode_h */

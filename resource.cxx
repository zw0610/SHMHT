#include "resource.h"

#include <errno.h>
#include <string.h>
#include <assert.h>


RNM::RNM()
{   
    constexpr std::size_t len_rna = 101;
    constexpr std::size_t len_ra = 1024;
    init(len_rna, len_ra);
}

RNM::RNM(const RNM &rnm)
{
    key_rna = rnm.key_rna;
    shm_id_rna = rnm.shm_id_rna;
    rnap = rnm.rnap;
    hash_d = rnm.hash_d;

    key_ra = rnm.key_ra;
    shm_id_ra = rnm.shm_id_ra;
    rap = rnm.rap;
}

RNM::RNM(const std::size_t len_rna, const std::size_t len_ra) {
    init(len_rna, len_ra);
}

void RNM::init(const std::size_t len_rna, const std::size_t len_ra) {
    // init RNode array in shared memory
    key_rna = 0x7214;
    hash_d = len_rna;
    std::size_t bytes_rna = len_rna * sizeof(RNode);
    
    this->init_rna(bytes_rna);
    RNode* rna_ptr = (RNode*)shmat(shm_id_rna, NULL, 0);

    rnap = SharedMemoryArrayPortal<RNode>(shm_id_rna, rna_ptr, len_rna, bytes_rna);
    
    // init Resource array in shared memory
    key_ra = 0x4127;
    std::size_t bytes_ra = len_ra * sizeof(Resource);

    this->init_ra(bytes_ra);
    Resource* ra_ptr = (Resource*)shmat(shm_id_ra, NULL, 0);

    rap = SharedMemoryArrayPortal<Resource>(shm_id_ra, ra_ptr, len_ra, bytes_ra);
}

void RNM::init_rna(const std::size_t bytes) {
    shm_id_rna = shmget(key_rna, bytes, 0666|IPC_CREAT);
}

void RNM::init_ra(const std::size_t bytes) {
    shm_id_ra = shmget(key_ra, bytes, 0666|IPC_CREAT);
}

const std::size_t RNM::pre_hash(const pid_st &key) const
{
    // temporary implemantation
    std::size_t seed = std::get<0>(key);
    seed = seed << 6;
    seed += std::size_t(std::get<1>(key));
    return seed;
}

const std::size_t RNM::hash(const pid_st &key, const int i) const
{
    const std::size_t seed = pre_hash(key);
    return (seed + i) % hash_d;
}

RNode *RNM::insert_rnode(const pid_st &key, const RNode &val)
{
    for (int i = 0; i < rnap.size(); i++)
    {
        std::size_t pos = hash(key, i);
        if (rnap[pos].get_stat() == RNodeStatus::NONEXIST || rnap[pos].get_stat() == RNodeStatus::DELETED)
        {
            //std::cout << "inserting at " << pos << " with value: " << val << std::endl;
            rnap[pos] = val;
            return &rnap[pos];
        }
        // rna[pos].get_stat() == RNodeStatus::EXIST
        if (rnap[pos].same_key(std::get<0>(key), std::get<1>(key)))
        {
            rnap[pos] = val;
            return &rnap[pos];
        }
        
    }
    return nullptr;
}

RNode *RNM::find_rnode(const pid_st &key) const
{
    for (int i = 0; i < rnap.size(); i++)
    {
        std::size_t pos = hash(key, i);
        if (rnap[pos].get_stat() == RNodeStatus::NONEXIST)
        {
            return nullptr;
        }
        else if (rnap[pos].get_stat() == RNodeStatus::EXIST)
        {
            if (rnap[pos].same_key(std::get<0>(key), std::get<1>(key)))
            {
                return &rnap[pos];
            }
        }
    }

    return nullptr;
}

void RNM::delete_rnode(const pid_st &key)
{
    RNode *del_ptr = find_rnode(key);
    if (del_ptr != nullptr)
    {
        del_ptr->shallow_delete();
    }
}

bool RNM::contain(const pid_st &key) const {
    return find_rnode(key) != nullptr;
}

void RNM::print_rnodes(void) const
{
    std::cout << "Listing items from Resource Node Map:" << std::endl;
    std::size_t count = 0;
    for (int i = 0; i < rnap.size(); i++)
    {
        RNode &rna_ref = rnap[i];
        if (rna_ref.get_stat() == RNodeStatus::EXIST)
        {
            rna_ref.print();
            count++;
        }
    }
    if (count == 0)
    {
        std::cout << "None" << std::endl;
    }
}

std::ostream &operator<<(std::ostream &os, const RNode &rn)
{
    os << "PID: " << rn.pid << " time: " << rn.t << " res_entry_idx: " << rn.res_entry_idx;
    return os;
}

const int RNM::push_resource(const Resource& r) {
    for (size_t i = 0; i < rap.size(); i++)
    {
        std::cout << "checking resource record: " << i << std::endl;
        Resource& rn = rap[i];
        std::cout << rn.bytes << std::endl;
        std::cout << "check if empty" << std::endl;
        if (rn.empty())
        {
            std::cout << "found empty slot at " << i << std::endl; 
            rn.set_res(r.ptr, r.bytes);
            return int(i);
        }
        
    }
    // if the shared memory is full:
    return -1;
}


void RNM::add_resource(const int32_t pid, const uint64_t stime, CUdeviceptr dptr, const std::size_t bytes) {
    // prepare data
    const pid_st key = std::make_tuple(pid, stime);
    const Resource r = Resource(dptr, bytes);

    // push the record to shared memory
    std::cout << "pushing resource" << std::endl;
    const int r_idx = this->push_resource(r);

    // check if the rnode exist
    std::cout << "finding rnode" << std::endl;
    RNode* target_rn = find_rnode(key);
    if (target_rn != nullptr)
    {
        std::cout << "found rnode, adding resource" << std::endl;
        target_rn->add_resource(r_idx, rap);
    }
    else
    {
        // if the node does not exist
        // create node first
        std::cout << "rnode not found, adding rnode first" << std::endl;
        RNode val_rn = RNode(pid, stime, 1);
        RNode* rn = this->insert_rnode(key, val_rn);
        // then insert resource
        std::cout << "adding resource after node inserted" << std::endl; 
        rn->add_resource(r_idx, rap);
    }
}

void RNM::remove_resource(const int32_t pid, const uint64_t stime, CUdeviceptr dptr) {
    const pid_st key = std::make_tuple(pid, stime);
    RNode* target_rn = find_rnode(key);

    bool res = target_rn->remove_resource_by_dptr(dptr, rap);
    assert(res);
}
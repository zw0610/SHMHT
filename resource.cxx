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
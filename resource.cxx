#include "resource.h"

#include <errno.h>
#include <string.h>

RNM::RNM()
{
    rna_len = 101;
    ra_len = 1024;
    hash_d = 101;
    key_rna = 0x7214;
    rna = NULL;
    std::cout << "Size of RNode is " << sizeof(RNode) << std::endl;
    rna_bytes = rna_len * sizeof(RNode);
    this->init_ptrs();
    rna = (RNode*)shmat(shm_id_rna, NULL, 0);
}

RNM::RNM(const RNM &rnm)
{
    rna = rnm.rna;
    rna_len = rnm.rna_len;
    key_rna = rnm.key_rna;
    rna_bytes = rnm.rna_bytes;

    ra = rnm.ra;
    ra_len = rnm.ra_len;

    hash_d = rnm.hash_d;
}

void RNM::init_ptrs(void)
{
    // rna = (RNode *)malloc(rna_len * sizeof(RNode));
    // for (std::size_t i = 0; i < rna_len; i++)
    // {
    //     rna[i].initialize();
    // }
    shm_id_rna = shmget(key_rna, rna_bytes, 0666|IPC_CREAT);
    if (shm_id_rna == -1)
    {
        printf("shmget failed: %d, %s\n", errno, strerror(errno));
    }
    
    assert(shm_id_rna != -1);
    

    ra = (Resource *)malloc(ra_len * sizeof(Resource));
    for (std::size_t i = 0; i < ra_len; i++)
    {
        ra[i].initialize();
    }
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
    for (int i = 0; i < rna_len; i++)
    {
        std::size_t pos = hash(key, i);
        if (rna[pos].get_stat() == RNodeStatus::NONEXIST || rna[pos].get_stat() == RNodeStatus::DELETED)
        {
            //std::cout << "inserting at " << pos << " with value: " << val << std::endl;
            rna[pos] = val;
            return rna + pos;
        }
        // rna[pos].get_stat() == RNodeStatus::EXIST
        if (rna[pos].same_key(std::get<0>(key), std::get<1>(key)))
        {
            rna[pos] = val;
            return rna + pos;
        }
        
    }
    return nullptr;
}

RNode *RNM::find_rnode(const pid_st &key) const
{
    for (int i = 0; i < rna_len; i++)
    {
        std::size_t pos = hash(key, i);
        if (rna[pos].get_stat() == RNodeStatus::NONEXIST)
        {
            return nullptr;
        }
        else if (rna[pos].get_stat() == RNodeStatus::EXIST)
        {
            if (rna[pos].same_key(std::get<0>(key), std::get<1>(key)))
            {
                return rna + pos;
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

void RNM::print_rnode(void)
{
    std::cout << "Listing items from Resource Node Map:" << std::endl;
    std::size_t count = 0;
    for (int i = 0; i < rna_len; i++)
    {
        RNode &rna_ref = rna[i];
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
#include "resource.h"

#include <errno.h>
#include <string.h>
#include <assert.h>

std::ostream &operator<<(std::ostream &os, const RNode &rn)
{
    os << "PID: " << rn.pid << " time: " << rn.t << " res_entry_idx: " << rn.res_entry_idx;
    return os;
}

std::ostream &operator<<(std::ostream &os, const GMem &gm) {
    os << "dptr (" << gm.ptr << ") with size of " << gm.bytes << " bytes and pointing to " << gm.next;
    return os;
}

/*----------------------------------------------------------------------*/

RNM::RNM()
{   
    len_rnode = 101;
    len_gmem = 1024;
    init();
}

RNM::RNM(const RNM &rnm)
{
    // RNode related copy constructor
    key_rnode = rnm.key_rnode;
    shm_id_rnode = rnm.shm_id_rnode;
    ptr_rnode = rnm.ptr_rnode;
    len_rnode = rnm.len_rnode;
    hash_d = rnm.hash_d;

    // GMem related copy constructor
    key_gmem = rnm.key_gmem;
    shm_id_gmem = rnm.shm_id_gmem;
    ptr_gmem = rnm.ptr_gmem;
    len_gmem = rnm.len_gmem;
}

RNM::RNM(const int len_rnode, const int len_gmem) {
    this->len_rnode = len_rnode;
    this->len_gmem = len_gmem;
    init();
}

RNM::~RNM() {
    shmdt(ptr_rnode);
    shmdt(ptr_gmem);
}

void RNM::init(void) {
    // init RNode array in shared memory
    key_rnode = 0x7C00;
    hash_d = len_rnode;
    std::size_t bytes_rnode = len_rnode * sizeof(RNode);
    
    bool if_rnode_created = init_rnode(bytes_rnode);
    ptr_rnode = (RNode*)shmat(shm_id_rnode, NULL, 0);
    if (!if_rnode_created) {
        for (int i=0; i<len_rnode; i++) {
            ptr_rnode[i].initialize();
        }
    }
    
    // init Resource array in shared memory
    key_gmem = 0x7E00;
    std::size_t bytes_gmem = len_gmem * sizeof(GMem);

    bool if_gmem_created = init_gmem(bytes_gmem);
    ptr_gmem = (GMem*)shmat(shm_id_gmem, NULL, 0);
    if (!if_gmem_created) {
        for (int i=0; i<len_gmem; i++) {
            ptr_gmem[i].initialize();
        }
    }
}

bool RNM::init_rnode(const std::size_t bytes) {
    shm_id_rnode = shmget(key_rnode, bytes, 0666|IPC_CREAT|IPC_EXCL);
    if (shm_id_rnode == -1) {
        // created, return true
        shm_id_rnode = shmget(key_rnode, bytes, 0666|IPC_CREAT);
        return true;
    }
    return false;
}

bool RNM::init_gmem(const std::size_t bytes) {
    shm_id_gmem = shmget(key_gmem, bytes, 0666|IPC_CREAT|IPC_EXCL);
    if (shm_id_gmem == -1) {
        shm_id_gmem = shmget(key_gmem, bytes, 0666|IPC_CREAT);
        return true;
    }
    return false;
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
    for (int i = 0; i < len_rnode; i++)
    {
        std::size_t pos = hash(key, i);
        if (ptr_rnode[pos].get_stat() == RNodeStatus::NONEXIST || ptr_rnode[pos].get_stat() == RNodeStatus::DELETED)
        {
            //std::cout << "inserting at " << pos << " with value: " << val << std::endl;
            ptr_rnode[pos] = val;
            return &ptr_rnode[pos];
        }
        // rna[pos].get_stat() == RNodeStatus::EXIST
        if (ptr_rnode[pos].same_key(key))
        {
            ptr_rnode[pos] = val;
            return &ptr_rnode[pos];
        }
        
    }
    // if nullptr is returned, it means the rnode hash table is full
    return nullptr;
}

RNode *RNM::find_rnode(const pid_st &key) const
{
    for (int i = 0; i < len_rnode; i++)
    {
        std::size_t pos = hash(key, i);
        if (ptr_rnode[pos].get_stat() == RNodeStatus::NONEXIST)
        {
            // not found
            return nullptr;
        }
        else if (ptr_rnode[pos].get_stat() == RNodeStatus::EXIST)
        {
            if (ptr_rnode[pos].same_key(key))
            {
                //std::cout << "found rnode: " << ptr_rnode[pos] << std::endl;
                return ptr_rnode + pos;
            }
            // if the rnode exist, but not match, check the next
        }
        // if the stat is DELETED, check the next
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
    for (int i = 0; i < len_rnode; i++)
    {
        RNode &rna_ref = ptr_rnode[i];
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

void RNM::print_gmem(void) const {
    std::cout << "Listing items from GPU Memory:" << std::endl;
    std::size_t count = 0;
    for (int i = 0; i < len_gmem; i++)
    {
        GMem &gm_ref = ptr_gmem[i];
        if (!gm_ref.empty())
        {
            std::cout << gm_ref << std::endl;
            count++;
        }
    }
    if (count == 0)
    {
        std::cout << "None" << std::endl;
    }
}

const int RNM::push_gmem(const GMem &gm){
    for (int i = 0; i < len_gmem; i++)
    {
        //std::cout << "checking resource record: " << i << std::endl;
        if (ptr_gmem[i].empty())
        {
//            std::cout << "found empty slot at " << i << std::endl;
            ptr_gmem[i].set_res(gm.ptr, gm.bytes);
            ptr_gmem[i].set_next(-1);
            return i;
        }
        
    }
    // if the shared memory is full:
    return -1;
}

const int RNM::last_rec(const int idx) const {
    if (ptr_gmem[idx].is_tail()) {
        return idx;
    }
    return last_rec(ptr_gmem[idx].next);
}

void RNM::link_rnode_gmem(RNode *rn, const int gmem_idx) {
    if (rn->get_entry() == -1) {
        rn->set_rec_entry(gmem_idx);
    } else {
        const int tail_idx = last_rec(rn->get_entry());
        if (tail_idx != gmem_idx) {
            ptr_gmem[tail_idx].set_next(gmem_idx);
        }
    }
}

const int RNM::find_gmem(const RNode * rn, const CUdeviceptr dptr) const {
    int g_idx = rn->res_entry_idx;
    while (!ptr_gmem[g_idx].empty()) {
        if (ptr_gmem[g_idx].ptr == dptr) {
            return g_idx;
        }
        g_idx = ptr_gmem[g_idx].next;
    }
    return -1;
}

void RNM::add_gmem(const int32_t pid, const uint64_t stime, CUdeviceptr dptr, const std::size_t bytes) {
    // prepare data
    const pid_st key = std::make_tuple(pid, stime);
    const GMem gm = GMem(dptr, bytes);
    
    // check if the rnode exist
    //std::cout << "finding rnode" << std::endl;
    RNode* target_rn = find_rnode(key);

    if (target_rn != nullptr)
    {
        // push the record to shared memory
        //std::cout << "pushing gmem record" << std::endl;
        int g_idx = find_gmem(target_rn, dptr);
        if (g_idx == -1) {
            g_idx = push_gmem(gm);
        }

        // if the process already exists
        //std::cout << "found rnode, adding gmem record" << std::endl;
        link_rnode_gmem(target_rn, g_idx);
    }
    else
    {
        // if the node does not exist
        // create node first
        //std::cout << "rnode not found, adding rnode first" << std::endl;
        RNode val_rn = RNode(pid, stime, -1);
        RNode* rn = insert_rnode(key, val_rn);
        
        // push the record to shared memory
        //std::cout << "pushing gmem record" << std::endl;
        int g_idx = find_gmem(rn, dptr);
        if (g_idx == -1) {
            g_idx = push_gmem(gm);
        }

        // then insert resource
        //std::cout << "adding gmem record after node inserted" << std::endl;
        link_rnode_gmem(rn, g_idx);
        }
}

void RNM::remove_gmem_by_dptr(const int prev_idx, const int g_idx, CUdeviceptr dptr) {
    if (ptr_gmem[g_idx].empty()) {
        return;
    }
    if (ptr_gmem[g_idx].ptr == dptr) {
        if (prev_idx == -1) {
            ptr_gmem[g_idx].shallow_delete();
        } else {
            ptr_gmem[prev_idx].set_next(ptr_gmem[g_idx].next);
            ptr_gmem[g_idx].shallow_delete();
        }
    }
    remove_gmem_by_dptr(g_idx, ptr_gmem[g_idx].next, dptr);
}

void RNM::remove_gmem(const int32_t pid, const uint64_t stime, CUdeviceptr dptr) {
    const pid_st key = std::make_tuple(pid, stime);
    RNode* target_rn = find_rnode(key);

    remove_gmem_by_dptr(-1, target_rn->get_entry(), dptr);
}

#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <stdio.h>
#include <cstring>
#include <iostream>
#include <assert.h>
#include <tuple>

typedef std::tuple<int, long> pid_st;

struct shared_mem_portal
{
    key_t key;

    char * local_ptr;
    size_t len;
    size_t bytes;

    int shm_id;
    
    shared_mem_portal() {
        key = ftok("shmfile", 65);
        local_ptr = NULL;
        len = 128;
        bytes = sizeof(len*sizeof(char));
        // create shared memory if not exist
        shm_id = shmget(key, bytes, 0666|IPC_CREAT);

        assert(shm_id != -1);
    }

    ~shared_mem_portal() {
        // there is no need to clear the memory as other process
        // may still be running and need the shared memory
        
        // OR we may choose to delete the shared memory with static
        // count shows it's the last process owning it
    }

    int write(std::string & str) {
        std::cout << "setting string: " << str << std::endl;
        local_ptr = (char*)shmat(shm_id, NULL, 0);
        assert(local_ptr != NULL);

        std::cout << "copy" << std::endl;
        strcpy(local_ptr, str.c_str());

        std::cout << "detach" << std::endl;
        int ret = shmdt(local_ptr);
        if (ret != 0) std::cout << "detach failed!" << std::endl; 
        return 0;
    }

    int read(std::string & str) {

        local_ptr= (char *)shmat(shm_id, NULL, 0);
        assert(local_ptr != NULL);
        
        str = std::string(local_ptr);
        std::cout << "Got: " << str << std::endl;

        int ret = shmdt(local_ptr);
        if (ret != 0) std::cout << "detach failed!" << std::endl; 
        return 0;
    }
};
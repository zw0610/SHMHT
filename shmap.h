#pragma once

#include <sys/ipc.h>
#include <sys/shm.h>
#include <iostream>

template <typename T>
class SharedMemoryArrayPortal
{
    int shm_id;
    T *local_ptr;
    std::size_t len;
    std::size_t bytes;
    bool attached;

public:
    SharedMemoryArrayPortal()
    {
        shm_id = -1;
        local_ptr = nullptr;
        len = 0;
        bytes = len * sizeof(T);
        attached = false;
    }

    SharedMemoryArrayPortal(const int shm_id, T *local_ptr, const std::size_t len, const std::size_t bytes)
    {
        this->shm_id = shm_id;
        this->local_ptr = local_ptr;
        this->len = len;
        this->bytes = bytes;
        attached = false;
        attach();
    }

    ~SharedMemoryArrayPortal()
    {
        detach();
        // there is no need to clear the memory as other process
        // may still be running and need the shared memory

        // OR we may choose to delete the shared memory with static
        // count shows it's the last process owning it
    }

    const std::size_t size(void) const {
        return len;
    }

    bool ready(void) {
        return attached;
    }

    bool attach(void) {
        local_ptr = (T*)shmat(shm_id, NULL, 0);
        attached = local_ptr != nullptr;
        return attached;
    }

    bool detach(void) {
        attached = !(shmdt(local_ptr) == 0);
        return !attached;
    }

    void write(const int idx, const T& val)
    {
        if (attached)
        {
            local_ptr[idx] = val;
        }
        else
        {
            std::cout << "cannot write, not attached" << std::endl;
        }
    }

    void read(const int idx, T& val) {
        // read content to val
        if (attached) {
            val = local_ptr[idx];
        } else {
            std::cout << "connot read, not attached" << std::endl;
        }
    }

    T& operator[](const int idx) const {
        return local_ptr[idx];
    }
};
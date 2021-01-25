# Shm-Kernel Project

## Memory Manager

This is a component of shm-kernel, which is used to manage the shared memory object. It is not designed to directly interact with shm obj, instead, manage the memory via memmory map. There are two major classes which is close to the user - mmgr and Segment.


There are 3 types of bins.
1. Static Bin
2. Instant Bin
3. Cache Bin

#### Static Bin
Static bin is used to store medium size data which is larger than 1KB and less than 1 MB. The data is stored in a pre-allocated shared memory object and is managed data. 

#### Instant Bin
Instant bin is used to store huge data which is normally larger than 1MB to **INF**. Each allocate each create a new shared memory object, and once it is finished, it will be destroyed.
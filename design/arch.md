# Memory Manager

This is a component of shm-kernel, which is used to manage the shared memory object. It is not designed to directly interact with shm obj, instead, manage the memory via proxy. There are two major classes which is close to the user - Arena and Segment.

## Arena


There are 3 types of bins.
1. Static Bin
2. Instant Bin
3. Cache Bin

### Static Bin

### Instant Bin

### Cache Bin
Cache bin is used to store tiny data. each Arena can only have **ONE** Cache Bin. Each Cache bin has **FOUR** buffer areas. Which is used to **TRANSFER** data only. 2 for write and 2 for read.
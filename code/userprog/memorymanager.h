#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include "bitmap.h"

class Thread;
class Lock;
#include "synch.h"


class MemoryManager{
public:
    MemoryManager(int numPages);	
    ~MemoryManager();	
    int AllocPage(int flag);
    void FreePage(int physPageNum);
    bool PageIsAllocated(int physPageNum);
    int GetFreePageCount();
private:
    int freePageCount;
    BitMap * bitmap;
    Lock *memoryLock;
};

#endif

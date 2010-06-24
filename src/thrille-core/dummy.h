
void myMemoryRead(int iid, void * addr);
void myMemoryWrite(int iid, void * addr);
void thrilleAssertC(int);
void thrilleAssertCPP(bool);
void * thrilleMallocCPP(size_t s);
void * thrilleMallocC(size_t s);

extern "C" {
    void thrilleAssertC(int);
    void * thrilleMallocC(size_t);
}



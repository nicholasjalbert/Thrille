/*
 * Author - Nick Jalbert (jalbert@cs.berkeley.edu)
 *
 */

#include "pin.H"
#include "assert.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>

PIN_LOCK global_lock;

/* Fast buffering from PIN tutorial */
BUFFER_ID bufId;
TLS_KEY mlog_key;

#define NUM_BUF_PAGES 1024
 
struct MemRef {
    ADDRINT ip; 
    UINT32 opcount;
    ADDRINT op1_addr;
    BOOL op1_iswrite;
    ADDRINT op1_size;
    ADDRINT op2_addr;
    BOOL op2_iswrite;
    ADDRINT op2_size;
};

class MemLog {
  public:
    MemLog(THREADID tid);
    ~MemLog();

    VOID DumpBufferToFile(struct MemRef* reference, 
            UINT64 numElements, 
            THREADID tid);

  private:
    ofstream _ofile;
};


MemLog::MemLog(THREADID tid) {
    string filename = "log-" + decstr(PIN_GetPid()) + "." + decstr(tid);
    _ofile.open(filename.c_str());
    if (! _ofile) {
        cerr << "Error: could not open output file." << endl;
        exit(1);
    }
    _ofile << hex;
}


MemLog::~MemLog() {
    _ofile.close();
}


ADDRINT ip; 
    UINT32 opcount;
    ADDRINT op1_addr;
    BOOL op1_iswrite;
    ADDRINT op1_size;
    ADDRINT op2_addr;
    BOOL op2_iswrite;
    ADDRINT op2_size;

VOID MemLog::DumpBufferToFile(struct MemRef * reference,
        UINT64 numElements,
        THREADID tid ) {
    for (UINT64 i = 0; i < numElements; i++, reference++) {
        _ofile << "INS" << endl;
        _ofile << "IP:0x" << reference->ip << endl;
        _ofile << "OPCOUNT:" << dec << reference->opcount << hex << endl;
        _ofile << "OP1ADDR:0x" << reference->op1_addr << endl;
        _ofile << "OP1WRITE:" << reference->op1_iswrite << endl;
        _ofile << "OP1SIZE:" << dec << reference->op1_size << hex << endl;
        _ofile << "OP2ADDR:0x" << reference->op2_addr << endl;
        _ofile << "OP2WRITE:" << reference->op2_iswrite << endl;
        _ofile << "OP2SIZE:" << dec << reference->op2_size << hex << endl;
    }
}

VOID Trace(THREADID thread,
        ADDRINT ip,
        UINT32 memop_count,
        ADDRINT op1_addr,
        BOOL op1_iswrite,
        ADDRINT op2_addr,
        BOOL op2_iswrite) {
    cout << memop_count << endl;
    //cout << hex << "0x" << ip << dec << endl;

}

BOOL InMainExecutable(TRACE trace) {
    RTN rtn = TRACE_Rtn(trace);
    if (! RTN_Valid(rtn)) {
        return false;
    }
    SEC sec = RTN_Sec(rtn);
    if (! SEC_Valid(sec)) {
        return false;
    }
    IMG img = SEC_Img(sec);
    if (! IMG_Valid(img)) {
        return false;
    }
    return IMG_IsMainExecutable(img);
}

VOID TraceMemory(TRACE trace, VOID* v) {
    if (! InMainExecutable(trace)) {
        return;
    }
    for (BBL bbl = TRACE_BblHead(trace);
            BBL_Valid(bbl);
            bbl = BBL_Next(bbl)) {
        for (INS ins = BBL_InsHead(bbl);
                INS_Valid(ins);
                ins = INS_Next(ins)) {
            if (INS_IsMemoryRead(ins) || INS_IsMemoryWrite(ins)) {
                UINT32 memoryOperands = INS_MemoryOperandCount(ins);
                if (memoryOperands == 1) {
                    UINT32 op1_size = INS_MemoryOperandSize(ins, 0);
                    INS_InsertFillBuffer(ins, IPOINT_BEFORE, bufId,
                            IARG_INST_PTR, offsetof(struct MemRef, ip),
                            IARG_UINT32, 1, offsetof(struct MemRef, opcount),
                            IARG_MEMORYOP_EA, 0, 
                                offsetof(struct MemRef, op1_addr),
                            IARG_BOOL, INS_MemoryOperandIsWritten(ins, 0),
                                offsetof(struct MemRef, op1_iswrite),
                            IARG_UINT32, op1_size,
                                offsetof(struct MemRef, op1_size),
                            IARG_ADDRINT, 0, 
                                offsetof(struct MemRef, op2_addr),
                            IARG_BOOL, false,
                                offsetof(struct MemRef, op2_iswrite),
                            IARG_UINT32, 0,
                                offsetof(struct MemRef, op2_size),
                            IARG_END);
               } else if (memoryOperands == 2) {
                    UINT32 op1_size = INS_MemoryOperandSize(ins, 0);
                    UINT32 op2_size = INS_MemoryOperandSize(ins, 1);
                    INS_InsertFillBuffer(ins, IPOINT_BEFORE, bufId,
                            IARG_INST_PTR, offsetof(struct MemRef, ip),
                            IARG_UINT32, 2, offsetof(struct MemRef, opcount),
                            IARG_MEMORYOP_EA, 0, 
                                offsetof(struct MemRef, op1_addr),
                            IARG_BOOL, INS_MemoryOperandIsWritten(ins, 0),
                                offsetof(struct MemRef, op1_iswrite),
                            IARG_UINT32, op1_size,
                                offsetof(struct MemRef, op1_size),
                            IARG_MEMORYOP_EA, 1, 
                                offsetof(struct MemRef, op2_addr),
                            IARG_BOOL, INS_MemoryOperandIsWritten(ins, 1),
                                offsetof(struct MemRef, op2_iswrite),
                            IARG_UINT32, op2_size,
                                offsetof(struct MemRef, op2_size),
                            IARG_END);
               } else {
                   cout << "Found instruction with " << memoryOperands;
                   cout << " memory operands" << endl;
                   assert(false);
               }
            }
        }
    }
}

VOID * BufferFull(BUFFER_ID id, 
        THREADID tid,
        const CONTEXT *ctxt,
        VOID *buf,
        UINT64 numElements, VOID *v) {
    struct MemRef * reference=(struct MemRef*)buf;
    MemLog * mlog = static_cast<MemLog*>( PIN_GetThreadData( mlog_key, tid ) );
    mlog->DumpBufferToFile( reference, numElements, tid );
    return buf;
}


VOID ThreadStart(THREADID tid,
        CONTEXT *ctxt,
        INT32 flags,
        VOID *v) {
    MemLog* mlog = new MemLog(tid);
    PIN_SetThreadData(mlog_key, mlog, tid);
}


VOID ThreadFini(THREADID tid,
        const CONTEXT *ctxt,
        INT32 code,
        VOID *v) {
    MemLog * mlog = static_cast<MemLog*>(PIN_GetThreadData(mlog_key, tid));
    delete mlog;
    PIN_SetThreadData(mlog_key, 0, tid);
}

INT32 Usage() {
    PIN_ERROR("This Pintool samples the IPs of instruction executed");
    return -1;
}

int main(int argc, char* argv[]) {
    if (PIN_Init(argc, argv)) {
        return Usage();
    }
    InitLock(&global_lock);
    
    bufId = PIN_DefineTraceBuffer(sizeof(struct MemRef), NUM_BUF_PAGES,
                                  BufferFull, 0);
    if (bufId == BUFFER_ID_INVALID) {
        cerr << "Error: could not allocate initial buffer" << endl;
        return 1;
    }
    mlog_key = PIN_CreateThreadDataKey(0);

    TRACE_AddInstrumentFunction(TraceMemory, 0);
    PIN_AddThreadStartFunction(ThreadStart, 0);
    PIN_AddThreadFiniFunction(ThreadFini, 0);
    
    PIN_StartProgram();
    return 0;
}

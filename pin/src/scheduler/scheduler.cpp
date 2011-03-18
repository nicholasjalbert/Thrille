/*
 * Author - Nick Jalbert (jalbert@cs.berkeley.edu)
 *
 */

#include "pin.H"
#include <iostream>

PIN_LOCK global_lock;

VOID Trace(THREADID thread,
        ADDRINT ip,
        ADDRINT addr,
        BOOL iswrite,
        BOOL isread,
        UINT32 readsize,
        UINT32 writesize) { 

}

VOID TraceMemory(TRACE trace, VOID* v) {
    for (BBL bbl = TRACE_BblHead(trace);
            BBL_Valid(bbl);
            bbl = BBL_Next(bbl)) {
        for (INS ins = BBL_InsHead(bbl);
                INS_Valid(ins);
                ins = INS_Next(ins)) {
            if (INS_IsMemoryRead(ins) || INS_IsMemoryWrite(ins)) {
                UINT32 memoryOperands = INS_MemoryOperandCount(ins);

                for (UINT32 memOp = 0; memOp < memoryOperands; memOp++) {
                    INS_InsertPredicatedCall(
                            ins, IPOINT_BEFORE, (AFUNPTR) Trace,
                            IARG_THREAD_ID,
                            IARG_INST_PTR,
                            IARG_MEMORYOP_EA, memOp,
                            IARG_BOOL, INS_MemoryOperandIsWritten(ins, memOp),
                            IARG_BOOL, INS_MemoryOperandIsRead(ins, memOp),
                            IARG_MEMORYREAD_SIZE,
                            IARG_MEMORYWRITE_SIZE,
                            IARG_END);
                }
            }
        }
    }
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
    TRACE_AddInstrumentFunction(TraceMemory, 0);
    PIN_StartProgram();
    return 0;
}

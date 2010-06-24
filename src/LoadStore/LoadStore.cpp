// LoadStore.cpp - Implements LLVM Compile Pass which inserts the functions:
//                      void myMemoryRead(int iid, void * addr)
//                      void myMemoryWrite(int iid, void * addr)
//                 before their respective memory operations.
//

#define DEBUG_TYPE "hello"
#include "llvm/LLVMContext.h"
#include "llvm/Pass.h"
#include "llvm/Function.h"
#include "llvm/User.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/IRBuilder.h"
#include "llvm/Instructions.h"
#include "llvm/Module.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Type.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IntrinsicInst.h"
#include "llvm/Analysis/DebugInfo.h"
#include <string>
#include <vector>

using std::vector;
using namespace llvm;

namespace {
    struct LoadStoreInstr : public BasicBlockPass {
        static char ID;
        static int iid;
        static int numloads;
        static int numstores;
        static int iidstart;
        static int iidend;
        static const int debug = 0; 

        LoadStoreInstr() : BasicBlockPass(&ID) {}

        virtual bool runOnBasicBlock(BasicBlock &BB) {
            Module* M = BB.getParent()->getParent();

            IRBuilder<> builder(&BB);

            std::vector<Value *> args;

            for (BasicBlock::iterator i = BB.begin(), e = BB.end();
                    i != e;
                    ++i) {
                // Found a store instruction
                if (StoreInst *SI = dyn_cast<StoreInst>(i)) {
                    if (debug) {
                        errs() << "found store\n" ;
                        std::string name = 
                            SI->getPointerOperand()->getNameStr();
                        errs() << "store on " << name << "\n";
                    }
                    iid++;
                    numstores++;
                    unsigned addrspace = 
                        (dyn_cast<PointerType> 
                         (SI->getOperand(1)->getType()))->getAddressSpace();
                    CastInst* CI = 
                        CastInst::CreatePointerCast(SI->getOperand(1), 
                                PointerType::get(Type::getInt64Ty(M->getContext()), addrspace), 
                                "", i);
                    vector<const Type *> params = vector<const Type *>();
                    params.push_back(Type::getInt32Ty(M->getContext()));
                    params.push_back(PointerType::get(Type::getInt64Ty(M->getContext()), 
                                        addrspace));
                    const FunctionType *FTy =
                        FunctionType::get(Type::getVoidTy(M->getContext()), 
                                params,
                                false);

                    Value * CallArgs[] = {
                      ConstantInt::get(Type::getInt32Ty(M->getContext()), iid), CI
                    };
                    Value * WriteFunction = M->getOrInsertFunction("_Z13myMemoryWriteiPv", FTy);
                    CallInst::Create(WriteFunction, CallArgs, CallArgs+2,"", i);

                // Found a load instruction
                } else if (LoadInst *LI = dyn_cast<LoadInst>(i)) {
                    if (debug) {
                        errs() << "found load\n" ;
                        std::string name = LI->getPointerOperand()->getNameStr();
                        errs() << "load on " << name << "\n";
                    }

                    iid++;
                    numloads++;
                    
                    unsigned addrspace = 
                        (dyn_cast<PointerType>(LI->getPointerOperand()->getType()))->getAddressSpace();

                    CastInst* CI = 
                        CastInst::CreatePointerCast(LI->getPointerOperand(), 
                                PointerType::get(Type::getInt64Ty(M->getContext()), addrspace), 
                                "", i);
                    vector<const Type *> params = vector<const Type *>();
                    params.push_back(Type::getInt32Ty(M->getContext()));
                    params.push_back(PointerType::get(Type::getInt64Ty(M->getContext()), 
                                        addrspace));
                    const FunctionType *FTy =
                        FunctionType::get(Type::getVoidTy(M->getContext()), 
                                params,
                                false);

                     Value * CallArgs[] = {
                      ConstantInt::get(Type::getInt32Ty(M->getContext()), iid), CI
                    };
                    Value * ReadFunction = M->getOrInsertFunction("_Z12myMemoryReadiPv", FTy);
                    CallInst::Create(ReadFunction, CallArgs, CallArgs+2,"", i);

                } 
            }
            return true;
        }

        virtual bool doInitialization(Module &M) {
            iidstart = iid + 1;
            return true;
        }
        
        virtual bool doFinalization(Module &M) {
            if (debug)
                errs() << "doFinalization called " << iid << "\n";
            iidend = iid;
            errs() << "Total Loads Instrumented: " << numloads << "\n";
            errs() << "Total Stores Instrumented: " << numstores << "\n";
            errs() << "IID Range: " << iidstart << " - " << iidend << "\n";
            FILE * f = fopen("iiddump","w");
            if (f != NULL)
                fprintf(f, "%d", iid);
            else {
                errs() << "cannot open file\n";
                return false;
            }
            
            return true;
        }

        virtual void getAnalysisUsage(AnalysisUsage &AU) const {
            AU.setPreservesAll();
        };
    };
}

int getIidFromFile() {
    int iid = 0;
    FILE * f = fopen("iiddump", "r"); 
    if (f == NULL)
        return 0;
    else {        
        if (fscanf(f,"%d", &iid) == 0)
            return 0;
        else
            return iid;
        fclose(f);
    }
}

char LoadStoreInstr::ID = 0;
int LoadStoreInstr::iidstart = 0;
int LoadStoreInstr::iidend = 0;
int LoadStoreInstr::numloads = 0;
int LoadStoreInstr::numstores = 0;
int LoadStoreInstr::iid = getIidFromFile();
static RegisterPass<LoadStoreInstr>
Z("loadstore", "Basic block pass");

#include "AArch64.h"
#include "AArch64Subtarget.h"
#include "AArch64RegisterInfo.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "AArch64MachineFunctionInfo.h"
#include "llvm/AOS/AOS.h"

#define DEBUG_TYPE "AArch64AOSPass"

STATISTIC(StatNumPACMA,
            DEBUG_TYPE "Number of PACMA intrinsics replaced");

STATISTIC(StatNumXPACM,
            DEBUG_TYPE "Number of XPACM intrinsics replaced");

STATISTIC(StatNumBNDSTR,
            DEBUG_TYPE "Number of BNDSTR intrinsics replaced");

STATISTIC(StatNumBNDCLR,
            DEBUG_TYPE "Number of BNDCLR intrinsics replaced");


using namespace llvm;

namespace {
 class AArch64AOSPass : public MachineFunctionPass {

 public:
   static char ID;

   AArch64AOSPass() : MachineFunctionPass(ID) {}

   StringRef getPassName() const override { return DEBUG_TYPE; }

   virtual bool doInitialization(Module &M) override;
   bool runOnMachineFunction(MachineFunction &) override;

 private:
   const AArch64Subtarget *STI = nullptr;
   const AArch64InstrInfo *TII = nullptr;
   inline bool handleInstruction(MachineFunction &MF, MachineBasicBlock &MBB,
                                      MachineBasicBlock::instr_iterator &MIi);
  };
}

char AArch64AOSPass::ID = 0;
FunctionPass *llvm::createAArch64AOSPass() { return new AArch64AOSPass(); }

bool AArch64AOSPass::doInitialization(Module &M) {
  return true;
}

bool AArch64AOSPass::runOnMachineFunction(MachineFunction &MF) {
  bool modified = false;
  STI = &MF.getSubtarget<AArch64Subtarget>();
  TII = STI->getInstrInfo();

  for (auto &MBB : MF) {
    for (auto MIi = MBB.instr_begin(), MIie = MBB.instr_end(); MIi != MIie;) {
      auto MIk = MIi++;

      switch (MIk->getOpcode()) {
        case AArch64::AOS_PACMA: {
          BuildMI(MBB, MIk, MIk->getDebugLoc(), TII->get(AArch64::PACMA), MIk->getOperand(0).getReg())
            .addReg(MIk->getOperand(1).getReg())
            .addReg(MIk->getOperand(2).getReg());

          MIk->removeFromParent();
          modified = true;
          ++StatNumPACMA;

          break;
        }
        case AArch64::AOS_XPACM: {
          BuildMI(MBB, MIk, MIk->getDebugLoc(), TII->get(AArch64::XPACM), MIk->getOperand(0).getReg())
            .addReg(MIk->getOperand(1).getReg());

          MIk->removeFromParent();
          modified = true;
          ++StatNumXPACM;

          break;
        }
        case AArch64::AOS_BNDSTR: {
          BuildMI(MBB, MIk, MIk->getDebugLoc(), TII->get(AArch64::BNDSTR))
            .addReg(MIk->getOperand(0).getReg())
            .addReg(MIk->getOperand(1).getReg());

          MIk->removeFromParent();
          modified = true;
          ++StatNumBNDSTR;

          break;
        }
        case AArch64::AOS_BNDCLR: {
          BuildMI(MBB, MIk, MIk->getDebugLoc(), TII->get(AArch64::BNDCLR))
            .addReg(MIk->getOperand(0).getReg());

          MIk->removeFromParent();
          modified = true;
          ++StatNumBNDCLR;

          break;
        }
        default:
          break;
      }
    }
  }

  return modified;
}

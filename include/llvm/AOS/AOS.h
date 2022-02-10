#ifndef LLVM_AOS_H
#define LLVM_AOS_H

#include "llvm/IR/Constant.h"
#include "llvm/IR/Type.h"
#include "llvm/Pass.h"

namespace llvm {
namespace AOS {
enum AOSInstType {
  AOSInstDisabled,
  AOSInstEnabled
};

AOSInstType getAOSInstType();

Pass *createAOSOptPass(); //yh+
}
} // llvm

#endif //LLVM_AOS_H

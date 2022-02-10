#include "llvm/AOS/AOS.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"
#include <regex>
#include <map>

using namespace llvm;
using namespace AOS;

static cl::opt<AOS::AOSInstType> AOSInst(
    "aos", cl::init(AOSInstDisabled),
    cl::desc("AOS protection"),
    cl::value_desc("mode"),
    cl::values(clEnumValN(AOSInstDisabled, "disable", "Disable AOS protection"),
               clEnumValN(AOSInstEnabled, "enable", "Enable AOS protection")));

AOSInstType AOS::getAOSInstType() {
  return AOSInst;
}


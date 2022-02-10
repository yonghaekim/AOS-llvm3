#include "llvm/AOS/AOSOptPass.h"

char AOSOptPass::ID = 0;
static RegisterPass<AOSOptPass> X("aos-opt", "AOS opt pass");

Pass *llvm::AOS::createAOSOptPass() { return new AOSOptPass(); }

bool AOSOptPass::runOnBasicBlock(BasicBlock &BB) {
  bool basicblock_modified = false;
  std::list<Instruction*> inst_list;

	// Iterate each basic block
	// Search for call or invoke insts for dynamic memory de-/allocation
  for (auto &I : BB) {
    CallInst *pCI = dyn_cast<CallInst>(&I);
    InvokeInst *pII = dyn_cast<InvokeInst>(&I);
		Function *pF = nullptr;

		if (pCI)
			pF = pCI->getCalledFunction();
		else if (pII)
			pF = pII->getCalledFunction();

		if (pF && (pF->getName() == "malloc" ||
								pF->getName() == "calloc" ||
								pF->getName() == "realloc" ||
								pF->getName() == "free" ||
								pF->getName() == "_Znwm" ||		// new
								pF->getName() == "_Znam" ||		// new[]
								pF->getName() == "_ZdlPv" ||	// delete
								pF->getName() == "_ZdaPv")) {	// delete[]
			inst_list.push_back(&I);
		}
	}

	// Handle each inst
  while (!inst_list.empty()) {
    Instruction *pI = inst_list.front();
    inst_list.pop_front();

		Function *pF = nullptr;
    CallInst *pCI = dyn_cast<CallInst>(pI);
    InvokeInst *pII = dyn_cast<InvokeInst>(pI);

		if (pCI)
			pF = pCI->getCalledFunction();
		else if (pII)
			pF = pII->getCalledFunction();

		assert(pF && "Found null function pointer");

		if (pF->getName() == "malloc" ||
				pF->getName() == "_Znwm"  ||
				pF->getName() == "_Znam") {
			basicblock_modified = handleMalloc(pI) || basicblock_modified;
		} else if (pF->getName() == "calloc") {
			basicblock_modified = handleCalloc(pI) || basicblock_modified;
		} else if (pF->getName() == "realloc") {
			basicblock_modified = handleRealloc(pI) || basicblock_modified;
		} else if (pF->getName() == "free"	||
							pF->getName() == "_ZdlPv"	||
							pF->getName() == "_ZdaPv") {
			basicblock_modified = handleFree(pI) || basicblock_modified;
		} else {
			assert(false && "Found wrong function name");
		}
	}

  return basicblock_modified;
}

bool AOSOptPass::handleMalloc(Instruction *pI) {
	Function *pF = pI->getFunction();
	std::vector<Type *> arg_type;
	arg_type.push_back(pI->getType());
	std::vector<Type *> arg_type2;
	auto arg0 = pI->getOperand(0);	// arg0 is alloc size
	auto next = pI->getNextNode();	

	if (InvokeInst *pII = dyn_cast<InvokeInst>(pI))
		next = &(pII->getNormalDest()->front());

	// Insert pacma / bndstr after malloc()
	IRBuilder<> Builder(next);
	auto pacma = Intrinsic::getDeclaration(pF->getParent(), Intrinsic::aos_pacma, arg_type);
	auto bndstr = Intrinsic::getDeclaration(pF->getParent(), Intrinsic::aos_bndstr, arg_type2);
	auto callA = Builder.CreateCall(pacma, {pI, arg0}, "");
	auto callB = Builder.CreateCall(bndstr, {callA, arg0}, "");
	pI->replaceAllUsesWith(callA);
	callA->setOperand(0, pI);

	return true;
}

bool AOSOptPass::handleCalloc(Instruction *pI) {
	Function *pF = pI->getFunction();
	std::vector<Type *> arg_type;
	arg_type.push_back(pI->getType());
	std::vector<Type *> arg_type2;
	auto arg0 = pI->getOperand(0);	// arg0 is element number
	auto arg1 = pI->getOperand(1);	// arg1 is element size

	// Calculate alloc size (arg0 * arg1) before calloc()
	IRBuilder<> BuilderA(pI);
	Value *res = BuilderA.CreateMul(arg0, arg1);

	// Insert pacma / bndstr after calloc()
	IRBuilder<> BuilderB(pI->getNextNode());
	auto pacma = Intrinsic::getDeclaration(pF->getParent(), Intrinsic::aos_pacma, arg_type);
	auto bndstr = Intrinsic::getDeclaration(pF->getParent(), Intrinsic::aos_bndstr, arg_type2);
	auto callA = BuilderB.CreateCall(pacma, {pI, res}, "");
	auto callB = BuilderB.CreateCall(bndstr, {callA, res}, "");
	pI->replaceAllUsesWith(callA);
	callA->setOperand(0, pI);

	return true;
}

bool AOSOptPass::handleRealloc(Instruction *pI) {
	Function *pF = pI->getFunction();
	std::vector<Type *> arg_type;
	arg_type.push_back(pI->getType());
	std::vector<Type *> arg_type2;
	auto arg0 = pI->getOperand(0);	// arg0 is ptr
	auto arg1 = pI->getOperand(1);	// arg1 is new size

	// TODO: Need to handle these cases
	// 1) ptr arg of calloc() can be NULL
	// 2) size arg can be zero

	// Insert bndclr / xpacm before realloc()
	IRBuilder<> BuilderA(pI);
	auto bndclr = Intrinsic::getDeclaration(pF->getParent(), Intrinsic::aos_bndclr, arg_type2);
	auto xpacm = Intrinsic::getDeclaration(pF->getParent(), Intrinsic::aos_xpacm, arg_type);
	auto callA = BuilderA.CreateCall(bndclr, {arg0}, "");
	auto callB = BuilderA.CreateCall(xpacm, {arg0}, "");
	pI->setOperand(0, callB);

	// Insert pacma / bndstr after realloc()
	IRBuilder<> BuilderB(pI->getNextNode());
	auto pacma = Intrinsic::getDeclaration(pF->getParent(), Intrinsic::aos_pacma, arg_type);
	auto bndstr = Intrinsic::getDeclaration(pF->getParent(), Intrinsic::aos_bndstr, arg_type2);
	auto callC = BuilderB.CreateCall(pacma, {pI, arg1}, "");
	auto callD = BuilderB.CreateCall(bndstr, {callC, arg1}, "");
	pI->replaceAllUsesWith(callC);
	callC->setOperand(0, pI);

	return true;
}

bool AOSOptPass::handleFree(Instruction *pI) {
	Function *pF = pI->getFunction();
	auto arg0 = pI->getOperand(0);	// arg0 is ptr to free
	std::vector<Type *> arg_type;
	arg_type.push_back(arg0->getType());
	std::vector<Type *> arg_type2;

	// Insert bndclr / xpacm before free()
	IRBuilder<> Builder(pI);
	auto bndclr = Intrinsic::getDeclaration(pF->getParent(), Intrinsic::aos_bndclr, arg_type2);
	auto xpacm = Intrinsic::getDeclaration(pF->getParent(), Intrinsic::aos_xpacm, arg_type);
	auto callA = Builder.CreateCall(bndclr, {arg0}, "");
	auto callB = Builder.CreateCall(xpacm, {arg0}, "");
	pI->setOperand(0, callB);

	return true;
}


def int_aos_pacma : Intrinsic<[llvm_anyptr_ty], [ llvm_ptr_ty, llvm_i64_ty ], [IntrNoMem]>;
def int_aos_xpacm : Intrinsic<[llvm_anyptr_ty], [ llvm_ptr_ty ], [IntrNoMem]>;
def int_aos_bndstr : Intrinsic<[], [ llvm_ptr_ty, llvm_i64_ty ]>;
def int_aos_bndclr : Intrinsic<[], [ llvm_ptr_ty ]>;

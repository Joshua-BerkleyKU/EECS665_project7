#include <ostream>
#include "3ac.hpp"

namespace cshanty{

void IRProgram::allocGlobals(){
	//Choose a label for each global
	int temp = 1;
	for (auto g: globals)
	{
		SymOpd * globalOpd = g.second;
		std::string memLoc = "glb_";
		const SemSymbol * sym = globalOpd->getSym();
		memLoc += sym->getName();
		globalOpd->setMemoryLoc("(" + memLoc + ")");
	}
	for (auto s: strings)
	{
		LitOpd * stringlOpd = s.first;
		std::string memLoc = "str_";
		std::string temp1 = std::to_string(temp);
		memLoc += temp1;
		stringlOpd->setMemoryLoc("(" + memLoc + ")");
		temp++;
	}
}

void IRProgram::datagenX64(std::ostream& out){
	out << ".globl main\n";
	out << ".data\n";
	
	for (auto g: globals)
	{
		SymOpd * globalOpd = g.second;
		std::string memLoc = "glb_";
		const SemSymbol * sym = globalOpd->getSym();
		memLoc += sym->getName();
		size_t width = sym->getDataType()->getSize();
		out << memLoc << ": ";
		if (width == 8)
		{
			out << ".quad 0 \n";
		}
		else
		{
			out << ".space " << width << "\n";
		}
		
	}
	int temp = 1;
	for (auto s: strings)
	{
		LitOpd * stringlOpd = s.first;
		std::string memLoc = "str_";
		std::string temp1 = std::to_string(temp);
		memLoc += temp1;
		out << memLoc << ": .asciz \"" + s.second + "\" \n";
		temp++;
	}
	//Put this directive after you write out strings
	// so that everything is aligned to a quadword value
	// again
	out << ".align 8\n";
	
}

void IRProgram::toX64(std::ostream& out){
	allocGlobals();
	datagenX64(out);
	out << ".text \n";
	//might need more stuff here
	// Iterate over each procedure and codegen it
	for (auto proc: *this->procs)
	{
		proc->toX64(out);
	}
}

void Procedure::allocLocals(){
	//Allocate space for locals
	// Iterate over each procedure and codegen it
	loc_offset = 24;
	for (auto t: temps)
	{
		std::string temp = std::to_string(loc_offset);
		t->setMemoryLoc("-" + temp + "(%rsp)");
		loc_offset = loc_offset + 8;
	}
	for (auto l: locals)
	{
		SymOpd * localsOpd = l.second;
		const SemSymbol * sym = localsOpd->getSym();
		std::string temp = std::to_string(loc_offset);
		localsOpd->setMemoryLoc("-" + temp + "(%rsp)");
		loc_offset = loc_offset + 8;
	}
	for (auto f: formals)
	{
		std::string temp = std::to_string(loc_offset);
		f->setMemoryLoc("-" + temp + "(%rsp)");
		loc_offset = loc_offset + 8;
	}
	for (auto a: addrOpds)
	{
		//may never happen
		TODO(Implement me)
	}
}

void Procedure::toX64(std::ostream& out){
	//Allocate all locals
	allocLocals();

	int offset_val = loc_offset - 16;
	enter->setoffset(offset_val);
	leave->setoffset(offset_val);

	enter->codegenLabels(out);
	enter->codegenX64(out);
	out << "#Fn body " << myName << "\n";
	for (auto quad : *bodyQuads){
		quad->codegenLabels(out);
		out << "#" << quad->toString() << "\n";
		quad->codegenX64(out);
	}
	out << "#Fn epilogue " << myName << "\n";
	leave->codegenLabels(out);
	leave->codegenX64(out);
}

void Quad::codegenLabels(std::ostream& out){
	if (labels.empty()){ return; }

	size_t numLabels = labels.size();
	size_t labelIdx = 0;
	for ( Label * label : labels){
		out << label->getName() << ": ";
		if (labelIdx != numLabels - 1){ out << "\n"; }
		labelIdx++;
	}
}

void BinOpQuad::codegenX64(std::ostream& out){

	if (opr == ADD64)
	{
		src1->genLoadVal(out, A);
		src2->genLoadVal(out, B);
		out << "      addq %rax, %rbx\n";
		dst->genStoreVal(out, B);
	}
	else if (opr == SUB64)
	{
		src1->genLoadVal(out, A);
		src2->genLoadVal(out, B);
		out << "      subq %rax, %rbx\n";
		dst->genStoreVal(out, B);
	}
	else if (opr == DIV64)
	{
		// todo unstable find a fix
		out << "      movq $0, %rax\n";
		src1->genLoadVal(out, B);
		src2->genLoadVal(out, C);
		out << "      idivq %rcx\n";
		dst->genStoreVal(out, A);
	}
	else if (opr == MULT64)
	{
		src1->genLoadVal(out, A);
		src2->genLoadVal(out, B);
		out << "      imulq %rbx\n";
		dst->genStoreVal(out, A);
	}
	else if (opr == EQ64)
	{
		src1->genLoadVal(out, A);
		src2->genLoadVal(out, B);
		out << "      cmpq %rax, %rbx\n";
		out << "      movq $0, %rcx\n";
		out << "      sete %cl\n";
		dst->genStoreVal(out, C);
	}
	else if (opr == NEQ64)
	{
		src1->genLoadVal(out, A);
		src2->genLoadVal(out, B);
		out << "      cmpq %rax, %rbx\n";
		out << "      movq $0, %rcx\n";
		out << "      setne %cl\n";
		dst->genStoreVal(out, C);
	}
	else if (opr == LT64)
	{
		src1->genLoadVal(out, A);
		src2->genLoadVal(out, B);
		out << "      cmpq %rax, %rbx\n";
		out << "      movq $0, %rcx\n";
		out << "      setl %cl\n";
		dst->genStoreVal(out, C);
	}
	else if (opr == GT64)
	{
		src1->genLoadVal(out, A);
		src2->genLoadVal(out, B);
		out << "      cmpq %rax, %rbx\n";
		out << "      movq $0, %rcx\n";
		out << "      setg %cl\n";
		dst->genStoreVal(out, C);
	}
	else if (opr == LTE64)
	{
		src1->genLoadVal(out, A);
		src2->genLoadVal(out, B);
		out << "      cmpq %rax, %rbx\n";
		out << "      movq $0, %rcx\n";
		out << "      setle %cl\n";
		dst->genStoreVal(out, C);
	}
	else if (opr == GTE64)
	{
		src1->genLoadVal(out, A);
		src2->genLoadVal(out, B);
		out << "      cmpq %rax, %rbx\n";
		out << "      movq $0, %rcx\n";
		out << "      setge %cl\n";
		dst->genStoreVal(out, C);
	}
	else if (opr == OR64)
	{
		src1->genLoadVal(out, A);
		src2->genLoadVal(out, B);
		out << "      orq %rax, %rbx\n";
		dst->genStoreVal(out, B);
	}
	else if (opr == AND64)
	{
		src1->genLoadVal(out, A);
		src2->genLoadVal(out, B);
		out << "      andq %rax, %rbx\n";
		dst->genStoreVal(out, B);
	}
}

void UnaryOpQuad::codegenX64(std::ostream& out){
	src->genLoadVal(out, A);

	if (op == NEG64)
	{
		out << "      negq %rax\n";
	}
	else if (op == NOT64)
	{
		out << "      notq %rax\n";
	}
	dst->genStoreVal(out, A);
}

void AssignQuad::codegenX64(std::ostream& out){
	src->genLoadVal(out, A);
	dst->genStoreVal(out, A);
}

void GotoQuad::codegenX64(std::ostream& out){
	out << "      jmp " << tgt->getName() << "\n";
}

void IfzQuad::codegenX64(std::ostream& out){
	out << "      " 
	<< cnd->getMovOp()
	<< " "
	<< cnd->getMemoryLoc()
	<< ", %rcx\n"
	<< "      cmpq $0, %rcx\n"
	<< "      je "
	<< tgt->getName()
	<< std::endl;
}

void NopQuad::codegenX64(std::ostream& out){
	out << "      nop" << "\n";
}

void IntrinsicOutputQuad::codegenX64(std::ostream& out){
	if (myType->isBool()){
		myArg->genLoadVal(out, DI);
		out << "      callq printBool\n";
	} else if (myType->isInt()) {
		myArg->genLoadVal(out, DI);
		out << "      callq printInt\n";
	} else {
		myArg->genLoadVal(out, DI);
		out << "      callq printString\n";
	}
}

void IntrinsicInputQuad::codegenX64(std::ostream& out){
	if (myType->isBool())
	{
		out << "      callq getBool\n";
		//need to get the return and then store it 
		myArg->genStoreVal(out, A);
	}
	else
	{
		out << "      callq getInt\n";
		//need to get the return and then store it
		myArg->genStoreVal(out, A);
	}
	
}

void CallQuad::codegenX64(std::ostream& out){
	out << "      callq " << callee->getName() << std::endl;
}

void EnterQuad::codegenX64(std::ostream& out){
	// need to find a way to get all allocated space on the stack
	//todo change 0 to real val
	std::string offset = std::to_string(total_offset);
	out << "      pushq %rbp\n";
	out << "      movq %rsp, %rbp\n";
	out << "      addq $16, %rbp\n";
	out << "      subq $" + offset + ", %rsp\n";
}

void LeaveQuad::codegenX64(std::ostream& out){
	// need to find a way to get all allocated space on the stack
	//todo change 0 to real val
	std::string offset = std::to_string(total_offset);
	out << "      addq $" + offset + ", %rsp\n";
	out << "      popq %rbp\n";
	out << "      retq\n";
}

void SetArgQuad::codegenX64(std::ostream& out){
	TODO(Implement me)
}

void GetArgQuad::codegenX64(std::ostream& out){
	//We don't actually need to do anything here
}

void SetRetQuad::codegenX64(std::ostream& out){
	if (myIsRecord == false)
	{
		out << "     movq " + opd->getMemoryLoc() + ", %rsi \n";
	}
	else
	{
		//the return of a record 
		TODO(Implement me)
	}
	
}

void GetRetQuad::codegenX64(std::ostream& out){
	if (myIsRecord == false)
	{
		out << "     movq %rsi, " + opd->getMemoryLoc() + " \n";
	}
	else
	{
		//the setting of a returned record 
		TODO(Implement me)
	}
}

void IndexQuad::codegenX64(std::ostream& out){
	TODO(Implement me)
}

void SymOpd::genLoadVal(std::ostream& out, Register reg){
	if (this->getMovOp() == "movq")
	{
		out << "      movq " << this->getMemoryLoc() << ", " << RegUtils::reg64(reg) << "\n";
	}
	else
	{
		out << "      movb " << this->getMemoryLoc() << ", " << RegUtils::reg8(reg) << "\n";
	}
}

void SymOpd::genStoreVal(std::ostream& out, Register reg){
	if (this->getMovOp() == "movq")
	{
		out << "      movq " << RegUtils::reg64(reg) << ", " << this->getMemoryLoc() << "\n";
	}
	else
	{
		out << "      movb " << RegUtils::reg8(reg) << ", " << this->getMemoryLoc() << "\n";
	}
}

void SymOpd::genLoadAddr(std::ostream& out, Register reg) {
	TODO(Implement me if necessary)
}

void AuxOpd::genLoadVal(std::ostream& out, Register reg){
	if (this->getMovOp() == "movq")
	{
		out << "      movq " << this->getMemoryLoc() << ", " << RegUtils::reg64(reg) << "\n";
	}
	else
	{
		out << "      movb " << this->getMemoryLoc() << ", " << RegUtils::reg8(reg) << "\n";
	}
}

void AuxOpd::genStoreVal(std::ostream& out, Register reg){
	if (this->getMovOp() == "movq")
	{
		out << "      movq " << RegUtils::reg64(reg) << ", " << this->getMemoryLoc() << "\n";
	}
	else
	{
		out << "      movb " << RegUtils::reg8(reg) << ", " << this->getMemoryLoc() << "\n";
	}
}
void AuxOpd::genLoadAddr(std::ostream& out, Register reg){
	TODO(Implement me)
}


void AddrOpd::genStoreVal(std::ostream& out, Register reg){
	if (this->getMovOp() == "movq")
	{
		out << "      movq " << RegUtils::reg64(reg) << ", " << this->getMemoryLoc() << "\n";
	}
	else
	{
		out << "      movb " << RegUtils::reg8(reg) << ", " << this->getMemoryLoc() << "\n";
	}
}

void AddrOpd::genLoadVal(std::ostream& out, Register reg){
	if (this->getMovOp() == "movq")
	{
		out << "      movq " << this->getMemoryLoc() << ", " << RegUtils::reg64(reg) << "\n";
	}
	else
	{
		out << "      movb " << this->getMemoryLoc() << ", " << RegUtils::reg8(reg) << "\n";
	}
}

void AddrOpd::genStoreAddr(std::ostream& out, Register reg){
	TODO(Implement me)
}

void AddrOpd::genLoadAddr(std::ostream & out, Register reg){
	TODO(Implement me)
}

void LitOpd::genLoadVal(std::ostream & out, Register reg){
	out << getMovOp() << " $" << val << ", " << getReg(reg) << "\n";
}

}

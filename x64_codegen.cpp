#include <ostream>
#include "3ac.hpp"

namespace cshanty{

void IRProgram::allocGlobals(){
	//Choose a label for each global
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
		TODO(Implement me)
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
		std::string memLoc = "loc_";
		memLoc += t->getName();
		std::string temp = std::to_string(loc_offset);
		t->setMemoryLoc("-" + temp + "%rsp(" + memLoc + ")");
		loc_offset = loc_offset + 8;
	}
	for (auto l: locals)
	{
		SymOpd * localsOpd = l.second;
		std::string memLoc = "loc_";
		const SemSymbol * sym = localsOpd->getSym();
		memLoc += sym->getName();
		std::string temp = std::to_string(loc_offset);
		localsOpd->setMemoryLoc("-" + temp + "%rsp(" + memLoc + ")");
		loc_offset = loc_offset + 8;
	}
	for (auto f: formals)
	{
		std::string memLoc = "loc_";
		std::string temp = std::to_string(loc_offset);
		memLoc += f->getName();
		f->setMemoryLoc("-" + temp + "%rsp(" + memLoc + ")");
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
	TODO(Implement me)
}

void UnaryOpQuad::codegenX64(std::ostream& out){
	TODO(Implement me)
}

void AssignQuad::codegenX64(std::ostream& out){
	src->genLoadVal(out, A);
	dst->genStoreVal(out, A);
}

void GotoQuad::codegenX64(std::ostream& out){
	out << "     jmp " << tgt->getName() << "\n";
}

void IfzQuad::codegenX64(std::ostream& out){
	TODO(Implement me)
}

void NopQuad::codegenX64(std::ostream& out){
	out << "     nop" << "\n";
}

void IntrinsicOutputQuad::codegenX64(std::ostream& out){
	if (myType->isBool()){
		myArg->genLoadVal(out, DI);
		out << "     callq printBool\n";
	} else {
		myArg->genLoadVal(out, DI);
		out << "     callq printInt\n";
	}
}

void IntrinsicInputQuad::codegenX64(std::ostream& out){
	TODO(Implement me)
}

void CallQuad::codegenX64(std::ostream& out){
	TODO(Implement me)
}

void EnterQuad::codegenX64(std::ostream& out){
	// need to find a way to get all allocated space on the stack
	out << "     pushq %rbp\n";
	out << "     movq %rsp, %rbp\n";
	out << "     addq %16, %rbp\n";
	out << "     subq %" +  + ", %rsp\n";
}

void LeaveQuad::codegenX64(std::ostream& out){
	// need to find a way to get all allocated space on the stack
	out << "     addq %" +  + ", %rsp\n";
	out << "     popq %rbp\n";
	out << "     retq\n";
}

void SetArgQuad::codegenX64(std::ostream& out){
	TODO(Implement me)
}

void GetArgQuad::codegenX64(std::ostream& out){
	//We don't actually need to do anything here
}

void SetRetQuad::codegenX64(std::ostream& out){
	TODO(Implement me)
}

void GetRetQuad::codegenX64(std::ostream& out){
	TODO(Implement me)
}

void IndexQuad::codegenX64(std::ostream& out){
	TODO(Implement me)
}

void SymOpd::genLoadVal(std::ostream& out, Register reg){
	// need to do more
	// need to worry about how long the stuff is 
	out << "     movq " << this->getMemoryLoc() << ", " << RegUtils::reg64(reg) << "\n";
	//TODO(Implement me)
}

void SymOpd::genStoreVal(std::ostream& out, Register reg){
	TODO(Implement me)
}

void SymOpd::genLoadAddr(std::ostream& out, Register reg) {
	TODO(Implement me if necessary)
}

void AuxOpd::genLoadVal(std::ostream& out, Register reg){
	TODO(Implement me)
}

void AuxOpd::genStoreVal(std::ostream& out, Register reg){
	TODO(Implement me)
}
void AuxOpd::genLoadAddr(std::ostream& out, Register reg){
	TODO(Implement me)
}


void AddrOpd::genStoreVal(std::ostream& out, Register reg){
	TODO(Implement me)
}

void AddrOpd::genLoadVal(std::ostream& out, Register reg){
	TODO(Implement me)
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

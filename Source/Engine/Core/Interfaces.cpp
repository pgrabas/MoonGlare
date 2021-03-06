#include <pch.h>
#include <nfMoonGlare.h>
#include <Engine/DataClasses/iFont.h>
#include "Core/Interfaces.h"
#include "Core/Component/ComponentRegister.h"

                      namespace MoonGlare::Core {

void Interfaces::DumpLists(ostream &out) {
	auto regdump = [&out](const Space::TemplateClassListBase::ClassInfoBase &type) {
		char buffer[256];
		sprintf(buffer, "%30s  [class %s]\n", type.Alias.c_str(), (type.TypeInfo ? type.TypeInfo->GetName() : "?"));
		out << buffer;
	};
	out << "\nFont class list\n";
	DataClasses::Fonts::FontClassRegister::GetRegister()->Enumerate(regdump);
	out << "\n"; 
	Component::ComponentRegister::Dump(out);
	out << "\n";
	auto func = [&out](const Space::ClassListRegisterBase<> *r) {
		out << r->GetCompilerName() << "\n";
	};
	Space::ClassListRegisterBase<>::EnumerateRegisters(func);
}

} 

/*
  * Generated by cppsrc.sh
  * On 2015-05-19 17:27:02,94
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <nfMoonGlare.h>
#include "ModuleManager.h"

#include "iModule.h"
#include <Core/Engine.h>

namespace MoonGlare::Modules {

using ModuleInfoList = ModulesManager::ModuleInfoList;

static ModuleInfoList *_ModuleList = 0;

static ModuleInfoList* ModuleList() {
    if (!_ModuleList) {
        _ModuleList = new ModuleInfoList();
        _ModuleList->reserve(16);
    }
    return _ModuleList;
}

//----------------------------------------------------------------

ModuleInfo::ModuleInfo(const char *Name):
        m_Name(Name) {
    ModuleList()->push_back(this);
}
ModuleInfo::~ModuleInfo() { }
bool ModuleInfo::Initialize() { return true; }
bool ModuleInfo::Finalize() { return true; }
const ModuleDescription* ModuleInfo::GetDescription() const { return nullptr; }
void ModuleInfo::RegisterModuleApi(ApiInitializer &api) { /* ignore */ }
void ModuleInfo::RegisterInternalApi(ApiInitializer &api) { /* ignore */ }
bool ModuleInfo::LoadSettings(const pugi::xml_node node) { return false; }
bool ModuleInfo::SaveSettings(pugi::xml_node node) const { return false; }

//----------------------------------------------------------------

ModulesManager *ModulesManager::s_instance = nullptr;


ModulesManager::ModulesManager(World *world) : world(world) {
    s_instance = this;

    ModuleClassRegister::GetRegister()->Enumerate([this](auto &item) {
        auto mod = item.SharedCreate(this->world);
        DebugLog(Hint, fmt::format("Created module of class '{}'", typeid(*mod).name()));
        moduleList.emplace_back(std::move(mod));
    });
}

ModulesManager::~ModulesManager() {
}

//----------------------------------------------------------------

bool ModulesManager::Initialize() {
    auto list = ModuleList();
    for (auto it = list->rbegin(), jt = list->rend(); it != jt; ++it) {
        auto *module = *it;
        AddLogf(Debug, "Initializing module %s", module->GetName());
        if (!module->Initialize()) {
            AddLogf(Warning, "Unable to initialize module %s", module->GetName());
        }
    }
    return true;
}

bool ModulesManager::Finalize() {
    auto list = GetModuleList();
    for (auto it = list->begin(), jt = list->end(); it != jt; ++it) {
        auto *module = *it;
        AddLogf(Debug, "Finalizing module %s", module->GetName());
        if (!module->Finalize()) {
            AddLogf(Warning, "Unable to Finalize module %s", module->GetName());
        }
    }
    return true;
}

//----------------------------------------------------------------

void ModulesManager::LoadSettings(const pugi::xml_node node) {
    if (!node)
        return;

    auto list = GetModuleList();
    for (auto &it : *list) {
        auto modnode = node.child(it->GetName());
        if (!modnode)
            continue;
        it->LoadSettings(modnode);
    }

    for (auto &it : moduleList) {
        auto mname = it->GetName();
        if (!mname.empty()) {
            auto modnode = node.child(mname.c_str());
            if(modnode)
                it->LoadSettings(modnode);
        }
    }
}

void ModulesManager::SaveSettings(pugi::xml_node node) const {
    if (!node)
        return;

    auto list = GetModuleList();
    for (auto &it : *list) {
        auto modnode = node.append_child(it->GetName());
        if (!it->SaveSettings(modnode))
            node.remove_child(modnode);
    }

    for (auto &it : moduleList) {
        auto mname = it->GetName();
        if (!mname.empty()) {
            auto modnode = node.append_child(mname.c_str());
            it->SaveSettings(modnode);
        }
    }
}

void ModulesManager::OnPostInit() {
    for (auto &it : *GetModuleList()) {
        auto *module = it;
        module->OnPostInit();
    }

    for (auto &it : moduleList) {
        it->OnPostInit();
    }
}

//----------------------------------------------------------------

const ModuleInfoList * ModulesManager::GetModuleList() const {
    return _ModuleList;
}

#ifdef DEBUG_DUMP
void ModulesManager::DumpModuleList(std::ostream &out) {
    const char *pattern = "%30s %20s";
    char linebuf[1024];
    out << "Modules:\n";
    sprintf(linebuf, pattern, "Module name", "Type");
    out << linebuf << "\n";

    for (auto* module : *GetModuleList()) {
        sprintf(linebuf, pattern, module->GetName(), "");
        out << linebuf << "\n";
    }
    out << "\n";
}
#endif

} //namespace MoonGlare::Modules

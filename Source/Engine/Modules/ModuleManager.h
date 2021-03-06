#pragma once

#include "Modules/iModuleManager.h"

namespace MoonGlare {
namespace Modules {

#define DEFINE_MODULE(CLASS) namespace detail { static CLASS CLASS##Module; }

class iModule;

struct ModuleDescription {
	const wchar_t *Author;
	const wchar_t *TextInfo;
};

struct ModuleInfo {
public:
	ModuleInfo(const ModuleInfo&) = delete;
	ModuleInfo& operator=(const ModuleInfo&) = delete;
	ModuleInfo(const char *Name);
	virtual ~ModuleInfo();

	virtual bool Initialize();
	virtual bool Finalize();

	virtual const ModuleDescription* GetDescription() const;

	virtual void RegisterModuleApi(ApiInitializer &api);
	virtual void RegisterInternalApi(ApiInitializer &api);

	virtual bool LoadSettings(const pugi::xml_node node);
	virtual bool SaveSettings(pugi::xml_node node) const;

    virtual void OnPostInit() {}

    auto GetName() { return m_Name; }
protected:           
	using BaseClass = ModuleInfo;
private:
	const char *m_Name;
};

class ModulesManager : public iModuleManager {
public:
    static ModulesManager *s_instance;
    static void DeleteInstance() { delete s_instance; s_instance = nullptr; }

 	ModulesManager(World *world);
 	virtual ~ModulesManager();

	void LoadSettings(const pugi::xml_node node) override;
	void SaveSettings(pugi::xml_node node) const override;

	using ModuleInfoList = std::vector < ModuleInfo* >;

	bool Initialize();
	bool Finalize();

    void OnPostInit();

	void DumpModuleList(std::ostream &out);

	const ModuleInfoList* GetModuleList() const;
private:
    std::vector<std::shared_ptr<iModule>> moduleList;
    World *world = nullptr;
};

} //namespace Modules 

inline Modules::ModulesManager* GetModulesManager() { return Modules::ModulesManager::s_instance; }

} //namespace MoonGlare 

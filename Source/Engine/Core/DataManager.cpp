#include <pch.h>
#include <MoonGlare.h>
#include <Engine/iSoundEngine.h>
#include <Engine/Modules/ModuleManager.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/DataManager.h>
#include <Engine/Core/Configuration.Runtime.h>
#include <Engine/Core/Scene/ScenesManager.h>
#include <Engine/DataClasses/iFont.h>
#include <Engine/Core/Scripts/iLuaRequire.h>

#include <StarVFS/core/nStarVFS.h>

#define xmlstr_Module_xml			"Module.xml"

namespace MoonGlare {
namespace Core {
namespace Data {

SPACERTTI_IMPLEMENT_CLASS_SINGLETON(Manager)
RegisterApiInstance(Manager, &Manager::Instance, "Data");
RegisterApiDerivedClass(Manager, &Manager::RegisterScriptApi);

Manager::Manager(World *world) : cRootClass(), world(world) {
    ASSERT(world);
    SetThisAsInstance();

    m_ScriptEngine = nullptr;

    OrbitLogger::LogCollector::SetChannelName(OrbitLogger::LogChannels::Resources, "RES");

    m_Modules.reserve(16);
    m_StringTables = std::make_unique<DataClasses::StringTable>(GetFileSystem());

    //TODO: DataManager is not threadsafe
}

Manager::~Manager() {
    Finalize();
    m_StringTables.reset();
}

//-------------------------------------------------------------------------------------------------

class DataManagerDebugScritpApi {
public:
#ifdef DEBUG_SCRIPTAPI
    static void ClearStringTables() {
        GetDataMgr()->m_StringTables->Clear();
    }
    static void ClearResources(Manager *mgr) {
        AddLogf(Warning, "Clearing resources");
        mgr->m_Fonts.Lock()->clear();
        mgr->m_StringTables->Clear();
    }
#endif
};

void Manager::RegisterScriptApi(::ApiInitializer &api) {
    api
    .deriveClass<ThisClass, BaseClass>("cDataManager")
        .addFunction("GetString", &ThisClass::GetString)
#ifdef DEBUG_SCRIPTAPI
        .addFunction("NotifyResourcesChanged", &ThisClass::NotifyResourcesChanged)
        .addFunction("ClearStringTables", Utils::Template::InstancedStaticCall<ThisClass, void>::get<&DataManagerDebugScritpApi::ClearStringTables>())
        .addFunction("ClearResources", Utils::Template::InstancedStaticCall<ThisClass, void>::callee<&DataManagerDebugScritpApi::ClearResources>())
#endif
        .endClass()
        .beginClass<RuntimeConfiguration>("cRuntimeConfiguration")
            .addData("scene", &RuntimeConfiguration::scene)
            .addData("consoleFont", &RuntimeConfiguration::consoleFont)
        .endClass()                                
    ;
}

//------------------------------------------------------------------------------------------

bool Manager::Initialize(const std::vector<std::string> &modules, std::string langCode, Scripts::ScriptEngine *ScriptEngine) {
    m_ScriptEngine = ScriptEngine;
    MoonGlareAssert(m_ScriptEngine);

    m_StringTables->SetLangCode(std::move(langCode));
    
    //GetFileSystem()->RegisterInternalContainer(&InternalFS::RootNode, FileSystem::InternalContainerImportPriority::Primary);
    //GetFileSystem()->LoadRegisteredContainers();
                                                         
    for (auto &it : modules) {
        AddLogf(Debug, "Trying to load module '%s'", it.c_str());
        
        if (!GetFileSystem()->LoadContainer(it)) {
            AddLogf(Error, "Unable to open module: '%s'", it.c_str());
        }
    }

#ifdef DEBUG_RESOURCEDUMP
    {
        std::ofstream file("logs/vfs.txt");
        GetFileSystem()->DumpStructure(file);
    }
#endif

    return true;
}

bool Manager::Finalize() {
    m_Fonts.clear();
    m_Modules.clear();

    return true;
}

//------------------------------------------------------------------------------------------

bool Manager::LoadModule(StarVFS::Containers::iContainer *Container) {
    MoonGlareAssert(Container);
    MoonGlareAssert(m_ScriptEngine);

    m_Modules.emplace_back();
    auto &mod = m_Modules.back();

    mod.m_Container = Container;
    mod.m_ModuleName = "?";

    auto cfid = Container->FindFile("/" xmlstr_Module_xml);
    StarVFS::ByteTable data;
    if (!Container->GetFileData(cfid, data)) {
        AddLogf(Error, "Failed to read module meta-data from container '%s'", Container->GetContainerURI().c_str());
        return false;
    }

    pugi::xml_document doc;
    if (!doc.load_string((char*)data.get())) {
        AddLogf(Error, "Failed to parse container meta-data xml '%s'", Container->GetContainerURI().c_str());
        return false;
    }

    auto rootnode = doc.document_element();
    mod.m_ModuleName = rootnode.child("ModuleName").text().as_string("?");

    AddLogf(Hint, "Loaded module '%s' from container '%s'", mod.m_ModuleName.c_str(), Container->GetContainerURI().c_str());

    if (!LoadModuleScripts(Container)) {
        AddLogf(Error, "Failed to load module scripts!");
        return false;
    }

    return true;
}

bool Manager::LoadModuleScripts(StarVFS::Containers::iContainer *Container) {
    MoonGlareAssert(Container);
    MoonGlareAssert(m_ScriptEngine);

    struct RTCfg : public Scripts::iRequireRequest {
        RTCfg(RuntimeConfiguration*r):currconf(*r), rtconf(r){}
        RuntimeConfiguration currconf;
        RuntimeConfiguration *rtconf;
        bool requested = false;
        bool OnRequire(lua_State *lua, std::string_view name) override {
            luabridge::push(lua, &currconf);
            requested = true;
            return true;
        }
        ~RTCfg() {
            if (requested)
                *rtconf = currconf;
        }
    };

    auto cfid = Container->FindFile("/init.lua");
    StarVFS::ByteTable data;
    if (!Container->GetFileData(cfid, data)) {
        //FIXME: read error / file does exists -> is it detectable
        AddLogf(Error, "Failed to read init script! (cid:%d;cfid:%d)", (int)Container->GetContainerID(), (int)cfid);
        return true;
    }
    else {
        RTCfg require(world->GetRuntimeConfiguration());
        auto rmod = m_ScriptEngine->QuerryModule<Scripts::iRequireModule>();
        rmod->RegisterRequire("RuntimeConfiguration", &require);
        
        bool result = false;
        try {
            std::string furi = fmt::format("cfid://{}/{}", (int)Container->GetContainerID(), "init.lua");
            m_ScriptEngine->ExecuteCode((char*)data.get(), data.byte_size(), furi.c_str());
            result = true;
        }
        catch (const std::exception &e) {
            AddLogf(Error, "Init script execution error (cid:%d;cfid:%d) : %s", (int)Container->GetContainerID(), (int)cfid, e.what());
        }
        catch (...) {
            AddLogf(Error, "Init script execution error (cid:%d;cfid:%d) : unknown error", (int)Container->GetContainerID(), (int)cfid);
        }

        rmod->RegisterRequire("RuntimeConfiguration", nullptr);
        return result;
    }
}

//------------------------------------------------------------------------------------------

void Manager::LoadGlobalData() {
    GetSoundEngine()->ScanForSounds();

    NotifyResourcesChanged();
}

//------------------------------------------------------------------------------------------

DataClasses::FontPtr Manager::GetConsoleFont() {
    return GetFont(world->GetRuntimeConfiguration()->consoleFont);
}

DataClasses::FontPtr Manager::GetDefaultFont() { 
    return GetFont(DEFAULT_FONT_NAME); 
}

//------------------------------------------------------------------------------------------

DataClasses::FontPtr Manager::GetFont(const string &Name) {
    auto fonts = m_Fonts.Lock();

    auto it = fonts->find(Name);
    FontResPtr *ptr = nullptr;
    if (it == fonts->end()) {
        ptr = &fonts[Name];
        ptr->SetValid(false);
    } else {
        ptr = &it->second;
        if (ptr->IsLoaded())
            return ptr->Get();

        if (!ptr->IsValid()) {
            AddLogf(Error, "Font '%s' is invalid.", Name.c_str());
            return nullptr;
        }
    }
    AddLogf(Debug, "Font '%s' not found. Trying to load.", Name.c_str());

//	AddLog(TODO, "Move ownership of loaded font xml to map instance");
    FileSystem::XMLFile xml;
    if (!GetFileSystem()->OpenResourceXML(xml, Name, DataPath::Fonts)) {
        AddLogf(Error, "Unable to open master resource xml for font '%s'", Name.c_str());
        fonts.unlock();
        return GetDefaultFont();
    }
    string Class = xml->document_element().attribute(xmlAttr_Class).as_string(ERROR_STR);
    auto font = DataClasses::Fonts::FontClassRegister::CreateShared(Class, Name);
    if (!font) {
        AddLogf(Error, "Unable to create font class '%s' for object '%s'", Class.c_str(), Name.c_str());
        fonts.unlock();
        return GetDefaultFont();
    }
    AddLogf(Debug, "Loading font '%s' of class '%s'", Name.c_str(), Class.c_str());
    if (!font->Initialize()) {
        AddLogf(Error, "Unable to initialize font '%s'", Name.c_str());
        fonts.unlock();
        return GetDefaultFont();
    }
    ptr->Set(font);
    ptr->SetValid(true);
    NotifyResourcesChanged();
    return font;
}

const string& Manager::GetString(const string &Id, const string& TableName) {
    return m_StringTables->GetString(Id, TableName);
}

//------------------------------------------------------------------------------------------

void Manager::DumpResources() {
#ifdef DEBUG_DUMP
    static std::mutex mutex;
    static unsigned RevisionIndex = 1;
    LOCK_MUTEX(mutex);
    static std::ofstream file (DEBUG_LOG_FOLDER "resources.txt");
    file << "Revision index: " << RevisionIndex << "\n\n";
    ++RevisionIndex;
    
    auto sm = world->GetScenesManager();
    if(sm)
        sm->DumpAllDescriptors(file);

    GetSoundEngine()->DumpContent(file);
    GetModulesManager()->DumpModuleList(file);

    file << "\n\n--------------------------------------------------------------------------------------\n" 
         << std::flush;
#endif
}

#ifdef DEBUG_DUMP
void Manager::NotifyResourcesChanged() {
    GetEngine()->PushSynchronizedAction([this]{ DumpResources(); });
} 
#endif

} // namespace Data
} // namespace Core
} // namespace MoonGlare

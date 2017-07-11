/*
  * Generated by cppsrc.sh
  * On 2015-06-29 10:15:07.37
  * by Immethis
*/
/*--END OF HEADER BLOCK--*/

#include <pch.h>
#include <boost/filesystem.hpp>

#include <MoonGlare.h>

#include <Engine/Core/DataManager.h>
#include <Engine/Modules/ModuleManager.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/InputProcessor.h>
#include <Engine/World.h>

#include <Assets/AssetManager.h>
#include <Renderer/Renderer.h>
#include <Renderer/Context.h>
#include <Renderer/ScriptApi.h>

#include "Modules/BasicConsole.h"

#include <iApplication.h>

#include <AssetSettings.x2c.h>
#include <RendererSettings.x2c.h>
#include <EngineSettings.x2c.h>

namespace MoonGlare {
namespace Application {

SPACERTTI_IMPLEMENT_CLASS_SINGLETON(iApplication);

iApplication::iApplication() : BaseClass() {
    m_Flags.m_UintValue = 0;

    m_Configuration = std::make_unique<x2c::Settings::EngineSettings_t>();
    m_ConfigurationFileName = "MoonGlare.Settings.xml";

    SetThisAsInstance();
}

iApplication::~iApplication() {}

//---------------------------------------------------------------------------------------

bool iApplication::PreSystemInit() {
    return true;
}

bool iApplication::PostSystemInit() {
    GetModulesManager()->OnPostInit();
    Core::GetEngine()->PostSystemInit();
    m_Renderer->GetContext()->HookMouse();
    return true;
}

//---------------------------------------------------------------------------------------

void iApplication::LoadSettings() {
    if (m_Flags.m_Initialized) {
        throw "Cannot load settings after initialization!";
    }
    m_Configuration->ResetToDefault();

    if (!boost::filesystem::is_regular_file(m_ConfigurationFileName.data())) {
        m_Flags.m_SettingsLoaded = true;
        m_Flags.m_SettingsChanged = true;
        AddLogf(Warning, "Settings not loaded. File is not valid");
        return;
    }

    pugi::xml_document doc;
    doc.load_file(m_ConfigurationFileName.data());
    auto root = doc.child("EngineSettings");
    m_Configuration->Read(root);
    GetModulesManager()->LoadSettings(root.child("Modules"));

    m_Flags.m_SettingsLoaded = true;
    m_Flags.m_SettingsChanged = false;
}

void iApplication::SaveSettings() {
    if (!m_Flags.m_SettingsLoaded) {
        AddLogf(Error, "Settings not loaded!");
        return;
    }
    if (!m_Flags.m_SettingsChanged) {
        AddLogf(Warning, "Settings not saved. There is no changes");
        return;
    }

    pugi::xml_document doc;
    auto root = doc.append_child("EngineSettings");
    m_Configuration->Write(root);
    GetModulesManager()->SaveSettings(root.append_child("Modules"));
    doc.save_file(m_ConfigurationFileName.data());

    m_Flags.m_SettingsChanged = false;
}

//---------------------------------------------------------------------------------------

using Modules::ModulesManager;
using FileSystem::MoonGlareFileSystem;
using MoonGlare::Core::Scripts::ScriptEngine;
using DataManager = MoonGlare::Core::Data::Manager;

void iApplication::Initialize() {
    m_World = std::make_unique<World>();
    auto ModManager = new ModulesManager(m_World.get());

    LoadSettings();

    if (!PreSystemInit()) {
        AddLogf(Error, "Pre system init action failed!");
        throw "Pre system init action failed";
    }
#define _init_chk(WHAT, ERRSTR, ...) \
do { if(!(WHAT)->Initialize()) { AddLogf(Error, ERRSTR, __VA_ARGS__); throw ERRSTR; } } while(false)

    _init_chk(new MoonGlareFileSystem(), "Unable to initialize internal filesystem!");

    m_AssetManager = std::make_unique<Asset::AssetManager>();
    if (!m_AssetManager->Initialize(&m_Configuration->m_Assets)) {
        AddLogf(Error, "Unable to initialize asset manager!");
        throw "Unable to initialize asset manager!";
    }

    if (!ModManager->Initialize()) {
        AddLogf(Error, "Unable to initialize modules manager!");
        throw "Unable to initialize modules manager";
    }

    auto scrEngine = new ScriptEngine(m_World.get());
    m_World->SetScriptEngine(scrEngine);
    _init_chk(scrEngine, "Unable to initialize script engine!");
    
    m_Renderer = std::make_unique<Renderer::RendererFacade>();
    m_World->SetRendererFacade(m_Renderer.get());

    m_Renderer->GetScriptApi()->Install(scrEngine->GetLua());
    m_Renderer->SetConfiguration(&m_Configuration->m_Renderer);
    m_Configuration->m_Display.visible = false;
    if (!m_Renderer->Initialize(m_Configuration->m_Display, m_AssetManager.get())) {
        AddLogf(Error, "Unable to initialize renderer");
        throw "Unable to initialize renderer";
    }

    auto datamgr = new DataManager(m_World.get());
    datamgr->SetLangCode(m_Configuration->m_Core.m_LangCode);

    for (auto &it : m_Configuration->m_Assets.m_ModuleList) {
        AddLogf(Debug, "Trying to load container '%s'", it.c_str());
        if (!GetFileSystem()->LoadContainer(it)) {
            AddLogf(Error, "Unable to open container: '%s'", it.c_str());
        }
    }

#ifdef DEBUG_RESOURCEDUMP
    GetFileSystem()->DumpStructure(std::ofstream("logs/vfs.txt"));
#endif

    auto Engine = new MoonGlare::Core::Engine(m_World.get());

    if (m_Configuration->m_Core.m_EnableConsole) {
        auto c = new Modules::BasicConsole();
        _init_chk(c, "Unable to initialize console!");
        m_World->SetConsole(c);
    }

    Engine->Initialize();

    //Temporary solution which probably will be used for eternity
    auto Input = Engine->GetWorld()->GetInputProcessor();
    auto rctx = m_Renderer->GetContext();
    rctx->SetInputHandler(Input);
    Input->SetInputSource(rctx);

    AddLog(Debug, "Application initialized");
#undef _init_chk

    m_Renderer->SetStopObserver([Engine]() { Engine->Exit(); });

    if (!PostSystemInit()) {
        AddLogf(Error, "Post system init action failed!");
        throw "Post system init action failed!";
    }

    m_Flags.m_Initialized = true;
}

void iApplication::Execute() {
    try {
        Initialize();
        MoonGlare::Core::GetEngine()->EngineMain();
    }
    catch (...) {
        Finalize();
        throw;
    }
    Finalize();
}

void iApplication::Finalize() {
#define _finit_chk(WHAT, ERRSTR, ...) do { if(!WHAT::InstanceExists()) break; if(!WHAT::Instance()->Finalize()) { AddLogf(Error, ERRSTR, __VA_ARGS__); } } while(false)
#define _del_chk(WHAT, ERRSTR, ...) do { _finit_chk(WHAT, ERRSTR, __VA_ARGS__); WHAT::DeleteInstance(); } while(false)

    SaveSettings();

    auto Console = dynamic_cast<Modules::BasicConsole*>(m_World->GetConsole());
    m_World->SetConsole(nullptr);
    if (Console) {
        Console->Finalize();
        delete Console;
    }

    MoonGlare::Core::Engine::Instance()->Finalize();
    DataManager::DeleteInstance();

    if (m_Renderer && !m_Renderer->Finalize()) 
        AddLogf(Error, "Unable to finalize renderer");
    m_Renderer.reset();

    _finit_chk(ModulesManager, "Finalization of modules manager failed!");

    ModulesManager::DeleteInstance();
    MoonGlare::Core::Engine::DeleteInstance();
    DataManager::DeleteInstance();

    _del_chk(ScriptEngine, "Finalization of script engine failed!");
    if(!m_AssetManager->Finalize()) {
         AddLogf(Error, "AssetManager finalization failed");
    }
    m_AssetManager.reset();
    _del_chk(FileSystem::MoonGlareFileSystem, "Finalization of filesystem failed!");

    m_World.reset();

    AddLog(Debug, "Application finalized");
#undef _finit_chk
#undef _del_chk
    m_Flags.m_Initialized = false;
}

//---------------------------------------------------------------------------------------

void iApplication::OnActivate() {
    m_Flags.m_Active = true;
    AddLogf(Debug, "Application activated");
}

void iApplication::OnDeactivate() {
    m_Flags.m_Active = false;
    AddLogf(Debug, "Application deactivated");
}

void iApplication::Exit() {
    AddLogf(Debug, "Exit called");
    MoonGlare::Core::GetEngine()->Exit();
}

void iApplication::Restart() {
    m_Flags.m_Restart = true;
    AddLogf(Debug, "Starting restart");
    MoonGlare::Core::GetEngine()->Exit();
}

const char* iApplication::ExeName() const {
    return "";
}

} //namespace Application
} //namespace MoonGlare

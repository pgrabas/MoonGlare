/*
  * Generated by cppsrc.sh
  * On 2015-06-29 10:15:07.37
  * by Immethis
*/
/*--END OF HEADER BLOCK--*/

#include <pch.h>
#include <boost/filesystem.hpp>

#include <nfMoonGlare.h>

#include <Engine/Core/DataManager.h>
#include <Engine/Modules/ModuleManager.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/InputProcessor.h>
#include <Engine/World.h>
#include "Core/Scene/ScenesManager.h"

#include <Renderer/Renderer.h>
#include <Renderer/ScriptApi.h>
#include <Foundation/OS/Path.h>
#include <Core/Scripts/ScriptEngine.h>

#include "Modules/BasicConsole.h"

#include "Application.h"

#include <AssetSettings.x2c.h>
#include <RendererSettings.x2c.h>
#include <EngineSettings.x2c.h>

#include <Foundation/SoundSystem/iSoundSystem.h>

#include <boost/algorithm/string.hpp>

#include <Foundation/Settings.h>

#include <Source/Renderer/RenderDevice.h>

namespace MoonGlare {

Application::Application() {
    m_Flags.m_UintValue = 0;

    m_Configuration = std::make_unique<x2c::Settings::EngineSettings_t>();
    m_ConfigurationFileName = OS::GetSettingsDirectory() + "Engine.xml";
    m_SettingsFileName = OS::GetSettingsDirectory() + "Engine.cfg";
}

Application::~Application() {}

//---------------------------------------------------------------------------------------

bool Application::PreSystemInit() {
    return true;
}

bool Application::PostSystemInit() {
    GetModulesManager()->OnPostInit();
    Core::GetEngine()->PostSystemInit();
    m_Renderer->GetContext()->HookMouse();
    return true;
}

//---------------------------------------------------------------------------------------

void Application::LoadSettings() {
    if (m_Flags.m_Initialized) {
        throw "Cannot load settings after initialization!";
    }

    auto stt = std::make_shared<Settings>();
    m_World->SetSharedInterface(stt);
    stt->LoadFromFile(m_SettingsFileName);

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

void Application::SaveSettings() {

    auto stt = m_World->GetSharedInterface<Settings>();
    if (stt->Changed())
        stt->SaveToFile(m_SettingsFileName);

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
    doc.save_file(m_ConfigurationFileName.c_str());

    m_Flags.m_SettingsChanged = false;
}

//---------------------------------------------------------------------------------------

using Modules::ModulesManager;
using FileSystem::MoonGlareFileSystem;
using MoonGlare::Core::Scripts::ScriptEngine;
using DataManager = MoonGlare::Core::Data::Manager;

void Application::Initialize() {
    m_World = std::make_unique<World>();
    m_World->SetInterface(this);
    auto ModManager = new ModulesManager(m_World.get());

    LoadSettings();

    if (!PreSystemInit()) {
        AddLogf(Error, "Pre system init action failed!");
        throw "Pre system init action failed";
    }
#define _init_chk(WHAT, ERRSTR, ...) \
do { if(!(WHAT)->Initialize()) { AddLogf(Error, ERRSTR, __VA_ARGS__); throw ERRSTR; } } while(false)

    _init_chk(new MoonGlareFileSystem(), "Unable to initialize internal filesystem!");

    m_World->SetInterface<iFileSystem>(GetFileSystem());

    if (!ModManager->Initialize()) {
        AddLogf(Error, "Unable to initialize modules manager!");
        throw "Unable to initialize modules manager";
    }

    auto ss = SoundSystem::iSoundSystem::Create();
    ss->Initialize(*m_World);
    m_World->SetSharedInterface(ss);

    auto scrEngine = new ScriptEngine(m_World.get());
    m_World->SetScriptEngine(scrEngine);
    _init_chk(scrEngine, "Unable to initialize script engine!");
    
    m_Renderer = std::make_unique<Renderer::RendererFacade>();
    m_World->SetRendererFacade(m_Renderer.get());

    m_Renderer->GetScriptApi()->Install(scrEngine->GetLua());
    m_Renderer->SetConfiguration(m_Configuration->m_Renderer);
    m_Configuration->m_Display.visible = false;
    m_Renderer->Initialize(m_Configuration->m_Display, GetFileSystem());

    auto datamgr = new DataManager(m_World.get());
    datamgr->SetLangCode(m_Configuration->m_Core.m_LangCode);

    LoadDataModules();

#ifdef DEBUG_RESOURCEDUMP
    {
        std::ofstream fsvfs("logs/vfs.txt");
        GetFileSystem()->DumpStructure(fsvfs);
    }
#endif

    auto Engine = new MoonGlare::Core::Engine(m_World.get());

    if (!m_World->Initialize()) {
        AddLogf(Error, "Failed to initialize world!");
        throw "Failed to initialize world!";
    }

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

void Application::LoadDataModules() {
    static const std::string moduleListFileName = OS::GetSettingsDirectory() + "ModuleList.txt";
    std::ifstream file(moduleListFileName, std::ios::in);
                   
    while (!file.eof()) {
        std::string mod;
        std::getline(file, mod);
        
        boost::trim(mod);
                    
        auto comment = mod.find('#');
        if (comment != std::string::npos) {
            mod = mod.substr(0, comment);
            boost::trim(mod);
        }
        if (mod.empty())
            continue;

        AddLogf(Debug, "Trying to load container '%s'", mod.c_str());
        if (!GetFileSystem()->LoadContainer(mod)) {
            AddLogf(Error, "Unable to open container: '%s'", mod.c_str());
        }
    }
}                       

void Application::Execute() {
    try {
        Initialize();
        if (!m_World->PreSystemStart()) {
            AddLogf(Error, "Failure during PreSystemStart");
            return;
        }

        WaitForFirstScene();
        m_Renderer->GetContext()->SetVisible(true);

        auto engineThread = std::thread([this]() {
            ::OrbitLogger::ThreadInfo::SetName("CORE");
            MoonGlare::Core::GetEngine()->EngineMain();
        });

        ::OrbitLogger::ThreadInfo::SetName("REND");
        m_Renderer->EnterLoop();

        engineThread.join();

        if (!m_World->PreSystemShutdown()) {
            AddLogf(Error, "Failure during PreSystemShutdown");
            return;
        }
    }
    catch (...) {
        Finalize();
        throw;
    }
    Finalize();
}

void Application::Finalize() {
#define _finit_chk(WHAT, ERRSTR, ...) do { if(!WHAT::s_instance) break; if(!WHAT::s_instance->Finalize()) { AddLogf(Error, ERRSTR, __VA_ARGS__); } } while(false)
#define _del_chk(WHAT, ERRSTR, ...) do { _finit_chk(WHAT, ERRSTR, __VA_ARGS__); WHAT::DeleteInstance(); } while(false)

    SaveSettings();

    auto Console = dynamic_cast<Modules::BasicConsole*>(m_World->GetConsole());
    m_World->SetConsole(nullptr);
    if (Console) {
        Console->Finalize();
        delete Console;
    }

    MoonGlare::Core::Engine::s_instance->Finalize();
    DataManager::DeleteInstance();

    if(m_Renderer)
        m_Renderer->Finalize();
    m_Renderer.reset();

    _finit_chk(ModulesManager, "Finalization of modules manager failed!");

    ModulesManager::DeleteInstance();
    MoonGlare::Core::Engine::DeleteInstance();
    DataManager::DeleteInstance();

    _del_chk(ScriptEngine, "Finalization of script engine failed!");
    _del_chk(FileSystem::MoonGlareFileSystem, "Finalization of filesystem failed!");

    m_World.reset();

    AddLog(Debug, "Application finalized");
#undef _finit_chk
#undef _del_chk
    m_Flags.m_Initialized = false;
}

void Application::WaitForFirstScene() {
    auto Device = m_Renderer->GetDevice();
    auto Ctx = m_Renderer->GetContext();

    DebugLog(Debug, "Engine initialized. Waiting for scene to be ready.");
    while (!m_World->GetScenesManager()->CurrentScene()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        m_World->GetScenesManager()->ChangeScene();
        Device->ProcessPendingCtrlQueues();
        Ctx->Process();
    }
    DebugLog(Debug, "Scene became ready. Starting main loop.");
}

//---------------------------------------------------------------------------------------

void Application::OnActivate() {
    m_Flags.m_Active = true;
    AddLogf(Debug, "Application activated");
}

void Application::OnDeactivate() {
    m_Flags.m_Active = false;
    AddLogf(Debug, "Application deactivated");
}

void Application::Exit() {
    AddLogf(Debug, "Exit called");
    MoonGlare::Core::GetEngine()->Exit();
}

void Application::Restart() {
    m_Flags.m_Restart = true;
    AddLogf(Debug, "Starting restart");
    MoonGlare::Core::GetEngine()->Exit();
}

const char* Application::ExeName() const {
    return "";
}

} //namespace MoonGlare

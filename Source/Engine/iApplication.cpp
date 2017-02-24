/*
  * Generated by cppsrc.sh
  * On 2015-06-29 10:15:07.37
  * by Immethis
*/
/*--END OF HEADER BLOCK--*/

#include <pch.h>
#include <MoonGlare.h>

#include <Engine/Core/DataManager.h>
#include <Engine/Core/Console.h>
#include <Engine/ModulesManager.h>
#include <Engine/Core/Engine.h>
#include <Engine/Core/InputProcessor.h>
#include <Engine/World.h>

#include <Assets/AssetManager.h>
#include <Renderer/Renderer.h>
#include <Renderer/Context.h>

#include <iApplication.h>

#include <AssetSettings.x2c.h>

#include "Graphic/GraphicSettings.h"

namespace MoonGlare {
namespace Application {

SPACERTTI_IMPLEMENT_CLASS_SINGLETON(iApplication);

iApplication::iApplication() : BaseClass() {
	m_Flags.m_UintValue = 0;
	SetThisAsInstance();
}

iApplication::~iApplication() {}

//---------------------------------------------------------------------------------------

bool iApplication::PreSystemInit() {
	return true;
}

bool iApplication::PostSystemInit() {
	return true;
}

//---------------------------------------------------------------------------------------

using Modules::ModulesManager;
using MoonGlare::Core::Console;
using FileSystem::MoonGlareFileSystem;
using MoonGlare::Core::Scripts::ScriptEngine;
using DataManager = MoonGlare::Core::Data::Manager;

#define _chk(WHAT, ERRSTR, ...) do { if(!(WHAT)) { AddLogf(Error, ERRSTR, __VA_ARGS__); } } while(false)
#define _chk_ret(WHAT, ERRSTR, ...) do { if(!(WHAT)) { AddLogf(Error, ERRSTR, __VA_ARGS__); return false; } } while(false)

bool iApplication::Initialize() {
	if (!PreSystemInit()) {
		AddLogf(Error, "Pre system init action failed!");
		return false;
	}
#define _init_chk(WHAT, ERRSTR, ...) do { if(!(WHAT)->Initialize()) { AddLogf(Error, ERRSTR, __VA_ARGS__); return false; } } while(false)

	_init_chk(new MoonGlareFileSystem(), "Unable to initialize internal filesystem!");

	m_AssetManager = std::make_unique<Asset::AssetManager>();
	if (!m_AssetManager->Initialize(x2c::Settings::AssetSettings_t())) {
		AddLogf(Error, "Unable to initialize asset manager!");
		return false;
	}

	auto ModManager = new ModulesManager();
	::Settings->Load();
	if (!ModManager->Initialize()) {
		AddLogf(Error, "Unable to initialize modules manager!");
		return false;
	}

	_init_chk(new ScriptEngine(), "Unable to initialize script engine!");

	m_Renderer = std::make_unique<Renderer::RendererFacade>();

	using Graphic::GraphicSettings;
	Renderer::ContextCreationInfo ctxifo;
	ctxifo.m_Width = GraphicSettings::Width::get();
	ctxifo.m_Height = GraphicSettings::Height::get();
	ctxifo.MonitorIndex = GraphicSettings::Monitor::get();
	ctxifo.FullScreen = GraphicSettings::FullScreen::get();
	if (!m_Renderer->Initialize(ctxifo, m_AssetManager.get())) {
		AddLogf(Error, "Unable to initialize renderer");
		return false;
	}

	auto window = std::make_unique<Graphic::Window>(m_Renderer->GetContext()->GetHandle(), true);
	auto Device = new Graphic::cRenderDevice(std::move(window), m_AssetManager.get());

	if (!(new DataManager())->Initialize(ScriptEngine::Instance())) {
		AddLogf(Error, "Unable to initialize data manager!");
		return false;
	}

	m_World = std::make_unique<World>();
	auto Engine = new MoonGlare::Core::Engine(m_World.get(), m_Renderer.get());

	if (Settings->Engine.EnableConsole) {
		auto c = new Console();
		_init_chk(c, "Unable to initialize console!");
		m_World->SetConsole(c);
	}

	Graphic::GetRenderDevice()->Initialize();
	Engine->Initialize();

	//Temporary solution which probably will be used for eternity
	auto Input = Engine->GetWorld()->GetInputProcessor();
	auto Window = Device->GetContext().get();
	Input->SetInputSource(Window);
	Window->SetInputProcessor(Input);

	AddLog(Debug, "Application initialized");
#undef _init_chk

	if (!PostSystemInit()) {
		AddLogf(Error, "Post system init action failed!");
		return false;
	}

	if (!Core::GetEngine()->PostSystemInit()) {
		AddLogf(Error, "Engine core post system init action failed!");
		return false;
	}

	return true;
}

bool iApplication::Execute() {
	bool Success = false;
	if (!Initialize()) {
		AddLogf(Error, "Unable to initialize application!");
	} else {
		MoonGlare::Core::GetEngine()->EngineMain();
		Success = true;
	}

	if (!Finalize()) {
		AddLogf(Error, "Unable to finalize application!");
	}

	return Success;
}

bool iApplication::Finalize() {
#define _finit_chk(WHAT, ERRSTR, ...) do { if(!WHAT::InstanceExists()) break; if(!WHAT::Instance()->Finalize()) { AddLogf(Error, ERRSTR, __VA_ARGS__); } } while(false)
#define _del_chk(WHAT, ERRSTR, ...) do { _finit_chk(WHAT, ERRSTR, __VA_ARGS__); WHAT::DeleteInstance(); } while(false)

	Settings->Save();

	_del_chk(Console, "Console finalization failed");

	_finit_chk(MoonGlare::Core::Engine, "Engine finalization failed");
	_finit_chk(DataManager, "Data Manager finalization failed");
	_finit_chk(Graphic::cRenderDevice, "Render device finalization failed");

	Graphic::cRenderDevice::DeleteInstance();

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

	AddLog(Debug, "Application finalized");
#undef _finit_chk
#undef _del_chk
	return true;
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

const char* iApplication::ExeName() const {
	return "";
}

//----------------------------------------------------------------------------------

void iApplication::RegisterScriptApi(ApiInitializer &root) {
	root
	.deriveClass<ThisClass, BaseClass>("ciApplication")
		.addFunction("Exit", &iApplication::Exit)
		.addProperty("DoRestart", &iApplication::DoRestart, &iApplication::SetRestart)
	.endClass()
	;
}

RegisterApiDerivedClass(iApplication, &iApplication::RegisterScriptApi);
RegisterApiInstance(iApplication, &iApplication::Instance, "Application");

} //namespace Application
} //namespace MoonGlare

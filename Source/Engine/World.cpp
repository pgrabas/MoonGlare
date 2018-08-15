/*
  * Generated by cppsrc.sh
  * On 2016-05-03 21:31:13,10
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>
#include "World.h"

#include "Core/InputProcessor.h"
#include "Core/Scene/ScenesManager.h"
#include "Core/Configuration.Runtime.h"

#include "iConsole.h"
#include "Core/Engine.h"

namespace MoonGlare {

World::World()
	: m_ScriptEngine(nullptr) {
    runtimeConfiguration = std::make_unique<Core::RuntimeConfiguration>();
}

World::~World() {
}

//------------------------------------------------------------------------------------------

bool World::Initialize() {
	THROW_ASSERT(m_ScriptEngine, "m_ScriptEngine assert failed!");

	if (!m_EntityManager.Initialize()) {
		AddLogf(Error, "Failed to initialize EntityManager!");
		return false;
	}

	m_InputProcessor = std::make_unique<Core::InputProcessor>();
	if (!m_InputProcessor->Initialize(this)) {
		AddLogf(Error, "Failed to initialize InputProcessor");
		return false;
	}

	m_ScenesManager = std::make_unique<Core::Scene::ScenesManager>();
	if (!m_ScenesManager->Initialize(this, &runtimeConfiguration->scene)) {
		AddLogf(Error, "Failed to initialize ScenesManager");
		return false;
	}

	return true;
}

bool World::Finalize() {
	if (m_ScenesManager && !m_ScenesManager->Finalize()) {
		AddLogf(Error, "Failed to finalize InputProcessor");
	}
	m_ScenesManager.reset();

	if (m_InputProcessor) {

#ifdef DEBUG_DUMP
		pugi::xml_document xdoc;
		m_InputProcessor->Save(xdoc.append_child("KeyConfDump"));
		xdoc.save_file("logs/KeyConfDump.xml");
#endif
		if (!m_InputProcessor->Finalize()) {
			AddLogf(Error, "Failed to finalize InputProcessor");
		}
		m_InputProcessor.reset();
	}

	if (!m_EntityManager.Finalize()) {
		AddLogf(Error, "Failed to finalize EntityManager!");
	}

    ReleaseAllInterfaces();

	return true;
}

bool World::PostSystemInit() {
	if (!m_ScenesManager || !m_ScenesManager->PostSystemInit()) {
		AddLog(Error, "Failure during SceneManager PostSystemInit");
		return false;
	}
	return true;
}

bool World::PreSystemStart() {
	if (!m_ScenesManager || !m_ScenesManager->PreSystemStart()) {
		AddLog(Error, "Failure during SceneManager PostSystemInit");
		return false;
	}
	return true;
}

bool World::PreSystemShutdown() {
	if (!m_ScenesManager || !m_ScenesManager->PreSystemShutdown()) {
		AddLog(Error, "Failure during SceneManager PostSystemInit");
		return false;
	}
	return true;
}

bool World::Step(const Core::MoveConfig & config) {
	if (!m_InputProcessor->Step(config)) {
		AddLog(Error, "Failed to Step InputProcessor");
	}
	if (!m_ScenesManager->Step(config)) {
		AddLog(Error, "Failed to Step ScenesManager");
	}
	return true;
}

//------------------------------------------------------------------------------------------

Core::Engine* World::GetEngine() {
    return MoonGlare::Core::GetEngine();
}

} //namespace MoonGlare

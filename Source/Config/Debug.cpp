/*
 * Debug.cpp
 *
 * Source file for debug configuration
 *
 *  Created on: 03-08-2013
 *      Author: Paweu
 */
#include <pch.h>
#ifdef DEBUG
#include <MoonGlare.h>

#include "Core/Interfaces.h"
#include "Core/Events.h"

#include <Core/Scripts/LuaApi.h>

namespace Config {
namespace Debug {

bool EnableFlags::Physics = true;
bool EnableFlags::PhysicsDebugDraw = false;

bool EnableFlags::ShowTitleBarDebugInfo = true;

//---------------------------------------------------------------------------------------

void DebugThrow(const char* msg) {
	throw std::runtime_error(msg);
}

void DebugSleep(int ms) {
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void ScriptDebug(ApiInitializer &root){
	root
	.addFunction("Dump", &DumpRuntime)
	.addFunction("Throw", &DebugThrow)
	.addFunction("Sleep", &DebugSleep)
	.beginNamespace("Flags")
		.beginNamespace("Physics")
			.addVariable("Enabled", &EnableFlags::Physics)
			.addVariable("DebugDraw", &EnableFlags::PhysicsDebugDraw)
		.endNamespace()
		.addVariable("ShowTitleBarDebugInfo", &EnableFlags::ShowTitleBarDebugInfo)
	.endNamespace();
	;
}

RegisterDebugApi(ScriptDebug, &ScriptDebug, "Debug");

//---------------------------------------------------------------------------------------

volatile bool _ThreadCanContinue;

void DebugThread() {
#if 0
	while (_ThreadCanContinue) {
		PROCESS_MEMORY_COUNTERS pmc;
		if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
			AddLogf(Hint, "Memory usage: %.3f MB", pmc.PeakWorkingSetSize / (1024.0f * 1024.0f));
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
#endif // 0
}

void Initialize() {
	_ThreadCanContinue = true;
	//std::thread(&DebugThread).detach();
#ifdef _BUILDING_ENGINE_
#endif
}
void Finalize() {
	_ThreadCanContinue = false;
}

//---------------------------------------------------------------------------------------
void Debug::DumpRuntime() {
	REQUIRE_REIMPLEMENT();
//#ifndef _DISABLE_LOG_SYSTEM_
	//GabiLib::GabiTracker::DumpInstances(__CreateLog(Debug));
//#endif
}

#ifdef _BUILDING_ENGINE_
void Debug::DumpAll(const char* prefixname) {
	char buf[256];
	Space::RTTI::Tracker::SortList();

	std::ofstream Instances((sprintf(buf, "logs/%s_Instances.txt", prefixname), buf));
	Space::RTTI::Tracker::DumpInstances(Instances);

	std::ofstream ClassInfo((sprintf(buf, "logs/%s_ClassInfo.txt", prefixname), buf));
	Space::RTTI::Tracker::DumpClasees(ClassInfo);

	std::ofstream ClassLists((sprintf(buf, "logs/%s_ClassLists.txt", prefixname), buf));
	MoonGlare::Core::Interfaces::DumpLists(ClassLists);

	std::ofstream Events((sprintf(buf, "logs/EventTable.txt"), buf));
	MoonGlare::Core::BaseEventInfo::DumpClasses(Events);
}

void Debug::CheckInstances() {
	bool hdr = false;
	for (auto it = Space::RTTI::Tracker::begin(), jt = Space::RTTI::Tracker::end(); it != jt; ++it) {
		if ((*it)->InstancesCount == 0)
			continue;
		if (!hdr) {
			AddLog(Error, "MEMORY LEAK!!!");
			AddLog(Error, "Remain instances:");
			hdr = true;
		}
		AddLogf(Error, "remain: %3d Class: '%s' ", (*it)->InstancesCount, (*it)->GetName());
	}
}
#endif 

//---------------------------------------------------------------------------------------
		
} //namespace Debug
} //namespace Config

#endif

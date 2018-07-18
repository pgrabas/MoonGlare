/*
  * Generated by cppsrc.sh
  * On 2016-08-04 19:45:53,67
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>
#include "EventDispatcher.h"


namespace MoonGlare::Core {
	
::Space::RTTI::TypeInfoInitializer<EventHandlerInterface, BaseEventCallDispatcher, EventDispatcher> EventTypeInfo;

EventDispatcher::EventDispatcher() {
	static bool loginit = false;
	if (!loginit) {
		loginit = true;
		::OrbitLogger::LogCollector::SetChannelName(OrbitLogger::LogChannels::Event, "EVNT");
	}
}

bool EventDispatcher::Initialize(World *world) {
	ASSERT(world);
    return true;
}

bool EventDispatcher::Finalize() {
	return true;
}

} //namespace Core::MoonGlare 

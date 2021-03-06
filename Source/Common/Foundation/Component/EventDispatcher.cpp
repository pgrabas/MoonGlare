#include "EventDispatcher.h"

namespace MoonGlare::Component {
	
EventDispatcher::EventDispatcher() {
    buffer.Zero();
}

void EventDispatcher::SetEventSink(lua_State *lua, EventScriptSink *sink) {
    assert((lua && sink) || (!lua && !sink));
    luaState = lua;
    eventSink = sink;
}

void EventDispatcher::Step() {
    if (buffer.Empty())
        return;

    std::lock_guard<std::recursive_mutex> lock(bufferMutex);

    for (auto item : buffer) {
        const BaseQueuedEvent *base = reinterpret_cast<const BaseQueuedEvent*>(item.memory);
        (this->*(base->sendFunc))(*base);
    }
    buffer.Clear();
}

}

/*
* Generated by cppsrc.sh
* by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once

#include <Foundation/MathSupportBullet3.h>

namespace MoonGlare::Physics::Component {	  

template<typename OWNER, Entity OWNER::*ptr>
inline Entity ExtractEntity(const OWNER *owner) {
    return (owner->*ptr);
}

struct OnCollisionEnterEvent {
    static constexpr char* EventName = "CollisionEnter";
    static constexpr char* HandlerName = "OnCollisionEnterEvent";

    Entity m_Destination;
    Entity m_Object;

    btVector3 m_Normal;

    friend std::ostream& operator<<(std::ostream& out, const OnCollisionEnterEvent &event) {
        out << "OnCollisionEnterEvent"
            << "[Destination:" << event.m_Destination
            << ";Object:" << event.m_Object
            << ";Normal:" << event.m_Normal
            << "]";
        return out;
    }

    static ApiInitializer RegisterLuaApi(ApiInitializer api) {
        return api
            .beginClass<OnCollisionEnterEvent>("cOnCollisionEnterEvent")
                .addData("Destination", &OnCollisionEnterEvent::m_Destination, false)
                .addData("Object", &OnCollisionEnterEvent::m_Object, false)
                .addStaticString("EventName", EventName)
                .addStaticString("HandlerName", HandlerName)
            .endClass();
    }
};

struct OnCollisionLeaveEvent {
    static constexpr char* EventName = "CollisionLeave";
    static constexpr char* HandlerName = "OnCollisionLeaveEvent";

    Entity m_Destination;
    Entity m_Object;

    btVector3 m_Normal;

    friend std::ostream& operator<<(std::ostream& out, const OnCollisionLeaveEvent &event) {
        out << "OnCollisionLeaveEvent"
            << "[Destination:" << event.m_Destination
            << ";Object:" << event.m_Object
            << ";Normal:" << event.m_Normal
            << "]";
        return out;
    }

    static ApiInitializer RegisterLuaApi(ApiInitializer api) {
        return api
            .beginClass<OnCollisionLeaveEvent>("cOnCollisionLeaveEvent")
                .addData("Destination", &OnCollisionLeaveEvent::m_Destination, false)
                .addData("Object", &OnCollisionLeaveEvent::m_Object, false)
                .addStaticString("EventName", EventName)
                .addStaticString("HandlerName", HandlerName)
            .endClass();
    }
};

} //namespace MoonGlare::Physics::Component

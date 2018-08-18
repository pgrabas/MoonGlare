#pragma once

#ifndef ScriptComponent_H
#define ScriptComponent_H

#include <Foundation/Scripts/ApiInit.h>
#include <Foundation/Scripts/ErrorHandling.h>
#include <Foundation/Scripts/LuaStackOverflowAssert.h>
#include <Foundation/Scripts/iLuaRequire.h>

#include <Foundation/Component/EntityEvents.h>

#include <Core/Component/TemplateStandardComponent.h>
#include <libSpace/src/Container/StaticVector.h>

#include <Core/Scripts/ScriptEngine.h>

namespace MoonGlare::Core::Scripts::Component {
using namespace MoonGlare::Scripts;
using namespace Core::Component;

struct GameObjectTable;

class ScriptComponent
    : public AbstractSubsystem
    , public SubSystemIdWrap<SubSystemId::Script> {
public:
    friend struct GameObject;    //TODO: this is temporary
    //friend struct ScriptObject;  //TODO: this is temporary

    ScriptComponent(SubsystemManager *Owner);
    virtual ~ScriptComponent();

    static ApiInitializer RegisterScriptApi(ApiInitializer root);

    virtual bool Initialize() override;
    virtual bool Finalize() override;
    virtual void Step(const MoveConfig &conf) override;
    virtual bool Load(ComponentReader &reader, Entity parent, Entity owner) override;

    void HandleEvent(const MoonGlare::Component::EntityDestructedEvent &event);

    using LuaHandle = int;

    union FlagsMap {
        struct MapBits_t {
            bool m_Valid : 1; //Entity is not valid or requested to be deleted;
            bool m_Active : 1; //Script has step function and it shall be called

            bool m_OnPerSecond : 1;//called when movedata.secondstep is true, need to be activated
            bool m_Step : 1;
            //bool m_Event : 1;
        };
        MapBits_t m_Map;
        uint32_t m_UintValue;

        void SetAll() { m_UintValue = 0; m_UintValue = ~m_UintValue; }
        void ClearAll() { m_UintValue = 0; }

        static_assert(sizeof(MapBits_t) <= sizeof(decltype(m_UintValue)), "Invalid Function map elements size!");
    };

    struct ScriptEntry {
        FlagsMap m_Flags;
        Entity m_OwnerEntity;	
        uint32_t padding;

        void Reset() {
            m_Flags.m_Map.m_Valid = false;
        }
    };
    //static_assert((sizeof(ScriptEntry) % 8) == 0, "Invalid ScriptEntry size!");
    //static_assert(std::is_pod<ScriptEntry>::value, "ScriptEntry must be pod!");

    int GetGameObject(lua_State *lua, Entity Owner);
    GameObject* GetGameObject(Entity Owner);

    ScriptEntry* GetEntry(Entity e) { 
        auto index = m_EntityMapper.GetIndex(e);
        if (index == ComponentIndex::Invalid)
            return nullptr;
        return &m_Array[index];
    }

    EventScriptSink* GetEventSink() { return &eventScriptSinkProxy; }
    void HandleEvent(lua_State* lua, Entity destination);
protected:
    Scripts::ScriptEngine *m_ScriptEngine;
    iRequireModule *requireModule;                                         
    std::shared_ptr<GameObjectTable> gameObjectTable;

    template<class T> using Array = Space::Container::StaticVector<T, MoonGlare::Configuration::Storage::ComponentBuffer>;
    Array<ScriptEntry> m_Array;
    EntityArrayMapper<> m_EntityMapper;

    //void GetObjectRootInstance(lua_State *lua, Entity Owner); //returns OR GO on success and never fails

    class EventScriptSinkProxy : public EventScriptSink {
        ScriptComponent * reciver;
    public:
        EventScriptSinkProxy(ScriptComponent *r) : reciver(r) {}
        void HandleEvent(lua_State* lua, Entity destination) override { reciver->HandleEvent(lua, destination); }
    };
    EventScriptSinkProxy eventScriptSinkProxy{ this };

    void ReleaseComponent(lua_State *lua, size_t Index);

    void GetInstancesTable(lua_State *lua) {
        lua_pushlightuserdata(lua, GetInstancesTablePointer());
        lua_gettable(lua, LUA_REGISTRYINDEX);
    }

    void *GetInstancesTablePointer() { return this; }
private:
//support functions
    int lua_GetScriptComponent(lua_State *lua, Entity Owner);
    int lua_GetComponentInfo(lua_State *lua, SubSystemId cid, Entity Owner);
    int lua_MakeComponentInfo(lua_State *lua, SubSystemId cid, Entity owner, iSubsystem *cptr);
    static int lua_DereferenceHandle(lua_State *lua);
    static int lua_SetComponentState(lua_State *lua);
}; 											

} //namespace MoonGlare::Core::Scripts::Component 

#endif

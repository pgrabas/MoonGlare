#pragma once

#include <libSpace/src/Container/StaticVector.h>

#include <Core/Component/TemplateStandardComponent.h>
#include <Core/Scripts/ScriptComponent.h>

#include <Foundation/Component/EntityEvents.h>

namespace MoonGlare::Component {     
using namespace Core::Component;

class TransformComponent;

struct MeshComponentEntry {
    union FlagsMap {
        struct MapBits_t {
            bool m_Valid : 1; //Entity is not valid or requested to be deleted;
            bool m_Visible : 1;
        };
        MapBits_t m_Map;
        uint32_t m_UintValue;

        void SetAll() { m_UintValue = 0; m_UintValue = ~m_UintValue; }
        void ClearAll() { m_UintValue = 0; }

        static_assert(sizeof(MapBits_t) <= sizeof(decltype(m_UintValue)), "Invalid Function map elements size!");
    };

    FlagsMap m_Flags;
    Entity m_Owner;

    Renderer::MeshResourceHandle meshHandle;
    Renderer::MaterialResourceHandle materialHandle;
   
    void Reset() {
        m_Flags.m_Map.m_Valid = false;
    }
};     
//static_assert((sizeof(MeshEntry) % 16) == 0, "Invalid MeshEntry size!");
//static_assert(std::is_pod_v<MeshComponentEntry>);

class MeshComponent                                                               
    : public TemplateStandardComponent<MeshComponentEntry, SubSystemId::Mesh> {
public:
    void HandleEvent(const MoonGlare::Component::EntityDestructedEvent &event);
    int PushToLua(lua_State *lua, Entity owner) override;

    MeshComponent(SubsystemManager *Owner);
    virtual ~MeshComponent();
    virtual bool Initialize() override;
    virtual void Step(const Core::MoveConfig &conf) override;
    virtual bool Load(ComponentReader &reader, Entity parent, Entity owner) override;
    virtual bool Create(Entity Owner) override;

    using MeshEntry = MeshComponentEntry;

    static MoonGlare::Scripts::ApiInitializer RegisterScriptApi(MoonGlare::Scripts::ApiInitializer root);
private:
    template<class T> using Array = Space::Container::StaticVector<T, MoonGlare::Configuration::Storage::ComponentBuffer>;
    struct LuaWrapper;
    
    TransformComponent *m_TransformComponent;

    void ReleaseElement(size_t Index);
};

}

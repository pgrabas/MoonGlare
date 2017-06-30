/*
  * Generated by cppsrc.sh
  * On 2016-08-10 17:23:57,17
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef MeshComponent_H
#define MeshComponent_H

#include <libSpace/src/Container/StaticVector.h>

#include <Core/Component/AbstractComponent.h>
#include <Core/Scripts/ScriptComponent.h>
#include <Core/Scripts/ComponentEntryWrap.h>

namespace MoonGlare {
namespace Renderer {
namespace Component {

using namespace Core::Component;

struct MeshComponentEntry : public ::Space::RTTI::TemplateTypeInfo<MeshComponentEntry> {
    union FlagsMap {
        struct MapBits_t {
            bool m_Valid : 1; //Entity is not valid or requested to be deleted;
            bool m_MeshValid : 1;
            bool m_MeshHandleChanged : 1;
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
    Handle m_SelfHandle;
    Handle m_MeshHandle;
    DataClasses::ModelPtr m_Model;
    std::string m_ModelName;

    Renderer::MeshResourceHandle meshHandle;

    bool IsVisible() const { return m_Flags.m_Map.m_Visible; }
    void SetVisible(bool v) { m_Flags.m_Map.m_Visible = v; }

    void SetMeshHandle(Handle h) {
        m_MeshHandle = h;
        m_Model.reset();
        m_Flags.m_Map.m_MeshHandleChanged = true;
    }
    Handle GetMeshHandle() const { return m_MeshHandle; }
    void SetModel(const char *name) {
        m_ModelName = name;
        m_Flags.m_Map.m_MeshHandleChanged = true;
    }

    void Reset() {
        m_Flags.m_Map.m_Valid = false;
        m_Model.reset();
        m_ModelName.clear();
    }
};     

class MeshComponent
    : public TemplateStandardComponent<MeshComponentEntry, ComponentID::Mesh> 
    , public Core::Scripts::Component::ComponentEntryWrap<MeshComponent>
{
public:
    static constexpr char *Name = "Mesh";
    static constexpr bool PublishID = true;

    MeshComponent(ComponentManager *Owner);
    virtual ~MeshComponent();
    virtual bool Initialize() override;
    virtual bool Finalize() override;
    virtual void Step(const Core::MoveConfig &conf) override;
    virtual bool Load(xml_node node, Entity Owner, Handle &hout) override;
    virtual bool Create(Entity Owner, Handle &hout) override;

//	static_assert((sizeof(MeshEntry) % 16) == 0, "Invalid MeshEntry size!");
//	static_assert(std::is_pod<MeshEntry>::value, "ScriptEntry must be pod!");

    using MeshEntry = MeshComponentEntry;

    template<bool Read, typename StackFunc>
    static bool ProcessProperty(lua_State *lua, MeshComponentEntry *e, uint32_t hash, int &luarets, int validx) {
        switch (hash) {
        case "Visible"_Hash32:
            luarets = StackFunc::funcProp<bool>(lua, e, &MeshComponentEntry::IsVisible, &MeshComponentEntry::SetVisible, validx);
            break;
        default:
            return false;
        }
        if (!Read) {
        //	e->SetDirty();
        }
        return true;
    }

    template<typename StackFunc, typename Entry>
    static bool QuerryFunction(lua_State *lua, Entry *e, uint32_t hash, int &luarets, int validx, MeshComponent *This) {
        //switch (hash) {
        //default:
            return false;
        //}
    }

    static void RegisterScriptApi(ApiInitializer &root);
private:
    template<class T> using Array = Space::Container::StaticVector<T, MoonGlare::Configuration::Storage::ComponentBuffer>;
    
    TransformComponent *m_TransformComponent;

    void ReleaseElement(size_t Index);
};

} //namespace Component 
} //namespace Renderer 
} //namespace MoonGlare 

#endif

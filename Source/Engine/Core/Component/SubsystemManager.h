#pragma once

#include <Foundation/Component/EventDispatcher.h>
#include <Foundation/Component/iSubsystem.h>
#include <Foundation/Component/EntityEvents.h>

namespace MoonGlare::Core::Component {
using namespace MoonGlare::Component;

class SubsystemManager final  
        : public MoonGlare::Component::iSubsystemManager {
public:
    SubsystemManager();
    ~SubsystemManager() override;

    bool Initialize(Entity root);
    bool Finalize();

    bool LoadComponents(pugi::xml_node node);

    InterfaceMap& GetInterfaceMap() override { return *m_World; }
    EventDispatcher& GetEventDispatcher() override { return m_EventDispatcher; }
    ComponentArray& GetComponentArray() override { return componentArray; };

    template<class T, class ... ARGS>
    bool InstallComponent(ARGS ... args) {
        return InsertComponent(std::make_unique<T>(this, std::forward<ARGS>(args)...), T::GetSubSystemId());
    }

    void Step(const MoveConfig &config);

    template<class T> 
    T* GetComponent() {
        return dynamic_cast<T*>(GetComponent(T::GetSubSystemId()));
    }

    iSubsystem* GetComponent(SubSystemId cid);

    World* GetWorld() { return m_World; }

    Entity GetRootEntity() const { return rootEntity; }

    struct ComponentInfo {
        float m_TotalStepDuration;
        unsigned m_PeriodCount;
    };

    void HandleEvent(const EntityDestructedEvent &event);
private:
    std::array<UniqueSubsystem, MoonGlare::Configuration::Storage::MaxComponentCount> m_Components;
    std::array<SubSystemId, MoonGlare::Configuration::Storage::MaxComponentCount> m_ComponentsIDs;
    size_t m_UsedCount;
    Entity rootEntity;
    World *m_World;
    EventDispatcher m_EventDispatcher;
    ComponentArray componentArray;

#ifdef PERF_PERIODIC_PRINT
    std::array<ComponentInfo, MoonGlare::Configuration::Storage::MaxComponentCount> m_ComponentInfo;
#endif

    bool InsertComponent(UniqueSubsystem cptr, SubSystemId cid);
};

} 

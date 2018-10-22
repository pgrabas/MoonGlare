/*
  * Generated by cppsrc.sh
  * On 2016-08-14 22:04:32,35
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <nfMoonGlare.h>
#include "Component/SubsystemManager.h"
#include "Component/ComponentRegister.h"
#include "Component/TemplateStandardComponent.h"
#include "EntityBuilder.h"

#include <Foundation/Component/EntityManager.h>

namespace MoonGlare {
namespace Core {

struct EntityImport {
    Entity entity{};
    int32_t parentIndex = -1;
    std::string name;
    bool enabled = true;
};

struct ComponentImport {
    int32_t entityIndex = 1;
    bool enabled = true;
    pugi::xml_node xmlNode;
};

struct EntityBuilder::ImportData {
    std::vector<EntityImport> entities;
    std::map<Component::SubSystemId, std::vector<ComponentImport>> components;
    std::vector<XMLFile> xmlFiles;

    void Prepare() {
        for (auto &c : components) {
            std::sort(c.second.begin(), c.second.end(), [](const ComponentImport &c1, const ComponentImport &c2) {
                return c1.entityIndex < c2.entityIndex;
            });
        }
    }

    void Dump(const std::string &ename, const std::string &srcname = "") {
        static int dumpid = 0;
        std::string fname = "logs/";
        fname = fmt::format("logs/{}.{}.entity", ename, dumpid++);

        std::ofstream of(fname, std::ios::out);

        if (!srcname.empty())
            of << "SOURCE: " << srcname << "\n\n";

        of << fmt::format("Entities: {}\n", entities.size());
        for (auto&it : entities) {
            of << fmt::format("\tParentIndex:{}  Name:{}  Enabled:{}\n", it.parentIndex, it.name, it.enabled);
        }
        of << "\n";
        of << fmt::format("Components: {}\n", components.size());
        for (auto &it : components) {
            std::string Name;
            if (it.first > SubSystemId::CoreBegin)
                Name = Component::ComponentRegister::GetComponentInfo(it.first)->m_Name;
            else
                Name = Component::BaseComponentInfo::GetComponentTypeInfo(static_cast<Component::ComponentClassId>(it.first)).componentName;

            of << fmt::format("\tComponent:{}  Name:{}\n", (int)it.first, Name);
            for (auto &c : it.second) {
                of << fmt::format("\t\tParentIndex:{}  Enabled:{}\n", c.entityIndex, c.enabled);
            }
        }

        of.close();
    }
};

//-------------------------------------------------------------------------------------------------

EntityBuilder::EntityBuilder(Component::SubsystemManager *Manager)
        : m_Manager(Manager) {
}

EntityBuilder::~EntityBuilder() {
}

//-------------------------------------------------------------------------------------------------

bool EntityBuilder::Build(Entity parent, const char *PatternUri, Entity &eout, std::string Name) {
    ImportData data;

    EntityImport ei;
    ei.parentIndex = -1;
    ei.name = std::move(Name);
    data.entities.emplace_back(std::move(ei));

    Import(data, PatternUri, 0);
    data.Prepare();
#ifdef DEBUG_DUMP
    data.Dump(Name, PatternUri);
#endif

    Spawn(data, parent);

    eout = data.entities[0].entity;

    return true;
}

bool EntityBuilder::Build(Entity parent, pugi::xml_node node, std::string Name) {
    ImportData data;

    Import(data, node, -1);
    data.Prepare();
#ifdef DEBUG_DUMP
    data.Dump(Name, Name);
#endif
    Spawn(data, parent);
    return true;
}

//-------------------------------------------------------------------------------------------------

void EntityBuilder::Spawn(ImportData &data, Entity parent) {
    auto world = m_Manager->GetWorld();
    auto &em = world->GetEntityManager();

    for (auto &ei : data.entities) {
        if (!ei.enabled)
            continue;

        Entity thisParent;
        if (ei.parentIndex >= 0)
            thisParent = data.entities[ei.parentIndex].entity;
        else
            thisParent = parent;

        if (!em.Allocate(thisParent, ei.entity, ei.name)) {
            AddLogf(Error, "Failed to allocate entity!");
            return;
        }         
    }

    std::array<SubSystemId, 3> ComponentOrder = {
        SubSystemId::Transform,
        SubSystemId::RectTransform,
        SubSystemId::Body,
    };

    auto SpawnComponents = [this, parent, &data](std::vector<ComponentImport>& cs) {
        for (auto &ci : cs) {
            if (!ci.enabled)
                continue; 
            Entity thisParent = {};
            Entity thisOwner = {};
            if (ci.entityIndex >= 0) {
                auto &e = data.entities[ci.entityIndex];
                thisOwner = e.entity;
                if (e.parentIndex >= 0)
                    thisParent = data.entities[e.parentIndex].entity;
                else
                    thisParent = parent;
            }
            else {
                __debugbreak();
                //owner = O/wner;
            }
            LoadComponent(thisParent, thisOwner, ci.xmlNode);
        }
    };

    for (auto cidx : ComponentOrder) {
        auto it = data.components.find(cidx);
        if (it != data.components.end()) {
            SpawnComponents(it->second);
            data.components.erase(it);
        }
    }
    
    std::vector<ComponentImport> scripts;
    auto scrit = data.components.find(SubSystemId::Script);
    if (scrit != data.components.end()) {
        scrit->second.swap(scripts);
        data.components.erase(scrit);
    }

    for (auto &c : data.components)
        SpawnComponents(c.second);

    SpawnComponents(scripts);
}

//-------------------------------------------------------------------------------------------------

void EntityBuilder::Import(ImportData &data, const char *PatternUri, int32_t entityIndex) {
    data.xmlFiles.emplace_back();
    XMLFile &xml = data.xmlFiles.back();
    if (!((iFileSystem*)GetFileSystem())->OpenXML(xml, std::string(PatternUri))) {
        AddLogf(Error, "Failed to open uri: %s", PatternUri);
        return;
    }

    auto node = xml->document_element();

    Import(data, node, entityIndex);
}

void EntityBuilder::Import(ImportData &data, pugi::xml_node node, int32_t entityIndex) {
    bool parentEnabled = entityIndex >= 0 ? data.entities[entityIndex].enabled : true;

    if (entityIndex >= 0) {
        auto &ei = data.entities[entityIndex];
        if (ei.name.empty()) 
            ei.name = node.attribute("Name").as_string("");

        ei.enabled = node.attribute("Enabled").as_bool(true) && parentEnabled;
    }

    for (auto it = node.first_child(); it; it = it.next_sibling()) {

        const char *nodename = it.name();
        auto hash = Space::Utils::MakeHash32(nodename);

        switch (hash) {
        case "Component"_Hash32: {
            ComponentImport ci;
            ci.xmlNode = it;
            ci.enabled = it.attribute("Enabled").as_bool(true) && parentEnabled;

            SubSystemId cid = SubSystemId::Invalid;
            if (!Component::ComponentRegister::ExtractCIDFromXML(it, cid)) {
                AddLogf(Warning, "Unknown component!");
                continue;
            }
            ci.entityIndex = entityIndex;
            data.components[cid].emplace_back(std::move(ci));
            continue;
        }

        case "Child"_Hash32: 
        case "Entity"_Hash32:
        {
            EntityImport ei;
            ei.parentIndex = entityIndex;
            ei.enabled = it.attribute("Enabled").as_bool(true) && parentEnabled;
            ei.name = it.attribute("Name").as_string("");

            int32_t index = static_cast<int32_t>(data.entities.size());
            data.entities.emplace_back(std::move(ei));

            auto pattern = it.attribute("Pattern").as_string(nullptr);
            if (pattern) {                                 
                Import(data, pattern, index);
            } 
            Import(data, it, index);
            continue;
        }
        default:
            AddLogf(Warning, "Unknown node: %s", nodename);
            continue;
        }
    }
}

//-------------------------------------------------------------------------------------------------

bool EntityBuilder::LoadComponent(Entity parent, Entity owner, pugi::xml_node node) {
    SubSystemId cid = SubSystemId::Invalid;

    if (!Component::ComponentRegister::ExtractCIDFromXML(node, cid)) {
        AddLogf(Warning, "Unknown component!");
        return false;
    }

    MoonGlare::Component::ComponentReader reader{ m_Manager, node };
    if (cid < SubSystemId::CoreBegin) {
        return m_Manager->GetComponentArray().Load(owner, static_cast<Component::ComponentClassId>(cid), reader);
    }     
             
    auto c = m_Manager->GetComponent(cid);
    if (!c) {
        AddLogf(Warning, "No such component: %d", cid);
        return false;
    }

    if (!c->Load(reader, parent, owner)) {
        AddLogf(Error, "Failure during loading component! cid:%d class: %s", cid, typeid(*c).name());
        return false;
    }

    return true;
}

} //namespace Core 
} //namespace MoonGlare 


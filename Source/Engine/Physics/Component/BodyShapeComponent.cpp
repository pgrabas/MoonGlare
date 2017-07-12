/*
  * Generated by cppsrc.sh
  * On 2016-08-30 21:19:19,54
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <nfMoonGlare.h>
#include <Physics/nfPhysics.h>

#include <Core/Component/ComponentManager.h>
#include <Core/Component/ComponentRegister.h>
#include <Core/Component/AbstractComponent.h>
#include <Core/Component/TransformComponent.h>
#include "BodyShapeComponent.h"
#include "BodyComponent.h"

#include <Math.x2c.h>
#include <BodyShapeComponent.x2c.h>


#include <Renderer/Renderer.h>
#include <Renderer/Resources/ResourceManager.h>
#include <Renderer/Resources/MeshResource.h>

namespace MoonGlare {
namespace Physics {
namespace Component {

struct TriangleMeshProxy : public btTriangleIndexVertexArray {
    TriangleMeshProxy(const Renderer::Resources::MeshData &data) : mesh(data){
        btIndexedMesh meshIndex;
        meshIndex.m_numTriangles = 0;
        meshIndex.m_numVertices = 0;
        meshIndex.m_indexType = PHY_INTEGER;
        meshIndex.m_triangleIndexBase = 0;
        meshIndex.m_triangleIndexStride = 3 * sizeof(int);
        meshIndex.m_vertexBase = 0;
        meshIndex.m_vertexStride = sizeof(mesh.verticles[0]);
        m_indexedMeshes.push_back(meshIndex);

        m_indexedMeshes[0].m_numTriangles = mesh.index.size() / 3;
        m_indexedMeshes[0].m_triangleIndexBase = (unsigned char*)&mesh.index[0];
        m_indexedMeshes[0].m_indexType = PHY_INTEGER;
        m_indexedMeshes[0].m_triangleIndexStride = sizeof(mesh.index[0]) * 3;

        m_indexedMeshes[0].m_numVertices = mesh.verticles.size();
        m_indexedMeshes[0].m_vertexBase = (unsigned char*)&mesh.verticles[0];
        m_indexedMeshes[0].m_vertexStride = sizeof(mesh.verticles[0]);
    }
private:
    const Renderer::Resources::MeshData &mesh;
};

//---------------------------------------------------------------------------------------

::Space::RTTI::TypeInfoInitializer<BodyShapeComponent, BodyShapeComponentEntry> BodyShapeComponentTypeInfo;
Core::Component::RegisterComponentID<BodyShapeComponent> BodyShapeComponentIDReg("BodyShape", true, &BodyShapeComponent::RegisterScriptApi);

BodyShapeComponent::BodyShapeComponent(Core::Component::ComponentManager *Owner)
        : AbstractComponent(Owner) {

    m_BodyComponent = nullptr;
    m_TransformComponent = nullptr;

    DebugMemorySetParent(GetManager());
    DebugMemorySetClassName("BodyShapeComponent");
    DebugMemoryRegisterCounter("IndexUsage", [this](DebugMemoryCounter& counter) {
        counter.Allocated = m_Array.Allocated();
        counter.Capacity = m_Array.Capacity();
        counter.ElementSize = sizeof(BodyShapeComponentEntry);
    });
}

BodyShapeComponent::~BodyShapeComponent() {
}

//---------------------------------------------------------------------------------------

void BodyShapeComponent::RegisterScriptApi(ApiInitializer &root) {
    root
        .beginClass<btCollisionShape>("cbtCollisionShape")
        .endClass()
        .beginClass<BodyShapeComponentEntry>("cBodyShapeComponentEntry")
            .addFunction("SetShape", &BodyShapeComponentEntry::SetShape)

            .addFunction("SetTriangleMesh", &BodyShapeComponentEntry::SetTriangleMesh)
            .addFunction("SetConvexMesh", &BodyShapeComponentEntry::SetConvexMesh)
            //.addFunction("SetSphere", &BodyShapeComponentEntry::SetSphere)
            //.addFunction("SetBox", &BodyShapeComponentEntry::SetBox)
        .endClass()
        ;
}

//---------------------------------------------------------------------------------------

bool BodyShapeComponent::Initialize() {
//	m_Array.fill(BodyShapeComponentEntry());

    m_BodyComponent = GetManager()->GetComponent<BodyComponent>();
    if (!m_BodyComponent) {
        AddLogf(Error, "Unable to get BodyComponent instance!");
        return false;
    }

    m_TransformComponent = GetManager()->GetComponent<TransformComponent>();
    if (!m_TransformComponent) {
        AddLogf(Error, "Unable to get TransformComponent instance!");
        return false;
    }

    return true;
}

bool BodyShapeComponent::Finalize() {
    return true;
}

//---------------------------------------------------------------------------------------

void BodyShapeComponent::Step(const Core::MoveConfig & conf) {
    return;
}

//---------------------------------------------------------------------------------------

bool BodyShapeComponent::BuildEntry(Entity Owner, Handle & hout, size_t & indexout) {
    size_t &index = indexout;
    if (!m_Array.Allocate(index)) {
        AddLogf(Error, "Failed to allocate index!");
        return false;
    }

    auto &entry = m_Array[index];
    entry.m_Flags.ClearAll();
    entry.m_Shape.reset();
    if (!GetHandleTable()->Allocate(this, Owner, hout, index)) {
        AddLog(Error, "Failed to allocate handle");
        //no need to deallocate entry. It will be handled by internal garbage collecting mechanism
        return false;
    }

    entry.m_OwnerEntity = Owner;
    entry.m_SelfHandle = hout;

    if (!m_BodyComponent->GetInstanceHandle(Owner, entry.m_BodyHandle)) {
        AddLogf(Warning, "Entity does not have body!");
        return false;
    }

    auto bodyentry = m_BodyComponent->GetEntry(entry.m_BodyHandle);
    if (!bodyentry) {
        AddLogf(Error, "Cannot get BodyComponent entry!");
        return false;
    }

    entry.m_BodyComponent = m_BodyComponent;
    entry.shapeComponent = this;

    m_EntityMapper.SetHandle(Owner, entry.m_SelfHandle);

    return true;
}

std::pair<std::unique_ptr<btCollisionShape>, ColliderType> BodyShapeComponent::LoadByName(const std::string &name, xml_node node) {
    switch (Space::Utils::MakeHash32(name.c_str())) {
    case "Box"_Hash32:
        return { std::make_unique<btBoxShape>(btVector3{ 1,1,1 }), ColliderType::Box};// convert(bbs.m_Size) / 2.0f);
    case "Sphere"_Hash32:
        return { std::make_unique<btSphereShape>(1), ColliderType::Sphere };
    case "Capsule"_Hash32:
    case "CapsuleY"_Hash32:
    {
        x2c::Component::BodyShapeComponent::CapsuleYBodyShape cbs;
        cbs.ResetToDefault();
        if (!cbs.Read(node))
            break;
        return { std::make_unique<btCapsuleShape>(cbs.radius, cbs.height), ColliderType::Capsule };
    }
    //case "Cylinder"_Hash32:
    //case "CylinderY"_Hash32:
    //{
    //    x2c::Component::BodyShapeComponent::CylinderYBodyShape_t cbs;
    //    cbs.ResetToDefault();
    //    if (!cbs.Read(ShapeNode))
    //        break;
    //    shape = std::make_unique<btCylinderShape>(convert(cbs.m_Size) / 2.0f);
    //    break;
    //}

    default:
        AddLogf(Error, "Attempt to add BodyShape of unknown type!");
    }
    return { nullptr , ColliderType::Unknown};
}

std::pair<std::unique_ptr<btCollisionShape>, ColliderType> BodyShapeComponent::LoadShape(xml_node node) {
    x2c::Component::BodyShapeComponent::ColliderComponent cc;
    cc.ResetToDefault();
    if (!cc.Read(node))
        return { nullptr , ColliderType::Unknown };

    switch (cc.type) {
    case ColliderType::Box:
        return { std::make_unique<btBoxShape>(btVector3{ 1,1,1 }), ColliderType::Box };// convert(bbs.m_Size) / 2.0f);
    case ColliderType::Capsule:
        return { std::make_unique<btCapsuleShape>(cc.radius, cc.height), ColliderType::Capsule };
    //case ColliderType::CapsuleY:
    //    return std::make_unique<btCapsuleShape>(cc.radius, cc.height);
    //case ColliderType::ConvexMesh:
    //    break;
    case ColliderType::Sphere:
        return { std::make_unique<btSphereShape>(1), ColliderType::Sphere };
    case ColliderType::TriangleMesh:
        break;
    default:
        break;
    }
    return { nullptr , ColliderType::Unknown };
}

bool BodyShapeComponent::Load(xml_node node, Entity Owner, Handle & hout) {
    size_t index;
    if (!BuildEntry(Owner, hout, index)) {
        AddLog(Error, "Failed to build entry!");
        return false;
    }

    auto &entry = m_Array[index];                                 
    std::pair<std::unique_ptr<btCollisionShape>, ColliderType>  shape;
    
    auto ShapeNode = node.child("Shape");
    auto ShapeName = ShapeNode.attribute("Name").as_string(nullptr);
    if (ShapeName) {
        shape = LoadByName(ShapeName, ShapeNode);
    }
    else {
        shape = LoadShape(node);
    }


    //if (!shape.first) {
    //    AddLogf(Error, "Failed to create BodyShape!");
    //    return false;
    //}

    entry.m_Flags.m_Map.m_Valid = true;
    //entry.
    if (shape.first) {
        entry.SetShapeInternal(std::move(shape.first));
    }
    
//	auto tcEntry = m_TransformComponent->GetEntry(Owner);
//	if (!tcEntry) {
//		AddLog(Warning, "Cannot get TransformComponent");
//	} else {
//		entry.m_Shape->setLocalScaling(tcEntry->m_GlobalScale);
//	}

    return true;
}

bool BodyShapeComponent::Create(Entity Owner, Handle &hout) {
    size_t index;
    if (!BuildEntry(Owner, hout, index)) {
        AddLog(Error, "Failed to build entry!");
        return false;
    }
    auto &entry = m_Array[index];

    entry.m_Flags.m_Map.m_Valid = true;
    return true;
}

//---------------------------------------------------------------------------------------

bool BodyShapeComponent::GetInstanceHandle(Entity Owner, Handle & hout) {
    auto h = m_EntityMapper.GetHandle(Owner);
    if (!GetHandleTable()->IsValid(this, h)) {
        return false;
    }
    hout = h;
    return true;
}

bool BodyShapeComponent::PushEntryToLua(Handle h, lua_State * lua, int &luarets) {
    auto entry = GetEntry(h);
    if (!entry) {
        return true;
    }

    luarets = 1;
    luabridge::Stack<BodyShapeComponentEntry*>::push(lua, entry);

    return true;
}

BodyShapeComponentEntry * BodyShapeComponent::GetEntry(Handle h) {
    auto *ht = GetHandleTable();
    HandleIndex hi;
    if (!ht->GetHandleIndex(this, h, hi)) {
        return nullptr;
    }
    return  &m_Array[hi];
}

//---------------------------------------------------------------------------------------

bool BodyShapeComponentEntry::SetShapeInternal(std::unique_ptr<btCollisionShape> shape) {
    m_Shape.swap(shape);

    if (!m_BodyComponent->SetShape(m_SelfHandle, m_BodyHandle, m_Shape.get())) {
        AddLogf(Error, "Failed to set body shape!");
        return false;
    }

    return true;
}

void BodyShapeComponentEntry::SetShape(btCollisionShape *shape) {
    SetShapeInternal(std::unique_ptr<btCollisionShape>(shape));
}

void BodyShapeComponentEntry::SetSphere(float Radius) {
    SetShapeInternal(std::make_unique<btSphereShape>(Radius));
}

void BodyShapeComponentEntry::SetBox(const math::vec3 & size) {
    SetShapeInternal(std::make_unique<btBoxShape>(convert(size) / 2.0f));
}

void BodyShapeComponentEntry::SetTriangleMesh(Renderer::MeshResourceHandle h) {
    auto *rf = shapeComponent->GetManager()->GetWorld()->GetRendererFacade();
    auto &mm = rf->GetResourceManager()->GetMeshManager();
    auto &md = mm.GetMeshData(h);

    meshInterface = std::make_unique<TriangleMeshProxy>(md);
    SetShapeInternal(std::make_unique<btBvhTriangleMeshShape>(meshInterface.get(), false));
}     

void BodyShapeComponentEntry::SetConvexMesh(Renderer::MeshResourceHandle h) {
    auto *rf = shapeComponent->GetManager()->GetWorld()->GetRendererFacade();
    auto &mm = rf->GetResourceManager()->GetMeshManager();
    auto &md = mm.GetMeshData(h);

    meshInterface.reset();
    SetShapeInternal(std::make_unique<btConvexHullShape>((float*)&md.verticles[0], md.verticles.size(), sizeof(md.verticles[0])));
}     

} //namespace Component 
} //namespace Physics 
} //namespace MoonGlare 

/*
  * Generated by cppsrc.sh
  * On 2016-08-10 17:23:57,17
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>

#include "MeshComponent.h"

#include <Core/Component/ComponentManager.h>
#include <Core/Component/ComponentRegister.h>
#include <Core/Component/TransformComponent.h>

#include <Engine/Renderer/nGraphic.h>
#include <Engine/Renderer/Dereferred/DereferredPipeline.h>

#include <Renderer/Resources/ResourceManager.h>
#include <Renderer/Resources/MeshResource.h>
#include <Renderer/Renderer.h>

namespace MoonGlare {
namespace Renderer {
namespace Component {

::Space::RTTI::TypeInfoInitializer<MeshComponent, MeshComponentEntry> MeshComponentTypeInfo;
RegisterComponentID<MeshComponent> MeshComponentReg("Mesh", true, &MeshComponent::RegisterScriptApi);

MeshComponent::MeshComponent(ComponentManager * Owner) 
    : TemplateStandardComponent(Owner)
    , m_TransformComponent(nullptr)
{

    DebugMemorySetClassName("MeshComponent");
    DebugMemoryRegisterCounter("IndexUsage", [this](DebugMemoryCounter& counter) {
        counter.Allocated = m_Array.Allocated();
        counter.Capacity = m_Array.Capacity();
        counter.ElementSize = sizeof(MeshEntry);
    });
}

MeshComponent::~MeshComponent() {
}

//------------------------------------------------------------------------------------------

void MeshComponent::RegisterScriptApi(ApiInitializer & root) {
    root
    .beginClass<MeshEntry>("cMeshEntry")
        .addProperty("Visible", &MeshEntry::IsVisible, &MeshEntry::SetVisible)
        .addProperty("MeshHandle", &MeshEntry::GetMeshHandle, &MeshEntry::SetMeshHandle)
        .addFunction("SetModel", &MeshEntry::SetModel)
    .endClass()
    ;
}

//------------------------------------------------------------------------------------------

bool MeshComponent::Initialize() {
//	m_Array.MemZeroAndClear();
    m_Array.fill(MeshEntry());

    m_TransformComponent = GetManager()->GetComponent<TransformComponent>();
    if (!m_TransformComponent) {
        AddLog(Error, "Failed to get RectTransformComponent instance!");
        return false;
    }
    
    return true;
}

bool MeshComponent::Finalize() {
    return true;
}

void MeshComponent::Step(const Core::MoveConfig &conf) {

    size_t LastInvalidEntry = 0;
    size_t InvalidEntryCount = 0;

    for (size_t i = 0; i < m_Array.Allocated(); ++i) {//ignore root entry
        auto &item = m_Array[i];

        if (!item.m_Flags.m_Map.m_Valid) {
            //mark and continue
            LastInvalidEntry = i;
            ++InvalidEntryCount;
            continue;
        }

        if (!GetHandleTable()->IsValid(this, item.m_SelfHandle)) {
            item.m_Flags.m_Map.m_Valid = false;
            LastInvalidEntry = i;
            ++InvalidEntryCount;
            //mark and continue but set valid to false to avoid further checks
            continue;
        }

        if (!item.m_Flags.m_Map.m_Visible) {
            continue;
        }

        auto *tcentry = m_TransformComponent->GetEntry(item.m_Owner);
        if (!tcentry) {
            item.m_Flags.m_Map.m_Valid = false;
            LastInvalidEntry = i;
            ++InvalidEntryCount;
            //mark and continue but set valid to false to avoid further checks
            continue;
        }

        if (item.m_Flags.m_Map.m_MeshHandleChanged) {
            item.m_Flags.m_Map.m_MeshHandleChanged = false;
            auto rt = GetManager()->GetWorld()->GetResourceTable();
            item.m_Model = rt->GetModel(item.m_MeshHandle);

            if (!item.m_Model) {
                item.m_Model = GetDataMgr()->GetModel(item.m_ModelName);
            }

            item.m_Flags.m_Map.m_MeshValid = item.m_Model && item.m_Model->Initialize();
            if (!item.m_Flags.m_Map.m_MeshValid) {
                AddLogf(Error, "Failed to get mesh!");
            }
        }

        if (item.meshHandle.deviceHandle) {//dirty valid check
            conf.deferredSink->Mesh(tcentry->m_GlobalMatrix, item.meshHandle);
            continue;
        }

        if (!item.m_Flags.m_Map.m_MeshValid) {
            continue;
        }

        auto &vao = item.m_Model->GetVAO();
        auto r = conf.deferredSink->Begin(tcentry->m_GlobalMatrix, vao);
        for (auto &mesh : item.m_Model->GetMeshVector()) {
            r.Mesh(mesh.m_Material, mesh.NumIndices, mesh.BaseIndex, mesh.BaseVertex, vao.IndexValueType());
        }
    }

    if (InvalidEntryCount > 0) {
        AddLogf(Performance, "MeshComponent:%p InvalidEntryCount:%lu LastInvalidEntry:%lu", this, InvalidEntryCount, LastInvalidEntry);
        ReleaseElement(LastInvalidEntry);
    }
}

bool MeshComponent::Load(xml_node node, Entity Owner, Handle &hout) {	
    std::string name = node.child("Model").text().as_string(0);
    if (name.empty()) {
        AddLogf(Error, "Attempt to load nameless Model!");
        return false;
    }
    size_t index;
    if (!m_Array.Allocate(index)) {
        AddLogf(Error, "Failed to allocate index!");
        return false;
    }

    auto &entry = m_Array[index];
    entry.m_Flags.ClearAll();

    if (name.find("file://") == 0) {
        auto &mm = GetManager()->GetWorld()->GetRendererFacade()->GetResourceManager()->GetMeshManager();
        if (!mm.LoadMesh(name, entry.meshHandle)) {
            AddLogf(Error, "Mesh load failed!");
            return false;
        }
    }
    else {
        AddLog(Error, fmt::format("Invalid mesh uri: '{}'", name));
        return false;
    }

    Handle &ch = hout;
    if (!GetHandleTable()->Allocate(this, Owner, ch, index)) {
        AddLogf(Error, "Failed to allocate handle!");
        //no need to deallocate entry. It will be handled by internal garbage collecting mechanism
        return false;
    }

    entry.m_Owner = Owner;
    entry.m_SelfHandle = ch;

    entry.m_Flags.m_Map.m_Valid = true;
    entry.m_Flags.m_Map.m_MeshHandleChanged = false;
    entry.m_Flags.m_Map.m_Visible = node.child("Visible").text().as_bool(true);

    m_EntityMapper.SetHandle(entry.m_Owner, ch);

    return true;
}

void MeshComponent::ReleaseElement(size_t Index) {
    auto lastidx = m_Array.Allocated() - 1;

    if (lastidx == Index) {
        auto &last = m_Array[lastidx];
        GetHandleTable()->Release(this, last.m_SelfHandle); // handle may be already released; no need to check for failure
        last.Reset();
    } else {
        auto &last = m_Array[lastidx];
        auto &item = m_Array[Index];

        std::swap(last, item);

        if (!GetHandleTable()->SetHandleIndex(this, item.m_SelfHandle, Index)) {
            AddLogf(Error, "Failed to move MeshComponent handle index!");
        }
        GetHandleTable()->Release(this, last.m_SelfHandle); // handle may be already released; no need to check for failure
        last.Reset();
    }
    m_Array.DeallocateLast();
}

bool MeshComponent::Create(Entity Owner, Handle &hout) {
    size_t index;
    if (!m_Array.Allocate(index)) {
        AddLogf(Error, "Failed to allocate index!");
        return false;
    }

    auto &entry = m_Array[index];
    entry.m_Flags.ClearAll();

    Handle &ch = hout;
    if (!GetHandleTable()->Allocate(this, Owner, ch, index)) {
        AddLogf(Error, "Failed to allocate handle!");
        //no need to deallocate entry. It will be handled by internal garbage collecting mechanism
        return false;
    }

    entry.m_Owner = Owner;
    entry.m_SelfHandle = ch;

    entry.m_Flags.m_Map.m_Valid = true;
    entry.m_Flags.m_Map.m_MeshHandleChanged = false;
    entry.m_Flags.m_Map.m_Visible = true;

    m_EntityMapper.SetHandle(entry.m_Owner, ch);

    return true;
}

} //namespace Component 
} //namespace Renderer
} //namespace MoonGlare 

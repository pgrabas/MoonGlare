/*
  * Generated by cppsrc.sh
  * On 2016-08-10 17:23:57,17
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>

#include <Renderer/RenderInput.h>

#include "MeshComponent.h"

#include <Core/Component/ComponentManager.h>
#include <Core/Component/ComponentRegister.h>
#include <Core/Component/TransformComponent.h>

namespace MoonGlare {
namespace Renderer {
namespace Component {

::Space::RTTI::TypeInfoInitializer<MeshComponent, MeshComponentEntry> MeshComponentTypeInfo;
RegisterComponentID<MeshComponent> MeshComponentReg("Mesh", true, &MeshComponent::RegisterScriptApi);

MeshComponent::MeshComponent(ComponentManager * Owner) 
	: TemplateStandardComponent(Owner)
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
	return true;
}

bool MeshComponent::Finalize() {
	return true;
}

void MeshComponent::Step(const Core::MoveConfig &conf) {
	auto *tc = GetManager()->GetTransformComponent();
	auto *RInput = conf.m_RenderInput.get();

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

		auto *tcentry = tc->GetEntry(item.m_Owner);
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

		if (item.m_Flags.m_Map.m_MeshValid) {
			RInput->m_RenderList.emplace_back(std::make_pair(tcentry->m_GlobalMatrix, item.m_Model));
		}
	}

	if (InvalidEntryCount > 0) {
		AddLogf(Performance, "MeshComponent:%p InvalidEntryCount:%lu LastInvalidEntry:%lu", this, InvalidEntryCount, LastInvalidEntry);
		ReleaseElement(LastInvalidEntry);
	}
}

bool MeshComponent::Load(xml_node node, Entity Owner, Handle &hout) {	
	auto name = node.child("Model").text().as_string(0);
	if (!name) {
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

	entry.m_Model = GetDataMgr()->GetModel(name);
	if (entry.m_Model) {
		if (!entry.m_Model->Initialize()) {
			AddLogf(Error, "Failed to initialize model!");
		} else 
			entry.m_Flags.m_Map.m_MeshValid = true;
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
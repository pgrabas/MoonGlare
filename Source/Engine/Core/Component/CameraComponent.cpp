/*
  * Generated by cppsrc.sh
  * On 2016-08-06 22:30:28,74
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>
#include "AbstractComponent.h"
#include "ComponentManager.h"
#include "CameraComponent.h"
#include "TransformComponent.h"

namespace MoonGlare {
namespace Core {
namespace Component {

CameraComponent::CameraComponent(ComponentManager *Owner)
	: AbstractComponent(Owner)
{
}

CameraComponent::~CameraComponent() {

}

bool CameraComponent::Initialize() {
//	m_Array.MemZeroAndClear();
	return true;
}

bool CameraComponent::Finalize() {
	return true;
}

void CameraComponent::Step(const MoveConfig & conf) {
	if (m_Array.Empty()) {
		return;
	}

	const int t = sizeof(CameraEntry);

	size_t LastInvalidEntry = 0;
	size_t InvalidEntryCount = 0;

	for (size_t i = 0; i < m_Array.Allocated(); ++i) {
		auto &item = m_Array[i];
		if (!item.m_Flags.m_Map.m_Valid) {
			//mark and ignore
			LastInvalidEntry = i;
			++InvalidEntryCount;
			continue;
		}

		if (!GetHandleTable()->IsValid(item.m_Handle)) {
			AddLogf(Error, "CameraComponent: invalid entity at index %d", i);
			item.m_Flags.m_Map.m_Valid = false;
			LastInvalidEntry = i;
			++InvalidEntryCount;
			continue;
		}
		if (!item.m_Flags.m_Map.m_Active) {
			//camera is not active, continue
			continue;
		}

		//process

		conf.Camera = &item.m_Camera;
	}

	if (InvalidEntryCount > 0) {
		AddLogf(Performance, "CameraComponent:%p InvalidEntryCount:%lu LastInvalidEntry:%lu", this, InvalidEntryCount, LastInvalidEntry);
		ReleaseElement(LastInvalidEntry);
	}
}

bool CameraComponent::Load(xml_node node, Entity Owner, Handle & hout) {
	//Physics::vec3 pos;
	//if (!XML::Vector::Read(node, "Position", pos)) {
	//	return false;
	//}

	//TODO

	Handle &h = hout;
	HandleIndex index;
	if (!m_Array.Allocate(index)) {
		AddLog(Error, "Failed to allocate index");
		//no need to deallocate entry. It will be handled by internal garbage collecting mechanism
		return false;
	}

	auto &entry = m_Array[index];
	entry.m_Flags.ClearAll();
	if (!GetHandleTable()->Allocate(this, Owner, h, index)) {
		AddLog(Error, "Failed to allocate handle");
		//no need to deallocate entry. It will be handled by internal garbage collecting mechanism
		return false;
	}

	auto *tc = GetManager()->GetTransformComponent();
	if (!tc->GetInstanceHandle(Owner, entry.m_TransformHandle)) {
		AddLog(Error, "Cannot get handle to TransformComponent instance!");
		//no need to deallocate entry. It will be handled by internal garbage collecting mechanism
		return false;
	}

	entry.m_Flags.m_Map.m_Valid = true;
	entry.m_Handle = h;
	entry.m_Owner = Owner;

	return true;
}

bool CameraComponent::GetInstanceHandle(Entity Owner, Handle & hout) {
	return false;
}

void CameraComponent::ReleaseElement(size_t Index) {
	LOG_NOT_IMPLEMENTED();
}

//-------------------------------------------------------------------------------------------------

} //namespace Component 
} //namespace Core 
} //namespace MoonGlare 

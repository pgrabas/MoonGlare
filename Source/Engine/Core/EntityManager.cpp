/*
  * Generated by cppsrc.sh
  * On 2015-08-04 16:59:00,99
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#include <pch.h>
#include <MoonGlare.h>

namespace MoonGlare {
namespace Core {

EntityManager::EntityManager() {

	DebugMemorySetClassName("EntityManager");

	DebugMemoryRegisterCounter("Entities", [this](DebugMemoryCounter& counter) {
		counter.Allocated = 0;
		counter.Capacity = 0;
		counter.ElementSize = 0;
	});
}

EntityManager::~EntityManager() {
}

//------------------------------------------------------------------------------------------

bool EntityManager::Initialize() {
	return true;
}

bool EntityManager::Finalize() {
	return true;
}

//------------------------------------------------------------------------------------------

Entity EntityManager::Allocate() {
	return Entity();
}

void EntityManager::Release(Entity e) {
}

bool EntityManager::IsValid(Entity e) {
	return false;
}

#if 0

using EntityIndex = unsigned short;
using EntityGeneration = unsigned short;
using EntityIndexQueue = Utils::Memory::StaticIndexQueue<EntityIndex, StaticSettings::StaticStorage::EntityBuffer, Utils::Memory::NoLockPolicy>;
using EntityGenerationBuffer = Utils::Memory::GenerationBuffer<EntityGeneration, StaticSettings::StaticStorage::EntityBuffer>;

struct EntityManagerImpl {
	std::mutex m_Lock;
	EntityIndexQueue m_IndexQueue;
	EntityGenerationBuffer m_Generations;
};

static EntityManagerImpl EMImpl;

bool EntityManager::Initialize() {
	EntityIndex e;
	EMImpl.m_IndexQueue.get(e); //reserve index 0 for invalid entry;
	EMImpl.m_Generations.NewGeneration(e); //increment generation for invalid index;
	return true;
}

bool EntityManager::Finalize() {
	return true;
}

Entity EntityManager::Allocate() {
	LOCK_MUTEX(EMImpl.m_Lock);
	EntityIndex index;
	if (!EMImpl.m_IndexQueue.get(index)) {
		AddLogf(Error, "No more free entity indexes!");
		Entity e;
		e.m_IntegerValue = 0;
		return e;
	}
	Entity e;
	e.m_Value = index;
	e.m_Generation = EMImpl.m_Generations.Generation(index);
	return e;
}

void EntityManager::Release(Entity e) {
	LOCK_MUTEX(EMImpl.m_Lock);
	EMImpl.m_Generations.NewGeneration(e.m_Value);
	EMImpl.m_IndexQueue.push(e.m_Value);
}

#ifdef DEBUG_MEMORY
static struct EntityManagerInfo_t : Config::Debug::MemoryInterface {
	virtual Info* GetInfo() const {
		static Info i = { 0, 0, sizeof(EntityIndexQueue::Item) + sizeof(EntityGenerationBuffer::Item), EntityIndexQueue::Size, "EntityManager" };
		i.Update(EntityIndexQueue::Size - EMImpl.m_IndexQueue.count());
		return &i;
	}
} EntityManagerInfo;
#endif

#endif

} //namespace Core 
} //namespace MoonGlare 

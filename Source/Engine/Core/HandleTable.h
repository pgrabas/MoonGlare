/*
  * Generated by cppsrc.sh
  * On 2016-07-28 19:21:16,18
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef HandleTable_H
#define HandleTable_H

#include "EntityManager.h"

namespace MoonGlare {
namespace Core {

class HandleTable final 
	: public Config::Current::DebugMemoryInterface {
public:
 	HandleTable();
 	~HandleTable();

	bool Initialize(EntityManager *EntityManager);
	bool Finalize();

	using HandlePrivateData = uint64_t;

	bool IsValid(Handle h);
	bool IsValid(ComponentID cid, Handle h);
	bool Release(ComponentID cid, Handle h);
	bool Release(Handle h);
	bool Allocate(ComponentID cid, Handle &hout, HandleIndex index, HandlePrivateData value = 0);
	bool Allocate(ComponentID cid, Entity Owner, Handle &hout, HandleIndex index, HandlePrivateData value = 0);

	bool GetHandleParentEntity(ComponentID cid, Handle h, Entity &eout);
	
	//below function return false if handle or owner entity is not valid
	bool GetHandleIndex(ComponentID cid, Handle h, HandleIndex &index);
	bool SetHandleIndex(ComponentID cid, Handle h, HandleIndex index);

	bool GetHandleData(ComponentID cid, Handle h, HandlePrivateData &value);
	bool SetHandleData(ComponentID cid, Handle h, HandlePrivateData value);

	bool SwapHandleIndexes(ComponentID cid, Handle ha, Handle hb);
	bool GetOwnerCID(Handle h, ComponentID &cidout);

	bool Step(const Core::MoveConfig &config);

	//template shortcut versions for components
	template<class COMPONENT> bool GetHandleIndex(COMPONENT *c, Handle h, HandleIndex &index) { return GetHandleIndex(c->GetHandleType(), h, index); }
	template<class COMPONENT >bool Allocate(COMPONENT *c, Entity Owner, Handle &hout, HandleIndex index, HandlePrivateData value = 0) { return Allocate(c->GetHandleType(), Owner, hout, index, value); }
	template<class COMPONENT> bool IsValid(COMPONENT *c, Handle h) { return IsValid(c->GetHandleType(), h); }
	template<class COMPONENT> bool Release(COMPONENT *c, Handle h) { return Release(c->GetHandleType(), h); }
	template<class COMPONENT> bool SwapHandleIndexes(COMPONENT *c, Handle ha, Handle hb) { return SwapHandleIndexes(c->GetHandleType(), ha, hb); }
	template<class COMPONENT> bool GetHandleParentEntity(COMPONENT *c, Handle h, Entity &eout) { return GetHandleParentEntity(c->GetHandleType(), h, eout); }
	template<class COMPONENT> bool SetHandleIndex(COMPONENT *c, Handle h, HandleIndex index) { return SetHandleIndex(c->GetHandleType(), h, index); }

	union HandleFlags {
		struct {
			bool m_Valid : 1;
			bool m_HasEntityOwner : 1;
		} m_Map;
		uint8_t m_UIntValue;
	};

	struct HandleEntry {
		Entity m_Owner;
		ComponentID m_OwnerCID;
		HandleFlags m_Flags;
		char padding[7];
		HandlePrivateData m_Data;
	};

	static_assert(sizeof(HandleEntry) == 24, "HandleEntry has invalid size!");
protected:
	template<class ... ARGS>
	using GenerationsAllocator_t = Space::Memory::StaticMultiAllocator<Configuration::Handle::IndexLimit, ARGS...>;
	using Generations_t = Space::Memory::GenerationRandomBuffer<GenerationsAllocator_t, Handle>;
	template<class T> using Array = std::array<T, Configuration::Handle::IndexLimit>;

	Array<HandleEntry> m_Array;
	Array<Handle> m_HandleValueArray;
	Generations_t m_Allocator;
	EntityManager *m_EntityManager;
	size_t m_GCIndex;
};

} //namespace Core 
} //namespace MoonGlare 

#endif

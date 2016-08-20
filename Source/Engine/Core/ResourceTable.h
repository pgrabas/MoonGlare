/*
  * Generated by cppsrc.sh
  * On 2016-07-28 19:21:16,18
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef ResourceTable_H
#define ResourceTable_H

#include "EntityManager.h"

namespace MoonGlare {
namespace Core {

class ResourceTable final
	: public Config::Current::DebugMemoryInterface {
public:
	ResourceTable();
 	~ResourceTable();

	bool Initialize();
	bool Finalize();

	enum class ResourceType : uint8_t {
		Unknown = 0,
	};

//	using HandlePrivateData = uint64_t;
//
	bool IsValid(Handle h);
	bool Release(Handle h);
	bool Allocate(DataClasses::ModelPtr model, Handle &hout);
//	bool Allocate(ComponentID cid, Handle &hout, HandleIndex index, HandlePrivateData value = 0);
//	bool Allocate(ComponentID cid, Entity Owner, Handle &hout, HandleIndex index, HandlePrivateData value = 0);
//
//	bool GetHandleParentEntity(ComponentID cid, Handle h, Entity &eout);
//	
//	//below function return false if handle or owner entity is not valid
//	bool GetHandleIndex(ComponentID cid, Handle h, HandleIndex &index);
//	bool SetHandleIndex(ComponentID cid, Handle h, HandleIndex index);
//
//	bool GetHandleData(ComponentID cid, Handle h, HandlePrivateData &value);
//	bool SetHandleData(ComponentID cid, Handle h, HandlePrivateData value);
//
//	bool SwapHandleIndexes(ComponentID cid, Handle ha, Handle hb);
//
//	//template shortcut versions for components
//	template<class COMPONENT> bool GetHandleIndex(COMPONENT *c, Handle h, HandleIndex &index) { return GetHandleIndex(c->GetHandleType(), h, index); }
//	template<class COMPONENT> bool Allocate(COMPONENT *c, Entity Owner, Handle &hout, HandleIndex index, HandlePrivateData value = 0) { return Allocate(c->GetHandleType(), Owner, hout, index, value); }
//	template<class COMPONENT> bool IsValid(COMPONENT *c, Handle h) { return IsValid(c->GetHandleType(), h); }
//	template<class COMPONENT> bool Release(COMPONENT *c, Handle h) { return Release(c->GetHandleType(), h); }
//	template<class COMPONENT> bool SwapHandleIndexes(COMPONENT *c, Handle ha, Handle hb) { return SwapHandleIndexes(c->GetHandleType(), ha, hb); }
//	template<class COMPONENT> bool GetHandleParentEntity(COMPONENT *c, Handle h, Entity &eout) { return GetHandleParentEntity(c->GetHandleType(), h, eout); }

	bool Step(const Core::MoveConfig &config);

	DataClasses::ModelPtr GetModel(Handle h);

	union ResourceFlags {
		struct {
			bool m_Valid : 1;
		} m_Map;
		uint8_t m_UIntValue;
	};

	struct ResourceEntry {
		//Entity m_Owner;
		//ComponentID m_OwnerCID;
		//HandlePrivateData m_Data;
		ResourceFlags m_Flags;
		char padding1[7];
		DataClasses::ModelPtr m_Model;
		//filehash
	};

	static_assert((sizeof(ResourceEntry) % 16) == 0, "ResourceEntry has invalid size!");
protected:
	template<class ... ARGS>
	using GenerationsAllocator_t = Space::Memory::StaticMultiAllocator<Configuration::Handle::IndexLimit, ARGS...>;
	using Generations_t = Space::Memory::GenerationRandomBuffer<GenerationsAllocator_t, Handle>;
	template<class T> using Array = std::array<T, Configuration::Handle::IndexLimit>;

	Array<ResourceEntry> m_Array;
	Generations_t m_Allocator;
};

} //namespace Core 
} //namespace MoonGlare 

#endif

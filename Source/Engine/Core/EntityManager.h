/*
  * Generated by cppsrc.sh
  * On 2015-08-04 16:59:00,99
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef EntityManager_H
#define EntityManager_H

namespace MoonGlare {
namespace Core {

class EntityManager final 
	: Space::RTTI::RTTIObject
	, public Config::Current::DebugMemoryInterface {
SPACERTTI_DECLARE_STATIC_CLASS(EntityManager, Space::RTTI::RTTIObject);
public:
	template<class T> using Array = std::array<T, Configuration::Entity::IndexLimit>;
	template<class ... ARGS>
	using GenerationsAllocator_t = Space::Memory::StaticMultiAllocator<Configuration::Entity::IndexLimit, ARGS...>;
	using Generations_t = Space::Memory::GenerationRandomAllocator<GenerationsAllocator_t, Entity>;

	EntityManager();
	~EntityManager();

	bool Initialize();
	bool Finalize();

	Entity GetRootEntity() { return m_Root; }

	bool Allocate(Entity &eout);
	bool Allocate(Entity parent, Entity &eout);
	bool Release(Entity entity);

	bool GetParent(Entity entity, Entity &ParentOut) const;
	bool IsValid(Entity entity) const;

	bool Step(const Core::MoveConfig &config);

	union EntityFlags {
		struct {
			bool m_Valid : 1;
		} m_Map;
		uint8_t m_UIntValue;
	};

	static void RegisterScriptApi(ApiInitializer &root);
private: 
	Array<Entity> m_Parent;
	Array<EntityFlags> m_Flags;
	Array<Entity> m_EntityValues;
	Generations_t m_Allocator;
	Entity m_Root;
	size_t m_GCIndex;
};

template<class T>
struct BaseEntityMapper {
	 using Array = EntityManager::Array<T>;

	const T& GetHandle(Entity e) const {
		return m_Array[e.GetIndex()];
	}

	void SetHandle(Entity e, const T &h) {
		m_Array[e.GetIndex()] = h;
	}
protected:
	Array m_Array;
};

using EntityMapper = BaseEntityMapper < Handle >;

} //namespace Core 
} //namespace MoonGlare 

#endif

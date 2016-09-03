/*
  * Generated by cppsrc.sh
  * On 2015-10-25 21:57:04,09
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef TransformComponent_H
#define TransformComponent_H

#include <libSpace/src/Container/StaticVector.h>

#include "AbstractComponent.h"

namespace MoonGlare {
namespace Core {
namespace Component {

class TransformComponent 
	: public AbstractComponent
	, public ComponentIDWrap<ComponentIDs::Transform> {
public:
	TransformComponent(ComponentManager *Owner);
	virtual ~TransformComponent();
	virtual bool Initialize() override;
	virtual bool Finalize() override;
	virtual void Step(const MoveConfig &conf) override;
	virtual bool PushEntryToLua(Handle h, lua_State *lua, int &luarets) override;
	virtual bool Load(xml_node node, Entity Owner, Handle &hout) override;
	virtual bool GetInstanceHandle(Entity Owner, Handle &hout) override;

	union FlagsMap {
		struct MapBits_t {
			bool m_Valid : 1; //Entity is not valid or requested to be deleted;
			bool m_Dirty : 1;
		};
		MapBits_t m_Map;
		uint32_t m_UintValue;

		void SetAll() { m_UintValue = 0; m_UintValue = ~m_UintValue; }
		void ClearAll() { m_UintValue = 0; }

		static_assert(sizeof(MapBits_t) <= sizeof(decltype(m_UintValue)), "Invalid Function map elements size!");
	};

	struct TransformEntry {
		Handle m_SelfHandle;
		Entity m_OwnerEntity;
		FlagsMap m_Flags;
		math::mat4 m_LocalMatrix;
		math::mat4 m_GlobalMatrix;
		Physics::vec3 m_LocalScale;
		Physics::vec3 m_GlobalScale;

		math::Transform m_LocalTransform;
		math::Transform m_GlobalTransform;
//		Physics::Transform m_CenterOfMass;

		Configuration::RuntimeRevision m_Revision;

		void Reset() {}
		void Recalculate(const TransformEntry *Parent);

		math::vec3 GetPosition() const { return convert(m_LocalTransform.getOrigin()); }
		void SetPosition(math::vec3 pos) { 
			m_LocalTransform.setOrigin(convert(pos)); 
			m_Revision = 0;
			m_Flags.m_Map.m_Dirty = true;
		}
		math::vec4 GetRotation() const { return convert(m_LocalTransform.getRotation()); }
		void SetRotation(math::vec4 rot) { 
			m_LocalTransform.setRotation(convert(rot));
			m_Revision = 0;
		}
		math::vec3 GetScale() const { return convert(m_LocalScale); }
		void SetScale(math::vec3 s) { 
			m_LocalScale = convert(s);
			m_Revision = 0;
			m_Flags.m_Map.m_Dirty = true;
		}

		math::Transform GetTransform() const { return m_LocalTransform; }
		void SetTransform(const math::Transform &s) {
			m_LocalTransform = s;
			m_Revision = 0;
			m_Flags.m_Map.m_Dirty = true;
		}
	};

	static_assert((sizeof(TransformEntry) % 16) == 0, "TransformEntry has invalid size");

	TransformEntry* GetEntry(Handle h);	 //return nullptr if h/e is not valid
	TransformEntry* GetEntry(Entity e);	 //return nullptr if h/e is not valid

	Configuration::RuntimeRevision GetCurrentRevision() const { return m_CurrentRevision; }
	static void RegisterScriptApi(ApiInitializer &root);
protected:
	template<class T> using Array = Space::Container::StaticVector<T, Configuration::Storage::ComponentBuffer>;
	Array<TransformEntry> m_Array;
	EntityMapper m_EntityMapper;
	Configuration::RuntimeRevision m_CurrentRevision;
	void ReleaseElement(size_t Index);
};

} //namespace Component 
} //namespace Core 
} //namespace MoonGlare 

#endif

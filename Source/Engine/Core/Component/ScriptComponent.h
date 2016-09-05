/*
  * Generated by cppsrc.sh
  * On 2016-07-23 16:55:00,64
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef ScriptComponent_H
#define ScriptComponent_H

#include <libSpace/src/Container/StaticVector.h>

namespace MoonGlare {
namespace Core {
namespace Component {

class ScriptComponent
	: public AbstractComponent
	, public ComponentIDWrap<ComponentIDs::Script> {
public:
 	ScriptComponent(ComponentManager *Owner);
 	virtual ~ScriptComponent();
	virtual bool Initialize() override;
	virtual bool Finalize() override;
	virtual void Step(const MoveConfig &conf) override;
	virtual bool Load(xml_node node, Entity Owner, Handle &hout) override;
	virtual bool GetInstanceHandle(Entity Owner, Handle &hout) override;

	using LuaHandle = int;

	union FlagsMap {
		struct MapBits_t {
			bool m_Valid : 1; //Entity is not valid or requested to be deleted;
			bool m_Active : 1; //Script has step function and it shall be called

			//bool m_OnStart : 1;
			//bool m_OnStop : 1;
		};
		MapBits_t m_Map;
		uint32_t m_UintValue;

		void SetAll() { m_UintValue = 0; m_UintValue = ~m_UintValue; }
		void ClearAll() { m_UintValue = 0; }

		static_assert(sizeof(MapBits_t) <= sizeof(decltype(m_UintValue)), "Invalid Function map elements size!");
	};

	struct ScriptEntry {
		FlagsMap m_Flags;
		Entity m_Owner;	
		Handle m_SelfHandle;
		uint32_t padding;

		void Reset() {
			m_Flags.m_Map.m_Valid = false;
		}
	};
	static_assert((sizeof(ScriptEntry) % 8) == 0, "Invalid ScriptEntry size!");
	static_assert(std::is_pod<ScriptEntry>::value, "ScriptEntry must be pod!");
protected:
	Scripts::ScriptEngine *m_ScriptEngine;

	template<class T> using Array = Space::Container::StaticVector<T, Configuration::Storage::ComponentBuffer>;
	Array<ScriptEntry> m_Array;
	//BaseEntityMapper<LuaHandle> m_EntityMapper;

	void ReleaseComponent(lua_State *lua, size_t Index);

	void GetInstancesTable(lua_State *lua) {
		lua_pushlightuserdata(lua, GetInstancesTableIndex());
		lua_gettable(lua, LUA_REGISTRYINDEX);
	}
	void GetGameObjectMetaTable(lua_State *lua) {
		lua_pushlightuserdata(lua, GetGameObjectMetaTableIndex());
		lua_gettable(lua, LUA_REGISTRYINDEX);
	}

	void *GetInstancesTableIndex() { return this; }
	void *GetGameObjectMetaTableIndex() { return reinterpret_cast<void*>(reinterpret_cast<int>(this) + 1); }
private:
	//utils
	static int lua_MakeComponentInfo(lua_State *lua, ComponentID cid, Handle h, AbstractComponent *cptr);
	static int lua_DereferenceHandle(lua_State *lua);

//ScriptComponent api
	static int lua_DestroyComponent(lua_State *lua);
	static int lua_GetComponent(lua_State *lua);

//GameObject api
	static int lua_CreateComponent(lua_State *lua);
	static int lua_SpawnChild(lua_State *lua);
	static int lua_DestroyObject(lua_State *lua);
	static int lua_Destroy(lua_State *lua);
};

} //namespace Component 
} //namespace Core 
} //namespace MoonGlare 

#endif

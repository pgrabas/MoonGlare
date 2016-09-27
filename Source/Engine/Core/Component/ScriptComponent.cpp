/*
  * Generated by cppsrc.sh
  * On 2016-07-23 16:55:00,64
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>
#include "AbstractComponent.h"
#include "ScriptComponent.h"
#include "TransformComponent.h"
#include "ComponentManager.h"
#include "ComponentRegister.h"
#include <Core/EntityBuilder.h>

#include <Utils/LuaUtils.h>

namespace MoonGlare {
namespace Core {
namespace Component {

static_assert(sizeof(void*) == sizeof(Handle), "Size of void* must match the size of Handle!");

namespace lua {
	enum {
		SelfPtrUpValue = 1,
		HandleUpValue,
	};

	static const char *Function_Step = "Step";
	static const char *Function_OnCreate = "OnCreate";
	static const char *Function_OnDestroy = "OnDestroy";
	static const char *Function_PerSecond = "PerSecond";

	static const char *EntityMemberName = "Entity";
	static const char *HandleMemberName = "Handle";
	static const char *ComponentIDMemberName = "ComponentId";
	static const char *DereferenceHandleName = "Get";
	static const char *ComponentSetStateName = "Set";

	static const char *GameObjectName = "GameObject";

	static const char *CreateComponentName = "CreateComponent";
	static const char *SpawnChildName = "SpawnChild";
	static const char *DestroyObjectName = "DestroyObject";
	static const char *DestroyName = "Destroy";
	static const char *GetName = "GetName";
	static const char *SetName = "SetName";
	static const char *FindChild = "FindChild";

	static const char *SetPerSecond = "SetPerSecond";
	static const char *SetStep = "SetStep";
	static const char *SetActive = "SetActive";
	
	bool Lua_SafeCall(lua_State *lua, int args, int rets, const char *CaleeName) {
		try {
			AddLogf(ScriptCall, "Call to %s", CaleeName);
			lua_call(lua, args, rets);
			return true;
		}
		catch (Core::Scripts::eLuaPanic &err) {
			AddLogf(Error, "Failure during call to %s message: %s", CaleeName, err.what());
			return false;
		}
	}

	template<typename T>
	void LuaSetField(lua_State *lua, T t, const char *Name, int index) {
		Utils::Scripts::Lua_push(lua, t);
		lua_setfield(lua, index - 1, Name); // -1 -> to include pushed value
	}
}

::Space::RTTI::TypeInfoInitializer<ScriptComponent, ScriptComponent::ScriptEntry> ScriptComponentTypeInfo;
RegisterComponentID<ScriptComponent> ScriptComponent("Script", false);

ScriptComponent::ScriptComponent(ComponentManager *Owner)
	: AbstractComponent(Owner) {

	DebugMemorySetClassName("ScriptComponent");
	DebugMemoryRegisterCounter("IndexUsage", [this](DebugMemoryCounter& counter) {
		counter.Allocated = m_Array.Allocated();
		counter.Capacity = m_Array.Capacity();
		counter.ElementSize = sizeof(ScriptEntry);
	});
}

ScriptComponent::~ScriptComponent() {
}

bool ScriptComponent::Initialize() {
	m_ScriptEngine = GetManager()->GetWorld()->GetScriptEngine();
	THROW_ASSERT(m_ScriptEngine, "No script engine instance!");

	m_Array.MemZeroAndClear();

	auto lua = m_ScriptEngine->GetLua();
	LOCK_MUTEX_NAMED(m_ScriptEngine->GetLuaMutex(), lock);

	Utils::Scripts::LuaStackOverflowAssert check(lua);

	lua_pushlightuserdata(lua, GetInstancesTableIndex());									 //stack: InstTblIdx
	lua_createtable(lua, m_Array.Capacity(), 0);											 //stack: InstTblIdx InstTbl
	MoonGlare::Core::Scripts::PublishSelfLuaTable(lua, "ScriptComponent", this, -1);		 //stack: InstTblIdx InstTbl
	lua_settable(lua, LUA_REGISTRYINDEX);													 //stack: -

	lua_pushlightuserdata(lua, GetObjectRootTableIndex());									 //stack: Index  
	lua_createtable(lua, 0, 0);																 //stack: Index ORTable
	MoonGlare::Core::Scripts::PublishSelfLuaTable(lua, "ScriptComponent_OR", this, -1);		 //stack: Index ORTable
	lua_settable(lua, LUA_REGISTRYINDEX);													 //stack: -

	lua_createtable(lua, 0, 0);																 //stack: GameObjectMT
	MoonGlare::Core::Scripts::PublishSelfLuaTable(lua, "ScriptComponent_mt", this, -1);		 //stack: GameObjectMT
	lua_pushlightuserdata(lua, GetGameObjectMetaTableIndex());								 //stack: GameObjectMT Index
	lua_pushvalue(lua, -2);																	 //stack: GameObjectMT Index GameObjectMT
	lua_settable(lua, LUA_REGISTRYINDEX);													 //stack: GameObjectMT

	lua_createtable(lua, 0, 0);																 //stack: GameObjectMT GameObjectMT_index
	lua_pushvalue(lua, -1);																	 //stack: GameObjectMT GameObjectMT_index GameObjectMT_index
	lua_setfield(lua, -3, "__index");														 //stack: GameObjectMT GameObjectMT_index

	lua_pushlightuserdata(lua, this);														 //stack: GameObjectMT GameObjectMT_index this
	lua_pushcclosure(lua, &lua_CreateComponent, 1);											 //stack: GameObjectMT GameObjectMT_index this lua_CreateComponent
	lua_setfield(lua, -2, lua::CreateComponentName);										 //stack: GameObjectMT GameObjectMT_index 

	lua_pushlightuserdata(lua, GetManager());												 //stack: GameObjectMT GameObjectMT_index Manager
	lua_pushcclosure(lua, &lua_SpawnChild, 1);												 //stack: GameObjectMT GameObjectMT_index Manager lua_SpawnChild
	lua_setfield(lua, -2, lua::SpawnChildName);												 //stack: GameObjectMT GameObjectMT_index 

	lua_pushlightuserdata(lua, GetManager());												 //stack: GameObjectMT GameObjectMT_index Manager
	lua_pushcclosure(lua, &lua_DestroyObject, 1);											 //stack: GameObjectMT GameObjectMT_index Manager lua_DestroyObject
	lua_setfield(lua, -2, lua::DestroyObjectName);											 //stack: GameObjectMT GameObjectMT_index 

	lua_pushlightuserdata(lua, GetManager()->GetWorld());									 //stack: GameObjectMT GameObjectMT_index Manager
	lua_pushcclosure(lua, &lua_SetName, 1);													 //stack: GameObjectMT GameObjectMT_index Manager lua_SetName
	lua_setfield(lua, -2, lua::SetName);													 //stack: GameObjectMT GameObjectMT_index 

	lua_pushlightuserdata(lua, GetManager()->GetWorld());									 //stack: GameObjectMT GameObjectMT_index Manager
	lua_pushcclosure(lua, &lua_GetName, 1);													 //stack: GameObjectMT GameObjectMT_index Manager lua_GetName
	lua_setfield(lua, -2, lua::GetName);													 //stack: GameObjectMT GameObjectMT_index 

	lua_pushlightuserdata(lua, this);														 //stack: GameObjectMT GameObjectMT_index this
	lua_pushcclosure(lua, &lua_FindChild, 1);												 //stack: GameObjectMT GameObjectMT_index this lua_FindChild
	lua_setfield(lua, -2, lua::FindChild);													 //stack: GameObjectMT GameObjectMT_inde

	Utils::Scripts::lua_PushCClosure(lua, &lua_Destroy, (void*)GetManager()->GetWorld());	 //stack: GameObjectMT GameObjectMT_index closure
	lua_setfield(lua, -2, lua::DestroyName);												 //stack: GameObjectMT GameObjectMT_index 

	lua_pop(lua, 2);																		 //stack: -

	return true;
}

bool ScriptComponent::Finalize() {
	auto lua = m_ScriptEngine->GetLua();
	LOCK_MUTEX_NAMED(m_ScriptEngine->GetLuaMutex(), lock);
	MoonGlare::Core::Scripts::HideSelfLuaTable(lua, "ScriptComponent", this);
	lua_pushlightuserdata(lua, (void *)this);
	lua_pushnil(lua);
	lua_settable(lua, LUA_REGISTRYINDEX);
	return true;
}

void ScriptComponent::Step(const MoveConfig & conf) {
	if (m_Array.Empty()) {
		return;
	}

	auto lua = m_ScriptEngine->GetLua();
	LOCK_MUTEX_NAMED(m_ScriptEngine->GetLuaMutex(), lock);
	Utils::Scripts::LuaStackOverflowAssert check(lua);
	//stack: -		
	GetInstancesTable(lua);									//stack: self

	luabridge::Stack<const MoveConfig*>::push(lua, &conf);  //stack: self movedata

	size_t LastInvalidEntry = 0;
	size_t InvalidEntryCount = 0;

	int luatop = lua_gettop(lua);

	for (size_t i = 0; i < m_Array.Allocated(); ++i) {
		auto &item = m_Array[i];
		if (!item.m_Flags.m_Map.m_Valid) {
			//mark and ignore
			LastInvalidEntry = i;
			++InvalidEntryCount;
			continue;
		}

		if (!GetHandleTable()->IsValid(this, item.m_SelfHandle)) {
			item.m_Flags.m_Map.m_Valid = false;
			LastInvalidEntry = i;
			++InvalidEntryCount;
			continue;
		}

		if (!item.m_Flags.m_Map.m_Active) {
			continue;
		}

		lua_rawgeti(lua, -2, i + 1);		//stack: self movedata Script/nil

		if (!lua_istable(lua, -1)) {
			lua_settop(lua, luatop);
			AddLogf(Error, "ScriptComponent: nil in lua script table at index: %d", i + 1);
			continue;
		}

		int luascrtop = lua_gettop(lua);
			
		if (item.m_Flags.m_Map.m_Step) {
			lua_pushvalue(lua, -1);								// stack: self movedata script script
			lua_getfield(lua, -1, lua::Function_Step);			//stack: self movedata script script Step/nil
			if (lua_isnil(lua, -1)) {
				item.m_Flags.m_Map.m_Step = false;
				AddLogf(Warning, "ScriptComponent: There is no Step function in component at index: %d, disabling", i);
			} else {
				//stack: self movedata script script Step
				lua_insert(lua, -2);							//stack: self movedata script Step script
				lua_pushvalue(lua, -4);							//stack: self movedata script Step script movedata

				if (!lua::Lua_SafeCall(lua, 2, 0, lua::Function_Step)) {
					AddLogf(Error, "Failure during OnStep call for component #%lu", i);
				}
			}
		}

		if (item.m_Flags.m_Map.m_OnPerSecond && conf.m_SecondPeriod) {
			lua_settop(lua, luascrtop);							//stack: self movedata script 
			lua_pushvalue(lua, -1);								//stack: self movedata script script
			lua_getfield(lua, -1, lua::Function_PerSecond);		//stack: self movedata script script persec/nil
			if (lua_isnil(lua, -1)) {
				item.m_Flags.m_Map.m_OnPerSecond = false;
				AddLogf(Warning, "ScriptComponent: There is no PerSecond function in component at index: %d, disabling", i);
			} else {
				//stack: self movedata script script persec
				lua_insert(lua, -2);						//stack: self movedata script persec script 
				if (!lua::Lua_SafeCall(lua, 1, 0, lua::Function_Step)) {
					AddLogf(Error, "Failure during PerSecond call for component #%lu", i);
				}
			}
		}

		lua_settop(lua, luatop);
	}

	lua_pop(lua, 2); //stack: -

	if (InvalidEntryCount > 0) {
		AddLogf(Performance, "ScriptComponent:%p InvalidEntryCount:%lu LastInvalidEntry:%lu", this, InvalidEntryCount, LastInvalidEntry);
		ReleaseComponent(lua, LastInvalidEntry);
	}
}

void ScriptComponent::ReleaseComponent(lua_State *lua, size_t Index) {
	Utils::Scripts::LuaStackOverflowAssert check(lua);

	auto lastidx = m_Array.Allocated() - 1;
	auto LuaIndex = Index + 1;				//lua index
	auto LuaLast = m_Array.Allocated();		//lua index
		
	GetInstancesTable(lua);						//stack: ScriptComponentInstances

	lua_pushinteger(lua, LuaIndex);				//stack: ScriptComponentInstances LuaIndex
	lua_gettable(lua, -2);						//stack: ScriptComponentInstances current_table

	lua_rawgeti(lua, -1, 1);					//stack: ScriptComponentInstances current_table script@1/nil
	if (lua_isnil(lua, -1)) {
		AddLogf(Error, "ScriptComponent: nil in lua Object script table at index: 1");
	} else {
		lua_getfield(lua, -1, lua::Function_OnDestroy); //stack: self current_table script@1 OnDestroy/nil
		if (lua_isnil(lua, -1)) {
			lua_pop(lua, 1);
		} else {
			lua_pushvalue(lua, -2);				//stack: self current_table script@1 OnDestroy script@1
			if (!lua::Lua_SafeCall(lua, 1, 0, lua::Function_OnDestroy)) { //stack: self current_table script@1
				//	nothing there, nothing more to be logged
			}
		}
	}
	lua_pop(lua, 1);							//stack: ScriptComponentInstances current_table

	ScriptEntry *entry;
	if (lastidx == Index) {
		entry = &m_Array[lastidx];
	} else {
		auto &last = m_Array[lastidx];
		auto &item = m_Array[Index];
		entry = &last;

		std::swap(last, item);

		//set last in place of current and nil last
		lua_pushinteger(lua, LuaLast);			//stack: ScriptComponentInstances current_table LuaLast
		lua_gettable(lua, -3);					//stack: ScriptComponentInstances current_table last_table
		lua_pushinteger(lua, LuaIndex);			//stack: ScriptComponentInstances current_table last_table current_id
		lua_insert(lua, -2);					//stack: ScriptComponentInstances current_table LuaIndex last_table 
		lua_settable(lua, -4);					//stack: ScriptComponentInstances current_table 

		if (!GetHandleTable()->SetHandleIndex(this, item.m_SelfHandle, Index)) {
			AddLogf(Error, "Failed to move ScriptComponent handle index!");
		}
	}

	//set last in place of current and nil last
	lua_pushinteger(lua, LuaLast);			//stack: ScriptComponentInstances current_table LuaLast
	lua_pushnil(lua);						//stack: ScriptComponentInstances current_table LuaLast nil
	lua_settable(lua, -4);					//stack: ScriptComponentInstances current_table 

	lua_pop(lua, 2); //stack: -

	entry->Reset();
	GetHandleTable()->Release(this, entry->m_SelfHandle); // handle may be already released; no need to check for failure
	m_Array.DeallocateLast();
}

//-------------------------------------------------------------------------------------------------

bool ScriptComponent::Load(xml_node node, Entity Owner, Handle &hout) {
	auto name = node.child("Script").text().as_string(0);
	if (!name) {
		AddLogf(Error, "Attempt to load nameless script!");
		return false;
	}

	Handle &ch = hout;
	size_t index;
	if (!m_Array.Allocate(index)) {
		AddLogf(Error, "Failed to allocate index!");
		return false;
	}

	auto &entry = m_Array[index];
	entry.m_Flags.ClearAll();

	if (!GetHandleTable()->Allocate(this, Owner, ch, index)) {
		AddLogf(Error, "Failed to allocate handle!");
		//no need to deallocate entry. It will be handled by internal garbage collecting mechanism
		return false;
	}

	auto lua = m_ScriptEngine->GetLua();
	LOCK_MUTEX_NAMED(m_ScriptEngine->GetLuaMutex(), lock);
	Utils::Scripts::LuaStackOverflowAssert check(lua);

	int top = lua_gettop(lua);
	//stack: -

	if (!m_ScriptEngine->GetRegisteredScript(name)) {
		AddLogf(Error, "There is no such script: %s", name);
		GetHandleTable()->Release(this, ch);
		//no need to deallocate entry. It will be handled by internal garbage collecting mechanism
		return false;
	}

	//stack: ScriptClass

	entry.m_Owner = Owner;
	entry.m_SelfHandle = ch;
	entry.m_Flags.m_Map.m_Active = true;
	entry.m_Flags.m_Map.m_Valid = true;
	entry.m_Flags.m_Map.m_Step = true;

	if (!GetObjectRootInstance(lua, Owner)) {
		AddLogf(Error, "CRITICAL INTERNAL ERROR!");
		throw "CRITICAL ERROR. CANNOT CONTINUE!";
	}

	//stack: ScriptClass ObjectRoot GameObject 

	lua_createtable(lua, 0, 0);									//stack: ScriptClass ObjectRoot GameObject Script
	lua_insert(lua, -2);										//stack: ScriptClass ObjectRoot Script GameObject
	lua_setfield(lua, -2, lua::GameObjectName);					//stack: ScriptClass ObjectRoot Script
	lua_pushvalue(lua, -1);										//stack: ScriptClass ObjectRoot Script Script
	lua_rawseti(lua, -3, 1);									//stack: ScriptClass ObjectRoot Script

	GetInstancesTable(lua);										//stack: ScriptClass ObjectRoot Script InstTable
	lua_pushvalue(lua, -2);										//stack: ScriptClass ObjectRoot Script InstTable Script
	lua_rawseti(lua, -2, static_cast<int>(index) + 1);			//stack: ScriptClass ObjectRoot Script InstTable
	lua_pop(lua, 1);											//stack: ScriptClass ObjectRoot Script
		
	lua_insert(lua, -3);										//stack: Script ScriptClass ObjectRoot 
	lua_insert(lua, -3);										//stack: ObjectRoot Script ScriptClass  
	lua_setmetatable(lua, -2);									//stack: ObjectRoot Script 

	lua_pushlightuserdata(lua, Owner.GetVoidPtr());				//stack: ObjectRoot Script Entity
	lua_setfield(lua, -2, lua::EntityMemberName);				//stack: ObjectRoot Script
	lua_pushlightuserdata(lua, ch.GetVoidPtr());				//stack: ObjectRoot Script SelfHandle
	lua_setfield(lua, -2, lua::HandleMemberName);				//stack: ObjectRoot Script

	lua_pushlightuserdata(lua, this);							//stack: ObjectRoot Script this
	lua_pushlightuserdata(lua, ch.GetVoidPtr());				//stack: ObjectRoot Script this SelfHandle 
	lua_pushcclosure(lua, &lua_DestroyComponent, 2);			//stack: ObjectRoot Script lua_DestroyComponent
	lua_setfield(lua, -2, "DestroyComponent");					//stack: ObjectRoot Script
	//TODO: DestroyObject(void/other)

	lua_pushlightuserdata(lua, this);							//stack: ObjectRoot Script this
	lua_pushcclosure(lua, &lua_SetPerSecond, 1);				//stack: ObjectRoot Script lua_SetPerSecond
	lua_setfield(lua, -2, lua::SetPerSecond);					//stack: ObjectRoot Script

	lua_pushlightuserdata(lua, this);							//stack: ObjectRoot Script this
	lua_pushcclosure(lua, &lua_SetStep, 1);						//stack: ObjectRoot Script lua_SetStep
	lua_setfield(lua, -2, lua::SetStep);						//stack: ObjectRoot Script

	lua_pushlightuserdata(lua, this);							//stack: ObjectRoot Script this
	lua_pushcclosure(lua, &lua_SetActive, 1);					//stack: ObjectRoot Script lua_SetActive
	lua_setfield(lua, -2, lua::SetActive);						//stack: ObjectRoot Script


	lua_pushlightuserdata(lua, this);							//stack: ObjectRoot Script this
	lua_pushlightuserdata(lua, ch.GetVoidPtr());				//stack: ObjectRoot Script this SelfHandle 
	lua_pushcclosure(lua, &lua_GetComponent, 2);				//stack: ObjectRoot Script lua_GetComponent
	lua_pushvalue(lua, -1);										//stack: ObjectRoot Script lua_GetComponent lua_GetComponent
	lua_setfield(lua, -3, "GetComponent");						//stack: ObjectRoot Script lua_GetComponent

	lua_pushvalue(lua, -2);										//stack: ObjectRoot Script lua_GetComponent Script
	lua_pushnumber(lua, (float)ComponentIDs::Transform);		//stack: ObjectRoot Script lua_GetComponent Script TransformCID
	lua_call(lua, 2, 1);										//stack: ObjectRoot Script TransformInfo
	lua_setfield(lua, -2, "Transform");							//stack: ObjectRoot Script 
	
	//TODO: DestroyObject(void/other)

	lua_getfield(lua, -1, lua::Function_OnCreate);				//stack: ObjectRoot Script OnCreate/nil
	if (lua_isnil(lua, -1)) {
		lua_pop(lua, 1);										//stack: ObjectRoot Script
	} else {
		lua_pushvalue(lua, -2);									//stack: ObjectRoot Script OnCreate Script
		if (!lua::Lua_SafeCall(lua, 1, 0, lua::Function_OnCreate)) {
			//no need for more logging
		}
	}

	//stack: ObjectRoot Script

	lua_settop(lua, top);
	//stack: -
	return true;
}

bool ScriptComponent::GetInstanceHandle(Entity Owner, Handle &hout) {
	LOG_NOT_IMPLEMENTED();
	return false;
	//TODO
}

//-------------------------------------------------------------------------------------------------

bool ExtractHandleFromArgument(lua_State *lua, int location, Handle &h, bool AllowSelfHandle = true) {
	int type = lua_type(lua, location);
	switch (type) {
	case LUA_TNIL:
		if (!AllowSelfHandle) {//TODO
			throw "!AllowSelfHandle";
		}
		h = Handle::FromVoidPtr(lua_touserdata(lua, lua_upvalueindex(lua::HandleUpValue)));
		break;
	case LUA_TLIGHTUSERDATA:
		h = Handle::FromVoidPtr(lua_touserdata(lua, location));
		break;
	case LUA_TTABLE:
		lua_getfield(lua, location, lua::HandleMemberName);
		if (!lua_islightuserdata(lua, -1)) {
			lua_pop(lua, 1);
			return false;
		}
		h = Handle::FromVoidPtr(lua_touserdata(lua, -1));
		lua_pop(lua, 1);
		break;
	default:
		return false;
	}
	return true;
}

//-------------------------------------------------------------------------------------------------

bool ScriptComponent::GetObjectRootInstance(lua_State *lua, Entity Owner) {
	Utils::Scripts::LuaStackOverflowAssert check(lua);

	GetObjectRootTable(lua);												//stack: ORTable
	lua_rawgeti(lua, -1, static_cast<int>(Owner.GetIndex()) + 1);			//stack: ORTable OR?
	
	if (lua_istable(lua, -1)) {												//stack: ORTable OR
		lua_insert(lua, -2);												//stack: OR ORTable
		lua_pop(lua, 1);													//stack: OR
		lua_getfield(lua, -1, lua::GameObjectName);							//stack: OR GO
		check.ReturnArgs(2);
		return true;
	}
																			//stack: ORTable nil
	lua_pop(lua, 1);														//stack: ORTable

	lua_createtable(lua, 0, 0);												//stack: ORTable OR		
	lua_pushvalue(lua, -1);													//stack: ORTable OR OR
	lua_rawseti(lua, -3, static_cast<int>(Owner.GetIndex()) + 1);			//stack: ORTable OR

	lua_insert(lua, -2);													//stack: OR ORTable
	lua_pop(lua, 1);														//stack: OR 

	lua_createtable(lua, 0, 0);												//stack: OR GO
	lua_pushvalue(lua, -1);													//stack: OR GO GO
	lua_setfield(lua, -3, lua::GameObjectName);								//stack: OR GO

	lua::LuaSetField(lua, Owner.GetVoidPtr(), lua::EntityMemberName, -1);	
	lua::LuaSetField(lua, Owner.GetVoidPtr(), lua::EntityMemberName, -2);	

	GetGameObjectMetaTable(lua);											//stack: OR GO GO_MT
	lua_setmetatable(lua, -2);												//stack: OR GO

	check.ReturnArgs(2);
	return true;
}

bool ScriptComponent::InvalidateObjectRoot(lua_State *lua, Entity Owner) {
	Utils::Scripts::LuaStackOverflowAssert check(lua);
	GetObjectRootTable(lua);												//stack: ORTable
		
	lua_pushnil(lua);														//stack: ORTable nil
	lua_rawseti(lua, -3, static_cast<int>(Owner.GetIndex()) + 1);			//stack: ORTable
	lua_pop(lua, 1);

	return true;
}

//-------------------------------------------------------------------------------------------------

int ScriptComponent::lua_MakeComponentInfo(lua_State *lua, ComponentID cid, Handle h, AbstractComponent *cptr) {
	lua_createtable(lua, 0, 5);

	lua_pushlightuserdata(lua, h.GetVoidPtr());
	lua_setfield(lua, -2, lua::HandleMemberName);

	lua_pushinteger(lua, static_cast<int>(cid));
	lua_setfield(lua, -2, lua::ComponentIDMemberName);

	lua_pushlightuserdata(lua, cptr);
	lua_pushlightuserdata(lua, h.GetVoidPtr());
	lua_pushcclosure(lua, &lua_DereferenceHandle, 2);
	lua_setfield(lua, -2, lua::DereferenceHandleName);

	lua_pushlightuserdata(lua, cptr);
	lua_pushlightuserdata(lua, h.GetVoidPtr());
	lua_pushcclosure(lua, &lua_SetComponentState, 2);
	lua_setfield(lua, -2, lua::ComponentSetStateName);
	return 1;
}

int ScriptComponent::lua_DereferenceHandle(lua_State *lua) {
	void *voidcptr = lua_touserdata(lua, lua_upvalueindex(lua::SelfPtrUpValue));
	AbstractComponent *cptr = reinterpret_cast<AbstractComponent*>(voidcptr);

	Handle h = Handle::FromVoidPtr(lua_touserdata(lua, lua_upvalueindex(lua::HandleUpValue)));

	int rets = 0;
	if (!cptr->PushEntryToLua(h, lua, rets)) {
		AddLogf(Error, "ComponentInstanceInfo::Get: Error: Component '%s' does not support lua api", typeid(*cptr).name());
		return 0;
	}

	return rets;
}

int ScriptComponent::lua_SetComponentState(lua_State *lua) {
	int argc = lua_gettop(lua);
	if (argc != 2) {
		AddLogf(Error, "ComponentInstanceInfo::Set: Error: Invalid argument count!");
		return 0;
	}

	if (lua_type(lua, -1) != LUA_TTABLE) {
		AddLogf(Error, "ComponentInstanceInfo::Set: Error: Invalid argument type!");
		return 0;
	}

	//stack: self valtable
	Utils::Scripts::LuaStackOverflowAssert check(lua);

	void *voidcptr = lua_touserdata(lua, lua_upvalueindex(lua::SelfPtrUpValue));
	AbstractComponent *cptr = reinterpret_cast<AbstractComponent*>(voidcptr);

	Handle h = Handle::FromVoidPtr(lua_touserdata(lua, lua_upvalueindex(lua::HandleUpValue)));
	int rets = 0;
	if (!cptr->PushEntryToLua(h, lua, rets)) {
		AddLogf(Error, "ComponentInstanceInfo::Set: Error: Component '%s' does not support lua api", typeid(*cptr).name());
		return 0;
	}
	//stack: self values component

	lua_pushnil(lua);								//stack: self values component nil

	while (lua_next(lua, 2) != 0) {					//stack: self values component key value
		lua_pushvalue(lua, -2);						//stack: self values component key value key
		lua_insert(lua, -2);						//stack: self values component key key value 
		lua_settable(lua, 3);						//stack: self values component key  
	}
	//stack: self values component
	lua_pop(lua, 1);

	return 0;
}

//-------------------------------------------------------------------------------------------------

int ScriptComponent::lua_DestroyComponent(lua_State *lua) {
	Handle h;
	if (!ExtractHandleFromArgument(lua, -1, h)) {
		AddLogf(Error, "ScriptComponent::DestroyComponent: Error: Invalid argument #1: not a valid handle!");
		lua_pushboolean(lua, 0);
		return 1;
	}

	void *voidthis = lua_touserdata(lua, lua_upvalueindex(lua::SelfPtrUpValue));
	ScriptComponent *This = reinterpret_cast<ScriptComponent*>(voidthis);

	ComponentID cid;
	if (This->GetHandleTable()->GetOwnerCID(h, cid)) {
		switch (static_cast<ComponentIDs>(cid)) {
		case ComponentIDs::Transform:
			AddLogf(Error, "ScriptComponent::DestroyComponent: Error: Cannot release component of cid: %d", cid);
			lua_pushboolean(lua, 0);
			return 1;
		default:
			break;
		}
	}

	if (!This->GetHandleTable()->Release(h)) {
		AddLogf(Error, "ScriptComponent::DestroyComponent: Error: Invalid argument #1: invalid handle");
		lua_pushboolean(lua, 0);
		return 1;
	}

	lua_pushboolean(lua, 1);
	return 1;
}

int ScriptComponent::lua_GetComponent(lua_State *lua) {
	int argc = lua_gettop(lua);

	ComponentID cid = 0;
	Handle RequestHandle;

	switch (argc) {
	case 2:
		if (!lua_isnumber(lua, -1)) {
			AddLogf(Error, "ScriptComponent::GetComponent: Error: Invalid argument #1: invalid type! (single argument mode)");
			return 0;
		}
		cid = static_cast<ComponentID>(lua_tointeger(lua, -1));
		if (!ExtractHandleFromArgument(lua, -2, RequestHandle)) {
			AddLogf(Error, "ScriptComponent::GetComponent: Error: Invalid argument #2: unknown type!");
			return 0;
		}
		break;
	case 3:
		if (!lua_isnumber(lua, -2)) {
			AddLogf(Error, "ScriptComponent::GetComponent: Error: Invalid argument #1: invalid type! (double argument mode)");
			return 0;
		}
		if (!lua_islightuserdata(lua, -1)) {
			AddLogf(Error, "ScriptComponent::GetComponent: Error: Invalid argument #32: invalid type! (double argument mode)");
			return 0;
		}
		cid = static_cast<ComponentID>(lua_tointeger(lua, -2));
		RequestHandle = Handle::FromVoidPtr(lua_touserdata(lua, -1));
		break;
	default:
		AddLogf(Error, "ScriptComponent::GetComponent: Error: Invalid argument count");
		return 0;
	}

	void *voidthis = lua_touserdata(lua, lua_upvalueindex(lua::SelfPtrUpValue));
	ScriptComponent *This = reinterpret_cast<ScriptComponent*>(voidthis);

	auto cptr = This->GetManager()->GetComponent(cid);
	if (!cptr) {
		AddLogf(Error, "ScripComponent::GetComponent: Error: There is no component %d", cid);
		return 0;
	}

	Entity OwnerEntity;
	if (!This->GetHandleTable()->GetHandleParentEntity(This, RequestHandle, OwnerEntity)) {
		AddLogf(Error, "ScripComponent::GetComponent: Error: Invalid owner handle!");
		return 0;
	}

	Handle ComponentHandle;

	if (!cptr->GetInstanceHandle(OwnerEntity, ComponentHandle)) {
		AddLogf(Error, "ScripComponent::GetComponent: no component instance for requested object");
		return 0;
	}

	return lua_MakeComponentInfo(lua, cid, ComponentHandle, cptr);
}

int ScriptComponent::lua_SetPerSecond(lua_State *lua) {
	void *voidthis = lua_touserdata(lua, lua_upvalueindex(lua::SelfPtrUpValue));
	ScriptComponent *This = reinterpret_cast<ScriptComponent*>(voidthis);

	lua_getfield(lua, 1, lua::HandleMemberName);					//stack: self enable handle
	Handle h = Handle::FromVoidPtr(lua_touserdata(lua, -1));
	lua_pop(lua, 1);												//stack: self enable

	bool enable = lua_toboolean(lua, 2);

	auto *entry = This->TemplateGetEntry(This, This->m_Array, h);
	if (!entry) {
		AddLogf(Error, "ScriptComponent::SetPerSecond: Error: Invalid self.handle!");
	}

	entry->m_Flags.m_Map.m_OnPerSecond = enable;
	return 0;
}

int ScriptComponent::lua_SetStep(lua_State * lua) {
	void *voidthis = lua_touserdata(lua, lua_upvalueindex(lua::SelfPtrUpValue));
	ScriptComponent *This = reinterpret_cast<ScriptComponent*>(voidthis);

	lua_getfield(lua, 1, lua::HandleMemberName);					//stack: self enable handle
	Handle h = Handle::FromVoidPtr(lua_touserdata(lua, -1));
	lua_pop(lua, 1);												//stack: self enable

	bool enable = lua_toboolean(lua, 2);

	auto *entry = This->TemplateGetEntry(This, This->m_Array, h);
	if (!entry) {
		AddLogf(Error, "ScriptComponent::SetStep: Error: Invalid self.handle!");
	}

	entry->m_Flags.m_Map.m_Step = enable;
	return 0;
}

int ScriptComponent::lua_SetActive(lua_State * lua) {
	void *voidthis = lua_touserdata(lua, lua_upvalueindex(lua::SelfPtrUpValue));
	ScriptComponent *This = reinterpret_cast<ScriptComponent*>(voidthis);

	lua_getfield(lua, 1, lua::HandleMemberName);					//stack: self enable handle
	Handle h = Handle::FromVoidPtr(lua_touserdata(lua, -1));
	lua_pop(lua, 1);												//stack: self enable

	bool enable = lua_toboolean(lua, 2);

	auto *entry = This->TemplateGetEntry(This, This->m_Array, h);
	if (!entry) {
		AddLogf(Error, "ScriptComponent::SetActive: Error: Invalid self.handle!");
	}

	entry->m_Flags.m_Map.m_Active = enable;
	return 0;
}

//-------------------------------------------------------------------------------------------------

int ScriptComponent::lua_CreateComponent(lua_State *lua) {
	ComponentID cid;

	if (!lua_istable(lua, -2)) {
		AddLogf(Error, "GameObject::CreateComponent: Error: Invalid self argument");
		lua_pushnil(lua);
		return 1;
	}

	switch (lua_type(lua, -1)) {
	case LUA_TNUMBER:
		cid = static_cast<ComponentID>(lua_tonumber(lua, -1));
		break;
	case LUA_TSTRING:
		if(ComponentRegister::GetComponentID(lua_tostring(lua, -1), cid))
			break;
		AddLogf(Error, "GameObject::CreateComponent: Error: Invalid argument #1: Unknown component: %s", lua_tostring(lua, -1));
	default:
		lua_pushnil(lua);
		return 1;
	}

	void *voidThis = lua_touserdata(lua, lua_upvalueindex(lua::SelfPtrUpValue));
	ScriptComponent *This = reinterpret_cast<ScriptComponent*>(voidThis);

	//stack: self cid

	lua_getfield(lua, -2, lua::EntityMemberName);					//stack: self cid Entity
	Entity Owner = Entity::FromVoidPtr(lua_touserdata(lua, -1));
	lua_pop(lua, 1);												//stack: self cid

	if (!This->GetManager()->GetWorld()->GetEntityManager()->IsValid(Owner)) {
		AddLogf(Error, "GameObject::CreateComponent: Error: Attempt to create component for invalid object! cid: %d", cid);
		lua_pushnil(lua);
		return 1;
	}
	
	auto *cptr = This->GetManager()->GetComponent(cid);
	if (!cptr) {
		AddLogf(Error, "GameObject::CreateComponent: Error: Invalid argument #1: There is no component cid: %d", cid);
		lua_pushnil(lua);
		return 1;
	}

	Handle hout;
	if (cptr->Create(Owner, hout)) {
		return lua_MakeComponentInfo(lua, cid, hout, cptr);
	} else {
		AddLogf(Error, "GameObject::CreateComponent: Error: Failure during component creation! cid: %d", cid);
		lua_pushnil(lua);
		return 1;
	}
}

int ScriptComponent::lua_SpawnChild(lua_State *lua) {
/*
GameObject:SpawnChild {
	Pattern = "URI",
	[Name = "..."]
	[Position = Vec3(5, 5, 5),] --local pos
	[Rotation = Quaternion(5, 5, 5, 5),] --local rotation
	
--TODO:
	[Data = { --[[ sth ]] },] -- data passed to script OnCreate function
	[Owner = someone, ] -- create as child of someone
	[Clone = someone, ] -- create clone of someone
}

returns:
	TODO
*/
	int argc = lua_gettop(lua);		//stack: self spawnarg
	Utils::Scripts::LuaStackOverflowAssert check(lua);

	if (argc != 2) {
		AddLogf(Error, "GameObject::SpawnChild: Error: Invalid argument count: %s", argc);
		return 0;
	}

	lua_getfield(lua, 2, "Pattern");									//stack: self spawnarg spawnarg.Pattern		
	const char *pattername = lua_tostring(lua, -1);
	if (!pattername) {
		AddLogf(Error, "GameObject::SpawnChild: Error: Invalid pattern name! (not a string!)", argc);
		lua_settop(lua, argc);
		return 0;
	}

	lua_getfield(lua, 2, "Name");										//stack: self spawnarg spawnarg.Name		
	const char *ChildName = lua_tostring(lua, -1);
	if (!pattername) {
		AddLogf(Error, "GameObject::SpawnChild: Error: Invalid Child! (not a string!)", argc);
		lua_settop(lua, argc);
		return 0;
	}

	//stack: self spawnarg spawnarg.Pattern

	lua_getfield(lua, 1, lua::EntityMemberName);						//stack: self spawnarg spawnarg.Pattern spawnarg.Name Entity
	Entity Owner = Entity::FromVoidPtr(lua_touserdata(lua, -1));
	lua_pop(lua, 1);													//stack: self spawnarg spawnarg.Pattern spawnarg.Name

	void *voidManager = lua_touserdata(lua, lua_upvalueindex(lua::SelfPtrUpValue));
	ComponentManager *cm = reinterpret_cast<ComponentManager*>(voidManager);

	Entity Child;
	EntityBuilder eb(cm);
	if (!eb.Build(Owner, pattername, Child, (ChildName ? std::string(ChildName) : std::string()))) {
		AddLogf(Error, "GameObject::SpawnChild: Error: Failed to build child: %s", pattername);
		lua_settop(lua, argc);
		return 0;
	}

	lua_pop(lua, 1);													//stack: self spawnarg spawnarg.Pattern
	ChildName = nullptr;

	bool HasPos = false;
	math::vec3 Position;
	bool HasRot = false;
	math::vec4 Rotation;

	lua_getfield(lua, 2, "Position");									//stack: self spawnarg spawnarg.Pattern spawnarg.Position	
	if (lua_isnil(lua, -1)) {
		HasPos = false;
	} else {
		HasPos = true;
		Position = luabridge::Stack<math::vec3>::get(lua, 4);
	}
	lua_pop(lua, 1);

	lua_getfield(lua, 2, "Rotation");									//stack: self spawnarg spawnarg.Pattern spawnarg.Rotation		
	if (lua_isnil(lua, -1)) {
		HasRot = false;
	} else {
		HasRot = true;
		Rotation = luabridge::Stack<math::vec4>::get(lua, 4);
	}
	lua_pop(lua, 1);

	if (HasRot || HasPos) {
		auto *tc = cm->GetTransformComponent();
		auto entry = tc->GetEntry(Child);
		if (!entry) {
			AddLogf(Error, "GameObject::SpawnChild: Child does not have transfrom component!");
		} else {
			if (HasPos)
				entry->m_LocalTransform.setOrigin(convert(Position));
			if (HasRot)
				entry->m_LocalTransform.setRotation(convert(Rotation));
		}
	}

	AddLogf(Hint, "GameObject::SpawnChild: Done");
	//TODO:
	lua_pushnil(lua);
	lua_settop(lua, argc + 1);
	return check.ReturnArgs(1);
}

int ScriptComponent::lua_DestroyObject(lua_State *lua) {
	Handle h;
	if (!ExtractHandleFromArgument(lua, -1, h)) {
		AddLogf(Error, "ScriptComponent::DestroyObject: Error: Invalid argument #1: unknown type!");
		lua_pushboolean(lua, 0);
		return 1;
	}

	void *voidthis = lua_touserdata(lua, lua_upvalueindex(lua::SelfPtrUpValue));
	ScriptComponent *This = reinterpret_cast<ScriptComponent*>(voidthis);

	size_t index;
	if (!This->GetHandleTable()->GetHandleIndex(This, h, index)) {
		AddLogf(Error, "ScriptComponent::DestroyObject: Error: Invalid argument #1: invalid handle");
		lua_pushboolean(lua, 0);
		return 1;
	}

	//auto &item = This->m_Array[index];
	//auto *scene = static_cast<GameScene*>(This->GetManager()->GetScene());
	//auto reg = scene->GetObjectRegister();

	LOG_NOT_IMPLEMENTED();
	//TODO: lua api DestroyObject
	AddLog(Error, "DestroyObject is not yet supported");

	lua_pushboolean(lua, 0);
	return 1;
}

int ScriptComponent::lua_Destroy(lua_State * lua) {
//	int argc = lua_gettop(lua);		//stack: self

	void *voidWorld = lua_touserdata(lua, lua_upvalueindex(lua::SelfPtrUpValue));
	World *world = reinterpret_cast<World*>(voidWorld);
	
	lua_getfield(lua, 1, lua::EntityMemberName);						//stack: self Entity
	Entity Owner = Entity::FromVoidPtr(lua_touserdata(lua, -1));
	lua_pop(lua, 1);													//stack: self

	if (!world->GetEntityManager()->Release(Owner)) {
		AddLogf(Error, "GameObject::Destroy: Error: Failed to release Entity");
		lua_pushboolean(lua, 0);
		return 1;
	}

	lua_pushboolean(lua, 1);
	return 1;
}

int ScriptComponent::lua_SetName(lua_State * lua) {
	void *voidWorld = lua_touserdata(lua, lua_upvalueindex(lua::SelfPtrUpValue));
	World *world = reinterpret_cast<World*>(voidWorld);

	const char *NewName = lua_tostring(lua, 2);
	if (!NewName) {
		AddLogf(Error, "GameObject::SetName: Error: Invalid name! (not a string!)");
		lua_pop(lua, 1);
		return 0;
	}

	lua_getfield(lua, 1, lua::EntityMemberName);					//stack: self Entity
	Entity Owner = Entity::FromVoidPtr(lua_touserdata(lua, -1));
	lua_pop(lua, 1);												//stack: self

	auto em = world->GetEntityManager();
	if (!em->SetEntityName(Owner, NewName)) {
		AddLogf(Error, "GameObject::SetName: Error: Set name failed!");
	}

	return 0;
}

int ScriptComponent::lua_GetName(lua_State * lua) {
	void *voidWorld = lua_touserdata(lua, lua_upvalueindex(lua::SelfPtrUpValue));
	World *world = reinterpret_cast<World*>(voidWorld);

	lua_getfield(lua, 1, lua::EntityMemberName);					//stack: self Entity
	Entity Owner = Entity::FromVoidPtr(lua_touserdata(lua, -1));
	lua_pop(lua, 1);												//stack: self

	auto em = world->GetEntityManager();
	const std::string *name = nullptr;
	if (!em->GetEntityName(Owner, name)) {
		AddLogf(Error, "GameObject::GetName: Error: Object does not valid!");
		return 0;
	}

	if (name)
		lua_pushlstring(lua, name->c_str(), name->length());
	else
		lua_pushstring(lua, "");

	return 1;
}

int ScriptComponent::lua_FindChild(lua_State * lua) {
	Utils::Scripts::LuaStackOverflowAssert check(lua);
	void *voidThis = lua_touserdata(lua, lua_upvalueindex(lua::SelfPtrUpValue));
	ScriptComponent *This = reinterpret_cast<ScriptComponent*>(voidThis);

	const char *ChildName = lua_tostring(lua, 2);								//stack: self ChildName			
	if (!ChildName) {
		AddLogf(Error, "GameObject::FindChild: Error: Invalid name! (not a string!)");
		return 0;
	}

	lua_getfield(lua, 1, lua::EntityMemberName);					//stack: self ChildName Entity
	Entity Owner = Entity::FromVoidPtr(lua_touserdata(lua, -1));
	lua_pop(lua, 1);												//stack: self ChildName

	Entity Child;
	if (!This->GetManager()->GetWorld()->GetEntityManager()->GetFirstChildByName(Owner, ChildName, Child)) {
		AddLogf(Error, "Child '%s' not found!", ChildName);
		return 0;
	}

	if (!This->GetObjectRootInstance(lua, Child)) {
		AddLogf(Error, "GameObject::FindChild: Error: Failed to get child gameobject");
		return 0;
	}
	//stack: self ChildName OR GO

	lua_insert(lua, -2);											//stack: self ChildName GO OR
	lua_pop(lua, 1);												//stack: self ChildName GO

	return check.ReturnArgs(1);
}

} //namespace Component 
} //namespace Core 
} //namespace MoonGlare 

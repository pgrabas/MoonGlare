/*
  * Generated by cppsrc.sh
  * On 2015-08-20 17:27:44,24
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef ComponentRegister_H
#define ComponentRegister_H

namespace MoonGlare {
namespace Core {
namespace Component {

struct LuaMetamethods {
	lua_CFunction m_Index = nullptr;
	lua_CFunction m_NewIndex = nullptr;

	constexpr LuaMetamethods(lua_CFunction i = nullptr, lua_CFunction ni = nullptr): m_Index(i), m_NewIndex(ni) { }

	constexpr operator bool() const { return m_Index || m_NewIndex; }
};

using ApiInitFunc = void(*)(ApiInitializer &api);

struct ComponentRegister {

	using ComponentCreateFunc = std::unique_ptr<AbstractComponent>(*)(ComponentManager*);
	struct ComponentInfo {
		ComponentID m_CID;
		ComponentCreateFunc m_CreateFunc;
		const char *m_Name;
		struct {
			bool m_RegisterID : 1;
		} m_Flags;
		ApiInitFunc m_ApiRegFunc;
		int(*m_GetCID)();
		const LuaMetamethods* m_EntryMetamethods;
	};

	using MapType = std::unordered_map < std::string, const ComponentInfo* >;
	static const ComponentInfo* GetComponentInfo(const char *Name) {
		auto it = s_ComponentMap->find(Name);
		if (it == s_ComponentMap->end())
			return nullptr;
		return it->second;
	}
	static const ComponentInfo* GetComponentInfo(ComponentID cid) {
		for (auto &it : *s_ComponentMap)
			if (it.second->m_CID == cid)
				return it.second;
		return nullptr;
	}
	static bool GetComponentID(const char *Name, ComponentID &cidout) {
		auto *ci = GetComponentInfo(Name);
		if (!ci)
			return false;
		cidout = ci->m_CID;
		return true;
	}
	static const MapType& GetComponentMap() { return *s_ComponentMap; }
	static void Dump(std::ostream &out);

	static bool ExtractCIDFromXML(pugi::xml_node node, ComponentID &out);
protected:
	static void SetComponent(const ComponentInfo *ci) {
		if (!s_ComponentMap)
			s_ComponentMap = new MapType();
		(*s_ComponentMap)[ci->m_Name] = ci;
	}
private:
	static MapType *s_ComponentMap;
};

template<class COMPONENT>
struct RegisterComponentID : public ComponentRegister {
	RegisterComponentID(const char *Name, bool PublishToLua = true, void(*ApiRegFunc)(ApiInitializer &api) = nullptr) {
		m_ComponentInfo.m_Name = Name;
		m_ComponentInfo.m_Flags.m_RegisterID = PublishToLua;
		m_ComponentInfo.m_ApiRegFunc = ApiRegFunc;
		m_ComponentInfo.m_CID = COMPONENT::GetComponentID();
		m_ComponentInfo.m_CreateFunc = &Construct<COMPONENT>;
		m_ComponentInfo.m_GetCID = &GetCID<COMPONENT::GetComponentID()>;
		m_ComponentInfo.m_EntryMetamethods = nullptr;// &COMPONENT::EntryMetamethods;
		SetComponent(&m_ComponentInfo);
	}

	RegisterComponentID() {
		m_ComponentInfo.m_Name = COMPONENT::Name;
		m_ComponentInfo.m_Flags.m_RegisterID = COMPONENT::PublishID;
		m_ComponentInfo.m_ApiRegFunc = &COMPONENT::RegisterScriptApi;
		m_ComponentInfo.m_CID = COMPONENT::GetComponentID();
		m_ComponentInfo.m_CreateFunc = &Construct<COMPONENT>;
		m_ComponentInfo.m_GetCID = &GetCID<COMPONENT::GetComponentID()>;
		m_ComponentInfo.m_EntryMetamethods = &COMPONENT::EntryMetamethods;
		SetComponent(&m_ComponentInfo);
	}
private:
	template<typename CLASS>
	static std::unique_ptr<AbstractComponent> Construct(ComponentManager* cm) { return std::make_unique<COMPONENT>(cm); }

	template<ComponentID value>
	static int GetCID() {
		return static_cast<int>(value);
	}
	static ComponentInfo m_ComponentInfo;
};

template<class COMPONENT>
ComponentRegister::ComponentInfo RegisterComponentID<COMPONENT>::m_ComponentInfo;

} //namespace Component 
} //namespace Core 
} //namespace MoonGlare 

#endif

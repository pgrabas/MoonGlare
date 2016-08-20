/*
  * Generated by cppsrc.sh
  * On 2016-08-10 17:23:57,17
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef MeshComponent_H
#define MeshComponent_H

#include <libSpace/src/Container/StaticVector.h>

namespace MoonGlare {
namespace Core {
namespace Component {

class MeshComponent
	: public AbstractComponent
	, public ComponentIDWrap<ComponentIDs::Mesh> {
public:
	MeshComponent(ComponentManager *Owner);
	virtual ~MeshComponent();
	virtual bool Initialize() override;
	virtual bool Finalize() override;
	virtual void Step(const MoveConfig &conf) override;
	virtual bool PushEntryToLua(Handle h, lua_State *lua, int &luarets) override;
	virtual bool Load(xml_node node, Entity Owner, Handle &hout) override;
	virtual bool GetInstanceHandle(Entity Owner, Handle &hout) override;
	virtual bool Create(Entity Owner, Handle &hout) override;

	union FlagsMap {
		struct MapBits_t {
			bool m_Valid : 1; //Entity is not valid or requested to be deleted;
			bool m_MeshValid : 1;
			bool m_MeshHandleChanged : 1;
			bool m_Visible : 1;
		};
		MapBits_t m_Map;
		uint32_t m_UintValue;

		void SetAll() { m_UintValue = 0; m_UintValue = ~m_UintValue; }
		void ClearAll() { m_UintValue = 0; }

		static_assert(sizeof(MapBits_t) <= sizeof(decltype(m_UintValue)), "Invalid Function map elements size!");
	};

	struct MeshEntry {
		FlagsMap m_Flags;
		Entity m_Owner;
		Handle m_SelfHandle;
		Handle m_MeshHandle;
		DataClasses::ModelPtr m_Model;
		std::string m_ModelName;

		bool IsVisible() const { return m_Flags.m_Map.m_Visible; }
		void SetVisible(bool v) { m_Flags.m_Map.m_Visible = v; }

		void SetMeshHandle(Handle h) {
			m_MeshHandle = h;
			m_Model.reset();
			m_Flags.m_Map.m_MeshHandleChanged = true;
		}
		Handle GetMeshHandle() const { return m_MeshHandle; }
		void SetModel(const char *name) {
			m_ModelName = name;
			m_Flags.m_Map.m_MeshHandleChanged = true;
		}

		void Reset() {
			m_Flags.m_Map.m_Valid = false;
			m_Model.reset();
			m_ModelName.clear();
		}
	};
//	static_assert((sizeof(MeshEntry) % 16) == 0, "Invalid MeshEntry size!");
//	static_assert(std::is_pod<MeshEntry>::value, "ScriptEntry must be pod!");

	MeshEntry* GetEntry(Handle h);	 //return nullptr if h/e is not valid
	MeshEntry* GetEntry(Entity e);	 //return nullptr if h/e is not valid

	static void RegisterScriptApi(ApiInitializer &root);
private:
	template<class T> using Array = Space::Container::StaticVector<T, Configuration::Storage::ComponentBuffer>;

	Array<MeshEntry> m_Array;
	EntityMapper m_EntityMapper;

	void ReleaseElement(size_t Index);
};

} //namespace Component 
} //namespace Core 
} //namespace MoonGlare 

#endif

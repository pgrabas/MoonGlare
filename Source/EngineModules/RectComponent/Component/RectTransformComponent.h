/*
  * Generated by cppsrc.sh
  * On 2016-09-12 17:58:33,31
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef RectTransformComponent_H
#define RectTransformComponent_H

#include <Core/Component/AbstractComponent.h>
#include <Core/Scripts/ScriptComponent.h>
#include <Core/Scripts/ComponentEntryWrap.h>

#include "../Margin.h"
#include "../Enums.h"
#include "../Rect.h"

namespace MoonGlare {
namespace GUI {
namespace Component {

using namespace Core::Component;

union RectTransformComponentEntryFlagsMap {
	struct MapBits_t {
		bool m_Valid : 1;
		bool m_Dirty : 1;
		bool m_Changed : 1;
	};
	MapBits_t m_Map;
	uint8_t m_UintValue;

	void SetAll() { m_UintValue = 0; m_UintValue = ~m_UintValue; }
	void ClearAll() { m_UintValue = 0; }

	static_assert(sizeof(MapBits_t) <= sizeof(decltype(m_UintValue)), "Invalid Function map elements size!");
};

struct RectTransformComponentEntry {
	Handle m_SelfHandle;
	Entity m_OwnerEntity;
	RectTransformComponentEntryFlagsMap m_Flags;

	AlignMode m_AlignMode;
	uint8_t m_padding;
	uint16_t m_Z;

	Point m_Position;				//not pod
	Point m_Size;					//not pod
	//TODO: margin property

	Margin m_Margin;				//not pod

	math::mat4 m_GlobalMatrix;		//not pod
	math::mat4 m_LocalMatrix;		//not pod
	Rect m_ScreenRect;				//not pod

	Configuration::RuntimeRevision m_Revision;

	void Recalculate(RectTransformComponentEntry &Parent);

	void SetDirty() { m_Revision = 0; m_Flags.m_Map.m_Dirty = true; }

	void Reset() {
		m_Revision = 0;
		m_Flags.ClearAll();
	}
};
//static_assert((sizeof(RectTransformComponentEntry) % 16) == 0, "RectTransformComponentEntry has invalid size");
//static_assert(std::is_pod<RectTransformComponentEntry>::value, "RectTransformComponentEntry must be pod!");

struct RectTransformSettingsFlagsMap {
	struct MapBits_t {
		bool m_UniformMode : 1;
	};
	MapBits_t m_Map;
	uint8_t m_UintValue;

	void SetAll() { m_UintValue = 0; m_UintValue = ~m_UintValue; }
	void ClearAll() { m_UintValue = 0; }

	static_assert(sizeof(MapBits_t) <= sizeof(decltype(m_UintValue)), "Invalid Function map elements size!");
};

using Core::Scripts::Component::ScriptComponent;

class RectTransformComponent 
	: public TemplateStandardComponent<RectTransformComponentEntry, ComponentID::RectTransform>
	, public Core::Scripts::Component::ComponentEntryWrap<RectTransformComponent> {
public:
	static constexpr char *Name = "RectTransform";
	static constexpr bool PublishID = true;

 	RectTransformComponent(ComponentManager *Owner);
 	virtual ~RectTransformComponent();
	virtual bool Initialize() override;
	virtual bool Finalize() override;
	virtual void Step(const Core::MoveConfig &conf) override;
	virtual bool Load(xml_node node, Entity Owner, Handle &hout) override;
	virtual bool LoadComponentConfiguration(pugi::xml_node node) override;
	virtual bool PushEntryToLua(Handle h, lua_State * lua, int & luarets) override { return false; }

	RectTransformComponentEntry &GetRootEntry() { return m_Array[0]; }
	const RectTransformComponentEntry &GetRootEntry() const { return m_Array[0]; }

	bool IsUniformMode() const { return m_Flags.m_Map.m_UniformMode; }
	const Renderer::VirtualCamera& GetCamera() const { return m_Camera; }

	bool FindChildByPosition(Handle Parent, math::vec2 pos, Entity &eout);

	static void RegisterScriptApi(ApiInitializer &root);
	static void RegisterDebugScriptApi(ApiInitializer &root);

	static int FindChild(lua_State *lua);

	math::vec2 PixelToCurrent(math::vec2 pix) const {
		if (!IsUniformMode())
			return pix;
		return pix / m_ScreenSize * GetRootEntry().m_Size;
	}

	template<bool Read, typename StackFunc>
	static bool ProcessProperty(lua_State *lua, RectTransformComponentEntry *e, uint32_t hash, int &luarets, int validx) {
		switch (hash) {
		case "Position"_Hash32:
			luarets = StackFunc::func(lua, e->m_Position, validx);
			break;
		case "Size"_Hash32:
			luarets = StackFunc::func(lua, e->m_Size, validx);
			break;
		case "Order"_Hash32:
			luarets = StackFunc::func(lua, e->m_Z, validx);
			break;
		case "ScreenPosition"_Hash32:
			luarets = StackFunc::func(lua, e->m_ScreenRect.LeftTop, validx);
			break;
		case "AlignMode"_Hash32:
			luarets = ComponentEntryWrap::ProcessEnum<AlignModeEnum, Read, StackFunc>(lua, e->m_AlignMode, validx);
			break;
		default:
			return false;
		}
		if (!Read) {
			e->SetDirty();
		}
		return true;
	}

	template<typename StackFunc, typename Entry>
	static bool QuerryFunction(lua_State *lua, Entry *e, uint32_t hash, int &luarets, int validx, RectTransformComponent *This) {
		switch (hash) {
		case "FindChild"_Hash32:
			lua_pushlightuserdata(lua, This);
			lua_pushcclosure(lua, &RectTransformComponent::FindChild, 1);
			luarets = 1;
			return true;
		case "PixelToCurrent"_Hash32:
			PushThisClosure<decltype(&PixelToCurrent), &PixelToCurrent>(lua, This);
			luarets = 1;
			return true;
		default:
			return false;
		}
	}
protected:
	ScriptComponent *m_ScriptComponent;
	RectTransformSettingsFlagsMap m_Flags;
	Configuration::RuntimeRevision m_CurrentRevision;
	Point m_ScreenSize;
	Renderer::VirtualCamera m_Camera;

//debug:
protected:
	void D2Draw(const Core::MoveConfig & conf);
	Graphic::Shaders::Shader *m_Shader = nullptr;
};

} //namespace Component 
} //namespace GUI 
} //namespace MoonGlare 

#endif

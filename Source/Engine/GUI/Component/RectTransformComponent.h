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
	char padding[3];
	RectTransformComponentEntryFlagsMap m_Flags;

	AlignMode m_AlignMode;
	Point m_Position;
	Point m_Size;
//	Point m_Scale;
	Margin m_Margin;				//not pod

	math::mat4 m_GlobalMatrix;		//not pod
	math::mat4 m_LocalMatrix;		//not pod
	Rect m_ScreenRect;				//not pod

	Configuration::RuntimeRevision m_Revision;

	Point GetPosition() const { return m_Position; }
	void SetPosition(const Point &pos) {
		m_Position = pos;
		m_Revision = 0;
	}
	Point GetSize() const { return m_Size; }
	void SetSize(const Point &s) {
		m_Size = s;
		m_Revision = 0;
	}

	void Recalculate(RectTransformComponentEntry &Parent);

	void Reset() {
		m_Revision = 0;
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

class RectTransformComponent 
	: public TemplateStandardComponent<RectTransformComponentEntry, ComponentIDs::RectTransform>
	, public Core::iCustomDraw {
public:
 	RectTransformComponent(ComponentManager *Owner);
 	virtual ~RectTransformComponent();
	virtual bool Initialize() override;
	virtual bool Finalize() override;
	virtual void Step(const Core::MoveConfig &conf) override;
	virtual bool Load(xml_node node, Entity Owner, Handle &hout) override;
	virtual bool LoadComponentConfiguration(pugi::xml_node node) override;

	RectTransformComponentEntry &GetRootEntry() { return m_Array[0]; }

	bool IsUniformMode() const { return m_Flags.m_Map.m_UniformMode; }

	static void RegisterScriptApi(ApiInitializer &root);
protected:
	RectTransformSettingsFlagsMap m_Flags;
	Configuration::RuntimeRevision m_CurrentRevision;
	Point m_ScreenSize;

//debug:
protected:
	// iCustomDraw
	virtual void D2Draw(Graphic::cRenderDevice& dev) override;
	Graphic::Shaders::Shader *m_Shader = nullptr;
};

} //namespace Component 
} //namespace GUI 
} //namespace MoonGlare 

#endif

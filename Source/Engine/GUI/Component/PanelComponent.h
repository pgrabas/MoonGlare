/*
  * Generated by cppsrc.sh
  * On 2016-09-18 19:00:53,20
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef PanelComponent_H
#define PanelComponent_H

#include "nfGUIComponent.h"
#include <Core/Component/AbstractComponent.h>
#include "../Margin.h"
#include "../Animation.h"

namespace MoonGlare {
namespace GUI {
namespace Component {

using namespace Core::Component;

struct PanelShader;

union PanelComponentEntryFlagsMap {
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

struct PanelComponentEntry {
	Handle m_SelfHandle;
	Entity m_OwnerEntity;
	char padding[3];
	PanelComponentEntryFlagsMap m_Flags;

	Configuration::RuntimeRevision m_TransformRevision;

	TextureFile m_Texture;
	Graphic::VAO m_VAO;
	float m_Border;
	math::vec4 m_Color;
	glm::ivec2 m_TileMode;

	void Reset() {
		m_TransformRevision = 0;
		m_Flags.ClearAll();
	}

};
//static_assert((sizeof(RectTransformComponentEntry) % 16) == 0, "RectTransformComponentEntry has invalid size");
//static_assert(std::is_pod<RectTransformComponentEntry>::value, "RectTransformComponentEntry must be pod!");

class PanelComponent
	: public TemplateStandardComponent<PanelComponentEntry, ComponentIDs::Panel> 
	, public Core::iCustomDraw {
public:
	PanelComponent(ComponentManager *Owner);
 	virtual ~PanelComponent();
	virtual bool Initialize() override;
	virtual bool Finalize() override;
	virtual void Step(const Core::MoveConfig &conf) override;
	virtual bool Load(xml_node node, Entity Owner, Handle &hout) override;

	static void RegisterScriptApi(ApiInitializer &root);
protected:
	RectTransformComponent *m_RectTransform;
	PanelShader *m_Shader = nullptr;

protected:
	// iCustomDraw
	virtual void D2Draw(Graphic::cRenderDevice& dev) override;
};

} //namespace Component 
} //namespace GUI 
} //namespace MoonGlare 

#endif
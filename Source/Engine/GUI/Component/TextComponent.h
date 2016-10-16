/*
  * Generated by cppsrc.sh
  * On 2016-09-23 21:24:39,56
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef TextComponent_H
#define TextComponent_H

#include "nfGUIComponent.h"
#include <Core/Component/AbstractComponent.h>
#include <TextProcessor.h>

#include "../Enums.h"

namespace MoonGlare {
namespace GUI {
namespace Component {

using namespace Core::Component;

union TextComponentEntryFlagsMap {
	struct MapBits_t {
		bool m_Valid : 1;
		bool m_Dirty : 1;
		bool m_Active : 1;
		bool m_TextDirty : 1;
	};
	MapBits_t m_Map;
	uint8_t m_UintValue;

	void SetAll() { m_UintValue = 0; m_UintValue = ~m_UintValue; }
	void ClearAll() { m_UintValue = 0; }

	static_assert(sizeof(MapBits_t) <= sizeof(decltype(m_UintValue)), "Invalid Function map elements size!");
};

struct TextComponentEntry {
	Handle m_SelfHandle;
	Entity m_OwnerEntity;
	char padding[3];
	TextComponentEntryFlagsMap m_Flags;

	math::vec4 m_Color;
	std::string m_Text;
	TextAlignMode m_AlignMode;

	//TODO: fontname property
	//TODO: fontsize property
	DEFINE_COMPONENT_PROPERTY(Color);
	DEFINE_COMPONENT_PROPERTY(Text);

	float GetFontSize() const { return m_FontStyle.Size; }
	void SetFontSize(float v) { m_FontStyle.Size = v; SetDirty(); }

	math::mat4 m_Matrix;
	DataClasses::FontPtr m_Font;
	DataClasses::SharedFontInstance m_FontInstance;
	DataClasses::Fonts::Descriptor m_FontStyle;

	void Reset() {
		m_Flags.ClearAll();
		m_Font.reset();
		m_FontInstance.reset();
	}

	void SetDirty() { m_Flags.m_Map.m_Dirty = true;  m_Flags.m_Map.m_TextDirty = true; }

	void Update(RectTransformComponentEntry &Parent, bool Uniform, TextProcessor &tproc);
};
//static_assert((sizeof(RectTransformComponentEntry) % 16) == 0, "RectTransformComponentEntry has invalid size");
//static_assert(std::is_pod<RectTransformComponentEntry>::value, "RectTransformComponentEntry must be pod!");

class TextComponent
	: public TemplateStandardComponent<TextComponentEntry, ComponentID::Text> {
public:
	TextComponent(ComponentManager *Owner);
	virtual ~TextComponent();
	virtual bool Initialize() override;
	virtual bool Finalize() override;
	virtual void Step(const Core::MoveConfig &conf) override;
	virtual bool Load(xml_node node, Entity Owner, Handle &hout) override;

	static void RegisterScriptApi(ApiInitializer &root);
protected:
	RectTransformComponent *m_RectTransform;
	GUIShader *m_Shader;
	TextProcessor m_TextProcessor;
};

} //namespace Component 
} //namespace GUI 
} //namespace MoonGlare 

#endif

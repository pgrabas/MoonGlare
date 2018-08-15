/*
  * Generated by cppsrc.sh
  * On 2016-09-23 21:24:39,56
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>

#define NEED_VAO_BUILDER

#include <MoonGlare.h>
#include <Engine/DataClasses/iFont.h>
#include "Engine/Core/DataManager.h"

#include "../nfGUI.h"

#include <Core/Component/SubsystemManager.h>
#include <Core/Component/ComponentRegister.h>
#include "RectTransformComponent.h"

#include <Renderer/Commands/OpenGL/ControllCommands.h>
#include <Renderer/Commands/OpenGL/ShaderCommands.h>
#include <Renderer/Commands/OpenGL/TextureCommands.h>
#include <Renderer/Commands/OpenGL/ArrayCommands.h>
#include <Renderer/Resources/ResourceManager.h>
#include <Source/Renderer/RenderDevice.h>
#include <Source/Renderer/Frame.h>
#include <Renderer/Renderer.h>

#include <Math.x2c.h>
#include <ComponentCommon.x2c.h>
#include <TextComponent.x2c.h>

#include "TextComponent.h"

namespace MoonGlare {
namespace GUI {
namespace Component {

//---------------------------------------------------------------------------------------

::Space::RTTI::TypeInfoInitializer<TextComponent, TextComponentEntry> TextComponentTypeInfo;
RegisterComponentID<TextComponent>TextComponentIDReg("Text");

//---------------------------------------------------------------------------------------

TextComponent::TextComponent(SubsystemManager * Owner)
		: TemplateStandardComponent(Owner) {
	m_RectTransform = nullptr;
}

TextComponent::~TextComponent() {
}

//---------------------------------------------------------------------------------------

MoonGlare::Scripts::ApiInitializer TextComponent::RegisterScriptApi(MoonGlare::Scripts::ApiInitializer root) {
	return root
		.beginClass<TextComponentEntry>("cTextComponentEntry")
			.addProperty("Color", &TextComponentEntry::GetColor, &TextComponentEntry::SetColor)
			.addProperty("Text", &TextComponentEntry::GetText, &TextComponentEntry::SetText)
			.addProperty("FontSize", &TextComponentEntry::GetFontSize, &TextComponentEntry::SetFontSize)
		.endClass()
		; 
}

//---------------------------------------------------------------------------------------

bool TextComponent::Initialize() {
	//memset(&m_Array, 0, m_Array.Capacity() * sizeof(m_Array[0]));
	//m_Array.fill(TextComponentEntry());
	for (auto &item : m_Array)
		item.Reset();
	m_Array.ClearAllocation();

	m_RectTransform = GetManager()->GetComponent<RectTransformComponent>();
	if (!m_RectTransform) {
		AddLog(Error, "Failed to get RectTransformComponent instance!");
		return false;
	}

	auto &shres = GetManager()->GetWorld()->GetRendererFacade()->GetResourceManager()->GetShaderResource();
	if (!shres.Load(m_ShaderHandle, "GUI")) {
		AddLogf(Error, "Failed to load GUI shader");
		return false;
	}

	m_FontDeviceOptions.m_UseUniformMode = m_RectTransform->IsUniformMode();
    m_FontDeviceOptions.m_DeviceSize = GetManager()->GetWorld()->GetRendererFacade()->GetContext()->GetSize();
	
	m_TextProcessor.SetTables(GetManager()->GetWorld()->GetStringTables());

	return true;
}

bool TextComponent::Finalize() {
	return true;
}

//---------------------------------------------------------------------------------------

void TextComponent::Step(const Core::MoveConfig & conf) {
    auto &layers = conf.m_BufferFrame->GetCommandLayers();
    auto &Queue = layers.Get<Renderer::Configuration::FrameBuffer::Layer::GUI>();
	auto &q = Queue;

	size_t LastInvalidEntry = 0;
	size_t InvalidEntryCount = 0;

	auto &shres = conf.m_BufferFrame->GetResourceManager()->GetShaderResource();
	auto shb = shres.GetBuilder(q, m_ShaderHandle);

	using Uniform = GUIShaderDescriptor::Uniform;
	using Sampler = GUIShaderDescriptor::Sampler;

	for (size_t i = 0; i < m_Array.Allocated(); ++i) {
		auto &entry = m_Array[i];

		if (!entry.m_Flags.m_Map.m_Valid) {
			//mark and continue
			LastInvalidEntry = i;
			++InvalidEntryCount;
			continue;
		}

		auto *rtentry = m_RectTransform->GetEntry(entry.m_Owner);
		if (!rtentry) {
            AddLog(Hint, "Invalid: owner: " << entry.m_Owner);

			LastInvalidEntry = i;
			++InvalidEntryCount;
			continue;
		}

		if (!entry.m_Flags.m_Map.m_Active)
			continue;

		Renderer::Commands::CommandKey key{ rtentry->m_Z };

		if (entry.m_Flags.m_Map.m_Dirty || rtentry->m_Flags.m_Map.m_Changed) {
			entry.Update(conf.m_BufferFrame, m_FontDeviceOptions, *rtentry, m_RectTransform->IsUniformMode(), m_TextProcessor);
		}

		shb.Set<Uniform::ModelMatrix>(emath::MathCast<emath::fmat4>(entry.m_Matrix), key);
		shb.Set<Uniform::BaseColor>(emath::fvec4(1,1,1,1), key);
		shb.Set<Uniform::TileMode>(emath::ivec2(0, 0), key);
		shb.Set<Sampler::DiffuseMap>(entry.m_FontResources.m_Texture, key);
        shb.Set<Uniform::FrameCount>(emath::ivec2(0, 0), key);

		auto vaob = conf.m_BufferFrame->GetResourceManager()->GetVAOResource().GetVAOBuilder(Queue, entry.m_FontResources.m_VAO);
		Queue.PushCommand<Renderer::Commands::VAOBindResource>(key)->m_VAO = vaob.m_HandlePtr;

		auto arg = Queue.PushCommand<Renderer::Commands::VAODrawTriangles>(key);
		arg->m_NumIndices = 6;
		arg->m_IndexValueType = Renderer::Device::TypeInfo<uint8_t>::TypeId;
	}

	if (InvalidEntryCount > 0) {
		AddLogf(Performance, "TextComponent:%p InvalidEntryCount:%lu LastInvalidEntry:%lu", this, InvalidEntryCount, LastInvalidEntry);
		TrivialReleaseElement(LastInvalidEntry);
	}
}

//---------------------------------------------------------------------------------------

bool TextComponent::Load(ComponentReader &reader, Entity parent, Entity owner) {
	size_t index;
	if (!m_Array.Allocate(index)) {
		AddLogf(Error, "Failed to allocate index!");
		return false;
	}
	auto &entry = m_Array[index];
	entry.Reset();
	entry.m_Owner = owner;

	x2c::Component::TextComponent::TextEntry_t te;
	te.ResetToDefault();
	if (!reader.Read(te)) {
		AddLog(Error, "Failed to read ImageEntry!");
		return false;
	}

	entry.m_Font = GetDataMgr()->GetFont(te.m_FontName);
	if (!entry.m_Font) {
		AddLogf(Error, "Unknown font: %s", te.m_FontName.c_str());
		return false;
	}

	entry.m_AlignMode = te.m_TextAlignMode;
	entry.m_FontStyle.m_Color = emath::fvec4(1, 1, 1, 1);
	entry.m_FontStyle.m_Size = te.m_FontSize;
	entry.m_Text = te.m_Text;
	entry.m_Flags.m_Map.m_Active = te.m_Active;

	entry.m_Flags.m_Map.m_Valid = true;
	entry.m_Flags.m_Map.m_Dirty = true;
	entry.m_Flags.m_Map.m_TextDirty = true;
	m_EntityMapper.SetIndex(owner, index);
	return true;
}

//---------------------------------------------------------------------------------------

void TextComponentEntry::Update(Renderer::Frame *frame, const DataClasses::Fonts::iFont::FontDeviceOptions &devopt, RectTransformComponentEntry & Parent, bool Uniform, TextProcessor &tproc) {
	if (m_Flags.m_Map.m_TextDirty) {
		std::string processed;
		tproc.Process(m_Text, processed);
		std::wstring txt = Utils::Strings::towstring(processed);
		m_Flags.m_Map.m_TextDirty = !m_Font->RenderText(txt, frame, m_FontStyle, devopt, m_FontRect, m_FontResources);
	}

	auto tsize = m_FontRect.m_TextBlockSize; 
	auto psize = Parent.m_ScreenRect.GetSize();

	math::vec3 Pos(0);

	switch (m_AlignMode) {
	case TextAlignMode::LeftTop:
		break;
	case TextAlignMode::MiddleTop:
		Pos.x = psize.x / 2.0f - tsize.x / 2.0f;
		break;
	case TextAlignMode::RightTop:
		Pos.x = psize.x - tsize.x;
		break;

	case TextAlignMode::LeftMiddle:
		Pos.y = psize.y / 2.0f - tsize.y / 2.0f;
		break;
	case TextAlignMode::Middle:
		Pos = math::vec3((psize - tsize) / 2.0f, 0);
		break;
	case TextAlignMode::RightMiddle:
		Pos.x = psize.x - tsize.x;
		Pos.y = psize.y / 2.0f - tsize.y / 2.0f;
		break;

	case TextAlignMode::LeftBottom:
		Pos.y += psize.y - tsize.y;
		break;
	case TextAlignMode::MiddleBottom:
		Pos.y = psize.y - tsize.y;
		Pos.x = psize.x / 2.0f - tsize.x / 2.0f;
		break;
	case TextAlignMode::RightBottom:
		Pos = math::vec3(psize - tsize, 0);
		break;

		//case TextAlignMode::Justified: break;
	default:
		LogInvalidEnum(m_AlignMode);
		break;
	}

	m_translate = glm::translate(math::mat4(), Pos);
	m_Matrix = Parent.m_GlobalMatrix * m_translate;
	m_Flags.m_Map.m_Dirty = m_Flags.m_Map.m_TextDirty;
}

} //namespace Component 
} //namespace GUI 
} //namespace MoonGlare 

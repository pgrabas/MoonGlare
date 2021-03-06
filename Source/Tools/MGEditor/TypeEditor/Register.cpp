#include PCH_HEADER

#ifdef _BUILDING_MGEDITOR_

#include <Source/Engine/Core/Component/nfComponent.h>

#include <Source/Renderer/Light.h>
#include <Source/Renderer/StaticFog.h>
#include <Engine/Modules/RectComponent/Types.h>

#include <Math.x2c.h>
#include <LightComponent.x2c.h>
#include <BodyComponent.x2c.h>
#include <BodyShapeComponent.x2c.h>
#include <TransformComponent.x2c.h>
#include <CameraComponent.x2c.h>
#include <MeshComponent.x2c.h>
#include <ScriptComponent.x2c.h>
#include <RectTransformComponent.x2c.h>
#include <ImageComponent.x2c.h>
#include <PanelComponent.x2c.h>
#include <TextComponent.x2c.h>
#include <StaticFog.x2c.h>
#include <Scene.x2c.h>

#include <DirectAnimationComponent.x2c.h>
#include <Foundation/SoundSystem/Component/SoundSourceComponent.h>
#include <SoundSourceComponent.x2c.h>

#include "CustomType.h"
#include "Structure.h"
#include "ComponentInfo.h"

#include <TypeEditor/x2cEnum.h>
#include <TypeEditor/x2cStructure.h>

namespace MoonGlare {
namespace TypeEditor {

using namespace x2c::Component;

using SubSystemId = Core::Component::SubSystemId;

//----------------------------------------------------------------------------------

struct bool_TypeInfo {
    static constexpr char *GetTypeName() {
        return "bool";
    }
    static bool GetValues(std::unordered_map<std::string, uint64_t> &values) {
        values["True"] = static_cast<uint64_t>(1);
        values["False"] = static_cast<uint64_t>(0);
        return true;
    }
};

//----------------------------------------------------------------------------------

enum class ComponentOrder {
    Unused,

    ScriptComponent,

    BodyComponent,
    BodyShapeComponent,

    Transform,

    CameraComponent,
    LightComponent,
    MeshComponent,
    DirectAnimation,

    RectTransform,
    ImageComponent,
    PanelComponent,
    TextComponent,

    SoundSource,

    Unknown = 0xFFFF,
};

struct EmptyBase {
    bool Read(const pugi::xml_node node, const char *name = nullptr) { return true; }
    bool Write(pugi::xml_node node, const char *name = nullptr) const { return true; }
    void ResetToDefault() {}
    bool WriteFile(const std::string& filename) const { return false; }
    bool ReadFile(const std::string& filename) { return true; }
    void GetMemberInfo(::x2c::cxxpugi::StructureMemberInfoTable &members) const {}
};
struct EmptySettings : public EmptyBase {
    static constexpr char *GetTypeName() { return "EmptySettings"; }
    void GetWriteFuncs(std::unordered_map<std::string, std::function<void(EmptySettings &self, const std::string &input)>> &funcs) const {}
    void GetReadFuncs(std::unordered_map<std::string, std::function<void(const EmptySettings &self, std::string &output)>> &funcs) {}
};
struct EmptyEntry : public EmptyBase {
    static constexpr char *GetTypeName() { return "EmptyEntry"; }
    void GetWriteFuncs(std::unordered_map<std::string, std::function<void(EmptyEntry &self, const std::string &input)>> &funcs) const {}
    void GetReadFuncs(std::unordered_map<std::string, std::function<void(const EmptyEntry &self, std::string &output)>> &funcs) {}
};

//-------------------------------------
struct TransformComponentDesc {
    using Entry_t = TransformComponent::TransformEntry_t;
    using Settings_t = EmptySettings;// TransformComponent::RectTransformSettings_t;
    static constexpr char *DisplayName = "Transform";
    static constexpr char *Name = "Transform";
    static constexpr SubSystemId CID = SubSystemId::Transform;
    static constexpr SubSystemId Depend = SubSystemId::Invalid;
    static constexpr ComponentOrder Order = ComponentOrder::Transform;
};
//-------------------------------------
struct LightComponentDesc {
    using Entry_t = LightComponent::LightEntry_t;
    using Settings_t = EmptySettings;
    static constexpr char *DisplayName = "Light";
    static constexpr char *Name = "Light";
    static constexpr SubSystemId CID = SubSystemId::Light;
    static constexpr SubSystemId Depend = SubSystemId::Transform;
    static constexpr ComponentOrder Order = ComponentOrder::LightComponent;
};
struct CameraComponentDesc {
    using Entry_t = CameraComponent::CameraEntry_t;
    using Settings_t = EmptySettings;
    static constexpr char *DisplayName = "Camera";
    static constexpr char *Name = "Camera";
    static constexpr SubSystemId CID = SubSystemId::Camera;
    static constexpr SubSystemId Depend = SubSystemId::Transform;
    static constexpr ComponentOrder Order = ComponentOrder::CameraComponent;
};
struct MeshComponentDesc {
    using Entry_t = MeshComponent::MeshEntry_t;
    using Settings_t = EmptySettings;
    static constexpr char *DisplayName = "Mesh";
    static constexpr char *Name = "Mesh";
    static constexpr SubSystemId CID = SubSystemId::Mesh;
    static constexpr SubSystemId Depend = SubSystemId::Transform;
    static constexpr ComponentOrder Order = ComponentOrder::MeshComponent;
//	RegComponent<, EmptySettings >
//("Mesh", "Mesh", SubSystemId::Mesh, {});
};		
struct DirectAnimationComponentDesc {
    using Entry_t = DirectAnimationComponent::DirectAnimationEntry_t;
    using Settings_t = EmptySettings;
    static constexpr char *DisplayName = "DirectAnimation";
    static constexpr char *Name = "DirectAnimation";
    static constexpr SubSystemId CID = SubSystemId::DirectAnimation;
    static constexpr SubSystemId Depend = SubSystemId::Transform;
    static constexpr ComponentOrder Order = ComponentOrder::DirectAnimation;
    //	RegComponent<, EmptySettings >
    //("Mesh", "Mesh", SubSystemId::Mesh, {});
};

struct ScriptComponentDesc {
    using Entry_t = ScriptComponent::ScriptEntry_t;
    using Settings_t = EmptySettings;
    static constexpr char *DisplayName = "Script";
    static constexpr char *Name = "Script";
    static constexpr SubSystemId CID = SubSystemId::Script;
    static constexpr SubSystemId Depend = SubSystemId::Invalid;
    static constexpr ComponentOrder Order = ComponentOrder::ScriptComponent;
//RegComponent<ScriptComponent::ScriptEntry_t, EmptySettings >
    //("Script", "Script", SubSystemId::Script, {});
};
//-------------------------------------
struct BodyComponentDesc {
    using Entry_t = BodyComponent::BodyEntry_t;
    using Settings_t = BodyComponent::BodyComponentSettings_t;
    static constexpr char *DisplayName = "Body";
    static constexpr char *Name = "Body";
    static constexpr SubSystemId CID = SubSystemId::Body;
    static constexpr SubSystemId Depend = SubSystemId::Transform;
    static constexpr ComponentOrder Order = ComponentOrder::BodyComponent;
};
struct BodyShapeComponentDesc {
    using Entry_t = BodyShapeComponent::ColliderComponent_t;
    using Settings_t = EmptySettings;
    static constexpr char *DisplayName = "BodyShape";
    static constexpr char *Name = "BodyShape";
    static constexpr SubSystemId CID = SubSystemId::BodyShape;
    static constexpr SubSystemId Depend = SubSystemId::Body;
    static constexpr ComponentOrder Order = ComponentOrder::BodyShapeComponent;
};
//-------------------------------------
struct RectTransformComponentDesc {
    using Entry_t = RectTransformComponent::RectTransformEntry_t;
    using Settings_t = RectTransformComponent::RectTransformSettings_t;
    static constexpr char *DisplayName = "Gui.RectTransform";
    static constexpr char *Name = "RectTransform";
    static constexpr SubSystemId CID = SubSystemId::RectTransform;
    static constexpr SubSystemId Depend = SubSystemId::Invalid;
    static constexpr ComponentOrder Order = ComponentOrder::RectTransform;
};
struct ImageComponentDesc {
    using Entry_t = ImageComponent::ImageEntry_t;
    using Settings_t = ImageComponent::ImageComponentSettings_t;
    static constexpr char *DisplayName = "Gui.Image";
    static constexpr char *Name = "Image";
    static constexpr SubSystemId CID = SubSystemId::Image;
    static constexpr SubSystemId Depend = SubSystemId::RectTransform;
    static constexpr ComponentOrder Order = ComponentOrder::ImageComponent;
};
struct PanelComponentDesc {	
    using Entry_t = PanelComponent::PanelEntry_t;
    using Settings_t = PanelComponent::PanelComponentSettings_t;
    static constexpr char *DisplayName = "Gui.Panel";
    static constexpr char *Name = "Panel";
    static constexpr SubSystemId CID = SubSystemId::Panel;
    static constexpr SubSystemId Depend = SubSystemId::RectTransform;
    static constexpr ComponentOrder Order = ComponentOrder::PanelComponent;
};
struct TextComponentDesc {
    using Entry_t = x2c::SoundSystem::SoundSourceComponentData_t;
    using Settings_t = TextComponent::TextComponentSettings_t;
    static constexpr char *DisplayName = "Gui.Text";
    static constexpr char *Name = "Text";
    static constexpr SubSystemId CID = SubSystemId::Text;
    static constexpr SubSystemId Depend = SubSystemId::RectTransform;
    static constexpr ComponentOrder Order = ComponentOrder::TextComponent;
};
//-------------------------------------
struct SoundSourceComponentDesc {
    using Entry_t = x2c::SoundSystem::SoundSourceComponentData_t;
    using Settings_t = EmptySettings;
    static constexpr char *DisplayName = "Sound.Source";
    static constexpr char *Name = "SoundSource";
    static constexpr SubSystemId CID = (SubSystemId)0x70;
    static constexpr SubSystemId Depend = SubSystemId::Invalid;
    static constexpr ComponentOrder Order = ComponentOrder::SoundSource;
};
//-------------------------------------


struct Register {
    Register() {
        RegEnum<RectTransformComponent::AlignMode_TypeInfo>();
        RegEnum<ImageComponent::ImageScaleMode_TypeInfo>();
        RegEnum<TextComponent::TextAlignMode_TypeInfo>();
        RegEnum<BodyShapeComponent::ColliderType_TypeInfo>();

        RegEnum<x2c::SoundSystem::PositionMode_TypeInfo>();
        RegEnum<x2c::SoundSystem::SoundKind_TypeInfo>();

        RegEnum<bool_TypeInfo>();

        RegComponent<ScriptComponentDesc>();

        RegComponent<TransformComponentDesc>();
        RegComponent<LightComponentDesc>();
        RegComponent<CameraComponentDesc>();
        RegComponent<MeshComponentDesc>();
        RegComponent<DirectAnimationComponentDesc>();
        
        RegComponent<BodyComponentDesc>();
        RegComponent<BodyShapeComponentDesc>();

        RegComponent<RectTransformComponentDesc>();
        RegComponent<ImageComponentDesc>();
        RegComponent<PanelComponentDesc>();
        RegComponent<TextComponentDesc>();

        RegComponent<SoundSourceComponentDesc>();

        RegStructure<x2c::Core::Scene::SceneConfiguration_t>();
    }

    template<typename ENUM>
    void RegEnum() {
        TypeEditorInfo::RegisterTypeEditor(std::make_shared<TemplateTypeEditorInfo<X2CEnumTemplate<ENUM>>>(), ENUM::GetTypeName());
    }

    template<typename STRUCT>
    SharedStructureInfo RegStructure() {
        SharedStructureInfo ssi;
        Structure::RegisterStructureInfo(ssi = Structure::MakeX2CStructureInfo<STRUCT>());
        return ssi;
    }

    template<typename CINFO>
    void RegComponent() {
        ComponentInfo ci;
        ci.m_CID = static_cast<SubSystemId>(CINFO::CID);
        ci.m_Name = CINFO::Name;
        ci.m_DisplayName = CINFO::DisplayName;
        ci.m_EntryStructure = RegStructure<CINFO::Entry_t>();
        ci.m_SettingsStructure = RegStructure<CINFO::Settings_t>();
        ci.m_Requirement = static_cast<SubSystemId>(CINFO::Depend);
        ci.m_DefautltIndex = static_cast<unsigned>(CINFO::Order);
        ComponentInfo::RegisterComponentInfo(std::make_shared<ComponentInfo>(std::move(ci)));
    }
};

const Register reg;

} //namespace TypeEditor
} //namespace MoonGlare

#endif
#pragma once

#include <libSpace/src/Container/StaticVector.h>
#include <Core/Component/TemplateStandardComponent.h>

#include <Renderer/VirtualCamera.h>

namespace MoonGlare::Component {           
using namespace Core::Component;

class TransformComponent;

struct CameraComponentEntry {
	union FlagsMap {
		struct MapBits_t {
			bool m_Valid : 1; //Entity is not valid or requested to be deleted;
			bool m_Active : 1;
			bool m_ActiveChanged : 1;
			bool m_Orthogonal : 1;
		};
		MapBits_t m_Map;
		uint32_t m_UintValue;

		void SetAll() { m_UintValue = 0; m_UintValue = ~m_UintValue; }
		void ClearAll() { m_UintValue = 0; }

		static_assert(sizeof(MapBits_t) <= sizeof(decltype(m_UintValue)), "Invalid Function map elements size!");
	};

	FlagsMap m_Flags;
	Entity m_Owner;

	emath::fmat4 m_ProjectionMatrix;

	Renderer::VirtualCamera m_Camera;

	float m_FoV;

	void Reset() {
		m_Flags.ClearAll();
	}

	void SetActive(bool v) { m_Flags.m_Map.m_Active = v; m_Flags.m_Map.m_ActiveChanged = v; }
	bool GetActive() const { return m_Flags.m_Map.m_Active; }

	void ResetProjectionMatrix(const emath::fvec2 &ScreenSize);
};
//static_assert((sizeof(CameraComponentEntry) % 16) == 0, "Invalid CameraEntry size!");
//static_assert(std::is_pod<LightComponentEntry>::value, "LightComponentEntry must be pod!");

class CameraComponent 
	: public TemplateStandardComponent<CameraComponentEntry, SubSystemId::Camera, MoonGlare::Configuration::Storage::CameraComponent> {
public:
	CameraComponent(SubsystemManager *Owner);
	virtual ~CameraComponent();
	virtual bool Initialize() override;
	virtual bool Finalize() override;
	virtual void Step(const Core::MoveConfig &conf) override;
	virtual bool Load(ComponentReader &reader, Entity parent, Entity owner) override;

	using CameraEntry = CameraComponentEntry;
	static MoonGlare::Scripts::ApiInitializer RegisterScriptApi(MoonGlare::Scripts::ApiInitializer root);
protected:
	TransformComponent *m_TransformComponent;
};

}


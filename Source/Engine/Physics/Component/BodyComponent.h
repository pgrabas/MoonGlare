/*
  * Generated by cppsrc.sh
  * On 2016-08-15 12:19:18,12
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef BodyComponent_H
#define BodyComponent_H

#include <libSpace/src/Container/StaticVector.h>

#include <Core/Component/AbstractComponent.h>

namespace MoonGlare {
namespace Physics {
namespace Component {

using namespace ::Physics;

class BodyComponent
		: public Core::Component::AbstractComponent
		, public Core::Component::ComponentIDWrap<Core::Component::ComponentIDs::Body>
		, public Core::iCustomDraw {
public:
	BodyComponent(Core::Component::ComponentManager *Owner);
	virtual ~BodyComponent();

	virtual bool Initialize() override;
	virtual bool Finalize() override;

	virtual void Step(const Core::MoveConfig &conf) override;
	virtual bool Load(xml_node node, Entity Owner, Handle &hout) override;

	virtual bool GetInstanceHandle(Entity Owner, Handle &hout) override;

	virtual bool LoadComponentConfiguration(pugi::xml_node node) override;

	//virtual bool Create(Entity Owner, Handle &hout);
	//virtual bool PushEntryToLua(Handle h, lua_State *lua, int &luarets);

	//Core::iCustomDraw;
	virtual void DefferedDraw(Graphic::cRenderDevice& dev) override;

	union FlagsMap {
		struct MapBits_t {
			bool m_Valid : 1; //Entity is not valid or requested to be deleted;
			bool m_Kinematic : 1;
			bool m_HasShape : 1;
		};
		MapBits_t m_Map;
		uint32_t m_UintValue;

		void SetAll() { m_UintValue = 0; m_UintValue = ~m_UintValue; }
		void ClearAll() { m_UintValue = 0; }

		static_assert(sizeof(MapBits_t) <= sizeof(decltype(m_UintValue)), "Invalid Function map elements size!");
	};

	struct BodyEntry {
		Handle m_SelfHandle;
		Handle m_TransformHandle;
		Entity m_OwnerEntity;
		FlagsMap m_Flags;

		Handle m_ShapeHandle;
		float m_Mass;

//		CollisionMask m_CollisionMask;

		Configuration::RuntimeRevision m_Revision;

//		SharedShape m_Shape;
//		vec3 m_BodyAngularFactor;
	};

	BodyEntry* GetEntry(Handle h);	
	BodyEntry* GetEntry(Entity e);	

	struct BulletRigidBody;
	BulletRigidBody* GetRigidBody(Handle h);	 //return nullptr if h/e is not valid

	bool SetShape(Handle ShapeHandle, Handle BodyHandle, btCollisionShape *ptr);

	struct BulletProxyCommon {
		BodyComponent *m_BodyComponent;
		Handle m_EntryHandle;
		Handle m_TransformHandle;
		Core::Component::TransformComponent *m_Transform;

		void Reset(BodyComponent *bc, Handle eh) {
			m_EntryHandle = eh;
			m_BodyComponent = bc;
		}
		void SetTrnsform(Core::Component::TransformComponent *Transform, Handle th) {
			m_TransformHandle = th;
			m_Transform = Transform;
		}

		void getWorldTransform(btTransform & centerOfMassWorldTrans) const {
			auto *entry = m_Transform->GetEntry(m_TransformHandle);
			if (entry) {
				centerOfMassWorldTrans = entry->m_LocalTransform;
			}
		}
		void setWorldTransform(const btTransform & centerOfMassWorldTrans) {
			auto *entry = m_Transform->GetEntry(m_TransformHandle);
			if (entry) {
			//	entry->m_LocalTransform = centerOfMassWorldTrans;
				entry->SetTransform(centerOfMassWorldTrans);
			//	entry->m_Revision = m_Transform->GetCurrentRevision();
				//auto *be = m_BodyComponent->GetEntry(m_EntryHandle);
				//if (be)
				//	be->m_Revision = entry->m_Revision;
			}
		}
	};

	struct BulletMotionStateProxy : public btMotionState, public BulletProxyCommon {
		BulletMotionStateProxy() {}
		///synchronizes world transform from user to physics
		virtual void getWorldTransform(btTransform& centerOfMassWorldTrans) const override { BulletProxyCommon::getWorldTransform(centerOfMassWorldTrans); }
		///synchronizes world transform from physics to user
		///Bullet only calls the update of worldtransform for active objects
		virtual void setWorldTransform(const btTransform& centerOfMassWorldTrans) override { BulletProxyCommon::setWorldTransform(centerOfMassWorldTrans); }
	};

	struct BulletRigidBody : public btRigidBody, public BulletProxyCommon {
		BulletRigidBody() :btRigidBody(0.0f, nullptr, nullptr) {}
	};

	static void RegisterScriptApi(ApiInitializer &api);
protected:
	template<class T> using Array = Space::Container::StaticVector<T, Configuration::Storage::ComponentBuffer>;

	std::unique_ptr<btDefaultCollisionConfiguration> m_CollisionConfiguration;
	std::unique_ptr<btCollisionDispatcher> m_Dispatcher;
	std::unique_ptr<btBroadphaseInterface> m_Broadphase;
	std::unique_ptr<btConstraintSolver> m_Solver;
	std::unique_ptr<btDiscreteDynamicsWorld> m_DynamicsWorld;
	std::unique_ptr<BulletDebugDrawer> m_DebugDrawer;

	Core::Component::TransformComponent *m_TransformComponent;

	Array<BodyEntry> m_Array;
	Array<BulletMotionStateProxy> m_MotionStateProxy;
	Array<BulletRigidBody> m_BulletRigidBody;
	Core::EntityMapper m_EntityMapper;
};

} //namespace Component 
} //namespace Physics 
} //namespace MoonGlare 

#endif

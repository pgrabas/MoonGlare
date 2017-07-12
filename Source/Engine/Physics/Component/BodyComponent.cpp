/*
  * Generated by cppsrc.sh
  * On 2016-08-15 12:19:18,12
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>

#include "../DebugDrawer.h"

#include <Core/Component/ComponentManager.h>
#include <Core/Component/ComponentRegister.h>
#include <Core/Component/AbstractComponent.h>
#include <Core/Component/TransformComponent.h>
#include <Core/Scripts/ScriptComponent.h> 
#include "BodyComponent.h"

#include "Coillision.Events.h"

#include <Math.x2c.h>
#include <ComponentCommon.x2c.h>
#include <BodyComponent.x2c.h>

namespace MoonGlare {
namespace Physics {
namespace Component {

::Space::RTTI::TypeInfoInitializer<BodyComponent, BodyComponent::BodyEntry, BodyComponent::BulletMotionStateProxy, BodyComponent::BulletProxyCommon, BodyComponent::BulletRigidBody> BodyComponentTypeInfo;
Core::Component::RegisterComponentID<BodyComponent> BodyComponentIDReg("Body", false, &BodyComponent::RegisterScriptApi);

BodyComponent::BodyComponent(Core::Component::ComponentManager * Owner) 
		: AbstractComponent(Owner) {

	DebugMemorySetParent(GetManager());
	DebugMemorySetClassName("BodyComponent");
	DebugMemoryRegisterCounter("IndexUsage", [this](DebugMemoryCounter& counter) {
		counter.Allocated = m_Array.Allocated();
		counter.Capacity = m_Array.Capacity();
		counter.ElementSize = sizeof(BodyEntry);
	});

	m_CollisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();
	m_Dispatcher = std::make_unique<btCollisionDispatcher>(m_CollisionConfiguration.get());
	m_Broadphase = std::make_unique<btDbvtBroadphase>();
	m_Solver = std::make_unique<btSequentialImpulseConstraintSolver>();
	m_DynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(m_Dispatcher.get(), m_Broadphase.get(), m_Solver.get(), m_CollisionConfiguration.get());
}

BodyComponent::~BodyComponent() {}

//---------------------------------------------------------------------------------------

void BodyComponent::RegisterScriptApi(ApiInitializer &root) {
//	root
//		.beginClass<InputProcessor>("cInputProcessor")
//			.addFunction("RegisterKeySwitch", &InputProcessor::RegisterKeySwitch)
//		.endClass()
//		;
}

//---------------------------------------------------------------------------------------

bool BodyComponent::Initialize() {
//	m_Array.MemZeroAndClear();

	auto &ed = GetManager()->GetEventDispatcher();

	m_TransformComponent = GetManager()->GetComponent<Core::Component::TransformComponent>();
	if (!m_TransformComponent) {
		AddLogf(Error, "Unable to get TransformComponent instance!");
		return false;
	}

	auto sc = GetManager()->GetComponent<Core::Scripts::Component::ScriptComponent>();
	if (sc) {
		ed.RegisterTemplate<OnCollisionEnterEvent>(sc);
		ed.RegisterTemplate<OnCollisionLeaveEvent>(sc);
	}

	return true;
}

bool BodyComponent::Finalize() {

	m_DynamicsWorld.reset();
	m_Solver.reset();
	m_Broadphase.reset();
	m_Dispatcher.reset();
	m_CollisionConfiguration.reset();

	return true;
}

//---------------------------------------------------------------------------------------

bool BodyComponent::LoadComponentConfiguration(pugi::xml_node node) {
	x2c::Component::BodyComponent::BodyComponentSettings_t bcs;
	bcs.ResetToDefault();
	if (!bcs.Read(node)) {
		AddLog(Error, "Failed to read settings!");
		return false;
	}

	m_DynamicsWorld->setGravity(convert(bcs.m_Gravity));

	return true;
}

//---------------------------------------------------------------------------------------

void BodyComponent::Step(const Core::MoveConfig & conf) {

	if (Config::Current::EnableFlags::PhysicsDebugDraw) {
        DebugDraw(conf);
	}
	
	if(!Config::Current::EnableFlags::Physics)
		return;

	for (size_t i = 0; i < m_Array.Allocated(); ++i) {//ignore root entry
		auto &item = m_Array[i];

		if (!item.m_Flags.m_Map.m_Valid) {
			//mark and continue
			//LastInvalidEntry = i;
			//++InvalidEntryCount;
			continue;
		}

		auto *tcentry = m_TransformComponent->GetEntry(item.m_TransformHandle);
		auto &body = m_BulletRigidBody[i];

		if (item.m_Revision == 0) {
			body.setMotionState(&m_MotionStateProxy[i]);
			item.m_Revision = m_TransformComponent->GetCurrentRevision();
            if (item.m_Flags.m_Map.m_HasShape) {
                auto *shape = ((btRigidBody&)body).getCollisionShape();
                if (shape) {
                    shape->setLocalScaling(tcentry->m_GlobalScale);
                    m_DynamicsWorld->updateSingleAabb(&body);
                }
            }
		} else {
			if (tcentry) {
				if (item.m_Flags.m_Map.m_HasShape && (!item.m_Flags.m_Map.m_Kinematic || tcentry->m_Revision == 0)) {
					if (item.m_Revision != tcentry->m_Revision) {
						((btRigidBody&) body).setWorldTransform(tcentry->m_LocalTransform);
						auto *shape = ((btRigidBody&) body).getCollisionShape();
						if (shape)
							shape->setLocalScaling(tcentry->m_GlobalScale);
						m_DynamicsWorld->updateSingleAabb(&body);
						item.m_Revision = tcentry->m_Revision;
					}
				}
			} else {
				item.m_Flags.m_Map.m_Valid = false;
			}
		}
	}

	struct T {
		static void myTickCallback(btDynamicsWorld *dynamicsWorld, btScalar timeStep) {
			CollisionMap & objectsCollisions = *((CollisionMap*) dynamicsWorld->getWorldUserInfo());
			int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
			for (int i = 0; i < numManifolds; i++) {
				btPersistentManifold *contactManifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
				auto *objA = contactManifold->getBody0();
				auto *objB = contactManifold->getBody1();

				bool ColA = objA->getUserIndex2() != 0;
				bool ColB = objB->getUserIndex2() != 0;

				if (!ColA && !ColB)
					continue;

				auto tA = std::make_tuple(objA, objB);
				auto tB = std::make_tuple(objB, objA);

				int numContacts = contactManifold->getNumContacts();
				for (int j = 0; j < numContacts; j++) {
					btManifoldPoint& pt = contactManifold->getContactPoint(j);
					if (pt.getDistance() > 0.0001f)
						continue;
					if(ColA) objectsCollisions[tA] = &pt;
					if(ColB) objectsCollisions[tB] = &pt;
				}
			}
		}
	};

	CollisionMap cmap;
	m_DynamicsWorld->setInternalTickCallback(&T::myTickCallback, &cmap);

	m_DynamicsWorld->stepSimulation(conf.TimeDelta, 1/*5*/, 1.0f / (60.0f));

	CollisionMap last;
	last.swap(m_LastCollisions);
		   
	auto world = GetManager()->GetWorld();

	while (!cmap.empty()) {
		auto item = cmap.begin();
		auto colit = last.find(item->first);
		if (colit == last.end()) {
			// new
			BodyEntry *be1 = &m_Array[std::get<0>(item->first)->getUserIndex()];
			BodyEntry *be2 = &m_Array[std::get<1>(item->first)->getUserIndex()];
			const std::string *name1 = nullptr, *name2 = nullptr;
			world->GetEntityManager()->GetEntityName(be1->m_OwnerEntity, name1);
			world->GetEntityManager()->GetEntityName(be2->m_OwnerEntity, name2);
		//	AddLogf(Warning, "new col %s->%s", name1 ? name1->c_str() : "?", name2 ? name2->c_str() : "?");

			OnCollisionEnterEvent ev;
			ev.m_Destination = be1->m_OwnerEntity;
			ev.m_Object = be2->m_OwnerEntity;
			ev.m_Normal = item->second->m_normalWorldOnB;
			GetManager()->GetEventDispatcher().SendMessage(ev);

			m_LastCollisions.insert(*item);
		} else {
			// old
			m_LastCollisions.insert(*item);
			last.erase(colit);
		}
		cmap.erase(item);
	}

	for (auto &old : last) {
		BodyEntry *be1 = &m_Array[std::get<0>(old.first)->getUserIndex()];
		BodyEntry *be2 = &m_Array[std::get<1>(old.first)->getUserIndex()];
		const std::string *name1 = nullptr, *name2 = nullptr;
		world->GetEntityManager()->GetEntityName(be1->m_OwnerEntity, name1);
		world->GetEntityManager()->GetEntityName(be2->m_OwnerEntity, name2);
	//	AddLogf(Warning, "old col %s->%s", name1 ? name1->c_str() : "?", name2 ? name2->c_str() : "?");

		OnCollisionLeaveEvent ev;
		ev.m_Destination = be1->m_OwnerEntity;
		ev.m_Object = be2->m_OwnerEntity;
		ev.m_Normal = old.second->m_normalWorldOnB;
		GetManager()->GetEventDispatcher().SendMessage(ev);
	}
}

//---------------------------------------------------------------------------------------

bool BodyComponent::Load(xml_node node, Entity Owner, Handle &hout) {
//	auto *ht = GetManager()->GetWorld()->GetHandleTable();
//	Handle &h = hout;
//	HandleIndex index = m_Allocated++;

	Handle TCHandle;
	if (!m_TransformComponent->GetInstanceHandle(Owner, TCHandle)) {
		AddLogf(Error, "Failed get transform handle!");
		return false;
	}

	size_t index;
	if (!m_Array.Allocate(index)) {
		AddLogf(Error, "Failed to allocate index!");
		return false;
	}
	
	auto &entry = m_Array[index];
	entry.m_Flags.ClearAll();
	if (!GetHandleTable()->Allocate(this, Owner, hout, index)) {
		AddLog(Error, "Failed to allocate handle");
		//no need to deallocate entry. It will be handled by internal garbage collecting mechanism
		return false;
	}

	entry.m_OwnerEntity = Owner;
	entry.m_SelfHandle = hout;

	entry.m_Revision = 0;
	entry.m_TransformHandle = TCHandle;

	auto &body = m_BulletRigidBody[index];
	auto &motionstste = m_MotionStateProxy[index];

	body.Reset(this, hout, Owner);
	body.SetTransform(m_TransformComponent, TCHandle);
	motionstste.Reset(this, hout, Owner);
	motionstste.SetTransform(m_TransformComponent, TCHandle);

//	auto cs = new btBoxShape(vec3(0.5f, 0.5f, 0.5f) * 2.0f);
	//body.setCollisionShape(cs);
	//vec3 internia;

	x2c::Component::BodyComponent::BodyEntry_t bodyentry;
	bodyentry.ResetToDefault();
	if (!bodyentry.Read(node)) {	
		AddLog(Error, "Failed to read BodyEntry!");
		return false;
	}

	entry.m_Mass = bodyentry.m_Mass;
	 
	//cs->calculateLocalInertia(bodyentry.m_Mass, internia);
//	body.setMassProps(bodyentry.m_Mass, internia);
	body.setAngularFactor(convert(bodyentry.m_AngularFactor));
	body.setLinearFactor(convert(bodyentry.m_LinearFactor));
	body.setLinearVelocity(convert(bodyentry.m_LinearVelocity));
	body.setAngularVelocity(convert(bodyentry.m_AngularVelocity));

	//body.setDamping(0.9f, 0.9f);
	body.setFriction(0.9f);
	body.setRollingFriction(0.9f);
	//body.setRestitution(0.9f);

	//Friction = 0.5f;
	//Restitution = 0.3f;
	//Damping.Angular = 0.2f;
	//Damping.Linear = 0.2f;

	//body.setDamping(phprop.Damping.Linear, phprop.Damping.Angular);
	//body.setRestitution(phprop.Restitution);
	//body.setFriction(phprop.Friction);
	body.setSleepingThresholds(0.1f, 0.1f);
//	entry.m_CollisionMask = CollisionMask();		//TODO:
	entry.m_Flags.m_Map.m_Kinematic = bodyentry.m_Kinematic;
	entry.m_Flags.m_Map.m_WantsCollisionEvent = bodyentry.m_WantsCollisionEvent;

	if (entry.m_Mass == 0.0f) {
		body.setActivationState(DISABLE_SIMULATION);
	}

	if (!entry.m_Flags.m_Map.m_Kinematic) {
		body.setActivationState(DISABLE_DEACTIVATION);
	}

	body.setUserPointer(&body);
	body.setUserIndex(index);
	body.setUserIndex2(entry.m_Flags.m_Map.m_WantsCollisionEvent ? 1 : 0);

//	m_DynamicsWorld->addRigidBody(&body);// , (short)entry.m_CollisionMask.Body, (short)entry.m_CollisionMask.Group);

//	entry.m_Flags.m_Map.m_Valid = true;
//	Physics::vec3 pos;
//	XML::Vector::Read(node, "Position", pos);
//	entry.m_LocalTransform.setOrigin(pos);
//	entry.m_LocalTransform.setRotation(Physics::Quaternion(0, 0, 0, 1));
//	XML::Vector::Read(node, "Scale", entry.m_Scale, Physics::vec3(1, 1, 1));
//	entry.m_SelfHandle = h;
//	entry.m_OwnerEntity = Owner;

	m_EntityMapper.SetHandle(Owner, entry.m_SelfHandle);
	entry.m_Flags.m_Map.m_Valid = true;
	return true;
}

bool BodyComponent::SetShape(Handle ShapeHandle, Handle BodyHandle, btCollisionShape * ptr) {
	auto *ht = GetHandleTable();
	HandleIndex index;
	if (!ht->GetHandleIndex(this, BodyHandle, index)) {
		AddLogf(Error, "Cannot set shape to invalid handle!");
		return false;
	}
	if (!ht->IsValid(ShapeHandle) && ptr) {
		AddLogf(Error, "Cannot set shape using invalid handle!");
		return false;
	}

	auto &entry = m_Array[index];
	auto &body = m_BulletRigidBody[index];

	vec3 internia;
	body.setCollisionShape(ptr);
	if (ptr) {
		if (entry.m_Mass > 0.0f) {
			ptr->calculateLocalInertia(entry.m_Mass, internia);
		} else {
			internia = vec3(0, 0, 0);
		}
		body.setMassProps(entry.m_Mass, internia);
      //  if (!entry.m_Flags.m_Map.m_HasShape) {
        m_DynamicsWorld->removeRigidBody(&body);
        m_DynamicsWorld->addRigidBody(&body);
       // }
	} else {
		body.setMassProps(0.0f, internia);
		m_DynamicsWorld->removeRigidBody(&body);
    }
    entry.m_Revision = 0;
    entry.m_Flags.m_Map.m_HasShape = ptr != nullptr;

	return true;
}

//-------------------------------------------------------------------------------------------------

bool BodyComponent::GetInstanceHandle(Entity Owner, Handle & hout) {
	auto h = m_EntityMapper.GetHandle(Owner);
	if (!GetHandleTable()->IsValid(this, h)) {
		return false;
	}
	hout = h;
	return true;
}

BodyComponent::BodyEntry *BodyComponent::GetEntry(Handle h) {
	auto *ht = GetHandleTable();
	HandleIndex hi;
	if (!ht->GetHandleIndex(this, h, hi)) {
		return nullptr;
	}
	return  &m_Array[hi];
}

BodyComponent::BulletRigidBody *BodyComponent::GetRigidBody(Handle h) {
	auto *ht = GetHandleTable();
	HandleIndex hi;
	if (!ht->GetHandleIndex(this, h, hi)) {
		return nullptr;
	}
	return  &m_BulletRigidBody[hi];
}

//-------------------------------------------------------------------------------------------------

void BodyComponent::DebugDraw(const Core::MoveConfig &conf) {
	if (!m_DebugDrawer) {
        m_DebugDrawer = std::make_unique<BulletDebugDrawer>();
		m_DynamicsWorld->setDebugDrawer(m_DebugDrawer.get());
	}

	m_DebugDrawer->Prepare();
	m_DynamicsWorld->debugDrawWorld();
	m_DebugDrawer->Submit(conf);
}

#if 0
disabled code

void GameScene::DoMove(const MoveConfig &conf) {
	BaseClass::DoMove(conf);
	m_Objects.Process(conf);

	struct T {
		static bool t(btManifoldPoint& cp, void* body0,void* body1) {
			if (cp.m_userPersistentData)
				return false;

			cp.m_userPersistentData = body0;
			btCollisionObject *b0 = (btCollisionObject*)body0;
			btCollisionObject *b1 = (btCollisionObject*)body1;
			Object *o0 = (Object*)b0->getUserPointer();
			Object *o1 = (Object*)b1->getUserPointer();

			AddLog(Hint, "contact " << b0 << "@" << o0->GetName() << "   " << b1 << "@" << o1->GetName());
			return false;
		}
		static bool destroy(void* userPersistentData) {
			return true;
		}
	};
	gContactProcessedCallback = &T::t;
	gContactDestroyedCallback = &T::destroy;

	AddLog(Hint, "Step begin");
	auto t = std::chrono::steady_clock::now();
	m_Physics->Step(conf.TimeDelta);
	if (conf.m_SecondPeriod) {
		std::chrono::duration<double> sec = std::chrono::steady_clock::now() - t;
		AddLogf(Performance, "ph step: %f ms", (float)(sec.count() * 1000));
	}

	AddLog(Hint, "Step end");
}

#endif

} //namespace Component 
} //namespace Physics 
} //namespace MoonGlare 

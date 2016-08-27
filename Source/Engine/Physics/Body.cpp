/*
	Generated by cppsrc.sh
	On 2014-12-29 17:42:18,49
	by Paweu
*/
#include <pch.h>
#if 0
#include <MoonGlare.h>

namespace Physics {

SPACERTTI_IMPLEMENT_STATIC_CLASS(Body)

Body::Body(Object* Owner, SharedShape ss): 
		cRootClass(),
		m_Mass(Owner->GetMass()),
		m_Shape(),
		m_ptr(),
		m_Owner(Owner),
		m_World(0),
		m_CenterOfMass(Quaternion(0, 0, 0)){  
	m_ptr.reset(new btRigidBody(1.0f, this, nullptr));

	if (m_Mass > 0.0f)
		m_ptr->forceActivationState(DISABLE_DEACTIVATION);

	m_ptr->setUserPointer(m_Owner);
	SetShape(ss);

	SetAngularFactor(vec3(1, 1, 1));
	SetLinearFactor(vec3(1, 1, 1));
}

Body::~Body() {
	SetWorldOwner(nullptr);
}

void Body::SetMass(float value) {
	m_Mass = value;
	if (!m_Shape) return;
	vec3 localInertia;
	m_Shape->CalculateLocalInertia(value, localInertia);
	m_ptr->setMassProps(value, localInertia);
	if (m_Mass > 0.0f)
		m_ptr->forceActivationState(DISABLE_DEACTIVATION);
	else
		m_ptr->forceActivationState(WANTS_DEACTIVATION);
}

void Body::SetShape(SharedShape Shape) {
	bool hadshape = static_cast<bool>(m_Shape);
	m_Shape = Shape;
	if (Shape)
		m_ptr->setCollisionShape(m_Shape->GetRawShape());
	else
		m_ptr->setCollisionShape(nullptr);
	SetMass(GetMass());
	if (Shape && !hadshape && m_World)
		m_World->AddBody(this);
}

void Body::SetWorldOwner(PhysicEngine *Owner) {
	if (m_World == Owner)
		return;
	
	if (m_World)
		m_World->RemoveBody(this);

	m_World = Owner;

	if (!m_World)
		return;

	if (!m_Shape) {
		AddLog(Warning, "Cannot add shapeless body to world!");
		return;
	}

	m_World->AddBody(this);
}

void Body::SetPhysicalProperties(const PhysicalProperties& phprop) {
	m_ptr->setDamping(phprop.Damping.Linear, phprop.Damping.Angular);
	m_ptr->setRestitution(phprop.Restitution);
	m_ptr->setFriction(phprop.Friction);
}

///synchronizes world transform from user to physics
void Body::getWorldTransform(btTransform& centerOfMassWorldTrans) const {
	centerOfMassWorldTrans = m_Owner->GetPositionTransform() * m_CenterOfMass.inverse();
}

///synchronizes world transform from physics to user
///Bullet only calls the update of worldtransform for active objects
void Body::setWorldTransform(const btTransform& centerOfMassWorldTrans) {
	m_Owner->SetPositionTransform(centerOfMassWorldTrans * m_CenterOfMass);
}

} //namespace Physics 
#endif
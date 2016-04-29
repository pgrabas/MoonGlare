/*
	Generated by cppsrc.sh
	On 2014-12-29 17:42:18,49
	by Paweu
*/

#pragma once
#ifndef Body_H
#define Body_H

namespace Physics {

class Body : public cRootClass, public btMotionState {
	GABI_DECLARE_CLASS_NOCREATOR(Body, cRootClass);
	friend struct BodyConstructor;
public:
	static BodyPtr Create(Object* Owner, SharedShape ss) { return BodyPtr(new Body(Owner, ss)); }
	~Body();

	void SetWorldOwner(PhysicEngine *Owner);
	PhysicEngine* GetWorldOwner() const { return m_World; }

	void SetPhysicalProperties(const PhysicalProperties& phprop);
	
	/** Set the ease with which the rotation will be made */
	void SetAngularFactor(const vec3 &vec) { m_ptr->setAngularFactor(vec); } 
	/** Set the ease with which the movement will be made */
	void SetLinearFactor(const vec3 &vec) { m_ptr->setLinearFactor(vec); } 

	const vec3& GetLinearVelocity() { return m_ptr->getLinearVelocity(); }
	void SetLinearVelocity(const vec3 &vec) { m_ptr->setLinearVelocity(vec); } 

	void UpdateMotionState() { m_ptr->setMotionState(m_ptr->getMotionState()); }

	void ApplyCentralForce(const vec3 &vec) { m_ptr->applyCentralForce(vec); } 

	/** Get current collision mask configuration */
	CollisionMask GetCollisionMask() const { return m_CollisionMask; }
	/** Set collision mask */
	void SetCollisionMask(CollisionMask mc) { m_CollisionMask = mc; }

	void SetMass(float value);
	float GetMass() const { return m_Mass; }
	void SetShape(SharedShape Shape);
	const SharedShape& GetShape() const { return m_Shape; }

	Object* GetOwner() { (Object*)m_ptr->getUserPointer(); }
	btRigidBody *GetRawBody() { return m_ptr.get(); }

	///synchronizes world transform from user to physics
	virtual void getWorldTransform(btTransform& centerOfMassWorldTrans) const override;
	///synchronizes world transform from physics to user
	///Bullet only calls the update of worldtransform for active objects
	virtual void setWorldTransform(const btTransform& centerOfMassWorldTrans) override;
protected:
	Body(Object* Owner, SharedShape ss);

	Transform m_CenterOfMass;

	SharedShape m_Shape;
	float m_Mass;
	CollisionMask m_CollisionMask;
	std::unique_ptr<btRigidBody> m_ptr;
	Object *m_Owner;
private: 
	PhysicEngine *m_World;
};

} //namespace Physics 

#endif

/*
  * Generated by cppsrc.sh
  * On 2015-10-25 21:57:04,09
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef TransformComponent_H
#define TransformComponent_H

namespace MoonGlare {
namespace Core {
namespace Component {

class TransformComponent : public AbstractComponent {
public:
	virtual bool Initialize() override;
	virtual bool Finalize() override;

	struct TransformEntry : public btMotionState {
		math::mat4 m_LocalMatrix;
		math::mat4 m_GlobalMatrix;
		float m_LocalScale;
		float m_GlobalScale;

		Entity m_Owner;

		Physics::Transform m_Transform;
		Physics::Transform m_CenterOfMass;
		///synchronizes world transform from user to physics
		void getWorldTransform(btTransform& centerOfMassWorldTrans) const {
			centerOfMassWorldTrans = m_Transform * m_CenterOfMass.inverse();
		}
		///synchronizes world transform from physics to user
		///Bullet only calls the update of worldtransform for active objects
		void setWorldTransform(const btTransform& centerOfMassWorldTrans) {
			m_Transform = centerOfMassWorldTrans * m_CenterOfMass;
		}
	};
	 
	template<class T> using Array = std::array<T, Configuration::Storage::Static::TransformComponent>;
	Array<TransformEntry> m_PositionData;

	TransformEntry* GetForEntry(Entity e);


/*
	btTransform
	
	d3		position
	d4		quaternion
	mat3x3	matrix
	?scale

	//process (position & quaternion) -> matrix  [done by physics engine]
	//static & dynamic <- future optimalisation
	indirect 
*/

	//static Entity Allocate();
	//static void Release(Entity e);
protected:
private:
};

} //namespace Component 
} //namespace Core 
} //namespace MoonGlare 

#endif

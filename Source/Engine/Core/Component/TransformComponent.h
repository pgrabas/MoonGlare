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
	TransformComponent(ComponentManager *Owner);
	virtual ~TransformComponent();
	virtual bool Initialize() override;
	virtual bool Finalize() override;
	virtual void Step(const MoveConfig &conf) override;
	virtual bool PushEntryToLua(Handle h, lua_State *lua, int &luarets) override;
	virtual bool Load(xml_node node, Entity Owner, Handle &hout) override;
	virtual bool GetInstanceHandle(Entity Owner, Handle &hout) override;

	constexpr static ComponentID GetComponentID() { return 2; };
	constexpr static uint16_t GetHandleType() { return 2; };

	union FlagsMap {
		struct MapBits_t {
			bool m_Valid : 1; //Entity is not valid or requested to be deleted;

		};
		MapBits_t m_Map;
		uint32_t m_UintValue;

		void SetAll() { m_UintValue = 0; m_UintValue = ~m_UintValue; }
		void ClearAll() { m_UintValue = 0; }

		static_assert(sizeof(MapBits_t) <= sizeof(decltype(m_UintValue)), "Invalid Function map elements size!");
	};

	struct TransformEntry {
		Handle m_SelfHandle;
		Entity m_OwnerEntity;
		FlagsMap m_Flags;
		math::mat4 m_LocalMatrix;
		math::mat4 m_GlobalMatrix;
//		Physics::vec3 m_LocalScale;
//		Physics::vec3 m_GlobalScale;

		math::Transform m_LocalTransform;
//		Physics::Transform m_GlobalTransform;
//		Physics::Transform m_CenterOfMass;

		math::vec3 GetPosition() const { return convert(m_LocalTransform.getOrigin()); }
		void SetPosition(math::vec3 pos) { m_LocalTransform.setOrigin(convert(pos)); }
//		void SetPosition(Physics::vec3 pos) {  }
		math::vec4 GetRotation() const { return convert(m_LocalTransform.getRotation()); }
		void SetRotation(math::vec4 rot) { m_LocalTransform.setRotation(convert(rot)); }
	};

//	struct BulletMotionStateProxy : public btMotionState {
//		///synchronizes world transform from user to physics
//		virtual void getWorldTransform(btTransform& centerOfMassWorldTrans) const override;
//		///synchronizes world transform from physics to user
//		///Bullet only calls the update of worldtransform for active objects
//		virtual void setWorldTransform(const btTransform& centerOfMassWorldTrans) override;
//	};

	TransformEntry* GetEntry(Handle h);	 //return nullptr if h/e is not valid
	TransformEntry* GetEntry(Entity e);	 //return nullptr if h/e is not valid

/*
	btTransform
	
	d3		position`
	d4		quaternion
	mat3x3	matrix
	?scale

	//process (position & quaternion) -> matrix  [done by physics engine]
	//static & dynamic <- future optimization
	indirect 
*/
	static void RegisterScriptApi(ApiInitializer &root);
protected:
	template<class T> using Array = std::array<T, Configuration::Storage::ComponentBuffer>;
	Array<TransformEntry> m_Array;
	EntityMapper m_EntityMapper;
	std::atomic<size_t> m_Allocated;
//	Array<BulletMotionStateProxy> m_Proxies;
	void ReleaseElement(size_t Index);
};

} //namespace Component 
} //namespace Core 
} //namespace MoonGlare 

#endif

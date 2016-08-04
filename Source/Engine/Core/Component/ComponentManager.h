/*
  * Generated by cppsrc.sh
  * On 2015-08-20 17:27:44,24
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef ComponentManager_H
#define ComponentManager_H

namespace MoonGlare {
namespace Core {
namespace Component {

class ComponentManager final {
public:
	ComponentManager();
	~ComponentManager();

	bool Initialize(ciScene *scene);
	bool Finalize();

	template<class T, class ... ARGS>
	bool InstallComponent(ARGS ... args) {
		return InsertComponent(std::make_unique<T>(this, std::forward<ARGS>(args)...), T::GetComponentID());
	}

	void Step(const MoveConfig &config);

	template<class T> 
	T* GetComponent() {
		return dynamic_cast<T*>(GetComponent(T::GetComponentID));
	}

	AbstractComponent* GetComponent(ComponentID cid);

	ciScene* GetScene() { return m_Scene; }
	World* GetWorld() { return m_World; }
private:
	std::array<UniqueAbstractComponent, Configuration::Storage::MaxComponentCount> m_Components;
	std::array<ComponentID, Configuration::Storage::MaxComponentCount> m_ComponentsIDs;
	size_t m_UsedCount;
	ciScene *m_Scene;
	World *m_World;

	bool InsertComponent(UniqueAbstractComponent cptr, ComponentID cid);
};

} //namespace Component 
} //namespace Core 
} //namespace MoonGlare 

#endif
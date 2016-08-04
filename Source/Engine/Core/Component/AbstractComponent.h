/*
  * Generated by cppsrc.sh
  * On 2015-08-19 22:57:41,79
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef AbstractComponent_H
#define AbstractComponent_H

namespace MoonGlare {
namespace Core {
namespace Component {

class AbstractComponent {
public:
	AbstractComponent(ComponentManager *Owner);
	virtual ~AbstractComponent();

	virtual bool Initialize() = 0;
	virtual bool Finalize() = 0;

	virtual void Step(const MoveConfig &conf) = 0;


//	virtual bool IsHandleValid(Handle h) = 0;
//	virtual bool Allocate(Entity owner, Handle &out) = 0;
//	virtual bool Release(Handle h) = 0;
//	virtual bool Release(Entity e) = 0;
//	virtual Handle GetHandle(Entity e) = 0;

	ComponentManager* GetManager() { return m_Owner; }

	virtual Handle Load(xml_node node, Entity Owner) = 0;
protected:
private:
	ComponentManager *m_Owner;
	void *m_padding;
};

} //namespace Component 
} //namespace Core 
} //namespace MoonGlare 

#endif
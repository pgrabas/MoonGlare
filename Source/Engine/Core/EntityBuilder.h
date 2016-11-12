/*
  * Generated by cppsrc.sh
  * On 2016-08-14 22:04:32,35
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef EntityBuilder_H
#define EntityBuilder_H

namespace MoonGlare {
namespace Core {

class EntityBuilder {
public:
 	EntityBuilder(Component::ComponentManager *Manager);
 	~EntityBuilder();

	static bool Build(Component::ComponentManager *Manager, Entity Owner, const char *PatternUri, Entity &eout, std::string Name) {
		EntityBuilder eb(Manager);
		return eb.Build(Owner, PatternUri, eout, std::move(Name));
	}

	bool Build(Entity Owner, const char *PatternUri, Entity &eout, std::string Name);

//xml
	/** Returns count of processed elements */
	unsigned BuildChild(Entity Owner, pugi::xml_node node, Entity &eout, std::string Name);
	/** Returns count of processed elements */
	unsigned ProcessXML(Entity Owner, pugi::xml_node node);
	/** Returns count of loaded components */
	unsigned LoadComponents(Entity Owner, pugi::xml_node node);
	/** Returns true on success */
	bool LoadComponent(Entity Owner, pugi::xml_node node, Handle &hout);
protected:
	Component::ComponentManager *m_Manager;
};

} //namespace Core 
} //namespace MoonGlare 

#endif

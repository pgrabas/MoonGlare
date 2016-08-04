/*
  * Generated by cppsrc.sh
  * On 2016-05-03 21:31:13,10
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef World_H
#define World_H

#include "Core/EntityManager.h"
#include "Core/HandeTable.h"

namespace MoonGlare {

class World {
public:
 	World();
 	~World();

	bool Initialize(::Core::cScriptEngine *se);
	bool Finalize();

	Core::EntityManager* GetEntityManager() { return &m_EntityManager; }

	::Core::cScriptEngine *GetScriptEngine() { return m_ScriptEngine; };
private: 
	::Core::cScriptEngine *m_ScriptEngine;
	Core::HandeTable m_HandleTable;
	Core::EntityManager m_EntityManager;
};

} //namespace MoonGlare 

#endif
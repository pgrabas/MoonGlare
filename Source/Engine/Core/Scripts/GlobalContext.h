/*
  * Generated by cppsrc.sh
  * On 2015-03-24 17:28:03,29
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef GlobalContext_H
#define GlobalContext_H

namespace Core {
namespace Scripts {

class ContextGroup;

class GlobalContext : public cRootClass {
	SPACERTTI_DECLARE_CLASS_SINGLETON(GlobalContext, cRootClass);
public:
 	GlobalContext();
 	virtual ~GlobalContext();

	bool Install(lua_State *lua);

	bool Initialize();
	bool Finalize();

	void CleanContext();

#ifdef DEBUG_DUMP
	void Dump(std::ostream& out);
	void LogDump();
#endif

	static void RegisterScriptApi(::ApiInitializer &api);
protected:
//	std::unique_ptr<ContextGroup> m_GlobalBase;
	std::unique_ptr<ContextGroup> m_PermanentBase;

//	static ContextGroup* GetGlobalContext() { return Instance()->m_GlobalBase.get(); }
	static ContextGroup* GetPermanentContext() { return Instance()->m_PermanentBase.get(); }
private: 
};

} //namespace Scripts 

inline Scripts::GlobalContext* GetGlobalContext() { return Scripts::GlobalContext::Instance(); }

} //namespace Core 

#endif

#include <pch.h>
#include <MoonGlare.h>
#include <Engine/Core/DataManager.h>
#include "GlobalContext.h"

namespace Core {
namespace Scripts {

SPACERTTI_IMPLEMENT_CLASS_SINGLETON(cScriptEngine)
RegisterApiInstance(cScriptEngine, &cScriptEngine::Instance, "ScriptEngine");
RegisterApiDerivedClass(cScriptEngine, &cScriptEngine::RegisterScriptApi);

cScriptEngine::cScriptEngine() :
		cRootClass(),
		m_CurrentGCStep(1),
		m_CurrentGCRiseCounter(0),
		m_LastMemUsage(0),
		m_Flags(0) {
	SetThisAsInstance();

	::OrbitLogger::LogCollector::SetChannelName(OrbitLogger::LogChannels::Script, "SCRI");
	::OrbitLogger::LogCollector::SetChannelName(OrbitLogger::LogChannels::ScriptCall, "SCCL", false);
}

cScriptEngine::~cScriptEngine() {
	Finalize();
}

//---------------------------------------------------------------------------------------

void cScriptEngine::Step(const MoveConfig &config) {

	//TBD
	if (!IsScriptsLoaded()) {
		return;
	}

	auto t = std::chrono::steady_clock::now();
	{
		auto &mutex = m_Script->GetMutex();
		LOCK_MUTEX(mutex);
		auto sc = luabridge::getGlobal(m_Script->GetLuaState(), "ScriptComponent");
		sc["Step"](&config);
		lua_gc(m_Script->GetLuaState(), LUA_GCSTEP, m_CurrentGCStep);
	}
	if (config.m_SecondPeriod) {
		AddLogf(Performance, "Lua step: %f ms", (float)((std::chrono::steady_clock::now() - t).count() * 1000.0f));
	}

	if (config.m_SecondPeriod) {
		float memusage = m_Script->GetMemoryUsage();
		if (memusage > (m_LastMemUsage + 10.0f) ) {
			m_LastMemUsage = memusage;
			++m_CurrentGCRiseCounter;
			if (m_CurrentGCRiseCounter == 5) {
				++m_CurrentGCStep;
				m_CurrentGCRiseCounter = 0;
				AddLogf(Debug, "New Lua GC step: %d", m_CurrentGCStep);
			}
		}

#ifdef PERF_PERIODIC_PRINT
		AddLogf(Performance, "Lua memory usage: %6.2f kbytes", memusage);
#endif
	}

}

//---------------------------------------------------------------------------------------

void cScriptEngine::RegisterScriptApi(ApiInitializer &root) {
	root
	.deriveClass<ThisClass, BaseClass>("cScriptEngine")
		.addFunction("LoadCode", &ThisClass::LoadCode)
		.addFunction("DefferExecution", &ThisClass::DefferExecution)
#ifdef DEBUG_SCRIPTAPI
		.addFunction("CollectGarbage", &ThisClass::CollectGarbage)
		.addFunction("PrintMemoryInfo", &ThisClass::PrintMemoryInfo)
#endif
	.endClass()
	.beginClass<MoveConfig>("cMoveConfig")
		.addData("TimeDelta", &MoveConfig::TimeDelta, false)
		.addData("SecondPeriod", &MoveConfig::m_SecondPeriod, false)
	.endClass();
}

//---------------------------------------------------------------------------------------

void cScriptEngine::DefferExecution(string fname, int parameter) {
	JobQueue::QueueJob([fname, parameter]() {
		ScriptProxy::RunFunction<int>(fname.c_str(), parameter);
	});
}

//---------------------------------------------------------------------------------------

void cScriptEngine::CollectGarbage() {
	if (m_Script)
		m_Script->CollectGarbage();
}

void cScriptEngine::PrintMemoryInfo() {
	if (m_Script)
		m_Script->PrintMemoryUsage();
}

//---------------------------------------------------------------------------------------

bool cScriptEngine::Initialize() {
	if (IsReady()) return true;

	new GlobalContext();
	GlobalContext::Instance()->Initialize();

	LOCK_MUTEX(m_Mutex);

	AddLog(Debug, "Constructing script object");
	auto s = std::make_shared<Script>();
	if (!s->Initialize()) {
		AddLog(Error, "Unable to initialize script instance!");
		s.reset();
		return false;
	}

	for (auto &it : m_ScriptCodeList) {
		s->LoadCode(it.Data.c_str(), it.Data.length(), it.Name.c_str());
	}
	m_Script.swap(s);

	auto lua = m_Script->GetLuaState();
	luabridge::Stack<cScriptEngine*>::push(lua, this);
	lua_setglobal(lua, "Script");

	lua_pushlightuserdata(lua, (void *)this);  /* push address */
	lua_createtable(lua, 0, 0);
	lua_settable(lua, LUA_REGISTRYINDEX);

	AddLog(Debug, "Script construction finished");

	SetReady(true);
	return true;
}

bool cScriptEngine::Finalize() {
	if (!IsReady()) return true;
	SetReady(false);

	LOCK_MUTEX(m_Mutex);

	AddLog(Debug, "Destroying script object");
	SharedScript s;
	s.swap(m_Script);
	if (!s->Finalize()) {
		AddLog(Warning, "An error has occur during finalization of script instance!");
	}

	s.reset();

	GlobalContext::Instance()->Finalize();
	GlobalContext::DeleteInstance();

	AddLog(Debug, "Destruction finished");
	return true;
}

//---------------------------------------------------------------------------------------

bool cScriptEngine::CreateScript(const std::string& Class, Entity Owner) {
	if (!IsScriptsLoaded()) {
		//TODO: log fatal error!
		return false;
	}

	{
		auto &mutex = m_Script->GetMutex();
		LOCK_MUTEX(mutex);
		auto sc = luabridge::getGlobal(m_Script->GetLuaState(), "ScriptComponent");
		auto ret = sc["AllocScript"](&Owner, Class.c_str());
	//	return ret.cast<int>() > 0;
	}

	return true;
}

//---------------------------------------------------------------------------------------

void cScriptEngine::LoadAllScriptsImpl() {
	FileSystem::FileInfoTable files;
	if (!GetFileSystem()->EnumerateFolder(DataPath::Scripts, files, true)) {
		AddLog(Error, "Unable to look for scripts!");
	}

	using FileInfo = MoonGlare::FileSystem::FileInfo;
	std::sort(files.begin(), files.end(), [](const FileInfo &a, const FileInfo &b) -> bool{
		return a.m_RelativeFileName < b.m_RelativeFileName;
	});

	for (auto &it : files){
		if (it.m_IsFolder)
			continue;

		string path;
		FileSystem::DataSubPaths.Translate(path, it.m_RelativeFileName, DataPath::Scripts);
		RegisterScript(path);
	}

	SetScriptsLoaded(true);

	GetDataMgr()->NotifyResourcesChanged();
	AddLog(Debug, "Finished looking for scripts");
}

void cScriptEngine::LoadAllScripts() {
	JobQueue::QueueJob([this]() { LoadAllScriptsImpl(); });
}

void cScriptEngine::RegisterScript(string Name) {
	ScriptCode *last;
	{
		ScriptCode sc_value;
		sc_value.Name.swap(Name);
		sc_value.Type = ScriptCode::Source::File;
		LOCK_MUTEX(m_Mutex);
		m_ScriptCodeList.emplace_back(std::move(sc_value));
		last = &m_ScriptCodeList.back();
	}

	ScriptCode &sc = *last;
	
	if (sc.Data.empty()) {
		StarVFS::ByteTable data;
		if (!GetFileSystem()->OpenFile(sc.Name, DataPath::Root, data)) {
			AddLog(Warning, "Unable to open script file " << sc.Name);
			return;
		}
		sc.Data = string(data.get(), data.size());
		AddLogf(Debug, "Loaded script code: %s", sc.Name.c_str());
	}

	if (m_Script) {
		try {
			m_Script->LoadCode(sc.Data.c_str(), sc.Data.length(), sc.Name.c_str());
		}
		catch (...) {
			AddLog(Error, "Unexpected exception while loading script code!");
		}
	}
}

void cScriptEngine::SetCode(const string& ChunkName, string Code) {
	ScriptCode *item = nullptr;
	LOCK_MUTEX(m_Mutex);
	{
		for (auto &it : m_ScriptCodeList) {
			if (it.Name == ChunkName) {
				item = &it;
				break;
			}
		}

		if(!item) {
			AddLogf(Debug, "Chunk '%s' not found. Creating new", ChunkName.c_str());
			ScriptCode sc;
			sc.Name = ChunkName;
			sc.Type = ScriptCode::Source::Code;
			m_ScriptCodeList.emplace_back(std::move(sc));
			item = &m_ScriptCodeList.back();
		}
	}

	ScriptCode &sc = *item;
	sc.Data.swap(Code);
	AddLogf(Debug, "Exchanged code for chunk '%s'", ChunkName.c_str());
}

//---------------------------------------------------------------------------------------

void cScriptEngine::LoadCode(string code) {
	ScriptCode *last;
	{
		ScriptCode sc;
		sc.Type = ScriptCode::Source::Code;
		sc.Data.swap(code);
		char buffer[64];
		sprintf(buffer, "Code_%d", m_ScriptCodeList.size());
		sc.Name = buffer;
		LOCK_MUTEX(m_Mutex);
		m_ScriptCodeList.emplace_back(std::move(sc));
		last = &m_ScriptCodeList.back();
	}

	try {
		LOCK_MUTEX(m_Mutex);
		if (m_Script)
			m_Script->LoadCode(last->Data.c_str(), last->Data.length(), last->Name.c_str());
	}
	catch (...) {
		AddLog(Error, "Unexpected exception while broadcasting script code!");
	}

	GetDataMgr()->NotifyResourcesChanged();
}

//---------------------------------------------------------------------------------------

bool cScriptEngine::ConstructScript() {
	LOCK_MUTEX(m_Mutex);

	if (m_Script)
		return true;

	AddLog(Debug, "Constructing script object");
	auto s = std::make_shared<Script>();
	if(!s->Initialize()) {
		AddLog(Error, "Unable to initialize script instance!");
		s.reset();
		return false;
	}

	for (auto &it : m_ScriptCodeList) {
		s->LoadCode(it.Data.c_str(), it.Data.length(), it.Name.c_str());
	}
	m_Script.swap(s);

	AddLog(Debug, "Construction finished");
	return true;
}

bool cScriptEngine::DestroyScript() {
	LOCK_MUTEX(m_Mutex);
	if (!m_Script) {
		//unexpected case, silently ignore
		return true;
	}

	AddLog(Debug, "Destroying script object");
	SharedScript s;
	s.swap(m_Script);
	if (!s->Finalize()) {
		AddLog(Warning, "An error has occur during finalization of script instance!");
	}

	s.reset();

	AddLog(Debug, "Destruction finished");
	return true;
}

//---------------------------------------------------------------------------------------

bool cScriptEngine::InitializeScriptProxy(ScriptProxy &proxy, SharedScript& ptr) {
	AddLog(Hint, "Thread requested script instance");
	LOCK_MUTEX(m_Mutex);

	if (!m_Script) {
		return false;
	}

	ptr = m_Script;
	m_ScriptList.push_back(proxy.shared_from_this());

	return true;
}

bool cScriptEngine::FinalizeScriptProxy(ScriptProxy &proxy, SharedScript& ptr) {
	AddLog(Hint, "Thread returned script instance");
//	{
//		LOCK_MUTEX(m_Mutex);
//		for (auto &it: m_ScriptList)
//			if (it.first == ptr.get()) {
//				it.second = nullptr;
//				break;
//			}
//	}
	ptr.reset();

	return true;
}

//---------------------------------------------------------------------------------------

void cScriptEngine::EnumerateScripts(EnumerateFunc func) {
	LOCK_MUTEX(m_Mutex);
	for (auto &it : m_ScriptCodeList)
		func(it);
}

//---------------------------------------------------------------------------------------

#ifdef DEBUG_DUMP

void cScriptEngine::DumpScripts(std::ostream &out) {
	out << "Registered scripts:\n";
	{
		LOCK_MUTEX(m_Mutex);
		for (auto &it : m_ScriptCodeList) {
			out << "\t" << it.Name << "  length: " << it.Data.length() << " bytes\n";
		}
	}
	out << "\n";
}

#endif

} //namespace Scripts
} //namespace Core

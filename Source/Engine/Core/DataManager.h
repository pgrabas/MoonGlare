/*
 * cModuleManager.h
 *
 *  Created on: 08-12-2013
 *      Author: Paweu
 */

#ifndef CDATAMANAGER_H_
#define CDATAMANAGER_H_

namespace MoonGlare {
namespace Core {
namespace Data {

#define DEFAULT_FONT_NAME	"Arial"
#define PLAYER_XML_FILE		"Player00.xml"

struct PredefObjectMeta {
	string Class;
	FileSystem::XMLFile Meta;
};

template<class T>
struct SynchronizedResourceMap {
	using Map_t = std::unordered_map < string, T > ;
	struct MapProxy {
		Map_t* operator->() { 
			THROW_ASSERT(m_Map, "Map proxy is not valid!");
			return m_Map; 
		}

		Map_t& operator*() {
			THROW_ASSERT(m_Map, "Map proxy is not valid!");
			return *m_Map; 
		}

		template<class V>
		T& operator[](V &&v) { 
			THROW_ASSERT(m_Map, "Map proxy is not valid!");
			return (*m_Map)[std::forward<V>(v)]; 
		}

		MapProxy(Map_t &map, std::mutex &mutex) : m_Map(&map), m_Lock(mutex) {  }
		MapProxy(MapProxy&& other): m_Map(nullptr), m_Lock() {
			m_Lock.swap(other.m_Lock);
			std::swap(m_Map, other.m_Map);
		}
		MapProxy& operator=(const MapProxy&) = delete;
		MapProxy() = delete;
		~MapProxy() { }

		void unlock() {
			m_Map = nullptr;
			std::unique_lock<std::mutex> l;
			m_Lock.swap(l);
		}
	private:
		Map_t *m_Map;
		std::unique_lock<std::mutex> m_Lock;
	};

	MapProxy Lock() {
		return MapProxy(m_Map, m_Mutex);
	}
	void clear() { Lock()->clear(); }
private:
	Map_t m_Map;
	std::mutex m_Mutex;
};

using UniqueModule = std::unique_ptr < DataModule > ;

class Manager : public cRootClass {
	friend class DataManagerDebugScritpApi;
	GABI_DECLARE_CLASS_SINGLETON(Manager, cRootClass)
public:
	typedef std::vector<UniqueModule> ModuleVector;

	Manager();
	virtual ~Manager();

	/** Takes ownership of the module */
	bool ImportModule(UniqueModule &module);

	bool LoadPlayer();
	void LoadGlobalData();

	DataClasses::FontPtr GetConsoleFont();
	DataClasses::FontPtr GetDefaultFont();
	const DataClasses::ModuleSettings& GetConfig() const { return m_Config; }

	DataClasses::MapPtr GetMap(const string& Name);
	DataClasses::FontPtr GetFont(const string &Name);
	Object* LoadObject(const string& Name, ::Core::GameScene *Owner);
	DataClasses::ModelPtr GetModel(const string& Name);

	const string& GetString(const string &Id, const string& TableName);
	::Core::ciScene* Manager::LoadScene(const string& Name, const string& Class) const;

	bool Initialize();
	bool Finalize();

	struct Flags {
		enum {
			Initialized		= 0x0010,
		};
	};

	DefineFlagGetter(m_Flags, Flags::Initialized, Initialized);

	void DumpAllResources(std::ostream &out);
	/** Reprint all resources */
#ifdef DEBUG_DUMP
	void NotifyResourcesChanged();
#else
	void NotifyResourcesChanged() { }
#endif

	static void RegisterScriptApi(::ApiInitializer &api);
private:
	unsigned m_Flags;
	ModuleVector m_Modules;

	DataClasses::ModuleSettings m_Config;

	SynchronizedResourceMap<FontResPtr> m_Fonts;
	SynchronizedResourceMap<PredefObjectMeta> m_PredefObjects;
	SynchronizedResourceMap<ModelResPtr> m_Models;
	
	std::unordered_map<string, MapResPtr> m_Maps;

	std::unique_ptr<DataClasses::StringTable> m_StringTables;

	template <class C> 
	void ImportResources(DataModule *module, bool(DataModule::*srcfun)(DataClasses::NameClassList&)const, std::unordered_map<string, C> &container);

	DefineFlagSetter(m_Flags, Flags::Initialized, Initialized);

	void DumpResources();
};

} // namespace Data
} // namespace Core 

inline MoonGlare::Core::Data::Manager* GetDataMgr() { return MoonGlare::Core::Data::Manager::Instance(); }

} // namespace MoonGlare 

#endif // CMODULEMANAGER_H_ 

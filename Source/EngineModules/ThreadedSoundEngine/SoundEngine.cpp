/*
	Generated by cppsrc.sh
	On 2015-01-22  6:51:10,16
	by Paweu
*/
#include <pch.h>
#include <MoonGlare.h>
#include <Engine/ModulesManager.h>
#include <Engine/iSoundEngine.h>
#include "SoundEngine.h"

#include <Utils/LuaUtils.h>

namespace MoonGlare {
namespace Modules {
namespace ThreadedSoundEngine {

struct SoundEngineSettings {
	static float xclamp(float f) { return math::clamp < float > (f, 0.0f, 1.0f); };

	struct Volume {
		struct Master : public Settings_t::BaseSettingInfo<float, Master> {
			static Type default() { return 1.0f; }
			static void set(Type v) { _value = xclamp(v);  }
		};
		struct Music : public Settings_t::BaseSettingInfo<float, Music> {
			static Type default() { return 0.7f; }
			static void set(Type v) { _value = xclamp(v);  }
		};
		struct Sound : public Settings_t::BaseSettingInfo<float, Sound> {
			static Type default() { return 1.0f; }
			static void set(Type v) { _value = xclamp(v);  }
		};
	};
	struct Enabled : public Settings_t::BaseSettingInfo<bool, Enabled> {
		static Type default() { return true; }
	};
};

struct ThreadedSoundEngineModule : public MoonGlare::Modules::ModuleInfo {
	ThreadedSoundEngineModule(): BaseClass("ThreadedSoundEngine", ModuleType::Core) { 
#define Setting Settings_t::DirectSettingManipulator
		Settings.RegisterDynamicSetting<Setting<SoundEngineSettings::Enabled>>("Sound.Enabled", false, SettingsGroup::Sound);
		Settings.RegisterDynamicSetting<Setting<SoundEngineSettings::Volume::Master>>("Sound.Volume.Master", false, SettingsGroup::Sound_Volume);
		Settings.RegisterDynamicSetting<Setting<SoundEngineSettings::Volume::Sound>>("Sound.Volume.Sound", false, SettingsGroup::Sound_Volume);
		Settings.RegisterDynamicSetting<Setting<SoundEngineSettings::Volume::Music>>("Sound.Volume.Music", false, SettingsGroup::Sound_Volume);
#undef Setting
	}

	virtual bool Initialize() override {
		return (new SoundEngine())->Initialize();
	}
	virtual bool Finalize() override {
		bool result = true;
		if (SoundEngine::InstanceExists()) {
			result = GetSoundEngine()->Finalize();
			SoundEngine::DeleteInstance();
		}
		return result;
	}

	virtual void Notify(SettingsGroup what) override {
		if (!iSoundEngine::InstanceExists())
			return; //ignore if instance does not exists
		auto se = dynamic_cast<SoundEngine*>(GetSoundEngine());
		if (!se)
			return; // ignore if another class of sound engine is set
		se->ConfigurationChanged(what);
	}
};
DEFINE_MODULE(ThreadedSoundEngineModule);

//----------------------------------------------------------------------------------

/*
	TODO for SoundEngine

	Music volume sliding
	3d sound support
*/

SPACERTTI_IMPLEMENT_STATIC_CLASS(SoundEngine);
RegisterApiDerivedClass(SoundEngine, &SoundEngine::RegisterScriptApi);

SoundEngine::SoundEngine():
		BaseClass(),
		m_ThreadRunning(false),
		m_Thread(),
		m_CurrentMusics(),
		m_CurrentPlayListItem(0),
		m_PlayList() {
}

SoundEngine::~SoundEngine() {
	if (m_Thread.joinable()) {
		m_Thread.join();
	}
}

//----------------------------------------------------------------------------------

#ifdef DEBUG_DUMP
void SoundEngine::DumpContent(std::ostream &out) {
	{
		LOCK_MUTEX(m_MusicMutex);
		out << "Musisc:\n";
		for (auto &it : m_Music) {
			auto &sd = it.second;
			char buf[4096];
			sprintf(buf, "%60s [class %-5s][File: %s]\n", 
					sd.m_Name.c_str(), sd.m_Class.c_str(), sd.m_FileName.c_str());
			out << buf;
		}
		out << "\n";
	}
	{
		LOCK_MUTEX(m_SoundsMutex);
		out << "Sounds:\n";
		for (auto &it : m_Sounds) {
			auto &sd = it.second;
			char buf[4096];
			sprintf(buf, "%60s [class %-5s][File: %s]\n", 
					sd.m_Name.c_str(), sd.m_Class.c_str(), sd.m_FileName.c_str());
			out << buf;
		}
		out << "\n";
	}
}
#endif

//----------------------------------------------------------------------------------

void SoundEngine::ThreadEntry() {
	m_ThreadRunning = true;
#pragma warning ( suppress: 4244 )
	srand(time(NULL));
	try {
		OrbitLogger::ThreadInfo::SetName("SNDE");
		AddLog(Info, "SoundEngine thread started");
		AddLog(Debug, "Initializing SoundEngine");
		InternalInitialize();
		AddLog(Debug, "SoundEngine Initialized");
		ThreadMain();
	}
	catch (const char * Msg){
		AddLogf(Error, "Fatal SoundEngine thread error! '%s'", Msg);
	}
	catch (const string & Msg){
		AddLogf(Error, "Fatal SoundEngine thread error! '%s'", Msg.c_str());
	}
	catch (std::exception &E){
		AddLogf(Error, "Fatal SoundEngine thread error! '%s'", E.what());
	}
	catch (const SilentExitException&) { }
	catch (...){
		AddLogf(Error, "Unknown SoundEngine thread error!");
	}
	m_ThreadRunning = false;
	InternalFinalize();
	m_Lock.notify_all();
}

bool SoundEngine::Initialize() {
	if (m_ThreadRunning) return true;
	m_ThreadRunning = true;
	m_Thread = std::thread(&ThisClass::ThreadEntry, this);
	return true;
}

bool SoundEngine::Finalize() {
	if (!m_ThreadRunning) return true;
	m_ThreadRunning = false;

	m_Thread.join();
	//delete m_Thread;
	//m_Thread = 0;

	return true;
}

struct SoundEngineDebugApi {
#ifdef DEBUG_SCRIPTAPI
	static SoundEngine* se() { return static_cast<SoundEngine*>(GetSoundEngine()); }
	static void ScanForSounds() {
		se()->ScanForSounds();
	}
#endif
};

void SoundEngine::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("cSoundEngine")
#ifdef DEBUG_SCRIPTAPI
	.addFunction("ScanForSounds", Utils::Template::InstancedStaticCall<ThisClass, void>::get<&SoundEngineDebugApi::ScanForSounds>())
#endif
	.endClass()
	;
}

//----------------------------------------------------------------------------------

void SoundEngine::ScanForSoundsScanPath(DataPath mode, const string& basepath, const string& namepath) {
	FileSystem::FileInfoTable files;

	GetFileSystem()->EnumerateFolder(basepath, files, true);

	for (auto &it : files) {
		if (it.m_IsFolder)
			continue;

		auto path = basepath + "/" + it.m_RelativeFileName;
		std::string fname;
		auto pos = it.m_RelativeFileName.rfind('.');
		if (pos == std::string::npos) {
			fname = it.m_RelativeFileName;
		} else {
			fname = it.m_RelativeFileName.substr(0, pos);
		}

		switch (mode) {
		case DataPath::Sounds:
			RegisterSound(fname, path);
			break;
		case DataPath::Music:
			RegisterMusic(fname, path);
			break;
		default:
			LogInvalidEnum(mode);
			break;
		}
	}
}

void SoundEngine::ScanForSoundsImpl() {
	string path;
	FileSystem::DataSubPaths.Translate(path, DataPath::Sounds);
	ScanForSoundsScanPath(DataPath::Sounds, path, "");
	FileSystem::DataSubPaths.Translate(path, DataPath::Music);
	ScanForSoundsScanPath(DataPath::Music, path, "");

	GetDataMgr()->NotifyResourcesChanged();
	AddLog(Debug, "Finished looking for sounds");
}

void SoundEngine::ScanForSounds() {
	LOCK_MUTEX(m_ActionListMutex);
	m_ActionList.push_back([this] { ScanForSoundsImpl(); });
	m_Lock.notify_all();
}

//----------------------------------------------------------------------------------

void SoundEngine::ConfigurationChanged(SettingsGroup what) {
	switch (what) {
	case SettingsGroup::Sound: {
		LOCK_MUTEX(m_ActionListMutex);
		m_ActionList.push_back([this] {
			if (!SoundEngineSettings::Enabled::get()) {
				FinalizeSounds();
			} else {
				ProcessPlayList();
			}
		});
		break;
	}
	case SettingsGroup::Sound_Volume: {
		LOCK_MUTEX(m_ActionListMutex);
		m_ActionList.push_back([this] {
			{
				LOCK_MUTEX(m_SoundsMutex);
				float v = SoundEngineSettings::Volume::Sound::get();
				for (auto &it : m_Sounds)
					if (it.second.m_Sound)
						it.second.m_Sound->SetVolume(v);
			}
			{
				LOCK_MUTEX(m_MusicMutex);
				float v = SoundEngineSettings::Volume::Music::get();
				for (auto &it : m_Music)
					if (it.second.m_Sound)
						it.second.m_Sound->SetVolume(v);
			}
		});
		break;
	}
	default:
		return;
	}
	Notify();
}

void SoundEngine::RegisterSound(string Name, string FileName) {
	string Class;
	if (!GetSoundClass(FileName, Class)) {
		AddLogf(Error, "Unable to find class for sound file '%s'", FileName.c_str());
		return;
	}

	SoundInfo si;
	si.m_Class.swap(Class);
	si.m_FileName.swap(FileName);
	si.m_Name.swap(Name);
	si.m_IsMusic = false;
	{
		LOCK_MUTEX(m_SoundsMutex);
		m_Sounds.insert(std::make_pair(si.m_Name, si));
	}
}

void SoundEngine::RegisterMusic(string Name, string FileName) {
	string Class;
	if (!GetSoundClass(FileName, Class)) {
		AddLogf(Error, "Unable to find class for music file '%s'", FileName.c_str());
		return;
	}

	SoundInfo si;
	si.m_Class.swap(Class);
	si.m_FileName.swap(FileName);
	si.m_Name.swap(Name);
	si.m_IsMusic = true;
	{
		LOCK_MUTEX(m_MusicMutex);
		m_Music.insert(std::make_pair(si.m_Name, si));
	}
}

void SoundEngine::PlaySound(const string &Name) {
	if (!SoundEngineSettings::Enabled::get()) return;
	LOCK_MUTEX(m_SoundsMutex);
	m_SoundPlayQueue.push_back(Name);
	m_Lock.notify_all();
}

void SoundEngine::PlayMusic(const string &Name) {
	if (!SoundEngineSettings::Enabled::get()) return;
	LOCK_MUTEX(m_MusicMutex);
	m_MusicPlayQueue.push_back(Name);
	m_Lock.notify_all();
}

void SoundEngine::PlaySoundScript(const string &Name, const string &OnEndFunc, int param) {
	if (!SoundEngineSettings::Enabled::get()) return;
	LOCK_MUTEX(m_ActionListMutex);
	m_ActionPlayList.push_back({ Name, SoundType::Sound, [OnEndFunc, param](iSound *s){
		SCRIPT_INVOKE_STATIC_NORETURN(OnEndFunc, s, param);
	} });
	m_Lock.notify_all();
}

void SoundEngine::PlayMusicScript(const string &Name, const string &OnEndFunc, int param) {
	if (!SoundEngineSettings::Enabled::get()) return;
	LOCK_MUTEX(m_ActionListMutex);
	m_ActionPlayList.push_back({ Name, SoundType::Music, [OnEndFunc, param](iSound *s) {
		SCRIPT_INVOKE_STATIC_NORETURN(OnEndFunc, s, param);
	} });
	m_Lock.notify_all();
}

void SoundEngine::SetPlayList(const string& PlayListName) {
	m_PlayList.reset();

	if (!PlayListName.empty()) { 
		m_PlayList = std::make_unique<PlayList>();
		FileSystem::XMLFile xml;
		char xmlname[64];
		sprintf(xmlname, "PlayList.%s.xml", PlayListName.c_str());
		if (!GetFileSystem()->OpenXML(xml, xmlname, DataPath::XML)) {
			AddLog(Error, "Unable to open playlist " << PlayListName);
			m_PlayList.reset();
		}
		else
			m_PlayList->LoadMeta(xml->document_element());
	}

	RunPlayList();
}

void SoundEngine::StopMusic() {
	if (m_CurrentMusics.empty()) return;
	LOCK_MUTEX(m_ActionListMutex);
	m_ActionList.push_back([this] {
		for (auto *it : m_CurrentMusics)
			it->Stop();
		m_CurrentMusics.clear();
		m_CurrentPlayListItem = 0;
	});
	m_Lock.notify_all();
}

void SoundEngine::RunPlayList() {
	if (!SoundEngineSettings::Enabled::get()) return;
	LOCK_MUTEX(m_ActionListMutex);
	m_ActionList.push_back([this] {
		ProcessPlayList();
	});
	m_Lock.notify_all();
}

//----------------------------------------------------------------------------------

void SoundEngine::InternalInitialize() {
	ConfigurationChanged(SettingsGroup::Sound_Volume);
}

void SoundEngine::InternalFinalize() {
	FinalizeSounds();
	m_Sounds.clear();
	m_Music.clear();
}

void SoundEngine::FinalizeSounds() {
	m_CurrentMusics.clear();
	m_CurrentPlayListItem = 0;
	{
		LOCK_MUTEX(m_SoundsMutex);
		m_SoundPlayQueue.clear();
		for (auto &it : m_Sounds)
			it.second.Release();
	}
	{
		LOCK_MUTEX(m_MusicMutex);
		m_MusicPlayQueue.clear();
		for (auto &it : m_Music)
			it.second.Release();
	}
}

void SoundEngine::ProcessPlayList() {
	if (!SoundEngineSettings::Enabled::get()) return;
	if (m_CurrentPlayListItem) {
		m_CurrentPlayListItem->Stop();
		m_CurrentPlayListItem = nullptr;
		if (!m_PlayList) {
			return;
		}
	}
	string next;
	if (!m_PlayList || !m_PlayList->Next(next)) {
		return;
	}

	auto s = GetMusic(next);
	if (!s) {
		AddLogf(Warning, "Music '%s' not found!", next.c_str());
		return;
	}
	s->Play();
	m_CurrentPlayListItem = s;
	m_CurrentMusics.push_back(s);
}

iSound* SoundEngine::GetSound(const string &Name) {
	LOCK_MUTEX(m_SoundsMutex);
	auto it = m_Sounds.find(Name);
	if (it == m_Sounds.end()) {
		AddLogf(Warning, "Sound '%s' not found!", Name.c_str());
		return 0;
	}
	auto &s = it->second;
	if (!s.m_Sound && !s.Load()) {
		AddLogf(Error, "Unable to load sound '%s'", Name.c_str());
		return 0;
	}
	return s.m_Sound;
}

iSound* SoundEngine::GetMusic(const string &Name) {
	LOCK_MUTEX(m_MusicMutex);
	auto it = m_Music.find(Name);
	if (it == m_Music.end()) {
		AddLogf(Warning, "Music '%s' not found!", Name.c_str());
		return 0;
	}
	auto &s = it->second;
	if (!s.m_Sound && !s.Load()) {
		AddLogf(Error, "Unable to load music '%s'", Name.c_str());
		return 0;
	}
	return s.m_Sound;
}

void SoundEngine::ThreadMain() {
	std::mutex LocalMutex;
	std::unique_lock<std::mutex> LocalLock(LocalMutex);

	while (m_ThreadRunning) {
		if (!m_ActionList.empty()) {
			LOCK_MUTEX(m_ActionListMutex);
			while (!m_ActionList.empty()) {
				m_ActionList.front()();
				m_ActionList.pop_front();
			}
		}
		if (!m_ActionPlayList.empty()) {
			LOCK_MUTEX(m_ActionListMutex);
			while (!m_ActionPlayList.empty()) {
				auto &item = m_ActionPlayList.front();
				iSound *s;
				if (item.Type == SoundType::Sound)
					s = GetSound(item.Name);
				else 
					s = GetMusic(item.Name);
				if (s) {
					s->AddEndActionNotification(item.action);
					s->Play();
					if (item.Type == SoundType::Music)
						m_CurrentMusics.push_back(s);
				}//else ignore error case
				m_ActionPlayList.pop_front();
			}
		}
		if (!m_SoundPlayQueue.empty()) {
			LOCK_MUTEX(m_SoundsMutex);
			while (!m_SoundPlayQueue.empty()) {
				string n;
				n.swap(m_SoundPlayQueue.front());
				m_SoundPlayQueue.pop_front();
				auto snd = GetSound(n);
				if (!snd) continue; //else ignore error case
				AddLogf(Debug, "Playing sound '%s'", n.c_str());
				snd->Play();
			}
		}
		if (!m_MusicPlayQueue.empty()) {
			LOCK_MUTEX(m_MusicMutex);
			while (!m_MusicPlayQueue.empty()) {
				string n;
				n.swap(m_MusicPlayQueue.front());
				m_MusicPlayQueue.pop_front();
				auto snd = GetMusic(n);
				if (!snd) continue; //else ignore error case
				AddLogf(Debug, "Playing music '%s'", n.c_str());
				snd->Play();
				m_CurrentMusics.push_back(snd);
			}
		}

		if (!m_ActionList.empty() || !m_ActionPlayList.empty() || !m_SoundPlayQueue.empty() || !m_MusicPlayQueue.empty())
			continue;

		m_Lock.wait_for(LocalLock, std::chrono::milliseconds(100));
	}
}

void SoundEngine::SoundNotifyEnd(iSound* s) {
	AddLog(Hint, "Sound ended - " << s->GetName());

	if (m_CurrentMusics.empty()) return;

	{
		LOCK_MUTEX(m_ActionListMutex);
		m_ActionList.push_back([this, s] {
			m_CurrentMusics.remove(s);
			if (s == m_CurrentPlayListItem) {
				m_CurrentPlayListItem = 0;
				ProcessPlayList();
			}
		});
	}
	m_Lock.notify_all();
}

void SoundEngine::HandleSoundEndAction(SoundEndAction action, iSound *sound) {
	if (!action || !sound)//silentyl ignore
		return;
	PostAction([action, sound] {
		try {
			action(sound);
		}
		catch (...) {
			AddLog(Error, "An error has occur during executing end sync action!");
		}
	});
}

void SoundEngine::EnumerateAudio(EnumerationFunc func) {
	{
		LOCK_MUTEX(m_SoundsMutex);
		for (auto &it : m_Sounds) {
			auto &s = it.second;
			func(s.m_Name, s.m_Class, SoundType::Sound);
		}
	}
	{
		LOCK_MUTEX(m_MusicMutex);
		for (auto &it : m_Music) {
			auto &s = it.second;
			func(s.m_Name, s.m_Class, SoundType::Music);
		}
	}
}

//----------------------------------------------------------------------------------

SoundEngine::SoundInfo::SoundInfo() {
}

SoundEngine::SoundInfo::~SoundInfo() {
	Release();
}

bool SoundEngine::SoundInfo::Load() {
	if (m_Sound) return true;

	m_Sound = SoundClassRegister::CreateClass(m_Class);
	if (!m_Sound) {
		AddLogf(Error, "Unable to create sound class '%s'", m_Class.c_str());
		return false;
	}

	m_Sound->Configure(m_FileName, m_Name, (m_IsMusic ? SoundType::Music : SoundType::Sound));
	if (!m_Sound->Initialize()) {
		delete m_Sound;
		m_Sound = 0;
		AddLogf(Error, "Unable to initialze sound '%s'", m_Name.c_str());
		return false;
	}
	if (m_IsMusic) {
		m_Sound->EnableEndNotification(true);
		m_Sound->SetVolume(SoundEngineSettings::Volume::Sound::get());
	} else {
		m_Sound->SetVolume(SoundEngineSettings::Volume::Music::get());
	}
	return true;
}

void SoundEngine::SoundInfo::Release() {
	if (!m_Sound) return;
	m_Sound->Stop();
	m_Sound->Finalize();
	delete m_Sound;
	m_Sound = 0;
}

//----------------------------------------------------------------------------------

bool SoundEngine::GetSoundClass(const string &FileName, string &out) {
	if (FileName.empty()) return false;
	const char * back = FileName.c_str();
	back = strrchr(back, '.');
	if (!back) return false;

	string ext = back;
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
	out.swap(ext);
	return true;
}

} //namespace ThreadedSoundEngine 
} //namespace Modules 
} //namespace MoonGlare 

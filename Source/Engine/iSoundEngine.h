/*
  * Generated by cppsrc.sh
  * On 2015-06-28 18:14:05.54
  * by Immethis
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef iSoundEngine_H
#define iSoundEngine_H

namespace MoonGlare {
namespace Sound {

enum class SoundType {
	Unknown, Sound, Music,
};

class iSound;
class PlayList;
using PlayListPtr = std::unique_ptr < PlayList > ;

using SoundEndAction = std::function < void(iSound*) > ;
using SoundClassRegister = Space::DynamicClassRegister < iSound > ;

class iSoundEngine : public cRootClass {
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
	SPACERTTI_DECLARE_CLASS_SINGLETON(iSoundEngine, cRootClass);
public:
 	iSoundEngine();
 	virtual ~iSoundEngine();

	virtual bool Initialize();
	virtual bool Finalize();

	virtual void ScanForSounds() = 0;

	virtual void RegisterSound(string Name, string FileName) = 0;
	virtual void RegisterMusic(string Name, string FileName) = 0;

	virtual void PlaySound(const string &Name) = 0;
	virtual void PlayMusic(const string &Name) = 0;

	virtual void PlaySoundScript(const string &Name, const string &Func, int param) = 0;
	virtual void PlayMusicScript(const string &Name, const string &Func, int param) = 0;
	virtual void PlaySoundTimer(const string &Name, Object *sender, int tid) = 0;
	virtual void PlayMusicTimer(const string &Name, Object *sender, int tid) = 0;

	virtual void StopMusic() = 0;

	virtual void SetPlayList(const string& PlayListName) = 0;
	virtual void RunPlayList() = 0;

	virtual void SoundNotifyEnd(iSound* s) = 0;
	virtual void HandleSoundEndAction(SoundEndAction action, iSound *sound) = 0;

	using EnumerationFunc = std::function<void(const string& Name, const string& ClassName, SoundType Type)>;
	virtual void EnumerateAudio(EnumerationFunc func);

	struct Flags {
		enum {
			Ready		= 0x10,
		};
	};
	DefineFlagGetter(m_Flags, Flags::Ready, Ready);

	virtual void DumpContent(std::ostream &out) = 0;

	static void RegisterScriptApi(ApiInitializer &api);
protected:
	volatile unsigned m_Flags;
	
	virtual iSound* GetSound(const string &Name) = 0;
	virtual iSound* GetMusic(const string &Name) = 0;
private: 
};

//----------------------------------------------------------------------------------

class iSound : public DataClasses::DataClass {
	SPACERTTI_DECLARE_ABSTRACT_CLASS(iSound, DataClasses::DataClass);
	DISABLE_COPY();
public:
 	iSound();
 	virtual ~iSound();

	virtual float Length() const = 0;
	virtual bool IsPlaying() const = 0;
	virtual void SetVolume(float value) const = 0;

	virtual void Play() = 0;
	//virtual void Pause() = 0;
	virtual void Stop() = 0;

	virtual void EnableEndNotification(bool value) = 0;
	virtual void AddEndActionNotification(SoundEndAction &action) = 0;

	void Configure(const string& FileName, const string& Name, SoundType Type, DataPath path = DataPath::Root);
	DefineRefGetterConst(FileName, string);
	DefineDirectGetter(Type, SoundType);

	static void RegisterScriptApi(ApiInitializer &api);
private:
	SoundType m_Type;
	string m_FileName;
};

//----------------------------------------------------------------------------------

class PlayList : public cRootClass {
	SPACERTTI_DECLARE_CLASS_NOCREATOR(PlayList, cRootClass);
public:
 	PlayList();
 	~PlayList();

	enum class Mode {
		Sequential,
		Random,
	};

	virtual bool LoadMeta(const xml_node node);

	bool Next(string &out) const;
	void Reset() const;

	DefineREADAcces(Mode, Mode);

	void Add(const string& name);
	void ClearList();

	static void RegisterScriptApi(ApiInitializer &api);
private: 
	Mode m_Mode;
	mutable unsigned m_Current;
	StringDeque m_List;
};

} //namespace Sound 

inline Sound::iSoundEngine* GetSoundEngine() { return Sound::iSoundEngine::Instance(); }

} //namespace MoonGlare 

#endif

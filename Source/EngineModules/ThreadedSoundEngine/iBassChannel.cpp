/*
	Generated by cppsrc.sh
	On 2015-01-24 10:42:25,64
	by Paweu
*/
#include <pch.h>
#include <nfMoonGlare.h>
#include <Engine/iSoundEngine.h>
#include <Engine/ModulesManager.h>
#include "iBassChannel.h"
#include "BassInternal.h"
#include "BassMusic.h"
#include "BassStream.h"
 
namespace MoonGlare {
namespace Modules {
namespace BassSound {

struct BassSoundModule : public MoonGlare::Modules::ModuleInfo {
	BassSoundModule(): BaseClass("BassSound", ModuleType::Functional) { }

	virtual bool Initialize() override {
		if (!iBassChannel::InitializeBass()) 
			return false;
		SoundClassRegister::Register<BassMusic> xmreg(".xm");
		SoundClassRegister::Register<BassMusic> itreg(".it");
		SoundClassRegister::Register<BassMusic> modreg(".mod");
		SoundClassRegister::Register<BassStream> mp3reg(".mp3");
		SoundClassRegister::Register<BassStream> oggreg(".ogg");
		SoundClassRegister::Register<BassStream> wavreg(".wav");
		return true;
	}
	virtual bool Finalize() override {
		return iBassChannel::FinalizeBass();
	}

	virtual void Notify(SettingsGroup what) override {
		switch (what) {
		case SettingsGroup::Sound_Volume:
			iBassChannel::ApplyGlobalConfig();
			break;
		default:
			break;
		}
	}

};
DEFINE_MODULE(BassSoundModule);

//----------------------------------------------------------------------------------

SPACERTTI_IMPLEMENT_ABSTRACT_CLASS(iBassChannel);

bool iBassChannel::InitializeBass() {
	auto ver = BASS_GetVersion();
	if (HIWORD(ver) != BASSVERSION) {
		AddLog(Error, "An incorrect version of BASS.DLL was loaded!");
		return false;
	}
	if (!BASS_Init(-1, 44100, 0, NULL, NULL)) {
		AddLog(Error, "Unable to initialize Bass.dll!");
		return false;
	}

	AddLogf(System, "Bass version: %d.%d.%d.%d", (ver >> 24) & 0xFF, (ver >> 16) & 0xFF, (ver >> 8) & 0xFF, ver & 0xFF);
	ApplyGlobalConfig();
	return true;
}

bool iBassChannel::FinalizeBass() {
	BASS_Stop();
	if (!BASS_Free()){
		AddLog(Error, "An error has occur during freeing bass.dll");
	}
	return true;
}

bool iBassChannel::ApplyGlobalConfig() {
//	float value;
//	if (!Settings.GetSetting("Sound.Volume.Master", value)) {
//		return false;
//	}
//	DWORD master = static_cast<DWORD>(10000 * value);
//	BASS_SetConfig(BASS_CONFIG_GVOL_MUSIC, master);
//	BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, master);
	return true;
}

//----------------------------------------------------------------------------------

iBassChannel::iBassChannel():
		BaseClass(),
		m_Handle(0),
		m_EndSyncHandle(0) {
	static_assert(sizeof(m_Handle) == sizeof(HCHANNEL), "sizeof(m_Handle) == sizeof(DWORD)");
}

iBassChannel::~iBassChannel() {
}

bool iBassChannel::Initialize() {
	return BaseClass::Initialize();
}

bool iBassChannel::Finalize() {
	for (auto &it : m_SyncActions) {
		BASS_ChannelRemoveSync(m_Handle, it->Handle);
		delete it;
	}
	m_SyncActions.clear();
	return BaseClass::Finalize();
}

void iBassChannel::SetVolume(float value) const {
	BASS_ChannelSetAttribute(m_Handle, BASS_ATTRIB_VOL, value);
}

bool iBassChannel::IsPlaying() const  {
	return BASS_ChannelIsActive(m_Handle) != BASS_ACTIVE_STOPPED;
}

float iBassChannel::Length() const {
	auto bytelen = BASS_ChannelGetLength(m_Handle, BASS_POS_BYTE);
	return static_cast<float>(BASS_ChannelBytes2Seconds(m_Handle, bytelen));
}

void iBassChannel::Play() {
	if (!BASS_ChannelPlay(m_Handle, true))
		DecodeErrorCode(__FUNCTION__);
}

void iBassChannel::Stop() {
	if(!BASS_ChannelStop(m_Handle))
		DecodeErrorCode(__FUNCTION__);
}

void iBassChannel::EnableEndNotification(bool value) {
	//those action may cause memory leeks!
	struct T {
		static void CALLBACK SyncFunc(HSYNC handle, DWORD channel, DWORD data, void *user) {
			GetSoundEngine()->SoundNotifyEnd((iSound*)user);
		}
	};
	if (value) {
		if (m_EndSyncHandle) EnableEndNotification(false);
		m_EndSyncHandle = BASS_ChannelSetSync(m_Handle, BASS_SYNC_END, 0, &T::SyncFunc, this);
		if (!m_EndSyncHandle)
			DecodeErrorCode(__FUNCTION__);
	} else {
		if (!m_EndSyncHandle) return;
		if (!BASS_ChannelRemoveSync(m_Handle, m_EndSyncHandle))
			DecodeErrorCode(__FUNCTION__);
		m_EndSyncHandle = 0;
	}
}

void iBassChannel::AddEndActionNotification(SoundEndAction &action) {
	//those action may cause memory leeks!
	struct EndActionNotification : VirtualSyncBase {
		static void CALLBACK SyncFunc(HSYNC handle, DWORD channel, DWORD data, void *user) {
			EndActionNotification *t = (EndActionNotification*)user;
			t->Sound->m_SyncActions.remove(t);
			GetSoundEngine()->HandleSoundEndAction(t->action, t->Sound);
			delete t;
		}

		EndActionNotification(SoundEndAction &p, iBassChannel *s) : action(p), Sound(s) { }

		SoundEndAction action;
		iBassChannel *Sound;
		~EndActionNotification() {}
	};
	EndActionNotification *t = new EndActionNotification(action, this);
	t->Handle = BASS_ChannelSetSync(m_Handle, BASS_SYNC_END | BASS_SYNC_ONETIME, 0, &EndActionNotification::SyncFunc, t);
	if (!t->Handle) {
		DecodeErrorCode(__FUNCTION__);
		delete t;
		return;
	}
	m_SyncActions.push_back(t);
}

void iBassChannel::DecodeErrorCode(const char* function) const {
	auto errc = BASS_ErrorGetCode();
	switch (errc) {
	case BASS_ERROR_HANDLE:	
		AddLogf(Error, "Bass error in function '%s' - Invalid handle", function);
		return;
	case BASS_ERROR_START:	
		AddLogf(Error, "Bass error in function '%s' - Bass is stopped", function);
		return;
	case BASS_ERROR_DECODE:	
		AddLogf(Error, "Bass error in function '%s' - Channel not playable", function);
		return;
	case BASS_ERROR_BUFLOST:	
		AddLogf(Error, "Bass error in function '%s' - BASS_ERROR_BUFLOST", function);
		return;
	case BASS_ERROR_NOHW:
		AddLogf(Error, "Bass error in function '%s' - BASS_ERROR_NOHW", function);
		return;
	case BASS_ERROR_ILLTYPE:
		AddLogf(Error, "Bass error in function '%s' - illegal type", function);
		return;
	case BASS_ERROR_ILLPARAM:
		AddLogf(Error, "Bass error in function '%s' - illegal param", function);
		return;
	case BASS_OK:
		return;//there is no error to report
	default:
		AddLogf(Error, "Unknown bass error in function '%s' - %x", function, errc);
	}
}

} //namespace BassSound 
} //namespace Modules 
} //namespace MoonGlare 
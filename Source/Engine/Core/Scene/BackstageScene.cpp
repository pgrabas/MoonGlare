/*
	Generated by cppsrc.sh
	On 2014-12-17 20:00:18,22
	by Paweu
*/
#include <pch.h>
#include <MoonGlare.h>
#include "BackstageScene.h"

namespace Core {
namespace Scene {

SPACERTTI_IMPLEMENT_CLASS(BackstageScene)

BackstageScene::BackstageScene() {
	SetName(string("_") + this->GetDynamicTypeInfo()->Name);
}

BackstageScene::~BackstageScene() {
}

//----------------------------------------------------------------

void BackstageScene::BeginScene() {
	THROW_ASSERT(IsInitialized(), 0);
	THROW_ASSERT(!IsReady(), 0);
	BaseClass::BeginScene();
}

void BackstageScene::EndScene() {
	THROW_ASSERT(IsReady(), 0);
	BaseClass::EndScene();
}

bool BackstageScene::DoInitialize() {
	return BaseClass::DoInitialize();
}

bool BackstageScene::DoFinalize() {
	return BaseClass::DoFinalize();
}

void BackstageScene::DoMove(const MoveConfig &conf) {
	AddLog(Warning, "Exiting by entering backstage scene!");
	GetEngine()->Exit();
}

//----------------------------------------------------------------

int BackstageScene::InvokeOnTimer(int TimerID) { SCRIPT_INVOKE(OnTimer, TimerID); }
int BackstageScene::InvokeOnEscape() { SCRIPT_INVOKE(OnEscape); }
int BackstageScene::InvokeOnBeginScene() { SCRIPT_INVOKE(OnBeginScene); }
int BackstageScene::InvokeOnEndScene() { SCRIPT_INVOKE(OnEndScene); }
int BackstageScene::InvokeOnInitialize() { SCRIPT_INVOKE(OnInitialize); }
int BackstageScene::InvokeOnFinalize() { SCRIPT_INVOKE(OnFinalize); }

} //namespace Scenes 
} //namespace Core 

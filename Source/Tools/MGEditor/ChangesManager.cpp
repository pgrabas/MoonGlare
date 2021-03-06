/*
  * Generated by cppsrc.sh
  * On 2016-09-11 17:09:28,63
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#include PCH_HEADER
#include "ChangesManager.h"

namespace MoonGlare {
namespace QtShared {

ChangesManager* ChangesManager::s_Instance = nullptr;

ChangesManager::ChangesManager() {
	assert(!s_Instance);
	s_Instance = this;
}

ChangesManager::~ChangesManager() {
	s_Instance = nullptr;
}

void ChangesManager::SetModiffiedState(iChangeContainer * sender, bool value) {
	if (value) {
		m_State[sender].m_Modiffied = value;
	} else {
		m_State.erase(sender);
	}
	emit Changed(sender, value);
}

void ChangesManager::SaveAll() {
	while (!m_State.empty()) {
		auto it = m_State.begin()->first;
		it->SaveChanges();
	}
}

} //namespace QtShared
} //namespace MoonGlare

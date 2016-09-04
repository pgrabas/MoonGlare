/*
  * Generated by cppsrc.sh
  * On 2016-08-04 19:45:53,67
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef InputProcessor_H
#define InputProcessor_H

#include <libSpace/src/Container/StaticVector.h>

namespace MoonGlare {
namespace Core {

using KeyId = uint16_t;
using InputStateId = uint8_t;
static_assert(1 << (sizeof(InputStateId) * 8) >= Configuration::Input::MaxInputStates, "To small InputStateId base type");

union InputStateValue {
	bool m_Boolean;
	float m_Float;
};

struct InputState {
	enum class Type {
		Invalid,
		Switch,
		FloatAxis,
	};

	struct {
		bool m_Valid : 1;
	} m_Flags;
	InputStateValue m_Value;
	Type m_Type;
	unsigned m_ActiveKeyId;
	Configuration::RuntimeRevision m_Revision;
};

struct KeyAction {
	struct {
		bool m_Valid : 1;
		bool m_Positive : 1;
	} m_Flags;
	InputStateId m_Id;
	InputStateValue m_Value;
};

enum class MouseAxisId {
	X,
	Y,
	ScrollX,
	ScrollY,
	Unknown,
	MaxValue = Unknown,
};
static_assert(static_cast<int>(MouseAxisId::MaxValue) == Configuration::Input::MaxMouseAxes, "Invalid mouse axes settings!");

struct AxisAction {
	struct {
		bool m_Valid : 1;
	} m_Flags;
	InputStateId m_Id;
	float m_Sensitivity;
};

struct InputKeyOffsets {
	enum e : unsigned {
		Keyboard = 0,
		Mouse = Configuration::Input::MaxKeyCode - Configuration::Input::MaxMouseButton,
	};
};

enum class InputSwitchState {
	Off,
	Pressed,
	On,
	Released,
};

//---------------------------------------------------------------------------------------

class InputProcessor final {
public:
 	InputProcessor();
 	~InputProcessor();

	bool Initialize(World *world);
	bool Finalize();

	bool Step(const Core::MoveConfig &config);

	void SetKeyState(unsigned KeyCode, bool Pressed) {
		ProcessKeyState(KeyCode + InputKeyOffsets::Keyboard, Pressed);
	}
	void SetMouseButtonState(unsigned Button, bool Pressed) {
		ProcessKeyState(Button + InputKeyOffsets::Mouse, Pressed);
	}
	void SetMouseDelta(const math::vec2 &delta) {
		ProcessMouseAxis(MouseAxisId::X, delta[0]);
		ProcessMouseAxis(MouseAxisId::Y, delta[1]);
	}
	void SetMouseScrollDelta(const math::vec2 &delta) {
		ProcessMouseAxis(MouseAxisId::ScrollX, delta[0]);
		ProcessMouseAxis(MouseAxisId::ScrollY, delta[1]);
	}
	void ClearStates();

	bool Save(pugi::xml_node node) const;
	bool Load(const pugi::xml_node node);

	void Clear();
	void ResetToInternalDefault();

	bool AddKeyboardAxis(const char *Name, KeyId PositiveKey, KeyId NegativeKey);
	bool AddKeyboardSwitch(const char *Name, KeyId Key);
	bool AddMouseAxis(const char *Name, MouseAxisId axis, float Sensitivity);

	bool RegisterKeySwitch(const char* Name, const char* KeyName);

	static void RegisterScriptApi(ApiInitializer &root);
protected:
	using InputStateArray = Space::Container::StaticVector<InputState, Configuration::Input::MaxInputStates>;
	using KeyMapArray = std::array<KeyAction, Configuration::Input::MaxKeyCode>;
	using MouseAxesArray = std::array<AxisAction, Configuration::Input::MaxMouseAxes>;

	InputStateArray m_InputStates;
	MouseAxesArray m_MouseAxes;
	KeyMapArray m_Keys;
	Configuration::RuntimeRevision m_CurrentRevision;

	std::unordered_map<std::string, InputStateId> m_InputNames;
	World *m_World;

	void ProcessKeyState(unsigned Id, bool Pressed);
	void ProcessMouseAxis(MouseAxisId Id, float Delta);
	bool GetInputStateName(InputStateId isid, std::string &out) const;

	InputState* AllocInputState(InputState::Type type, const std::string &Name, InputStateId &outindex);
	KeyAction* AllocKeyAction(KeyId kid, InputStateId isid, bool Positive);
	AxisAction* AllocMouseAxis(MouseAxisId maid, InputStateId isid, float Sensitivity);
private:
	static int luaIndexInput(lua_State *lua);
};

//---------------------------------------------------------------------------------------

struct KeyNamesTable {
	KeyNamesTable();
	const char *Get(KeyId kid) const {
		if (kid >= m_Array.size())
			return nullptr;
		return m_Array[kid];
	}
	bool Find(const char *Name, KeyId &out) const {
		for (KeyId kid = 0; kid < m_Array.size(); ++kid)
			if (m_Array[kid] && stricmp(Name, m_Array[kid]) == 0)
				return out = kid, true;
		return false;
	}
private:
	std::array<const char *, Configuration::Input::MaxKeyCode> m_Array;
};

extern const KeyNamesTable g_KeyNamesTable;

} //namespace Core 
} //namespace MoonGlare 

#endif

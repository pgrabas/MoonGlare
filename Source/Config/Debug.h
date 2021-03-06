#ifndef DEBUGCLASS_H_
#define DEBUGCLASS_H_

#ifndef DEBUG
#error Attempt to include debug header on non debug build
#endif

#define BreakPoint { __debugbreak(); }

#define DEBUG_MSG(A, MSG, ACTION)	{ if(!(m_DebugFlags & Config::Debug::A)) { m_DebugFlags |= Config::Debug::A; ErrorLog(MSG); } ACTION }

#define MINOR_ASSERT(COND, A, ACTION) { if(!(COND)) DEBUG_MSG(A, "!(" #COND ")", ACTION) }
#define THROW_ASSERT(CHECK, MSG) { ((CHECK)?(void)0:throw Config::Debug::eAsserationError(__LINE__, __FILE__, __FUNCTION__, #CHECK, MSG, this)); }
#define THROW_ASSERTs(CHECK, ...) { ((CHECK)?(void)0:throw Config::Debug::eAsserationError(__LINE__, __FILE__, __FUNCTION__, #CHECK, __VA_ARGS__)); }

#define ASSERT(CHECK, ...)				THROW_ASSERTs(CHECK, __VA_ARGS__)
#define MoonGlareAssert(CHECK, ...)		THROW_ASSERTs(CHECK, __VA_ARGS__)


#define SPACERTTI_SINGLETON_CHECK		ASSERT(_Instance, "Singleton check failed!")
#define SPACERTTI_SINGLETON_SET_CHECK	THROW_ASSERT(!_Instance, "Cannot change existing singleton instance!")

#define REQUIRE_REIMPLEMENT()		do { AddLog(Error, "Reached code has to be reimplemented! Quitting."); throw "Code need to be reimplemented"; } while (false)

#define DEBUG_INTERFACE 1

#define CHECK(WHAT) if(!(WHAT)) { AddLogf(Error, "CHECK FAILED: '" #WHAT "'"); };
#define INITCHECK(WHAT) if(!(WHAT)) { AddLogf(Error, "CHECK FAILED: '" #WHAT "'"); return false; };

#define DEBUG_LOG 1
#define DEBUG_DUMP 1
#define DEBUG_SCRIPTAPI 1
#define DEBUG_RESOURCEDUMP 1
#define DEBUG_CHECKS 1

#define DEBUG_INTERFACE 1
#define DEBUG_MEMORY 1

#define PERF_PERIODIC_PRINT	1

#define _FEATURE_LOG_COUNTERS_ 1

#define DEBUG_LOG_FOLDER	"logs/"

class cRootClass;

namespace Graphic { class cRenderDevice; }

namespace Config {
namespace Debug {

enum eLocalDebugMessages {
	dlm_NoModelInstance		= 0x0001,
};

class eAsserationError : public std::exception {
	std::string m_Msg;
public:
	using cstr = const char *;

	virtual const char* what() const { return m_Msg.c_str(); }
	eAsserationError(unsigned line, cstr file, cstr fun, cstr check, cstr Msg = 0, const void *sender = 0){
		std::stringstream ss;
		buildmsg(ss, line, file, fun, check, Msg);
	//	if(sender)
	//		ss << "Sender: " << sender->Info() << "\n";
		ss << "\n";
		m_Msg = ss.str();
	}
private:
	void buildmsg(std::stringstream &ss, unsigned line, cstr file, cstr fun, cstr check, cstr Msg) {
		ss << "\n";
		ss << "Assertion Error!\n"; // for object: [" << root->Info() << "]\n";
		ss << "Check: \"" << check << "\"";
		if(Msg) ss << " with message: \"" << Msg << "\"\n";
		else ss << " failed but no message was provided\n";
		const char *c = strrchr(file, '\\');
		if(!c) c = strrchr(file, '/');
		if(!c) c = file;
		else ++c;
		ss << "Location: " << c << ":" << line << "@" << fun << "\n";
	}
};

struct EnableFlags {
	static bool Physics;							//<! Debug global switch: On/Off physics
	static bool PhysicsDebugDraw;					//<! Debug global switch: On/Off physics debug draw

	static bool ShowTitleBarDebugInfo;				//<! Debug global switch: show/hide main window title bar info
};

} //namespace Debug
} //namespace Config

#endif // DEBUGCLASS_H_ 

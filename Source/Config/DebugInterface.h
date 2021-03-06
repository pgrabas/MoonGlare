/*
  * Generated by cppsrc.sh
  * On 2015-08-22  9:29:51,31
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#if !defined(DebugInterface_H) && defined(DEBUG_INTERFACE)
#define DebugInterface_H

namespace Config {
namespace Debug {

#ifdef DEBUG_MEMORY

struct DebugMemoryInterface {
	struct DebugMemoryCounter {
		uint32_t Allocated;
		uint32_t ElementSize;
		uint32_t Capacity;
	};
	
	using DebugMemoryInfoFunction = std::function<void(DebugMemoryCounter &)>;
	struct DebugMemoryCounterInfo {
		DebugMemoryInfoFunction m_Function;
		std::string m_Name;
	};

	DebugMemoryInterface();
	~DebugMemoryInterface();

	void DebugMemorySetClassName(std::string ClassName);
	void DebugMemoryRegisterCounter(std::string CounterName, DebugMemoryInfoFunction func);
	void DebugMemorySetParent(DebugMemoryInterface *Parent) { m_DebugMemoryParent = Parent; }

	DebugMemoryInterface* GetNext() { return m_Next; }
	DebugMemoryInterface* GetPrev() { return m_Prev; }
	const std::vector<DebugMemoryCounterInfo> &DebugMemoryGetCounters() { return m_Counters; }
	const std::string &DebugMemoryGetClassName() const { return m_ClassName; }
	DebugMemoryInterface *DebugMemoryGetParent() const { return m_DebugMemoryParent; }

	static std::pair<std::unique_lock<std::mutex>, DebugMemoryInterface*> GetFirstDebugMemoryInterface();
private:
	std::vector<DebugMemoryCounterInfo> m_Counters;
	std::string m_ClassName;
	DebugMemoryInterface *m_DebugMemoryParent;
	DebugMemoryInterface *m_Next;
	DebugMemoryInterface *m_Prev;

	static DebugMemoryInterface* s_First;
	static std::mutex m_DebugMemoryMutex;
};

#endif

} //namespace Debug 
} //namespace Config 

#endif

#ifndef DEBUG_INTERFACE
#ifdef DEBUG_MEMORY
#error DEBUG_MEMORY cannot be defined without DEBUG_INTERFACE
#endif
#endif

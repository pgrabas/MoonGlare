/*
  * Generated by cppsrc.sh
  * On 2016-09-21 19:16:53,84
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef CommandQueue_H
#define CommandQueue_H

#include "nfRenderer.h"

namespace MoonGlare {
namespace Renderer {

//vitality and copying is forbidden
//should be zeroed manually before first use!
struct CommandQueue final {
	CommandQueue() { ClearAllocation(); }
	CommandQueue(const CommandQueue&) = delete;
	CommandQueue(CommandQueue&&) = delete;
	const CommandQueue& operator = (const CommandQueue&) = delete;
	const CommandQueue& operator = (const CommandQueue&&) = delete;
 	~CommandQueue() { }

	size_t CommandsCapacity() const { return RendererConf::CommandQueue::CommandLimit; }
	size_t CommandsAllocated() const { return m_AllocatedMemory; }
	size_t MemoryCapacity() const { return RendererConf::CommandQueue::ArgumentMemoryBuffer; }
	size_t MemoryAllocated() const { return m_AllocatedMemory; }

	bool IsEmpty() const { return CommandsAllocated() == 0; }
	bool IsFull() const { return CommandsAllocated() >= CommandsCapacity(); }

	void Execute(bool Clear = true) {
		for (size_t it = 0; it < m_AllocatedCommands; ++it) {
			m_CommandFunctions[it](m_CommandArguments[it]);
		}
		if (Clear) 
			ClearAllocation();
	}

	void MemZero() { memset(this, 0, sizeof(*this)); }
	void ClearAllocation() { m_AllocatedCommands = m_AllocatedMemory = 0; }

	template<typename CMD>
	typename CMD::Argument* PushCommand() {
		auto *argptr = AllocateMemory<CMD::Argument>(CMD::ArgumentSize());
		if (IsFull() || !argptr) {
			AddLogf(Warning, "Command queue is full. Command %s has not been allocated!", typeid(CMD).name());
			return nullptr;
		}

		size_t index = m_AllocatedCommands;
		++m_AllocatedCommands;

		m_CommandFunctions[index] = CMD::GetFunction();
		m_CommandArguments[index] = argptr;

		return argptr;
	}

	struct RollbackPoint {
		size_t m_CommandCount;
		size_t m_MemoryUsed;
	};

	//unsafe; Rolling-back has to be used carefully
	RollbackPoint GetSavePoint() const { return RollbackPoint{ m_AllocatedCommands, m_AllocatedMemory }; }
	//unsafe; Rolling-back has to be used carefully
	void Rollback(RollbackPoint point) { m_AllocatedCommands = point.m_CommandCount; m_AllocatedMemory = point.m_MemoryUsed; }
private: 
	template<typename T> using Array = std::array<T, RendererConf::CommandQueue::CommandLimit>;

	template<typename T>
	T* AllocateMemory(size_t size) {
		if (m_AllocatedMemory + size >= MemoryCapacity())
			return nullptr;

		T *ptr = reinterpret_cast<T*>(&m_Memory[0] + m_AllocatedMemory);
		m_AllocatedMemory += size;
		return ptr;
	}
	
	size_t m_AllocatedCommands;
	size_t m_AllocatedMemory;

	Array<CommandFunction> m_CommandFunctions;
	Array<void*> m_CommandArguments;

	std::array<uint8_t, RendererConf::CommandQueue::ArgumentMemoryBuffer> m_Memory;
};

} //namespace Renderer 
} //namespace MoonGlare 

#endif

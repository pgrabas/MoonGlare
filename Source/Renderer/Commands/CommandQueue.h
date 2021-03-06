#pragma once

#include "../nfRenderer.h"
#include "CommandQueueBase.h"

namespace MoonGlare::Renderer::Commands {

//virtuality and copying is forbidden
//should be zeroed manually before first use!
class alignas(16) CommandQueue final {
    using Conf = Configuration::CommandQueue;
public:
    CommandQueue() = default;
    CommandQueue(const CommandQueue&) = delete;
    CommandQueue& operator=(const CommandQueue&) = delete;

    template<typename T>
    using ByteArray = Space::Memory::StaticMemory<T, Conf::ArgumentMemoryBuffer>;
    using Allocator_t = Space::Memory::StackAllocator<ByteArray>;

    uint32_t CommandsCapacity() const { return Conf::CommandLimit; }
    uint32_t MemoryCapacity() const { return Conf::ArgumentMemoryBuffer; }
    uint32_t CommandsAllocated() const { return m_AllocatedCommands; }
    uint32_t MemoryAllocated() const { return m_Memory.Allocated(); }

    bool IsEmpty() const { return CommandsAllocated() == 0; }
    bool IsFull() const { return CommandsAllocated() >= CommandsCapacity(); }

    void SortExecuteClear() {
        Sort();
        Execute();
    }
    void Execute(bool Clear = true) {
        assert(this);
        for (uint32_t it = 0; it < m_AllocatedCommands; ++it) {
            auto f = m_CommandFunctions[it];
            auto a = m_CommandArguments[it];
            f(a);
        }
        if (Clear) 
            ClearAllocation();
    }

    void MemZero() { memset(this, 0, sizeof(*this)); }
    void ClearAllocation() { 
        m_AllocatedCommands = m_CommandsPreamble;
        m_Memory.PartialClear(m_MemoryPreamble);
    }
    void Clear() {
        m_AllocatedCommands = m_CommandsPreamble = 0;
        m_MemoryPreamble = 0;
        m_Memory.Clear();
    }

    void Sort() {
        if (m_AllocatedCommands > 1) {
            //m_SortKeys[m_AllocatedCommands].m_UIntValue = 0xFFFF;
            SortBegin(static_cast<int>(m_CommandsPreamble), static_cast<int>(m_AllocatedCommands)-1);
        }
    }

    template<typename CMD>
    typename CMD::Argument* PushCommand(CommandKey SortKey = CommandKey()) {
        auto *argptr = AllocateMemory<CMD::Argument>(CMD::ArgumentSize());
        if (IsFull() || !argptr) {
            __debugbreak();
            AddLogf(Warning, "Command queue is full. Command %s has not been allocated!", typeid(CMD).name());
            return nullptr;
        }

        size_t index = m_AllocatedCommands;
        ++m_AllocatedCommands;

        m_CommandFunctions[index] = CMD::GetFunction();
        m_CommandArguments[index] = argptr;
        m_SortKeys[index] = SortKey;

        return argptr;
    }


    template<typename CMD, typename ... ARGS>
    typename CMD::Argument* MakeCommandKey(CommandKey key, ARGS&& ... args) {
        auto *argptr = AllocateMemory<CMD::Argument>(CMD::ArgumentSize());
        if (IsFull() || !argptr) {
            AddLogf(Warning, "Command queue is full. Command %s has not been allocated!", typeid(CMD).name());
            return nullptr;
        }

        size_t index = m_AllocatedCommands;
        ++m_AllocatedCommands;

        m_CommandFunctions[index] = CMD::GetFunction();
        m_CommandArguments[index] = argptr;
        m_SortKeys[index] = key;

        return new(argptr)typename CMD::Argument{ std::forward<ARGS>(args)... };
    }

    template<typename CMD, typename ... ARGS>
    typename CMD::Argument* MakeCommand(ARGS&& ... args) {
        return MakeCommandKey<CMD>(CommandKey(), std::forward<ARGS>(args)...);
    }

    struct ExecuteQueueArgument {
        CommandQueue *m_Queue;
        static void Execute(const ExecuteQueueArgument *arg) {
            assert(arg->m_Queue);
            arg->m_Queue->Execute(false);
        }
    };
    using ExecuteQueue = CommandTemplate<ExecuteQueueArgument>;

    bool PushQueue(CommandQueue *queue, CommandKey key = CommandKey()) {
        if (!queue) {
            AddLogf(Warning, "Attempt to queue null queue!");
            return false;
        }
        auto *argptr = AllocateMemory<ExecuteQueue::Argument>(ExecuteQueue::ArgumentSize());
        if (IsFull() || !argptr) {
            AddLogf(Warning, "Command queue is full. Command %s has not been allocated!", typeid(ExecuteQueue).name());
            return false;
        }

        size_t index = m_AllocatedCommands;
        ++m_AllocatedCommands;

        m_CommandFunctions[index] = ExecuteQueue::GetFunction();
        m_CommandArguments[index] = argptr;
        m_SortKeys[index] = key;

        argptr->m_Queue = queue;

        return true;
    }

    void SetQueuePreamble() {
        m_CommandsPreamble = m_AllocatedCommands;
        m_MemoryPreamble = m_Memory.Allocated();
    }

    Allocator_t& GetMemory() { return m_Memory; }
private: 
    template<typename T> using Array = std::array<T, Conf::CommandLimit>;
    
    //TODO make this class atomic
    template<typename T>
    T* AllocateMemory(uint32_t size) {
        T *ptr = reinterpret_cast<T*>(m_Memory.Allocate<char>(size));
        return ptr;
    }

    uint32_t m_AllocatedCommands;
    uint32_t m_CommandsPreamble;
    uint32_t m_MemoryPreamble;

    Allocator_t m_Memory;
#ifdef X64
    uint32_t p_padding;
#endif
    Array<CommandFunction> m_CommandFunctions;
    Array<void*> m_CommandArguments;
    Array<CommandKey> m_SortKeys;

    void SortBegin(int first, int last);
};

static_assert((sizeof(CommandQueue) % 16) == 0, "Invalid size!");

using ExecuteQueue = CommandQueue::ExecuteQueue;

} //namespace MoonGlare::Renderer::Commands

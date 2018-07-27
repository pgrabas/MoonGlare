#pragma once

#include <Foundation/iFileSystem.h>

#include "Decoder/iDecoder.h"
#include "Configuration.h"
#include "HandleApi.h"
#include "OpenAl.h"

namespace MoonGlare::SoundSystem {

class SoundSystem;


//todo: remove FinitPending?
enum class SourceStatus : uint8_t {
    Invalid, Standby, Inactive, InitPending, Playing, Paused, Stopped, FinitPending,
};

enum class SourceCommand : uint8_t {
    None, Finalize, ResumePlaying, StopPlaying, Pause,
};

using SoundHandleGeneration = uint16_t;
static constexpr SoundHandleGeneration InvalidSoundHandleGeneration = 0;

union SoundHandleComposite {
    struct {
        SoundHandleGeneration generation;
        SourceIndex index;
    };
    SoundHandle handle;
};

static_assert(sizeof(SoundHandle) == sizeof(uint32_t));
static_assert(sizeof(SourceIndex) + sizeof(SoundHandleGeneration) == sizeof(SoundHandle));
static_assert(sizeof(SoundHandle) == sizeof(SoundHandleComposite));

struct SourceState {
    //TODO: split for public and private part
    using conf = MoonGlare::SoundSystem::Configuration;

    SourceStatus status = SourceStatus::Invalid;
    SourceCommand command = SourceCommand::None;
    bool streamFinished = false;
    bool releaseOnStop = false;
    bool loop = false;
    uint8_t bufferCount = 0;
    SoundSource sourceSoundHandle = InvalidSoundSource;
    std::string uri;
    std::unique_ptr<Decoder::iDecoder> decoder;

    uint32_t processedBuffers = 0;
    uint32_t processedBytes = 0;
    float processedSeconds = 0;
    float duration = 0;

    iPlaybackWatcher *watcherInterface = nullptr;
    UserData userData = 0;

    bool Playable() const {
        return status >= SourceStatus::Playing && status <= SourceStatus::Stopped;
    }
};

class StateProcessor {
public:
    StateProcessor(iFileSystem * fs);
    ~StateProcessor();

    void Step();
    void Initialize();
    void Finalize();

    void PrintState() const;

    SoundHandle AllocateSource();
    void ReleaseSource(SoundHandle handle);

    void ActivateSource(SoundHandle handle);
    void DeactivateSource(SoundHandle handle);

    void SetCommand(SoundHandle handle, SourceCommand command);  
    SourceStatus GetStatus(SoundHandle handle);
    bool IsSoundHandleValid(SoundHandle handle) const { return CheckSoundHandle(handle).first; }
    SoundSource GetSoundSource(SoundHandle handle);
    void SetReleaseOnStop(SoundHandle handle, bool value);
    void CloseSoundHandle(SoundHandle handle); //closes only handle, state does not change
    bool Open(SoundHandle handle, const std::string &uri, SoundKind kind);
    float GetDuration(SoundHandle handle) const;
    float GetTimePosition(SoundHandle handle) const;
    void SetLoop(SoundHandle handle, bool value);
    void SetCallback(SoundHandle handle, iPlaybackWatcher *iface, UserData userData);
private:
    iFileSystem * fileSystem = nullptr;

    using conf = MoonGlare::SoundSystem::Configuration;

    template<typename T, size_t S>
    using StaticVector = Space::Container::StaticVector<T, S>;

    std::unordered_map<std::string, std::shared_ptr<Decoder::iDecoderFactory>> decoderFactories;

    StaticVector<SoundBuffer, conf::MaxBuffers> standbyBuffers;
    StaticVector<SourceIndex, conf::MaxSources> activeSources;
    StaticVector<SourceState, conf::MaxSources> sourceState;
    StaticVector<SoundHandleGeneration, conf::MaxSources> sourceStateGeneration;

    //access protected by standbySourcesMutex;
    StaticVector<SourceIndex, conf::MaxSources> standbySources;
    //access protected by sourceAcivationQueueMutex;
    StaticVector<SourceIndex, conf::SourceAcivationQueue> sourceAcivationQueue;

    uint32_t allocatedBuffersCount = 0;

    std::mutex standbySourcesMutex;
    std::mutex sourceAcivationQueueMutex;
    using lock_guard = std::lock_guard<std::mutex>;

    enum class SourceProcessStatus {
        Continue, ReleaseAndRemove, Remove,
    };

    void CheckOpenAlError() const;

    void ActivateSources();
    SourceProcessStatus StateProcessor::ProcessSource(SourceIndex si);

    void ProcessPlayState(SourceIndex index, SourceState &state);
    bool LoadBuffer(SourceState &state, SoundBuffer sb);

    void ReleaseSourceBufferQueue(SourceState &state);
    bool InitializeSource(SourceState &state);

    bool GenBuffers();
    SoundBuffer GetNextBuffer();
    void ReleaseBuffer(SoundBuffer b);

    bool GenSources();
    void ActivateSource(SourceIndex index);
    SourceIndex GetNextSource();
    void ReleaseSource(SourceIndex s);

    std::shared_ptr<Decoder::iDecoderFactory> FindFactory(const std::string &uri);

    SoundHandle GetSoundHandle(SourceIndex s);
    std::pair<bool, SourceIndex> CheckSoundHandle(SoundHandle h) const;
};

}

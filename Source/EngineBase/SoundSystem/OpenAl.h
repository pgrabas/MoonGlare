#pragma once

#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"

namespace MoonGlare::SoundSystem {

enum class SourceIndex : uint16_t { };

struct SoundSource;

struct SoundBuffer {
    using type = ALuint;
    constexpr SoundBuffer() : value(0) { }
    constexpr SoundBuffer(type v) : value(v) {}
    SoundBuffer(const SoundBuffer &ss) = default;
    SoundBuffer(SoundBuffer &&ss) = default;
    SoundBuffer(const SoundSource &ss) = delete;

    type handle() const { return value; }
    operator type() const { return handle(); }
    const type* operator &() const { return &value; }
    type* operator &() { return &value; }
    SoundBuffer& operator =(type v) { value = v; return *this; }
    SoundBuffer& operator =(const SoundBuffer &v) = default;
    SoundBuffer& operator =(const SoundSource &v) = delete;
    bool operator ==(const SoundBuffer &v) const { return handle() == v.handle(); }
    bool operator !=(const SoundBuffer &v) const { return handle() != v.handle(); }
    operator bool() const { return handle() != 0; };

    void ClearData() const { alBufferData(handle(), AL_FORMAT_STEREO8, nullptr, 0, 8000); }
    void SetData(ALuint buffer, ALenum format, const ALvoid *data, ALsizei size, ALsizei freq) const { alBufferData(handle(), format, data, size, freq); }

private:
    type value;
};

struct SoundSource {
    using type = ALuint;
    constexpr SoundSource() : value(0) { }
    constexpr SoundSource(type v) : value(v) {}
    SoundSource(const SoundSource &ss) = default;
    SoundSource(SoundSource &&ss) = default;
    SoundSource(SoundBuffer ss) = delete;

    type handle() const { return value; }
    operator type() const { return handle(); }
    const type* operator &() const { return &value; }
    type* operator &() { return &value; }
    SoundSource& operator =(type v) { value = v; return *this; }
    SoundSource& operator =(const SoundSource &v) = default;
    SoundSource& operator =(const SoundBuffer &v) = delete;
    bool operator ==(const SoundSource &v) const { return handle() == v.handle(); }
    bool operator !=(const SoundSource &v) const { return handle() != v.handle(); }
    operator bool() const { return handle() != 0; };

    void Play() const { alSourcePlay(handle()); }
    void Stop() const { alSourceStop(handle()); }
    void Pause() const { alSourcePause(handle()); }

    ALint GetInt(ALenum param) const {
        ALint r = 0;
        alGetSourcei(handle(), param, &r);
        return r;
    }

    ALfloat GetFloat(ALenum param) const {
        ALfloat r = 0;
        alGetSourcef(handle(), param, &r);
        return r;
    }

    void QueueBuffer(SoundBuffer b) const { alSourceQueueBuffers(handle(), 1, &b); }
    SoundBuffer UnqueueBuffer() const { 
        SoundBuffer b;
        alSourceUnqueueBuffers(handle(), 1, &b); 
        return b;
    }

    ALint GetProcessedBuffers() const { return GetInt(AL_BUFFERS_PROCESSED); }
    ALint GetQueuedBuffers() const { return GetInt(AL_BUFFERS_QUEUED); }
    ALint GetState() const { return GetInt(AL_SOURCE_STATE); }

    ALfloat GetTimePosition() const { return GetFloat(AL_SEC_OFFSET); }

    void SetGain(ALfloat gain) const { alGetSourcef(handle(), AL_GAIN, &gain); }

    void SetPosition(ALfloat x, ALfloat y, ALfloat z) const { alGetSource3f(handle(), AL_POSITION, &x, &y, &z); }
    void SetValocity(ALfloat x, ALfloat y, ALfloat z) const { alGetSource3f(handle(), AL_VELOCITY, &x, &y, &z); }
private:
    type value;
};

static constexpr SoundBuffer InvalidSoundBuffer = { 0 };
static constexpr SoundSource InvalidSoundSource ( 0u );

static constexpr SourceIndex InvalidSourceIndex = (SourceIndex)0xFFFF;

}
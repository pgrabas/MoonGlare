#pragma once

namespace MoonGlare::Component::Configuration {

//----------------------------------------------------------------------------

static constexpr uint32_t MaxEventTypes = 16;

//----------------------------------------------------------------------------

static constexpr uint32_t EventDispatcherQueueSize = 64 * 1024; //bytes

 //----------------------------------------------------------------------------

static constexpr uint32_t EntityIndexBitCount = 14;
static constexpr uint32_t EntityGenerationBitCount = 32 - EntityIndexBitCount;

static constexpr uint32_t EntityLimit = 1 << EntityIndexBitCount;

}

/*
  * Generated by cppsrc.sh
  * On 2017-02-09 21:40:01,47
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#pragma once

#include "Configuration.Renderer.h"

namespace MoonGlare::Renderer {

class FrameBuffer final {
public:
 	FrameBuffer() { }
	~FrameBuffer() { }
private:
	std::array<uint8_t, Configuration::FrameBufferSize> m_Memory;
};

} //namespace MoonGlare::Renderer

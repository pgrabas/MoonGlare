/*
	Generated by cppsrc.sh
	On 2015-01-24 10:42:35,52
	by Paweu
*/

#pragma once
#ifndef BassMusic_H
#define BassMusic_H

namespace MoonGlare {
namespace Modules {
namespace BassSound {

class BassMusic : public iBassChannel {
	SPACERTTI_DECLARE_CLASS(BassMusic, iBassChannel);
public:
 	BassMusic();
 	virtual ~BassMusic();

	virtual bool Initialize();
	virtual bool Finalize();
};

} //namespace BassSound 
} //namespace Modules 
} //namespace MoonGlare 

#endif
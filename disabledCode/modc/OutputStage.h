/*
  * Generated by cppsrc.sh
  * On 2015-03-11 22:25:08,13
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef OutputStage_H
#define OutputStage_H

namespace modc {
namespace pipeline {

class OutputStage : public PipelineStage {
public:
	static PipelineStageInfo GetInfo() {
		return PipelineStageInfo{ "OutputStage", &Queue };
	}

	static SettingsMap OutputConfig;
protected:
	static void Queue(Pipeline &p);
};

} //namespace pipeline 
} //namespace modc 

#endif

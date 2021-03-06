/*
  * Generated by cppsrc.sh
  * On 2015-03-11 18:10:38,51
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef DumpStage_H
#define DumpStage_H

namespace modc {
namespace pipeline {

class DumpStage : public PipelineStage {
public:
	static void Queue(Pipeline &p) {
		auto fs = Init<DumpStage>(p);
		p.QueueFirstAction([fs](WorkConfiguration &data) { fs->ValidateStage(data); });
	}

	static PipelineStageInfo GetInfo() {
		return PipelineStageInfo{ "DumpStage", &Queue };
	}
protected:
	void ValidateStage(WorkConfiguration &data);
};

} //namespace pipeline 
} //namespace modc 

#endif

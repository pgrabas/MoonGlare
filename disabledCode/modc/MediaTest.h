/*
	Generated by cppsrc.sh
	On 
	by Paweu
*/

#pragma once
#ifndef MediaTest_H
#define MediaTest_H

namespace modc {
namespace pipeline {	

class MediaTest : public PipelineStage {
public:
	static PipelineStageInfo GetInfo() {
		return PipelineStageInfo{ "MediaTestStage", &Queue };
	}
	static PipelineStageInfo GetSkippedInfo() {
		return PipelineStageInfo{ "SkippedMediaTestStage", &QueueSkipped };
	}

	MediaTest();
protected:
	static void Queue(Pipeline &p) {
		auto fs = Init<MediaTest>(p);
		p.QueueFirstAction([fs](WorkConfiguration &data) { fs->ValidateStage(data); });
	}
	static void QueueSkipped(Pipeline &p) {
		auto fs = Init<MediaTest>(p);
		p.QueueFirstAction([fs](WorkConfiguration &data) { fs->SkipStage(data); });
	}
	
	void ValidateStage(WorkConfiguration &data);
	void SkipStage(WorkConfiguration &data);
	void ExitAction(WorkConfiguration &data);

	//static void QueueMediaTest(Pipeline &p, const string &file, std::shared_ptr<ValidationStage> stage) {
		//p.QueueAction([file, stage](WorkConfiguration &data) {
			//DispatchMediaTest(data, file, stage);
		//});
	//}

	std::map<string, void(MediaTest::*)(WorkConfiguration &data, FileInfo&)> m_MediaTesters;

	void TestLua(WorkConfiguration &data, FileInfo&);
	void TestXML(WorkConfiguration &data, FileInfo&);
	void TestImage(WorkConfiguration &data, FileInfo&);
	void TestSound(WorkConfiguration &data, FileInfo&);
	void TestJustPass(WorkConfiguration &data, FileInfo&);
	void TestJustFail(WorkConfiguration &data, FileInfo&);
	void TestForceRemove(WorkConfiguration &data, FileInfo& fi);
	
	unsigned m_ValidatedFiles = 0;
	unsigned m_InvalidFiles = 0;
private: 
	//static void DispatchMediaTest(WorkConfiguration &data, const string &file, std::shared_ptr<ValidationStage> stage);
};

#endif
}// namespace pipeline
}// namespace modc
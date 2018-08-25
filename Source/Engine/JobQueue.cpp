/*
  * Generated by cppsrc.sh
  * On 2015-03-30 17:40:04,79
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <nfMoonGlare.h>
#include "JobQueue.h"

#include "EngineSettings.h"

namespace MoonGlare {

SPACERTTI_IMPLEMENT_CLASS_SINGLETON(JobQueue);

JobQueue::JobQueue():
		m_PendingJobs(0), 
		m_RunningThreads(0) {
	SetThisAsInstance();



	AddLogf(Debug, "Executing job queue with %d thread", ::Settings_t::JobQueueThreads);

#ifdef _FEATURE_EXTENDED_PERF_COUNTERS_
	m_JobCounter.store(0);
#endif
	m_CanRun = true;
	m_RunningThreads.store(0);
	for (unsigned i = 0; i < ::Settings_t::JobQueueThreads; ++i) {
		std::thread([this, i]{ ThreadMain(i); }).detach();
	}
}

JobQueue::~JobQueue() {
	m_CanRun = false;

	while (m_RunningThreads.load() > 0) {
		m_Lock.notify_all();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	AddLog(Debug, "All job threads has been stopped");
}

void JobQueue::ThreadMain(unsigned ThreadIndex) {
	{
		char buffer[16];
		sprintf(buffer, "THQ%X", ThreadIndex);
		::OrbitLogger::ThreadInfo::SetName(buffer);
	}
	m_RunningThreads += 1;
	AddLogf(Debug, "Job thread %d started", ThreadIndex);
	
	std::mutex local_mutex;
	std::unique_lock<std::mutex> local_lock(local_mutex);

#ifdef _FEATURE_EXTENDED_PERF_COUNTERS_
	unsigned selfCounter = 0;
#endif

	try {
		while (m_CanRun) {
			if (m_PendingJobs.load() > 0)//may not be safe!
			{
				JobInfo job;
				{
					LOCK_MUTEX(m_mutex);
					if (m_PendingJobs.load() == 0)
						continue;
					m_PendingJobs.fetch_sub(1);
					if (m_Jobs.empty())
						continue;
					job.swap(m_Jobs.front());
					m_Jobs.pop_front();
				}

#ifdef _FEATURE_EXTENDED_PERF_COUNTERS_
				AddLogf(Debug, "Fetched job with index %d (out of %d)", job.JobIndex, m_Jobs.size());
#endif
				try {
#ifdef _FEATURE_EXTENDED_PERF_COUNTERS_
					++selfCounter;
#endif
					if (job.func)
						job.func();
				}
				catch (std::exception &e) {
					AddLogf(Error, "An error has occur during executing of job! (%s)", e.what());
				}
				catch (...) {
					AddLog(Error, "An unknown error has occur during executing of job!");
				}
			}
			if (m_PendingJobs.load() > 0)
				continue;
			//std::this_thread::sleep_for(std::chrono::milliseconds(1));
			m_Lock.wait(local_lock);
		}
#ifdef _FEATURE_EXTENDED_PERF_COUNTERS_
		AddLogf(Debug, "Job thred %d stopped. Executed jobs: %d", ThreadIndex, selfCounter);
#else
		AddLogf(Debug, "Job thred %d stopped", ThreadIndex);
#endif
	}
	catch(...) {
		AddLogf(Error, "Job thread %d error!. Thread has been stopped!", ThreadIndex);
	}
	m_RunningThreads -= 1;
}


} //namespace MoonGlare 


/*
  * Generated by cppsrc.sh
  * On 2015-03-30 17:40:04,79
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef JobQueue_H
#define JobQueue_H

namespace MoonGlare {

class JobQueue : public Space::RTTI::RTTIObject {
	SPACERTTI_DECLARE_CLASS_SINGLETON(JobQueue, Space::RTTI::RTTIObject);
public:
 	JobQueue();
 	virtual ~JobQueue();

	template<class T>
	static void QueueJob(T &&t) {
		Instance()->AddJob(std::forward<T>(t));
	}

	template<class T>
	void AddJob(T && t) {
#ifdef _FEATURE_EXTENDED_PERF_COUNTERS_
		unsigned id = m_JobCounter.fetch_add(1);
		{
			LOCK_MUTEX(m_mutex);
			m_Jobs.emplace_back(t, id);
		}
		AddLog(Debug, "Queued job with index " << id);
#else
		{
			LOCK_MUTEX(m_mutex);
			m_Jobs.emplace_back(t);
		}
#endif
		m_PendingJobs.fetch_add(1);
		m_Lock.notify_one();
	}

private: 
	using JobFunction = std::function < void() > ;

	struct JobInfo {
		JobFunction func;
#ifdef _FEATURE_EXTENDED_PERF_COUNTERS_
		unsigned JobIndex;
#endif

		JobInfo(JobInfo&& j) : func(std::move(j.func)) {
#ifdef _FEATURE_EXTENDED_PERF_COUNTERS_
			JobIndex = j.JobIndex;
#endif
		}
		JobInfo() { }
#ifdef _FEATURE_EXTENDED_PERF_COUNTERS_
		JobInfo(JobFunction&& f, unsigned idx): func(std::move(f)), JobIndex(idx) { }
		JobInfo(JobFunction& f, unsigned idx): func(f), JobIndex(idx) { }
#endif
		JobInfo(JobFunction&& f): func(std::move(f)) { }
		JobInfo(JobFunction& f): func(f) { }

		void swap(JobInfo &j) {
			func.swap(j.func);
#ifdef _FEATURE_EXTENDED_PERF_COUNTERS_
			JobIndex = j.JobIndex;
#endif
		}
	};

#ifdef _FEATURE_EXTENDED_PERF_COUNTERS_
	std::atomic<unsigned> m_JobCounter;
#endif

	using JobList = std::list < JobInfo > ;
	JobList m_Jobs;
	std::mutex m_mutex;
	std::atomic<unsigned> m_RunningThreads;
	std::atomic<unsigned> m_PendingJobs;
	std::condition_variable m_Lock;
	volatile bool m_CanRun;

	void ThreadMain(unsigned ThreadIndex);
};

} //namespace MoonGlare 

#endif

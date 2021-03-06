/*
  * Generated by cppsrc.sh
  * On 2015-08-08 19:50:25,77
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef RemoteConsoleObserver_H
#define RemoteConsoleObserver_H

#include "RemoteConsole.h"

struct RemoteConsoleRequestQueue;

class RemoteConsoleObserver : public QObject, public std::enable_shared_from_this<RemoteConsoleObserver> {
	Q_OBJECT;
public:
	RemoteConsoleObserver(InsiderApi::MessageTypes MessageType, RemoteConsoleRequestQueue *Queue = nullptr);

	enum class HanderStatus {
		Remove,
		Keep,
	};
	virtual HanderStatus Message(InsiderApi::InsiderMessageBuffer &message) { return HanderStatus::Remove; };
	HanderStatus ProcessMessage(InsiderApi::InsiderMessageBuffer &message);

	enum class TimeOutAction {
		Remove,
		Resend,
	};
	virtual TimeOutAction TimedOut();

	enum class ErrorStatus {
		Fatal,
		TimedOut,					//all resend attempts failed
		MessageNotSupported,
	};
	virtual void OnError(ErrorStatus status) { } //send failed, with no implicit way to resend

	virtual void OnSend();
	virtual void OnRecive();

	virtual void BuildMessage(InsiderApi::InsiderMessageBuffer &buffer) { };

	InsiderApi::u32 GetRequestID() const {
		return m_RequestId;
	}
	InsiderApi::MessageTypes GetMessageType() const {
		return m_MessageType;
	}

	void SetQueueOwner(RemoteConsoleRequestQueue *q) { m_Queue = q; }
	void SetRetriesCount(unsigned timeout, unsigned attempts);
protected:
	RemoteConsoleRequestQueue *m_Queue;
	unsigned m_Timeout;
	unsigned m_MaxAttempts, m_Attempts;
private:
	InsiderApi::u32 m_RequestId;
	InsiderApi::MessageTypes m_MessageType;
	static std::atomic<InsiderApi::u32> s_RequestCounter;
	std::unique_ptr<QTimer> m_Timer;
private slots:
	void Expired();
};

using SharedRemoteConsoleObserver = std::shared_ptr < RemoteConsoleObserver >;

class RemoteConsoleEnumerationObserver : public RemoteConsoleObserver {
public:
	RemoteConsoleEnumerationObserver(InsiderApi::MessageTypes MessageType, const char *location, RemoteConsoleRequestQueue *Queue = nullptr):
		RemoteConsoleObserver(MessageType, Queue), m_Location(location) {
	}

	virtual void BuildMessage(InsiderApi::InsiderMessageBuffer &buffer);

	DefineRefGetterConst(Location, std::string);
private:
	std::string m_Location;
};

struct RemoteConsoleRequestQueue {
	using SharedRequest = std::shared_ptr < RemoteConsoleObserver >;

	RemoteConsoleRequestQueue();
	~RemoteConsoleRequestQueue();

	void CancelRequests();
	void RequestFinished(RemoteConsoleObserver *sender);
	void QueueRequest(SharedRequest request);
	bool IsQueued(SharedRequest request) const;

	void SetQueueName(const std::string &Name);
private:
	std::list<SharedRequest> m_RequestList;
	SharedRequest m_CurrentRequest;
	std::string m_QueueName;

	void Process();
};

#endif

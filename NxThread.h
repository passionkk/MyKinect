#pragma once
// define

class CNxThread;
class CNxThreadCallBack
{
public:
	virtual BOOL OnNxThreadTriggered(CNxThread * pThread, DWORD dwWaitObj) = 0;
};

class CNxThread
{
public:
	CNxThread();
	~CNxThread();

	BOOL CreateThread(CNxThreadCallBack * pCallBack, BOOL bTimeCritical = FALSE, HANDLE * pExtTriggers = NULL, int nExtTriggersCount = 0);
	BOOL DestroyThread();

	BOOL AddExtEvent(HANDLE hEvent);
	void RemoveExtEvent(HANDLE hEvent);

	void SetTrigger();

protected:
	BOOL StartThread(BOOL bTimeCritical);
	BOOL StopThread();

	static DWORD WINAPI StaticThreadProc(LPVOID lpData);
	DWORD LocalThreadProc();
	
public:
	CNxThreadCallBack * m_pCallBack;

	DWORD	m_nThreadID;
	HANDLE	m_hThread;
	HANDLE	m_hTriggerEvent;
	HANDLE	m_hExitEvent;

	CArray<HANDLE>	m_aWaitHandles;
};

// inline implement
inline CNxThread::CNxThread()
{
	m_pCallBack = NULL;
	m_hThread = NULL;
	m_nThreadID = 0;
	m_hTriggerEvent = NULL;
	m_hExitEvent = NULL;
}

inline CNxThread::~CNxThread()
{
	if(m_hTriggerEvent != NULL)
		CloseHandle(m_hTriggerEvent);

	if(m_hExitEvent != NULL)
		CloseHandle(m_hExitEvent);
}

inline BOOL CNxThread::CreateThread(CNxThreadCallBack * pCallBack, BOOL bTimeCritical, HANDLE * pExtTriggers, int nExtTriggersCount)
{
	if(m_hThread != NULL)
		return FALSE;

	if(pCallBack == NULL)
		return FALSE;

	m_pCallBack = pCallBack;

	if(m_hTriggerEvent == NULL)
		m_hTriggerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	if(m_hExitEvent == NULL)
		m_hExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	m_aWaitHandles.RemoveAll();
	m_aWaitHandles.Add(m_hExitEvent);
	m_aWaitHandles.Add(m_hTriggerEvent);

	if(pExtTriggers != NULL)
	{
		for(int i = 0; i < nExtTriggersCount; i++)
			m_aWaitHandles.Add(pExtTriggers[i]);
	}

	return StartThread(bTimeCritical);
}

inline BOOL CNxThread::DestroyThread()
{
	StopThread();
	return TRUE;
}

inline BOOL CNxThread::AddExtEvent(HANDLE hEvent)
{
	if(m_aWaitHandles.GetCount() == MAXIMUM_WAIT_OBJECTS)
		return FALSE;

	m_aWaitHandles.Add(hEvent);
	return TRUE;
}

inline void CNxThread::RemoveExtEvent(HANDLE hEvent)
{
	for(int i = 2; i < (int)m_aWaitHandles.GetCount(); i++)
	{
		if(m_aWaitHandles[i] == hEvent)
		{
			m_aWaitHandles.RemoveAt(i);
			break;
		}
	}
}

inline void CNxThread::SetTrigger()
{
	if(m_hTriggerEvent != NULL)
		SetEvent(m_hTriggerEvent);
}

inline BOOL CNxThread::StartThread(BOOL bTimeCritical)
{
	if(m_hThread) return FALSE;

	m_hThread = ::CreateThread(NULL, 0, StaticThreadProc, this, bTimeCritical ? CREATE_SUSPENDED : 0, &m_nThreadID);
	if(m_hThread == NULL)
	{
		TRACE(_T("CNxThread::StartThread begin thread failed! [%08x]"), GetLastError());
		return FALSE;
	}

	if(bTimeCritical)
	{
		SetThreadPriority(m_hThread, THREAD_PRIORITY_TIME_CRITICAL);
		ResumeThread(m_hThread);
	}

	return TRUE;
}

inline BOOL CNxThread::StopThread()
{
	if(m_hThread != NULL)
	{
		SetEvent(m_hExitEvent);
		WaitForSingleObject(m_hThread, INFINITE);
		m_hThread = NULL;
		m_nThreadID = 0;
	}

	m_aWaitHandles.RemoveAll();

	return TRUE;
}

inline DWORD WINAPI CNxThread::StaticThreadProc(LPVOID lpData)
{
	CNxThread * pThread = (CNxThread *)lpData;
	return pThread->LocalThreadProc();
}

inline DWORD CNxThread::LocalThreadProc()
{
	ASSERT(m_pCallBack);
	CoInitialize(NULL);

	while(1)
	{
		DWORD nWait = WaitForMultipleObjects((int)m_aWaitHandles.GetCount(), m_aWaitHandles.GetData(), FALSE, INFINITE);
		if(nWait == WAIT_OBJECT_0)
			//循环退出
			return 0;

		if(nWait == WAIT_IO_COMPLETION)
		{
			TRACE("WaitForMultipleObjects got WAIT_IO_COMPLETION.");
			continue;
		}

		if(!m_pCallBack->OnNxThreadTriggered(this, nWait))
			// 退出线程
			return 1;
	}

	return 0;
}

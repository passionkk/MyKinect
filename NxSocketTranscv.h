#pragma once
#include <afxsock.h>
#include "NxThread.h"

// Export directive
#ifndef SCTranscv_EXP
#define SCTranscv_EXP /*__declspec(dllimport) */
#endif

struct SNxSocketCmdHead
{
	DWORD fccTo;
	DWORD fccFrom;
	DWORD nCmd;
	DWORD nDataSize;
};

// �ⲿ���ݻ���ѭ�����ýӿ�
class SCTranscv_EXP IM5TExtDataRecycler
{
public:
	virtual void FreeExtDataBuffer(LPBYTE lpData) = 0;
};

class CNxSocketCmd;
class SCTranscv_EXP IM5TSockerCmdTrace
{
public:
	virtual void OnSendOver(CNxSocketCmd * pCmd) = 0;
};

class SCTranscv_EXP CNxSocketCmd : public CM5Object
{
	DECLARE_SERIAL(CNxSocketCmd);
public:
	CNxSocketCmd();
	~CNxSocketCmd();

	BOOL	AllocCmdDataBuf(DWORD dwBytes);
	void    ResetCurPtr();

	void *	GetCurPtr();
	int		GetDataLeft();

	BOOL	IsOver(int nIncrement);
	BOOL	IsHeadReceiveOver();

	// ����һ�����ÿ������ÿ�����������������SmartPoint��
	// ���Һ������������һ����Ӧ�ÿ���û���Լ�����m_pData���ڴ�
	// ����ֱ��ʹ��ԭ�����m_pData���ڴ棬Ҳ�������ͷ���
	CNxSocketCmd * MakeRefCopy();

protected:
	static	IM5TExtDataRecycler * ms_pIExtDataRecycler;

public:
	static  void SetExtDataRecycler(IM5TExtDataRecycler * pIExtDataRecycler)
	{
		ms_pIExtDataRecycler = pIExtDataRecycler;
	};

public:
	SNxSocketCmdHead	m_sHead;
	LPBYTE				m_pData;
	SYSTEMTIME			m_sTime;

protected:
	LONG	m_nPos;
	BOOL	m_bLocalDataBuffer;

	// �����õ������SmartPoint��
	// ���m_pJRef != NULL ��˵��������������������Աm_pData�����Լ���
	// �������Ա����õ�����ģ������������ͷţ����ɱ����õ�������ͷ�
	M5RCPtr<CNxSocketCmd> m_pJRef;	

public:
	IM5TSockerCmdTrace * m_pITrace;
};

//////////////////////////////////////////////////////////////////////////

class SCTranscv_EXP CNxCmdSocket : public CM5Object
{
	DECLARE_SERIAL(CNxCmdSocket);
public:
	CNxCmdSocket();
	~CNxCmdSocket();

	void Clear();
	void GetIPAddrString(CString & str);

	volatile mutable DWORD m_fccFlag;
	SOCKET		m_hSocket;
	sockaddr_in	m_sAddr;
	HANDLE		m_hWaitevent;

	CList<M5RCPtr<CNxSocketCmd>, CNxSocketCmd*>	m_lstSendCmds;
	M5RCPtr<CNxSocketCmd>	m_pJRecvCmd;
};

//////////////////////////////////////////////////////////////////////////

typedef enum EM5TSCEvent{
	keM5TCmdSocket_Accept = 0,
	keM5TCmdSocket_Connect,
	keM5TCmdSocket_Close,
	keM5TCmdSocket_ReceiveCmd,
	keM5TCmdSocket_error = 10,
	keM5TCmdSocket_EnumEventError,
	keM5TCmdSocket_ConnectFailed,
	keM5TCmdSocket_NetDown,	// ������Connect��Closeʱ��ͬ�����
	keM5TCmdSocket_ErrorOnAccept,
	keM5TCmdSocket_ErrorOnSetOpt,
	keM5TCmdSocket_ErrorOnClose,
	keM5TCmdSocket_ErrorOnReceive,
	keM5TCmdSocket_ErrorOnSend,
} EM5TSCEvent;

// 
class CNxCmdTCPTranscv;
struct SNxCmdSocketEvent
{
	SNxCmdSocketEvent(CNxCmdTCPTranscv * pTCPTranscv, SOCKET socket, int event, int subcode) {
		pTranscv = pTCPTranscv;
		hSocket = socket;
		nEvent = event;
		nSubcode = subcode;
	};

	SNxCmdSocketEvent(CNxCmdTCPTranscv * pTCPTranscv, SOCKET socket, CNxSocketCmd * pCmd) {
		pTranscv = pTCPTranscv;
		hSocket = socket;
		nEvent = keM5TCmdSocket_ReceiveCmd;
		nSubcode = 0;
		pJReceive = pCmd;
	};

	CNxCmdTCPTranscv * pTranscv;
	SOCKET	hSocket;
	int		nEvent;
	int		nSubcode;

	CString msg;
	M5RCPtr<CNxSocketCmd> pJReceive;
};

class SCTranscv_EXP IM5TCmdSocketCallback
{
public:
	virtual BOOL	OnSocketEvent(SNxCmdSocketEvent * pEvent) { return FALSE ;};
};

class SCTranscv_EXP CNxCmdTCPTranscv : public CNxThreadCallBack
{
public:
	CNxCmdTCPTranscv();
	virtual ~CNxCmdTCPTranscv();

	BOOL Create(DWORD fccFlag, IM5TCmdSocketCallback * pCallBack, HWND hAsyncNotifyWnd = NULL);
	BOOL Connect(DWORD fccSvrFlag, LPCTSTR lpszIPAddr, UINT nPort, LPCTSTR lpszHostIP = NULL);
	BOOL Listen(LPCTSTR lpszIPAddr, UINT nPort);
	void Destroy();

	BOOL SendCmd(CNxSocketCmd* pCmd, SOCKET socket = NULL);

	SNxCmdSocketEvent * PeekEvent();

	int	 GetIncomingCount();
	BOOL GetIncomingSocket(int nIdx, M5RCPtr<CNxCmdSocket> & pJSocket);
	BOOL GetIncomingSocket(SOCKET socket, M5RCPtr<CNxCmdSocket> & pJSocket);
	BOOL RemoveIncoming(SOCKET socket);

	void GetLastErrString(CString & strErr);
	int	 GetLastErrorCode();

	BOOL IsCreated() { return m_oSocket.m_hSocket != NULL; };
	BOOL IsServer() { return m_fccSvrFlag == 0; };

protected:
	//
	void OnError(SOCKET socket, int nError, LPCTSTR lpszErrMsgFmt);
	void DoNotify(CNxCmdSocket * pSocket, int nNotify, LPCTSTR lpszNotifyMsgFmt);
	void DoReceive(CNxCmdSocket * pSocket);

	void GetSocketHasNewSendCmd(CArray<CNxCmdSocket *> & aSockets);

	CNxSocketCmd * GetCurSendCmd(CNxCmdSocket * pSocket, BOOL bNext);


public:
	int	GetCurSendCmdCount();

	// CNxThreadCallBack
	virtual BOOL OnNxThreadTriggered(CNxThread * pThread, DWORD dwWaitObj);

protected:
	virtual void OnAccept(int nErrorCode);
	virtual void OnConnect(CNxCmdSocket * pSocket, int nErrorCode);
	virtual void OnSend(CNxCmdSocket * pSocket = NULL, int nErrorCode = 0);
	virtual void OnReceive(CNxCmdSocket * pSocket, int nErrorCode);
	virtual BOOL OnClose(CNxCmdSocket * pSocket, int nErrorCode);	// return TRUE , to exit thread

public:
	static UINT	ms_nAsyncNotifyMsg;

public:
	HWND					m_hAsyncNotifyWnd;
	IM5TCmdSocketCallback *	m_pICallback;
	DWORD					m_fccSvrFlag;

	//
	CNxCmdSocket	m_oSocket;
	CNxThread		m_oThread;

	//
	CCriticalSection										m_oCriSect;
	CArray<M5RCPtr<CNxCmdSocket>, CNxCmdSocket *>			m_aIncomings;
	CMap<UINT, UINT, CNxCmdSocket *, CNxCmdSocket *>		m_mapIncomings;

	//
	CList<SNxCmdSocketEvent *>	m_lstEvents;
	CList<SOCKET>				m_lstNewSends;

	//
	CString	m_strLastError;
};
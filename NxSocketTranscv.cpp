#include "StdAfx.h"
#include "NxSockettranscv.h"

//////////////////////////////////////////////////////////////////////////
// CNxSocketCmd
IMPLEMENT_SERIAL(CNxSocketCmd, CM5Object, 0);

IM5TExtDataRecycler * CNxSocketCmd::ms_pIExtDataRecycler = NULL;
CNxSocketCmd::CNxSocketCmd()
{
	memset(&m_sHead, 0, sizeof(m_sHead));
	m_pData = NULL;
	m_bLocalDataBuffer = FALSE;
	memset(&m_sTime, 0, sizeof(m_sTime));
	ResetCurPtr();
	m_pITrace = NULL;
}

CNxSocketCmd::~CNxSocketCmd()
{
	if(m_pJRef == NULL)
	{
		// 并非引用命令，所以需要释放资源
		if (m_bLocalDataBuffer) 
		{
			if(m_pData != NULL)
				GlobalFree(m_pData);
		}
		else
		if (ms_pIExtDataRecycler != NULL)
		{
			ms_pIExtDataRecycler->FreeExtDataBuffer(m_pData);
		}
	}
}

BOOL CNxSocketCmd::AllocCmdDataBuf(DWORD dwBytes)
{
	// Current buffer can be reused
	if (m_pData == NULL || dwBytes != m_sHead.nDataSize) 
	{
		// Free current buffer
		if (m_pData) 
		{
			GlobalFree(m_pData);
			m_pData = NULL;
			m_bLocalDataBuffer = FALSE;
		}	

		if(dwBytes > 0)
		{
			// Allocate new buffer
			m_pData = (LPBYTE)GlobalAlloc(GPTR, dwBytes);
			if (m_pData == NULL)
			{
				TRACE(_T("Socket Cmd data buffer allocation failed.\n"));
				return FALSE;
			}

			m_bLocalDataBuffer = TRUE;
		}

		m_sHead.nDataSize = dwBytes;
	}

	return TRUE;
}

void CNxSocketCmd::ResetCurPtr()
{
	m_nPos = - (int)sizeof(m_sHead);
}

void * CNxSocketCmd::GetCurPtr()
{
	if(m_nPos < 0)
		return ((LPBYTE)&m_sHead) + sizeof(m_sHead) + m_nPos;

	if(m_nPos < (int)m_sHead.nDataSize)
		return (void *)(m_pData + m_nPos);

	return NULL;
}

int CNxSocketCmd::GetDataLeft()
{
	if(m_nPos < 0)
		return - m_nPos;

	return (int)m_sHead.nDataSize - m_nPos;
}

BOOL CNxSocketCmd::IsOver(int nIncrement)
{
	m_nPos += nIncrement;
	if(m_nPos >= (int)m_sHead.nDataSize)
	{
		if(m_pITrace != NULL)
			m_pITrace->OnSendOver(this);

		return TRUE;
	}

	return FALSE;
}

BOOL CNxSocketCmd::IsHeadReceiveOver()
{
	return m_nPos == 0;
}

CNxSocketCmd * CNxSocketCmd::MakeRefCopy()
{
	CNxSocketCmd * pCmd = new CNxSocketCmd();
	memcpy(&pCmd->m_sHead, &m_sHead, sizeof(m_sHead));
	pCmd->m_pData = m_pData;
	pCmd->m_pJRef = this;
	return pCmd;
}
//////////////////////////////////////////////////////////////////////////
// CNxCmdSocket
IMPLEMENT_SERIAL(CNxCmdSocket, CM5Object, 0);
CNxCmdSocket::CNxCmdSocket()
{
	m_fccFlag = 0;
	m_hSocket = NULL;
	m_hWaitevent = NULL;
	memset(&m_sAddr, 0, sizeof(m_sAddr));
}

CNxCmdSocket::~CNxCmdSocket()
{
	Clear();
}

void CNxCmdSocket::Clear()
{
	if(m_hSocket != NULL)
	{
		::shutdown(m_hSocket, SD_BOTH);
		::closesocket(m_hSocket);
	}
	m_hSocket = NULL;

	if(m_hWaitevent != NULL)
		WSACloseEvent(m_hWaitevent);
	m_hWaitevent = NULL;

	m_lstSendCmds.RemoveAll();
	m_pJRecvCmd = NULL;
};

void CNxCmdSocket::GetIPAddrString(CString & str)
{
	str.Format(_T("%d.%d.%d.%d"), m_sAddr.sin_addr.S_un.S_un_b.s_b1, m_sAddr.sin_addr.S_un.S_un_b.s_b2, m_sAddr.sin_addr.S_un.S_un_b.s_b3, m_sAddr.sin_addr.S_un.S_un_b.s_b4);
};

//////////////////////////////////////////////////////////////////////////
// CNxCmdTCPTranscv
UINT CNxCmdTCPTranscv::ms_nAsyncNotifyMsg = ::RegisterWindowMessage(_T("NxCmdSocketNotify"));
CNxCmdTCPTranscv::CNxCmdTCPTranscv()
{
	m_hAsyncNotifyWnd = NULL;
	m_pICallback = NULL;
	m_fccSvrFlag = 0;
}

CNxCmdTCPTranscv::~CNxCmdTCPTranscv()
{
	Destroy();
}

BOOL CNxCmdTCPTranscv::Create(DWORD fccFlag, IM5TCmdSocketCallback * pCallBack, HWND hAsyncNotifyWnd/* = NULL*/)
{
	m_strLastError.Empty();

	//
	if(m_oSocket.m_hSocket != NULL)
	{
		CSingleLock lock(&m_oCriSect, TRUE);
		m_strLastError = _T("Can not be recreated!");
		TRACE("%s\r\n", m_strLastError);
		return FALSE;
	}

	//
	m_hAsyncNotifyWnd = hAsyncNotifyWnd;
	m_pICallback = pCallBack;
	m_fccSvrFlag = 0;

	//
	m_oSocket.m_fccFlag = fccFlag;
	m_oSocket.m_hSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if( m_oSocket.m_hSocket == INVALID_SOCKET )
	{
		m_strLastError.Format(_T("Create a socket Failed! %d"), WSAGetLastError());
		TRACE("%s\r\n", m_strLastError);

		return FALSE;
	}

	m_oSocket.m_hWaitevent = ::WSACreateEvent();
	if( m_oSocket.m_hWaitevent == NULL )
	{
		m_strLastError.Format(_T("Create a WSAEventsocket Failed! %d"), WSAGetLastError());
		TRACE("%s\r\n", m_strLastError);

		Destroy();
		return FALSE;
	}

	if(::WSAEventSelect(m_oSocket.m_hSocket, m_oSocket.m_hWaitevent, FD_ACCEPT|FD_CONNECT|FD_CLOSE|FD_WRITE|FD_READ) == SOCKET_ERROR)
	{
		m_strLastError.Format(_T("Call WSAEventSelect got a error! %d"), WSAGetLastError());
		TRACE("%s\r\n", m_strLastError);

		Destroy();
		return FALSE;
	}

	if(!m_oThread.CreateThread(this, FALSE, &m_oSocket.m_hWaitevent, 1))
	{
		m_strLastError = _T("Create socket thread failed!");
		TRACE("%s\r\n", m_strLastError);

		Destroy();
		return FALSE;
	}

	return TRUE;
}

ULONG GetIPAddress( LPCTSTR lpszIPAddr )
{
	ULONG		uAddr = INADDR_NONE;
	if (lpszIPAddr != NULL)
	{
		CStringA strAsc(lpszIPAddr);
		uAddr = inet_addr(strAsc);
	}

	return uAddr;
}

BOOL CNxCmdTCPTranscv::Connect(DWORD fccSvrFlag, LPCTSTR lpszIPAddr, UINT nPort, LPCTSTR lpszHostIP /*= NULL*/)
{
	if(!IsCreated())
	{
		m_strLastError = _T("Socket is not created!");
		TRACE("%s\r\n", m_strLastError);

		return FALSE;
	}

	if(lpszHostIP != NULL)
	{
		sockaddr_in sBind = { 0 };
		sBind.sin_family = AF_INET;
		sBind.sin_port = htons(0);
		sBind.sin_addr.S_un.S_addr = GetIPAddress(lpszHostIP);
		if(::bind(m_oSocket.m_hSocket, (sockaddr*)&sBind, sizeof(sBind)) == SOCKET_ERROR)
		{
			m_strLastError.Format(_T("Bind socket to %s failed! %d"), lpszHostIP, WSAGetLastError());
			TRACE("%s\r\n", m_strLastError);
			return FALSE;
		}
	}

	m_fccSvrFlag = fccSvrFlag;

	//
	m_oSocket.m_sAddr.sin_family = AF_INET;
	m_oSocket.m_sAddr.sin_port = htons(nPort);
	m_oSocket.m_sAddr.sin_addr.S_un.S_addr = GetIPAddress(lpszIPAddr);
	if(::connect(m_oSocket.m_hSocket, (sockaddr*)&m_oSocket.m_sAddr, sizeof(m_oSocket.m_sAddr)) == SOCKET_ERROR)
	{
		int nError = WSAGetLastError();
		if(nError != WSAEWOULDBLOCK)	// 连接时如果没有立即连通会返回WSAEWOULDBLOCK, 不是错误
		{
			m_strLastError.Format(_T("Connect to %s (%d) failed! %d"), lpszIPAddr, nPort, WSAGetLastError());
			TRACE("%s\r\n", m_strLastError);

			return FALSE;
		}
	}

	return TRUE;
}

BOOL CNxCmdTCPTranscv::Listen(LPCTSTR lpszIPAddr, UINT nPort)
{
	if(!IsCreated())
	{
		m_strLastError = _T("Socket is not created!");
		TRACE("%s\r\n", m_strLastError);
		return FALSE;
	}

	m_fccSvrFlag = 0;
 
	m_oSocket.m_sAddr.sin_family = AF_INET;
	m_oSocket.m_sAddr.sin_port = htons(nPort);
	m_oSocket.m_sAddr.sin_addr.S_un.S_addr = GetIPAddress(lpszIPAddr);
	if(::bind(m_oSocket.m_hSocket, (sockaddr*)&m_oSocket.m_sAddr, sizeof(m_oSocket.m_sAddr)) == SOCKET_ERROR)
	{
		m_strLastError.Format(_T("Bind socket to %s (%d) failed! %d"), lpszIPAddr, nPort, WSAGetLastError());
		TRACE("%s\r\n", m_strLastError);
		return FALSE;
	}

	if(::listen(m_oSocket.m_hSocket, 5) == SOCKET_ERROR)
	{
		m_strLastError.Format(_T("socket start listen failed! %d"), WSAGetLastError());
		TRACE("%s\r\n", m_strLastError);
		Destroy();
		return FALSE;
	}

	return TRUE;
}

void CNxCmdTCPTranscv::Destroy()
{
	m_oThread.DestroyThread();

	if(IsServer())
	{
		m_aIncomings.RemoveAll();
		m_mapIncomings.RemoveAll();
	}

	m_oSocket.Clear();

	// Free all messages
	while(m_lstEvents.GetCount())
		delete m_lstEvents.RemoveHead();
}

BOOL CNxCmdTCPTranscv::SendCmd(CNxSocketCmd* pCmd, SOCKET socket/* = NULL*/)
{
	if(!IsCreated())
		return FALSE;

	M5RCPtr<CNxSocketCmd> pJCmd = pCmd;
	CSingleLock lock(&m_oCriSect, TRUE);

	if(IsServer())
	{
		pCmd->m_sHead.fccFrom = m_oSocket.m_fccFlag;

		if(socket != NULL)
		{
			M5RCPtr<CNxCmdSocket> pJSocket;
			if(!GetIncomingSocket(socket, pJSocket))
			{
				m_strLastError = _T("Incoming socket is closed! send cmd failed.");
				TRACE("%s\r\n", m_strLastError);
				
				return FALSE;
			}

			if(pJSocket->m_fccFlag != 0)
			{
				pCmd->m_sHead.fccTo = pJSocket->m_fccFlag;
				pJSocket->m_lstSendCmds.AddTail(pCmd);
				m_lstNewSends.AddTail(socket);
			}
		}
		else	// Send to all
		{
			for(int i = 0; i < (int)m_aIncomings.GetCount(); i++)
			{
				if(m_aIncomings[i]->m_fccFlag == 0)
					continue;

				if(i > 0) 
					pCmd = pCmd->MakeRefCopy();

				pCmd->m_sHead.fccTo = m_aIncomings[i]->m_fccFlag;
				m_aIncomings[i]->m_lstSendCmds.AddTail(pCmd);
				m_lstNewSends.AddTail(m_aIncomings[i]->m_hSocket);
			}
		}
	}
	else
	{
		pCmd->m_sHead.fccFrom = m_oSocket.m_fccFlag;
		pCmd->m_sHead.fccTo = m_fccSvrFlag;
		m_oSocket.m_lstSendCmds.AddTail(pCmd);
		m_lstNewSends.AddTail(m_oSocket.m_hSocket);
	}

	m_oThread.SetTrigger();

	return TRUE;
}

SNxCmdSocketEvent * CNxCmdTCPTranscv::PeekEvent()
{
	if(!IsCreated())
		return NULL;

	CSingleLock lock(&m_oCriSect, TRUE);
	if(!m_lstEvents.IsEmpty())
		return m_lstEvents.RemoveHead();

	return NULL;
}

int	 CNxCmdTCPTranscv::GetIncomingCount()
{
	CSingleLock lock(&m_oCriSect, TRUE);
	return (int)m_aIncomings.GetCount();
}

BOOL CNxCmdTCPTranscv::GetIncomingSocket(int nIdx, M5RCPtr<CNxCmdSocket> & pJSocket)
{
	CSingleLock lock(&m_oCriSect, TRUE);
	if(nIdx < 0 || nIdx > m_aIncomings.GetUpperBound())
		return FALSE;

	pJSocket = m_aIncomings.GetAt(nIdx);
	return TRUE;
}

BOOL CNxCmdTCPTranscv::GetIncomingSocket(SOCKET socket, M5RCPtr<CNxCmdSocket> & pJSocket)
{
	CSingleLock lock(&m_oCriSect, TRUE);

	CNxCmdSocket * pSocket = NULL;
	if(m_mapIncomings.Lookup((UINT)socket, pSocket))
		pJSocket = pSocket;

	return pSocket != NULL;
}

BOOL CNxCmdTCPTranscv::RemoveIncoming(SOCKET socket)
{
	CSingleLock lock(&m_oCriSect, TRUE);

	for(int i = 0; i < (int)m_aIncomings.GetCount(); i++)
	{
		if(m_aIncomings[i]->m_hSocket == socket)
		{
			m_oThread.RemoveExtEvent(m_aIncomings[i]->m_hWaitevent);
			m_aIncomings.RemoveAt(i);
			m_mapIncomings.RemoveKey((UINT)socket);
			return TRUE;
		}
	}

	return FALSE;
}

void CNxCmdTCPTranscv::GetLastErrString(CString & strErr)
{
	strErr = m_strLastError;
}

int	 CNxCmdTCPTranscv::GetLastErrorCode()
{
	return WSAGetLastError();
}

void CNxCmdTCPTranscv::OnError(SOCKET socket, int nError, LPCTSTR lpszErrMsgFmt)
{
	SNxCmdSocketEvent * pErrEvent = new SNxCmdSocketEvent(this, socket, nError, WSAGetLastError());
	pErrEvent->msg.Format(lpszErrMsgFmt, pErrEvent->nSubcode);
	TRACE(_T("%s\r\n"), pErrEvent->msg);

	if(m_pICallback && m_pICallback->OnSocketEvent(pErrEvent))
	{
		delete pErrEvent;
		return;
	}
	
	if(m_hAsyncNotifyWnd)
	{
		CSingleLock lock(&m_oCriSect, TRUE);
		m_lstEvents.AddTail(pErrEvent);
		PostMessage(m_hAsyncNotifyWnd, ms_nAsyncNotifyMsg, (WPARAM)this, 0);
	}
}

void CNxCmdTCPTranscv::DoNotify(CNxCmdSocket * pSocket, int nNotify, LPCTSTR lpszNotifyMsgFmt)
{
	ASSERT(pSocket);
	SNxCmdSocketEvent * pNotifyEvent = new SNxCmdSocketEvent(this, pSocket->m_hSocket, nNotify, 0);

	CString strIPAddr;
	pSocket->GetIPAddrString(strIPAddr);
	pNotifyEvent->msg.Format(_T("%s %s"), lpszNotifyMsgFmt, strIPAddr);
	TRACE(_T("%s\r\n"), pNotifyEvent->msg);

	if(m_pICallback && m_pICallback->OnSocketEvent(pNotifyEvent))
	{
		delete pNotifyEvent;
		return;
	}

	if(m_hAsyncNotifyWnd)
	{
		CSingleLock lock(&m_oCriSect, TRUE);
		m_lstEvents.AddTail(pNotifyEvent);
		PostMessage(m_hAsyncNotifyWnd, ms_nAsyncNotifyMsg, (WPARAM)this, 0);
	}
}

void CNxCmdTCPTranscv::DoReceive(CNxCmdSocket * pSocket)
{
	ASSERT(pSocket);
	SNxCmdSocketEvent * pReceiveEvent = new SNxCmdSocketEvent(this, pSocket->m_hSocket, (CNxSocketCmd *)pSocket->m_pJRecvCmd);
	pSocket->m_pJRecvCmd = NULL;

	if(m_pICallback && m_pICallback->OnSocketEvent(pReceiveEvent))
	{
		delete pReceiveEvent;
		return;
	}

	if(m_hAsyncNotifyWnd)
	{
		CSingleLock lock(&m_oCriSect, TRUE);
		m_lstEvents.AddTail(pReceiveEvent);
		PostMessage(m_hAsyncNotifyWnd, ms_nAsyncNotifyMsg, (WPARAM)this, 0);
	}
}

void CNxCmdTCPTranscv::GetSocketHasNewSendCmd(CArray<CNxCmdSocket *> & aSockets)
{
	CSingleLock lock(&m_oCriSect, TRUE);

	if(!IsServer())
	{
		aSockets.Add(&m_oSocket);
	}
	else
	{
		M5RCPtr<CNxCmdSocket> pJSocket;
		POSITION pos = m_lstNewSends.GetHeadPosition();
		while(pos)
		{
			SOCKET socket = m_lstNewSends.GetNext(pos);
			if(GetIncomingSocket(socket, pJSocket))
				aSockets.Add((CNxCmdSocket *)pJSocket);
		}
	}

	m_lstNewSends.RemoveAll();
}

CNxSocketCmd * CNxCmdTCPTranscv::GetCurSendCmd(CNxCmdSocket * pSocket, BOOL bNext)
{
	ASSERT(pSocket);
	CSingleLock lock(&m_oCriSect, TRUE);
	if(bNext)
		pSocket->m_lstSendCmds.RemoveHead();

	if(pSocket->m_lstSendCmds.IsEmpty())
		return NULL;

	return (CNxSocketCmd *)pSocket->m_lstSendCmds.GetHead();
}

int	CNxCmdTCPTranscv::GetCurSendCmdCount()
{
	CSingleLock lock(&m_oCriSect, TRUE);
	CArray<CNxCmdSocket *> aSockets;
	GetSocketHasNewSendCmd(aSockets);

	int nRet = 0;
	for(int i = 0; i < (int)aSockets.GetCount(); i++)
		nRet += (int)aSockets[i]->m_lstSendCmds.GetCount();

	return nRet;
}

// CNxThreadCallBack
BOOL CNxCmdTCPTranscv::OnNxThreadTriggered(CNxThread * pThread, DWORD dwWaitObj)
{
	int nWaitEventIdx = dwWaitObj - WAIT_OBJECT_0;
	ASSERT(nWaitEventIdx > 0);

	if(nWaitEventIdx == 1) // Thread trigger 
	{
		CArray<CNxCmdSocket *> aSockets;
		GetSocketHasNewSendCmd(aSockets);

		for(int i = 0; i < (int)aSockets.GetCount(); i++)
			OnSend(aSockets[i], 0);

		return TRUE;
	}

	// Get socket for event
	CNxCmdSocket * pSocket = NULL;
	if(nWaitEventIdx == 2)
	{
		pSocket = &m_oSocket;
	}
	else
	{
		int nIncomingIdx = nWaitEventIdx - 3;
		if(nIncomingIdx < 0 || nIncomingIdx > m_aIncomings.GetUpperBound())
		{
			TRACE("Index of incoming socket is invalid!");
			return TRUE;
		}

		pSocket = m_aIncomings[nIncomingIdx];
	}

	// Enum Network Events
	WSANETWORKEVENTS event;
	if(::WSAEnumNetworkEvents(pSocket->m_hSocket, pSocket->m_hWaitevent, &event) == SOCKET_ERROR)
	{
		OnError(pSocket->m_hSocket, keM5TCmdSocket_EnumEventError, _T("WSAEnumNetworkEvents return an error %d."));
		return TRUE;
	}

	// process event
	if(event.lNetworkEvents & FD_ACCEPT)
		OnAccept(event.iErrorCode[FD_ACCEPT_BIT]);

	if(event.lNetworkEvents & FD_CONNECT)
		OnConnect(pSocket, event.iErrorCode[FD_CONNECT_BIT]);

	if(event.lNetworkEvents & FD_READ)
		OnReceive(pSocket, event.iErrorCode[FD_READ_BIT]);

	if(event.lNetworkEvents & FD_WRITE)
		OnSend(pSocket, event.iErrorCode[FD_WRITE_BIT]);

	if(event.lNetworkEvents & FD_CLOSE)
	{
		if(OnClose(pSocket, event.iErrorCode[FD_CLOSE_BIT]))
			return FALSE;
	}

	return TRUE;
}

void CNxCmdTCPTranscv::OnAccept(int nErrorCode)
{
	if(nErrorCode == 0)
	{
		struct sockaddr_in clientAddress;
		int clientAddrLen = sizeof(sockaddr_in);
		SOCKET socket = ::accept( m_oSocket.m_hSocket, (sockaddr *)&clientAddress, &clientAddrLen );
		if(socket == INVALID_SOCKET)
		{
			OnError(m_oSocket.m_hSocket, keM5TCmdSocket_ErrorOnAccept, _T("NxCmdServer accept Failed! %d"));
			return;
		}

		CNxCmdSocket * pSocket = new CNxCmdSocket();
		pSocket->m_hSocket = socket;
		pSocket->m_sAddr = clientAddress;

		pSocket->m_hWaitevent = WSACreateEvent();
		if( pSocket->m_hWaitevent == NULL )
		{
			OnError(m_oSocket.m_hSocket, keM5TCmdSocket_ErrorOnAccept, _T("Create a WSAEvent failed when accept connect! %d"));
			delete pSocket;
			return;
		}

		if(SOCKET_ERROR == ::WSAEventSelect(pSocket->m_hSocket, pSocket->m_hWaitevent, FD_CONNECT|FD_CLOSE|FD_WRITE|FD_READ))
		{
			OnError(m_oSocket.m_hSocket, keM5TCmdSocket_ErrorOnAccept, _T("Call WSAEventSelect return a error when accept connect! %d"));
			delete pSocket;
			return ;
		}
		else
		if(!m_oThread.AddExtEvent(pSocket->m_hWaitevent))
		{
			OnError(m_oSocket.m_hSocket, keM5TCmdSocket_ErrorOnAccept, _T("Too many event to wait! accept failed!"));
			delete pSocket;
			return ;
		}
		else
		{
			CSingleLock lock(&m_oCriSect, TRUE);
			m_aIncomings.Add(pSocket);
			m_mapIncomings.SetAt((UINT)socket, pSocket);
		}

		DoNotify(pSocket, keM5TCmdSocket_Accept, _T("An incoming socket is Accepted."));

		// init incoming socket
		DWORD nValue = 1;
		int nLen = sizeof( nValue );
		if( SOCKET_ERROR == setsockopt( socket, IPPROTO_TCP, TCP_NODELAY, (char *)&nValue, nLen ) )
			OnError(socket, keM5TCmdSocket_ErrorOnSetOpt, _T("setsockopt(TCP_NODELAY) has error! %d"));

		nValue = 16384;
		if( SOCKET_ERROR == setsockopt( socket, SOL_SOCKET, SO_SNDBUF, (char *)&nValue, nLen ) )
			OnError(socket, keM5TCmdSocket_ErrorOnSetOpt, _T("setsockopt(SO_SNDBUF) has error! %d"));
		if( SOCKET_ERROR == setsockopt( socket, SOL_SOCKET, SO_RCVBUF, (char *)&nValue, nLen ) )
			OnError(socket, keM5TCmdSocket_ErrorOnSetOpt, _T("setsockopt(SO_RCVBUF) has error! %d"));
	}
	else
	{
		ASSERT(nErrorCode == WSAENETDOWN);
		OnError(m_oSocket.m_hSocket, keM5TCmdSocket_NetDown, _T("NxCmdSocket Accept incooming Failed. The network subsystem has failed."));
	}
}

void CNxCmdTCPTranscv::OnConnect(CNxCmdSocket * pSocket, int nErrorCode)
{
	ASSERT(pSocket);
	if(nErrorCode != 0)
	{
		switch(nErrorCode) {
		case WSAEAFNOSUPPORT:
			m_strLastError = _T("Addresses in the specified family cannot be used with this socket.");
			break;
		case WSAECONNREFUSED:
			m_strLastError = _T("The attempt to connect was forcefully rejected.");
			break;
		case WSAENETUNREACH:
			m_strLastError = _T("The network cannot be reached from this host at this time.");
			break;
		case WSAENOBUFS:
			m_strLastError = _T("No buffer space is available. The socket cannot be connected.");
			break;
		case WSAETIMEDOUT:
			m_strLastError = _T("An attempt to connect timed out without establishing a connection.");
			break;
		default :
			m_strLastError = _T("Unknown error for connect!");
			break;
		}

		OnError(pSocket->m_hSocket, keM5TCmdSocket_ConnectFailed, m_strLastError);
		return;
	}

	DoNotify(pSocket, keM5TCmdSocket_Connect, _T("Connect succeeded!"));
}

void CNxCmdTCPTranscv::OnSend(CNxCmdSocket * pSocket, int nErrorCode)
{
	ASSERT(pSocket);

	if(nErrorCode != 0)
	{
		ASSERT(nErrorCode == WSAENETDOWN);
		OnError(pSocket->m_hSocket, keM5TCmdSocket_NetDown, _T("NxCmdSocket Send cmd Failed. The network subsystem has failed."));

		return;
	}

	CNxSocketCmd * pCmd = GetCurSendCmd(pSocket, FALSE);
	while(pCmd)
	{
		if(pCmd->GetDataLeft() == 0)
		{
			// TODO: 到这里这是不合逻辑的，但是实际运行时会来这，有空可以分析下
			ASSERT(FALSE);
			pCmd = GetCurSendCmd(pSocket, TRUE);
			continue;
		}

		int nS = ::send(pSocket->m_hSocket, (char *)pCmd->GetCurPtr(), pCmd->GetDataLeft(), 0);
		if( nS == 0 )
		{
			ASSERT(FALSE);
			return; 
		}
		else
		if( nS == SOCKET_ERROR )
		{
			int nErr = WSAGetLastError();
			if(nErr == WSAEWOULDBLOCK) // It`s not error, send buffer is full.
			{
				TRACE("Got WSAEWOULDBLOCK on send.\r\n");
				return;
			}

			OnError(pSocket->m_hSocket, keM5TCmdSocket_ErrorOnSend, _T("NxCmdSocket Send cmd Failed. %d"));
			return;
		}
		else
		if(pCmd->IsOver(nS))
		{
			pCmd = GetCurSendCmd(pSocket, TRUE);
		}
	}
}

void CNxCmdTCPTranscv::OnReceive(CNxCmdSocket * pSocket, int nErrorCode)
{
	ASSERT(pSocket);
	
	//
	if(nErrorCode != 0)
	{
		ASSERT(nErrorCode == WSAENETDOWN);
		OnError(pSocket->m_hSocket, keM5TCmdSocket_NetDown, _T("NxCmdSocket Receive cmd Failed. The network subsystem has failed."));
		return;
	}	

	//
	while(1)
	{
		if(pSocket->m_pJRecvCmd == NULL)
			pSocket->m_pJRecvCmd = new CNxSocketCmd();

		ASSERT(pSocket->m_pJRecvCmd->GetDataLeft() > 0);
		int nR = ::recv(pSocket->m_hSocket, (char *)pSocket->m_pJRecvCmd->GetCurPtr(), pSocket->m_pJRecvCmd->GetDataLeft(), 0);
		if( pSocket->m_pJRecvCmd->GetDataLeft() != 0 && nR == 0 )
		{
			ASSERT(FALSE);
			//OnClose(pSocket, 0);
			return;
		}
		else
		if( nR == SOCKET_ERROR )
		{
			int nErr = WSAGetLastError();
			if(nErr == WSAEWOULDBLOCK)
			{
				TRACE("Got WSAEWOULDBLOCK on recv.\r\n");
				return;
			}

			OnError(pSocket->m_hSocket, keM5TCmdSocket_ErrorOnReceive, _T("NxCmdSocket Receive cmd Failed. %d"));
		}
		else
		if(pSocket->m_pJRecvCmd->IsOver(nR))
		{
			// cmd send over, remove it form send list
			DoReceive(pSocket);
		}
		else
		if(pSocket->m_pJRecvCmd->IsHeadReceiveOver())
		{
			if(pSocket->m_pJRecvCmd->m_sHead.fccTo != m_oSocket.m_fccFlag)
			{
				LPBYTE c = (LPBYTE)(&pSocket->m_pJRecvCmd->m_sHead.fccTo);
				CString str;
				str.Format(_T("Receiving a command with a bogus fccTo flag. %c%c%c%c"), c[3], c[2], c[1], c[0]);

				pSocket->m_pJRecvCmd = NULL;
				OnError(pSocket->m_hSocket, keM5TCmdSocket_ErrorOnReceive, m_strLastError);
				return;
			}
			else
			if( (m_fccSvrFlag != 0 && pSocket->m_pJRecvCmd->m_sHead.fccFrom != m_fccSvrFlag) ||				// client 
				(IsServer() && pSocket->m_fccFlag != 0 && pSocket->m_pJRecvCmd->m_sHead.fccFrom != pSocket->m_fccFlag) )	// server
			{
				LPBYTE c = (LPBYTE)(&pSocket->m_pJRecvCmd->m_sHead.fccFrom);
				CString str;
				str.Format(_T("Receiving a command with a bogus fccFrom flag. %c%c%c%c"), c[3], c[2], c[1], c[0]);

				pSocket->m_pJRecvCmd = NULL;
				OnError(pSocket->m_hSocket, keM5TCmdSocket_ErrorOnReceive, str);
				return;
			}
			else
			{
				if(pSocket->m_fccFlag == 0)
				{
					pSocket->m_fccFlag = pSocket->m_pJRecvCmd->m_sHead.fccFrom;
				}
				
				if(!pSocket->m_pJRecvCmd->AllocCmdDataBuf(pSocket->m_pJRecvCmd->m_sHead.nDataSize))
				{
					CString str; str.Format(_T("Too Large data size! %d. Cmd data buffer alloc failed!"), pSocket->m_pJRecvCmd->m_sHead.nDataSize);
					pSocket->m_pJRecvCmd = NULL;

					OnError(pSocket->m_hSocket, keM5TCmdSocket_ErrorOnReceive, str);
					break;
				}
			}
		}
	}
}

BOOL CNxCmdTCPTranscv::OnClose(CNxCmdSocket * pSocket, int nErrorCode)
{
	LPARAM lpParam = (LPARAM)keM5TCmdSocket_Close;
	if(nErrorCode != 0)
	{
		lpParam = (LPARAM)keM5TCmdSocket_ErrorOnClose;

		CString strMsg;
		switch(nErrorCode) {
		case WSAENETDOWN: 
			strMsg = _T("The network subsystem has failed.");
			break;
		case WSAECONNRESET:
			strMsg = _T("The connection was reset by the remote side.");
			break;
		case WSAECONNABORTED:
			strMsg = _T("The connection was terminated due to a time-out or other failure.");
			break;
		default :
			strMsg = _T("Unknown error for connect!");
			break;
		}

		OnError(pSocket->m_hSocket, keM5TCmdSocket_ErrorOnClose, strMsg);
	}

	M5RCPtr<CNxCmdSocket> pJSocket;
	if(pSocket == &m_oSocket)
	{
		{
			CSingleLock lock(&m_oCriSect, TRUE);
			m_aIncomings.RemoveAll();
			m_mapIncomings.RemoveAll();
			m_oSocket.Clear();

			// Free all messages
			while(m_lstEvents.GetCount())
				delete m_lstEvents.RemoveHead();
		}

		DoNotify(pSocket, keM5TCmdSocket_Close, _T("CNxCmdTCPTranscv closed!"));
		return TRUE;
	}
	else
	{
		M5RCPtr<CNxCmdSocket> pJSocket = pSocket;
		RemoveIncoming(pSocket->m_hSocket);
		DoNotify(pSocket, keM5TCmdSocket_Close, _T("Incoming socket close. "));
	}

	return FALSE;
}

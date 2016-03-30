
// MyKinectDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MyKinect.h"
#include "MyKinectDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMyKinectDlg �Ի���

CMyKinectDlg::CMyKinectDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMyKinectDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pNuiSensor = NULL;
	m_ColorStreamHandle = INVALID_HANDLE_VALUE;
	m_hNextColorFrameEvent = INVALID_HANDLE_VALUE;
	m_hNextSkeletonEvent = INVALID_HANDLE_VALUE;
	m_hNextDepthFrameEvent = INVALID_HANDLE_VALUE;
	m_DepthStreamHandle = INVALID_HANDLE_VALUE;

	m_bOpen = false;

	//Frame Size
	m_sourceStride = cScreenWidth * sizeof(long);
	m_pRenderColor = NULL;
	m_pRenderDepth = NULL;
	m_pRenderFront = NULL;
	m_pRenderSide  = NULL;
	m_pRenderTop = NULL;
	m_bShowSkeleton = true;
	m_bShowImage = true;
	m_bShowDepth = true;
	m_bIsFullScreen = false;
	m_bHasInit = false;
}

CMyKinectDlg::~CMyKinectDlg()
{
	//CloseCapture();
	if (m_pRenderDepth != NULL)
	{
		delete m_pRenderDepth;
		m_pRenderDepth = NULL;
	}
	if (m_pRenderColor != NULL)
	{
		delete m_pRenderColor;
		m_pRenderColor = NULL;
	}
	if (m_pRenderFront != NULL)
	{
		delete m_pRenderFront;
		m_pRenderFront = NULL;
	}
	if (m_pRenderSide != NULL)
	{
		delete m_pRenderSide;
		m_pRenderSide = NULL;
	}
	if (m_pRenderTop != NULL)
	{
		delete m_pRenderTop;
		m_pRenderTop = NULL;
	}
}


void CMyKinectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMyKinectDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_SWITCH, &CMyKinectDlg::OnBnClickedBtnSwitch)
	ON_BN_CLICKED(IDC_CHECK_SHOWSKELETON, &CMyKinectDlg::OnClickedCheckShowWhich)
	ON_BN_CLICKED(IDC_CHECK_SHOWIMAGE, &CMyKinectDlg::OnClickedCheckShowWhich)
	ON_BN_CLICKED(IDC_CHECK_SHOWDEPTH, &CMyKinectDlg::OnClickedCheckShowWhich)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CMyKinectDlg ��Ϣ�������

BOOL CMyKinectDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	((CButton*)GetDlgItem(IDC_CHECK_SHOWIMAGE))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_CHECK_SHOWSKELETON))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_CHECK_SHOWDEPTH))->SetCheck(BST_CHECKED);

	m_bHasInit = true;
	PostMessage(WM_SIZE);
	CRect rect;
	GetDlgItem(IDC_TOP_SKELETON_VIEW)->GetWindowRect(&rect);
	/*
	CWnd* pWnd = new CWnd;
	pWnd->Create(NULL, _T("1111"), WS_OVERLAPPEDWINDOW|WS_CHILD, rect, this, NULL);
	pWnd->ShowWindow(SW_SHOW);
	CDialog* pDlg = new CDialog;
	pDlg->Create(IDD_DIALOG_TEST, this);
	pDlg->MoveWindow(rect);
	pDlg->ShowWindow(SW_SHOWNORMAL);*/
	
	OnBnClickedBtnSwitch();
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CMyKinectDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CMyKinectDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CMyKinectDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


UINT CMyKinectDlg::CaptureThread(LPVOID pParam)
{
	ASSERT(pParam);
	CMyKinectDlg* pThis = (CMyKinectDlg*)pParam;
	pThis->UpdateCapture();
	
	return 0;
}

void CMyKinectDlg::UpdateCapture()
{
#if 1
	bool bDraw = false;

	HANDLE hEvent[4];
	hEvent[0] = m_hNextSkeletonEvent;
	hEvent[1] = m_hNextColorFrameEvent;
	hEvent[2] = m_hNextDepthFrameEvent;
	hEvent[3] = m_hNextInteractionEvent;
	
	while(m_bOpen)
	{
		DWORD dwEvent = MsgWaitForMultipleObjects(4, hEvent, FALSE, INFINITE, QS_ALLINPUT);
		//if (WAIT_OBJECT_0 == dwEvent)
		{
			bDraw = false;
			if (WAIT_OBJECT_0 == WaitForSingleObject(m_hNextInteractionEvent, 0))
			{
				ProcessInteration();
			}
			if (WAIT_OBJECT_0 == WaitForSingleObject(m_hNextSkeletonEvent, 0) && m_bShowSkeleton)
			{
				ProcessSkeleton();
				m_pRenderFront->Draw();
				m_pRenderSide->Draw();
				m_pRenderTop->Draw();
				bDraw = true;
			}
			if (WAIT_OBJECT_0 == WaitForSingleObject(m_hNextColorFrameEvent, 0) && m_bShowImage)
			{
 				ProcessColorFrame();
				bDraw = true;
			}
			if (WAIT_OBJECT_0 == WaitForSingleObject(m_hNextDepthFrameEvent, 0) && m_bShowDepth)
			{
				ProcessDepthFrame();
				m_pRenderDepth->Draw();
			}
			
			if (bDraw)
			{
				m_pRenderColor->Draw();
			}
			//test
			//ProcessInteration();
			//test end
		}
	}
#endif
}

HRESULT CMyKinectDlg::InitCapture()
{
	INuiSensor* pNuiSensor;
	int nSensorCount = 0;
	HRESULT hr = NuiGetSensorCount(&nSensorCount);
	if (FAILED(hr))
	{
		m_bOpen = false;
		return hr;
	}
	for (int i = 0; i < nSensorCount; i++)
	{
		hr = NuiCreateSensorByIndex(i, &pNuiSensor);
		if (FAILED(hr))
		{
			continue;
		}
		hr = pNuiSensor->NuiStatus();
		if (S_OK == hr)
		{
			m_pNuiSensor = pNuiSensor;
			break;
		}
		pNuiSensor->Release();
	}
	if (NULL != m_pNuiSensor)
	{
		//try  NUI_INITIALIZE_FLAG_USES_HIGH_QUALITY_COLOR -- show nothing
		//DWORD dwFlags = NUI_INITIALIZE_FLAG_USES_SKELETON | NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH;
		DWORD dwFlags = NUI_INITIALIZE_FLAG_USES_SKELETON |
						NUI_INITIALIZE_FLAG_USES_COLOR | 
						NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX;
		hr = m_pNuiSensor->NuiInitialize(dwFlags);
		if (SUCCEEDED(hr))
		{
			m_hNextSkeletonEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			m_hNextColorFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			m_hNextDepthFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			m_hNextInteractionEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

			hr = m_pNuiSensor->NuiSkeletonTrackingEnable(m_hNextSkeletonEvent, 0);
			
			hr = m_pNuiSensor->NuiImageStreamOpen(
				NUI_IMAGE_TYPE_COLOR,
				NUI_IMAGE_RESOLUTION_640x480,
				0,
				2,
				m_hNextColorFrameEvent,
				&m_ColorStreamHandle);
			
			hr = m_pNuiSensor->NuiImageStreamOpen(
				NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX/*NUI_IMAGE_TYPE_DEPTH*/,
				NUI_IMAGE_RESOLUTION_640x480,
				0,
				2,
				m_hNextDepthFrameEvent,
				&m_DepthStreamHandle);

			hr = NuiCreateInteractionStream(m_pNuiSensor, (INuiInteractionClient*)&m_InteractionClient, &m_pInterStream);
			hr = m_pInterStream->Enable(m_hNextInteractionEvent);
			//hr = m_pInterStream->Enable(NULL);
		}
	}

	if (NULL == m_pNuiSensor || FAILED(hr))
	{
		AfxMessageBox(_T("No ready Kinect found"));
		return E_FAIL;
	}

	m_pRenderColor = new CRenderColorFrame;
	m_pRenderColor->Initlize(GetDlgItem(IDC_VIDEO_VIEW)->GetSafeHwnd(), cScreenWidth * 1.0, cScreenHeight * 1.0);
	
	m_pRenderDepth = new CRenderDepthFrame;
	m_pRenderDepth->Initlize(GetDlgItem(IDC_DEPTH_VIEW)->GetSafeHwnd(), cScreenWidth * 1.0, cScreenHeight * 1.0);
	
	m_pRenderSide = new CRenderSkeleton;
	m_pRenderSide->Initlize(GetDlgItem(IDC_SIDE_SKELETON_VIEW)->GetSafeHwnd(), cScreenWidth * 1.0, cScreenHeight * 1.0);
	m_pRenderSide->SetCameraDirection(CamDir_Side);

	m_pRenderFront = new CRenderSkeleton;
	m_pRenderFront->Initlize(GetDlgItem(IDC_FRONT_SKELETON_VIEW)->GetSafeHwnd(), cScreenWidth * 1.0, cScreenHeight * 1.0);
	m_pRenderFront->SetCameraDirection(CamDir_Front);

	m_pRenderTop = new CRenderSkeleton;
	m_pRenderTop->Initlize(GetDlgItem(IDC_TOP_SKELETON_VIEW)->GetSafeHwnd(), cScreenWidth * 1.0, cScreenHeight * 1.0);
	m_pRenderTop->SetCameraDirection(CamDir_Top);

	//create a thread to wait register event
	AfxBeginThread(CaptureThread, (LPVOID)this);
	SetDlgItemText(IDC_BTN_SWITCH, L"�ر� ");
	return hr;
}

HRESULT CMyKinectDlg::CloseCapture()
{
	if (m_pNuiSensor)
	{
		m_pNuiSensor->NuiShutdown();
		//m_pNuiSensor->Release();
	}

	if (m_hNextSkeletonEvent && (m_hNextSkeletonEvent != INVALID_HANDLE_VALUE))
	{
		CloseHandle(m_hNextSkeletonEvent);
	}
	if (m_hNextColorFrameEvent&& (m_hNextColorFrameEvent != INVALID_HANDLE_VALUE))
	{
		CloseHandle(m_hNextColorFrameEvent);
	}

	m_pRenderColor->UnInitlize();
	m_pRenderDepth->UnInitlize();

	SafeRelease(m_pNuiSensor);
	if (GetDlgItem(IDC_BTN_SWITCH)->GetSafeHwnd() != NULL)
	{
		SetDlgItemText(IDC_BTN_SWITCH, L"����");
	}
	return S_OK;
}

void CMyKinectDlg::ProcessSkeleton()
{
	NUI_SKELETON_FRAME skeletonFrame = {0};
	HRESULT hr = m_pNuiSensor->NuiSkeletonGetNextFrame(0, &skeletonFrame);
	if ( FAILED(hr))
	{
		return;
	}
	
	// smooth out the skeleton data
	m_pNuiSensor->NuiTransformSmooth(&skeletonFrame, NULL);

	Vector4 v;
	m_pNuiSensor->NuiAccelerometerGetCurrentReading(&v);
	hr = m_pInterStream->ProcessSkeleton(NUI_SKELETON_COUNT, 
		skeletonFrame.SkeletonData,
		&v,
		skeletonFrame.liTimeStamp);


	m_pRenderColor->SetSkeletonFrame(skeletonFrame);
	m_pRenderDepth->SetSkeletonFrame(skeletonFrame);
	m_pRenderSide->SetSkeletonFrame(skeletonFrame);
	m_pRenderFront->SetSkeletonFrame(skeletonFrame);
	m_pRenderTop->SetSkeletonFrame(skeletonFrame);

	// Device lost, need to recreate the render target
	// We'll dispose it now and retry drawing
	if (D2DERR_RECREATE_TARGET == hr)
	{
		hr = S_OK;
	}
}

void CMyKinectDlg::ProcessColorFrame()
{
	NUI_IMAGE_FRAME imageFrame;
	HRESULT hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_ColorStreamHandle, 0, &imageFrame);
	if ( FAILED(hr))
	{
		return;
	}

	m_pRenderColor->SetColorFrame(imageFrame);
	// Release the frame
	m_pNuiSensor->NuiImageStreamReleaseFrame(m_ColorStreamHandle, &imageFrame);

	// Device lost, need to recreate the render target
	// We'll dispose it now and retry drawing
	if (D2DERR_RECREATE_TARGET == hr)
	{
		hr = S_OK;
	}
}

void CMyKinectDlg::ProcessDepthFrame()
{
#if 0
	m_pRenderDepth->SetDepthFrame((void*)m_pNuiSensor, m_DepthStreamHandle);
	//m_pInterStream->ProcessDepth()
#endif
#if 1
	NUI_IMAGE_FRAME depthFrame;
	HRESULT hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_DepthStreamHandle, 0, &depthFrame);
	if ( FAILED(hr))
	{
		return;
	}

	BOOL nearMode;
	INuiFrameTexture* pTexture;
	hr = m_pNuiSensor->NuiImageFrameGetDepthImagePixelFrameTexture(
		m_DepthStreamHandle, &depthFrame, &nearMode, &pTexture);

	NUI_LOCKED_RECT LockedRect;

	// Lock the frame data so the Kinect knows not to modify it while we're reading it
	pTexture->LockRect(0, &LockedRect, NULL, 0);

	// �ý�������ȥ����������ݣ�����������������ݺ͹������ݼ��㣬����һ���н������ݣ����͵����������ý�������ȥ������  
	m_pInterStream->ProcessDepth(LockedRect.size, PBYTE(LockedRect.pBits), depthFrame.liTimeStamp);

	m_pRenderDepth->SetDepthData(LockedRect);
#if 0
	// Make sure we've received valid data
	if (LockedRect.Pitch != 0)
	{
		// Get the min and max reliable depth for the current frame
		int minDepth = (nearMode ? NUI_IMAGE_DEPTH_MINIMUM_NEAR_MODE : NUI_IMAGE_DEPTH_MINIMUM) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;
		int maxDepth = (nearMode ? NUI_IMAGE_DEPTH_MAXIMUM_NEAR_MODE : NUI_IMAGE_DEPTH_MAXIMUM) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;

		
		BYTE * rgbrun = new BYTE[cScreenWidth * cScreenHeight * cBytesPerPixel];
		const NUI_DEPTH_IMAGE_PIXEL * pBufferRun = reinterpret_cast<const NUI_DEPTH_IMAGE_PIXEL *>(LockedRect.pBits);

		// end pixel is start + width*height - 1
		const NUI_DEPTH_IMAGE_PIXEL * pBufferEnd = pBufferRun + (cScreenWidth * cScreenHeight);

		while ( pBufferRun < pBufferEnd )
		{
			// discard the portion of the depth that contains only the player index
			USHORT depth = pBufferRun->depth;

			// To convert to a byte, we're discarding the most-significant
			// rather than least-significant bits.
			// We're preserving detail, although the intensity will "wrap."
			// Values outside the reliable depth range are mapped to 0 (black).

			// Note: Using conditionals in this loop could degrade performance.
			// Consider using a lookup table instead when writing production code.
			BYTE intensity = static_cast<BYTE>(depth >= minDepth && depth <= maxDepth ? depth % 256 : 0);

			// Write out blue byte
			*(rgbrun++) = intensity;

			// Write out green byte
			*(rgbrun++) = intensity;

			// Write out red byte
			*(rgbrun++) = intensity;

			// We're outputting BGR, the last byte in the 32 bits is unused so skip it
			// If we were outputting BGRA, we would write alpha here.
			++rgbrun;

			// Increment our index into the Kinect's depth buffer
			++pBufferRun;
		}

		// Draw the data with Direct2D
		m_pRenderDepth->SetDepthData(rgbrun, cScreenWidth * cScreenHeight * cBytesPerPixel);
		delete[] rgbrun;
	}
#endif
	// We're done with the texture so unlock it
	pTexture->UnlockRect(0);

	pTexture->Release();

	// Release the frame
	m_pNuiSensor->NuiImageStreamReleaseFrame(m_DepthStreamHandle, &depthFrame);

#endif
}

void CMyKinectDlg::ProcessInteration()
{
	NUI_INTERACTION_FRAME Interaction_Frame;
	HRESULT ret = m_pInterStream->GetNextFrame( 0, &Interaction_Frame);
	if (ret == E_POINTER)
	{
		OutputDebugString(L"Failed GetNextFrame  E_POINTER\n");
		return ;
	}
	else if (ret == E_NUI_FRAME_NO_DATA)
	{
		OutputDebugString(L"Failed GetNextFrame  E_NUI_FRAME_NO_DATA\n");
		return ;
	}
	else if( FAILED( ret  ) ) {
		OutputDebugString(L"Failed GetNextFrame\n");
		return ;
	}

	int trackingID = 0;
	//  COORD pos = {0,0};
	//  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	// SetConsoleCursorPosition(hOut, pos);   

	// ���ݽ��������ṩ�����ݣ���ӡץȭ�������ֵ���Ϣ
	static int LeftCatchIndex = 0;
	static int RightCatchIndex = 0;
	for(int i=0;i<NUI_SKELETON_COUNT;i++)
	{
		trackingID = Interaction_Frame.UserInfos[i].SkeletonTrackingId;
		if (trackingID > 0)
		{
			JudgeHandGesture(Interaction_Frame.UserInfos[i].HandPointerInfos[0]);
			JudgeHandGesture(Interaction_Frame.UserInfos[i].HandPointerInfos[1]);
		}
	}
	return ;
}


void CMyKinectDlg::SetStatusMessage(WCHAR* szMessage)
{
	//SendDlgItemMessageW(m_hWnd, IDC_STATUS, WM_SETTEXT, 0, (LPARAM)szMessage);
	TRACE(szMessage);
}

void CMyKinectDlg::OnBnClickedBtnSwitch()
{
	m_bOpen = !m_bOpen;
	Sleep(1000);
	m_bOpen ? InitCapture() : CloseCapture();
}


void CMyKinectDlg::OnClickedCheckShowWhich()
{
	m_bShowSkeleton = ((CButton*)GetDlgItem(IDC_CHECK_SHOWSKELETON))->GetCheck() > 0 ? true : false;
	m_bShowImage = ((CButton*)GetDlgItem(IDC_CHECK_SHOWIMAGE))->GetCheck() > 0 ? true : false;
	m_bShowDepth = ((CButton*)GetDlgItem(IDC_CHECK_SHOWDEPTH))->GetCheck() > 0 ? true : false;
}


void CMyKinectDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	RECT rc;
	GetClientRect(&rc);
	int nHeight = rc.bottom;
	int nWidth = rc.right;
	
	if (m_bHasInit)
	{
		{	//����������
			GetDlgItem(IDC_COLOR_VIEW)->MoveWindow(0, 0, rc.right/2, (int)(rc.bottom * 0.8));
		}
		{	//�Ҳ��ĸ�С����
			GetDlgItem(IDC_DEPTH_VIEW)->MoveWindow(rc.right/2 + 1, 0, rc.right/4, (int)(rc.bottom*0.8 / 2-1));
			GetDlgItem(IDC_FRONT_SKELETON_VIEW)->MoveWindow(rc.right * 3/4 + 1, 0, rc.right/4, (int)(rc.bottom*0.8 / 2-1));
			GetDlgItem(IDC_SIDE_SKELETON_VIEW)->MoveWindow(rc.right/2 + 1, (int)(rc.bottom*0.8 / 2), rc.right/4, (int)(rc.bottom*0.8 / 2-1));
			GetDlgItem(IDC_TOP_SKELETON_VIEW)->MoveWindow(rc.right * 3/4 + 1, (int)(rc.bottom*0.8 / 2), rc.right/4, (int)(rc.bottom*0.8 / 2-1));
		}
		{	//�ײ��ؼ�
			GetDlgItem(IDC_BTN_SWITCH)->MoveWindow(3, (int)(rc.bottom - 50), 60, 30);
			GetDlgItem(IDC_CHECK_SHOWSKELETON)->MoveWindow(70, (int)(rc.bottom - 50), 100, 30);
			GetDlgItem(IDC_CHECK_SHOWIMAGE)->MoveWindow(180, (int)(rc.bottom - 50), 100, 30);
			GetDlgItem(IDC_CHECK_SHOWDEPTH)->MoveWindow(290, (int)(rc.bottom - 50), 100, 30);
		}
	}
}

void CMyKinectDlg::JudgeHandGesture(const NUI_HANDPOINTER_INFO& HandPointerInfos)
{
	NUI_HAND_TYPE HandType = HandPointerInfos.HandType;
	NUI_HAND_EVENT_TYPE	HandEvent = HandPointerInfos.HandEventType;
	CString strInfo;
	if (HandType == NUI_HAND_TYPE_LEFT)
	{
		if ( HandEvent == NUI_HAND_EVENT_TYPE_GRIP) 
		{
			strInfo.Format(_T("���� ץȭ x=%f, y=%f\n"), HandPointerInfos.X, HandPointerInfos.Y);
			//TRACE(strInfo);
			m_pRenderColor->m_RenderSkeleton.m_strLeftHandState = strInfo;
		}
		else if ( HandEvent == NUI_HAND_EVENT_TYPE_GRIPRELEASE)
		{
			strInfo.Format(_T("���� ���� x=%f, y=%f\n"), HandPointerInfos.X, HandPointerInfos.Y);
			//TRACE(strInfo);
			m_pRenderColor->m_RenderSkeleton.m_strLeftHandState = strInfo;
		}
		else
		{
			//pass
		}
	}
	else if (HandType == NUI_HAND_TYPE_RIGHT)
	{
		if ( HandEvent == NUI_HAND_EVENT_TYPE_GRIP) {
			strInfo.Format(_T("���� ץȭ x=%f, y=%f\n"), HandPointerInfos.X, HandPointerInfos.Y);
			//TRACE(strInfo);
			m_pRenderColor->m_RenderSkeleton.m_strRightHandState = strInfo;
		}
		else if ( HandEvent == NUI_HAND_EVENT_TYPE_GRIPRELEASE) {
			strInfo.Format(_T("���� ���� x=%f, y=%f"), HandPointerInfos.X, HandPointerInfos.Y);
			//TRACE(strInfo);
			m_pRenderColor->m_RenderSkeleton.m_strRightHandState = strInfo;
		}
		else
		{
			//pass
		}
	}
	else if (HandEvent == NUI_HAND_TYPE_NONE)
	{
		//pass
	}

}
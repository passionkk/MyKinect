
// MyKinectDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MyKinect.h"
#include "MyKinectDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CMyKinectDlg 对话框

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
	m_bShowSkeleton = true;
	m_bShowImage = true;
	m_bShowDepth = true;
}

CMyKinectDlg::~CMyKinectDlg()
{
	//CloseCapture();
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
END_MESSAGE_MAP()


// CMyKinectDlg 消息处理程序

BOOL CMyKinectDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	((CButton*)GetDlgItem(IDC_CHECK_SHOWIMAGE))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_CHECK_SHOWSKELETON))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_CHECK_SHOWDEPTH))->SetCheck(BST_CHECKED);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMyKinectDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
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
	while(m_bOpen)
	{
		HANDLE hEvent[3];
		hEvent[0] = m_hNextSkeletonEvent;
		hEvent[1] = m_hNextColorFrameEvent;
		hEvent[2] = m_hNextDepthFrameEvent;
		DWORD dwEvent = MsgWaitForMultipleObjects(3, hEvent, FALSE, INFINITE, QS_ALLINPUT);
		if (WAIT_OBJECT_0 == dwEvent)
		{
			bDraw = false;
			if (WAIT_OBJECT_0 == WaitForSingleObject(m_hNextSkeletonEvent, 0) && m_bShowSkeleton)
			{
				ProcessSkeleton();
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
				//bDraw = true;
				m_pRenderDepth->Draw();
			}
			if (bDraw)
			{
				m_pRenderColor->Draw();
			}
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
		DWORD dwFlags = NUI_INITIALIZE_FLAG_USES_SKELETON | NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH;
		hr = m_pNuiSensor->NuiInitialize(dwFlags);
		if (SUCCEEDED(hr))
		{
			m_hNextSkeletonEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			m_hNextColorFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			m_hNextDepthFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			
			hr = m_pNuiSensor->NuiSkeletonTrackingEnable(m_hNextSkeletonEvent, 0);
			
			hr = m_pNuiSensor->NuiImageStreamOpen(
				NUI_IMAGE_TYPE_COLOR,
				NUI_IMAGE_RESOLUTION_640x480,
				0,
				2,
				m_hNextColorFrameEvent,
				&m_ColorStreamHandle);
			
			hr = m_pNuiSensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH,
				NUI_IMAGE_RESOLUTION_640x480,
				0,
				2,
				m_hNextDepthFrameEvent,
				&m_DepthStreamHandle);
		}
	}

	if (NULL == m_pNuiSensor || FAILED(hr))
	{
		AfxMessageBox(_T("No ready Kinect found"));
		return E_FAIL;
	}

	m_pRenderColor = new CRender;
	m_pRenderColor->Initlize(GetDlgItem(IDC_VIDEO_VIEW)->GetSafeHwnd(), cScreenWidth * 1.0, cScreenHeight * 1.0);
	m_pRenderColor->SetShow(true, true, false);

	//show color and depth
	m_pRenderDepth = new CRender;
	m_pRenderDepth->Initlize(GetDlgItem(IDC_DEPTH_VIEW)->GetSafeHwnd(), cScreenWidth * 1.0, cScreenHeight * 1.0);
	//show skeleton and depth
	m_pRenderDepth->SetShow(false, true, true);
	//create a thread to wait register event
	AfxBeginThread(CaptureThread, (LPVOID)this);
	SetDlgItemText(IDC_BTN_SWITCH, L"关闭 ");
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
		SetDlgItemText(IDC_BTN_SWITCH, L"开启");
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

	m_pRenderColor->SetSkeletonFrame(skeletonFrame);
	m_pRenderDepth->SetSkeletonFrame(skeletonFrame);
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

	m_pRenderColor->SetImageFrame(imageFrame);
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
#if 1
	m_pRenderDepth->SetDepthFrame((void*)m_pNuiSensor, m_DepthStreamHandle);
#endif
#if 0
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

	// We're done with the texture so unlock it
	pTexture->UnlockRect(0);

	pTexture->Release();


	// Release the frame
	m_pNuiSensor->NuiImageStreamReleaseFrame(m_DepthStreamHandle, &depthFrame);

#endif
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

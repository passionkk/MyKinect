
// MyKinectDlg.h : 头文件
//

#pragma once

#include "NuiApi.h"
#include <d2d1.h>
#include "KinectInteraction.h"	//Interaction

#include "RenderColorFrame.h"
#include "RenderDepthFrame.h"
#include "RenderSkeleton.h"
#include "InteractionClient.h"

// CMyKinectDlg 对话框
class CMyKinectDlg : public CDialogEx
{
// 构造
public:
	CMyKinectDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CMyKinectDlg();
// 对话框数据
	enum { IDD = IDD_MYKINECT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedBtnSwitch();
	afx_msg void OnClickedCheckShowWhich();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()

	//初始化捕获
	HRESULT InitCapture();
	//结束捕获
	HRESULT CloseCapture();
public:
	static UINT CaptureThread(LPVOID pParam);
	void UpdateCapture();
	void ProcessSkeleton();
	void ProcessColorFrame();
	void ProcessDepthFrame();
	void ProcessInteration();
	void SetStatusMessage(WCHAR* szMessage);
	void JudgeHandGesture(const NUI_HANDPOINTER_INFO&  HandPointerInfos);
	
private:
	INuiSensor*				m_pNuiSensor;
	INuiInteractionStream*	m_pInterStream;
	//event
	HANDLE					m_hNextSkeletonEvent;
	HANDLE                  m_hNextColorFrameEvent;
	HANDLE					m_hNextDepthFrameEvent;
	HANDLE					m_hNextInteractionEvent;	//interaction

	//handle
	HANDLE                  m_ColorStreamHandle;
	HANDLE					m_DepthStreamHandle;

	LONG                     m_sourceStride;

	bool					m_bOpen;
	CRenderColorFrame*		m_pRenderColor;
	CRenderDepthFrame*		m_pRenderDepth;
	CRenderSkeleton*		m_pRenderFront;
	CRenderSkeleton*		m_pRenderSide;
	CRenderSkeleton*		m_pRenderTop;
	bool					m_bShowSkeleton;
	bool					m_bShowImage;
	bool					m_bShowDepth;

	bool					m_bIsFullScreen;
	bool					m_bHasInit;
	CWnd*					m_pTestWnd;
	CInteractionClient		m_InteractionClient;
};

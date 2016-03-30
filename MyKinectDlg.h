
// MyKinectDlg.h : ͷ�ļ�
//

#pragma once

#include "NuiApi.h"
#include <d2d1.h>
#include "KinectInteraction.h"	//Interaction

#include "RenderColorFrame.h"
#include "RenderDepthFrame.h"
#include "RenderSkeleton.h"
#include "InteractionClient.h"

// CMyKinectDlg �Ի���
class CMyKinectDlg : public CDialogEx
{
// ����
public:
	CMyKinectDlg(CWnd* pParent = NULL);	// ��׼���캯��
	~CMyKinectDlg();
// �Ի�������
	enum { IDD = IDD_MYKINECT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedBtnSwitch();
	afx_msg void OnClickedCheckShowWhich();
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()

	//��ʼ������
	HRESULT InitCapture();
	//��������
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

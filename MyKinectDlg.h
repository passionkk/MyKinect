
// MyKinectDlg.h : ͷ�ļ�
//

#pragma once

#include "NuiApi.h"
#include <d2d1.h>
#include "Render.h"

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
	void SetStatusMessage(WCHAR* szMessage);
	
private:
	INuiSensor*				m_pNuiSensor;
	//event
	HANDLE					m_hNextSkeletonEvent;
	HANDLE                  m_hNextColorFrameEvent;
	HANDLE					m_hNextDepthFrameEvent;
	//handle
	HANDLE                  m_ColorStreamHandle;
	HANDLE					m_DepthStreamHandle;

	LONG                     m_sourceStride;

	bool					m_bOpen;
	CRender*				m_pRenderColor;
	CRender*				m_pRenderDepth;
	bool					m_bShowSkeleton;
	bool					m_bShowImage;
	bool					m_bShowDepth;
};

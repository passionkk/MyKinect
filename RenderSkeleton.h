#pragma once
#include "RenderBase.h"
#include "KinectInteraction.h"
//йс╫г
enum CameraDirection{
	CamDir_Front = 0,
	CamDir_Side,
	CamDir_Top
};

class CRenderSkeleton :
	public CRenderBase
{
public:
	CRenderSkeleton(void);
	virtual ~CRenderSkeleton(void);

	HRESULT Initlize(HWND hWnd, float windowWidth, float windowHeight);
	void UnInitlize();
	void SetRenderTarget(ID2D1HwndRenderTarget* pRenderTarget);
	
	void SetCameraDirection(CameraDirection camDir);
	void SetSkeletonFrame(NUI_SKELETON_FRAME skeletonFrame) { m_skeletonFrame = skeletonFrame; };
	
	void DrawSkeletonFrame();
	void DrawSkeletonInfo();
	void DrawSkeleton(const NUI_SKELETON_DATA & skel, int windowWidth, int windowHeight);
	D2D1_POINT_2F SkeletonToScreen(Vector4 skeletonPoint, int width, int height);
	void DrawBone(const NUI_SKELETON_DATA & skel, NUI_SKELETON_POSITION_INDEX bone0, NUI_SKELETON_POSITION_INDEX bone1);

	HRESULT Draw(bool bHasDraw = false);

	CString				m_strRenderInfo;
	CString				m_strLeftHandState;
	CString				m_strRightHandState;
private:
	CameraDirection m_nCamDir;

	ID2D1SolidColorBrush*	m_pBrushJointTracked;
	ID2D1SolidColorBrush*   m_pBrushJointInferred;
	ID2D1SolidColorBrush*   m_pBrushBoneTracked;
	ID2D1SolidColorBrush*   m_pBrushBoneInferred;
	ID2D1SolidColorBrush*   m_pBrushRenderInfo;
	D2D1_POINT_2F           m_Points[NUI_SKELETON_POSITION_COUNT];

	NUI_SKELETON_FRAME	m_skeletonFrame;

	//CString				m_strRenderInfo;
	
};


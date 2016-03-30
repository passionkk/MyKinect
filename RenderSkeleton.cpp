#include "StdAfx.h"
#include "RenderSkeleton.h"

CRenderSkeleton::CRenderSkeleton(void)
{
	m_pBrushJointTracked = NULL;
	m_pBrushJointInferred = NULL;
	m_pBrushBoneTracked = NULL;
	m_pBrushBoneInferred = NULL;
	m_pBrushRenderInfo = NULL;
	m_bShowSkeleton = true;
	m_strRenderInfo = _T(".....Render Info.....");
	m_strLeftHandState.Empty();
	m_strRightHandState.Empty();
}


CRenderSkeleton::~CRenderSkeleton(void)
{
}

HRESULT CRenderSkeleton::Initlize(HWND hWnd, float windowWidth, float windowHeight)
{
	HRESULT hr = __super::Initlize(hWnd, windowWidth, windowHeight);

	m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.27f, 0.75f, 0.27f), &m_pBrushJointTracked);
	m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red, 1.0f), &m_pBrushJointInferred);
	m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green, 1.0f), &m_pBrushBoneTracked);
	m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray, 1.0f), &m_pBrushBoneInferred);
	m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow, 2.5f), &m_pBrushRenderInfo);
	return hr;
}

void CRenderSkeleton::UnInitlize()
{
	SafeRelease(m_pBrushJointTracked);
	SafeRelease(m_pBrushJointInferred);
	SafeRelease(m_pBrushBoneTracked);
	SafeRelease(m_pBrushBoneInferred);
	SafeRelease(m_pBrushRenderInfo);
}

void CRenderSkeleton::SetRenderTarget(ID2D1HwndRenderTarget* pRenderTarget)
{
	if (pRenderTarget != NULL)
	{
		m_pRenderTarget = pRenderTarget;
	}
}

HRESULT CRenderSkeleton::Draw(bool bHasDraw)
{
	HRESULT hre = S_OK;
	if (m_pRenderTarget != nullptr)
	{
		if (m_bShowSkeleton)
		{
			if (bHasDraw)
			{
				DrawSkeletonFrame();
			}
			else
			{
				m_pRenderTarget->BeginDraw();
				m_pRenderTarget->Clear();
				DrawSkeletonFrame();
				m_pRenderTarget->EndDraw();
			}
		}
	}
	else
		hre = S_FALSE;
	return hre;
}

void CRenderSkeleton::DrawSkeletonFrame()
{
	DrawSkeletonInfo();
	for (int i = 0 ; i < NUI_SKELETON_COUNT; ++i)
	{
		NUI_SKELETON_TRACKING_STATE trackingState = m_skeletonFrame.SkeletonData[i].eTrackingState;

		if (NUI_SKELETON_TRACKED == trackingState)
		{
			// We're tracking the skeleton, draw it
			DrawSkeleton(m_skeletonFrame.SkeletonData[i], cScreenWidth, cScreenHeight);
		}
		else if (NUI_SKELETON_POSITION_ONLY == trackingState)
		{
			// we've only received the center point of the skeleton, draw that
			D2D1_ELLIPSE ellipse = D2D1::Ellipse(
				SkeletonToScreen(m_skeletonFrame.SkeletonData[i].Position, cScreenWidth, cScreenHeight),
				g_JointThickness,
				g_JointThickness
				);

			m_pRenderTarget->DrawEllipse(ellipse, m_pBrushJointTracked);
		}
	}
}

void CRenderSkeleton::DrawSkeletonInfo()
{
	if (m_pRenderTarget != NULL)
	{
		RECT rc;
		GetClientRect(m_hWndShow, &rc);
		D2D1_RECT_F textLayoutRect = D2D1::RectF(
			static_cast<FLOAT>(rc.left),
			static_cast<FLOAT>(rc.top),
			static_cast<FLOAT>(rc.right - rc.left),
			static_cast<FLOAT>(rc.bottom - rc.top)
			);

		m_pRenderTarget->DrawText(
			m_strRenderInfo,					// Text to render
			m_strRenderInfo.GetLength(),        // Text length
			m_pTextFormat,						// Text format
			textLayoutRect,						// The region of the window where the text will be rendered
			m_pBrushRenderInfo					// The brush used to draw the text
			);

		if (!m_strLeftHandState.IsEmpty())
		{
			textLayoutRect.top += 40;
			m_pRenderTarget->DrawText(
				m_strLeftHandState,					// Text to render
				m_strLeftHandState.GetLength(),     // Text length
				m_pTextFormat,						// Text format
				textLayoutRect,						// The region of the window where the text will be rendered
				m_pBrushRenderInfo					// The brush used to draw the text
				);
		}

		if (!m_strRightHandState.IsEmpty())
		{
			textLayoutRect.top += 40;
			m_pRenderTarget->DrawText(
				m_strRightHandState,				// Text to render
				m_strRightHandState.GetLength(),    // Text length
				m_pTextFormat,						// Text format
				textLayoutRect,						// The region of the window where the text will be rendered
				m_pBrushRenderInfo					// The brush used to draw the text
				);
		}
	}
}

void CRenderSkeleton::DrawSkeleton( const NUI_SKELETON_DATA & skel, int windowWidth, int windowHeight )
{
	for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i)
	{
		m_Points[i] = SkeletonToScreen(skel.SkeletonPositions[i], windowWidth, windowHeight);
	}

	// Render Torso
	DrawBone(skel, NUI_SKELETON_POSITION_HEAD, NUI_SKELETON_POSITION_SHOULDER_CENTER);
	DrawBone(skel, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_LEFT);
	DrawBone(skel, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_RIGHT);
	DrawBone(skel, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SPINE);
	DrawBone(skel, NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_HIP_CENTER);
	DrawBone(skel, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT);
	DrawBone(skel, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_RIGHT);

	// Left Arm
	DrawBone(skel, NUI_SKELETON_POSITION_SHOULDER_LEFT, NUI_SKELETON_POSITION_ELBOW_LEFT);
	DrawBone(skel, NUI_SKELETON_POSITION_ELBOW_LEFT, NUI_SKELETON_POSITION_WRIST_LEFT);
	DrawBone(skel, NUI_SKELETON_POSITION_WRIST_LEFT, NUI_SKELETON_POSITION_HAND_LEFT);

	// Right Arm
	DrawBone(skel, NUI_SKELETON_POSITION_SHOULDER_RIGHT, NUI_SKELETON_POSITION_ELBOW_RIGHT);
	DrawBone(skel, NUI_SKELETON_POSITION_ELBOW_RIGHT, NUI_SKELETON_POSITION_WRIST_RIGHT);
	DrawBone(skel, NUI_SKELETON_POSITION_WRIST_RIGHT, NUI_SKELETON_POSITION_HAND_RIGHT);

	// Left Leg
	DrawBone(skel, NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_KNEE_LEFT);
	DrawBone(skel, NUI_SKELETON_POSITION_KNEE_LEFT, NUI_SKELETON_POSITION_ANKLE_LEFT);
	DrawBone(skel, NUI_SKELETON_POSITION_ANKLE_LEFT, NUI_SKELETON_POSITION_FOOT_LEFT);

	// Right Leg
	DrawBone(skel, NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_KNEE_RIGHT);
	DrawBone(skel, NUI_SKELETON_POSITION_KNEE_RIGHT, NUI_SKELETON_POSITION_ANKLE_RIGHT);
	DrawBone(skel, NUI_SKELETON_POSITION_ANKLE_RIGHT, NUI_SKELETON_POSITION_FOOT_RIGHT);

	// Draw the joints in a different color
	for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i)
	{
		D2D1_ELLIPSE ellipse = D2D1::Ellipse( m_Points[i], g_JointThickness, g_JointThickness );

		if ( skel.eSkeletonPositionTrackingState[i] == NUI_SKELETON_POSITION_INFERRED )
		{
			m_pRenderTarget->DrawEllipse(ellipse, m_pBrushJointInferred, 10.0f);
		}
		else if ( skel.eSkeletonPositionTrackingState[i] == NUI_SKELETON_POSITION_TRACKED )
		{
			m_pRenderTarget->DrawEllipse(ellipse, m_pBrushJointTracked, 10.0f);
		}
	}
}

D2D1_POINT_2F CRenderSkeleton::SkeletonToScreen( Vector4 skeletonPoint, int width, int height )
{
	LONG x, y;
	USHORT depth;

	// Calculate the skeleton's position on the screen
	// NuiTransformSkeletonToDepthImage returns coordinates in NUI_IMAGE_RESOLUTION_320x240 space
	const NUI_IMAGE_RESOLUTION imageResolution = NUI_IMAGE_RESOLUTION_640x480;
	NuiTransformSkeletonToDepthImage(skeletonPoint, &x, &y, &depth, imageResolution);

	LONG backupX = x, backupY = y;
	if (FAILED(NuiImageGetColorPixelCoordinatesFromDepthPixelAtResolution(imageResolution, imageResolution, nullptr, x, y, depth, &x, &y)))
	{
		x = backupX;
		y = backupY;
	}

	DWORD imageWidth, imageHeight;
	NuiImageResolutionToSize(imageResolution, imageWidth, imageHeight);

	FLOAT resultX, resultY, resultZ;
	resultX = x * (width + 1.0f) / imageWidth;
	resultY = y * (height + 1.0f) / imageHeight;
	resultZ = static_cast<float>(skeletonPoint.z / (4-0.8) * imageWidth);

	switch (m_nCamDir)
	{
	case CamDir_Front:
		return D2D1::Point2F(resultX, resultY);
	case CamDir_Side:
		return D2D1::Point2F(resultZ, resultY);
	case CamDir_Top:
		return D2D1::Point2F(resultX, resultZ);
	default:
		return D2D1::Point2F(0.0f, 0.0f);
	}
}

void CRenderSkeleton::DrawBone( const NUI_SKELETON_DATA & skel, NUI_SKELETON_POSITION_INDEX joint0, NUI_SKELETON_POSITION_INDEX joint1 )
{
	NUI_SKELETON_POSITION_TRACKING_STATE joint0State = skel.eSkeletonPositionTrackingState[joint0];
	NUI_SKELETON_POSITION_TRACKING_STATE joint1State = skel.eSkeletonPositionTrackingState[joint1];

	// If we can't find either of these joints, exit
	if (joint0State == NUI_SKELETON_POSITION_NOT_TRACKED || joint1State == NUI_SKELETON_POSITION_NOT_TRACKED)
	{
		return;
	}

	// Don't draw if both points are inferred
	if (joint0State == NUI_SKELETON_POSITION_INFERRED && joint1State == NUI_SKELETON_POSITION_INFERRED)
	{
		return;
	}

	// We assume all drawn bones are inferred unless BOTH joints are tracked
	if (joint0State == NUI_SKELETON_POSITION_TRACKED && joint1State == NUI_SKELETON_POSITION_TRACKED)
	{
		m_pRenderTarget->DrawLine(m_Points[joint0], m_Points[joint1], m_pBrushBoneTracked, g_TrackedBoneThickness);
	}
	else
	{
		m_pRenderTarget->DrawLine(m_Points[joint0], m_Points[joint1], m_pBrushBoneInferred, g_InferredBoneThickness);
	}
}

void CRenderSkeleton::SetCameraDirection( CameraDirection camDir )
{
	m_nCamDir = camDir;
	switch (m_nCamDir)
	{
	case CamDir_Front:
		m_strRenderInfo = _T("正视图");
		break;
	case CamDir_Side:
		m_strRenderInfo = _T("侧视图");
		break;
	case CamDir_Top:
		m_strRenderInfo = _T("俯视图");
		break;
	default:
		m_strRenderInfo = _T("...");
		break;
	}
}

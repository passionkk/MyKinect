#include "StdAfx.h"
#include "Render.h"

const int sourceStride = cScreenWidth * sizeof(long);

CRender::CRender(void)
{
	m_pD2DFactory = NULL;
	m_pRenderTarget = NULL;
	m_pBrushJointTracked = NULL;
	m_pBrushJointInferred = NULL;
	m_pBrushBoneTracked = NULL;
	m_pBrushBoneInferred = NULL;
	m_pBitmap = NULL;

	m_bShowColorFrame = true;
	m_bShowSkeleton = true;
	m_bShowDepthFrame = true;
	m_pDepthRGBX = new BYTE[cScreenWidth * cScreenHeight * cBytesPerPixel];
	m_ulDepth = 0;
}


CRender::~CRender(void)
{
	delete[] m_pDepthRGBX;
}

HRESULT CRender::Initlize(HWND hWnd, float windowWidth, float windowHeight)
{
	HRESULT hr = S_OK;
	if (m_pD2DFactory == NULL)
	{
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
	}
	
	if (NULL == m_pRenderTarget)
	{
		D2D1_SIZE_U size = D2D1::SizeU( (int)windowWidth, (int)windowHeight );
		D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
		rtProps.pixelFormat = D2D1::PixelFormat( DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
		rtProps.usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;

		// Create a Hwnd render target, in order to render to the window set in initialize
		hr = m_pD2DFactory->CreateHwndRenderTarget(
			rtProps,
			D2D1::HwndRenderTargetProperties(hWnd, size), 
			&m_pRenderTarget);

		if ( FAILED(hr) )
		{
			TRACE(L"Couldn't create Direct2D render target!");
			return hr;
		}

		hr = m_pRenderTarget->CreateBitmap(
			size,
			D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE) ),
			&m_pBitmap
			);
		if (FAILED(hr))
		{
			SafeRelease(m_pRenderTarget);
			return hr;
		}

		//light green
		m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.27f, 0.75f, 0.27f), &m_pBrushJointTracked);
		m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red, 1.0f), &m_pBrushJointInferred);
		m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green, 1.0f), &m_pBrushBoneTracked);
		m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray, 1.0f), &m_pBrushBoneInferred);
	}
	
	return hr;
}

void CRender::UnInitlize()
{
	SafeRelease(m_pRenderTarget);
	SafeRelease(m_pBitmap);

	SafeRelease(m_pBrushJointTracked);
	SafeRelease(m_pBrushJointInferred);
	SafeRelease(m_pBrushBoneTracked);
	SafeRelease(m_pBrushBoneInferred);
	SafeRelease(m_pD2DFactory);
}

HRESULT CRender::SetImageFrame(NUI_IMAGE_FRAME imageFrame)
{
	HRESULT hr = S_OK;
	m_imageFrame = imageFrame;

	INuiFrameTexture* pTexture = imageFrame.pFrameTexture;
	NUI_LOCKED_RECT LockedRect;
	//Lock the frame data so the Kinect knows not to modify it while we're reading it
	pTexture->LockRect(0, &LockedRect, NULL, 0);

	//Make sure we've received valid data
	if (LockedRect.Pitch != 0)
	{
		if ( LockedRect.size < ((cScreenHeight - 1) * sourceStride) + (cScreenWidth * 4) )
		{
			return E_INVALIDARG;
		}
		hr = m_pBitmap->CopyFromMemory(NULL, static_cast<BYTE*>(LockedRect.pBits), sourceStride);
		if ( FAILED(hr) )
		{
			return hr;
		}
	}
	// We're done with the texture so unlock it
	pTexture->UnlockRect(0);
	return hr;
}

HRESULT CRender::SetDepthFrame( void* pNuiSensor, /*NUI_IMAGE_FRAME depthFrame,*/ HANDLE handle )
{
	HRESULT hr = S_OK;
	INuiSensor* pINuiSensor = (INuiSensor*)pNuiSensor;
	NUI_IMAGE_FRAME depthFrame;
	hr = pINuiSensor->NuiImageStreamGetNextFrame(handle, 0, &depthFrame);

	m_depthFrame = depthFrame;
	if (pNuiSensor == NULL)
	{
		return S_FALSE;
	}
	
	BOOL nearMode;
	INuiFrameTexture* pTexture;
	hr = pINuiSensor->NuiImageFrameGetDepthImagePixelFrameTexture(
		handle, &m_depthFrame, &nearMode, &pTexture);

	NUI_LOCKED_RECT LockedRect;

	// Lock the frame data so the Kinect knows not to modify it while we're reading it
	pTexture->LockRect(0, &LockedRect, NULL, 0);

	// Make sure we've received valid data
	if (LockedRect.Pitch != 0)
	{
		// Get the min and max reliable depth for the current frame
		int minDepth = (nearMode ? NUI_IMAGE_DEPTH_MINIMUM_NEAR_MODE : NUI_IMAGE_DEPTH_MINIMUM) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;
		int maxDepth = (nearMode ? NUI_IMAGE_DEPTH_MAXIMUM_NEAR_MODE : NUI_IMAGE_DEPTH_MAXIMUM) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;


		BYTE * rgbrun = m_pDepthRGBX;
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
		//SetDepthData(rgbrun, cScreenWidth * cScreenHeight * cBytesPerPixel);
		if ( cScreenWidth * cScreenHeight * cBytesPerPixel < ((cScreenHeight - 1) * sourceStride) + (cScreenWidth * 4) )
		{
			return E_INVALIDARG;
		}

		// Copy the image that was passed in into the direct2d bitmap
		hr = m_pBitmap->CopyFromMemory(NULL, m_pDepthRGBX, sourceStride);
		if ( FAILED(hr) )
		{
			return hr;
		}
	}

	// We're done with the texture so unlock it
	pTexture->UnlockRect(0);

	pTexture->Release();

	// Release the frame
	pINuiSensor->NuiImageStreamReleaseFrame(handle, &depthFrame);
	return hr;
}

//outer don't call this func,bcs byte point copy error or get bitmap from byte point error
HRESULT CRender::SetDepthData(const BYTE* pDepthByte, unsigned long cbImage)
{
	memset(m_pDepthRGBX, 0, cbImage * sizeof(BYTE));
	memcpy(m_pDepthRGBX, pDepthByte, cbImage * sizeof(BYTE));
	m_ulDepth = cbImage;
	HRESULT hr;
	if ( cbImage < ((cScreenHeight - 1) * sourceStride) + (cScreenWidth * 4) )
	{
		return E_INVALIDARG;
	}

	// Copy the image that was passed in into the direct2d bitmap
	//直接从pDepthByte中拷贝也是失败
	hr = m_pBitmap->CopyFromMemory(NULL, m_pDepthRGBX/*pDepthByte*/, sourceStride);

	if ( FAILED(hr) )
	{
		return hr;
	}

	return S_OK;
}
void CRender::SetSkeletonFrame(NUI_SKELETON_FRAME skeletonFrame)
{
	m_skeletonFrame = skeletonFrame;
}

void CRender::SetShow(bool bShowColorFrame, bool bShowSkeleton, bool bShowDepth)
{
	m_bShowColorFrame = bShowColorFrame;
	m_bShowSkeleton = bShowSkeleton;
	m_bShowDepthFrame = bShowDepth;
}

HRESULT CRender::Draw()
{
	HRESULT hre = S_OK;
	if (m_pRenderTarget != nullptr)
	{
		m_pRenderTarget->BeginDraw();
		//m_pRenderTarget->Clear();
		if (m_bShowColorFrame)
		{	// draw color frame
			m_pRenderTarget->DrawBitmap(m_pBitmap);
		}
		if (m_bShowDepthFrame)
		{	// draw depth frame
			m_pRenderTarget->DrawBitmap(m_pBitmap);
		}
		if (m_bShowSkeleton)
		{	//draw skeleton
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
		m_pRenderTarget->EndDraw();
	}
	else
		hre = S_FALSE;
	return hre;
}

void CRender::DrawSkeleton( const NUI_SKELETON_DATA & skel, int windowWidth, int windowHeight )
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

D2D1_POINT_2F CRender::SkeletonToScreen( Vector4 skeletonPoint, int width, int height )
{
	LONG x, y;
	USHORT depth;

	// Calculate the skeleton's position on the screen
	// NuiTransformSkeletonToDepthImage returns coordinates in NUI_IMAGE_RESOLUTION_320x240 space
	const NUI_IMAGE_RESOLUTION imageResolution = NUI_IMAGE_RESOLUTION_640x480;
	NuiTransformSkeletonToDepthImage(skeletonPoint, &x, &y, &depth, imageResolution);
	float screenPointX = static_cast<float>(x * width) / cScreenWidth;
	float screenPointY = static_cast<float>(y * height) / cScreenHeight;

	LONG backupX = x, backupY = y;
	if (FAILED(NuiImageGetColorPixelCoordinatesFromDepthPixelAtResolution(imageResolution, imageResolution, nullptr, x, y, depth, &x, &y)))
	{
		x = backupX;
		y = backupY;
	}

	DWORD imageWidth, imageHeight;
	NuiImageResolutionToSize(imageResolution, imageWidth, imageHeight);

	FLOAT resultX, resultY;
	resultX = x * (width + 1.0f) / imageWidth;
	resultY = y * (height + 1.0f) / imageHeight;

	return D2D1::Point2F(resultX, resultY);
}

void CRender::DrawBone( const NUI_SKELETON_DATA & skel, NUI_SKELETON_POSITION_INDEX joint0, NUI_SKELETON_POSITION_INDEX joint1 )
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

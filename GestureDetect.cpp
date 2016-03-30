#include "StdAfx.h"
#include "GestureDetect.h"


CGestureDetect::CGestureDetect(void)
{
	m_gestureType = GESTURE_NONE;
	m_skeletonPosIndex = NUI_SKELETON_POSITION_HAND_LEFT;
	m_dequeSkeletonData.clear();
}


CGestureDetect::~CGestureDetect(void)
{
}

void CGestureDetect::SetJudgePos( NUI_SKELETON_POSITION_INDEX posIndex )
{
	m_skeletonPosIndex = posIndex;
}

void CGestureDetect::SetSkeletonFrame( const NUI_SKELETON_FRAME& SkeletonFrame )
{
	//目前只计算一个人
	for (int i = 0; i < NUI_SKELETON_COUNT; i++)
	{
		if (SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED)
		{
			m_dequeSkeletonData.push_back(SkeletonFrame.SkeletonData[i]);
			break;
		}
	}
	
	while (m_dequeSkeletonData.size() > cSkeletonFrameMax)
	{
		m_dequeSkeletonData.pop_front();
	}
}

unsigned int CGestureDetect::GetCurrentGesture()
{
	return m_gestureType;
}

bool CGestureDetect::IsInValidRange()
{
	bool bRet = false;

	if (m_dequeSkeletonData.size() == cSkeletonFrameMax)
	{
		if (m_dequeSkeletonData.at(cSkeletonFrameMax-1).SkeletonPositions[m_skeletonPosIndex].y >
			m_dequeSkeletonData.at(cSkeletonFrameMax-1).SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_LEFT].y &&
			m_dequeSkeletonData.at(cSkeletonFrameMax-1).SkeletonPositions[m_skeletonPosIndex].y <
			m_dequeSkeletonData.at(cSkeletonFrameMax-1).SkeletonPositions[NUI_SKELETON_POSITION_HEAD].y
			)
		{
			return true;
		}
		else 
			return false;
	}

	return bRet;
}

bool CGestureDetect::IsInValidRange(NUI_SKELETON_DATA data, NUI_SKELETON_POSITION_INDEX indexLimitSmaller, NUI_SKELETON_POSITION_INDEX indexLimitBigger, 
	NUI_SKELETON_POSITION_INDEX indexIn, bool bIsVertical)
{
	bool bRet = false;
	if (bIsVertical)
	{
		if (data.SkeletonPositions[indexIn].y >= data.SkeletonPositions[indexLimitSmaller].y && 
			data.SkeletonPositions[indexIn].y <= data.SkeletonPositions[indexLimitBigger].y)
			bRet = true;
	}
	else
	{
		if (data.SkeletonPositions[indexIn].x >= data.SkeletonPositions[indexLimitSmaller].x && 
			data.SkeletonPositions[indexIn].x <= data.SkeletonPositions[indexLimitBigger].x)
			bRet = true;
	}
	return bRet;
}

void CGestureDetect::JudgeGesture( NUI_SKELETON_POSITION_INDEX posIndex )
{
	//计算当前手势
#if 0
	if (!IsInValidRange())
	{
		m_gestureType = GESTURE_NONE;
		return ;
	}
	int nHorizonToRight = 0;
	int nHorizonToLeft = 0;
	int nVerticalToTop = 0;
	int nVerticalToBottom = 0;
	for (int i = 0; i < cSkeletonFrameMax - 1; i++)
	{
		if (m_dequeSkeletonData.at(i).SkeletonPositions[posIndex].x <= m_dequeSkeletonData.at(i+1).SkeletonPositions[posIndex].x)
			nHorizonToRight++;
		else
			nHorizonToLeft++;
		if (m_dequeSkeletonData.at(i).SkeletonPositions[posIndex].y <= m_dequeSkeletonData.at(i+1).SkeletonPositions[posIndex].y)
			nVerticalToBottom++;
		else
			nVerticalToTop++;
	}
	m_gestureType = 0;
	nHorizonToLeft > 10 ? m_gestureType |= GESTURE_TOLEFT : m_gestureType |= GESTURE_TORIGHT;
	nVerticalToTop > 10 ? m_gestureType |= GESTURE_TOTOP : m_gestureType |= GESTURE_TOBOTTOM;
#endif
	m_gestureType = 0;
	if (m_HandPointerInfo[0].X < m_HandPointerInfo[1].X)
		m_gestureType |= GESTURE_TORIGHT;
	else
		m_gestureType |= GESTURE_TOLEFT;
	if (m_HandPointerInfo[0].Y < m_HandPointerInfo[1].Y)
		m_gestureType |= GESTURE_TOBOTTOM;
	else
		m_gestureType |= GESTURE_TOTOP;
}

void CGestureDetect::SetHandPointerInfo(NUI_HANDPOINTER_INFO data, int nIndex)
{
	switch(nIndex)
	{
	case 0:
		m_HandPointerInfo[0] = data;
		break;
	case 1:
		m_HandPointerInfo[1] = data;
		break;
	default:
		break;
	}
}

void CGestureDetect::GestureCallback(void(* GestureCalc)(unsigned int))
{
	GestureCalc(m_gestureType);
}

CString CGestureDetect::GetStringFromGestrue(unsigned int gesture)
{
	CString strRet;
	if (gesture & GESTURE_NONE)
	{
		strRet.Append(L"None");
	}
	if (gesture & GESTURE_ACTIVE)
	{
		strRet.Append(L" 激活");
	}
	if (gesture & GESTURE_WAVE)
	{
		strRet.Append(L" 挥舞");
	}
	if (gesture & GESTURE_STAY)
	{
		strRet.Append(L" 原地");
	}
	if (gesture & GESTURE_TOLEFT)
	{
		strRet.Append(L" 向左");
	}
	if (gesture & GESTURE_TORIGHT)
	{
		strRet.Append(L" 向右");
	}
	if (gesture & GESTURE_TOTOP)
	{
		strRet.Append(L" 向上");
	}
	if (gesture & GESTURE_TOBOTTOM)
	{
		strRet.Append(L" 向下");
	}
	if (gesture & GESTURE_TOFRONT)
	{
		strRet.Append(L" 向前");
	}
	if (gesture & GESTURE_TOBACK)
	{
		strRet.Append(L" 向后");
	}
	return strRet;
}
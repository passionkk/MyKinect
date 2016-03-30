#include "StdAfx.h"
#include "GestureDetect.h"


CGestureDetect::CGestureDetect(void)
{
	m_gestureType = GESTURE_NONE;
	m_skeletonPosIndex = NUI_SKELETON_POSITION_HAND_LEFT;
	m_dequeSkeletonFrame.clear();
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
	m_dequeSkeletonFrame.push_back(SkeletonFrame);
	while (m_dequeSkeletonFrame.size() > cSkeletonFrameMax)
	{
		m_dequeSkeletonFrame.pop_front();
	}
}

void CGestureDetect::GetCurrentGesture( GESTURE_TYPE& GestureType, bool bIsLeftHand )
{
	GestureType = m_gestureType;
}

void CGestureDetect::JudgeGesture( NUI_SKELETON_POSITION_INDEX posIndex )
{
	//计算当前手势

}

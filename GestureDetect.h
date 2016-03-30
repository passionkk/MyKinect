#pragma once

#include "NuiApi.h"
#include <deque>

enum GESTURE_TYPE
{
	GESTURE_NONE = 0,
	GESTURE_ACTIVE,
	GESTURE_WAVE,
	GESTURE_STAY,
	GESTURE_TOLEFT,
	GESTURE_TORIGHT,
	GESTURE_TOTOP,
	GESTURE_TOBOTTOM,
	GESTURE_TOFRONT,
	GESTURE_TOBACK
};

const int cSkeletonFrameMax = 20;	//�洢20������֡���ݣ��ݴ������ǰ����

class CGestureDetect
{
public:
	CGestureDetect(void);
	~CGestureDetect(void);

public:
	//����Ҫ�жϵ����岿λ
	void SetJudgePos(NUI_SKELETON_POSITION_INDEX posIndex);
	//���ù�����Ϣ
	void SetSkeletonFrame(const NUI_SKELETON_FRAME& SkeletonFrame);
	//��ȡ��ǰ����
	void GetCurrentGesture(GESTURE_TYPE& GestureType, bool isLeftHand = true);

private:
	void JudgeGesture(NUI_SKELETON_POSITION_INDEX posIndex = NUI_SKELETON_POSITION_HAND_LEFT);

private:
	std::deque<NUI_SKELETON_FRAME>	m_dequeSkeletonFrame;
	GESTURE_TYPE					m_gestureType;
	NUI_SKELETON_POSITION_INDEX		m_skeletonPosIndex;
};


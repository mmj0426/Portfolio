#pragma once
#include "CoreMinimal.h"
#include "SkillConditionType.generated.h"

UENUM(BlueprintType)
enum skill_condition_terms_id //  bit index�� ���.  ��Ŷ set_skill_condition=>skill_condition_mask_value_
{
	SCTI_ZERO = 0,

	SCTI_BLIEND,// ����
	SCTI_SILENCE,// ħ��
	SCTI_FEAR,// ����	
	SCTI_HOLD,// Ȧ��
	SCTI_STUN,// ����
	SCTI_PETRIFY,// ��ȭ
	SCTI_CONFUSION,// ȥ��
	SCTI_FREEZE,// ����
	SCTI_BURN, //ȭ��
	SCTI_POISON, // ��
	SCTI_HATE = 11, // ����
	SCTI_HIDE, // ���̵�
	SCTI_SLOW, // ��ȭ
	SCTI_BLEED, // ����
	SCTI_SLEEP = 15, // ����
	SCTI_STIFFEN, // ����-���
	SCTI_DISEASE, // ����
	SCTI_INCREASED_DAMAGE, // ��������
	SCTI_ARMOR_BREAK, // �� �ı�
	SCTI_DEATH_MATCH_DEBUFF = 26, // ������ ���� �����
	SCTI_KNOCKDOWN = 28,	// �Ѿ���
	SCTI_KNOCKBACK = 29,	// �и�
	SCTI_BOSS_MARK_EFFECT = 23,	// ���� ���� ȿ��

	SCTI_MAX = 64, // �ִ�ġ ����. �� ������ �۰� �����ؾ� ��.
};
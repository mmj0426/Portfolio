#pragma once
#include "CoreMinimal.h"
#include "SkillConditionType.generated.h"

UENUM(BlueprintType)
enum skill_condition_terms_id //  bit index로 사용.  패킷 set_skill_condition=>skill_condition_mask_value_
{
	SCTI_ZERO = 0,

	SCTI_BLIEND,// 암흑
	SCTI_SILENCE,// 침묵
	SCTI_FEAR,// 공포	
	SCTI_HOLD,// 홀드
	SCTI_STUN,// 스턴
	SCTI_PETRIFY,// 석화
	SCTI_CONFUSION,// 혼란
	SCTI_FREEZE,// 빙결
	SCTI_BURN, //화장
	SCTI_POISON, // 독
	SCTI_HATE = 11, // 증오
	SCTI_HIDE, // 하이드
	SCTI_SLOW, // 둔화
	SCTI_BLEED, // 출혈
	SCTI_SLEEP = 15, // 슬립
	SCTI_STIFFEN, // 경직-즉시
	SCTI_DISEASE, // 질병
	SCTI_INCREASED_DAMAGE, // 피해증가
	SCTI_ARMOR_BREAK, // 방어구 파괴
	SCTI_DEATH_MATCH_DEBUFF = 26, // 결투의 전장 디버프
	SCTI_KNOCKDOWN = 28,	// 넘어짐
	SCTI_KNOCKBACK = 29,	// 밀림
	SCTI_BOSS_MARK_EFFECT = 23,	// 보스 낙인 효과

	SCTI_MAX = 64, // 최대치 지정. 이 값보다 작게 설정해야 함.
};
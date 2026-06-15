#include "AgpmSkill.h"
#include "AgpmEventSkillMaster.h"
#include "AgpmSiegeWar.h"
#include "AgpmPvP.h"
#include <stdio.h>

#include "ApLockManager.h"


/////////////////////////////////////////////////////////////////////

INT32	g_alFactorTable[AGPMSKILL_CONST_MAGNIFY_END + 1][3] =
{
	{0,											0,											0										},
	{AGPD_FACTORS_TYPE_CHAR_STATUS,				AGPD_FACTORS_CHARSTATUS_TYPE_CON,			0										},
	{AGPD_FACTORS_TYPE_CHAR_STATUS,				AGPD_FACTORS_CHARSTATUS_TYPE_STR,			0										},
	{AGPD_FACTORS_TYPE_CHAR_STATUS,				AGPD_FACTORS_CHARSTATUS_TYPE_INT,			0										},
	{AGPD_FACTORS_TYPE_CHAR_STATUS,				AGPD_FACTORS_CHARSTATUS_TYPE_DEX,			0										},
	{AGPD_FACTORS_TYPE_CHAR_STATUS,				AGPD_FACTORS_CHARSTATUS_TYPE_CHA,			0										},
	{AGPD_FACTORS_TYPE_CHAR_STATUS,				AGPD_FACTORS_CHARSTATUS_TYPE_WIS,			0										},
	{AGPD_FACTORS_TYPE_CHAR_STATUS,				AGPD_FACTORS_CHARSTATUS_TYPE_MOVEMENT,		0										},
	{AGPD_FACTORS_TYPE_CHAR_STATUS,				AGPD_FACTORS_CHARSTATUS_TYPE_MOVEMENT_FAST,	0										},
	{AGPD_FACTORS_TYPE_CHAR_POINT,				AGPD_FACTORS_CHARPOINT_TYPE_HP,				0										},
	{AGPD_FACTORS_TYPE_CHAR_POINT,				AGPD_FACTORS_CHARPOINT_TYPE_MP,				0										},
	{AGPD_FACTORS_TYPE_CHAR_POINT,				AGPD_FACTORS_CHARPOINT_TYPE_SP,				0										},
	{AGPD_FACTORS_TYPE_CHAR_POINT,				AGPD_FACTORS_CHARPOINT_TYPE_AP,				0										},
	{AGPD_FACTORS_TYPE_CHAR_POINT,				AGPD_FACTORS_CHARPOINT_TYPE_AGRO,			0										},
	{AGPD_FACTORS_TYPE_CHAR_POINT_MAX,			AGPD_FACTORS_CHARPOINTMAX_TYPE_HP,			0										},
	{AGPD_FACTORS_TYPE_CHAR_POINT_MAX,			AGPD_FACTORS_CHARPOINTMAX_TYPE_MP,			0										},
	{AGPD_FACTORS_TYPE_CHAR_POINT_MAX,			AGPD_FACTORS_CHARPOINTMAX_TYPE_SP,			0										},
	{AGPD_FACTORS_TYPE_ATTACK,					AGPD_FACTORS_ATTACK_TYPE_SPEED,				0										},
	{AGPD_FACTORS_TYPE_ATTACK,					AGPD_FACTORS_ATTACK_TYPE_ATTACKRANGE,		0										},
	{AGPD_FACTORS_TYPE_ATTACK,					AGPD_FACTORS_ATTACK_TYPE_SKILL_CAST,		0										},
	{AGPD_FACTORS_TYPE_ATTACK,					AGPD_FACTORS_ATTACK_TYPE_SKILL_DELAY,		0										},
	{AGPD_FACTORS_TYPE_ATTACK,					AGPD_FACTORS_ATTACK_TYPE_HIT_RATE	,		0										},
	{AGPD_FACTORS_TYPE_ATTACK,					AGPD_FACTORS_ATTACK_TYPE_EVADE_RATE,		0										},
	{AGPD_FACTORS_TYPE_ATTACK,					AGPD_FACTORS_ATTACK_TYPE_DODGE_RATE,		0										},

	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MIN,				AGPD_FACTORS_ATTRIBUTE_TYPE_PHYSICAL	},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MIN,				AGPD_FACTORS_ATTRIBUTE_TYPE_MAGIC		},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MIN,				AGPD_FACTORS_ATTRIBUTE_TYPE_WATER		},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MIN,				AGPD_FACTORS_ATTRIBUTE_TYPE_AIR			},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MIN,				AGPD_FACTORS_ATTRIBUTE_TYPE_EARTH		},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MIN,				AGPD_FACTORS_ATTRIBUTE_TYPE_FIRE		},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MIN,				AGPD_FACTORS_ATTRIBUTE_TYPE_POISON		},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MIN,				AGPD_FACTORS_ATTRIBUTE_TYPE_ICE			},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MIN,				AGPD_FACTORS_ATTRIBUTE_TYPE_LIGHTENING	},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MIN,				AGPD_FACTORS_ATTRIBUTE_TYPE_HEROIC		},

	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MAX,				AGPD_FACTORS_ATTRIBUTE_TYPE_PHYSICAL	},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MAX,				AGPD_FACTORS_ATTRIBUTE_TYPE_MAGIC		},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MAX,				AGPD_FACTORS_ATTRIBUTE_TYPE_WATER		},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MAX,				AGPD_FACTORS_ATTRIBUTE_TYPE_AIR			},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MAX,				AGPD_FACTORS_ATTRIBUTE_TYPE_EARTH		},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MAX,				AGPD_FACTORS_ATTRIBUTE_TYPE_FIRE		},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MAX,				AGPD_FACTORS_ATTRIBUTE_TYPE_POISON		},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MAX,				AGPD_FACTORS_ATTRIBUTE_TYPE_ICE			},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MAX,				AGPD_FACTORS_ATTRIBUTE_TYPE_LIGHTENING	},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MAX,				AGPD_FACTORS_ATTRIBUTE_TYPE_HEROIC		},

	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_POINT,	AGPD_FACTORS_ATTRIBUTE_TYPE_PHYSICAL	},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_POINT,	AGPD_FACTORS_ATTRIBUTE_TYPE_MAGIC		},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_POINT,	AGPD_FACTORS_ATTRIBUTE_TYPE_WATER		},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_POINT,	AGPD_FACTORS_ATTRIBUTE_TYPE_AIR			},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_POINT,	AGPD_FACTORS_ATTRIBUTE_TYPE_EARTH		},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_POINT,	AGPD_FACTORS_ATTRIBUTE_TYPE_FIRE		},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_POINT,	AGPD_FACTORS_ATTRIBUTE_TYPE_POISON		},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_POINT,	AGPD_FACTORS_ATTRIBUTE_TYPE_ICE			},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_POINT,	AGPD_FACTORS_ATTRIBUTE_TYPE_LIGHTENING	},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_POINT,	AGPD_FACTORS_ATTRIBUTE_TYPE_HEROIC		},

	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_RATE,		AGPD_FACTORS_ATTRIBUTE_TYPE_PHYSICAL	},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_RATE,		AGPD_FACTORS_ATTRIBUTE_TYPE_PHYSICAL_BLOCK	},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_RATE,		AGPD_FACTORS_ATTRIBUTE_TYPE_SKILL_BLOCK	},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_RATE,		AGPD_FACTORS_ATTRIBUTE_TYPE_HEROIC_MELEE	},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_RATE,		AGPD_FACTORS_ATTRIBUTE_TYPE_HEROIC_RANGE	},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_RATE,		AGPD_FACTORS_ATTRIBUTE_TYPE_HEROIC_MAGIC	},

	{0,											0,											0										},

	{0,											0,											0										},
	{AGPD_FACTORS_TYPE_CHAR_STATUS,				AGPD_FACTORS_CHARSTATUS_TYPE_CON,			0										},
	{AGPD_FACTORS_TYPE_CHAR_STATUS,				AGPD_FACTORS_CHARSTATUS_TYPE_STR,			0										},
	{AGPD_FACTORS_TYPE_CHAR_STATUS,				AGPD_FACTORS_CHARSTATUS_TYPE_INT,			0										},
	{AGPD_FACTORS_TYPE_CHAR_STATUS,				AGPD_FACTORS_CHARSTATUS_TYPE_DEX,			0										},
	{AGPD_FACTORS_TYPE_CHAR_STATUS,				AGPD_FACTORS_CHARSTATUS_TYPE_CHA,			0										},
	{AGPD_FACTORS_TYPE_CHAR_STATUS,				AGPD_FACTORS_CHARSTATUS_TYPE_WIS,			0										},
	{AGPD_FACTORS_TYPE_CHAR_STATUS,				AGPD_FACTORS_CHARSTATUS_TYPE_MOVEMENT,		0										},
	{AGPD_FACTORS_TYPE_CHAR_STATUS,				AGPD_FACTORS_CHARSTATUS_TYPE_MOVEMENT_FAST,	0										},
	{AGPD_FACTORS_TYPE_CHAR_POINT,				AGPD_FACTORS_CHARPOINT_TYPE_HP,				0										},
	{AGPD_FACTORS_TYPE_CHAR_POINT,				AGPD_FACTORS_CHARPOINT_TYPE_MP,				0										},
	{AGPD_FACTORS_TYPE_CHAR_POINT,				AGPD_FACTORS_CHARPOINT_TYPE_SP,				0										},
	{AGPD_FACTORS_TYPE_CHAR_POINT,				AGPD_FACTORS_CHARPOINT_TYPE_AP,				0										},
	{AGPD_FACTORS_TYPE_CHAR_POINT,				AGPD_FACTORS_CHARPOINT_TYPE_AGRO,			0										},
	{AGPD_FACTORS_TYPE_CHAR_POINT_MAX,			AGPD_FACTORS_CHARPOINTMAX_TYPE_HP,			0										},
	{AGPD_FACTORS_TYPE_CHAR_POINT_MAX,			AGPD_FACTORS_CHARPOINTMAX_TYPE_MP,			0										},
	{AGPD_FACTORS_TYPE_CHAR_POINT_MAX,			AGPD_FACTORS_CHARPOINTMAX_TYPE_SP,			0										},
	{AGPD_FACTORS_TYPE_ATTACK,					AGPD_FACTORS_ATTACK_TYPE_SPEED,				0										},
	{AGPD_FACTORS_TYPE_ATTACK,					AGPD_FACTORS_ATTACK_TYPE_ATTACKRANGE,		0										},
	{AGPD_FACTORS_TYPE_ATTACK,					AGPD_FACTORS_ATTACK_TYPE_SKILL_CAST,		0										},
	{AGPD_FACTORS_TYPE_ATTACK,					AGPD_FACTORS_ATTACK_TYPE_SKILL_DELAY,		0										},
	{AGPD_FACTORS_TYPE_ATTACK,					AGPD_FACTORS_ATTACK_TYPE_HIT_RATE	,		0										},
	{AGPD_FACTORS_TYPE_ATTACK,					AGPD_FACTORS_ATTACK_TYPE_EVADE_RATE,		0										},
	{AGPD_FACTORS_TYPE_ATTACK,					AGPD_FACTORS_ATTACK_TYPE_DODGE_RATE,		0										},

	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MIN,				AGPD_FACTORS_ATTRIBUTE_TYPE_PHYSICAL	},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MIN,				AGPD_FACTORS_ATTRIBUTE_TYPE_MAGIC		},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MIN,				AGPD_FACTORS_ATTRIBUTE_TYPE_WATER		},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MIN,				AGPD_FACTORS_ATTRIBUTE_TYPE_AIR			},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MIN,				AGPD_FACTORS_ATTRIBUTE_TYPE_EARTH		},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MIN,				AGPD_FACTORS_ATTRIBUTE_TYPE_FIRE		},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MIN,				AGPD_FACTORS_ATTRIBUTE_TYPE_POISON		},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MIN,				AGPD_FACTORS_ATTRIBUTE_TYPE_ICE			},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MIN,				AGPD_FACTORS_ATTRIBUTE_TYPE_LIGHTENING	},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MIN,				AGPD_FACTORS_ATTRIBUTE_TYPE_HEROIC		},

	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MAX,				AGPD_FACTORS_ATTRIBUTE_TYPE_PHYSICAL	},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MAX,				AGPD_FACTORS_ATTRIBUTE_TYPE_MAGIC		},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MAX,				AGPD_FACTORS_ATTRIBUTE_TYPE_WATER		},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MAX,				AGPD_FACTORS_ATTRIBUTE_TYPE_AIR			},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MAX,				AGPD_FACTORS_ATTRIBUTE_TYPE_EARTH		},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MAX,				AGPD_FACTORS_ATTRIBUTE_TYPE_FIRE		},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MAX,				AGPD_FACTORS_ATTRIBUTE_TYPE_POISON		},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MAX,				AGPD_FACTORS_ATTRIBUTE_TYPE_ICE			},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MAX,				AGPD_FACTORS_ATTRIBUTE_TYPE_LIGHTENING	},
	{AGPD_FACTORS_TYPE_DAMAGE,					AGPD_FACTORS_DAMAGE_TYPE_MAX,				AGPD_FACTORS_ATTRIBUTE_TYPE_HEROIC		},

	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_POINT,	AGPD_FACTORS_ATTRIBUTE_TYPE_PHYSICAL	},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_POINT,	AGPD_FACTORS_ATTRIBUTE_TYPE_MAGIC		},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_POINT,	AGPD_FACTORS_ATTRIBUTE_TYPE_WATER		},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_POINT,	AGPD_FACTORS_ATTRIBUTE_TYPE_AIR			},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_POINT,	AGPD_FACTORS_ATTRIBUTE_TYPE_EARTH		},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_POINT,	AGPD_FACTORS_ATTRIBUTE_TYPE_FIRE		},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_POINT,	AGPD_FACTORS_ATTRIBUTE_TYPE_POISON		},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_POINT,	AGPD_FACTORS_ATTRIBUTE_TYPE_ICE			},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_POINT,	AGPD_FACTORS_ATTRIBUTE_TYPE_LIGHTENING	},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_POINT,	AGPD_FACTORS_ATTRIBUTE_TYPE_HEROIC		},

	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_RATE,		AGPD_FACTORS_ATTRIBUTE_TYPE_PHYSICAL	},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_RATE,		AGPD_FACTORS_ATTRIBUTE_TYPE_PHYSICAL_BLOCK	},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_RATE,		AGPD_FACTORS_ATTRIBUTE_TYPE_SKILL_BLOCK	},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_RATE,		AGPD_FACTORS_ATTRIBUTE_TYPE_HEROIC_MELEE	},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_RATE,		AGPD_FACTORS_ATTRIBUTE_TYPE_HEROIC_RANGE	},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_RATE,		AGPD_FACTORS_ATTRIBUTE_TYPE_HEROIC_MAGIC	},

	{0,											0,											0										},

	{0,											0,											0										},
	{AGPD_FACTORS_TYPE_CHAR_STATUS,				AGPD_FACTORS_CHARSTATUS_TYPE_MOVEMENT,		0										},
	{AGPD_FACTORS_TYPE_ATTACK,					AGPD_FACTORS_ATTACK_TYPE_SPEED,				0										},
	{0,											0,											0										}

	//{AGPD_FACTORS_TYPE_CHAR_STATUS,				AGPD_FACTORS_CHARSTATUS_TYPE_LUK,			0										},
	//{AGPD_FACTORS_TYPE_CHAR_STATUS,				AGPD_FACTORS_CHARSTATUS_TYPE_MOVEMENT_FAST,	0										},
	//{AGPD_FACTORS_TYPE_CHAR_POINT,				AGPD_FACTORS_CHARPOINT_TYPE_MAP,			0										},
	//{AGPD_FACTORS_TYPE_CHAR_POINT,				AGPD_FACTORS_CHARPOINT_TYPE_MI,				0										},
	//{AGPD_FACTORS_TYPE_CHAR_POINT_MAX,			AGPD_FACTORS_CHARPOINTMAX_TYPE_AP,			0										},
	//{AGPD_FACTORS_TYPE_CHAR_POINT_MAX,			AGPD_FACTORS_CHARPOINTMAX_TYPE_MAP,			0										},
	//{AGPD_FACTORS_TYPE_CHAR_POINT_MAX,			AGPD_FACTORS_CHARPOINTMAX_TYPE_MI,			0										},
	/*
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_RATE,		AGPD_FACTORS_ATTRIBUTE_TYPE_PHYSICAL	},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_RATE,		AGPD_FACTORS_ATTRIBUTE_TYPE_MAGIC		},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_RATE,		AGPD_FACTORS_ATTRIBUTE_TYPE_FIRE		},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_RATE,		AGPD_FACTORS_ATTRIBUTE_TYPE_EARTH		},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_RATE,		AGPD_FACTORS_ATTRIBUTE_TYPE_AIR			},
	{AGPD_FACTORS_TYPE_DEFENSE,					AGPD_FACTORS_DEFENSE_TYPE_DEFENSE_RATE,		AGPD_FACTORS_ATTRIBUTE_TYPE_WATER		},
	*/
};

INT32	g_lSpecializeLevelPoint[AGPMSKILL_MAX_SPECIALIZE_LEVEL_POINT] = { 5, 25, 45, 65, 85, 0 };

/////////////////////////////////////////////////////////////////////

/*****************************************************************
*   Function : AgpmSkill()
*   Comment  : 생성자 
*   Date&Time : 2002-04-17, 오후 3:20
*   Code By : Seong Yon-jun@NHN Studio 
*****************************************************************/
AgpmSkill::AgpmSkill()
{
	SetModuleName("AgpmSkill");

	// Data의 크기를 설정한다.
	SetModuleData(sizeof(AgpdSkill), AGPMSKILL_DATA_TYPE_SKILL);
	SetModuleData(sizeof(AgpdSkillTemplate), AGPMSKILL_DATA_TYPE_TEMPLATE);
	SetModuleData(sizeof(AgpdSkillSpecializeTemplate), AGPMSKILL_DATA_TYPE_SPECIALIZE);
	SetModuleData(sizeof(AgpdSkillTooltipTemplate), AGPMSKILL_DATA_TYPE_TOOLTIP);

	m_csPacket.SetFlagLength(sizeof(INT16));
	m_csPacket.SetFieldType(
							AUTYPE_INT8,		1,			// operation
							AUTYPE_INT32,		1,			// skill id
							AUTYPE_PACKET,		1,			// Base Packet
							AUTYPE_PACKET,		1,			// factor packet
							AUTYPE_INT32,		1,			// template id
							AUTYPE_INT8,		1,			// status (eAgpmSkillStatus)
							AUTYPE_PACKET,		1,			// action packet
							AUTYPE_UINT8,		1,			// skill point
//							AUTYPE_PACKET,		1,			// mastery packet
//							AUTYPE_UINT8,		1,			// skill의 mastery index
//							AUTYPE_UINT8,		1,			// add sp to mastery result
//							AUTYPE_MEMORY_BLOCK,	1,			// select skill template id

							AUTYPE_PACKET,		AGPMSKILL_MAX_SKILL_BUFF,

//							AUTYPE_UINT32,		1,			// buffed skill end time
//							AUTYPE_INT32,		1,			// caster tid
//							AUTYPE_INT32,		AGPMSKILL_MAX_SKILL_BUFF,	// all buffed skill tid
//							AUTYPE_INT32,		AGPMSKILL_MAX_SKILL_BUFF,	// all buffed skill caster tid
							AUTYPE_MEMORY_BLOCK,	1,		// buffedSkillCombatArg
							AUTYPE_END,			0
							);

	m_csPacketBase.SetFlagLength(sizeof(INT8));
	m_csPacketBase.SetFieldType(
							AUTYPE_INT8,		1,			// base type
							AUTYPE_INT32,		1,			// base id
							AUTYPE_END,			0
							);

	m_csPacketAction.SetFlagLength(sizeof(INT16));
	m_csPacketAction.SetFieldType(
							AUTYPE_INT8,		1,			// action type
							AUTYPE_PACKET,		1,			// target base packet
							AUTYPE_PACKET,		1,			// cast result factor packet
							AUTYPE_POS,			1,			// destination position packet
							AUTYPE_INT8,		1,			// 강제어택 여부 (Ctrl + 스킬) : 강제어택이면 1, 아니면 필드를 뺀다.
							AUTYPE_UINT32,		1,			// cast delay
							AUTYPE_UINT32,		1,			// duration
							AUTYPE_UINT32,		1,			// recast delay
							AUTYPE_UINT8,		1,			// skill level
							AUTYPE_UINT8,		1,			// result factor queueing 여부 (데미지를 바로 보여줄지 큐에 넣을지 여부. 보통때는 암것도 안보내고 바로 보여줘야 할때만 플래그를 1로 세팅해서 보낸다.)
							AUTYPE_UINT32,		1,			// additional effect. 2005.12.15. steeple
							AUTYPE_MEMORY_BLOCK,	1,		// target character id
							AUTYPE_END,			0
							);

	m_csPacketBuff.SetFlagLength(sizeof(INT16));
	m_csPacketBuff.SetFieldType(
							AUTYPE_INT32,		1,			// buffed skill tid
							AUTYPE_UINT32,		1,			// buffed skill end time
							AUTYPE_INT32,		1,			// caster tid
							AUTYPE_UINT8,		1,			// charge level
							AUTYPE_UINT32,		1,			// expired time
							AUTYPE_MEMORY_BLOCK,	1,		// BuffedSkillCombatArg
							AUTYPE_END,			0
							);

//	m_csPacketMastery.SetFlagLength(sizeof(INT8));
//	m_csPacketMastery.SetFieldType(
//							AUTYPE_INT8,		1,			// mastery index
//							AUTYPE_UINT8,		1,			// m_ucTotalInputSP
//
//							AUTYPE_UINT8,		AGPMSKILL_MAX_MASTERY,			// total mastery's m_ucTotalInputSP
//
//							AUTYPE_PACKET,		128,
//
//							// 특화 관련 내용들...
//
//							AUTYPE_END,			0
//							);
//
//	m_csPacketMasteryNode.SetFlagLength(sizeof(INT8));
//	m_csPacketMasteryNode.SetFieldType(
//							AUTYPE_INT8,		1,			// mastery index
//							AUTYPE_INT8,		1,			// skill tree index
//							AUTYPE_INT8,		1,			// node index
//							AUTYPE_INT32,		1,			// skill id
//							AUTYPE_UINT8,		1,			// sp for active node
//							AUTYPE_END,			0
//							);

	SetPacketType(AGPMSKILL_PACKET_TYPE);

//	for (int i = 0; i < AURACE_TYPE_MAX * AUCHARCLASS_TYPE_MAX; ++i)
//	{
//		m_csSkillMasteryTemplate[i].m_lCharRaceType		= 0;
//		m_csSkillMasteryTemplate[i].m_lCharClassType	= 0;
//		m_csSkillMasteryTemplate[i].m_ucNumMastery		= 0;
//		ZeroMemory(m_csSkillMasteryTemplate[i].m_csMastery, sizeof(AgpdMasteryTree) * AGPMSKILL_MAX_MASTERY);
//	}
	
	m_nIndexADCharacter			= 0;
	m_nIndexADCharacterTemplate	= 0;

	m_papmObject			= NULL;
	m_pagpmCharacter		= NULL;
	m_pagpmGrid				= NULL;
	m_pagpmItem				= NULL;
	m_pagpmFactors			= NULL;
	m_pagpmParty			= NULL;
	m_papmEventManager		= NULL;
	m_pagpmEventSkillMaster	= NULL;

	m_lMaxAddedTID		= 0;

	EnableIdle2(TRUE);

	m_ulPrevRemoveClockCount	= 0;
}

/*****************************************************************
*   Function : ~AgpmSkill()
*   Comment  : 소멸자 
*   Date&Time : 2002-04-17, 오후 3:20
*   Code By : Seong Yon-jun@NHN Studio 
*****************************************************************/
AgpmSkill::~AgpmSkill()
{
	
}

/*****************************************************************
*   Function : OnAddModule()
*   Comment  : 
*   Date&Time : 2002-04-17, 오후 3:20
*   Code By : Seong Yon-jun@NHN Studio 
*****************************************************************/
BOOL	AgpmSkill::OnAddModule()
{
	//m_papmEventManager		= (ApmEventManager *)	GetModule("ApmEventManager");
	m_papmObject			= (ApmObject *)			GetModule("ApmObject");
	m_pagpmFactors			= (AgpmFactors *)		GetModule("AgpmFactors");
	m_pagpmCharacter		= (AgpmCharacter *)		GetModule("AgpmCharacter");
	m_pagpmGrid				= (AgpmGrid *)			GetModule("AgpmGrid");
	m_pagpmItem				= (AgpmItem *)			GetModule("AgpmItem");
	m_pagpmParty			= (AgpmParty *)			GetModule("AgpmParty");
	m_pagpmBillInfo			= (AgpmBillInfo *)		GetModule("AgpmBillInfo");
	m_pagpmConfig			= (AgpmConfig *)		GetModule("AgpmConfig");

	m_papmEventManager		= (ApmEventManager *)	GetModule("ApmEventManager");

	if (!m_papmEventManager ||
		!m_pagpmCharacter || 
		!m_pagpmGrid ||
		!m_pagpmItem ||
		!m_pagpmBillInfo)
		return FALSE;

	m_nIndexADCharacter = m_pagpmCharacter->AttachCharacterData(this, sizeof(AgpdSkillAttachData), ConAgpdSkillAttachData, DesAgpdSkillAttachData);
	if (m_nIndexADCharacter < 0)
		return FALSE;

	m_nIndexADCharacterTemplate = m_pagpmCharacter->AttachCharacterTemplateData(this, sizeof(AgpdSkillTemplateAttachData), ConAgpdSkillTemplateAttachData, DesAgpdSkillTemplateAttachData);
	if (m_nIndexADCharacterTemplate < 0)
		return FALSE;

	if (!m_pagpmCharacter->SetCallbackRemoveChar(CBRemoveCharacter, this))
		return FALSE;
	if (!m_pagpmCharacter->SetCallbackUpdateSkillPoint(CBUpdateSkillPoint, this))
		return FALSE;
	if (!m_pagpmCharacter->SetCallbackUpdateLevel(CBUpdateLevel, this))
		return FALSE;
	if (!m_pagpmCharacter->SetCallbackUpdateActionStatus(CBUpdateActionStatus, this))
		return FALSE;
	if (!m_pagpmCharacter->SetCallbackInitChar(CBInitCharacter, this))
		return FALSE;
	if (!m_pagpmItem->SetCallbackGetPetTIDByItemFromSkill(CBGetPetTIDByItemFromSkill, this))
		return FALSE;

	//if (!m_papmEventManager->RegisterEvent(APDEVENT_FUNCTION_SKILL, NULL, NULL, ProcessIdleEvent, NULL, NULL, this))
	//	return FALSE;

	if (!AddStreamCallback(AGPMSKILL_DATA_TYPE_TEMPLATE, TemplateReadCB, TemplateWriteCB, this))
		return FALSE;
	if (!AddStreamCallback(AGPMSKILL_DATA_TYPE_SPECIALIZE, SpecializeReadCB, SpecializeWriteCB, this))
		return FALSE;

	if (!m_pagpmCharacter->AddStreamCallback(AGPMCHAR_DATA_TYPE_TEMPLATE, AttachedTemplateReadCB, AttachedTemplateWriteCB, this))
		return FALSE;

	return TRUE;	
}

/*****************************************************************
*   Function : OnInit()
*   Comment  : 
*   Date&Time : 2002-04-17, 오후 3:21
*   Code By : Seong Yon-jun@NHN Studio 
*****************************************************************/
BOOL	AgpmSkill::OnInit()
{
	if (!m_csAdminSkill.InitializeObject(sizeof(AgpdSkill *) , m_csAdminSkill.GetCount()))
		return FALSE;

	if (!m_csAdminTemplate.InitializeObject(sizeof(AgpdSkillTemplate *) , m_csAdminTemplate.GetCount()))
		return FALSE;

	if (!m_csAdminSpecializeTemplate.InitializeObject(sizeof(AgpdSkillSpecializeTemplate *) , m_csAdminSpecializeTemplate.GetCount()))
		return FALSE;

	if (!m_csAdminTooltipTemplate.InitializeObject(sizeof(AgpdSkillTooltipTemplate *) , m_csAdminTooltipTemplate.GetCount()))
		return FALSE;

	if (!m_csAdminSkillRemove.InitializeObject(sizeof(AgpdSkill *), m_csAdminSkillRemove.GetCount()))
		return FALSE;

	m_pagpmEventSkillMaster = (AgpmEventSkillMaster*) GetModule("AgpmEventSkillMaster");
	if(!m_pagpmEventSkillMaster)
		return FALSE;

	return TRUE;
}

/*****************************************************************
*   Function : OnDestroy()
*   Comment  : 
*   Date&Time : 2002-04-17, 오후 3:44
*   Code By : Seong Yon-jun@NHN Studio 
*****************************************************************/
BOOL	AgpmSkill::OnDestroy()
{
	// 스킬을 모두 삭제한다.

	INT32	lIndex = 0;
	AgpdSkillTemplate **ppcsSkillTemplate = (AgpdSkillTemplate **) m_csAdminTemplate.GetObjectSequence(&lIndex);
	while (ppcsSkillTemplate && *ppcsSkillTemplate)
	{
		DestroySkillTemplate(*ppcsSkillTemplate);

		ppcsSkillTemplate = (AgpdSkillTemplate **) m_csAdminTemplate.GetObjectSequence(&lIndex);
	}

	lIndex = 0;
	AgpdSkill **ppcsSkill = (AgpdSkill **) m_csAdminSkill.GetObjectSequence(&lIndex);
	while (ppcsSkill && *ppcsSkill)
	{
		DestroySkill(*ppcsSkill);

		ppcsSkill = (AgpdSkill **) m_csAdminSkill.GetObjectSequence(&lIndex);
	}

	lIndex = 0;
	AgpdSkillSpecializeTemplate **ppcsSpecialize = (AgpdSkillSpecializeTemplate **) m_csAdminSpecializeTemplate.GetObjectSequence(&lIndex);
	while (ppcsSpecialize && *ppcsSpecialize)
	{
		DestroySpecialize(*ppcsSpecialize);

		ppcsSpecialize = (AgpdSkillSpecializeTemplate **) m_csAdminSpecializeTemplate.GetObjectSequence(&lIndex);
	}

	lIndex = 0;
	AgpdSkillTooltipTemplate **ppcsTooltip = (AgpdSkillTooltipTemplate **) m_csAdminTooltipTemplate.GetObjectSequence(&lIndex);
	while (ppcsTooltip && *ppcsTooltip)
	{
		DestroyTooltipTemplate(*ppcsTooltip);

		ppcsTooltip = (AgpdSkillTooltipTemplate **) m_csAdminTooltipTemplate.GetObjectSequence(&lIndex);
	}

	m_csAdminTemplate.RemoveObjectAll();
	m_csAdminSkill.RemoveObjectAll();
	m_csAdminSpecializeTemplate.RemoveObjectAll();
	m_csAdminTooltipTemplate.RemoveObjectAll();

	return TRUE;
}

BOOL AgpmSkill::OnIdle2(UINT32 ulClockCount)
{
	PROFILE("AgpmSkill::OnIdle2");

	//STOPWATCH2(GetModuleName(), _T("OnIdle2"));

	ProcessRemove(ulClockCount);

	return TRUE;
}

BOOL AgpmSkill::SetMaxSkill(INT32 nCount)
{
	return m_csAdminSkill.SetCount(nCount);
}

BOOL AgpmSkill::SetMaxSkillTemplate(INT32 nCount)
{
	return m_csAdminTemplate.SetCount(nCount);
}

BOOL AgpmSkill::SetMaxSkillTooltipTemplate(INT32 nCount)
{
	return m_csAdminTooltipTemplate.SetCount(nCount);
}

BOOL AgpmSkill::SetMaxSkillSpecializeTemplate(INT32 nCount)
{
	return m_csAdminSpecializeTemplate.SetCount(nCount);
}

BOOL AgpmSkill::SetMaxSkillRemove(INT32 lCount)
{
	return m_csAdminSkillRemove.SetCount(lCount);
}

AgpdSkill* AgpmSkill::CreateSkill()
{
	AgpdSkill	*pcsSkill = (AgpdSkill *) CreateModuleData(AGPMSKILL_DATA_TYPE_SKILL);

	if (pcsSkill)
	{
		pcsSkill->m_Mutex.Init((PVOID) pcsSkill);
		pcsSkill->m_eType				= APBASE_TYPE_SKILL;
		pcsSkill->m_lID					= AP_INVALID_SKILLID;

		ZeroMemory(&pcsSkill->m_csFactor, sizeof(AgpdFactor));

		//ZeroMemory(&pcsSkill->m_acsAffectedBase, sizeof(ApBase) * AGPMSKILL_MAX_AFFECTED_BASE);

		if (m_pagpmFactors)
			m_pagpmFactors->InitFactor(&pcsSkill->m_csFactor);

		pcsSkill->m_pcsBase				= NULL;
		//pcsSkill->m_pcsTargetBase		= NULL;
		pcsSkill->m_csTargetBase.m_eType	= APBASE_TYPE_NONE;
		pcsSkill->m_csTargetBase.m_lID		= 0;
		
		ZeroMemory(&pcsSkill->m_posTarget, sizeof(AuPOS));

		pcsSkill->m_pcsTemplate			= NULL;

		pcsSkill->m_eStatus				= AGPMSKILL_STATUS_NOT_CAST;

		pcsSkill->m_ulStartTime			= 0;
		pcsSkill->m_ulEndTime			= 0;
		pcsSkill->m_ulInterval			= 0;
		pcsSkill->m_ulNextProcessTime	= 0;
		pcsSkill->m_ulCastDelay			= 0;
		pcsSkill->m_ulRecastDelay		= 0;

		pcsSkill->m_pSaveSkillData.Clear();

		pcsSkill->m_lMasteryIndex		= 0;
		pcsSkill->m_lActivedSkillPoint	= 0;

		pcsSkill->m_pcsGridItem			= m_pagpmGrid->CreateGridItem();
		if (pcsSkill->m_pcsGridItem)
			pcsSkill->m_pcsGridItem->SetParentBase((ApBase *) pcsSkill);

		pcsSkill->m_bCloneObject		= FALSE;
	}

	return pcsSkill;
}

BOOL AgpmSkill::DestroySkill(AgpdSkill *pcsSkill)
{
	if (!pcsSkill)
		return FALSE;

	if (m_pagpmFactors)
		m_pagpmFactors->DestroyFactor(&pcsSkill->m_csFactor);

	if (pcsSkill->m_pcsGridItem)
	{
		m_pagpmGrid->DeleteGridItem(pcsSkill->m_pcsGridItem);
		pcsSkill->m_pcsGridItem	= NULL;
	}

	pcsSkill->m_Mutex.Destroy();

	return DestroyModuleData((PVOID) pcsSkill, AGPMSKILL_DATA_TYPE_SKILL);
}

AgpdSkillTemplate* AgpmSkill::CreateSkillTemplate()
{
	AgpdSkillTemplate	*pcsSkillTemplate = (AgpdSkillTemplate *) CreateModuleData(AGPMSKILL_DATA_TYPE_TEMPLATE);

	if (pcsSkillTemplate)
	{
		pcsSkillTemplate->m_Mutex.Init((PVOID) pcsSkillTemplate);
		pcsSkillTemplate->m_eType = APBASE_TYPE_SKILL_TEMPLATE;

		ZeroMemory(pcsSkillTemplate->m_szName, sizeof(CHAR) * AGPMSKILL_MAX_SKILL_NAME + 1);
		ZeroMemory(pcsSkillTemplate->m_szBelowLevelSkillName, sizeof(CHAR) * AGPMSKILL_MAX_SKILL_NAME + 1);

		pcsSkillTemplate->m_nAttribute					= 0;

		pcsSkillTemplate->m_lTargetType					= 0;
		pcsSkillTemplate->m_lRequirementType			= 0;

//		ZeroMemory(pcsSkillTemplate->m_lConditionType, sizeof(UINT64) * AGPMSKILL_MAX_SKILL_CONDITION);
//		ZeroMemory(pcsSkillTemplate->m_stConditionArg, sizeof(stAgpmSkillFormulaArg) * AGPMSKILL_MAX_SKILL_CONDITION);

		pcsSkillTemplate->m_lConditionType.MemSetAll();
		pcsSkillTemplate->m_stConditionArg.MemSetAll();

		pcsSkillTemplate->m_lCostType					= 0;

//		ZeroMemory(pcsSkillTemplate->m_lEffectType, sizeof(UINT64) * AGPMSKILL_MAX_SKILL_CONDITION);
//		ZeroMemory(pcsSkillTemplate->m_lProcessIntervalEffectType, sizeof(UINT64) * AGPMSKILL_MAX_SKILL_CONDITION);
//
//		ZeroMemory(pcsSkillTemplate->m_lEffectType2, sizeof(UINT64) * AGPMSKILL_MAX_SKILL_CONDITION);
//		ZeroMemory(pcsSkillTemplate->m_lProcessIntervalEffectType2, sizeof(UINT64) * AGPMSKILL_MAX_SKILL_CONDITION);

		pcsSkillTemplate->m_lEffectType.MemSetAll();
		pcsSkillTemplate->m_lProcessIntervalEffectType.MemSetAll();

		pcsSkillTemplate->m_lEffectType2.MemSetAll();
		pcsSkillTemplate->m_lProcessIntervalEffectType2.MemSetAll();

		pcsSkillTemplate->m_lEndEffectType				= 0;
		pcsSkillTemplate->m_lRangeType					= 0;
		pcsSkillTemplate->m_lRangeType2					= 0;

		pcsSkillTemplate->m_ulCondition2				= 0;

		//pcsSkillTemplate->m_nClass	= 0;
		//pcsSkillTemplate->m_nRace	= 0;
		ZeroMemory(&pcsSkillTemplate->m_stUseDIRTPoint, sizeof(stDIRT));
		//pcsSkillTemplate->m_nPhase	= 0;
		//ZeroMemory(pcsSkillTemplate->m_szDescription, sizeof(CHAR) * AGPMSKILL_MAX_SKILL_DESCRIPTION);

		ZeroMemory(pcsSkillTemplate->m_fUsedConstFactor, sizeof(FLOAT) * AGPMSKILL_CONST_MAX * AGPMSKILL_MAX_SKILL_CAP);
		ZeroMemory(pcsSkillTemplate->m_fUsedConstFactor2, sizeof(FLOAT) * AGPMSKILL_CONST_MAX * AGPMSKILL_MAX_SKILL_CAP);

		pcsSkillTemplate->m_lID = AP_INVALID_SKILLID;

		pcsSkillTemplate->m_bShrineSkill				= FALSE;

		pcsSkillTemplate->m_pcsGridItem					= m_pagpmGrid->CreateGridItem();
		pcsSkillTemplate->m_pcsGridItemAlarm			= m_pagpmGrid->CreateGridItem();

		if (pcsSkillTemplate->m_pcsGridItem)
			pcsSkillTemplate->m_pcsGridItem->SetParentBase((ApBase *) pcsSkillTemplate);

		pcsSkillTemplate->m_pcsSkillTooltipTemplate		= NULL;
		
		pcsSkillTemplate->m_bIsAutoAttack				= FALSE;

		memset(pcsSkillTemplate->m_allCreatureTID, 0, sizeof(pcsSkillTemplate->m_allCreatureTID));
		
		pcsSkillTemplate->m_lLimitedMaxLevel			= 0;

		pcsSkillTemplate->m_bHighLevelSkill				= 0;
		pcsSkillTemplate->m_bHeroicSkill				= 0;
	}

	return pcsSkillTemplate;
}

BOOL AgpmSkill::DestroySkillTemplate(AgpdSkillTemplate *pcsSkillTemplate)
{
	if (!pcsSkillTemplate)
		return FALSE;

	if (pcsSkillTemplate->m_pcsGridItem)
		m_pagpmGrid->DeleteGridItem(pcsSkillTemplate->m_pcsGridItem);
	if (pcsSkillTemplate->m_pcsGridItemAlarm)
		m_pagpmGrid->DeleteGridItem(pcsSkillTemplate->m_pcsGridItemAlarm);

	pcsSkillTemplate->m_Mutex.Destroy();

	return DestroyModuleData((PVOID) pcsSkillTemplate, AGPMSKILL_DATA_TYPE_TEMPLATE);
}

AgpdSkillSpecializeTemplate* AgpmSkill::CreateSpecialize()
{
	AgpdSkillSpecializeTemplate	*pcsSpecialize = (AgpdSkillSpecializeTemplate *) CreateModuleData(AGPMSKILL_DATA_TYPE_SPECIALIZE);

	if (pcsSpecialize)
	{
		pcsSpecialize->m_Mutex.Init((PVOID) pcsSpecialize);

		ZeroMemory(pcsSpecialize->m_szName, sizeof(CHAR) * AGPMSKILL_MAX_SPECIALIZE_NAME);
		ZeroMemory(pcsSpecialize->m_szDescription, sizeof(CHAR) * AGPMSKILL_MAX_SPECIALIZE_DESCRIPTION);
		ZeroMemory(pcsSpecialize->m_szRestriction, sizeof(CHAR) * AGPMSKILL_MAX_SPECIALIZE_RESTRICTION);

		pcsSpecialize->m_lDuration		= 0;

		pcsSpecialize->m_pcsGridItem	= m_pagpmGrid->CreateGridItem();
	}

	return pcsSpecialize;
}

BOOL AgpmSkill::DestroySpecialize(AgpdSkillSpecializeTemplate *pcsSpecialize)
{
	if (!pcsSpecialize)
		return FALSE;

	if (pcsSpecialize->m_pcsGridItem)
		m_pagpmGrid->DeleteGridItem(pcsSpecialize->m_pcsGridItem);

	pcsSpecialize->m_Mutex.Destroy();

	return DestroyModuleData((PVOID) pcsSpecialize, AGPMSKILL_DATA_TYPE_SPECIALIZE);
}

AgpdSkillTooltipTemplate* AgpmSkill::CreateTooltipTemplate()
{
	AgpdSkillTooltipTemplate	*pcsTooltip = (AgpdSkillTooltipTemplate *) CreateModuleData(AGPMSKILL_DATA_TYPE_TOOLTIP);

	if (pcsTooltip)
	{
		pcsTooltip->m_Mutex.Init((PVOID) pcsTooltip);

		ZeroMemory(pcsTooltip->m_szSkillName, sizeof(CHAR) * (AGPMSKILL_MAX_SKILL_NAME + 1));
		ZeroMemory(pcsTooltip->m_szSkillDescription, sizeof(CHAR) * (AGPMSKILL_MAX_SKILL_DESCRIPTION + 1));

		pcsTooltip->m_ullBasicAttributeType		= 0;
		pcsTooltip->m_ullBasicEffectType		= 0;
		pcsTooltip->m_ullCostAttributeType		= 0;
		pcsTooltip->m_ullDamageAttributeType	= 0;
		pcsTooltip->m_ullSpecialEffectType		= 0;
		pcsTooltip->m_ullFactorPointType		= 0;
		pcsTooltip->m_ullFactorPercentType		= 0;

//		pcsTooltip->m_lBasicAbilityType			= 0;
//		pcsTooltip->m_lRestirctTimeType			= 0;
//		pcsTooltip->m_lOtherAbilityType			= 0;
//		pcsTooltip->m_lCostType					= 0;
	}

	return pcsTooltip;
}

BOOL AgpmSkill::DestroyTooltipTemplate(AgpdSkillTooltipTemplate *pcsTooltip)
{
	if (!pcsTooltip)
		return FALSE;

	pcsTooltip->m_Mutex.Destroy();

	return DestroyModuleData((PVOID) pcsTooltip, AGPMSKILL_DATA_TYPE_TOOLTIP);
}

BOOL AgpmSkill::ConAgpdSkillAttachData(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgpmSkill		*pThis			= (AgpmSkill *)		pClass;
	ApBase			*pcsBase		= (ApBase	*)		pData;

	AgpdSkillAttachData	*pcsAttachSkillData	= pThis->GetAttachSkillData(pcsBase);
	if (pcsAttachSkillData)
	{
//		ZeroMemory(pcsAttachSkillData->m_alSkillID, sizeof(INT32) * AGPMSKILL_MAX_SKILL_OWN);
//		ZeroMemory(pcsAttachSkillData->m_astBuffSkillList, sizeof(stAgpmSkillBuffList) * AGPMSKILL_MAX_SKILL_BUFF);
//		ZeroMemory(pcsAttachSkillData->m_alUseSkillID, sizeof(INT32) * AGPMSKILL_MAX_SKILL_USE);
//		ZeroMemory(pcsAttachSkillData->m_alUsePassiveSkillID, sizeof(INT32) * AGPMSKILL_MAX_SKILL_USE);

		pcsAttachSkillData->m_alSkillID.MemSetAll();
		pcsAttachSkillData->m_astBuffSkillList.MemSetAll();
		pcsAttachSkillData->m_alUseSkillID.MemSetAll();
		pcsAttachSkillData->m_alUsePassiveSkillID.MemSetAll();

		pcsAttachSkillData->m_apcsSkill.MemSetAll();
		pcsAttachSkillData->m_apcsUseSkill.MemSetAll();
		pcsAttachSkillData->m_apcsUsePassiveSkill.MemSetAll();

		pcsAttachSkillData->m_nDefenseTypeEffect		= 0;
		pcsAttachSkillData->m_nMagicDefensePoint		= 0;
		pcsAttachSkillData->m_nMagicReflectPoint		= 0;
		pcsAttachSkillData->m_nMeleeDefensePoint		= 0;
		pcsAttachSkillData->m_nMeleeReflectPoint		= 0;
		pcsAttachSkillData->m_lModifyCharLevel			= 0;

		pcsAttachSkillData->m_lModifiedSkillLevel		= 0;

		//pcsAttachSkillData->m_bPayMasteryRollbackCost	= FALSE;
//		ZeroMemory(pcsAttachSkillData->m_csMastery, sizeof(AgpdSkillMastery) * AGPMSKILL_MAX_MASTERY);

		pcsAttachSkillData->m_ullBuffedSkillCombatEffect	= 0;
		pcsAttachSkillData->m_ullBuffedSkillCombatEffect2	= 0;
		pcsAttachSkillData->m_ullBuffedSkillFactorEffect	= 0;
		pcsAttachSkillData->m_ullBuffedSkillFactorEffect2	= 0;
		ZeroMemory(&pcsAttachSkillData->m_stBuffedSkillCombatEffectArg, sizeof(AgpdSkillBuffedCombatEffectArg));
		ZeroMemory(&pcsAttachSkillData->m_stBuffedSkillFactorEffectArg, sizeof(AgpdSkillBuffedFactorEffectArg));

		pcsAttachSkillData->m_alLastCastTwiceSkillID.MemSetAll();

//		for (int i = 0; i < AGPMSKILL_MAX_MASTERY; ++i)
//		{
//			pcsAttachSkillData->m_csMastery[i].m_ucMasteryIndex	= i;
//		}

		//ZeroMemory(pcsAttachSkillData->m_bSpecializeLevelPoint, sizeof(BOOL) * AGPMSKILL_MAX_SPECIALIZE_LEVEL_POINT);
		pcsAttachSkillData->m_bSpecializeLevelPoint.MemSetAll();
		pcsAttachSkillData->m_lAvailableSpecializeNum	= 0;

		pcsAttachSkillData->m_lWrongSpellCount			= 0;

		pcsAttachSkillData->m_lMeleeComboInCharge		= 0;
	}

	return TRUE;
}

BOOL AgpmSkill::DesAgpdSkillAttachData(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgpmSkill		*pThis			= (AgpmSkill *)		pClass;
	ApBase			*pcsBase		= (ApBase	*)		pData;

	AgpdSkillAttachData	*pcsAttachSkillData	= pThis->GetAttachSkillData(pcsBase);
	if (pcsAttachSkillData)
	{
		for (int i = 0; i < AGPMSKILL_MAX_MASTERY; ++i)
		{
			pThis->m_pagpmGrid->Remove(&pcsAttachSkillData->m_csMasterySpecializeGrid[i]);

			for (int j = 0; j < AGPMSKILL_MAX_MASTERY_SKILL; ++j)
			{
				pThis->m_pagpmGrid->Remove(&pcsAttachSkillData->m_csMasteryGrid[i][j]);
			}
		}
	}

	return TRUE;
}

BOOL AgpmSkill::ConAgpdSkillTemplateAttachData(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgpmSkill		*pThis			= (AgpmSkill *)		pClass;
	ApBase			*pcsBase		= (ApBase	*)		pData;

	AgpdSkillTemplateAttachData	*pcsAttachSkillTemplateData	= pThis->GetAttachSkillTemplateData(pcsBase);
	if (pcsAttachSkillTemplateData)
	{
//		ZeroMemory(pcsAttachSkillTemplateData->m_aszUsableSkillTName, sizeof(CHAR *) * AGPMSKILL_MAX_SKILL_OWN);
		pcsAttachSkillTemplateData->m_aszUsableSkillTName.MemSetAll();
		ZeroMemory(pcsAttachSkillTemplateData->m_aszDefaultTName, sizeof(CHAR) * AGPMSKILL_MAX_DEFAULT_SKILL * (AGPMSKILL_MAX_SKILL_NAME + 1));
	}
	else
		return FALSE;

	return TRUE;
}

BOOL AgpmSkill::DesAgpdSkillTemplateAttachData(PVOID pData, PVOID pClass, PVOID pCustData)
{
	return TRUE;
}

AgpdSkillAttachData* AgpmSkill::GetAttachSkillData(ApBase *pcsBase)
{
	if (!pcsBase)
		return NULL;

	switch (pcsBase->m_eType) {
	case APBASE_TYPE_CHARACTER:
		return (AgpdSkillAttachData *) m_pagpmCharacter->GetAttachedModuleData(m_nIndexADCharacter, (PVOID) pcsBase);
		break;

	case APBASE_TYPE_OBJECT:
		return NULL;
		break;
	}

	return NULL;
}

AgpdSkillTemplateAttachData* AgpmSkill::GetAttachSkillTemplateData(ApBase *pcsBase)
{
	if (!pcsBase)
		return NULL;

	switch (pcsBase->m_eType) {
	case APBASE_TYPE_CHARACTER_TEMPLATE:
		return (AgpdSkillTemplateAttachData *) m_pagpmCharacter->GetAttachedModuleData(m_nIndexADCharacterTemplate, (PVOID) pcsBase);
		break;

	case APBASE_TYPE_OBJECT_TEMPLATE:
		return NULL;
		break;
	}

	return NULL;
}

AgpdSkill* AgpmSkill::AddSkill(INT32 lSkillID, INT32 lSkillTID)
{
	if (lSkillID == AP_INVALID_SKILLID)
		return NULL;

	AgpdSkillTemplate *pcsTemplate = GetSkillTemplate(lSkillTID);
	if (!pcsTemplate)
		return NULL;

	AgpdSkill *pcsSkill = CreateSkill();
	if (!pcsSkill)
		return NULL;

	pcsSkill->m_lID = lSkillID;

	pcsSkill->m_pcsTemplate = pcsTemplate;

	pcsSkill->m_pcsGridItem->m_bMoveable			= TRUE;
	pcsSkill->m_pcsGridItem->m_eType				= AGPDGRID_ITEM_TYPE_SKILL;
	pcsSkill->m_pcsGridItem->m_lItemID				= lSkillID;
	pcsSkill->m_pcsGridItem->m_lItemTID				= lSkillTID;
	pcsSkill->m_pcsGridItem->m_pcsTemplateGrid		= pcsTemplate->m_pcsGridItem;

	return AddSkill(pcsSkill);
}

AgpdSkill* AgpmSkill::AddSkill(AgpdSkill *pcsSkill)
{
	if (!pcsSkill)
		return NULL;

	return m_csAdminSkill.AddSkill(pcsSkill);
}

BOOL AgpmSkill::RemoveSkill(INT32 lSkillID, BOOL bIsDelete)
{
	if (lSkillID == AP_INVALID_SKILLID)
		return FALSE;

	//if (!m_csAdminSkill.GlobalWLock())
	//	return FALSE;

	AgpdSkill	*pcsSkill	= GetSkill(lSkillID);
	//if (!pcsSkill || !pcsSkill->m_Mutex.RemoveLock())
	if (!pcsSkill)
	{
	//	m_csAdminSkill.GlobalRelease();
		return FALSE;
	}

	//if (!m_csAdminSkill.GlobalRelease())
	//{
	//	pcsSkill->m_Mutex.Release();
	//	return FALSE;
	//}

	if (bIsDelete)
		EnumCallback(AGPMSKILL_CB_DELETE_SKILL, pcsSkill, NULL);

	EnumCallback(AGPMSKILL_CB_REMOVE_SKILL, pcsSkill, NULL);

	m_csAdminSkill.RemoveSkill(pcsSkill->m_lID);

	EnumCallback(AGPMSKILL_CB_REMOVE_ID, pcsSkill, NULL);

	pcsSkill->m_Mutex.SafeRelease();

	if (m_csAdminSkillRemove.GetCount() > 0)
	{
		return AddRemoveSkill(pcsSkill);
	}
	else
	{
		return DestroySkill(pcsSkill);
	}

	return	TRUE;
}

/*
BOOL AgpmSkill::DeleteSkill(AgpdSkill *pcsSkill)
{
	EnumCallback(AGPMSKILL_CB_DELETE_SKILL, pcsSkill, NULL);

	return RemoveSkill(pcsSkill);
}
*/

AgpdSkill* AgpmSkill::GetSkill(INT32 lSkillID)
{
	return m_csAdminSkill.GetSkill(lSkillID);
}

AgpdSkill* AgpmSkill::GetSkill(ApBase *pcsBase, CHAR *szSkillName)
{
	if (!pcsBase || !szSkillName)
		return NULL;

	AgpdSkillAttachData	*pcsAttachData	= GetAttachSkillData(pcsBase);
	if (!pcsAttachData)
		return NULL;

//	for (int i = 0; i < AGPMSKILL_MAX_SKILL_OWN; ++i)
//	{
//		if (pcsAttachData->m_alSkillID[i] == AP_INVALID_SKILLID)
//			break;
//
//		AgpdSkill *pcsSkill = GetSkill(pcsAttachData->m_alSkillID[i]);
//		if (!pcsSkill)
//			continue;
//
//		if (!strncmp(((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_szName, szSkillName, AGPMSKILL_MAX_SKILL_NAME))
//			return pcsSkill;
//	}

	for (int i = 0; i < AGPMSKILL_MAX_SKILL_OWN; ++i)
	{
		AgpdSkill *pcsSkill = pcsAttachData->m_apcsSkill[i];
		if (!pcsSkill || !pcsSkill->m_pcsTemplate)
			break;

		if (!strncmp(((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_szName, szSkillName, AGPMSKILL_MAX_SKILL_NAME))
			return pcsSkill;
	}

	return NULL;
}

AgpdSkill* AgpmSkill::GetSkillLock(INT32 lSkillID)
{
//	if (!m_csAdminSkill.GlobalWLock())
//		return NULL;
//
	AgpdSkill *pcsSkill = GetSkill(lSkillID);
	if (!pcsSkill)
	{
//		m_csAdminSkill.GlobalRelease();
		return NULL;
	}

	if (!pcsSkill->m_Mutex.WLock())
	{
//		m_csAdminSkill.GlobalRelease();
		return NULL;
	}

//	if (!m_csAdminSkill.GlobalRelease())
//	{
//		pcsSkill->m_Mutex.Release();
//		return NULL;
//	}

	return pcsSkill;
}

BOOL AgpmSkill::InitSkill(AgpdSkill *pcsSkill)
{
	if (!pcsSkill)
		return FALSE;

	return EnumCallback(AGPMSKILL_CB_INIT_SKILL, pcsSkill, NULL);
}

// 2007.07.23. steeple
// 스킬시간에 관련한 데이터를 초기화 한다.
BOOL AgpmSkill::InitSkillCoolTime(AgpdSkill* pcsSkill)
{
	if(!pcsSkill)
		return FALSE;

	pcsSkill->m_ulEndTime			= 0;
	pcsSkill->m_ulInterval			= 0;
	pcsSkill->m_ulNextProcessTime	= 0;
	pcsSkill->m_ulStartTime			= 0;
	pcsSkill->m_ulCastDelay			= 0;
	pcsSkill->m_ulRecastDelay		= 0;
	pcsSkill->m_pSaveSkillData.Clear();

	return TRUE;
}

AgpdSkillTemplate* AgpmSkill::AddSkillTemplate(INT32 lSkillTID)
{
	if (lSkillTID == AP_INVALID_SKILLID)
	{
		TRACE( "AgpmSkill::AddSkillTemplate AP_INVALID_SKILLID\n" );
//		TRACEFILE(ALEF_ERROR_FILENAME, "AgpmSkill::AddSkillTemplate AP_INVALID_SKILLID\n" );
		return NULL;
	}

	AgpdSkillTemplate *pcsTemplate = CreateSkillTemplate();
	if (!pcsTemplate)
	{
		TRACE( "AgpmSkill::AddSkillTemplate CreateSkillTemplate 실패\n" );
//		TRACEFILE(ALEF_ERROR_FILENAME, "AgpmSkill::AddSkillTemplate CreateSkillTemplate 실패\n" );
		return NULL;
	}

	pcsTemplate->m_lID = lSkillTID;

	if (!m_csAdminTemplate.AddSkillTemplate(pcsTemplate))
	{
		TRACE( "AgpmSkill::AddSkillTemplate 스킬 추가 실패\n" );
//		TRACEFILE(ALEF_ERROR_FILENAME, "AgpmSkill::AddSkillTemplate 스킬 추가 실패\n" );
		DestroySkillTemplate(pcsTemplate);
		return NULL;
	}

	pcsTemplate->m_pcsGridItem->m_bMoveable			= FALSE;
	pcsTemplate->m_pcsGridItem->m_eType				= AGPDGRID_ITEM_TYPE_SKILL;
	pcsTemplate->m_pcsGridItem->m_lItemID			= AP_INVALID_SKILLID;
	pcsTemplate->m_pcsGridItem->m_lItemTID			= lSkillTID;

	pcsTemplate->m_pcsGridItemAlarm->m_bMoveable	= FALSE;
	pcsTemplate->m_pcsGridItemAlarm->m_eType		= AGPDGRID_ITEM_TYPE_SKILL;
	pcsTemplate->m_pcsGridItemAlarm->m_lItemID		= AP_INVALID_SKILLID;
	pcsTemplate->m_pcsGridItemAlarm->m_lItemTID		= lSkillTID;

	return pcsTemplate;
}

BOOL AgpmSkill::RemoveSkillTemplate(INT32 lSkillTID)
{
	AgpdSkillTemplate *pcsTemplate = GetSkillTemplate(lSkillTID);
	if (!pcsTemplate)
		return FALSE;

	m_csAdminTemplate.RemoveSkillTemplate(lSkillTID);

	return DestroySkillTemplate(pcsTemplate);
}

BOOL AgpmSkill::RemoveAllSkillTemplate()
{
	INT32 lIndex = 0;

	AgpdSkillTemplate **ppcsSkillTemplate = (AgpdSkillTemplate **) m_csAdminTemplate.GetObjectSequence(&lIndex);
	while (ppcsSkillTemplate && *ppcsSkillTemplate)
	{
//		DestroySkillTemplate(*ppcsSkillTemplate);
		if(!RemoveSkillTemplate((*(ppcsSkillTemplate))->m_lID))
			return FALSE;

		ppcsSkillTemplate = (AgpdSkillTemplate **) m_csAdminTemplate.GetObjectSequence(&lIndex);
	}

	return m_csAdminTemplate.RemoveObjectAll();
}

AgpdSkillTemplate* AgpmSkill::GetSkillTemplate(INT32 lSkillTID)
{
	return m_csAdminTemplate.GetSkillTemplate(lSkillTID);
}

AgpdSkillTemplate* AgpmSkill::GetSkillTemplate(const CHAR *szName)
{
	return m_csAdminTemplate.GetSkillTemplate(( char * ) szName);
}

AgpdSkillTemplate* AgpmSkill::GetSkillTemplateSequence(INT32 *pnIndex)
{
	if (!pnIndex)
		return NULL;

	AgpdSkillTemplate **ppSkillTemplate = (AgpdSkillTemplate **) m_csAdminTemplate.GetObjectSequence(pnIndex);

	if (ppSkillTemplate)
		return *ppSkillTemplate;

	return NULL;
}

AgpdSkillSpecializeTemplate* AgpmSkill::AddSpecialize(INT32 lSpecializeID)
{
	if (lSpecializeID == 0)
		return NULL;

	AgpdSkillSpecializeTemplate	*pcsSpecialize	= CreateSpecialize();
	if (!pcsSpecialize)
		return NULL;

	pcsSpecialize->m_lID	= lSpecializeID;

	if (!m_csAdminSpecializeTemplate.AddSkillSpecializeTemplate(pcsSpecialize))
	{
		DestroySpecialize(pcsSpecialize);
		return NULL;
	}

	if (pcsSpecialize->m_pcsGridItem)
	{
		pcsSpecialize->m_pcsGridItem->m_bMoveable	= TRUE;
		pcsSpecialize->m_pcsGridItem->m_eType		= AGPDGRID_ITEM_TYPE_SPECIALIZE;
		pcsSpecialize->m_pcsGridItem->m_lItemID		= lSpecializeID;
	}

	return pcsSpecialize;
}

AgpdSkillSpecializeTemplate* AgpmSkill::GetSpecialize(INT32 lSpecializeID)
{
	return m_csAdminSpecializeTemplate.GetSkillSpecializeTemplate(lSpecializeID);
}

BOOL AgpmSkill::RemoveSpecialize(INT32 lSpecializeID)
{
	AgpdSkillSpecializeTemplate *pcsSpecialize = GetSpecialize(lSpecializeID);
	if (!pcsSpecialize)
		return FALSE;

	m_csAdminSpecializeTemplate.RemoveSkillSpecializeTemplate(lSpecializeID);

	return DestroySpecialize(pcsSpecialize);
}

AgpdSkillTooltipTemplate* AgpmSkill::AddTooltipTemplate(CHAR *szSkillName)
{
	if (!szSkillName || !szSkillName[0])
		return NULL;

	AgpdSkillTooltipTemplate	*pcsTooltip	= CreateTooltipTemplate();
	if (!pcsTooltip)
		return NULL;

	strncpy(pcsTooltip->m_szSkillName, szSkillName, AGPMSKILL_MAX_SKILL_NAME);

	if (!m_csAdminTooltipTemplate.AddSkillTooltipTemplate(pcsTooltip))
	{
		DestroyTooltipTemplate(pcsTooltip);
		return NULL;
	}

	return pcsTooltip;
}

AgpdSkillTooltipTemplate* AgpmSkill::GetTooltipTemplate(CHAR *szSkillName)
{
	return m_csAdminTooltipTemplate.GetSkillTooltipTemplate(szSkillName);
}

BOOL AgpmSkill::RemoveTooltipTemplate(CHAR *szSkillName)
{
	AgpdSkillTooltipTemplate *pcsTooltip = GetTooltipTemplate(szSkillName);
	if (!pcsTooltip)
		return FALSE;

	m_csAdminTooltipTemplate.RemoveSkillTooltipTemplate(szSkillName);

	return DestroyTooltipTemplate(pcsTooltip);
}

INT16 AgpmSkill::AttachSkillData(PVOID pClass, INT32 nDataSize, ApModuleDefaultCallBack pfConstructor, ApModuleDefaultCallBack pfDestructor)
{
	return SetAttachedModuleData(pClass, AGPMSKILL_DATA_TYPE_SKILL, nDataSize, pfConstructor, pfDestructor);
}

INT16 AgpmSkill::AttachSkillTemplateData(PVOID pClass, INT32 nDataSize, ApModuleDefaultCallBack pfConstructor, ApModuleDefaultCallBack pfDestructor)
{
	return SetAttachedModuleData(pClass, AGPMSKILL_DATA_TYPE_TEMPLATE, nDataSize, pfConstructor, pfDestructor);
}

INT16 AgpmSkill::AttachSpecializeData(PVOID pClass, INT32 nDataSize, ApModuleDefaultCallBack pfConstructor, ApModuleDefaultCallBack pfDestructor)
{
	return SetAttachedModuleData(pClass, AGPMSKILL_DATA_TYPE_SPECIALIZE, nDataSize, pfConstructor, pfDestructor);
}

BOOL AgpmSkill::SetSkillStatus(AgpdSkill *pcsSkill, eAgpmSkillStatus eStatus)
{
	if (!pcsSkill)
		return FALSE;

	eAgpmSkillStatus	ePrevStatus = pcsSkill->m_eStatus;

	pcsSkill->m_eStatus = eStatus;

	EnumCallback(AGPMSKILL_CB_CHANGE_STATUS, pcsSkill, &ePrevStatus);

	return TRUE;
}

BOOL AgpmSkill::CBRemoveCharacter(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgpmSkill		*pThis			= (AgpmSkill *)		pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;

	//STOPWATCH2(pThis->GetModuleName(), _T("CBRemoveCharacter"));

	AgpdSkillAttachData	*pcsSkillAttachData = pThis->GetAttachSkillData((ApBase *) pcsCharacter);
	if (!pcsSkillAttachData)
		return FALSE;

	// pcsCharacter가 소유하고 있는 모든 스킬을 삭제한다.
	for (int i = 0; i < AGPMSKILL_MAX_SKILL_OWN; ++i)
	{
		if (pcsSkillAttachData->m_alSkillID[i] != AP_INVALID_SKILLID)
			pThis->RemoveSkill(pcsSkillAttachData->m_alSkillID[i]);
	}

	return TRUE;
}

BOOL AgpmSkill::CBUpdateSkillPoint(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgpmSkill		*pThis			= (AgpmSkill *)		pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *)	pData;
	INT32			lPoint			= *(INT32 *)		pCustData;

	if (lPoint < 0)
	{
		// 현재 마스터리에 갖고 있는 모든 스킬을 없애고
		// 마스터리에 넣은 SP들을 모두 빼낸다.
		// 정상적인 게임 플레이인경우 이런 조건은 발생하지 않는다.

		//if (!pThis->InitMastery((ApBase *) pcsCharacter))
		//	return FALSE;
	}

	return TRUE;
}

BOOL AgpmSkill::CBUpdateLevel(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	AgpmSkill			*pThis			= (AgpmSkill *)		pClass;
	AgpdCharacter		*pcsCharacter	= (AgpdCharacter *)	pData;

	// specialize를 할 수 있는 포인트인지 검사한다.
//	if (pThis->CalcSpecializeLevel((ApBase *) pcsCharacter))
//	{
//		// 새롭게 specialize를 할 수 있는 시간이다.
//		//
//		//
//		//
//	}

	return TRUE;
}

BOOL AgpmSkill::CBUpdateActionStatus(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass || !pCustData)
		return FALSE;

	AgpmSkill			*pThis			= (AgpmSkill *)		pClass;
	AgpdCharacter		*pcsCharacter	= (AgpdCharacter *)	pData;
	INT16				nOldStatus		= *(INT16 *)		pCustData;

	if ((AgpdCharActionStatus) nOldStatus == AGPDCHAR_STATUS_DEAD &&
		pcsCharacter->m_unActionStatus != AGPDCHAR_STATUS_DEAD)
	{
		// 모든 스킬의 Recast Delay를 초기화시킨다.
		AgpdSkillAttachData	*pcsAttachData	= pThis->GetAttachSkillData((ApBase *) pcsCharacter);

//		for (int i = 0; i < AGPMSKILL_MAX_SKILL_OWN; ++i)
//		{
//			AgpdSkill		*pcsSkill		= pThis->GetSkill(pcsAttachData->m_alSkillID[i]);
//			if (!pcsSkill)
//				continue;
//
//			pcsSkill->m_ulStartTime			= 0;
//			pcsSkill->m_ulEndTime			= 0;
//			pcsSkill->m_ulRecastDelay		= 0;
//		}

		for (int i = 0; i < AGPMSKILL_MAX_SKILL_OWN; ++i)
		{
			AgpdSkill		*pcsSkill		= pcsAttachData->m_apcsSkill[i];
			if (!pcsSkill)
				break;

			pcsSkill->m_ulStartTime			= 0;
			pcsSkill->m_ulEndTime			= 0;
			pcsSkill->m_ulCastDelay			= 0;
			pcsSkill->m_ulRecastDelay		= 0;
		}
	}

	return TRUE;
}

BOOL AgpmSkill::CBInitCharacter(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if (!pData || !pClass)
		return FALSE;

	return TRUE;
}

// 2008.06.23. steeple
BOOL AgpmSkill::CBGetPetTIDByItemFromSkill(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass || !pCustData)
		return FALSE;

	AgpmSkill* pThis = static_cast<AgpmSkill*>(pClass);
	AgpdItemTemplate* pcsItemTemplate = static_cast<AgpdItemTemplate*>(pData);
	INT32* plPetTID = static_cast<INT32*>(pCustData);

	*plPetTID = pThis->GetSummonsTIDByItem(pcsItemTemplate);

	return TRUE;
}

ApBase* AgpmSkill::GetBaseFromPacket(PVOID pvBasePacket)
{
	if (!pvBasePacket)
		return NULL;

	INT8	cType	= (-1);
	INT32	lID		= AP_INVALID_CID;

	m_csPacketBase.GetField(FALSE, pvBasePacket, 0, &cType, &lID);

	return GetBase((ApBaseType) cType, lID);
}

ApBase* AgpmSkill::GetBase(INT8 cType, INT32 lID)
{
	switch (cType)
	{
	case APBASE_TYPE_OBJECT:
		return m_papmObject ? m_papmObject->GetObject(lID) : NULL;

	case APBASE_TYPE_OBJECT_TEMPLATE:
		return m_papmObject ? m_papmObject->GetObjectTemplate(lID) : NULL;

	case APBASE_TYPE_CHARACTER:
		return m_pagpmCharacter ? m_pagpmCharacter->GetCharacter(lID) : NULL;

	case APBASE_TYPE_CHARACTER_TEMPLATE:
		return m_pagpmCharacter ? m_pagpmCharacter->GetCharacterTemplate(lID) : NULL;

	case APBASE_TYPE_ITEM:
		return m_pagpmItem ? m_pagpmItem->GetItem(lID) : NULL;

	case APBASE_TYPE_ITEM_TEMPLATE:
		return m_pagpmItem ? m_pagpmItem->GetItemTemplate(lID) : NULL;

	case APBASE_TYPE_SKILL:
		return GetSkill(lID);
	}

	return NULL;
}

ApBase* AgpmSkill::GetBaseLock(INT8 cType, INT32 lID)
{
	switch (cType)
	{
	case APBASE_TYPE_CHARACTER:
		return m_pagpmCharacter ? m_pagpmCharacter->GetCharacterLock(lID) : NULL;
	}

	return NULL;
}

BOOL AgpmSkill::OnReceive(UINT32 ulType, PVOID pvPacket, INT16 nSize, UINT32 ulNID, DispatchArg *pstCheckArg)
{
	PROFILE("AgpmSkill::OnReceive");

	if (!pvPacket || nSize == 0)
		return FALSE;

	INT8	cOperation			= (-1);
	INT32	lSkillID			= AP_INVALID_SKILLID;
	INT32	lTID				= AP_INVALID_SKILLID;
	INT8	cStatus				= (-1);
	UINT8	lSkillPoint			= 0;

	PVOID	pvPacketBase		= NULL;
	PVOID	pvPacketFactor		= NULL;
	PVOID	pvPacketAction		= NULL;
//	PVOID	pvPacketMastery		= NULL;

//	INT8	lMasteryIndex		= (-1);

	UINT32	ulDurationTimeMSec		= 0;
	PVOID	pvBuffedSkillCombatArg	= NULL;
	INT		nBuffedLength			=	0;

//	INT32	lAllBuffedSkillTID[AGPMSKILL_MAX_SKILL_BUFF];
//	INT32	lAllBuffedSkillOwnerID[AGPMSKILL_MAX_SKILL_BUFF];

//	UINT8	cResultAddSP		= 0;
//	INT32	lCasterTID			= 0;

//	PVOID	*pvSelectSkillTID	= NULL;
//	INT16	nSelectSkillTIDSize	= 0;

//	ZeroMemory(lAllBuffedSkillTID, sizeof(INT32) * AGPMSKILL_MAX_SKILL_BUFF);
//	ZeroMemory(lAllBuffedSkillOwnerID, sizeof(INT32) * AGPMSKILL_MAX_SKILL_BUFF);

//	PVOID	ppvPacketBuff[AGPMSKILL_MAX_SKILL_BUFF];
//	ZeroMemory(ppvPacketBuff, sizeof(PVOID) * AGPMSKILL_MAX_SKILL_BUFF);

	ApSafeArray<PVOID, AGPMSKILL_MAX_SKILL_BUFF>	ppvPacketBuff;
	ppvPacketBuff.MemSetAll();

	m_csPacket.GetField(TRUE, pvPacket, nSize,
						&cOperation,
						&lSkillID,
						&pvPacketBase,
						&pvPacketFactor,
						&lTID,
						&cStatus,
						&pvPacketAction,
						&lSkillPoint,
//						&pvPacketMastery,
//						&lMasteryIndex,
//						&cResultAddSP,
//						&pvSelectSkillTID, &nSelectSkillTIDSize,
//						&ulDurationTimeMSec,
//						&lCasterTID,
//						&lAllBuffedSkillTID,
//						&lAllBuffedSkillOwnerID);
						&ppvPacketBuff[0],
						&pvBuffedSkillCombatArg,
						&nBuffedLength );

	//TRACE("Skill Packet : %d,%d,%d,%d\n", lSkillID, lTID, cStatus, cOperation);

//	if (nSelectSkillTIDSize != 0)
//		nSelectSkillTIDSize	= nSelectSkillTIDSize / sizeof(INT32);

//	if (!m_csAdminSkill.GlobalWLock())
//		return FALSE;

	ApBase *pcsBase = GetBaseFromPacket(pvPacketBase);
	if (!pcsBase || !pcsBase->m_Mutex.WLock())
	{
		//		m_csAdminSkill.GlobalRelease();
		return FALSE;
	}

	AgpdSkillBuffedCombatEffectArg*	pSkillBuffed			=	static_cast< AgpdSkillBuffedCombatEffectArg* >(pvBuffedSkillCombatArg);
	AgpdSkillAttachData*			pSkillAttachData		=	GetAttachSkillData( pcsBase );


	if( pSkillAttachData && pSkillBuffed )
	{
		pSkillAttachData->m_stBuffedSkillCombatEffectArg	=	*pSkillBuffed;
	}
	
//	if (!m_csAdminSkill.GlobalRelease())
//	{
//		pcsBase->m_Mutex.Release();
//		return FALSE;
//	}

	if (pcsBase->m_eType == APBASE_TYPE_CHARACTER)
	{
		if (!pstCheckArg->bReceivedFromServer &&
			pstCheckArg->lSocketOwnerID != AP_INVALID_CID &&
			pstCheckArg->lSocketOwnerID != pcsBase->m_lID)
		{
			pcsBase->m_Mutex.Release();
			return FALSE;
		}

		if (!pstCheckArg->bReceivedFromServer && m_pagpmCharacter->IsAllBlockStatus((AgpdCharacter *) pcsBase))
		{
			pcsBase->m_Mutex.Release();
			return FALSE;
		}
	}

	BOOL	bRetval = FALSE;

	if (cOperation < (INT8) AGPMSKILL_PACKET_OPERATION_SEPARATOR)
	{
		bRetval = OnReceiveSkill(pstCheckArg->bReceivedFromServer, cOperation, lSkillID, pcsBase, pvPacketFactor, lTID, cStatus, pvPacketAction, lSkillPoint, /*lMasteryIndex, ulDurationTimeMSec, lAllBuffedSkillTID, lAllBuffedSkillOwnerID, lCasterTID*/
								 &ppvPacketBuff[0]);
	}
	else if (cOperation > (INT8) AGPMSKILL_PACKET_OPERATION_SEPARATOR)
	{
//		bRetval = OnReceiveSkillMastery(bReceivedFromServer, cOperation, pcsBase, lMasteryIndex, pvPacketMastery, cResultAddSP, pvSelectSkillTID, nSelectSkillTIDSize);
	}
	else
		bRetval = FALSE;

	pcsBase->m_Mutex.Release();

	return bRetval;
}

//BOOL AgpmSkill::OnReceiveSkill(BOOL bReceivedFromServer, INT8 cOperation, INT32 lSkillID, ApBase *pcsBase, PVOID pvPacketFactor, 
//							   INT32 lTID, INT8 cStatus, PVOID pvPacketAction, INT32 lSkillPoint,
//							   INT32 lMasteryIndex,
//							   UINT32 ulDurationTimeMSec,
//							   INT32 *plAllBuffedSkillTID, INT32 *plAllBuffedSkillOwnerID, INT32 lCasterTID)
BOOL AgpmSkill::OnReceiveSkill(BOOL bReceivedFromServer, INT8 cOperation, INT32 lSkillID, ApBase *pcsBase, PVOID pvPacketFactor, 
							   INT32 lTID, INT8 cStatus, PVOID pvPacketAction, INT32 lSkillPoint,
							   PVOID *ppvPacketBuff)
{
	AgpdSkillTemplate*	pcsSkillTemplate				=	GetSkillTemplate(lTID);

	switch (cOperation) {
	case AGPMSKILL_PACKET_OPERATION_ADD:
		{
			if (!bReceivedFromServer)
				return FALSE;

			if (lSkillID == AP_INVALID_SKILLID || lTID == AP_INVALID_SKILLID)
				return FALSE;

			AgpdSkill *pcsSkill = AddSkill(lSkillID, lTID);
			if (!pcsSkill)
			{
				pcsSkill	= GetSkill(lSkillID);
				if (pcsSkill)
				{
					return OnReceiveSkill(bReceivedFromServer, AGPMSKILL_PACKET_OPERATION_UPDATE, lSkillID, pcsBase, pvPacketFactor, 
									lTID, cStatus, pvPacketAction, lSkillPoint,
									ppvPacketBuff);
//					return OnReceiveSkill(bReceivedFromServer, AGPMSKILL_PACKET_OPERATION_UPDATE, lSkillID, pcsBase, pvPacketFactor, 
//									lTID, cStatus, pvPacketAction, lSkillPoint,
//									lMasteryIndex,
//									ulDurationTimeMSec, plAllBuffedSkillTID, plAllBuffedSkillOwnerID, lCasterTID);
				}

				return FALSE;
			}

			pcsSkill->m_lID = lSkillID;
			pcsSkill->m_pcsBase = pcsBase;

			if (m_pagpmFactors)
				m_pagpmFactors->SetOwnerFactor(&pcsSkill->m_csFactor, pcsBase->m_lID, pcsBase);

			if (cStatus != (-1))
				pcsSkill->m_eStatus = (eAgpmSkillStatus) cStatus;

			if (pvPacketFactor && m_pagpmFactors)
				m_pagpmFactors->ReflectPacket(&pcsSkill->m_csFactor, pvPacketFactor, 0);

//			if (lMasteryIndex != (-1))
//				pcsSkill->m_lMasteryIndex = lMasteryIndex;

			/*
			if (pvPacketAction)
				// action packet이 왔다. 처리한다. (여기선 처리 안하고.. callback function을 호출한다.)
				EnumCallback(AGPMSKILL_CB_RECEIVE_ACTION, pcsSkill, pvPacketAction);
			*/

			AddOwnSkillList(pcsSkill);
			InitSkill( pcsSkill );
		}
		break;

	case AGPMSKILL_PACKET_OPERATION_REMOVE:
		{
			if (!bReceivedFromServer)
				return FALSE;

			AgpdSkill *pcsSkill = GetSkillLock(lSkillID);
			if (!pcsSkill)
				return FALSE;

			RemoveOwnSkillList(pcsSkill);

			pcsSkill->m_Mutex.Release();

			RemoveSkill(lSkillID);
		}
		break;

	case AGPMSKILL_PACKET_OPERATION_UPDATE:
		{
			if (!bReceivedFromServer)
				return FALSE;

			AgpdSkill *pcsSkill = GetSkill(lSkillID);
			if (!pcsSkill)
				return FALSE;

			if (pcsBase)
				pcsSkill->m_pcsBase = pcsBase;

			if (cStatus != (-1))
				pcsSkill->m_eStatus = (eAgpmSkillStatus) cStatus;

			if (pvPacketFactor && m_pagpmFactors)
				m_pagpmFactors->ReflectPacket(&pcsSkill->m_csFactor, pvPacketFactor, 0);

//			if (lMasteryIndex != (-1))
//				pcsSkill->m_lMasteryIndex = lMasteryIndex;

			/*
			if (pvPacketAction)
				// action packet이 왔다. 처리한다. (여기선 처리 안하고.. callback function을 호출한다.)
				EnumCallback(AGPMSKILL_CB_RECEIVE_ACTION, pcsSkill, pvPacketAction);
			*/

			//pcsSkill->m_Mutex.Release();

			EnumCallback(AGPMSKILL_CB_UPDATE_SKILL, pcsSkill, NULL);
		}
		break;

	case AGPMSKILL_PACKET_OPERATION_ADD_BUFFED_LIST:
		{
			if (!bReceivedFromServer ||
				!ppvPacketBuff ||
				!ppvPacketBuff[0])
				return FALSE;

			INT32		lBuffedSkillTID					=	AP_INVALID_SKILLID;
			UINT32		ulDurationTimeMSec				=	0;
			INT32		lCasterTID						=	AP_INVALID_CID;
			UINT8		ucChargeLevel					=	0;
			UINT32		ulExpireTime					=	0;
			PVOID		pvSkillBuffedCombatEffectArg	=	NULL;
			INT			nBuffedLength					=	0;
			
			m_csPacketBuff.GetField(FALSE, ppvPacketBuff[0], 0,
									&lBuffedSkillTID,
									&ulDurationTimeMSec,
									&lCasterTID,
									&ucChargeLevel,
									&ulExpireTime,
									&pvSkillBuffedCombatEffectArg,
									&nBuffedLength
									);

			AgpdSkillBuffedCombatEffectArg*		pSkillBuffed	=	static_cast<AgpdSkillBuffedCombatEffectArg*>(pvSkillBuffedCombatEffectArg);
			if( pSkillBuffed )
			{
				AgpdSkillAttachData*	pAttachData	=	GetAttachSkillData( pcsBase );
				if( pAttachData )
				{
					pAttachData->m_stBuffedSkillCombatEffectArg	=	*pSkillBuffed;
				}
			}

			OnReceiveAddBuffedList(pcsBase, lBuffedSkillTID, ulDurationTimeMSec, lCasterTID, ucChargeLevel , ulExpireTime );
		}
		break;

	case AGPMSKILL_PACKET_OPERATION_REMOVE_BUFFED_LIST:
		{
			if (!bReceivedFromServer ||
				!ppvPacketBuff ||
				!ppvPacketBuff[0])
				return FALSE;

			AgpdSkillAttachData *pcsAttachData = GetAttachSkillData(pcsBase);
			if (!pcsAttachData)
				return FALSE;

			INT32		lBuffedSkillTID					= AP_INVALID_SKILLID;
			PVOID		pvSkillBuffedCombatEffectArg	=	NULL;
			INT			nBuffedLength					=	0;

			m_csPacketBuff.GetField(FALSE, ppvPacketBuff[0], 0,
									&lBuffedSkillTID,
									NULL,
									NULL,
									NULL,
									NULL,
									&pvSkillBuffedCombatEffectArg,
									&nBuffedLength);

			AgpdSkillBuffedCombatEffectArg*		pSkillBuffed	=	static_cast<AgpdSkillBuffedCombatEffectArg*>(pvSkillBuffedCombatEffectArg);
			if( pSkillBuffed )
			{
				AgpdSkillAttachData*	pAttachData	=	GetAttachSkillData( pcsBase );
				if( pAttachData )
				{
					pAttachData->m_stBuffedSkillCombatEffectArg	=	*pSkillBuffed;
				}
			}

			if (RemoveBuffedList(&pcsAttachData->m_astBuffSkillList[0], lBuffedSkillTID) >= 0)
				EnumCallback(AGPMSKILL_CB_REMOVE_BUFF, (PVOID)pcsBase, IntToPtr(lBuffedSkillTID) );
		}
		break;

	case AGPMSKILL_PACKET_OPERATION_ALL_BUFFED_SKILL:
		{
			if (!bReceivedFromServer ||
				!ppvPacketBuff ||
				!ppvPacketBuff[0])
				return FALSE;

			INT32	lBuffedSkillTID;
			UINT32	ulDurationTimeMSec;
			INT32	lCasterTID;
			UINT8	ucChargeLevel;
			UINT32	ulExpiredTime;

			for (int i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
			{
				if (!ppvPacketBuff[i])
					break;

				lBuffedSkillTID		= AP_INVALID_SKILLID;
				ulDurationTimeMSec	= 0;
				lCasterTID			= AP_INVALID_CID;
				ucChargeLevel		= 0;
				ulExpiredTime		= 0;
				

				m_csPacketBuff.GetField(FALSE, ppvPacketBuff[i], 0,
										&lBuffedSkillTID,
										&ulDurationTimeMSec,
										&lCasterTID,
										&ucChargeLevel,
										&ulExpiredTime,
										NULL);

				OnReceiveAddBuffedList(pcsBase, lBuffedSkillTID, ulDurationTimeMSec, lCasterTID, ucChargeLevel , ulExpiredTime );
			}
		}
		break;

	case AGPMSKILL_PACKET_OPERATION_CAST_SKILL:
		{
			if (!pvPacketAction)
				return FALSE;

			INT8	cActionType				= (-1);
			PVOID	pvTargetBase			= NULL;
			PVOID	pvTargetResultFactor	= NULL;
			AuPOS	posTarget;
			INT8	cForceAttack			= 0;
			CHAR*	szTargetCharID			= NULL;
			UINT16	nNumTargetCharID		= 0;
			UINT32	ulCastDelay				= 0;
			UINT32	ulDuration				= 0;
			UINT32	ulRecastDelay			= 0;
			UINT8	cSkillLevel				= 0;
			UINT8	cIsFactorNotQueueing	= 0;
			UINT32	ulAdditionalEffect		= 0;
			
			m_csPacketAction.GetField(FALSE, pvPacketAction, 0,
										&cActionType,
										&pvTargetBase,
										&pvTargetResultFactor,
										&posTarget,
										&cForceAttack,
										&ulCastDelay,
										&ulDuration,
										&ulRecastDelay,
										&cSkillLevel,
										&cIsFactorNotQueueing,
										&ulAdditionalEffect,
										&szTargetCharID, &nNumTargetCharID
										);

			ApBase *pcsOwnerBase = pcsBase;
			if (!pcsOwnerBase)
				return FALSE;

			if (pcsOwnerBase->m_eType == APBASE_TYPE_CHARACTER && 
				(!m_pagpmCharacter->CheckActionStatus((AgpdCharacter *) pcsOwnerBase, AGPDCHAR_STATUS_READY_SKILL) ||
				(!m_pagpmCharacter->IsArchlord((AgpdCharacter*)pcsOwnerBase) && ((AgpdCharacter *)pcsOwnerBase)->m_bIsTrasform)// ||
				//(!IsRideSkill(pcsSkillTemplate) && ((AgpdCharacter *)pcsOwnerBase)->m_bRidable))
				))
				return FALSE;

			INT32	alCID[2];
			ZeroMemory(alCID, sizeof(INT32) * 2);

			INT32	lNumCharacter	= 1;

			alCID[0]	= pcsOwnerBase->m_lID;

			if (pvTargetBase)
			{
				INT8	cType	= (-1);
				INT32	lID		= AP_INVALID_CID;

				m_csPacketBase.GetField(FALSE, pvTargetBase, 0, &cType, &lID);

				if (lID != AP_INVALID_CID &&
					pcsOwnerBase->m_lID != lID)
				{
					alCID[1]		= lID;
					lNumCharacter	= 2;
				}
			}

			pcsOwnerBase->m_Mutex.Release();

			AgpdCharacter	*pacsCharacter[2];
			ZeroMemory(pacsCharacter, sizeof(AgpdCharacter *) * 2);

			ApBase	*pcsTargetBase	= NULL;

			if (!m_pagpmCharacter->GetCharacterLock(alCID, lNumCharacter, pacsCharacter))
			{
				pcsOwnerBase->m_Mutex.WLock();
				return FALSE;
			}

			if (pacsCharacter[0] == pcsOwnerBase)
			{
				pcsOwnerBase	= pacsCharacter[0];
				pcsTargetBase	= pacsCharacter[1];
			}
			else
			{
				pcsOwnerBase	= pacsCharacter[1];
				pcsTargetBase	= pacsCharacter[0];
			}


			stAgpmSkillActionData	stActionData;

			if (pcsTargetBase)
			{
				
				stActionData.lTargetUnion	= m_pagpmCharacter->GetUnion((AgpdCharacter*) pcsTargetBase);
				
				stActionData.csTargetPos	= *(m_papmEventManager->GetBasePos(pcsTargetBase, NULL));

				stActionData.csTargetBase[0].m_eType	= pcsTargetBase->m_eType;
				stActionData.csTargetBase[0].m_lID		= pcsTargetBase->m_lID;
				stActionData.lTargetHitRange			= 0;

				if (pcsTargetBase->m_eType == APBASE_TYPE_CHARACTER)
				{
					if (!bReceivedFromServer && m_pagpmCharacter->IsAllBlockStatus((AgpdCharacter *) pcsTargetBase))
					{
						pcsTargetBase->m_Mutex.Release();
						return FALSE;
					}

					if (m_pagpmFactors)
					{
						m_pagpmFactors->GetValue(&((AgpdCharacter *) pcsTargetBase)->m_csFactor, &stActionData.lTargetHitRange, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_HITRANGE);
					}
				}

				pcsTargetBase->m_Mutex.Release();
			}
			else
				stActionData.csTargetPos	= posTarget;

			// 2005.03.28. steeple
			// 스킬도 강제 공격 개념을 넣었다.
			//
			// 역시 여기도 PK를 막는다.
			//cForceAttack	= 0;

			stActionData.nActionType	= cActionType;
			stActionData.lSkillID		= lSkillID;
			stActionData.lSkillTID		= lTID;
			stActionData.lSkillLevel	= (INT32) cSkillLevel;
			stActionData.pcsOwnerBase	= pcsOwnerBase;
			stActionData.bForceAttack	= (BOOL) cForceAttack;
			stActionData.ulCastDelay	= ulCastDelay;
			stActionData.ulDuration		= ulDuration;
			stActionData.ulRecastDelay	= ulRecastDelay;
			stActionData.ulAdditionalEffect	= ulAdditionalEffect;

			if (szTargetCharID && nNumTargetCharID / sizeof(INT32) > 0)
			{
				INT32	*plTargetCharID = (INT32 *) szTargetCharID;
				int j = 0;
				for (unsigned int i = 0; i < nNumTargetCharID / sizeof(INT32); ++i)
				{
					if (plTargetCharID[i])
					{
						stActionData.csTargetBase[j].m_eType	= APBASE_TYPE_CHARACTER;
						stActionData.csTargetBase[j].m_lID		= plTargetCharID[i];

						++j;
					}
				}
			}

			/*
			if (IsAttackSkill(lTID))
				m_pagpmCharacter->SetCombatMode((AgpdCharacter *) pcsOwnerBase, GetClockCount());
			*/

			EnumCallback(AGPMSKILL_CB_RECEIVE_ACTION, &stActionData, NULL);
		}
		break;

	case AGPMSKILL_PACKET_OPERATION_CAST_SKILL_RESULT:
		{
			if (!bReceivedFromServer)
				return FALSE;

			if (!pvPacketAction)
				return FALSE;

			INT8	cActionType = (-1);
			PVOID	pvTargetBase = NULL;
			PVOID	pvTargetResultFactor	= NULL;
			AuPOS	posTarget;
			INT8	cForceAttack = 0;
			CHAR*	szTargetCharID = NULL;
			UINT16	nNumTargetCharID	= 0;
			UINT32	ulCastDelay	= 0;
			UINT32	ulDuration = 0;
			UINT32	ulRecastDelay = 0;
			UINT8	cSkillLevel	= 0;
			UINT8	cIsFactorNotQueueing = 0;
			UINT32	ulAdditionalEffect = 0;
			
			m_csPacketAction.GetField(FALSE, pvPacketAction, 0,
										&cActionType,
										&pvTargetBase,
										&pvTargetResultFactor,
										&posTarget,
										&cForceAttack,
										&ulCastDelay,
										&ulDuration,
										&ulRecastDelay,
										&cSkillLevel,
										&cIsFactorNotQueueing,
										&ulAdditionalEffect,
										&szTargetCharID, &nNumTargetCharID
										);

			ApBase *pcsOwnerBase = pcsBase;
			if (!pcsOwnerBase)
				return FALSE;

			/*
			if (pcsOwnerBase->m_eType == APBASE_TYPE_CHARACTER && 
				!m_pagpmCharacter->CheckActionStatus((AgpdCharacter *) pcsOwnerBase, AGPDCHAR_STATUS_READY_SKILL))
				return FALSE;
			*/

			INT32	alCID[2];
			ZeroMemory(alCID, sizeof(INT32) * 2);

			INT32	lNumCharacter	= 1;

			alCID[0]	= pcsOwnerBase->m_lID;

			if (pvTargetBase)
			{
				INT8	cType	= (-1);
				INT32	lID		= AP_INVALID_CID;

				m_csPacketBase.GetField(FALSE, pvTargetBase, 0, &cType, &lID);

				if (lID != AP_INVALID_CID &&
					pcsOwnerBase->m_lID != lID)
				{
					alCID[1]		= lID;
					lNumCharacter	= 2;
				}
			}

			pcsOwnerBase->m_Mutex.Release();

			AgpdCharacter	*pacsCharacter[2];
			ZeroMemory(pacsCharacter, sizeof(AgpdCharacter *) * 2);

			ApBase	*pcsTargetBase	= NULL;

			if (!m_pagpmCharacter->GetCharacterLock(alCID, lNumCharacter, pacsCharacter))
			{
				pcsOwnerBase->m_Mutex.WLock();
				return FALSE;
			}

			if (pacsCharacter[0] == pcsOwnerBase)
			{
				pcsOwnerBase	= pacsCharacter[0];
				pcsTargetBase	= pacsCharacter[1];
			}
			else
			{
				pcsOwnerBase	= pacsCharacter[1];
				pcsTargetBase	= pacsCharacter[0];
			}


			stAgpmSkillActionData	stActionData;

			BOOL	bIsNowUpdate	= TRUE;
			EnumCallback(AGPMSKILL_CB_CHECK_NOW_UPDATE_ACTION_FACTOR, pcsTargetBase, &bIsNowUpdate);

			AgpdFactor	csResultFactor;
			if (cIsFactorNotQueueing || bIsNowUpdate && pvTargetResultFactor)
			{
				if (m_pagpmFactors)
					m_pagpmFactors->InitFactor(&csResultFactor);

				m_pagpmCharacter->ReflectFactorPacket((AgpdCharacter *) pcsTargetBase, pvTargetResultFactor, &csResultFactor);
			}
			else
			{
				stActionData.pvPacketFactor	= pvTargetResultFactor;
			}

			if (pcsTargetBase)
			{
				stActionData.csTargetBase[0].m_eType	= pcsTargetBase->m_eType;
				stActionData.csTargetBase[0].m_lID		= pcsTargetBase->m_lID;

				pcsTargetBase->m_Mutex.Release();
			}

			stActionData.nActionType	= cActionType;
			stActionData.lSkillID		= lSkillID;
			stActionData.lSkillTID		= lTID;
			stActionData.lSkillLevel	= (INT32) cSkillLevel;
			stActionData.pcsOwnerBase	= pcsOwnerBase;
			stActionData.bForceAttack	= (BOOL) cForceAttack;
			stActionData.pcsResultFactor	= &csResultFactor;
			stActionData.bIsFactorNotQueueing	= (BOOL) cIsFactorNotQueueing;
			stActionData.bIsNowUpdate	= bIsNowUpdate;				// 2005.03.14. steeple
			stActionData.ulAdditionalEffect = ulAdditionalEffect;	// 2005.12.15. steeple

			if (szTargetCharID && nNumTargetCharID / sizeof(INT32) > 0)
			{
				INT32	*plTargetCharID = (INT32 *) szTargetCharID;
				int j = 0;
				for (unsigned int i = 0; i < nNumTargetCharID / sizeof(INT32); ++i)
				{
					if (plTargetCharID[i])
					{
						stActionData.csTargetBase[j].m_eType	= APBASE_TYPE_CHARACTER;
						stActionData.csTargetBase[j].m_lID		= plTargetCharID[i];

						++j;
					}
				}
			}

			EnumCallback(AGPMSKILL_CB_RECEIVE_ACTION_RESULT, &stActionData, NULL);

			if (m_pagpmFactors)
				m_pagpmFactors->DestroyFactor(&csResultFactor);
		}
		break;
		
	case AGPMSKILL_PACKET_OPERATION_REAL_HIT:
		{
			// 2004.11.09. steeple
			// Twice Packet Skill 사용시 RealHit 지점에서 Client 에서 Server 로 보내진다.
			// Target 은 서버에서 저장해놓고 있기 때문에, 날아온 패킷에는 Target 정보가 없다.
			// 고로 Target 은 신경쓰지 않아도 된다.
			
			ApBase* pcsOwnerBase = pcsBase;
			if(!pcsOwnerBase)
				return FALSE;

			if(pcsOwnerBase->m_eType == APBASE_TYPE_CHARACTER && 
				(!m_pagpmCharacter->CheckActionStatus((AgpdCharacter*)pcsOwnerBase, AGPDCHAR_STATUS_READY_SKILL) ||
				(!m_pagpmCharacter->IsArchlord((AgpdCharacter*)pcsOwnerBase) && ((AgpdCharacter *)pcsOwnerBase)->m_bIsTrasform)// ||
				//(!IsRideSkill(pcsSkillTemplate) && ((AgpdCharacter *)pcsOwnerBase)->m_bRidable))
				))
				return FALSE;

			EnumCallback(AGPMSKILL_CB_RECEIVE_REAL_HIT, pcsOwnerBase, &lSkillID);
		}
		break;

	case AGPMSKILL_PACKET_OPERATION_UPDATE_DIRT_POINT:
		{
			if (!bReceivedFromServer)
				return FALSE;

			if (!pvPacketFactor)
				return FALSE;

			AgpdFactor	csUpdateFactor;

			m_pagpmFactors->InitFactor(&csUpdateFactor);

			if (!m_pagpmFactors->ReflectPacket(&csUpdateFactor, pvPacketFactor, 0))
			{
				m_pagpmFactors->DestroyFactor(&csUpdateFactor);
				return FALSE;
			}

			AgpdSkill *pcsSkill = GetSkillLock(lSkillID);
			if (!pcsSkill)
			{
				m_pagpmFactors->DestroyFactor(&csUpdateFactor);
				return FALSE;
			}

			UpdateDIRT(pcsSkill, &csUpdateFactor);

			pcsSkill->m_Mutex.Release();

			m_pagpmFactors->DestroyFactor(&csUpdateFactor);
		}
		break;

	case AGPMSKILL_PACKET_OPERATION_UPDATE_SKILL_POINT:
		{
			if (!bReceivedFromServer)
				return FALSE;

			if (lSkillPoint == 0)
				return FALSE;

			AgpdSkill *pcsSkill = GetSkillLock(lSkillID);
			if (!pcsSkill)
				return FALSE;

			if (lSkillPoint > 0)
				AddSkillPoint(pcsSkill, lSkillPoint);
			else if (lSkillPoint < 0)
				SubtractSkillPoint(pcsSkill, abs(lSkillPoint));

			pcsSkill->m_Mutex.Release();
		}
		break;

	case AGPMSKILL_PACKET_OPERATION_ADDITIONAL_EFFECT:
		{
			// 2005.07.05. steeple
			// 문엘프 스킬 작업 하다 보니깐, 어떤 상황에서 스킬 이펙트를 보여주고 싶은데
			// 모델툴에서 세팅할 수 없는 상황이 너무나도 많아서 이렇게 따로 스킬 이펙트를 보내는 것을 만듬.
			if(!bReceivedFromServer)
				return FALSE;

			// Base 는 이펙트가 터질 대상이다.
			if(!pcsBase || pcsBase->m_eType != APBASE_TYPE_CHARACTER)
				return FALSE;

			// 스킬 아이디에 Additional Effect ID 가 온다.
			INT32 lEffectID = lSkillID;

			// 스킬 TID 에 TargetCID 가 온다.
			INT32 lTargetCID = lTID;

			PVOID pvBuffer[2];
			pvBuffer[0] = &lEffectID;
			pvBuffer[1] = &lTID;
			EnumCallback(AGPMSKILL_CB_RECEIVE_ADDITIONAL_EFFECT, pcsBase, pvBuffer);
		}
		break;

	case AGPMSKILL_PACKET_OPERATION_REQUEST_SKILL_ROLLBACK:
		{
			if(!pcsBase || pcsBase->m_eType != APBASE_TYPE_CHARACTER ||
				lSkillID == AP_INVALID_SKILLID)
				return FALSE;

			/*
			if (!m_pagpmBillInfo->IsPCBang((AgpdCharacter *) pcsBase))
				return FALSE;
			*/

			AgpdSkill	*pcsSkill			= GetSkill(lSkillID);
			if (!pcsSkill)
				return FALSE;

			AgpdItem	*pcsRollbackScroll	= m_pagpmItem->GetSkillRollbackScroll((AgpdCharacter *) pcsBase);
			if (!pcsRollbackScroll)
				pcsRollbackScroll	= m_pagpmItem->GetCashSkillRollbackScroll((AgpdCharacter *) pcsBase);

			if (!pcsRollbackScroll)
				return FALSE;

			return EnumCallback(AGPMSKILL_CB_ROLLBACK_SKILL, pcsSkill, pcsRollbackScroll);
		}
		break;

		// 2007.02.06. steeple
	case AGPMSKILL_PACKET_OPERATION_MODIFIED_SKILL_LEVEL:
		{
			if(!pcsBase || pcsBase->m_eType != APBASE_TYPE_CHARACTER)
				return FALSE;

			INT32 lModifiedSkillLevel = lSkillID;
			if(lModifiedSkillLevel >= AGPMSKILL_MAX_SKILL_CAP)
				return FALSE;

			AgpdSkillAttachData* pcsAttachData = GetAttachSkillData(pcsBase);
			if(!pcsBase)
				return FALSE;

			// 그냥 대입한다.
			pcsAttachData->m_lModifiedSkillLevel = lModifiedSkillLevel;
			return EnumCallback(AGPMSKILL_CB_MODIFIED_SKILL_LEVEL, pcsBase, NULL);
		}
		break;

		// 2007.07.20. steeple
	case AGPMSKILL_PACKET_OPERATION_INIT_COOLTIME:
		{
			if(!pcsBase || pcsBase->m_eType != APBASE_TYPE_CHARACTER)
				return FALSE;
			
			AgpdSkill* pcsSkill = GetSkillByTID((AgpdCharacter*)pcsBase, lTID);
			if(!pcsSkill)
				break;

			InitSkillCoolTime(pcsSkill);
			return EnumCallback(AGPMSKILL_CB_INIT_COOLTIME, pcsBase, &lTID);
		}
		break;

		// 2007.11.14. steeple
	case AGPMSKILL_PACKET_OPERATION_SPECIFIC_SKILL_LEVEL:
		{
			if(!pcsBase || pcsBase->m_eType != APBASE_TYPE_CHARACTER || lSkillID == AP_INVALID_SKILLID)
				return FALSE;

			AgpdSkill* pcsSkill = GetSkill(lSkillID);
			if(!pcsSkill)
				return FALSE;

			INT32 lModifiedSkillLevel = lTID;

			// 그냥 대입한다.
			pcsSkill->m_lModifiedLevel = lModifiedSkillLevel;
			return EnumCallback(AGPMSKILL_CB_SPECIFIC_SKILL_LEVEL, pcsSkill, pcsBase);
		}
		break;

	default:
		return FALSE;
		break;
	}

	return TRUE;
}

BOOL AgpmSkill::OnReceiveAddBuffedList(ApBase *pcsBase, INT32 lBuffedSkillTID, UINT32 ulDurationMSec, INT32 lCasterID, INT32 ucChargeLevel , UINT32 ulExpireTime )
{
	if (!pcsBase)
		return FALSE;

	AgpdSkillAttachData *pcsAttachData = GetAttachSkillData(pcsBase);
	if (!pcsAttachData)
		return FALSE;

	INT32	lBuffedIndex	= AddBuffedList(&pcsAttachData->m_astBuffSkillList[0], lBuffedSkillTID, AP_INVALID_SKILLID,
											APBASE_TYPE_NONE, 0, lCasterID, 0, ulDurationMSec, ucChargeLevel , ulExpireTime );

	if (lBuffedIndex >= 0)
	{
		PVOID	pvBuffer[3];
		pvBuffer[0]	= IntToPtr(lBuffedSkillTID);
		pvBuffer[1]	= IntToPtr(lCasterID);
		pvBuffer[2]	= IntToPtr(pcsAttachData->m_astBuffSkillList[lBuffedIndex].bIsOnlyUpdate);

		EnumCallback( AGPMSKILL_CB_ADD_BUFF, (PVOID)pcsBase, pvBuffer );

		pcsAttachData->m_astBuffSkillList[lBuffedIndex].bIsOnlyUpdate	= FALSE;
	}

	return TRUE;
}

BOOL AgpmSkill::SetCallbackInitSkill(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_INIT_SKILL, pfCallback, pClass);
}

BOOL AgpmSkill::SetCallbackUpdateSkill(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_UPDATE_SKILL, pfCallback, pClass);
}

BOOL AgpmSkill::SetCallbackInitSkillTemplate(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_INIT_SKILL_TEMPLATE, pfCallback, pClass);
}

BOOL AgpmSkill::SetCallbackReceiveAction(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_RECEIVE_ACTION, pfCallback, pClass);
}

BOOL AgpmSkill::SetCallbackReceiveActionResult(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_RECEIVE_ACTION_RESULT, pfCallback, pClass);
}

BOOL AgpmSkill::SetCallbackReceiveRealHit(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_RECEIVE_REAL_HIT, pfCallback, pClass);
}

BOOL AgpmSkill::SetCallbackReceiveAdditionalEffect(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_RECEIVE_ADDITIONAL_EFFECT, pfCallback, pClass);
}

BOOL AgpmSkill::SetCallbackCheckNowUpdateActionFactor(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_CHECK_NOW_UPDATE_ACTION_FACTOR, pfCallback, pClass);
}

BOOL AgpmSkill::SetCallbackChangeStatus(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_CHANGE_STATUS, pfCallback, pClass);
}

BOOL  AgpmSkill::SetCallbackAddBuffedList(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_ADD_BUFF, pfCallback, pClass);
}

BOOL  AgpmSkill::SetCallbackRemoveBuffedList(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_REMOVE_BUFF, pfCallback, pClass);
}

BOOL  AgpmSkill::SetCallbackRemoveSkill(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_REMOVE_SKILL, pfCallback, pClass);
}

BOOL  AgpmSkill::SetCallbackRemoveID(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_REMOVE_ID, pfCallback, pClass);
}

BOOL  AgpmSkill::SetCallbackDeleteSkill(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_DELETE_SKILL, pfCallback, pClass);
}

BOOL  AgpmSkill::SetCallbackUpdateSkillPoint(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_UPDATE_SKILL_POINT, pfCallback, pClass);
}

BOOL  AgpmSkill::SetCallbackUpdateDIRTPoint(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_UPDATE_DIRT_POINT, pfCallback, pClass);
}

BOOL  AgpmSkill::SetCallbackGetInputTotalCostSP(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_GET_INPUT_TOTAL_COST_SP, pfCallback, pClass);
}

BOOL AgpmSkill::SetCallbackGetInputTotalCostHeroicPoint(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_GET_INPUT_TOTAL_COST_HEROIC_POINT, pfCallback, pClass);
}

BOOL  AgpmSkill::SetCallbackLearnSkill(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_LEARN_SKILL, pfCallback, pClass);
}

BOOL  AgpmSkill::SetCallbackForgetSkill(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_FORGET_SKILL, pfCallback, pClass);
}

BOOL  AgpmSkill::SetCallbackSetConstFactor(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_SET_CONST_FACTOR, pfCallback, pClass);
}

BOOL  AgpmSkill::SetCallbackUpdateSkillExp(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_UPDATE_SKILL_EXP, pfCallback, pClass);
}

/*
BOOL  AgpmSkill::SetCallbackMasteryResult(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_MASTERY_RESULT, pfCallback, pClass);
}
*/

//BOOL  AgpmSkill::SetCallbackMasteryUpdate(ApModuleDefaultCallBack pfCallback, PVOID pClass)
//{
//	return SetCallback(AGPMSKILL_CB_MASTERY_UPDATE, pfCallback, pClass);
//}
//
//BOOL  AgpmSkill::SetCallbackMasteryNodeUpdate(ApModuleDefaultCallBack pfCallback, PVOID pClass)
//{
//	return SetCallback(AGPMSKILL_CB_MASTERY_NODE_UPDATE, pfCallback, pClass);
//}
//
//BOOL  AgpmSkill::SetCallbackMasterySpecialize(ApModuleDefaultCallBack pfCallback, PVOID pClass)
//{
//	return SetCallback(AGPMSKILL_CB_MASTERY_SPECIALIZE, pfCallback, pClass);
//}
//
//BOOL  AgpmSkill::SetCallbackMasteryUnSpecialize(ApModuleDefaultCallBack pfCallback, PVOID pClass)
//{
//	return SetCallback(AGPMSKILL_CB_MASTERY_UNSPECIALIZE, pfCallback, pClass);
//}
//
//BOOL  AgpmSkill::SetCallbackMasterySpecializeResult(ApModuleDefaultCallBack pfCallback, PVOID pClass)
//{
//	return SetCallback(AGPMSKILL_CB_MASTERY_SPECIALIZE_RESULT, pfCallback, pClass);
//}
//
//BOOL  AgpmSkill::SetCallbackMasteryInit(ApModuleDefaultCallBack pfCallback, PVOID pClass)
//{
//	return SetCallback(AGPMSKILL_CB_MASTERY_ADD, pfCallback, pClass);
//}
//
//BOOL  AgpmSkill::SetCallbackMasteryChange(ApModuleDefaultCallBack pfCallback, PVOID pClass)
//{
//	return SetCallback(AGPMSKILL_CB_MASTERY_CHANGE, pfCallback, pClass);
//}
//
//BOOL  AgpmSkill::SetCallbackMasteryChangeResult(ApModuleDefaultCallBack pfCallback, PVOID pClass)
//{
//	return SetCallback(AGPMSKILL_CB_MASTERY_CHANGE_RESULT, pfCallback, pClass);
//}
//
//BOOL  AgpmSkill::SetCallbackMasteryUpdateTotalSP(ApModuleDefaultCallBack pfCallback, PVOID pClass)
//{
//	return SetCallback(AGPMSKILL_CB_MASTERY_UPDATE_TOTAL_SP, pfCallback, pClass);
//}
//
//BOOL  AgpmSkill::SetCallbackMasteryRollback(ApModuleDefaultCallBack pfCallback, PVOID pClass)
//{
//	return SetCallback(AGPMSKILL_CB_MASTERY_ROLLBACK, pfCallback, pClass);
//}
//
//BOOL  AgpmSkill::SetCallbackMasteryRollbackResult(ApModuleDefaultCallBack pfCallback, PVOID pClass)
//{
//	return SetCallback(AGPMSKILL_CB_MASTERY_ROLLBACK_RESULT, pfCallback, pClass);
//}

BOOL  AgpmSkill::SetCallbackInitSpecialize(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_INIT_SPECIALIZE, pfCallback, pClass);
}

//BOOL  AgpmSkill::SetCallbackAddSPResult(ApModuleDefaultCallBack pfCallback, PVOID pClass)
//{
//	return SetCallback(AGPMSKILL_CB_MASTERY_ADD_SP_RESULT, pfCallback, pClass);
//}

BOOL AgpmSkill::SetCallbackCheckSkillPvP(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_CHECK_SKILL_PVP, pfCallback, pClass);
}

BOOL AgpmSkill::SetCallbackRollbackSkill(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_ROLLBACK_SKILL, pfCallback, pClass);
}

BOOL AgpmSkill::SetCallbackModifiedSkillLevel(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_MODIFIED_SKILL_LEVEL, pfCallback, pClass);
}

BOOL AgpmSkill::SetCallbackGetModifiedSkillLevel(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_GET_MODIFIED_SKILL_LEVEL, pfCallback, pClass);
}

BOOL AgpmSkill::SetCallbackInitCoolTime(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_INIT_COOLTIME, pfCallback, pClass);
}

BOOL AgpmSkill::SetCallbackSpecificSkillLevel(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGPMSKILL_CB_SPECIFIC_SKILL_LEVEL, pfCallback, pClass);
}

INT32 AgpmSkill::AddBuffedList( pstAgpmSkillBuffList pstBuffList, INT32 lSkillTID, INT32 lSkillID, INT16 nOwnerType, INT32 lOwnerID, INT32 lCasterTID, INT32 lSpellCount, UINT32 ulDurationTimeMSec, INT32 lChargeLevel , UINT32 ulExpireTime )
{
	if (!pstBuffList || lSkillTID == AP_INVALID_SKILLID)
		return (-1);

	int i = 0;

	// 이미 들어있는 넘인지 살펴본다.
	for (i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
	{
		if (pstBuffList[i].lSkillTID == lSkillTID)
		{
			pstBuffList[i].bIsOnlyUpdate	= TRUE;
			break;
		}

		if (pstBuffList[i].lSkillTID == AP_INVALID_SKILLID)
		{
			pstBuffList[i].bIsOnlyUpdate	= FALSE;
			break;
		}
	}

	if (i == AGPMSKILL_MAX_SKILL_BUFF)	// 이미 꽉찼다. ㅡ.ㅡ
		return (-1);

	pstBuffList[i].lSkillTID = lSkillTID;
	pstBuffList[i].lSkillID = lSkillID;
	pstBuffList[i].csOwner.m_eType = (ApBaseType) nOwnerType;
	pstBuffList[i].csOwner.m_lID = lOwnerID;
	pstBuffList[i].lCasterTID = lCasterTID;
	pstBuffList[i].lSpellCount = lSpellCount;
	pstBuffList[i].ulDurationTimeMSec = ulDurationTimeMSec;
	pstBuffList[i].lChargeLevel	= lChargeLevel;
	pstBuffList[i].ulExpireTime = ulExpireTime;

	pstBuffList[i].pcsSkillTemplate	= GetSkillTemplate(lSkillTID);

	return i;
}

INT32 AgpmSkill::RemoveBuffedList(pstAgpmSkillBuffList pstBuffList, INT32 lTID)
{
	if (!pstBuffList || lTID == AP_INVALID_SKILLID)
		return (-1);

	for (int i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
	{
		if (pstBuffList[i].lSkillTID == AP_INVALID_SKILLID)
			break;

		if (pstBuffList[i].lSkillTID == lTID)
		{
			return RemoveBuffedListByIndex(pstBuffList, i);
		}
	}

	return (-1);
}

INT32 AgpmSkill::RemoveBuffedListByIndex(pstAgpmSkillBuffList pstBuffList, INT32 lIndex)
{
	if (!pstBuffList || lIndex < 0 || lIndex >= AGPMSKILL_MAX_SKILL_BUFF)
		return (-1);

	CopyMemory(pstBuffList + lIndex, pstBuffList + lIndex + 1, sizeof(stAgpmSkillBuffList) * (AGPMSKILL_MAX_SKILL_BUFF - lIndex - 1));
	ZeroMemory(pstBuffList + AGPMSKILL_MAX_SKILL_BUFF - 1, sizeof(stAgpmSkillBuffList));

	return lIndex;
}

pstAgpmSkillBuffList AgpmSkill::GetBuffedList(ApBase *pcsBase)
{
	if (!pcsBase)
		return NULL;

	AgpdSkillAttachData *pcsAttachData = GetAttachSkillData(pcsBase);
	if (!pcsAttachData)
		return NULL;

	return &pcsAttachData->m_astBuffSkillList[0];
}

// 2005.05.09. steeple
INT32 AgpmSkill::GetBuffedSkillCount(ApBase* pcsBase)
{
	if(!pcsBase)
		return -1;

	AgpdSkillAttachData *pcsAttachData = GetAttachSkillData(pcsBase);
	if (!pcsAttachData)
		return -1;

	// 하나도 없을 때 체크.
	if(pcsAttachData->m_astBuffSkillList[0].lSkillID == AP_INVALID_SKILLID ||
		!pcsAttachData->m_astBuffSkillList[0].pcsSkillTemplate)
		return 0;

	INT32 iCount = 0;
	for(iCount; iCount < AGPMSKILL_MAX_SKILL_BUFF; iCount++)
	{
		if(pcsAttachData->m_astBuffSkillList[iCount].lSkillID == AP_INVALID_SKILLID ||
			!pcsAttachData->m_astBuffSkillList[iCount].pcsSkillTemplate)
			break;
	}

	return iCount;
}

// 2007.10.24. steeple
// pcsBase 한테 lSkillTID 의 스킬이 버프상태인지 확인한다.
BOOL AgpmSkill::IsBuffedSkillByTID(ApBase* pcsBase, INT32 lSkillTID)
{
	if(!pcsBase || !lSkillTID)
		return FALSE;

	AgpdSkillAttachData* pcsAttachData = GetAttachSkillData(pcsBase);
	if(!pcsAttachData)
		return FALSE;

	for(INT32 i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
	{
		if(pcsAttachData->m_astBuffSkillList[i].lSkillID == AP_INVALID_SKILLID ||
			!pcsAttachData->m_astBuffSkillList[i].pcsSkillTemplate)
			break;

		if(pcsAttachData->m_astBuffSkillList[i].pcsSkillTemplate->m_lID == lSkillTID)
			return TRUE;
	}

	return FALSE;
}

BOOL AgpmSkill::AddDefaultSkillTID(ApBase *pcsBaseTemplate, INT32 lSkillTID)
{
	if (!pcsBaseTemplate || lSkillTID == AP_INVALID_SKILLID)
		return FALSE;

	AgpdSkillTemplateAttachData *pcsTemplateAttachData = GetAttachSkillTemplateData(pcsBaseTemplate);

	AgpdSkillTemplate	*pcsTemplate = GetSkillTemplate(lSkillTID);
	if (!pcsTemplate)
		return FALSE;
	
	for (int i = 0; i < AGPMSKILL_MAX_DEFAULT_SKILL; ++i)
	{
		if (!strlen(pcsTemplateAttachData->m_aszDefaultTName[i]))
		{
			strncpy(pcsTemplateAttachData->m_aszDefaultTName[i], pcsTemplate->m_szName, AGPMSKILL_MAX_SKILL_NAME);

			break;
		}
	}

	return TRUE;
}

BOOL AgpmSkill::RemoveDefaultSkillTID(ApBase *pcsBaseTemplate, INT32 lSkillTID)
{
	if (!pcsBaseTemplate || lSkillTID == AP_INVALID_SKILLID)
		return FALSE;

	AgpdSkillTemplateAttachData *pcsTemplateAttachData = GetAttachSkillTemplateData(pcsBaseTemplate);
	
	AgpdSkillTemplate	*pcsTemplate = GetSkillTemplate(lSkillTID);
	if (!pcsTemplate)
		return FALSE;
	
	for (int i = 0; i < AGPMSKILL_MAX_DEFAULT_SKILL; ++i)
	{
		if (!strncmp(pcsTemplateAttachData->m_aszDefaultTName[i], pcsTemplate->m_szName, AGPMSKILL_MAX_SKILL_NAME))
		{
			CopyMemory(pcsTemplateAttachData->m_aszDefaultTName + i * AGPMSKILL_MAX_SKILL_NAME, pcsTemplateAttachData->m_aszDefaultTName + (i + 1) * AGPMSKILL_MAX_SKILL_NAME, 
				sizeof(CHAR) * (AGPMSKILL_MAX_DEFAULT_SKILL - i - 1) * AGPMSKILL_MAX_SKILL_NAME);

			pcsTemplateAttachData->m_aszDefaultTName[AGPMSKILL_MAX_DEFAULT_SKILL - 1][0] = NULL;

			break;
		}
	}

	return TRUE;
}

BOOL AgpmSkill::CleanDefaultSkillTID(ApBase *pcsBaseTemplate)
{
	if (!pcsBaseTemplate)
		return FALSE;

	AgpdSkillTemplateAttachData *pcsTemplateAttachData = GetAttachSkillTemplateData(pcsBaseTemplate);

	ZeroMemory(pcsTemplateAttachData->m_aszDefaultTName, sizeof(CHAR) * AGPMSKILL_MAX_DEFAULT_SKILL * (AGPMSKILL_MAX_SKILL_NAME + 1));

	return TRUE;
}

/*
BOOL AgpmSkill::AddAffectedBase(AgpdSkill *pcsSkill, ApBase *pcsBase)
{
	if (!pcsSkill || !pcsBase)
		return FALSE;

	for (int i = 0; i < AGPMSKILL_MAX_AFFECTED_BASE; ++i)
	{
		if (pcsSkill->m_acsAffectedBase[i].m_eType == pcsBase->m_eType && pcsSkill->m_acsAffectedBase[i].m_lID == pcsBase->m_lID)
			return TRUE;

		if (pcsSkill->m_acsAffectedBase[i].m_lID == 0)
		{
			pcsSkill->m_acsAffectedBase[i].m_eType	= pcsBase->m_eType;
			pcsSkill->m_acsAffectedBase[i].m_lID	= pcsBase->m_lID;

			return TRUE;
		}
	}

	return FALSE;
}

BOOL AgpmSkill::InitialAffectedBase(AgpdSkill *pcsSkill)
{
	if (!pcsSkill)
		return FALSE;

	ZeroMemory(pcsSkill->m_acsAffectedBase, sizeof(ApBase) * AGPMSKILL_MAX_AFFECTED_BASE);

	return TRUE;
}
*/

BOOL AgpmSkill::AddUseList(AgpdSkill *pcsSkill, INT32 lSkillID)
{
	if (!pcsSkill || !pcsSkill->m_pcsBase || lSkillID == AP_INVALID_SKILLID)
		return FALSE;

	AgpdSkillAttachData *pcsAttachData = GetAttachSkillData(pcsSkill->m_pcsBase);
	if (!pcsAttachData)
		return FALSE;

	for (int i = 0; i < AGPMSKILL_MAX_SKILL_USE; ++i)
	{
		if (pcsAttachData->m_alUseSkillID[i] == lSkillID)
			return TRUE;

		if (pcsAttachData->m_alUseSkillID[i] == AP_INVALID_SKILLID)
		{
			pcsAttachData->m_alUseSkillID[i] = lSkillID;
			pcsAttachData->m_apcsUseSkill[i] = pcsSkill;

			return TRUE;
		}
	}

	return FALSE;
}

BOOL AgpmSkill::RemoveUseList(AgpdSkill *pcsSkill, INT32 lSkillID)
{
	if (!pcsSkill || !pcsSkill->m_pcsBase || lSkillID == AP_INVALID_SKILLID)
		return FALSE;

	AgpdSkillAttachData *pcsAttachData = GetAttachSkillData(pcsSkill->m_pcsBase);
	if (!pcsAttachData)
		return FALSE;

	for (int i = 0; i < AGPMSKILL_MAX_SKILL_USE; ++i)
	{
		if (pcsAttachData->m_alUseSkillID[i] == lSkillID)
		{
			//CopyMemory(pcsAttachData->m_alUseSkillID + i, pcsAttachData->m_alUseSkillID + i + 1, sizeof(INT32) * (AGPMSKILL_MAX_SKILL_USE - i - 1));
			pcsAttachData->m_alUseSkillID.MemCopy(i, &pcsAttachData->m_alUseSkillID[i + 1], AGPMSKILL_MAX_SKILL_USE - i - 1);
			pcsAttachData->m_alUseSkillID[AGPMSKILL_MAX_SKILL_USE - 1] = AP_INVALID_SKILLID;

			pcsAttachData->m_apcsUseSkill.MemCopy(i, &pcsAttachData->m_apcsUseSkill[i + 1], AGPMSKILL_MAX_SKILL_USE - i - 1);
			pcsAttachData->m_apcsUseSkill[AGPMSKILL_MAX_SKILL_USE - 1] = NULL;

			return TRUE;
		}

		if (pcsAttachData->m_alUseSkillID[i] == AP_INVALID_SKILLID)
			break;
	}

	return TRUE;
}

BOOL AgpmSkill::CheckSkillInUseList(ApBase *pcsBase, CHAR *szSkillName)
{
	if(pcsBase == NULL || szSkillName == NULL)
		return FALSE;

	AgpdSkillAttachData *pcsAttachData = GetAttachSkillData(pcsBase);
	if(pcsAttachData == NULL)
		return FALSE;

	for(int i=0; i<AGPMSKILL_MAX_SKILL_OWN; ++i)
	{
		AgpdSkill *pcsSkill = pcsAttachData->m_apcsUseSkill[i];
		if(pcsSkill == NULL)
			break;

		if(!strncmp(((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate)->m_szName, szSkillName, AGPMSKILL_MAX_SKILL_NAME))
			return TRUE;
	}

	return FALSE;
}

BOOL AgpmSkill::CheckHaveAnySkill(AgpdCharacter *pcsCharacter)
{
	if(pcsCharacter == NULL)
		return FALSE;

	BOOL bHaveAnySkill = FALSE;

	AgpdSkillAttachData *pcsAttachData = GetAttachSkillData((ApBase*)pcsCharacter);
	if(pcsAttachData == NULL)
		return FALSE;

	AgpmEventSkillMaster *pcsEventSkillMaster = (AgpmEventSkillMaster*)GetModule("AgpmEventSkillMaster");
	if(pcsEventSkillMaster == NULL)
		return FALSE;

	for(int i = 0; i < AGPMSKILL_MAX_SKILL_OWN; ++i)
	{
		if (pcsAttachData->m_alSkillID[i] == AP_INVALID_SKILLID)
			break;

		AgpdSkill* pcsSkill = GetSkill(pcsAttachData->m_alSkillID[i]);
		if (pcsSkill != NULL)
		{
			AgpdEventSkillAttachTemplateData	*pcsAttachTemplateData	= pcsEventSkillMaster->GetSkillAttachTemplateData((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate);
			if(pcsAttachTemplateData)
			{
				// 스킬포인트가 0이라는것은 기본스킬(일반공격), 즉 요건 Pass해준다.
				if(pcsAttachTemplateData->m_stSkillCost[1].lCostSkillPoint == 0)
					continue;
			}
			
			bHaveAnySkill = TRUE;
			break;
		}
	}

	return bHaveAnySkill;
}

BOOL AgpmSkill::AddUsePassiveList(AgpdSkill *pcsSkill, INT32 lSkillID)
{
	if (!pcsSkill || !pcsSkill->m_pcsBase || lSkillID == AP_INVALID_SKILLID)
		return FALSE;

	AgpdSkillAttachData *pcsAttachData = GetAttachSkillData(pcsSkill->m_pcsBase);
	if (!pcsAttachData)
		return FALSE;

	for (int i = 0; i < AGPMSKILL_MAX_SKILL_USE; ++i)
	{
		if (pcsAttachData->m_alUsePassiveSkillID[i] == lSkillID)
			return TRUE;

		if (pcsAttachData->m_alUsePassiveSkillID[i] == AP_INVALID_SKILLID)
		{
			pcsAttachData->m_alUsePassiveSkillID[i] = lSkillID;
			pcsAttachData->m_apcsUsePassiveSkill[i] = pcsSkill;

			return TRUE;
		}
	}

	return FALSE;
}

BOOL AgpmSkill::RemoveUsePassiveList(AgpdSkill *pcsSkill, INT32 lSkillID)
{
	if (!pcsSkill || !pcsSkill->m_pcsBase || lSkillID == AP_INVALID_SKILLID)
		return FALSE;

	AgpdSkillAttachData *pcsAttachData = GetAttachSkillData(pcsSkill->m_pcsBase);
	if (!pcsAttachData)
		return FALSE;

	for (int i = 0; i < AGPMSKILL_MAX_SKILL_USE; ++i)
	{
		if (pcsAttachData->m_alUsePassiveSkillID[i] == lSkillID)
		{
			//CopyMemory(pcsAttachData->m_alUsePassiveSkillID + i, pcsAttachData->m_alUsePassiveSkillID + i + 1, sizeof(INT32) * (AGPMSKILL_MAX_SKILL_USE - i - 1));
			pcsAttachData->m_alUsePassiveSkillID.MemCopy(i, &pcsAttachData->m_alUsePassiveSkillID[i + 1], AGPMSKILL_MAX_SKILL_USE - i - 1);
			pcsAttachData->m_alUsePassiveSkillID[AGPMSKILL_MAX_SKILL_USE - 1] = AP_INVALID_SKILLID;

			pcsAttachData->m_apcsUsePassiveSkill.MemCopy(i, &pcsAttachData->m_apcsUsePassiveSkill[i + 1], AGPMSKILL_MAX_SKILL_USE - i - 1);
			pcsAttachData->m_apcsUsePassiveSkill[AGPMSKILL_MAX_SKILL_USE - 1] = NULL;

			return TRUE;
		}

		if (pcsAttachData->m_alUsePassiveSkillID[i] == AP_INVALID_SKILLID)
			break;
	}

	return TRUE;
}

BOOL AgpmSkill::AddOwnSkillList(AgpdSkill *pcsSkill)
{
	if (!pcsSkill || !pcsSkill->m_pcsBase)
		return FALSE;

	AgpdSkillAttachData	*pcsAttachData = GetAttachSkillData(pcsSkill->m_pcsBase);
	if (!pcsAttachData)
		return FALSE;

	for (int i = 0; i < AGPMSKILL_MAX_SKILL_OWN; ++i)
	{
		if (pcsAttachData->m_alSkillID[i] == AP_INVALID_SKILLID)
		{
			pcsAttachData->m_alSkillID[i] = pcsSkill->m_lID;
			pcsAttachData->m_apcsSkill[i] = pcsSkill;

			return TRUE;
			break;
		}
	}

	return FALSE;
}

BOOL AgpmSkill::RemoveOwnSkillList(AgpdSkill *pcsSkill)
{
	if (!pcsSkill || !pcsSkill->m_pcsBase)
		return FALSE;

	AgpdSkillAttachData	*pcsAttachData = GetAttachSkillData(pcsSkill->m_pcsBase);
	if (!pcsAttachData)
		return FALSE;

	for (int i = 0; i < AGPMSKILL_MAX_SKILL_OWN; ++i)
	{
		if (pcsAttachData->m_alSkillID[i] == pcsSkill->m_lID)
		{
			//CopyMemory(pcsAttachData->m_alSkillID + i, pcsAttachData->m_alSkillID + i + 1, sizeof(INT32) * (AGPMSKILL_MAX_SKILL_OWN - i - 1));
			pcsAttachData->m_alSkillID.MemCopy(i, &pcsAttachData->m_alSkillID[i + 1], AGPMSKILL_MAX_SKILL_OWN - i - 1);
			pcsAttachData->m_alSkillID[AGPMSKILL_MAX_SKILL_OWN - 1] = AP_INVALID_SKILLID;

			pcsAttachData->m_apcsSkill.MemCopy(i, &pcsAttachData->m_apcsSkill[i + 1], AGPMSKILL_MAX_SKILL_OWN - i - 1);
			pcsAttachData->m_apcsSkill[AGPMSKILL_MAX_SKILL_OWN - 1] = NULL;

			return TRUE;
			break;
		}
	}

	return FALSE;
}

UINT64 AgpmSkill::GetTargetType(AgpdSkill *pcsSkill, ApBase *pcsTargetBase)
{
	if (!pcsSkill || !pcsSkill->m_pcsBase || !pcsTargetBase)
		return 0;

	if (pcsSkill->m_pcsBase == pcsTargetBase)
		return AGPMSKILL_TARGET_SELF_ONLY;
	else if (pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER &&
			m_pagpmCharacter->IsPC((AgpdCharacter *) pcsSkill->m_pcsBase) &&
			pcsTargetBase->m_eType == APBASE_TYPE_CHARACTER &&
			m_pagpmCharacter->IsPC((AgpdCharacter *) pcsTargetBase))
	{
		return AGPMSKILL_TARGET_FRIENDLY_UNITS;
	}
	else
		return AGPMSKILL_TARGET_ENEMY_UNITS;

	return 0;
}

AgpmSkillCheckTargetResult AgpmSkill::IsValidTarget(AgpdSkill *pcsSkill, ApBase *pcsTargetBase, BOOL IsForceAttack, AuPOS *pstCalcPos)
{
	if (!pcsSkill || !pcsSkill->m_pcsBase || pcsSkill->m_pcsBase->m_eType != APBASE_TYPE_CHARACTER || !pstCalcPos || !pcsTargetBase)
		return AGPMSKILL_CHECK_TARGET_INVALID;

	if (m_pagpmCharacter->IsAllBlockStatus((AgpdCharacter *) pcsSkill->m_pcsBase) ||
		pcsTargetBase->m_eType == APBASE_TYPE_CHARACTER &&
		m_pagpmCharacter->IsAllBlockStatus((AgpdCharacter *) pcsTargetBase) ||
		((AgpdCharacter*)pcsTargetBase)->m_bIsReadyRemove)
		return AGPMSKILL_CHECK_TARGET_INVALID;
		
	// 추후 부활 스킬등이 들어가게 된다면 이부분을 보완해야 한다.
	////////////////////////////////////////////////////////////////////////

	// if product skill, return TRUE;
	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_nAttribute & AGPMSKILL_SKILL_ATTR_PRODUCT)
		return AGPMSKILL_CHECK_TARGET_OK;

	// pcsTemplate->m_lTargetType을 보고 일단 필터링을 함 한다.
	//	Type에 따라 pcsTargetBase와 pcsSkill->m_pcsBase와의 관계를 정리한다.
	////////////////////////////////////////////////////////////////////////
	UINT64	ulTargetType	= GetTargetType(pcsSkill, pcsTargetBase);
	if (ulTargetType == AGPMSKILL_TARGET_FRIENDLY_UNITS)
	{
		if (!(((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lTargetType & ulTargetType))
		{
			if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lTargetType & AGPMSKILL_TARGET_ENEMY_UNITS)
			{
				if (!m_pagpmCharacter->CheckNormalAttack((AgpdCharacter *) pcsSkill->m_pcsBase, (AgpdCharacter *) pcsTargetBase, IsForceAttack))
					return AGPMSKILL_CHECK_TARGET_INVALID;
			}
		}
	}

	// 타겟이 그라운드 관련이라면 일단 넘어간다. 2006.10.25. steeple
	else if(IsGroundTargetSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
	{
	}

	else if (ulTargetType == AGPMSKILL_TARGET_SELF_ONLY &&
		((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lTargetType & AGPMSKILL_TARGET_SELF_ONLY)
		return AGPMSKILL_CHECK_TARGET_OK;

	// 타겟이 공성관련이라면 넘어간다.
	else if(IsUnlimitedTargetSiegeWarSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
	{
		// 공성 관련 스킬에서는 일단 넘어간다.
	}

	else
	{
		if (!(((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lTargetType & ulTargetType))
			return AGPMSKILL_CHECK_TARGET_INVALID;
	}
	
	if (pcsTargetBase->m_eType == APBASE_TYPE_CHARACTER)
	{

		if (!IsDeadTarget((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate) && ((AgpdCharacter *) pcsTargetBase)->m_unActionStatus == AGPDCHAR_STATUS_DEAD)
			return AGPMSKILL_CHECK_TARGET_INVALID;

		if (!m_pagpmCharacter->IsTargetable((AgpdCharacter *) pcsTargetBase))
			return AGPMSKILL_CHECK_TARGET_INVALID;
	}

	// Check Target Restriction
	///////////////////////////////////////////////////////////////////////////
	if(CheckTargetRestriction(pcsSkill, pcsTargetBase) == TRUE)
		return AGPMSKILL_CHECK_TARGET_INVALID;

	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lRangeType & AGPMSKILL_CHECK_RANGE_NOT_CHECK ||
		((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lRangeType & AGPMSKILL_RANGE_SELF_ONLY)
		return AGPMSKILL_CHECK_TARGET_OK;

	if (!pcsTargetBase || pcsTargetBase->m_eType != APBASE_TYPE_CHARACTER)
		return AGPMSKILL_CHECK_TARGET_INVALID;

	BOOL	bOK	= FALSE;

	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lRangeType & AGPMSKILL_RANGE_TARGET_ONLY)
	{
		if (pcsSkill->m_pcsBase->m_eType	== pcsTargetBase->m_eType &&
			pcsSkill->m_pcsBase->m_lID		== pcsTargetBase->m_lID)
			return AGPMSKILL_CHECK_TARGET_INVALID;

		if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lRangeType & AGPMSKILL_RANGE_TARGET_ONLY_ENEMY_UNITS)
		{
			if (!m_pagpmCharacter->IsTargetable((AgpdCharacter *) pcsTargetBase))
				return AGPMSKILL_CHECK_TARGET_INVALID;

			if (!m_pagpmCharacter->CheckNormalAttack((AgpdCharacter *) pcsSkill->m_pcsBase, (AgpdCharacter *) pcsTargetBase, IsForceAttack))
				return AGPMSKILL_CHECK_TARGET_INVALID;
			else
				bOK	= TRUE;
		}

		if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lRangeType & AGPMSKILL_RANGE_TARGET_ONLY_FRIENDLY_UNITS)
		{
			if (!m_pagpmCharacter->CheckNormalAttack((AgpdCharacter *) pcsSkill->m_pcsBase, (AgpdCharacter *) pcsTargetBase, FALSE))
				return AGPMSKILL_CHECK_TARGET_INVALID;
			else
				bOK	= TRUE;
		}

		// 2007.07.12 아무한테나(몹이던, 적이던 같은 편이던) 다 쓸수 있는 스킬인 데 거리체크는 필요하다.
		if(!bOK && !(((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lRangeType & AGPMSKILL_CHECK_RANGE_NOT_CHECK))
			bOK = TRUE;
	}
	else if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lRangeType & AGPMSKILL_RANGE_SPHERE)
	{
		if (m_pagpmParty && ((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lRangeType & AGPMSKILL_RANGE_SPHERE_ONLY_PARTY_MEMBER)
		{
			AgpdPartyADChar	*pcsPartyADChar	= m_pagpmParty->GetADCharacter((AgpdCharacter *) pcsSkill->m_pcsTemplate);
			if (!pcsPartyADChar)
				return AGPMSKILL_CHECK_TARGET_INVALID;

			if (pcsPartyADChar->lPID == AP_INVALID_PARTYID)
				return AGPMSKILL_CHECK_TARGET_INVALID;

			AgpdPartyADChar	*pcsTargetPartyADChar	= m_pagpmParty->GetADCharacter((AgpdCharacter *) pcsTargetBase);
			if (!pcsTargetPartyADChar)
				return AGPMSKILL_CHECK_TARGET_INVALID;

			if (pcsPartyADChar->lPID == pcsTargetPartyADChar->lPID)
				bOK = TRUE;
			else
				return AGPMSKILL_CHECK_TARGET_INVALID;
		}

		if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lRangeType & AGPMSKILL_RANGE_SPHERE_ONLY_ENEMY_UNITS)
		{
			if (!m_pagpmCharacter->IsTargetable((AgpdCharacter *) pcsTargetBase))
				return AGPMSKILL_CHECK_TARGET_INVALID;

			if (!m_pagpmCharacter->CheckNormalAttack((AgpdCharacter *) pcsSkill->m_pcsBase, (AgpdCharacter *) pcsTargetBase, IsForceAttack))
				return AGPMSKILL_CHECK_TARGET_INVALID;
			else
				bOK	= TRUE;

		}

		if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lRangeType & AGPMSKILL_RANGE_SPHERE_ONLY_FRIENDLY_UNITS)
		{
			if (!m_pagpmCharacter->CheckNormalAttack((AgpdCharacter *) pcsSkill->m_pcsBase, (AgpdCharacter *) pcsTargetBase, FALSE))
				return AGPMSKILL_CHECK_TARGET_INVALID;
			else
				bOK	= TRUE;
		}

		// 2006.08.23. steeple
		// 공성관련해서 범위 스킬인거다.
		// 일단 타겟할 수만 있으면 통과시킨다.
		if (IsSphereRangeSiegeWarSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
		{
			if (!m_pagpmCharacter->IsTargetable((AgpdCharacter *) pcsTargetBase))
				return AGPMSKILL_CHECK_TARGET_INVALID;
			else
				bOK	= TRUE;
		}
	}
	else if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lRangeType & AGPMSKILL_RANGE_BOX)
	{
		if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lRangeType & AGPMSKILL_RANGE_BOX_ONLY_ENEMY_UNITS)
		{
			if (!m_pagpmCharacter->IsTargetable((AgpdCharacter *) pcsTargetBase))
				return AGPMSKILL_CHECK_TARGET_INVALID;

			if (!m_pagpmCharacter->CheckNormalAttack((AgpdCharacter *) pcsSkill->m_pcsBase, (AgpdCharacter *) pcsTargetBase, IsForceAttack))
				return AGPMSKILL_CHECK_TARGET_INVALID;
			else
				bOK	= TRUE;
		}
	}

	// 그라운드 타겟이라면 2006.10.20. steeple
	if(IsGroundTargetSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
	{
		// 일단 OK
		bOK = TRUE;
	}

	if (bOK)
	{
		// 거리를 체크한다.
		INT32	lRange	= GetRange(pcsSkill);
		if (lRange <= 0)
			return AGPMSKILL_CHECK_TARGET_INVALID;

		AuPOS	*pcsBasePos = m_papmEventManager->GetBasePos(pcsSkill->m_pcsBase, NULL);
		AuPOS	*pcsSourcePos = NULL;
		
		// 2006.10.20. steeple
		if(IsGroundTargetSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
			pcsSourcePos = &pcsSkill->m_posTarget;
		else
			pcsSourcePos = m_papmEventManager->GetBasePos(pcsTargetBase, NULL);

		INT32	lTargetRange	= 0;

		//if (lRange < lTargetDistance)
		if (!m_pagpmFactors->IsInRange(pcsBasePos, pcsSourcePos, lRange + pcsSkill->m_lTargetHitRange - 25, 0, pstCalcPos, &lTargetRange))
		{
            // 너무 멀리 있다면 잘못된 타겟으로 간주한다. (100미터 밖에 있는 놈에게 스킬을 쓸 일이 있느냐,,,)
			if (lTargetRange > 10000)
				return AGPMSKILL_CHECK_TARGET_INVALID;

			return AGPMSKILL_CHECK_TARGET_NOT_IN_RANGE;
		}
	}

	return AGPMSKILL_CHECK_TARGET_OK;
}

BOOL AgpmSkill::CheckSkillPvP(AgpdSkill* pcsSkill, ApBase* pcsTargetBase)
{
	if(!pcsSkill || !pcsSkill->m_pcsBase || !pcsTargetBase)
		return FALSE;

	return EnumCallback(AGPMSKILL_CB_CHECK_SKILL_PVP, pcsSkill, pcsTargetBase);
}

// Check Target Restriction
// return true  = Restriction
//		  false = Not Restriction
//////////////////////////////////////////////////////////////////////////////////////////
BOOL AgpmSkill::CheckTargetRestriction(AgpdSkill* pcsSkill, ApBase* pcsTargetBase)
{
	if(NULL == pcsSkill || NULL == pcsTargetBase)
		return FALSE;

	// Target Restrict Check
	////////////////////////////////////////////////////////////////////////////////////
	if(((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate)->m_lEffectType2[0] & AGPMSKILL_EFFECT2_TARGET_RESTRICTION)
	{
		AgpmSiegeWar *pcsSiegeWar = (AgpmSiegeWar*)GetModule("AgpmSiegeWar");
		if(pcsSiegeWar)
		{
			// restriction siegewar object
			if(((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate)->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_TARGET_RESTRICTION] & AGPMSKILL_EFFECT_DETAIL_TARGET_RESTRICTION_SIEGE_OBJECT)
			{
				AgpdSiegeWarMonsterType eType = pcsSiegeWar->GetSiegeWarMonsterType((AgpdCharacter*)pcsTargetBase);
				if(eType >= AGPD_SIEGE_MONSTER_GUARD_TOWER && eType <= AGPD_SIEGE_MONSTER_CATAPULT)
					return TRUE;
			}

		}

		// 몬스터에게 사용 불가
		if(((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate)->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_TARGET_RESTRICTION] & AGPMSKILL_EFFECT_DETAIL_TARGET_RESTRICTION_MONSTER)
		{			
			if ( m_pagpmCharacter->IsMonster( (AgpdCharacter*)pcsTargetBase ) )
				return TRUE;
		}

		// 적 플레이어 캐릭터에게 사용 불가
		if(((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate)->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_TARGET_RESTRICTION] & AGPMSKILL_EFFECT_DETAIL_TARGET_RESTRICTION_ENEMY_PC)
		{		
			AgpmPvP* pAgpmPvP = (AgpmPvP*) GetModule("AgpmPvP");
			if(!pAgpmPvP)
				return TRUE;

 			if ( pAgpmPvP->IsEnemyCharacter( (AgpdCharacter*)pcsSkill->m_pcsBase, (AgpdCharacter*)pcsTargetBase ) )
 				return TRUE;
		}
	}

	return FALSE;
}

INT32 AgpmSkill::GetCostHP(AgpdSkill *pcsSkill)
{
	if (!pcsSkill || !pcsSkill->m_pcsTemplate || !pcsSkill->m_pcsBase)
		return FALSE;

	INT32	lSkillLevel	= 0;

	EnumCallback(AGPMSKILL_CB_GET_MODIFIED_SKILL_LEVEL, pcsSkill, &lSkillLevel);

	INT32	lCostHP = (INT32) ((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_fUsedConstFactor[AGPMSKILL_CONST_COST_HP][lSkillLevel];

	if (lCostHP > 0)
	{
//		lCostHP	= GetSpecializedCost(pcsSkill, lCostHP);

		AgpdSkillAttachData	*pcsAttachData	= GetAttachSkillData(pcsSkill->m_pcsBase);
		if (pcsAttachData)
		{
			if (pcsAttachData->m_ullBuffedSkillFactorEffect & AGPMSKILL_EFFECT_SKILL_FACTOR_ADJUST_COST_HP)
			{
				INT32	lRandomNumber	= m_csRandom.randInt(100);
				if (lRandomNumber <= pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostHPProbability)
				{
					lCostHP = (INT32) (lCostHP * (pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostHP / 100.0));
				}
			}
		}
	}

	return lCostHP;
}

INT32 AgpmSkill::GetCostSP(AgpdSkill *pcsSkill)
{
	if (!pcsSkill || !pcsSkill->m_pcsTemplate || !pcsSkill->m_pcsBase)
		return FALSE;

	INT32	lSkillLevel	= 0;

	EnumCallback(AGPMSKILL_CB_GET_MODIFIED_SKILL_LEVEL, pcsSkill, &lSkillLevel);

	INT32	lCostSP = (INT32) ((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_fUsedConstFactor[AGPMSKILL_CONST_COST_SP][lSkillLevel];

	if (lCostSP > 0)
	{
//		lCostSP	= GetSpecializedCost(pcsSkill, lCostSP);

		AgpdSkillAttachData	*pcsAttachData	= GetAttachSkillData(pcsSkill->m_pcsBase);
		if (pcsAttachData)
		{
			if (pcsAttachData->m_ullBuffedSkillFactorEffect & AGPMSKILL_EFFECT_SKILL_FACTOR_ADJUST_COST_SP)
			{
				INT32	lRandomNumber	= m_csRandom.randInt(100);
				if (lRandomNumber <= pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostSPProbability)
				{
					lCostSP = (INT32) (lCostSP * (pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostSP / 100.0));
				}
			}
		}
	}

	return lCostSP;
}

INT32 AgpmSkill::GetCostMP(AgpdSkill *pcsSkill)
{
	if (!pcsSkill || !pcsSkill->m_pcsTemplate || !pcsSkill->m_pcsBase)
		return 0;

	INT32	lSkillLevel	= 0;

	EnumCallback(AGPMSKILL_CB_GET_MODIFIED_SKILL_LEVEL, pcsSkill, &lSkillLevel);

	INT32	lCostMP = (INT32) ((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_fUsedConstFactor[AGPMSKILL_CONST_COST_MP][lSkillLevel];

	if (lCostMP > 0)
	{
//		lCostMP	= GetSpecializedCost(pcsSkill, lCostMP);

		AgpdSkillAttachData	*pcsAttachData	= GetAttachSkillData(pcsSkill->m_pcsBase);
		if (pcsAttachData)
		{
			if (pcsAttachData->m_ullBuffedSkillFactorEffect & AGPMSKILL_EFFECT_SKILL_FACTOR_ADJUST_COST_MP)
			{
				lCostMP = (INT32) (lCostMP * (pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostMP + 100) / 100.0);

				// 2005.01.31. steeple 변경. 일정확률로 CostMP를 0 으로 만든다.
				if(pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostMPProbability > 0)
				{
					INT32	lRandomNumber	= m_csRandom.randInt(100);
					if (lRandomNumber <= pcsAttachData->m_stBuffedSkillFactorEffectArg.lCostMPProbability)
					{
						lCostMP = 0;
					}
				}
			}
		}
	}

	return lCostMP;
}

INT32 AgpmSkill::GetCostArrow(AgpdSkill *pcsSkill)
{
	if (!pcsSkill || !pcsSkill->m_pcsTemplate || !pcsSkill->m_pcsBase)
		return FALSE;

	INT32	lSkillLevel	= 0;

	EnumCallback(AGPMSKILL_CB_GET_MODIFIED_SKILL_LEVEL, pcsSkill, &lSkillLevel);

	return (INT32) ((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_fUsedConstFactor[AGPMSKILL_CONST_COST_ARROW][lSkillLevel];
}

BOOL AgpmSkill::IsSufficientCost(AgpdSkill *pcsSkill, eAgpmSkillActionType *peActionType)
{
	if (!pcsSkill || !pcsSkill->m_pcsTemplate || !pcsSkill->m_pcsBase || !peActionType)
		return FALSE;

	// save skill - arycoat 2008.7.
	if(pcsSkill->m_pSaveSkillData.eStep == SAVESKILL_RECASTING)
		return TRUE;

	// owner의 HP, MP, SP를 가져온다.
	INT32	lHP	= 0;
	INT32	lMP	= 0;
	INT32	lSP	= 0;
	INT32	lArrow = 0;
	INT32	lBolt = 0;

	switch (pcsSkill->m_pcsBase->m_eType) {
	case APBASE_TYPE_CHARACTER:
		{
			m_pagpmFactors->GetValue(&((AgpdCharacter *) pcsSkill->m_pcsBase)->m_csFactor, &lHP, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT, AGPD_FACTORS_CHARPOINT_TYPE_HP);
			m_pagpmFactors->GetValue(&((AgpdCharacter *) pcsSkill->m_pcsBase)->m_csFactor, &lMP, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT, AGPD_FACTORS_CHARPOINT_TYPE_MP);
			m_pagpmFactors->GetValue(&((AgpdCharacter *) pcsSkill->m_pcsBase)->m_csFactor, &lSP, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_CHAR_POINT, AGPD_FACTORS_CHARPOINT_TYPE_SP);

// 			// lArrow 를 가져온다. -> 화살 소모하는 부분에서 가져오도록 변경
// 			lArrow	= m_pagpmItem->GetTotalArrowCountInInventory((AgpdCharacter *) pcsSkill->m_pcsBase);
// 			lBolt	= m_pagpmItem->GetTotalBoltCountInInventory((AgpdCharacter *) pcsSkill->m_pcsBase);
		}
		break;

	default:
		break;
	}

	// cost 값과 현재 값들과 비교해서 cost가 더 큰 경우 FALSE 리턴
	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lCostType & AGPMSKILL_COST_HP)
	{
		INT32	lCostHP = GetCostHP(pcsSkill);

		if (lCostHP != 0 && lCostHP > lHP)
		{
			*peActionType	= AGPMSKILL_ACTION_NOT_ENOUGH_HP;
			return FALSE;
		}
	}

	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lCostType & AGPMSKILL_COST_MP)
	{
		INT32	lCostMP = GetCostMP(pcsSkill);

		if (lCostMP != 0 && lCostMP > lMP)
		{
			*peActionType	= AGPMSKILL_ACTION_NOT_ENOUGH_MP;
			return FALSE;
		}
	}

	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lCostType & AGPMSKILL_COST_SP)
	{
		INT32	lCostSP = GetCostSP(pcsSkill);

		if (lCostSP != 0 && lCostSP > lSP)
		{
			*peActionType	= AGPMSKILL_ACTION_NOT_ENOUGH_SP;
			return FALSE;
		}
	}

	if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lCostType & AGPMSKILL_COST_ARROW)
	{
		INT32	lCostArrow = GetCostArrow(pcsSkill);

		if (m_pagpmItem->IsEquipBow((AgpdCharacter *) pcsSkill->m_pcsBase))
		{
			lArrow = m_pagpmItem->GetTotalArrowCountInInventory((AgpdCharacter *) pcsSkill->m_pcsBase);

			if (lCostArrow != 0 && lCostArrow > lArrow)
			{
				*peActionType	= AGPMSKILL_ACTION_NOT_ENOUGH_ARROW;
				return FALSE;
			}
		}
		else if (m_pagpmItem->IsEquipCrossBow((AgpdCharacter *) pcsSkill->m_pcsBase))
		{
			lBolt = m_pagpmItem->GetTotalBoltCountInInventory((AgpdCharacter *) pcsSkill->m_pcsBase);

			if (lCostArrow != 0 && lCostArrow > lBolt)
			{
				*peActionType	= AGPMSKILL_ACTION_NOT_ENOUGH_BOLT;
				return FALSE;
			}
		}
		// 2005.09.08. steeple
		// 카타리야 들고 있으면 화살이 필요없다.
		else if (m_pagpmItem->IsEquipKatariya((AgpdCharacter *) pcsSkill->m_pcsBase))
		{
		}
		else
		{
			*peActionType	= AGPMSKILL_ACTION_NOT_ENOUGH_ARROW;
			return FALSE;
		}

	}

	return TRUE;
}

BOOL AgpmSkill::IsSatisfyRequirement(AgpdSkill *pcsSkill)
{
	if (!pcsSkill || !pcsSkill->m_pcsBase || !pcsSkill->m_pcsTemplate)
		return FALSE;

	AgpdSkillTemplate	*pcsTemplate	= (AgpdSkillTemplate *) pcsSkill->m_pcsTemplate;

	// 아크로드 스킬이면 아크로드여야 한다. 2006.10.17. steeple
	if(IsArchlordSkill(pcsTemplate))
		return m_pagpmCharacter->IsArchlord((AgpdCharacter*)pcsSkill->m_pcsBase);

	// 2006.11.23. steeple
	// 아크로드는 오직 아크로드 스킬만 쓸 수 있다.
	else if(!(pcsTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_CANNOT_UPGRADE) &&
		m_pagpmCharacter->IsArchlord((AgpdCharacter*)pcsSkill->m_pcsBase))
		return FALSE;

	if (pcsTemplate->m_lRequirementType & AGPMSKILL_REQUIREMENT_EQUIP_WEAPON)
	{
		switch (pcsSkill->m_pcsBase->m_eType) {
		case APBASE_TYPE_CHARACTER:
			{
				AgpdCharacter	*pcsCharacter	= (AgpdCharacter *) pcsSkill->m_pcsBase;

				// 2005.01.16. steeple
				if(pcsTemplate->m_lRequirementType & AGPMSKILL_REQUIREMENT_EQUIP_ONE_HAND_SWORD)
				{
					return m_pagpmItem->IsEquipOneHandSword(pcsCharacter);
				}
				else if(pcsTemplate->m_lRequirementType & AGPMSKILL_REQUIREMENT_EQUIP_ONE_HAND_AXE)
				{
					return m_pagpmItem->IsEquipOneHandAxe(pcsCharacter);
				}
				else if(pcsTemplate->m_lRequirementType & AGPMSKILL_REQUIREMENT_EQUIP_BLUNT)
				{
					return m_pagpmItem->IsEquipBlunt(pcsCharacter);
				}
				else if(pcsTemplate->m_lRequirementType & AGPMSKILL_REQUIREMENT_EQUIP_TWO_HAND_SLASH)
				{
					return m_pagpmItem->IsEquipTwoHandSlash(pcsCharacter);
				}
				else if(pcsTemplate->m_lRequirementType & AGPMSKILL_REQUIREMENT_EQUIP_STAFF)
				{
					return m_pagpmItem->IsEquipStaff(pcsCharacter);
				}
				else if(pcsTemplate->m_lRequirementType & AGPMSKILL_REQUIREMENT_EQUIP_WAND)
				{
					return m_pagpmItem->IsEquipWand(pcsCharacter);
				}
				else if(pcsTemplate->m_lRequirementType & AGPMSKILL_REQUIREMENT_EQUIP_BOW)
				{
					return m_pagpmItem->IsEquipBow(pcsCharacter);
				}
				else if(pcsTemplate->m_lRequirementType & AGPMSKILL_REQUIREMENT_EQUIP_CROSS_BOW)
				{
					return m_pagpmItem->IsEquipCrossBow(pcsCharacter);
				}
				else if(pcsTemplate->m_lRequirementType & AGPMSKILL_REQUIREMENT_EQUIP_KATARIYA)
				{
					return m_pagpmItem->IsEquipKatariya(pcsCharacter);
				}
				else if(pcsTemplate->m_lRequirementType & AGPMSKILL_REQUIREMENT_EQUIP_CHAKRAM)
				{
					return m_pagpmItem->IsEquipChakram(pcsCharacter);
				}
				else if(pcsTemplate->m_lRequirementType & AGPMSKILL_REQUIREMENT_EQUIP_LEFT_HAND_SWORD)
				{
					return m_pagpmItem->IsEquipTwoHandBlunt(pcsCharacter);
				}
				else if(pcsTemplate->m_lRequirementType & AGPMSKILL_REQUIREMENT_EQUIP_RIGHT_WEAPON_ONLY)
				{
					return m_pagpmItem->IsEquipRightHandOnly(pcsCharacter);
				}
				else if(pcsTemplate->m_lRequirementType & AGPMSKILL_REQUIREMENT_EQUIP_CHARON)
				{
					return m_pagpmItem->IsEquipCharon(pcsCharacter);
				}
				else if(pcsTemplate->m_lRequirementType & AGPMSKILL_REQUIREMENT_EQUIP_ZENON)
				{
					return m_pagpmItem->IsEquipZenon(pcsCharacter);
				}
				else if(pcsTemplate->m_lRequirementType & AGPMSKILL_REQUIREMENT_EQUIP_CHARON_AND_ZENON)
				{
					return (m_pagpmItem->IsEquipCharon(pcsCharacter) && m_pagpmItem->IsEquipZenon(pcsCharacter));
				}
				else
				{
					return m_pagpmItem->IsEquipWeapon(pcsCharacter);
				}
			}
			break;
		}
	}

	if (pcsTemplate->m_lRequirementType & AGPMSKILL_REQUIREMENT_EQUIP_ARROW)
	{
		// arrow 는 weapon에 포함되어 버렸다. 고로 뭐 이넘은 쓸모없는 넘이다.
	}

	if (pcsTemplate->m_lRequirementType & AGPMSKILL_REQUIREMENT_EQUIP_SHIELD)
	{
		switch (pcsSkill->m_pcsBase->m_eType) {
		case APBASE_TYPE_CHARACTER:
			{
				AgpdItem	*pcsShield	= m_pagpmItem->GetEquipSlotItem( (AgpdCharacter *) pcsSkill->m_pcsBase, AGPMITEM_PART_HAND_LEFT );

				if( pcsShield == NULL )
					return FALSE;

				if (((AgpdItemTemplateEquip *) pcsShield->m_pcsItemTemplate)->m_nKind != AGPMITEM_EQUIP_KIND_SHIELD)
					return FALSE;

				AgpdCharacter	*pcsCharacter	= (AgpdCharacter *) pcsSkill->m_pcsBase;
				INT32			lCharClass		= m_pagpmFactors->GetClass(&pcsCharacter->m_pcsCharacterTemplate->m_csFactor);

				if (lCharClass == AUCHARCLASS_TYPE_RANGER)
					return FALSE;
			}
			break;
		}
	}

	if (pcsTemplate->m_lRequirementType & AGPMSKILL_REQUIREMENT_EQUIP_PRODUCT_TOOL)
	{
		switch (pcsSkill->m_pcsBase->m_eType) {
		case APBASE_TYPE_CHARACTER:
			{
				// template의 ARG1이 필요 아이템의 IID
				INT32 lItemTID = pcsTemplate->m_stConditionArg[0].lArg1;
				AgpdItem *pcsAgpdItem = m_pagpmItem->GetInventoryItemByTID( (AgpdCharacter *)pcsSkill->m_pcsBase, lItemTID );
				if( pcsAgpdItem == NULL )
					return FALSE;
			}
			break;
		}
	}

	return TRUE;
}

BOOL AgpmSkill::IsPassiveSkill(AgpdSkill *pcsSkill)
{
	if (!pcsSkill)
		return FALSE;

	return IsPassiveSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate);
}

BOOL AgpmSkill::IsPassiveSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_PASSIVE)
		return TRUE;

	return FALSE;
}

// 2005.07.15. steeple
BOOL AgpmSkill::IsActionPassiveSkill(AgpdSkill* pcsSkill)
{
	if(!pcsSkill)
		return FALSE;

	if(IsPassiveSkill(pcsSkill) &&
		((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate)->m_lEffectType2[0] & AGPMSKILL_EFFECT2_ACTION_PASSIVE)
		return TRUE;

	return FALSE;
}

//JK_패시브스킬중 크리티컬 관련 UI 표시 안되는 현상수정
//크리티컬을 올려주는 패시브 스킬인가?
BOOL AgpmSkill::IsCriticalUpPassiveSkill(AgpdSkill* pcsSkill)
{
	if(!pcsSkill)
		return FALSE;

	if(IsPassiveSkill(pcsSkill) &&
		((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate)->m_lEffectType[0] & AGPMSKILL_EFFECT_MELEE_ATTACK_CRITICAL)
		return TRUE;

	return FALSE;
}


// 2007.05.07. steeple
BOOL AgpmSkill::IsProductSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	return pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_PRODUCT ? TRUE : FALSE ;
}

BOOL AgpmSkill::IsNormalAttack(AgpdSkill* pcsSKill )
{
	if(!pcsSKill)
		return FALSE;

	AgpdSkillTemplate*		pcsSkillTemplate		=	static_cast< AgpdSkillTemplate* >(pcsSKill->m_pcsTemplate);

	if( pcsSkillTemplate->m_bIsAutoAttack )
		return TRUE;

	return FALSE;
}

BOOL AgpmSkill::IsActionPassiveSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(IsPassiveSkill(pcsSkillTemplate) &&
		pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_ACTION_PASSIVE)
		return TRUE;

	return FALSE;
}

//	get skill const factor functions... (여기부터)
///////////////////////////////////////////////////////////////////////////////

UINT32 AgpmSkill::GetCastDelay(AgpdSkill *pcsSkill)
{
	if (!pcsSkill || !pcsSkill->m_pcsTemplate)
		return 0;

	AgpdSkillTemplate	*pcsSkillTemplate =  (AgpdSkillTemplate *) pcsSkill->m_pcsTemplate;
	if (!pcsSkillTemplate)
		return 0;

	UINT32	ulCastDelay	= 0;

	if (pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_PRODUCT)
	{
		ulCastDelay	= 0;
	}
	else
	{
		INT32	lSkillLevel	= 0;
		m_pagpmFactors->GetValue(&pcsSkill->m_csFactor, &lSkillLevel, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_SKILL_LEVEL);

		ulCastDelay = GetCastDelay(pcsSkillTemplate, lSkillLevel);

		// passive skill에 의해 변화된 값 반영
		AgpdSkillAttachData	*pcsAttachData	= GetAttachSkillData(pcsSkill->m_pcsBase);
		if (pcsAttachData)
		{
			if (pcsAttachData->m_ullBuffedSkillFactorEffect & AGPMSKILL_EFFECT_SKILL_FACTOR_ADJUST_CAST_TIME)
			{
				/*
				INT32	lRandom	= m_csRandom.randInt(100);
				if (lRandom <= pcsAttachData->m_stBuffedSkillFactorEffectArg.lCastTimeProbability)
				{
					ulCastDelay = (UINT32) (ulCastDelay * (pcsAttachData->m_stBuffedSkillFactorEffectArg.lCastTime / 100.0));
				}
				*/

				ulCastDelay = (UINT32) (ulCastDelay * (100 + pcsAttachData->m_stBuffedSkillFactorEffectArg.lCastTime) / 100.0f);
			}
		}
	}

	return ulCastDelay;
}

UINT32 AgpmSkill::GetCastDelay(AgpdSkillTemplate *pcsSkillTemplate, INT32 lSkillLevel)
{
	if (!pcsSkillTemplate || lSkillLevel < 1 || lSkillLevel >= AGPMSKILL_MAX_SKILL_CAP)
		return 0;

	return (UINT32) pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_CAST_TIME][lSkillLevel];
}

// 2005.11.27. steeple
INT32 AgpmSkill::GetAdjustCastDelay(AgpdCharacter* pcsCharacter)
{
	if(!pcsCharacter)
		return 0;

	AgpdSkillAttachData* pcsAttachData = GetAttachSkillData((ApBase*)pcsCharacter);
	if(!pcsAttachData)
		return 0;

	INT32 lValue = 0;
	m_pagpmFactors->GetValue(&pcsCharacter->m_csFactor, &lValue, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_SKILL_CAST);

	if(lValue > 95)
		lValue = 95;

	return -(lValue);
}

UINT32 AgpmSkill::GetRecastDelay(AgpdSkill *pcsSkill)
{
	if (!pcsSkill)
		return 0;

	AgpdSkillTemplate	*pcsSkillTemplate =  (AgpdSkillTemplate *) pcsSkill->m_pcsTemplate;
	if (!pcsSkillTemplate)
		return 0;

	if (pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_PRODUCT)
	{
		return (UINT32) pcsSkillTemplate->m_stConditionArg[0].lArg4;
	}
	else
	{
		INT32	lSkillLevel	= 0;
		m_pagpmFactors->GetValue(&pcsSkill->m_csFactor, &lSkillLevel, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_SKILL_LEVEL);

		return GetRecastDelay(pcsSkillTemplate, lSkillLevel);
	}

	return 0;
}

UINT32 AgpmSkill::GetRecastDelay(AgpdSkillTemplate *pcsSkillTemplate, INT32 lSkillLevel)
{
	if (!pcsSkillTemplate || lSkillLevel < 1 || lSkillLevel >= AGPMSKILL_MAX_SKILL_CAP)
		return 0;

	//return (UINT32) pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_RECAST_TIME][lSkillLevel];

	//if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_RECAST_TIME][lSkillLevel] <
	//	(pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_CAST_TIME][lSkillLevel] + 
	//	 pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_DURATION][lSkillLevel]))
	//	 return 0;

	//return (UINT32)
	//	(pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_RECAST_TIME][lSkillLevel]
	//	- pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_CAST_TIME][lSkillLevel]
	//	- pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_DURATION][lSkillLevel]);

	// 2005.01.23. steeple
	//return (UINT32)(pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_RECAST_TIME][lSkillLevel] +
	//		pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_CAST_TIME][lSkillLevel]);

	// 2005.01.25. steeple
	return (UINT32)pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_RECAST_TIME][lSkillLevel];
}

// 2005.11.27. steeple
INT32 AgpmSkill::GetAdjustRecastDelay(AgpdCharacter* pcsCharacter)
{
	if(!pcsCharacter)
		return 0;

	AgpdSkillAttachData* pcsAttachData = GetAttachSkillData((ApBase*)pcsCharacter);
	if(!pcsAttachData)
		return 0;

	INT32 lValue = 0;
	m_pagpmFactors->GetValue(&pcsCharacter->m_csFactor, &lValue, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_SKILL_DELAY);

	if(lValue > 95)
		lValue = 95;

	return -(lValue);
}


UINT32 AgpmSkill::GetSemiRecastDelay(AgpdSkill *pcsSkill)
{
	if (!pcsSkill)
		return 0;

	AgpdSkillTemplate	*pcsSkillTemplate =  (AgpdSkillTemplate *) pcsSkill->m_pcsTemplate;
	if (!pcsSkillTemplate)
		return 0;

	if (pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_PRODUCT)
		return 0;

	INT32	lSkillLevel	= 0;
	m_pagpmFactors->GetValue(&pcsSkill->m_csFactor, &lSkillLevel, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_SKILL_LEVEL);

	return (UINT32) pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SEMI_RECAST_TIME][lSkillLevel];
}

UINT32 AgpmSkill::GetSkillDurationTime(AgpdSkill *pcsSkill, INT32 lSkillLevel)
{
	if (!pcsSkill || !pcsSkill->m_pcsTemplate)
		return 0;

	AgpdSkillTemplate	*pcsSkillTemplate =  (AgpdSkillTemplate *) pcsSkill->m_pcsTemplate;

	if (pcsSkillTemplate->m_bShrineSkill ||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_PRODUCT)
		return 0;

	UINT32 ulClockCount = GetClockCount();
	UINT32 ulCastDelay = GetCastDelay(pcsSkillTemplate, lSkillLevel);

	//// 2005.07.18. steeple
	//// 스킬 지속시간이 거리에 따라 결정되는 거라면
	//if(IsDurationByDistanceSkill(pcsSkillTemplate))
	//	return 0xFFFFFFFF - ulClockCount - ulCastDelay;	// 조낸 큰 수로 리턴.
	//// 2005.09.04. steeple
	//// 스킬 지속시간이 무한대인 놈이라면
	//else if(IsDurationUnlimited(pcsSkillTemplate))
	//	return 0xFFFFFFFF;	// 역시 조낸 큰 수로 리턴.

	// 2005.01.23. steeple
	if(lSkillLevel == 0)
		m_pagpmFactors->GetValue(&pcsSkill->m_csFactor, &lSkillLevel, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_SKILL_LEVEL);

	INT32	lDuration	= 0;
	m_pagpmFactors->GetValue(&pcsSkill->m_csFactor, &lDuration, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_DURATION);

	INT32	lDurationTime = (INT32) pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_DURATION][lSkillLevel];

	// skill의 타입에 따라 DIRT중 Duration 값으로 duration time 을 계산해낸다.
	if (pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_PHYSICAL_DMG		||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_MAGIC	||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_FIRE	||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_EARTH	||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_AIR		||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_WATER)
	{
		// 10%씩 상승
		lDurationTime += (INT32) (lDurationTime * 0.1) * lDuration;
	}
	else if (pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_BUFF)
	{
		// 20%씩 상승
		lDurationTime += (INT32) (lDurationTime * 0.2) * lDuration;
	}
	else if (pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_DEBUFF)
	{
		// 10%씩 상승
		lDurationTime += (INT32) (lDurationTime * 0.1) * lDuration;
	}

	// specialized 에 따른 변화된 값 반영
//	lDurationTime	= GetSpecializedDuration(pcsSkill, lDurationTime);

	// 2005.07.09. steeple
	// 버프 또는 디버프 스킬의 지속 시간을 늘려준다.
	if(pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_BUFF ||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_DEBUFF)
	{
		if(pcsSkill->m_pcsBase)
		{
			AgpdSkillAttachData* pcsAttachData = GetAttachSkillData(pcsSkill->m_pcsBase);
			if(pcsAttachData)
			{
				// 아래 단위는 % 이므로 이렇게 해줘야 한다. 2005.09.19. steeple
				lDurationTime += (INT32)((FLOAT)(lDurationTime * pcsAttachData->m_stBuffedSkillFactorEffectArg.lDurationTime) / 100.0f);
			}
		}
	}

	// 2012. 01. 09. silvermoo
	// 디버프 스킬이라면 다른 패시브/버프 스킬에 의해 스킬지속시간이 변경되는 경우, 확인하여 적용한다
	if (pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_DEBUFF)
	{
		lDurationTime += GetModifiedSkillDurationTime(pcsSkill);
	}

	// 2006.11.14. steeple
	// DOT 스킬이라면 Interval 만큼 더해준다.
	if(IsDOTSkill(pcsSkillTemplate))
		lDurationTime += GetSkillInterval(pcsSkill, lSkillLevel);

	return (UINT32) lDurationTime;
}

UINT32 AgpmSkill::GetSkillInterval(AgpdSkill *pcsSkill, INT32 lSkillLevel)
{
	if (!pcsSkill)
		return 0;

	AgpdSkillTemplate	*pcsSkillTemplate =  (AgpdSkillTemplate *) pcsSkill->m_pcsTemplate;
	if (!pcsSkillTemplate)
		return 0;

	if (pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_PRODUCT)
		return 0;

	if(lSkillLevel == 0)
		m_pagpmFactors->GetValue(&pcsSkill->m_csFactor, &lSkillLevel, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_SKILL_LEVEL);

	if(((UINT32) pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_DOT_DAMAGE_TIME][lSkillLevel]) != 0)
		return (UINT32) pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_DOT_DAMAGE_TIME][lSkillLevel];
	else
		return (UINT32) pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_INTERVAL][lSkillLevel];
}

// // 2012. 01. 09. silvermoo
// 디버프 스킬이라면 다른 패시브/버프 스킬에 의해 스킬지속시간이 변경되는 경우, 변경된 시간을 계산해준다
INT32	AgpmSkill::GetModifiedSkillDurationTime(AgpdSkill *pcsSkill)
{
	if (!pcsSkill)
		return FALSE;

	if ( FALSE == IsDebuffSkill((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate) )
		return FALSE;

	AgpdCharacter* pcsTarget				 = m_pagpmCharacter->GetCharacter(pcsSkill->m_csTargetBase.m_lID);
	AgpdSkillAttachData* pcsAttachData		 = GetAttachSkillData(pcsSkill->m_pcsBase);
	AgpdSkillAttachData* pcsAttachDataTarget = GetAttachSkillData((ApBase*)pcsTarget);
	if (!pcsTarget || !pcsAttachData || !pcsAttachDataTarget)
		return FALSE;

	INT32 lDurationTime = 0;

	// 시전자의 패시브 스킬을 확인한다
	for (int i = 0; i < AGPMSKILL_MAX_SKILL_USE; ++i)
	{
		if (pcsAttachData->m_alUsePassiveSkillID[i] == AP_INVALID_SKILLID)
			break;

		AgpdSkill* pcsPassiveSkill						= pcsAttachData->m_apcsUsePassiveSkill[i];
		if (!pcsPassiveSkill)
			continue;

		AgpdSkillTemplate* pcsSkillTemplatePassive		= (AgpdSkillTemplate *) pcsPassiveSkill->m_pcsTemplate;
		if (!pcsSkillTemplatePassive)
			continue;

		if( pcsSkillTemplatePassive->m_lEffectType[0] & AGPMSKILL_EFFECT_SKILL_FACTOR_ADJUST_MODIFIED_SKILL_DURATION )
		{
			INT32 lSkillLevel = GetSkillLevel(pcsPassiveSkill);
			INT32 lAdjustTime = (INT32) pcsSkillTemplatePassive->m_fUsedConstFactor[AGPMSKILL_CONST_TIME_CONTROL][lSkillLevel];
			if (lAdjustTime > 0)
			{
				lDurationTime += lAdjustTime * 1000;
			}
		}
	}

	// 타겟의 패시브 스킬을 확인한다
	for (int i = 0; i < AGPMSKILL_MAX_SKILL_USE; ++i)
	{
		if (pcsAttachDataTarget->m_alUsePassiveSkillID[i] == AP_INVALID_SKILLID)
			break;

		AgpdSkill* pcsPassiveSkill						= pcsAttachDataTarget->m_apcsUsePassiveSkill[i];
		if (!pcsPassiveSkill)
			continue;

		AgpdSkillTemplate* pcsSkillTemplatePassive		= (AgpdSkillTemplate *) pcsPassiveSkill->m_pcsTemplate;
		if (!pcsSkillTemplatePassive)
			continue;

		if( pcsSkillTemplatePassive->m_lEffectType[0] & AGPMSKILL_EFFECT_SKILL_FACTOR_ADJUST_MODIFIED_SKILL_DURATION )
		{
			INT32 lSkillLevel = GetSkillLevel(pcsPassiveSkill);
			INT32 lAdjustTime = (INT32) pcsSkillTemplatePassive->m_fUsedConstFactor[AGPMSKILL_CONST_TIME_CONTROL][lSkillLevel];
			if (lAdjustTime < 0)
			{
				lDurationTime += lAdjustTime * 1000;
			}
			
		}
	}

	// 시전자의 버프 스킬을 확인한다
	for(int i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
	{
		if(pcsAttachData->m_astBuffSkillList[i].lSkillID == AP_INVALID_SKILLID)
			break;

		if(!pcsAttachData->m_astBuffSkillList[i].pcsSkillTemplate)
			continue;

		if (pcsAttachData->m_astBuffSkillList[i].pcsSkillTemplate->m_lEffectType[0] & AGPMSKILL_EFFECT_SKILL_FACTOR_ADJUST_MODIFIED_SKILL_DURATION)
		{
			INT32 lSkillLevel = GetSkillLevel(GetSkill(pcsAttachData->m_astBuffSkillList[i].lSkillID));
			INT32 lAdjustTime = (INT32) pcsAttachData->m_astBuffSkillList[i].pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_TIME_CONTROL][lSkillLevel];
			if (lAdjustTime > 0)
			{
				lDurationTime += lAdjustTime * 1000;
			}
		}
	}

	// 타겟의 버프 스킬을 확인한다
	for(int i = 0; i < AGPMSKILL_MAX_SKILL_BUFF; ++i)
	{
		if(pcsAttachDataTarget->m_astBuffSkillList[i].lSkillID == AP_INVALID_SKILLID)
			break;

		if(!pcsAttachDataTarget->m_astBuffSkillList[i].pcsSkillTemplate)
			continue;

		if (pcsAttachDataTarget->m_astBuffSkillList[i].pcsSkillTemplate->m_lEffectType[0] & AGPMSKILL_EFFECT_SKILL_FACTOR_ADJUST_MODIFIED_SKILL_DURATION)
		{
			INT32 lSkillLevel = GetSkillLevel(GetSkill(pcsAttachDataTarget->m_astBuffSkillList[i].lSkillID));
			INT32 lAdjustTime = (INT32) pcsAttachDataTarget->m_astBuffSkillList[i].pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_TIME_CONTROL][lSkillLevel];
			if (lAdjustTime < 0)
			{
				lDurationTime += lAdjustTime * 1000;
			}
		}
	}

	return lDurationTime;
}

INT32 AgpmSkill::GetRange(AgpdSkill *pcsSkill)
{
	if (!pcsSkill)
		return 0;

	AgpdSkillTemplate	*pcsSkillTemplate =  (AgpdSkillTemplate *) pcsSkill->m_pcsTemplate;
	if (!pcsSkillTemplate)
		return 0;

	INT32	lRange	= 0;

	if (pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_PRODUCT)
	{
		lRange	= (INT32) pcsSkillTemplate->m_stConditionArg[0].lArg3;
	}
	else
	{
		INT32 lSkillLevel = 0;
		EnumCallback(AGPMSKILL_CB_GET_MODIFIED_SKILL_LEVEL, pcsSkill, &lSkillLevel);

		if(!lSkillLevel)
			return 0;

		INT32	lSkillRange	= 0;
				
		//m_pagpmFactors->GetValue(&pcsSkill->m_csFactor, &lSkillLevel, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_SKILL_LEVEL);
		m_pagpmFactors->GetValue(&pcsSkill->m_csFactor, &lSkillRange, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_RANGE);

		if(pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_BOX)
			lRange = (INT32) pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_TARGET_AREA_F1][lSkillLevel];
		else
			lRange = (INT32) pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_RANGE][lSkillLevel];

		if (lRange <= 0)
		{
			if (!(pcsSkillTemplate->m_lRangeType & AGPMSKILL_CHECK_RANGE_NOT_CHECK))
			{
				// 2005.01.18. steeple 주석 처리.
				//if (pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_BOX_ONLY_ENEMY_UNITS)
				//{
				//	// 직선 범위 공격인 경우 range가 0이라면 직선 거리를 range로 사용한다.
				//	lRange	= (INT32) pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_TARGET_AREA_F1][lSkillLevel];
				//}
				//else
				//{
					if (pcsSkill->m_pcsBase && pcsSkill->m_pcsBase->m_eType == APBASE_TYPE_CHARACTER)
					{
						// 현재 캐릭터의 공격 가능 거리를 lRange로 사용한다.
						// attack range를 가져온다.
						lRange	= 0;
						m_pagpmFactors->GetValue(&((AgpdCharacter *) pcsSkill->m_pcsBase)->m_csFactor, &lRange, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_ATTACKRANGE);
					}
				//}
			}
		}

		if (lRange > 0)
		{
			/*
			if (pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_PHYSICAL_DMG		||
				pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_MAGIC	||
				pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_FIRE	||
				pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_EARTH	||
				pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_AIR		||
				pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_WATER)
			{
				// 10%씩 상승
				lRange += (INT32) (lRange * 0.1) * lSkillRange;
			}
			else if (pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_BUFF)
			{
				// 20%씩 상승
				lRange += (INT32) (lRange * 0.2) * lSkillRange;
			}
			else if (pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_DEBUFF)
			{
				// 10%씩 상승
				lRange += (INT32) (lRange * 0.1) * lSkillRange;
			}

			// specialized 에 따른 변화된 값 반영
	//		lRange	= GetSpecializedDistance(pcsSkill, lRange);
			*/

			// passive skill에 의해 변화된 값 반영
			AgpdSkillAttachData	*pcsAttachData	= GetAttachSkillData(pcsSkill->m_pcsBase);
			if (pcsAttachData)
			{
				if (pcsAttachData->m_ullBuffedSkillFactorEffect & AGPMSKILL_EFFECT_SKILL_FACTOR_ADJUST_RANGE)
				{
					/*
					INT32	lRandom	= m_csRandom.randInt(100);
					if (lRandom <= pcsAttachData->m_stBuffedSkillFactorEffectArg.lMagicSkillRangeProbability)
					{
						lRange = (INT32) (lRange * (pcsAttachData->m_stBuffedSkillFactorEffectArg.lMagicSkillRange / 100.0));
					}
					*/

					// 2005.01.23. steeple. Skill Range 값 Percent 에서 Point 로 변경
					lRange	+= pcsAttachData->m_stBuffedSkillFactorEffectArg.lRangeAdjustPercent * 100;
					//lRange	= (INT32) (lRange * (pcsAttachData->m_stBuffedSkillFactorEffectArg.lRangeAdjustPercent + 100) / 100.0);
				}
			}
		}
	}

	return lRange;
}

INT32 AgpmSkill::GetTargetArea1(AgpdSkill *pcsSkill)
{
	if (!pcsSkill)
		return 0;

	AgpdSkillTemplate	*pcsSkillTemplate =  (AgpdSkillTemplate *) pcsSkill->m_pcsTemplate;
	if (!pcsSkillTemplate)
		return 0;

	if (pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_PRODUCT)
		return 0;

	INT32	lSkillLevel		= 0;
	INT32	lSkillTarget	= 0;

	m_pagpmFactors->GetValue(&pcsSkill->m_csFactor, &lSkillLevel, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_SKILL_LEVEL);
	m_pagpmFactors->GetValue(&pcsSkill->m_csFactor, &lSkillTarget, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_TARGET);

	INT32	lArea1 = (INT32) pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_TARGET_AREA_R][lSkillLevel];
	lArea1 += (INT32) pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_TARGET_AREA_F1][lSkillLevel];

	if (pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_PHYSICAL_DMG		||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_MAGIC	||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_FIRE	||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_EARTH	||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_AIR		||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_WATER)
	{
		if (pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_SPHERE)
			// 10%씩 상승
			lArea1 += (INT32) (lArea1 * 0.03) * lSkillTarget;
	}
	else if (pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_BUFF)
	{
		if (pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_SPHERE)
			// 20%씩 상승
			lArea1 += (INT32) (lArea1 * 0.2) * lSkillTarget;
	}
	else if (pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_DEBUFF)
	{
		if (pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_SPHERE)
			// 10%씩 상승
			lArea1 += (INT32) (lArea1 * 0.1) * lSkillTarget;
	}

	// specialized 에 따른 변화된 값 반영
//	lArea1	= GetSpecializedArea(pcsSkill, lArea1);

	return lArea1;
}

INT32 AgpmSkill::GetTargetArea2(AgpdSkill *pcsSkill)
{
	if (!pcsSkill)
		return 0;

	AgpdSkillTemplate	*pcsSkillTemplate =  (AgpdSkillTemplate *) pcsSkill->m_pcsTemplate;
	if (!pcsSkillTemplate)
		return 0;

	if (pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_PRODUCT)
		return 0;

	INT32	lSkillLevel		= 0;
	INT32	lSkillTarget	= 0;

	m_pagpmFactors->GetValue(&pcsSkill->m_csFactor, &lSkillLevel, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_SKILL_LEVEL);
	m_pagpmFactors->GetValue(&pcsSkill->m_csFactor, &lSkillTarget, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_TARGET);

	INT32	lArea2 = (INT32) pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_TARGET_AREA_F2][lSkillLevel];

	if (pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_PHYSICAL_DMG		||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_MAGIC	||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_FIRE	||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_EARTH	||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_AIR		||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_WATER)
	{
		if (pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_BOX)
			// 10%씩 상승
			lArea2 += (INT32) (lArea2 * 0.1) * lSkillTarget;
	}

	// specialized 에 따른 변화된 값 반영
//	lArea2	= GetSpecializedArea(pcsSkill, lArea2);

	return lArea2;
}

//INT32 AgpmSkill::GetRemainSkillPointToArriveMaxLevel(AgpdSkill *pcsSkill, INT32 lPoint)
//{
//	if (!pcsSkill)
//		return 0;
//
//	INT32	lSkillPoint = GetSkillPoint(pcsSkill);
//	if (lSkillPoint < 0)
//		return 0;
//
//	INT32	lRemainSkillPoint = GetMaxInputSkillPoint(pcsSkill) - lSkillPoint;
//
//	if (lPoint <= lRemainSkillPoint)
//		return lPoint;
//
//	/*
//	for (int i = lSkillLevel + 1; i < AGPMSKILL_MAX_SKILL_CAP; ++i)
//	{
//		lRemainSkillPoint += i;
//		if (lPoint <= lRemainSkillPoint)
//			return lPoint;
//	}
//	*/
//
//	return lRemainSkillPoint;
//}

INT32 AgpmSkill::GetMaxInputSkillPoint(AgpdSkill *pcsSkill)
{
	if (!pcsSkill || !pcsSkill->m_pcsBase)
		return 0;

//	if (pcsSkill->m_lMaxInputSkillPoint > 0)
//		return pcsSkill->m_lMaxInputSkillPoint;
//
//	AgpdSkillAttachData	*pcsAttachData	= GetAttachSkillData(pcsSkill->m_pcsBase);
//	if (!pcsAttachData)
//		return 0;
//
//	for (int i = 0; i < AGPMSKILL_MAX_MASTERY_SKILL; ++i)
//	{
//		if (pcsAttachData->m_csMasteryTemplate.m_csMastery[pcsSkill->m_lMasteryIndex].m_csSkillTree[i][0].m_lSkillTID == 0)
//			continue;
//
//		for (int j = 0; j < AGPMSKILL_MAX_MASTERY_TREE_DEPTH; ++j)
//		{
//			if (pcsAttachData->m_csMasteryTemplate.m_csMastery[pcsSkill->m_lMasteryIndex].m_csSkillTree[i][0].m_lSkillTID == 0)
//				break;
//
//			if (pcsAttachData->m_csMasteryTemplate.m_csMastery[pcsSkill->m_lMasteryIndex].m_csSkillTree[i][j].m_lSkillID == pcsSkill->m_lID)
//			{
//				pcsSkill->m_lMaxInputSkillPoint	= pcsAttachData->m_csMasteryTemplate.m_csMastery[pcsSkill->m_lMasteryIndex].m_csSkillTree[i][j].m_ucMaxInputSP;
//
//				return pcsSkill->m_lMaxInputSkillPoint;
//			}
//		}
//	}

	return pcsSkill->m_lMaxInputSkillPoint;
}

BOOL AgpmSkill::AddSkillPoint(AgpdSkill *pcsSkill, INT32 lPoint)
{
	if (!pcsSkill || !pcsSkill->m_pcsBase || lPoint < 1)
		return FALSE;

	// 이 스킬이 더이상 올라갈데가 없는지 검사한다.
//	INT32	lAddPoint = GetRemainSkillPointToArriveMaxLevel(pcsSkill, lPoint);
//
//	if (lAddPoint < 1)
//		return FALSE;

	// 먼저 pcsSkill 의 owner가 lPoint만큼의 skill point를 가지고 있는지 검사한다.
	// owner의 skill point에서 lPoint 만큼을 뺀다.

	BOOL	bUpdateOwner = FALSE;

	switch (pcsSkill->m_pcsBase->m_eType) {
	case APBASE_TYPE_CHARACTER:
		{
			INT32 lSkillPoint = m_pagpmCharacter->GetSkillPoint((AgpdCharacter *) pcsSkill->m_pcsBase);
			if (lSkillPoint < lPoint)
				return FALSE;

			/*
			if (!m_pagpmCharacter->UpdateSkillPoint((AgpdCharacter *) pcsSkill->m_pcsBase, -lAddPoint))
				return FALSE;
			*/

			bUpdateOwner = TRUE;
		}
		break;
	}

	if (!bUpdateOwner)
		return FALSE;

	// owner의 skill point를 뺐으니.. 이제 여기서 skill의 skill point를 올린다. 올린 후.. skill level을 검사한다.
	return UpdateSkillPoint(pcsSkill, lPoint);
}

BOOL AgpmSkill::SubtractSkillPoint(AgpdSkill *pcsSkill, INT32 lPoint)
{
	if (!pcsSkill || !pcsSkill->m_pcsBase || lPoint < 1)
		return FALSE;

	// 이 스킬에서 뺄 포인트가 있는지 검사한다.
	INT32	lSkillPoint = GetSkillPoint(pcsSkill);
	if (lSkillPoint < 1)
		return FALSE;

	// skill 에서 SP를 뺀다.
	if (!UpdateSkillPoint(pcsSkill, (-lPoint)))
		return FALSE;

	/*
	// 이제 owner의 SP에 skill에서 뺀 SP를 더해준다.
	switch (pcsSkill->m_pcsBase->m_eType) {
	case APBASE_TYPE_CHARACTER:
		{
			if (!m_pagpmCharacter->UpdateSkillPoint((AgpdCharacter *) pcsSkill->m_pcsBase, lPoint))
				return FALSE;
		}
		break;
	}
	*/

	return TRUE;
}

BOOL AgpmSkill::UpdateSkillPoint(AgpdSkill *pcsSkill, INT32 lPoint)
{
	if (!pcsSkill || lPoint == 0)
		return FALSE;

	//INT32	lUpdatePoint = lPoint;

	INT32	lSkillLevel	= 0;

	EnumCallback(AGPMSKILL_CB_GET_MODIFIED_SKILL_LEVEL, pcsSkill, &lSkillLevel);

	if (lSkillLevel < 0 /*|| (lPoint + lSkillLevel) > GetMaxInputSkillPoint(pcsSkill)*/)
		return FALSE;

	//if (lPoint > 0)
	//{
		//for ( ; ; )
		//{
			// skill point를 update 하구, skill level을 검사해 update 한당.
			INT32	lSkillPoint	= GetSkillPoint(pcsSkill);

			if (lPoint < 0)
			{
				if (lSkillPoint < 1)
					return FALSE;
			}
			else
			{
				if (lSkillPoint < 0 /*|| lSkillPoint >= GetMaxInputSkillPoint(pcsSkill)*/)
					return FALSE;
			}

			lSkillPoint += lPoint;

			/*
			if (lSkillPoint < 0)
				lSkillPoint = 0;
			if (lSkillPoint > GetMaxInputSkillPoint(pcsSkill))
				lSkillPoint = GetMaxInputSkillPoint(pcsSkill);
			*/

			//INT32	lNewSkillLevel = lSkillPoint / 2 + lSkillPoint % 2;
			INT32	lNewSkillLevel = lSkillPoint;

			AgpdFactorDIRT *pcsFactorDIRT = (AgpdFactorDIRT *) m_pagpmFactors->SetFactor(&pcsSkill->m_csFactor, NULL, AGPD_FACTORS_TYPE_DIRT);
			if (!pcsFactorDIRT)
				return FALSE;

			m_pagpmFactors->SetValue(&pcsSkill->m_csFactor, lNewSkillLevel, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_SKILL_LEVEL);
			m_pagpmFactors->SetValue(&pcsSkill->m_csFactor, lSkillPoint, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_SKILL_POINT);

			//lUpdatePoint -= lAddPoint;
			
			//if (lUpdatePoint < 1)
			//	break;
		//}
	//}
	//else
	//{
		// 요긴 스킬에서 스킬포인트를 빼는 경우이다. 음냐...
		//
		//		위 코드와 중복되는 부분이다. 위 코드에서 빼는 부분도 해결 가능하다.
		//
	//}

	return EnumCallback(AGPMSKILL_CB_UPDATE_SKILL_POINT, pcsSkill, &lPoint);
}

BOOL AgpmSkill::UpdateSkillLevelAndExp(AgpdSkill *pcsSkill, INT32 lLevel, INT32 lExp)
{
	if (!pcsSkill)
		return FALSE;

	AgpdFactorDIRT *pcsFactorDIRT = (AgpdFactorDIRT *) m_pagpmFactors->SetFactor(&pcsSkill->m_csFactor, NULL, AGPD_FACTORS_TYPE_DIRT);
	if (!pcsFactorDIRT)
		return FALSE;

	m_pagpmFactors->SetValue(&pcsSkill->m_csFactor, lExp, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_SKILL_EXP);
	m_pagpmFactors->SetValue(&pcsSkill->m_csFactor, lLevel, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_SKILL_LEVEL);

	return EnumCallback(AGPMSKILL_CB_UPDATE_SKILL_POINT, pcsSkill, &lExp);
}

BOOL AgpmSkill::UpdateDIRT(AgpdSkill *pcsSkill, AgpdFactor *pcsFactor)
{
	if (!pcsSkill || !pcsFactor)
		return FALSE;

	// pcsFactor에 있는 AgpdFactorDIRT 값으로 DIRT 값을 업데이뚜 한당
	//

	// 제대로 된 넘인지 검사한다.
	//		- pcsSkill 이 사용하는 DIRT로만 이루어졌는지 검사한다.

	INT32	lDuration	= 0;
	INT32	lIntensity	= 0;
	INT32	lRange		= 0;
	INT32	lTarget		= 0;

	m_pagpmFactors->GetValue(pcsFactor, &lDuration, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_DURATION);
	m_pagpmFactors->GetValue(pcsFactor, &lIntensity, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_INTENSITY);
	m_pagpmFactors->GetValue(pcsFactor, &lRange, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_RANGE);
	m_pagpmFactors->GetValue(pcsFactor, &lTarget, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_TARGET);

	if (lDuration > 0 && 
		((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stUseDIRTPoint.nDuration == 0)
		return FALSE;
	if (lIntensity > 0 && 
		((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stUseDIRTPoint.nIntensity == 0)
		return FALSE;
	if (lRange > 0 && 
		((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stUseDIRTPoint.nRange == 0)
		return FALSE;
	if (lTarget > 0 && 
		((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_stUseDIRTPoint.nTarget == 0)
		return FALSE;

	// 총합이 100%인지 검사한다.
	INT32	lTotalPoint = lDuration + lIntensity + lRange + lTarget;

	if (lTotalPoint != 100)
		return FALSE;

	m_pagpmFactors->SetValue(&pcsSkill->m_csFactor, lDuration, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_DURATION);
	m_pagpmFactors->SetValue(&pcsSkill->m_csFactor, lIntensity, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_INTENSITY);
	m_pagpmFactors->SetValue(&pcsSkill->m_csFactor, lRange, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_RANGE);
	m_pagpmFactors->SetValue(&pcsSkill->m_csFactor, lTarget, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_TARGET);

	return EnumCallback(AGPMSKILL_CB_UPDATE_DIRT_POINT, pcsSkill, NULL);
}

INT32 AgpmSkill::GetSkillLevel(AgpdSkill *pcsSkill)
{
	if (!pcsSkill || !pcsSkill->m_pcsTemplate)
		return 0;
	
	INT32	lSkillLevel	= 0;

	m_pagpmFactors->GetValue(&pcsSkill->m_csFactor, &lSkillLevel, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_SKILL_LEVEL);
	
	if (((AgpdSkillTemplate*) pcsSkill->m_pcsTemplate)->m_nAttribute & AGPMSKILL_SKILL_ATTR_PRODUCT)
	{
		if (lSkillLevel < 1)
		{
			/*
			char strCharBuff[256] = { 0, };
			sprintf_s(strCharBuff, sizeof(strCharBuff), "[%s][%d] : %s(%s)", __FUNCTION__, __LINE__,
					((AgpdCharacter*)pcsSkill->m_pcsBase)->m_szID, ((AgpdSkillTemplate*) pcsSkill->m_pcsTemplate)->m_szName);
			AuLogFile_s("LOG\\SkillLevelError", strCharBuff);
			*/

			return 0;
		}
		else if (lSkillLevel > 100)
		{
			/*
			char strCharBuff[256] = { 0, };
			sprintf_s(strCharBuff, sizeof(strCharBuff), "[%s][%d] : %s(%s)", __FUNCTION__, __LINE__,
				((AgpdCharacter*)pcsSkill->m_pcsBase)->m_szID, ((AgpdSkillTemplate*) pcsSkill->m_pcsTemplate)->m_szName);
			AuLogFile_s("LOG\\SkillLevelError", strCharBuff);
			*/
			
			return 100;
		}
	}
	else
	{
		if (lSkillLevel < 1)
		{
			/*
			char strCharBuff[256] = { 0, };
			sprintf_s(strCharBuff, sizeof(strCharBuff), "[%s][%d] : %s(%s)", __FUNCTION__, __LINE__,
					((AgpdCharacter*)pcsSkill->m_pcsBase)->m_szID, ((AgpdSkillTemplate*) pcsSkill->m_pcsTemplate)->m_szName);
			AuLogFile_s("LOG\\SkillLevelError", strCharBuff);
			*/

			return 0;
		}
		else if(lSkillLevel >= AGPMSKILL_MAX_SKILL_CAP)
		{
			/*
			char strCharBuff[256] = { 0, };
			sprintf_s(strCharBuff, sizeof(strCharBuff), "[%s][%d] : %s(%s)", __FUNCTION__, __LINE__,
					((AgpdCharacter*)pcsSkill->m_pcsBase)->m_szID, ((AgpdSkillTemplate*) pcsSkill->m_pcsTemplate)->m_szName);
			AuLogFile_s("LOG\\SkillLevelError", strCharBuff);
			*/

			return (AGPMSKILL_MAX_SKILL_CAP - 1);
		}
	}

	return lSkillLevel;
}


INT32 AgpmSkill::GetSkillPoint(AgpdSkill *pcsSkill)
{
	if (!pcsSkill)
		return 0;

	INT32	lSkillPoint	= 0;

	m_pagpmFactors->GetValue(&pcsSkill->m_csFactor, &lSkillPoint, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_SKILL_POINT);

	return lSkillPoint;
}

INT32 AgpmSkill::GetSkillExp(AgpdSkill *pcsSkill)
{
	if (!pcsSkill)
		return 0;

	INT32	lSkillExp	= 0;

	m_pagpmFactors->GetValue(&pcsSkill->m_csFactor, &lSkillExp, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_SKILL_EXP);

	return lSkillExp;
}

BOOL AgpmSkill::SetSkillPoint(AgpdSkill *pcsSkill, INT32 lSkillPoint)
{
	if (!pcsSkill || lSkillPoint < 0)
		return FALSE;

	m_pagpmFactors->SetValue(&pcsSkill->m_csFactor, lSkillPoint, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_SKILL_POINT);
	m_pagpmFactors->SetValue(&pcsSkill->m_csFactor, lSkillPoint, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_SKILL_LEVEL);

	return TRUE;
}

BOOL AgpmSkill::SetSkillExp(AgpdSkill *pcsSkill, INT32 lSkillExp)
{
	if (!pcsSkill || lSkillExp < 0)
		return FALSE;

	m_pagpmFactors->SetValue(&pcsSkill->m_csFactor, lSkillExp, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_SKILL_EXP);

	return TRUE;
}

BOOL AgpmSkill::CheckLearnableSkill(ApBase *pcsBase, INT32 lSkillTID)
{
	AgpdSkillTemplate	*pcsSkillTemplate	= GetSkillTemplate(lSkillTID);
	if (!pcsSkillTemplate)
		return FALSE;

	return CheckLearnableSkill(pcsBase, pcsSkillTemplate->m_szName);
}

//		CheckLearnableSkill
//	Functions
//		- skill을 배울 수 있는지 검사한다.
//			2. character의 m_aszUsableSkillTName 리스트에 szSkillName이 있는지 본다.
//			3. ok 면 TRUE를, 그렇지 않음 FALSE를 리턴한다.
//	Arguments
//		- pcsCharacter : 스킬을 배울 캐릭터
//		- szSkillName : 배울 스킬 이름
//	Return value
//		- BOOL : 스킬을 배웠는지 여부
///////////////////////////////////////////////////////////////////////////////
BOOL AgpmSkill::CheckLearnableSkill(ApBase *pcsBase, CHAR *szSkillName)
{
	if (!pcsBase || !szSkillName)
		return FALSE;

	if (pcsBase->m_eType != APBASE_TYPE_CHARACTER)
		return TRUE;

//	ApBase	*pcsBaseTemplate	= NULL;
//	switch (pcsBase->m_eType) {
//	case APBASE_TYPE_CHARACTER:
//		pcsBaseTemplate	= ((AgpdCharacter *) pcsBase)->m_pcsCharacterTemplate;
//		break;
//
//	case APBASE_TYPE_OBJECT:
//		pcsBaseTemplate = ((ApdObject *) pcsBase)->m_pcsTemplate;
//		break;
//	}
//	if (!pcsBaseTemplate)
//		return FALSE;
//
//	AgpdSkillTemplateAttachData	*pcsAttachTemplateData = GetAttachSkillTemplateData(pcsBaseTemplate);
//	if (!pcsAttachTemplateData)
//		return FALSE;
//
//	int	i = 0;
//
//	// pcsCharacter가 사용할 수 있는 스킬 리스트에 szSkillName이 있는지 살펴본다.
//	for (i = 0; i < AGPMSKILL_MAX_SKILL_OWN; ++i)
//	{
//		if (pcsAttachTemplateData->m_aszUsableSkillTName[i] == NULL)
//			return FALSE;
//
//		if (!strcmp(pcsAttachTemplateData->m_aszUsableSkillTName[i], szSkillName))
//			break;
//	}
//
//	// 리스트에서 못찾았다.
//	if (i == AGPMSKILL_MAX_SKILL_OWN)
//		return FALSE;

	AgpdSkillTemplate	*pcsSkillTemplate	= GetSkillTemplate(szSkillName);
	if (!pcsSkillTemplate)
		return FALSE;

#ifdef _HIGHLEVEL_SKILL_TEST_
	if (IsHighLevelSkill(pcsSkillTemplate))
		return TRUE;
#endif

	if ((pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_PRODUCT) && !IsOwnSkill(pcsBase, pcsSkillTemplate))
		return TRUE;

	// 아크로드 스킬. 2006.10.17. steeple
	if (IsArchlordSkill(pcsSkillTemplate) && m_pagpmCharacter->IsArchlord(((AgpdCharacter*)pcsBase)))
		return TRUE;

	if(!CheckRequireClass((AgpdCharacter*)pcsBase, pcsSkillTemplate))
		return FALSE;

	// 이미 배운 스킬인지 살펴본다.
	if (IsOwnSkill(pcsBase, pcsSkillTemplate))
		return FALSE;

	return TRUE;
}

BOOL AgpmSkill::CheckRequireClass(AgpdCharacter* pcsCharacter, AgpdSkillTemplate *pcsSkillTemplate)
{
	if(!pcsCharacter || !pcsSkillTemplate)
		return FALSE;

	INT32	lBaseRace	= m_pagpmFactors->GetRace(&pcsCharacter->m_csFactor);
	INT32	lBaseClass	= m_pagpmFactors->GetClass(&pcsCharacter->m_csFactor);

	switch ((AuRaceType) lBaseRace) 
	{
	case AURACE_TYPE_HUMAN:
		{
			switch ((AuCharClassType) lBaseClass) 
			{
			case AUCHARCLASS_TYPE_KNIGHT:
				if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_REQUIRE_CLASS_KNIGHT][1] == 0)
					return FALSE;
				break;

			case AUCHARCLASS_TYPE_RANGER:
				if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_REQUIRE_CLASS_ARCHER][1] == 0)
					return FALSE;
				break;

			case AUCHARCLASS_TYPE_MAGE:
				if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_REQUIRE_CLASS_WIZARD][1] == 0)
					return FALSE;
				break;

			default:
				return FALSE;
				break;
			};
		}
		break;

	case AURACE_TYPE_ORC:
		{
			switch ((AuCharClassType) lBaseClass) 
			{
			case AUCHARCLASS_TYPE_KNIGHT:
				if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_REQUIRE_CLASS_BERSERKER][1] == 0)
					return FALSE;
				break;

			case AUCHARCLASS_TYPE_RANGER:
				if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_REQUIRE_CLASS_HUNTER][1] == 0)
					return FALSE;
				break;

			case AUCHARCLASS_TYPE_MAGE:
				if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_REQUIRE_CLASS_SORCERER][1] == 0)
					return FALSE;
				break;

			default:
				return FALSE;
				break;
			};
		}
		break;

	case AURACE_TYPE_MOONELF:
		{
			switch ((AuCharClassType) lBaseClass) 
			{
			case AUCHARCLASS_TYPE_KNIGHT:
				if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_REQUIRE_CLASS_SWASHBUCKLER][1] == 0)
					return FALSE;
				break;

			case AUCHARCLASS_TYPE_RANGER:
				if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_REQUIRE_CLASS_RANGER][1] == 0)
					return FALSE;
				break;

			case AUCHARCLASS_TYPE_MAGE:
				if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_REQUIRE_CLASS_ELEMENTALER][1] == 0)
					return FALSE;
				break;

			default:
				return FALSE;
				break;
			};
		}
		break;

	case AURACE_TYPE_DRAGONSCION:
		{
			switch ((AuCharClassType) lBaseClass) 
			{
			case AUCHARCLASS_TYPE_KNIGHT:
				if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_REQUIRE_CLASS_SLAYER][1] == 0)
					return FALSE;
				break;

			case AUCHARCLASS_TYPE_RANGER:
				if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_REQUIRE_CLASS_OBITER][1] == 0)
					return FALSE;
				break;

			case AUCHARCLASS_TYPE_SCION:
				if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_REQUIRE_CLASS_SCION][1] == 0)
					return FALSE;
				break;

			case AUCHARCLASS_TYPE_MAGE:
				if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_REQUIRE_CLASS_SUMMONER][1] == 0)
					return FALSE;
				break;

			default:
				return FALSE;
				break;
			};
		}
		break;

	default:
		return FALSE;
		break;
	};

	return TRUE;
}

BOOL AgpmSkill::IsOwnSkill(ApBase *pcsBase, AgpdSkillTemplate *pcsSkillTemplate)
{
	if (!pcsBase || !pcsSkillTemplate)
		return FALSE;

	// 이미 배운 스킬인지 살펴본다.
	AgpdSkillAttachData *pcsAttachData = GetAttachSkillData(pcsBase);
	if (!pcsAttachData)
		return FALSE;

//	for (int i = 0; i < AGPMSKILL_MAX_SKILL_OWN; ++i)
//	{
//		if (pcsAttachData->m_alSkillID[i] == AP_INVALID_SKILLID)
//			return FALSE;
//
//		AgpdSkill *pcsSkill = GetSkill(pcsAttachData->m_alSkillID[i]);
//		if (pcsSkill)
//		{
//			if (pcsSkill->m_pcsTemplate == pcsSkillTemplate)
//				return TRUE;
//		}
//	}

	for (int i = 0; i < AGPMSKILL_MAX_SKILL_OWN; ++i)
	{
		if (pcsAttachData->m_alSkillID[i] == AP_INVALID_SKILLID)
			return FALSE;

		AgpdSkill *pcsSkill = pcsAttachData->m_apcsSkill[i];
		if (pcsSkill)
		{
			if (pcsSkill->m_pcsTemplate == pcsSkillTemplate)
				return TRUE;
		}
	}

	return FALSE;
}

INT32 AgpmSkill::GetTotalSkillPoint(ApBase *pcsBase, INT32 lRace, INT32 lClass , BOOL bIncludingHeroic )
{
	if(!pcsBase)
		return NULL;

	// 이미 배운 스킬인지 살펴본다.
	AgpdSkillAttachData *pcsAttachData = GetAttachSkillData(pcsBase);
	if (!pcsAttachData)
		return NULL;

	INT32 lSkillPoint = 0;

	for (int i = 0; i < AGPMSKILL_MAX_SKILL_OWN; ++i)
	{
		if (pcsAttachData->m_alSkillID[i] == AP_INVALID_SKILLID)
			break;

		AgpdSkill *pcsSkill = pcsAttachData->m_apcsSkill[i];
		if (pcsSkill)

		{
			if(lRace == AGPMSKILL_NONRACE_NONCLASS_SKILL || lClass == AGPMSKILL_NONRACE_NONCLASS_SKILL)
			{
				INT32 lSkillLevel = GetSkillLevel(pcsSkill);
				if(lSkillLevel < 1)
				{
					// error
					/*char strCharBuff[256] = { 0, };
					sprintf_s(strCharBuff, sizeof(strCharBuff), "[%s][%d]: %s(%s)", __FUNCTION__, __LINE__,
						((AgpdCharacter*)pcsSkill->m_pcsBase)->m_szID, ((AgpdSkillTemplate*) pcsSkill->m_pcsTemplate)->m_szName);
					AuLogFile_s("LOG\\SkillLevelError.txt", strCharBuff);*/
				}
				else
				{
					lSkillPoint += lSkillLevel;
				}
			}
			else
			{
				if(IsWantedRaceClassSkill(pcsSkill, lRace, lClass))
				{
					if( !IsPassiveSkill(pcsSkill) && !IsEvolutionSkill((AgpdSkillTemplate*)pcsSkill->m_pcsTemplate) && !IsNormalAttack(pcsSkill) )
					{
						INT32 lSkillLevel = GetSkillLevel(pcsSkill);
						if(lSkillLevel < 1)
						{
							// error
							/*char strCharBuff[256] = { 0, };
							sprintf_s(strCharBuff, sizeof(strCharBuff), "[%s][%d]: %s(%s)", __FUNCTION__, __LINE__,
								((AgpdCharacter*)pcsSkill->m_pcsBase)->m_szID, ((AgpdSkillTemplate*) pcsSkill->m_pcsTemplate)->m_szName);
							AuLogFile_s("LOG\\SkillLevelError.txt", strCharBuff);*/
						}
						else
						{
							lSkillPoint += lSkillLevel;
						}
					}
				}
			}
		}
	}

	return lSkillPoint;
}

INT32 AgpmSkill::GetTotalUsedHeroicPoint(ApBase *pcsBase)
{
	if(NULL == pcsBase)
		return 0;

	//Get skilllist that user have
	AgpdSkillAttachData *pcsAttachData = GetAttachSkillData(pcsBase);
	if(NULL == pcsAttachData)
		return 0;

	INT32 lHeroicPoint = 0;

	for(int i=0; i < AGPMSKILL_MAX_SKILL_OWN; ++i)
	{
		if(pcsAttachData->m_alSkillID[i] == AP_INVALID_SKILLID)
			break;

		AgpdSkill *pcsSkill = pcsAttachData->m_apcsSkill[i];
		if(pcsSkill && pcsSkill->m_pcsTemplate)
		{
			// Heroic Skill이 아니라면 스킵 시킨다
			if( !m_pagpmEventSkillMaster->IsHeroicSkill( pcsSkill->m_pcsTemplate->m_lID ) )
				continue;

			if( !m_pagpmEventSkillMaster->CheckHeroicSkillForClass((AgpdCharacter*)pcsBase, (AgpdSkillTemplate*)pcsSkill->m_pcsTemplate))
				continue;

			INT32 lHeroicPointByOneSkill = 0;

			lHeroicPointByOneSkill = m_pagpmEventSkillMaster->GetInputTotalCostHeroicPoint(pcsSkill);

			lHeroicPoint += lHeroicPointByOneSkill;
		}
	}

	return lHeroicPoint;
}

BOOL AgpmSkill::IsWantedRaceClassSkill(AgpdSkill *pcsSkill, INT32 lRace, INT32 lClass)
{
	if(!pcsSkill)
		return FALSE;

	AgpdSkillTemplate *pcsSkillTemplate = GetSkillTemplate(pcsSkill->m_pcsTemplate->m_lID);

	switch ((AuRaceType) lRace) {
	case AURACE_TYPE_HUMAN:
		{
			switch ((AuCharClassType) lClass) {
			case AUCHARCLASS_TYPE_KNIGHT:
				if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_REQUIRE_CLASS_KNIGHT][1] == 0)
					return FALSE;
			break;

			case AUCHARCLASS_TYPE_RANGER:
				if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_REQUIRE_CLASS_ARCHER][1] == 0)
					return FALSE;
				break;

			case AUCHARCLASS_TYPE_MAGE:
				if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_REQUIRE_CLASS_WIZARD][1] == 0)
					return FALSE;
				break;

			default:
				return FALSE;
				break;
			};
		}
		break;

	case AURACE_TYPE_ORC:
		{
			switch ((AuCharClassType) lClass) {
			case AUCHARCLASS_TYPE_KNIGHT:
				if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_REQUIRE_CLASS_BERSERKER][1] == 0)
					return FALSE;
				break;

			case AUCHARCLASS_TYPE_RANGER:
				if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_REQUIRE_CLASS_HUNTER][1] == 0)
					return FALSE;
				break;

			case AUCHARCLASS_TYPE_MAGE:
				if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_REQUIRE_CLASS_SORCERER][1] == 0)
					return FALSE;
				break;

			default:
				return FALSE;
				break;
			};
		}
		break;

		// 2005.08.18. steeple
		// 이제서야 비로소 정상 코드로 돌아갔다.
		// 2005.07.19. steeple
	case AURACE_TYPE_MOONELF:
		{
			switch ((AuCharClassType) lClass) {
			case AUCHARCLASS_TYPE_KNIGHT:
				if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_REQUIRE_CLASS_SWASHBUCKLER][1] == 0)
					return FALSE;
				break;

			case AUCHARCLASS_TYPE_RANGER:
				if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_REQUIRE_CLASS_RANGER][1] == 0)
					return FALSE;
				break;

			case AUCHARCLASS_TYPE_MAGE:
				if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_REQUIRE_CLASS_ELEMENTALER][1] == 0)
					return FALSE;
				break;

			default:
				return FALSE;
				break;
			};
		}
		break;

	case AURACE_TYPE_DRAGONSCION:
		{
			switch ((AuCharClassType) lClass) {
			case AUCHARCLASS_TYPE_KNIGHT:
				if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_REQUIRE_CLASS_SLAYER][1] == 0)
					return FALSE;
				break;

			case AUCHARCLASS_TYPE_RANGER:
				if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_REQUIRE_CLASS_OBITER][1] == 0)
					return FALSE;
				break;

			case AUCHARCLASS_TYPE_SCION:
				if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_REQUIRE_CLASS_SCION][1] == 0)
					return FALSE;
				break;

			case AUCHARCLASS_TYPE_MAGE:
				if (pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_REQUIRE_CLASS_SUMMONER][1] == 0)
					return FALSE;
				break;

			default:
				return FALSE;
				break;
			};
		}
		break;

	default:
		return FALSE;
		break;
	};

	return TRUE;
}

AgpdSkill* AgpmSkill::LearnSkill(ApBase *pcsBase, INT32 lSkillTID, INT32 lSkillLevel)
{
	AgpdSkillTemplate	*pcsSkillTemplate	= GetSkillTemplate(lSkillTID);
	if (!pcsSkillTemplate)
		return FALSE;

	return LearnSkill(pcsBase, pcsSkillTemplate->m_szName, lSkillLevel);
}

//		LearnSkill
//	Functions
//		- skill을 배운다.
//			1. CheckLearnableSkill()을 호출해 배울 수 있는지 검사한다.
//			2. 배울 수 있다면 callback을 호출한다.
//	Arguments
//		- pcsCharacter : 스킬을 배울 캐릭터
//		- szSkillName : 배울 스킬 이름
//	Return value
//		- BOOL : 스킬을 배웠는지 여부
///////////////////////////////////////////////////////////////////////////////
AgpdSkill* AgpmSkill::LearnSkill(ApBase *pcsBase, CHAR *szSkillName, INT32 lSkillLevel)
{
	if (!pcsBase || !szSkillName)
		return NULL;

	PVOID	pvBuffer[3];
	pvBuffer[0] = (PVOID) szSkillName;
	pvBuffer[1] = NULL;
	pvBuffer[2] = (PVOID) &lSkillLevel;

	EnumCallback(AGPMSKILL_CB_LEARN_SKILL, pcsBase, pvBuffer);

	return (AgpdSkill *) pvBuffer[1];
}

BOOL AgpmSkill::ForgetSkill(ApBase *pcsBase, INT32 lSkillID)
{
	if (!pcsBase || lSkillID == AP_INVALID_SKILLID)
		return FALSE;

	return ForgetSkill(pcsBase, GetSkill(lSkillID));
}

BOOL AgpmSkill::ForgetSkill(ApBase *pcsBase, AgpdSkill *pcsSkill)
{
	if (!pcsBase || !pcsSkill)
		return FALSE;

	return EnumCallback(AGPMSKILL_CB_FORGET_SKILL, pcsBase, pcsSkill);
}

// 2005.12.13. steeple
BOOL AgpmSkill::IsStatusActionOnActionType4(ApBase* pcsBase)
{
	if(!pcsBase)
		return FALSE;

	AgpdSkillAttachData* pcsAttachData = GetAttachSkillData(pcsBase);
	if(!pcsAttachData)
		return FALSE;

	if(pcsAttachData->m_ulBuffedSkillEffectDetail[AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION] & AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION_TYPE4)
		return TRUE;
	else
		return FALSE;
}

// 2005.10.04. steeple
INT32 AgpmSkill::GetTimeBuffType(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return 0;

	for(INT16 nIndex = 0; nIndex < AGPMSKILL_MAX_SKILL_CONDITION; nIndex++)
	{
		if(pcsSkillTemplate->m_lEffectType[nIndex] & AGPMSKILL_EFFECT_UPDATE_FACTOR_TIME)
		{
			return pcsSkillTemplate->m_stConditionArg[nIndex].lArg1;
		}
	}

	return 0;
}

BOOL AgpmSkill::ChangeSkillTemplateName(CHAR *szOriginalName, CHAR * szNewName)
{
	if (!szOriginalName || !strlen(szOriginalName) || !szNewName || !strlen(szNewName))
		return FALSE;

	AgpdSkillTemplate	*pcsTemplate	= GetSkillTemplate(szOriginalName);
	if (!pcsTemplate)
		return FALSE;

	strncpy(pcsTemplate->m_szName, szNewName, AGPMSKILL_MAX_SKILL_NAME);

	return m_csAdminTemplate.UpdateStringKey(szOriginalName, szNewName);
}

INT32 AgpmSkill::GetDivideDMGNumForShow(AgpdSkill *pcsSkill)
{
	if (!pcsSkill)
		return 0;

	INT32	lSkillLevel	= 0;

	m_pagpmFactors->GetValue(&pcsSkill->m_csFactor, &lSkillLevel, AGPD_FACTORS_TYPE_DIRT, AGPD_FACTORS_DIRT_TYPE_SKILL_LEVEL);

	return GetDivideDMGNumForShow((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate, lSkillLevel);
}

INT32 AgpmSkill::GetDivideDMGNumForShow(AgpdSkillTemplate *pcsSkillTemplate, INT32 lSkillLevel)
{
	if (!pcsSkillTemplate || lSkillLevel < 1 || lSkillLevel >= AGPMSKILL_MAX_SKILL_CAP)
		return 0;

	//return (UINT32) pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_DMG_SHOW_DIVIDE][lSkillLevel];
	return (UINT32) pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SHOW_DAMAGE][lSkillLevel];
}

INT32 AgpmSkill::GetDivideDMGNumForShow(INT32 lTID, INT32 lSkillLevel)
{
	return GetDivideDMGNumForShow(GetSkillTemplate(lTID), lSkillLevel);
}

BOOL AgpmSkill::IsAttackSkill(INT32 lSkillTID)
{
	return IsAttackSkill(GetSkillTemplate(lSkillTID));
}

BOOL AgpmSkill::IsAttackSkill(AgpdSkillTemplate *pcsSkillTemplate)
{
	if (!pcsSkillTemplate)
		return FALSE;

	if (pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_PHYSICAL_DMG ||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_MAGIC ||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_FIRE ||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_EARTH ||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_AIR ||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_MAGIC_DMG_WATER ||
		pcsSkillTemplate->m_lRangeType2 & AGPMSKILL_RANGE2_ONLY_ENEMY_UNITS	||	// 2007.07.02. steeple
		pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_DISPEL] & AGPMSKILL_EFFECT_DETAIL_DISPEL_ALL_BUFF
		)
		return TRUE;

	return FALSE;
}

BOOL AgpmSkill::IsAreaSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_SPHERE ||
		pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_BOX)
		return TRUE;

	return FALSE;
}

BOOL AgpmSkill::IsDebuffSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

    if(pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_BUFF_TYPE][1] == 1)
		return TRUE;
	else
		return FALSE;
}

BOOL AgpmSkill::IsSelfBuffSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(!(pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_BUFF))
		return FALSE;

	if((pcsSkillTemplate->m_lTargetType & AGPMSKILL_TARGET_SELF_ONLY) ||
		(pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_SELF_ONLY) ||
		(pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_SPHERE_INVOLVE_SELF) ||
		(pcsSkillTemplate->m_lRangeType & AGPMSKILL_RANGE_TARGET_ONLY_INVOLVE_SELF))
		return TRUE;
	else
		return FALSE;
}

BOOL AgpmSkill::IsETCSkill( AgpdSkillTemplate* pcsSkillTemplate )
{
	if(!pcsSkillTemplate)
		return FALSE;

	if( pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_ETC )
		return TRUE;

	return FALSE;
}

BOOL AgpmSkill::IsDurationByDistanceSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if((pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_DURATION_TYPE) &&
		(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_DURATION_TYPE] & AGPMSKILL_EFFECT_DETAIL_DURATION_TYPE1))
		return TRUE;
	else
		return FALSE;
}

BOOL AgpmSkill::IsDurationUnlimited(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	// 2007.03.15. steeple
	// Event Skill 이라면 시간 무제한으로 준다.
	if(m_pagpmConfig && m_pagpmConfig->IsEventChatting())
	{
		std::vector<INT32>::iterator iter = std::find(m_vcEventSkillTID.begin(), m_vcEventSkillTID.end(), pcsSkillTemplate->m_lID);
		if(iter != m_vcEventSkillTID.end())
			return TRUE;
	}

	if((pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_DURATION_TYPE) &&
		(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_DURATION_TYPE] & AGPMSKILL_EFFECT_DETAIL_DURATION_TYPE2))
		return TRUE;
	else
		return FALSE;
}

// 2005.09.01. steeple
// Summons 를 소환하는 스킬인지 체크
BOOL AgpmSkill::IsSummonsSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	// Summons Type 이 1, 6, 7, 8 인 것만 TRUE 로 리턴한다.
	if(pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_SUMMONS)
	{
		if((pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_SUMMONS] & AGPMSKILL_EFFECT_DETAIL_SUMMONS_TYPE1) ||
			(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_SUMMONS] & AGPMSKILL_EFFECT_DETAIL_SUMMONS_TYPE6) ||
			(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_SUMMONS] & AGPMSKILL_EFFECT_DETAIL_SUMMONS_TYPE7) ||
			(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_SUMMONS] & AGPMSKILL_EFFECT_DETAIL_SUMMONS_TYPE8) || 
			(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_SUMMONS] & AGPMSKILL_EFFECT_DETAIL_SUMMONS_TYPE11)
			)
			return TRUE;
	}

	return FALSE;
}

// 2005.09.01. steeple
// Taming 하는 스킬인지 체크
BOOL AgpmSkill::IsTameSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	// Taming 인지 체크
	if((pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_SUMMONS) && 
		(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_SUMMONS] & AGPMSKILL_EFFECT_DETAIL_SUMMONS_TYPE3))
			return TRUE;

	return FALSE;
}

// 2005.10.05. steeple
// 고정형 소환 타입인지 체크
BOOL AgpmSkill::IsFixedSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if((pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_SUMMONS) && 
		(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_SUMMONS] & AGPMSKILL_EFFECT_DETAIL_SUMMONS_TYPE2))
			return TRUE;

	return FALSE;
}

// 2005.10.04. steeple
// 투명 스킬인지 체크
BOOL AgpmSkill::IsTransparentSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if((pcsSkillTemplate->m_lEffectType[0] & AGPMSKILL_EFFECT_SPECIAL_STATUS) &&
		(pcsSkillTemplate->m_lSpecialStatus & AGPMSKILL_SPECIAL_STATUS_TRANSPARENT))
		return TRUE;

	return FALSE;
}

// 2005.10.10. steeple
// 캐스트 및, 처음 적용 시점에서 Agro 를 더 하지 않는 스킬인지 체크
BOOL AgpmSkill::IsNotAddAgroAtCast(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	// 먼저 ActionOnActionType1 (버프가 끝날 때 공격 데미지) 면 return TRUE
	if(GetActionOnActionType(pcsSkillTemplate) == 1)
		return TRUE;

	return FALSE;
}

// 2005.10.21. steeple
// ATField Attack 스킬인지 체크
BOOL AgpmSkill::IsATFieldAttackSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if((pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_AT_FIELD) &&
		(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_AT_FIELD] & AGPMSKILL_EFFECT_DETAIL_AT_FIELD_ATTACK))
		return TRUE;

	return FALSE;
}

// 2005.10.24. steeple
// Mutation 스킬인지 체크
BOOL AgpmSkill::IsMutationSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if((pcsSkillTemplate->m_lEffectType[0] & AGPMSKILL_EFFECT_TRANSFORM_TARGET) &&
		(pcsSkillTemplate->m_lEffectType[0] & AGPMSKILL_EFFECT_TRANSFORM_TARGET_TYPE4))
		return TRUE;


	return FALSE;
}

// 2005.11.15. steeple
// Release Target 스킬인지 체크
BOOL AgpmSkill::IsReleaseTargetSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_RELEASE_TARGET)
		return TRUE;

	return FALSE;
}

// 2005.12.19. steeple
// Visible Effect Type 이 Self Only 인지 체크
// 현재(2005.12.19) 는 타입이 Self Only 뿐이므로 세팅만 되어 있다면 return TRUE 이다.
BOOL AgpmSkill::IsVisibleEffectTypeSelfOnly(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_VISIBLE_EFFECT_TYPE)
		return TRUE;

	return FALSE;
}

// 2006.02.08. steeple
// 타입을 넣었3 Skill_Spec 에
// 2006.01.10. steeple
BOOL AgpmSkill::IsSubCashItemStackCountOnSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	// 이게 세팅되어 있으면 까면 안되므로 FALSE 로 리턴해준다.
	if(pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_ITEM_USE_TYPE)
		return FALSE;

	return TRUE;
}

// 2005.11.25. steeple
// Game Bonus Exp 스킬인지 체크
BOOL AgpmSkill::IsGameBonusExpSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if((pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_GAME_BONUS) &&
		(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_GAME_BONUS] & AGPMSKILL_EFFECT_DETAIL_GAME_BONUS_EXP))
		return TRUE;

	return FALSE;
}

// 2005.11.25. steeple
// Game Bonus Money 스킬인지 체크
BOOL AgpmSkill::IsGameBonusMoneySkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if((pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_GAME_BONUS) &&
		(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_GAME_BONUS] & AGPMSKILL_EFFECT_DETAIL_GAME_BONUS_MONEY))
		return TRUE;

	return FALSE;
}

// 2005.11.25. steeple
// Cash Skill 인지 체크
BOOL AgpmSkill::IsCashSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_CASH)
		return TRUE;

	return FALSE;
}

// 2006.06.27. steeple
// Level Diff Type 인지 체크
BOOL AgpmSkill::IsLevelDiffSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_LEVEL_DIFF_TYPE)
		return TRUE;

	return FALSE;
}

// 2006.08.11. steeple
// 타겟카운트 제한없는 공성 스킬인지 리턴.
BOOL AgpmSkill::IsUnlimitedTargetSiegeWarSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(pcsSkillTemplate->m_lRangeType2 == 0)
		return FALSE;

	if(pcsSkillTemplate->m_lRangeType2 & AGPMSKILL_RANGE2_SIEGE_WAR_TO_ALL_ATTACKERS ||
		pcsSkillTemplate->m_lRangeType2 & AGPMSKILL_RANGE2_SIEGE_WAR_TO_ALL_DEFENDERS)
		return TRUE;

	return FALSE;
}

// 2008.05.23. iluvs
// 타겟카운트 제한없는 종족 스킬인지 리턴
BOOL AgpmSkill::IsUnlimitedTargetRaceSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(pcsSkillTemplate->m_lRangeType2 == 0)
		return FALSE;

	if(pcsSkillTemplate->m_lRangeType2 & AGPMSKILL_RANGE2_TARGET_RACE)
		return TRUE;

	return FALSE;
}

// 2008.05.26. iluvs
BOOL AgpmSkill::IsPartySkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(pcsSkillTemplate->m_lRangeType2 == 0)
		return FALSE;

	if(pcsSkillTemplate->m_lRangeType2 & AGPMSKILL_RANGE2_TARGET_PARTY)
		return TRUE;

	return FALSE;
}

// 2009.03.26. iluvs
BOOL AgpmSkill::IsInvolveParentRegionSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(pcsSkillTemplate == NULL)
		return FALSE;

	if(pcsSkillTemplate->m_lRangeType2 == 0)
		return FALSE;

	// 파티, 종족에 상관없이 지역에 있는 모든 사람한테 줘야하는 스킬 
	if((pcsSkillTemplate->m_lRangeType2 & AGPMSKILL_RANGE2_TARGET_REGION_INVOLVE_PARENT) &&
	   !(pcsSkillTemplate->m_lRangeType2 & AGPMSKILL_RANGE2_TARGET_RACE) &&
	   !(pcsSkillTemplate->m_lRangeType2 & AGPMSKILL_RANGE2_TARGET_PARTY))
		return TRUE;

	return FALSE;

}

// 2006.08.11. steeple
BOOL AgpmSkill::IsSphereRangeSiegeWarSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(pcsSkillTemplate->m_lRangeType2 & AGPMSKILL_RANGE2_SPHERE &&
		((pcsSkillTemplate->m_lRangeType2 & AGPMSKILL_RANGE2_SIEGE_WAR_TO_ALL_ATTACKERS) ||
		(pcsSkillTemplate->m_lRangeType2 & AGPMSKILL_RANGE2_SIEGE_WAR_TO_ALL_DEFENDERS))
		)
		return TRUE;

	return FALSE;
}

// 2006.08.22. steeple
// SiegeWar 용 Summons 인지 리턴한다.
BOOL AgpmSkill::IsSummonsSkillForSiegeWar(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(!IsSummonsSkill(pcsSkillTemplate))
		return FALSE;

	if(GetSummonsType(pcsSkillTemplate) == 6)
		return TRUE;
	
	return FALSE;
}

// 2006.09.20. steeple
// Detect 스킬인지 아닌지
BOOL AgpmSkill::IsDetectSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_DETECT)
		return TRUE;

	return FALSE;
}

// 2006.11.02. steeple
// Ride 스킬인지 아닌지
BOOL AgpmSkill::IsRideSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_RIDE)
		return TRUE;

	return FALSE;
}

// 2006.10.11. steeple
// Archlord 스킬인지 아닌지
BOOL AgpmSkill::IsArchlordSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	// 스킬 타입이 9로 되어 있으면 아크로드용 스킬이다.
	if((INT32)pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SKILL_TYPE][1] == 9)
		return TRUE;

	return FALSE;
}

// 2006.10.20. steeple
// Ground Target Skill 인지
BOOL AgpmSkill::IsGroundTargetSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(pcsSkillTemplate->m_lRangeType2 & AGPMSKILL_RANGE2_GROUND)
		return TRUE;

	return FALSE;
}

// 2006.11..14. steeple
// DOT Skill 인지
BOOL AgpmSkill::IsDOTSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(pcsSkillTemplate->m_lEffectType[0] & AGPMSKILL_EFFECT_DOT_DAMAGE)
		return TRUE;

	return FALSE;
}

// 2007.07.07. steeple
// 재우는 스킬인지 확인.
BOOL AgpmSkill::IsSleepSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(pcsSkillTemplate->m_lSpecialStatus & AGPMSKILL_SPECIAL_STATUS_SLEEP)
		return TRUE;

	return FALSE;
}

// 2007.07.31. steeple
BOOL AgpmSkill::IsDispelAllBuffSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_DISPEL] & AGPMSKILL_EFFECT_DETAIL_DISPEL_ALL_BUFF)
		return TRUE;

	return FALSE;
}

// 2006.10.24. steeple
// 강제 스킬인지 아닌지.
BOOL AgpmSkill::IsForceSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_FORCE)
		return TRUE;

	return FALSE;
}

// 2006.10.25. steeple
// 스킬의 bForce 를 TRUE 로 해준다.
BOOL AgpmSkill::IsForceSetSkill(AgpdSkillTemplate *pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_FORCE &&
		pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_FORCE] & AGPMSKILL_EFFECT_DETAIL_FORCE_TYPE1)
		return TRUE;

	return FALSE;
}

// 2006.10.25. steeple
// 타겟이 없더라고 Casting 한다.
BOOL AgpmSkill::IsForceCastSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_FORCE &&
		pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_FORCE] & AGPMSKILL_EFFECT_DETAIL_FORCE_TYPE2)
		return TRUE;

	return FALSE;
}

// 2007.11.21. steeple
// 전투중이어도 투명 스킬 Casting 한다.
BOOL AgpmSkill::IsForceTransparentSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_FORCE &&
		pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_FORCE] & AGPMSKILL_EFFECT_DETAIL_FORCE_TYPE3)
		return TRUE;

	return FALSE;
}

// 2007.08.23. steeple
// 시체를 타겟할 수 있는 지 리턴한다.
BOOL AgpmSkill::IsDeadTarget(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	//드래곤시온용 예외처리 추가 ㅎㄷㄷ 
	if( IsEvolutionSkill(pcsSkillTemplate) )
		return TRUE;

	if(pcsSkillTemplate->m_lRangeType2 & AGPMSKILL_RANGE2_TARGET_DEAD)
		return TRUE;

	return FALSE;
}

// 2007.08.23. steeple
// 부활 스킬인지 리턴.
BOOL AgpmSkill::IsResurrectionSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_RESURRECTION)
		return TRUE;

	return FALSE;
}

// 2007.10.17. steeple
// Skill Union Type1 스킬인지 체크
BOOL AgpmSkill::IsSkillUnionType1(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(!(pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_SKILL_UNION))
		return FALSE;

	if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_SKILL_UNION] & AGPMSKILL_EFFECT_DETAIL_SKILL_UNION_TYPE1)
		return TRUE;

	return FALSE;
}

// 2007.10.17. steeple
// Skill Union Type2 스킬인지 체크
BOOL AgpmSkill::IsSkillUnionType2(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(!(pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_SKILL_UNION))
		return FALSE;

	if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_SKILL_UNION] & AGPMSKILL_EFFECT_DETAIL_SKILL_UNION_TYPE2)
		return TRUE;

	return FALSE;
}

// 2007.11.18. steeple
// Move Position Target 인지 체크
BOOL AgpmSkill::IsMovePosTarget(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(!(pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_MOVE_POS))
		return FALSE;

	if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_MOVE_POS] & AGPMSKILL_EFFECT_DETAIL_MOVE_POS_TARGET)
		return TRUE;

	return FALSE;
}

// 2007.11.18. steeple
// Move Position Self 인지 체크
BOOL AgpmSkill::IsMovePosSelf(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	if(!(pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_MOVE_POS))
		return FALSE;

	if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_MOVE_POS] & AGPMSKILL_EFFECT_DETAIL_MOVE_POS_SELF)
		return TRUE;

	return FALSE;
}

BOOL AgpmSkill::IsEvolutionSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	for (INT32 i = 0; i < AGPMSKILL_MAX_SKILL_CONDITION; ++i)
	{
		UINT64 lEffectType = pcsSkillTemplate->m_lEffectType[i];
		if ( lEffectType & AGPMSKILL_EFFECT_TRANSFORM_TARGET)
		{
			if (lEffectType & AGPMSKILL_EFFECT_TRANSFORM_TARGET_TYPE5)
				return TRUE;
		}
	}

	return FALSE;
}

BOOL AgpmSkill::IsTransformSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	for (INT32 i = 0; i < AGPMSKILL_MAX_SKILL_CONDITION; ++i)
	{
		UINT64 lEffectType = pcsSkillTemplate->m_lEffectType[i];
		if ( lEffectType & AGPMSKILL_EFFECT_TRANSFORM_TARGET)
		{
			if((lEffectType & AGPMSKILL_EFFECT_TRANSFORM_TARGET_TYPE4) 
				|| (lEffectType & AGPMSKILL_EFFECT_TRANSFORM_TARGET_TYPE5))
				return FALSE;

			return TRUE;
		}
	}

	return FALSE;
}
//JK_쉬라인추가
// 배틀그라운드에서 받을수 있는 종족버프..
BOOL AgpmSkill::IsBattleGroundRaceSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return FALSE;

	switch(pcsSkillTemplate->m_lID)
	{
	case 929:
	case 931:
	case 932:
	case 933:
	case 934:
	case 935:
	case 936:
	case 937:
		return TRUE;
		break;
	default:
		return FALSE;
		break;

	}

	return FALSE;
}

// 2006.01.10. steeple
// Action on Action Type 을 리턴한다. 해당 스킬이 아니라면 0 리턴.
INT32 AgpmSkill::GetActionOnActionType(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return 0;

	if(!(pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_ACTOIN_ON_ACTION))
		return 0;

	if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION] & AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION_TYPE1)
		return 1;
	else if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION] & AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION_TYPE2)
		return 2;
	else if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION] & AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION_TYPE3)
		return 3;
	else if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION] & AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION_TYPE4)
		return 4;
	else if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION] & AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION_TYPE5)
		return 5;
	else if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION] & AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION_TYPE6)
		return 6;
	else if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION] & AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION_TYPE7)
		return 7;
	else if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION] & AGPMSKILL_EFFECT_DETAIL_ACTION_ON_ACTION_TYPE8)
		return 8;

	return 0;
}

// 2006.01.10. steeple
// Summons Type 을 리턴한다. 해당 스킬이 아니면 0 리턴.
INT32 AgpmSkill::GetSummonsType(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return 0;

	if(!(pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_SUMMONS))
		return 0;

	if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_SUMMONS] & AGPMSKILL_EFFECT_DETAIL_SUMMONS_TYPE1)
		return 1;
	else if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_SUMMONS] & AGPMSKILL_EFFECT_DETAIL_SUMMONS_TYPE2)
		return 2;
	else if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_SUMMONS] & AGPMSKILL_EFFECT_DETAIL_SUMMONS_TYPE3)
		return 3;
	else if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_SUMMONS] & AGPMSKILL_EFFECT_DETAIL_SUMMONS_TYPE4)
		return 4;
	else if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_SUMMONS] & AGPMSKILL_EFFECT_DETAIL_SUMMONS_TYPE5)
		return 5;
	else if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_SUMMONS] & AGPMSKILL_EFFECT_DETAIL_SUMMONS_TYPE6)
		return 6;
	else if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_SUMMONS] & AGPMSKILL_EFFECT_DETAIL_SUMMONS_TYPE7)
		return 7;
	else if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_SUMMONS] & AGPMSKILL_EFFECT_DETAIL_SUMMONS_TYPE8)
		return 8;

	return 0;
}

// 2008.06.20. steeple
// return Summons TID
INT32 AgpmSkill::GetSummonsTIDByItem(AgpdItemTemplate* pcsItemTemplate)
{
	if(!pcsItemTemplate || pcsItemTemplate->m_nType != AGPMITEM_TYPE_USABLE)
		return 0;

	if(((AgpdItemTemplateUsable*)pcsItemTemplate)->m_nUsableItemType != AGPMITEM_USABLE_TYPE_SKILL_SCROLL)
		return 0;

	INT32 lSkillTID = ((AgpdItemTemplateUsableSkillScroll*)pcsItemTemplate)->m_lSkillTID;
	AgpdSkillTemplate* pcsSkillTemplate = GetSkillTemplate(lSkillTID);
	if(!pcsSkillTemplate)
		return 0;

	return (INT32)pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_SUMMONS_TID][1];
}

// 2007.07.04. steeple
// Detect Type 을 리턴한다.
INT32 AgpmSkill::GetDetectType(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return 0;

	if(!IsDetectSkill(pcsSkillTemplate))
		return 0;

	if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_DETECT] & AGPMSKILL_EFFECT_DETAIL_DETECT_TYPE1)
		return 1;
	else if(pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_DETECT] & AGPMSKILL_EFFECT_DETAIL_DETECT_TYPE2)
		return 2;

	return 0;
}

// 2007.12.18. steeple
INT32 AgpmSkill::GetSkillLevelUpType(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(!pcsSkillTemplate)
		return 0;

	if(!(pcsSkillTemplate->m_lEffectType2[0] & AGPMSKILL_EFFECT2_SKILL_LEVELUP))
		return 0;

	return pcsSkillTemplate->m_nEffectDetailType[AGPMSKILL_EFFECT_DETAIL_SKILL_LEVELUP];
}

AgpdSkill* AgpmSkill::GetSkillByAction(AgpdCharacter *pcsCharacter, AgpdCharacterActionResultType eActionResult)
{
	if (!pcsCharacter)
		return NULL;

	AgpdSkillAttachData	*pcsAttachData	= GetAttachSkillData((ApBase *) pcsCharacter);

	switch (eActionResult) {
	case AGPDCHAR_ACTION_RESULT_TYPE_ATTACK_CRITICAL:
		{
			for (int i = AGPMSKILL_MAX_SKILL_USE - 1; i >= 0; --i)
			{
//				AgpdSkill				*pcsSkill				= GetSkill(pcsAttachData->m_alUsePassiveSkillID[i]);
				AgpdSkill				*pcsSkill				= pcsAttachData->m_apcsUsePassiveSkill[i];
				if (pcsSkill && pcsSkill->m_pcsTemplate)
				{
					if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lEffectType[0] & AGPMSKILL_EFFECT_MELEE_ATTACK_CRITICAL ||
						((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lEffectType[0] & AGPMSKILL_EFFECT_MAGIC_ATTACK_CRITICAL)
						return pcsSkill;
				}
			}
		}
		break;

	case AGPDCHAR_ACTION_RESULT_TYPE_ATTACK_EVADE:
		{
			for (int i = AGPMSKILL_MAX_SKILL_USE - 1; i >= 0; --i)
			{
//				AgpdSkill				*pcsSkill				= GetSkill(pcsAttachData->m_alUsePassiveSkillID[i]);
				AgpdSkill				*pcsSkill				= pcsAttachData->m_apcsUsePassiveSkill[i];
				if (pcsSkill && pcsSkill->m_pcsTemplate)
				{
					if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lEffectType[0] & AGPMSKILL_EFFECT_DEFENSE_EVADE)
						return pcsSkill;
				}
			}
		}
		break;

	case AGPDCHAR_ACTION_RESULT_TYPE_COUNTER_ATTACK:
		{
			for (int i = AGPMSKILL_MAX_SKILL_USE - 1; i >= 0; --i)
			{
//				AgpdSkill				*pcsSkill				= GetSkill(pcsAttachData->m_alUsePassiveSkillID[i]);
				AgpdSkill				*pcsSkill				= pcsAttachData->m_apcsUsePassiveSkill[i];
				if (pcsSkill && pcsSkill->m_pcsTemplate)
				{
					if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lEffectType[0] & AGPMSKILL_EFFECT_DEFENSE_COUNTER_ATTACK)
						return pcsSkill;
				}
			}
		}
		break;

	case AGPDCHAR_ACTION_RESULT_TYPE_DEATH_STRIKE:
		{
			for (int i = AGPMSKILL_MAX_SKILL_USE - 1; i >= 0; --i)
			{
//				AgpdSkill				*pcsSkill				= GetSkill(pcsAttachData->m_alUsePassiveSkillID[i]);
				AgpdSkill				*pcsSkill				= pcsAttachData->m_apcsUsePassiveSkill[i];
				if (pcsSkill && pcsSkill->m_pcsTemplate)
				{
					if (((AgpdSkillTemplate *) pcsSkill->m_pcsTemplate)->m_lEffectType[0] & AGPMSKILL_EFFECT_MELEE_ATTACK_DEATH)
						return pcsSkill;
				}
			}
		}
		break;
	}

	return NULL;
}

INT32 AgpmSkill::GetRandomCreatureTID(AgpdSkillTemplate* pcsSkillTemplate, INT32 lSkillLevel)
{
	if(!pcsSkillTemplate || lSkillLevel < 1 || lSkillLevel >= AGPMSKILL_MAX_SKILL_CAP)
		return 0;

	// 해당 레벨에 valid 한 값의 count 를 구한다.
	INT32 lValidCount = 0;
	for(lValidCount; lValidCount < AGPMSKILL_MAX_CREATURE_TID_NUM; lValidCount++)
		if(pcsSkillTemplate->m_allCreatureTID[lSkillLevel][lValidCount] == 0)
			break;

	if(lValidCount == 0)
		return 0;

	MTRand csRandom;
	INT32 lRandom = csRandom.randInt(lValidCount);
	if(lRandom < 0 || lRandom >= AGPMSKILL_MAX_CREATURE_TID_NUM)
		return 0;

	return pcsSkillTemplate->m_allCreatureTID[lSkillLevel][lRandom];
}

INT32 AgpmSkill::GetBonusExpRate(AgpdCharacter* pcsCharacter)
{
	if(!pcsCharacter)
		return 0;

	AgpdSkillAttachData* pcsAttachData = GetAttachSkillData((ApBase*)pcsCharacter);
	if(!pcsAttachData)
		return 0;

	return pcsAttachData->m_stBuffedSkillFactorEffectArg.lBonusExpRate;
}

INT32 AgpmSkill::GetBonusMoneyRate(AgpdCharacter* pcsCharacter)
{
	if(!pcsCharacter)
		return 0;

	AgpdSkillAttachData* pcsAttachData = GetAttachSkillData((ApBase*)pcsCharacter);
	if(!pcsAttachData)
		return 0;

	return pcsAttachData->m_stBuffedSkillFactorEffectArg.lBonusMoneyRate;
}

INT32 AgpmSkill::GetBonusCharismaRate(AgpdCharacter* pcsCharacter)
{
	if(!pcsCharacter)
		return 0;

	AgpdSkillAttachData* pcsAttachData = GetAttachSkillData((ApBase*)pcsCharacter);
	if(!pcsAttachData)
		return 0;

	return pcsAttachData->m_stBuffedSkillFactorEffectArg.lBonusDropCharismaRate;
}

AgpdSkill* AgpmSkill::GetSkillByTID(AgpdCharacter *pcsCharacter, INT32 lTID)
{
	if (!pcsCharacter)
		return NULL;

	AgpdSkillAttachData	*pcsAttachData	= GetAttachSkillData((ApBase *) pcsCharacter);

	for (int i = 0; i < AGPMSKILL_MAX_SKILL_OWN; ++i)
	{
//		AgpdSkill	*pcsSkill	= GetSkill(pcsAttachData->m_alSkillID[i]);
		AgpdSkill	*pcsSkill	= pcsAttachData->m_apcsSkill[i];
		if (pcsSkill && pcsSkill->m_pcsTemplate)
		{
			if (pcsSkill->m_pcsTemplate->m_lID == lTID)
				return pcsSkill;
		}
	}

	return NULL;
}

INT32 AgpmSkill::GetModifiedHitRate(AgpdCharacter *pcsCharacter)
{
	return 0;
	//if (!pcsCharacter)
	//	return 0;

	//AgpdSkillAttachData	*pcsAttachData	= GetAttachSkillData((ApBase *) pcsCharacter);

	//return pcsAttachData->m_stBuffedSkillCombatEffectArg.lHitRate;
}

INT32 AgpmSkill::GetModifiedBlockRate(AgpdCharacter *pcsCharacter)
{
	// 2006.11.21. steeple
	// Factor 쪽으로 옮겨가서 이제 리턴 0 한다.
	return 0;
	//if (!pcsCharacter)
	//	return 0;

	//AgpdSkillAttachData	*pcsAttachData	= GetAttachSkillData((ApBase *) pcsCharacter);

	//return pcsAttachData->m_stBuffedSkillCombatEffectArg.lBlockRate;
}

INT32 AgpmSkill::GetModifiedHPRegen(AgpdCharacter *pcsCharacter)
{
	if (!pcsCharacter)
		return 0;

	AgpdSkillAttachData	*pcsAttachData	= GetAttachSkillData((ApBase *) pcsCharacter);

	return pcsAttachData->m_stBuffedSkillFactorEffectArg.lHPRegen;
}

INT32 AgpmSkill::GetModifiedMPRegen(AgpdCharacter *pcsCharacter)
{
	if (!pcsCharacter)
		return 0;

	AgpdSkillAttachData	*pcsAttachData	= GetAttachSkillData((ApBase *) pcsCharacter);

	return pcsAttachData->m_stBuffedSkillFactorEffectArg.lMPRegen;
}

eAgpmSkillWeaponDamageType AgpmSkill::GetWeaponDamageType(AgpdCharacter *pcsCharacter)
{
	if (!pcsCharacter)
		return AGPMSKILL_WEAPON_DAMAGE_TYPE_NORMAL;

	AgpdSkillAttachData	*pcsAttachData	= GetAttachSkillData((ApBase *) pcsCharacter);

	return (eAgpmSkillWeaponDamageType) pcsAttachData->m_stBuffedSkillCombatEffectArg.lWeaponDamageType;
}


BOOL AgpmSkill::ProcessRemove(UINT32 ulClockCount)
{
	if (m_ulPrevRemoveClockCount + AGPMSKILL_PROCESS_REMOVE_INTERVAL > ulClockCount)
		return TRUE;

	INT32		lIndex		= 0;
	AgpdSkill	*pcsSkill	= NULL;

	AgpdSkill	**ppcsSkill = (AgpdSkill **) m_csAdminSkillRemove.GetObjectSequence(&lIndex);

	while (ppcsSkill && *ppcsSkill)
	{
		pcsSkill		= *ppcsSkill;

		if (pcsSkill->m_ulRemoveTimeMSec + AGPMSKILL_RESERVE_SKILL_DATA < ulClockCount)
		{
			// 이제 모듈 데이타를 삭제할 때가 되었다.
			m_csAdminSkillRemove.RemoveObject((INT_PTR) pcsSkill);
			lIndex = 0;

			DestroySkill(pcsSkill);
		}

		ppcsSkill = (AgpdSkill **) m_csAdminSkillRemove.GetObjectSequence(&lIndex);
	}

	m_ulPrevRemoveClockCount = ulClockCount;

	return TRUE;
}

BOOL AgpmSkill::AddRemoveSkill(AgpdSkill *pcsSkill)
{
	if (!pcsSkill)
		return FALSE;

	pcsSkill->m_ulRemoveTimeMSec	= GetClockCount();

	if (!m_csAdminSkillRemove.AddObject(&pcsSkill, (INT_PTR) pcsSkill))
	{
		AgpdSkill	**ppcsSkill = (AgpdSkill **) m_csAdminSkillRemove.GetObject((INT_PTR) pcsSkill);
		if (ppcsSkill && *ppcsSkill)
		{
			DestroySkill(*ppcsSkill);

			m_csAdminSkillRemove.RemoveObject((INT_PTR) (*ppcsSkill));

			if (m_csAdminSkillRemove.AddObject(&pcsSkill, (INT_PTR) pcsSkill))
				return TRUE;
		}

		return FALSE;
	}

	return TRUE;
}

INT32 AgpmSkill::GetModifiedSkillLevel(ApBase* pcsBase)
{
	if(!pcsBase)
		return 0;

	AgpdSkillAttachData* pcsAttachData = GetAttachSkillData(pcsBase);
	if(!pcsAttachData)
		return 0;

	return pcsAttachData->m_lModifiedSkillLevel;
}

BOOL AgpmSkill::AddEventSkillTID(INT32 lSkillTID)
{
	if(!lSkillTID)
		return FALSE;

	m_vcEventSkillTID.push_back(lSkillTID);
	return TRUE;
}

BOOL AgpmSkill::IsCharacterConfigSkill(AgpdSkillTemplate* pcsSkillTemplate)
{
	if(NULL == pcsSkillTemplate)
		return FALSE;

	// 캐릭터별 채팅 가능여부 설정 
	if(pcsSkillTemplate->m_lSpecialStatus & AGPMSKILL_SPECIAL_STATUS_DISABLE_CHATTING)
		return TRUE;

	return FALSE;
}

BOOL AgpmSkill::SetSkillTemplateDurationTime(AgpdSkillTemplate *pcsSkillTemplate, UINT32 lSkillDuration, UINT32 lSkillLevel)
{
	if(NULL == pcsSkillTemplate)
		return FALSE;

	if (pcsSkillTemplate->m_bShrineSkill ||
		pcsSkillTemplate->m_nAttribute & AGPMSKILL_SKILL_ATTR_PRODUCT)
		return FALSE;

	pcsSkillTemplate->m_fUsedConstFactor[AGPMSKILL_CONST_DURATION][lSkillLevel] = (FLOAT)lSkillDuration;

	return TRUE;
}

eAgpmCharacterRoleType AgpmSkill::GetCharacterSkillRoleType(AgpdCharacter *pcsCharacter)
{
	if(NULL == pcsCharacter)
		return AGPMSKILL_CHARACTER_ROLETYPE_NONE;

	INT16	nClass = AUCHARCLASS_TYPE_NONE;
	eAgpmCharacterRoleType eRoleType = AGPMSKILL_CHARACTER_ROLETYPE_NONE;

	nClass = m_pagpmFactors->GetClass(&pcsCharacter->m_csFactor);
	switch(nClass)
	{
	case AUCHARCLASS_TYPE_KNIGHT:
		{
			eRoleType = AGPMSKILL_CHARACTER_ROLETYPE_MELEE;
		} break;

	case AUCHARCLASS_TYPE_RANGER:
		{
			eRoleType = AGPMSKILL_CHARACTER_ROLETYPE_RANGED;
		} break;

	case AUCHARCLASS_TYPE_MAGE:
		{
			eRoleType = AGPMSKILL_CHARACTER_ROLETYPE_MAGIC;
		} break;

	default:
		{
			eRoleType = AGPMSKILL_CHARACTER_ROLETYPE_NONE;
		} 
	}

	return eRoleType;
}
#include "AgcmTargeting.h"
#include <limits.h>
#include "AgcmUIConsole.h"

#include "ApMemoryTracker.h"
#include "ApModuleStream.h"
#include "AgpmArchlord.h"

#include "AgcmGlyph.h"

#include "AgcmTextBoardMng.h"

#define AGCMTARGETING_LOCK_DELAY	1000

// Lua Function
LuaGlue	LG_SendMoveSelfCharacter( lua_State *L )
{
	AuLua * pLua = AuLua::GetPointer();

	FLOAT	fDestX		= ( FLOAT ) pLua->GetNumberArgument( 1 , -1 );
	FLOAT	fDestZ		= ( FLOAT ) pLua->GetNumberArgument( 2 , -1 );

	if( -1.0f == fDestX || -1.0f == fDestZ )
	{
		g_pEngine->LuaErrorMessage( "SendMoveSelfCharacter() : Destination is needed" );
		LUA_RETURN( FALSE );
	}

	AgcmTargeting		* pcsAgcmTargeting	= ( AgcmTargeting * ) g_pEngine->GetModule( "AgcmTargeting" );

	AuPOS	posDest;
	posDest.x = fDestX	;
	posDest.y = 0.0f	;
	posDest.z = fDestZ	;

	pcsAgcmTargeting->MoveSelfCharacter( &posDest );

	LUA_RETURN( TRUE );
}

AgcmTargeting::AgcmTargeting()
:m_pAgpmShrineBattle(NULL)
{
	int i;

	SetModuleName("AgcmTargeting");
	EnableIdle(TRUE);

	m_pAgpmCharacter		= NULL;
	m_pAgcmGlyph			= NULL;
	m_pAgcmCharacter		= NULL;
	m_pApmEventManager		= NULL;
	m_pAgcmEventManager		= NULL;
	m_pAgcmMap				= NULL;
	m_pAgcmSkill			= NULL;
	m_pAgpmItem				= NULL;
	m_pAgcmItem				= NULL;
	m_pAgcmPrivateTrade		= NULL;
	m_pAgcmRender			= NULL;
	m_pAgcmParty			= NULL;
	m_pAgpmFactors			= NULL;
	m_pAgcmUIManager2		= NULL;
	m_pAgcmUILogin			= NULL;
	m_pAgcmUICharacter		= NULL;
	m_pAgcmTextBoardMng		= NULL;
	m_pApmMap				= NULL;
	m_pAgcmShrine			= NULL;//JK_쉬라인머지

	m_pAgcmShadow			= NULL;

	m_pAgcmOcTree			= NULL;
	m_pApmOcTree			= NULL;

	m_pCamera	=	NULL;
	m_pWorld	= NULL;

	m_iPickedNum	= 0;
	m_listPicking	= NULL;	
	m_pPrevPickingClump = NULL;

	for(i=0;i<AGCMTARGETING_DESTMOVE_IMAGE_NUM;++i)
		m_pDestMove[i] = NULL;

	//@{ 2006/10/26 burumal
	for (i=0; i< AGCMTARGETING_AREATARGETING_IMAGE_NUM; ++i)
		m_pAreaTargetingTex[i] = NULL;
	//@}

	ZeroMemory( m_hMouseCursorMove	, sizeof(HCURSOR) * AGCMTARGETING_MCURSOR_MOVE_NUM	);
	ZeroMemory( m_hMouseCursorAttack, sizeof(HCURSOR) * AGCMTARGETING_MCURSOR_ATTACK_NUM);
	ZeroMemory( m_hMouseCursorTalk	, sizeof(HCURSOR) * AGCMTARGETING_MCURSOR_TALK_NUM	);
	ZeroMemory( m_hMouseCursorPick	, sizeof(HCURSOR) * AGCMTARGETING_MCURSOR_PICK_NUM	);
	ZeroMemory( m_hMouseCursorNoMove, sizeof(HCURSOR) * AGCMTARGETING_MCURSOR_NOMOVE_NUM);

	m_lCursorMove	= 0;
	m_lCursorAttack	= 0;
	m_lCursorTalk	= 0;
	m_lCursorPick	= 0;
	m_lCursorNoMove	= 0;

	m_iMouseCursorMode	= m_lCursorMove;											// 마우스 커서 모드

	m_uiLastTick = 0;
	ZeroMemory(&m_vCurMousePos, sizeof(RwV2d));

	m_bMouseLDown = false;

	m_iTargetMode = AGCMTARGETING_TARGET_MODE_NONE;

	m_pMoveTarget = NULL;
	
	m_pAreaTarget = NULL;
	m_iLastTargetImage = 0;

	m_pEnemyTargetClump = NULL;

	m_pEnemyMouseOnClump = NULL;
	m_bShowCursor = FALSE;

	m_bCurrentLoginMode	=	FALSE;
	m_bProcessMouseMove = FALSE;
	m_bProtectMove = FALSE;

	m_bShowGlyph	= TRUE	;

	// 마고자 (2005-03-22 오후 5:41:06) : 
	// 일단 대충 돌아가므로 실전 디버그.
	m_lUsePathFind	= TRUE	;

	// 마고자 (2005-05-17 오후 6:36:08) : 
	// 마지막 선택한 NPC 정보.
	m_lLastClickNPCID	= -1;
	SetTargetBase(NULL);

	m_fKeyTargettingRange	= 6000.0f;
}

AgcmTargeting::~AgcmTargeting()
{

}

BOOL AgcmTargeting::OnAddModule()
{
	m_pAgpmCharacter = (AgpmCharacter*) GetModule("AgpmCharacter");
	m_pAgcmGlyph = (AgcmGlyph*) GetModule("AgcmGlyph");
	m_pAgcmCharacter = (AgcmCharacter*) GetModule("AgcmCharacter");
	m_pAgcmEventManager	= (AgcmEventManager*)GetModule("AgcmEventManager");
	m_pAgcmMap	= (AgcmMap*)GetModule("AgcmMap");
	m_pAgcmSkill = (AgcmSkill*)GetModule("AgcmSkill");
	m_pAgpmItem	= (AgpmItem*)GetModule("AgpmItem");
	m_pAgcmItem = (AgcmItem*)GetModule("AgcmItem");
	m_pAgcmPrivateTrade	= (AgcmPrivateTrade*)GetModule("AgcmPrivateTrade");
	m_pApmEventManager = (ApmEventManager*)GetModule("ApmEventManager");
	m_pAgcmRender = (AgcmRender*)GetModule("AgcmRender");
	m_pAgcmParty = (AgcmParty *)GetModule("AgcmParty");
	m_pAgpmFactors = (AgpmFactors*)GetModule("AgpmFactors");
	m_pAgcmUIManager2 = (AgcmUIManager2*)GetModule("AgcmUIManager2");
	m_pAgcmUILogin	= (AgcmUILogin*)GetModule("AgcmUILogin");
	m_pAgcmUICharacter = (AgcmUICharacter*)GetModule("AgcmUICharacter");
	m_pAgcmTextBoardMng = (AgcmTextBoardMng*)GetModule("AgcmTextBoardMng");
	m_pApmMap = (ApmMap*)GetModule("ApmMap");
	m_pAgcmReturnToLogin = (AgcmReturnToLogin*)GetModule("AgcmReturnToLogin");
	m_pAgpmSiegeWar = (AgpmSiegeWar*)GetModule("AgpmSiegeWar");
	m_pAgcmSiegeWar = (AgcmSiegeWar *)GetModule("AgcmSiegeWar");
	m_pAgcmShrine = (AgcmShrine*)GetModule("AgcmShrine");//JK_쉬라인머지


	AgcmLogin* pcmLogin = (AgcmLogin*)GetModule("AgcmLogin");
	ASSERT( NULL != pcmLogin );

	m_pAgcmShadow = (AgcmShadow*)GetModule("AgcmShadow");

	m_pAgcmOcTree = (AgcmOcTree*)GetModule("AgcmOcTree");
	m_pApmOcTree  = (ApmOcTree*)GetModule("ApmOcTree");

	if(pcmLogin)
	{
		pcmLogin->SetCallbackEnterGameEnd( CBLoginProcessEnd, this ); 
		pcmLogin->SetCallbackEncryptCodeSuccess( CBEncryptCodeSuccess, this );
	}

	if(m_pAgcmReturnToLogin)
	{
		m_pAgcmReturnToLogin->SetCallbackEndProcess(CBReturnToLoginEndProcess, this);
	}

	if(m_pAgcmCharacter)
	{
		m_pAgcmCharacter->SetCallbackRemoveCharacter( CBRemoveCharacter,this );
		m_pAgcmCharacter->SetCallbackLockTarget( CB_LockTarget, this);
		m_pAgcmCharacter->SetCallbackUnlockTarget( CB_UnlockTarget, this);

		//. 2006. 3. 7. Nonstopdj
		//. Release Self Character call back.
		//. targetmode -> none.
		m_pAgcmCharacter->SetCallbackReleaseSelfCharacter( CBSetTargetMode, this );
	}

	if(m_pAgpmCharacter)
	{
		m_pAgpmCharacter->SetCallbackUpdateActionStatus(CB_UpdateActionStatus, this);
	}

	if(m_pAgcmSkill)
	{
		m_pAgcmSkill->SetCallbackStartGroundTargetSkill(CBStartGroundTarget, this);
	}

	AgcmUIMain*		pUIMain = (AgcmUIMain*)GetModule("AgcmUIMain");
	if (pUIMain)
	{
		pUIMain->SetCallbackRButtonSkillCast(CBRButtonSkill, this);

		//@{ 2006/11/02 burumal
		pUIMain->SetCallbackKeydownESC(CBKeydownESC, this);
		//@}
	}

	SetRpWorld(GetWorld());
	SetRwCamera(GetCamera());

	//@{ 2006/08/11 burumal
	m_pAgcmUIManager2->AddFunction(this, "Targeting_FuncAutoTargetingF", CBAutoTargetingForward, 0);
	m_pAgcmUIManager2->AddFunction(this, "Targeting_FuncAutoTargetingB", CBAutoTargetingBackward, 0);
	//@}

	return	TRUE;
}

BOOL AgcmTargeting::OnInit()
{
	m_clSenderWindow.SetTargetingClass(this);

	AS_REGISTER_TYPE_BEGIN(AgcmTargeting, AgcmTargeting);
		AS_REGISTER_METHOD0(void, ShowTarget);
		AS_REGISTER_VARIABLE(int, m_lTargetType);
		AS_REGISTER_VARIABLE(int, m_lTargetID);
		AS_REGISTER_VARIABLE(int, m_lTargetTID);
		AS_REGISTER_VARIABLE(string, m_szTargetName);
		AS_REGISTER_VARIABLE(int, m_lUsePathFind);

		#ifdef _DEBUG
		AS_REGISTER_METHOD0(void, TargetNearestDebug);
		AS_REGISTER_METHOD0(void, TargetNextDebug);
		#endif // _DEBUG
	AS_REGISTER_TYPE_END;

	m_pAgpmShrineBattle = (AgpmShrineBattle*)GetModule("AgpmShrineBattle");

	return TRUE;
}

void	AgcmTargeting::OnLuaInitialize( AuLua * pLua )
{
	luaDef MVLuaGlue[] = 
	{
		{"SendMoveSelfCharacter"		,	LG_SendMoveSelfCharacter	},
		{NULL							,	NULL						},
	};

	for(int i=0; MVLuaGlue[i].name; i++)
	{
		pLua->AddFunction(MVLuaGlue[i].name, MVLuaGlue[i].func);
	}
}


BOOL AgcmTargeting::OnDestroy()
{
	int i;

	ClearPickList();

	for(i=0;i<AGCMTARGETING_DESTMOVE_IMAGE_NUM;++i)
	{
		if(m_pDestMove[i])
		{
			RwTextureDestroy(m_pDestMove[i]);
			m_pDestMove[i] = NULL;
		}
	}

	//@{ 2006/10/26 burumal
	for ( i = 0; i < AGCMTARGETING_AREATARGETING_IMAGE_NUM; ++i )
	{
		if ( m_pAreaTargetingTex[i] )
		{
			RwTextureDestroy(m_pAreaTargetingTex[i]);
			m_pAreaTargetingTex[i] = NULL;
		}
	}
	//@}

	if (m_clSenderWindow.pParent)
	{
		m_clSenderWindow.pParent->DeleteChild(&m_clSenderWindow, TRUE, FALSE);
		m_clSenderWindow.pParent->WindowListUpdate();
		m_clSenderWindow.pParent = NULL;
	}

	return	TRUE;
}

BOOL AgcmTargeting::OnIdle(UINT32 ulClockCount)
{
	PROFILE("AgcmTargeting::OnIdle");

	UINT32	tickdiff = ulClockCount - m_uiLastTick;
	m_uiLastTick = ulClockCount;

	m_bProcessMouseMove = FALSE;
	m_bProtectMove = FALSE;

	//@{ Jaewon 20051209
	// Adjust the player-look-at according to the camera-look-at
	// if the current target mode is NONE or GROUNDCLICK.
	if ( m_iTargetMode == AGCMTARGETING_TARGET_MODE_NONE || 
			m_iTargetMode == AGCMTARGETING_TARGET_MODE_GROUNDCLICK || 
			m_iTargetMode == AGCMTARGETING_TARGET_MODE_AREATARGETING )
	{
		RwV3d at = RwFrameGetLTM(RwCameraGetFrame(GetCamera()))->at;
		at.x *= -1.0f; at.y *= -1.0f; at.z *= -1.0f;
		m_pAgcmCharacter->SetPlayerLookAtTarget(at);
	}
	else
	{
		if( m_pAgcmCharacter->GetSelfCharacter() )
		{
			//@{ Jaewon 20051213
			// Adjust the player-look-at according to the target direction.
			RpClump *pPlayerClump, *pTargetClump;
			RwV3d at, playerCenter, targetCenter;

			pPlayerClump = m_pAgcmCharacter->GetCharacterData(m_pAgcmCharacter->GetSelfCharacter())->m_pClump;
			INT32 lTargetID = m_pAgcmCharacter->GetCharacterData(m_pAgcmCharacter->GetSelfCharacter())->m_lSelectTargetID;
			if(lTargetID != AP_INVALID_CID)
			{
				AgpdCharacter *pAgpdCharacter = m_pAgpmCharacter->GetCharacter(lTargetID);
				if(pAgpdCharacter)
				{
					AgcdCharacter *pAgcdCharacter = m_pAgcmCharacter->GetCharacterData(pAgpdCharacter);
					if(pAgcdCharacter)
					{
						pTargetClump = pAgcdCharacter->m_pClump;

						if(pPlayerClump && pTargetClump && pAgcdCharacter->m_pPickAtomic )
						{
							m_pAgcmCharacter->SetPlayerLookAtFactor(1.0);
							playerCenter = pPlayerClump->stType.boundingSphere.center;
							RwV3dTransformPoint(&playerCenter, &playerCenter, RwFrameGetLTM(RpClumpGetFrame(pPlayerClump)));
							
							//. 2006. 3. 29. nonstopdj
							//. multi pick atomic일 경우 target은 현재 선택한 picking atomic.
							if ( pAgcdCharacter->m_pPickAtomic != pAgcdCharacter->m_pPickAtomic->next )
								targetCenter = pAgcdCharacter->m_pPickAtomic->boundingSphere.center;
							else
								targetCenter = pTargetClump->stType.boundingSphere.center;

							RwV3dTransformPoint(&targetCenter, &targetCenter, RwFrameGetLTM(RpClumpGetFrame(pTargetClump)));						

							//RwV3dSub(&at, &targetCenter, &playerCenter);
							RwV3dSub(&at, &playerCenter, &targetCenter);
							
							if(RwV3dNormalize(&at, &at) > 0.1f)
							{
								//at.x *= -1.0f;
								//at.y *= -1.0f; 
								//at.z *= -1.0f;

								//@{ 2006/05/11 burumal
								// multi picking atomic 캐릭터인 경우
								if ( m_iTargetMode == AGCMTARGETING_TARGET_MODE_ATTACKLOCK &&
									pAgcdCharacter->m_pPickAtomic != pAgcdCharacter->m_pPickAtomic->next )
								{
									m_pAgcmCharacter->SetPlayerAttackDirection(at);
								}
								//@}

								m_pAgcmCharacter->SetPlayerLookAtTarget(at);
							}
						}
					}
				}
			}
			//@} Jaewon
		}
	}
	//@} Jaewon

	return	TRUE;
}

void	AgcmTargeting::OnLButtonDblClk(RwV2d* pPos)
{
	// 처리 
	FLOAT		fMapDistance = (FLOAT)_I32_MAX;
	AuPOS		posDestPoint;
	m_pAgcmMap->GetMapPositionFromMousePosition( (int)pPos->x, (int)pPos->y,  &posDestPoint.x, &posDestPoint.y, &posDestPoint.z, NULL, NULL, &fMapDistance );

	// Login 중일때 먼저 처리 
	if ( m_bCurrentLoginMode )
	{
		// Character 를 선택하다 
		if ( m_pAgcmUILogin && m_listPicking && m_iPickedNum > 0 && fMapDistance >= m_listPicking->fDistance && m_listPicking->eType == ACUOBJECT_TYPE_CHARACTER )
		{
			m_pAgcmUILogin->CharacterDoubleClick( m_listPicking->lID );
		}
	}
	else
	{
		OnLButtonDown(pPos);
		OnLButtonDown(pPos);
	}
}

//@{ Jaewon 20051201
// On the right button double click, pick nothing and just move.
void AgcmTargeting::OnRButtonDblClk(RwV2d* pPos)
{
// 	if(m_bCurrentLoginMode)
// 		return;
// 
// 	FLOAT fMapDistance = (FLOAT)_I32_MAX;
// 	AuPOS posDestPoint;
// 	BOOL bMapPosResult;
// 
// 	//히로익 상태이상 스킬을 맞았으면 리턴
// 	AgpdCharacter	*pcsSelfCharacter	= m_pAgcmCharacter->GetSelfCharacter();
// 	if(m_pAgpmCharacter->IsHitHeroic(pcsSelfCharacter))
// 		return;
// 
// 	if(!m_pAgcmMap->GetMapPositionFromMousePosition((int)pPos->x, (int)pPos->y, &posDestPoint.x, &posDestPoint.y, &posDestPoint.z, NULL, NULL, &fMapDistance))
// 		bMapPosResult = FALSE;
// 	else 
// 		bMapPosResult = TRUE;
// 
// 	if(bMapPosResult)
// 		MoveSelfCharacter(&posDestPoint);
// 	else
// 		MoveSelfCharacter(NULL);
}
//@} Jaewon

void	AgcmTargeting::OnRButtonDown(RwV2d* pPos )
{	
}

void	AgcmTargeting::OnLButtonDown(RwV2d* pPos)
{
	// 처리 
	FLOAT		fMapDistance = (FLOAT)_I32_MAX;
	AuPOS		posDestPoint;
	BOOL		bMapPosResult;
	
	if(!m_pAgcmMap->GetMapPositionFromMousePosition( (int)pPos->x, (int)pPos->y,  &posDestPoint.x, &posDestPoint.y, &posDestPoint.z, NULL, NULL, &fMapDistance ))
		bMapPosResult = FALSE;
	else 
		bMapPosResult = TRUE;

	FLOAT	fRayLength;
	{
		RwLine		pixelRay	;
		RwV3d		stRaySub	;

		RwCameraCalcPixelRay( m_pCamera , &pixelRay , pPos );
		RwV3dSub(&stRaySub, &pixelRay.start, &pixelRay.end);
		fRayLength = RwV3dLength(&stRaySub);
	}

	// Login 중일때 먼저 처리 
	if ( m_bCurrentLoginMode )
	{
		// Character 를 선택하다 
		if ( m_pAgcmUILogin && m_listPicking && m_iPickedNum > 0 && fMapDistance >= m_listPicking->fDistance * fRayLength && m_listPicking->eType == ACUOBJECT_TYPE_CHARACTER )
		{
			m_pAgcmUILogin->CharacterClick( m_listPicking->lID );
		}

		return;
	}

	m_pAgcmCharacter->ClearMoveKey();

	//@{ 2006/10/26 burumal
	if ( m_iTargetMode == AGCMTARGETING_TARGET_MODE_AREATARGETING )
		return;
	//@}

	if(!m_bProcessMouseMove)
	{
		// 여기서 한번 더 Check를 해줘야 한다. (Interval 동안 없어지는 Clump나 Atomic이 있더라.
		OnMouseMove(pPos);
	}

	if(!m_iPickedNum)
	{
		if (m_pAgcmSkill)
			m_pAgcmSkill->SetSelectedPos(&posDestPoint);
		
		// LButtonDown Pick Nothing Callback Function을 호출한다. - steeple
		EnumCallback( AGCMTARGETING_CB_ID_LBUTTONDOWN_PICK_NOTHING, (PVOID)&posDestPoint, NULL);
	}
	else if(m_listPicking && fMapDistance >= m_listPicking->fDistance * fRayLength )
	{
		// LButtonDown Pick Something Callback Function을 호출한다 - 98pastel
		EnumCallback( AGCMTARGETING_CB_ID_LBUTTONDOWN_PICK_SOMETHING, (PVOID)(&m_listPicking->eType), (PVOID)(&m_listPicking->lID) );

		if (m_listPicking->eType == ACUOBJECT_TYPE_NONE)
		{
			if (m_pAgcmSkill)
				m_pAgcmSkill->SetSelectedPos(&posDestPoint);
		}
		else if(m_listPicking->eType == ACUOBJECT_TYPE_CHARACTER )
		{
			if (m_pAgcmSkill)
				m_pAgcmSkill->SetSelectedBase(APBASE_TYPE_CHARACTER, m_listPicking->lID, &posDestPoint);
		}
		//else if(m_listPicking->eType == ACUOBJECT_TYPE_OBJECT )
		//{
			//m_pAgcmOcTree->RemoveClumpFromOcTree(m_listPicking->pClump);
			//m_pAgcmRender->RemoveClumpFromWorld(m_listPicking->pClump);
		//}
	}

	BOOL	bProcessed = FALSE;

	AgpdCharacter	*pcsSelfCharacter	= m_pAgcmCharacter->GetSelfCharacter();
	AgpdSiegeWar	*pcsSiegeWar		= m_pAgpmSiegeWar->GetSiegeWarInfo(pcsSelfCharacter);

	// 케릭터가 죽었으면 걍 리턴시킴..
	if( pcsSelfCharacter && pcsSelfCharacter->IsDead() )
	{
		// -_-;;
		return;
	}

	//히로익 상태이상 스킬을 맞았으면 리턴
	if(m_pAgpmCharacter->IsHitHeroic(pcsSelfCharacter))
		return;

	BOOL	bExistSiegeEnemy	= FALSE;

	BOOL	bIsAttackGuild		= m_pAgpmSiegeWar->IsAttackGuild(pcsSelfCharacter, pcsSiegeWar);
	BOOL	bIsDefenseGuild		= m_pAgpmSiegeWar->IsDefenseGuild(pcsSelfCharacter, pcsSiegeWar);
	
	if ((bIsAttackGuild || bIsDefenseGuild) && pcsSiegeWar && pcsSelfCharacter)
	{
		PickedObject *pstPick = m_listPicking;
		while(pstPick)
		{
			if (pstPick->eType == ACUOBJECT_TYPE_CHARACTER &&
				(AgpdCharacter *) pstPick->pcsBase != pcsSelfCharacter &&
				((AgpdCharacter *) pstPick->pcsBase)->m_unActionStatus != AGPDCHAR_STATUS_DEAD)
			{
				if (bIsAttackGuild && m_pAgpmSiegeWar->IsDefenseGuild((AgpdCharacter *) pstPick->pcsBase, pcsSiegeWar) ||
					bIsDefenseGuild && m_pAgpmSiegeWar->IsAttackGuild((AgpdCharacter *) pstPick->pcsBase, pcsSiegeWar))
				{
					bExistSiegeEnemy	= TRUE;
					break;
				}
			}

			pstPick	= pstPick->next;
		}
		
	}	
	else
	{		
	}
		
	// 처리 
	PickedObject*	cur_PN = m_listPicking;
	while(cur_PN)
	{
		if(cur_PN->fDistance > fMapDistance)
			break;

		if( (NULL != cur_PN->pcsBase) && (m_pAgcmEventManager->CheckEvent(cur_PN->pcsBase, m_pAgcmCharacter->m_pcsSelfCharacter)))
		{
			// 마고자 (2005-05-17 오후 6:36:20) : 
			// 마지막 선택한 NPC ID를 기록함.
			// NPC 사운드 처리용.

			m_lLastClickNPCID = -1;
			if(cur_PN->eType == ACUOBJECT_TYPE_CHARACTER )
			{
				AgpdCharacter *	pcsCharacter = (AgpdCharacter *) cur_PN->pcsBase;
				m_lLastClickNPCID	= pcsCharacter->m_lID;
			}
			// 마고자 (2005-05-17 오후 6:37:23) : 요까지.

			bProcessed = TRUE;
			break;
		}

		if(cur_PN->eType == ACUOBJECT_TYPE_CHARACTER )
		{
			AgpdCharacter *	pcsCharacter = (AgpdCharacter *) cur_PN->pcsBase;

			BOOL	bIsSkip	= FALSE;

			if (m_pAgpmSiegeWar &&
				pcsCharacter &&
				m_pAgpmSiegeWar->IsSiegeWarMonster(pcsCharacter))
			{
				if (bExistSiegeEnemy)
					bIsSkip = TRUE;
				else if (pcsCharacter->m_unActionStatus == AGPDCHAR_STATUS_DEAD &&
					(m_pAgpmSiegeWar->GetSiegeWarMonsterType(pcsCharacter) != AGPD_SIEGE_MONSTER_THRONE &&
					 m_pAgpmSiegeWar->GetSiegeWarMonsterType(pcsCharacter) != AGPD_SIEGE_MONSTER_ATTACK_RESURRECTION_TOWER &&
					 m_pAgpmSiegeWar->GetSiegeWarMonsterType(pcsCharacter) != AGPD_SIEGE_MONSTER_CATAPULT))
				{
					bIsSkip	= TRUE;
				}
			}

			// 2006.12.27. steeple
			// 트랩은 타겟하지 않는다.
			if (m_pAgpmCharacter->IsTrap(pcsCharacter))
				bIsSkip = TRUE;

			if (!bIsSkip)
			{
				if( GetKeyState( VK_SHIFT ) < 0 )
				{
					if(!m_pAgpmShrineBattle->IsInShrineRegionByIndex(pcsCharacter))
					{
						m_pAgcmUICharacter->OpenCharacterMenu((ApBase *) pcsCharacter);

						bProcessed = TRUE;
					}
				}
				else if (cur_PN->lID != m_pAgcmCharacter->m_lSelfCID)
				{
					//쉬라인 //JK_쉬라인머지
					if( m_pAgcmShrine && m_pAgcmShrine->ClickShrine(m_pAgcmCharacter->GetSelfCharacter(), pcsCharacter) )
					{
						m_pAgcmCharacter->SelectTarget(m_pAgcmCharacter->GetSelfCharacter(), cur_PN->lID);
					}
					//공성 
					else if (m_pAgcmSiegeWar && m_pAgcmSiegeWar->ClickCharacter(pcsCharacter))
					{
						m_pAgcmCharacter->SelectTarget(m_pAgcmCharacter->GetSelfCharacter(), cur_PN->lID);
					}
					else
					{

						if (m_pAgcmCharacter->IsSelectTarget(m_pAgcmCharacter->GetSelfCharacter()) &&
							m_pAgcmCharacter->GetSelectTarget(m_pAgcmCharacter->GetSelfCharacter()) == cur_PN->lID)
						{
							// Control 누르고 공격하면 강제공격
							if ( GetKeyState( VK_CONTROL ) < 0 )
							{
								//@{ 2006/03/06 burumal 
								//m_pAgcmCharacter->LockTarget(m_pAgcmCharacter->GetSelfCharacter(), cur_PN->lID, TRUE);
								m_pAgcmCharacter->LockTarget(m_pAgcmCharacter->GetSelfCharacter(), cur_PN->lID, cur_PN->pNewFirstPickAtomic, TRUE);
								//@}

								m_pAgcmCharacter->ActionSelfCharacter(AGPDCHAR_ACTION_TYPE_ATTACK, cur_PN->lID, 0, TRUE );
							}
							else 
							{
								//@{ 2006/03/06 burumal
								//m_pAgcmCharacter->LockTarget(m_pAgcmCharacter->GetSelfCharacter(), cur_PN->lID);
								m_pAgcmCharacter->LockTarget(m_pAgcmCharacter->GetSelfCharacter(), cur_PN->lID, cur_PN->pNewFirstPickAtomic);
								//@}

								m_pAgcmCharacter->ActionSelfCharacter(AGPDCHAR_ACTION_TYPE_ATTACK, cur_PN->lID);
							}
						}
						else
						{
							m_pAgcmCharacter->SelectTarget(m_pAgcmCharacter->GetSelfCharacter(), cur_PN->lID);
						}
					}

					bProcessed = TRUE;

					if(m_iTargetMode == AGCMTARGETING_TARGET_MODE_GROUNDCLICK)
					{
						if (m_pAgcmGlyph && m_bShowGlyph && m_pMoveTarget)
						{
							m_pAgcmGlyph->DeleteGlyph(m_pMoveTarget);

							// 3D DestMove Target Disable
							m_pAgcmGlyph->StopDestMoveTarget();
						}
						
						m_pMoveTarget = NULL;
					}

					if(m_pEnemyTargetClump != cur_PN->pClump)
					{
						RwFrame*	frame = RpClumpGetFrame(cur_PN->pClump);
						INT32		lRange = 0;
						m_pAgpmFactors->GetValue(&((AgpdCharacter *) cur_PN->pcsBase)->m_csFactor, &lRange,AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_HITRANGE);

						if (m_pAgcmGlyph && m_bShowGlyph)
						{
							if(lRange)
							{
								FLOAT		range_val = (FLOAT)lRange * 2.0f;
								m_pAgcmGlyph->StartEnemyTarget(frame, cur_PN->pClump,range_val);
							}
							else
							{
								m_pAgcmGlyph->StartEnemyTarget(frame,cur_PN->pClump,150.0f);
							}
						}
						
						if(m_pEnemyTargetClump)
						{
							m_pAgcmTextBoardMng->DisableHPBar(m_pEnemyTargetClump);
						}

						if(m_listPicking == cur_PN && m_pAgcmTextBoardMng)
						{
							m_pAgcmTextBoardMng->EnableHPBar(cur_PN->pcsBase);
						}

						m_pEnemyTargetClump = cur_PN->pClump;
						m_iTargetMode = AGCMTARGETING_TARGET_MODE_ATTACK;
					}
				}
			}

			if (bProcessed)
				break;
		}
		else if (cur_PN->eType == ACUOBJECT_TYPE_ITEM)
		{
			// 아이템 줍기
			if (m_pAgcmItem->PickupItem(m_pAgpmItem->GetItem(cur_PN->lID)))
			{
				bProcessed = TRUE;
				break;
			}
		}

		cur_PN = cur_PN->next;
	}

	// 이전에 처리한 것이 없거나, 죽은 케릭터를 찍지 않았을때.
	if (!bProcessed && !m_bProtectMove)
	{
		// 마고자 (2005-03-16 오후 2:52:48) : 펑션으로 수정함..
		// 외부에서 이동 명령을 호출할때 사용하기 위함.

		AgpdCharacter *pcsSelfCharacter = m_pAgcmCharacter->GetSelfCharacter();

		if( pcsSelfCharacter && pcsSelfCharacter->m_ulSpecialStatus & AGPDCHAR_SPECIAL_STATUS_DONT_MOVE )
		{
			// 블럭걸려있으니까 아무짓도 안함
		}
		else
		{
			if( bMapPosResult )
			{
				MoveSelfCharacter( &posDestPoint );
			}
			else
			{
				MoveSelfCharacter( NULL );
			}
		}
	}	
}

BOOL	AgcmTargeting::TargetCID		( INT32 nCID , BOOL bAttack )
{
	//@{ 2006/10/26 burumal
	if ( m_iTargetMode == AGCMTARGETING_TARGET_MODE_AREATARGETING )
		return FALSE;
	//@}

	m_pAgcmCharacter->SelectTarget(m_pAgcmCharacter->GetSelfCharacter(), nCID );

	if(m_iTargetMode == AGCMTARGETING_TARGET_MODE_GROUNDCLICK)
	{
		if (m_pAgcmGlyph && m_bShowGlyph && m_pMoveTarget)
		{
			m_pAgcmGlyph->DeleteGlyph(m_pMoveTarget);

			// 3D DestMove Target Disable
			m_pAgcmGlyph->StopDestMoveTarget();
		}
		
		m_pMoveTarget = NULL;
	}

	AgpdCharacter * pCharacter = m_pAgpmCharacter->GetCharacter( nCID );
	AgcdCharacter	*pcdCharacter;

	if( NULL == pCharacter ) return FALSE;

	pcdCharacter = m_pAgcmCharacter->GetCharacterData( pCharacter );
	if( NULL == pcdCharacter || NULL == pcdCharacter->m_pClump ) return FALSE;

	if(m_pEnemyTargetClump != pcdCharacter->m_pClump )
	{
		RwFrame*	frame = RpClumpGetFrame( pcdCharacter->m_pClump );
		INT32		lRange = 0;
		m_pAgpmFactors->GetValue(&pCharacter->m_csFactor, &lRange,AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_HITRANGE);

		if (m_pAgcmGlyph && m_bShowGlyph)
		{
			if(lRange)
			{
				FLOAT		range_val = (FLOAT)lRange * 2.0f;
				m_pAgcmGlyph->StartEnemyTarget(frame,pcdCharacter->m_pClump,range_val);
			}
			else
			{
				m_pAgcmGlyph->StartEnemyTarget(frame,pcdCharacter->m_pClump,150.0f);
			}
		}
		
		if(m_pEnemyTargetClump)
		{
			m_pAgcmTextBoardMng->DisableHPBar(m_pEnemyTargetClump);
		}

		if(m_pAgcmTextBoardMng)
		{
			m_pAgcmTextBoardMng->EnableHPBar(pCharacter);
		}

		m_pEnemyTargetClump = pcdCharacter->m_pClump;
		m_iTargetMode = AGCMTARGETING_TARGET_MODE_ATTACK;

		if( bAttack )
		{
			m_pAgcmCharacter->LockTarget(m_pAgcmCharacter->GetSelfCharacter(), nCID, pcdCharacter->m_pPickAtomic);
			m_pAgcmCharacter->ActionSelfCharacter(AGPDCHAR_ACTION_TYPE_ATTACK, nCID);
		}
	}

	return TRUE;
}

void	AgcmTargeting::MoveSelfCharacter( AuPOS *pPos )
{
	//. 2006. 3. 8. Nonstopdj
	//. NULL체크 추가.

	if( NULL == pPos ) return;

	AgpdCharacter * pCharacter = m_pAgcmCharacter->GetSelfCharacter();
	if( !pCharacter )
		return;

	//@{ 2006/10/26 burumal
	if ( m_iTargetMode == AGCMTARGETING_TARGET_MODE_AREATARGETING )
		return;
	//@}

	AuPOS	vValid = * pPos;
	BOOL		bIsBlockingPosition	= FALSE;
	ApmMap::BLOCKINGTYPE eType = m_pAgpmCharacter->GetBlockingType( pCharacter );

	#ifdef _DEBUG
	static bool _sbCheckBlock = true;
	if( _sbCheckBlock )
	{
	#endif
		if( pPos && m_pApmMap->CheckBlockingInfo( *pPos , eType ) & ApmMap::GEOMETRY_BLOCKING )
			bIsBlockingPosition	= TRUE;

		// 마고자 (2005-12-21 오후 5:39:37) : 
		// 블러킹이 없는 곳까지의 포지션으로 수정함.

		if( pPos											&& 
			!m_pApmMap->GetValidDestination( 
			&pCharacter->m_stPos	,
			&vValid											,
			&vValid											,
			pCharacter->m_pcsCharacterTemplate->m_fSiegeWarCollSphereRadius ,
			eType											) )
		{
			TRACE( "GetValidDestination 체크걸림! \n" );
			// 체크됐다.
			pPos = &vValid;
		}

		if( pPos											&& 
			m_pAgcmCharacter->GetValidDestination( 
			&pCharacter->m_stPos	,
			&vValid											,
			&vValid											) )
		{
			TRACE( "GetValidDestination 체크걸림! \n" );
			// 체크됐다.
			pPos = &vValid;
		}

		if( pPos											&& 
			m_pAgpmCharacter->GetValidDestination( 
			&pCharacter->m_stPos	,
			&vValid											,
			&vValid											,
			eType											,
			pCharacter										) )
		{
			TRACE( "GetValidDestination 케릭터 체크걸림! \n" );
			// 체크됐다.
			pPos = &vValid;
		}
	#ifdef _DEBUG
	}
	#endif 
	
	// 거리체크..
	// 2미터 이상이면 그냥 뛰어감.
	// 가다 걸리겠지머.
	if( bIsBlockingPosition )
	{
		FLOAT	fDist = AUPOS_DISTANCE_XZ( pCharacter->m_stPos , ( *pPos ) );

		const	FLOAT cBlockingLimitDistance = 200.0f; // 2미터 제한..

		if( cBlockingLimitDistance < fDist )
		{
			// 그냥 지나가라..
			bIsBlockingPosition = FALSE; 
		}
	}

	if( pPos && !bIsBlockingPosition)
	{
		// 마고자 (2005-03-11 오후 3:09:47) : 
		// 패스파인딩 사용함.
		m_pAgcmCharacter->MoveSelfCharacter( pPos, MD_NODIRECTION , m_lUsePathFind, 0 );
	}

	if(m_iTargetMode == AGCMTARGETING_TARGET_MODE_ATTACKLOCK)
	{
		// 타겟락 해제
		if (m_pAgcmGlyph && m_bShowGlyph)
			m_pAgcmGlyph->StopEnemyTarget();

		/*
		if (m_pAgcmTextBoard)
			m_pAgcmTextBoard->DisableHpBar(m_pEnemyTargetClump);

		m_pEnemyTargetClump = NULL;
		*/

		m_pAgcmCharacter->UnLockTarget(m_pAgcmCharacter->GetSelfCharacter());
	}
	else if(m_iTargetMode == AGCMTARGETING_TARGET_MODE_ATTACK)
	{
		// 타겟팅 모드 해제
		if (m_pAgcmGlyph && m_bShowGlyph)
			m_pAgcmGlyph->StopEnemyTarget();

		/*
		if (m_pAgcmTextBoard)
			m_pAgcmTextBoard->DisableHpBar(m_pEnemyTargetClump);

		m_pEnemyTargetClump = NULL;
		*/

		m_pAgcmCharacter->UnLockTarget(m_pAgcmCharacter->GetSelfCharacter());
	}
	
	if( pPos )
	{
		// 마고자 (2005-03-16 오후 2:53:09) : 
		// Valid 한 포지션이 아닌경우에도 이 연산이 이루어져서
		// 일단 통과하지 못하게 막아둠.

		if(m_pMoveTarget == NULL)
		{
			if (m_pAgcmGlyph && m_bShowGlyph)
			{
				RwV3d	dPos;
				dPos.x = pPos->x;
				dPos.y = pPos->y;
				dPos.z = pPos->z;

				m_pMoveTarget = m_pAgcmGlyph->AddStaticGlyph(m_pDestMove[0],&dPos,70.0f,70.0f);

				// 3D DestMove Target Enable
				m_pAgcmGlyph->StartDestMoveTarget(&dPos,100.0f,m_pMoveTarget);
			}
		}
		else
		{
			m_pAgcmGlyph->UpdatePosStaticGlyph(m_pMoveTarget,pPos->x,pPos->y,pPos->z);
			
			// 3D DestMove Target Pos Update
			m_pAgcmGlyph->StartDestMoveTarget(&m_pMoveTarget->staticpos, 100.0f,m_pMoveTarget);
		}
		
		m_iTargetMode = AGCMTARGETING_TARGET_MODE_GROUNDCLICK;
	}
}

void	AgcmTargeting::OnLButtonUp(RwV2d *pPos)
{
	if (!pPos)		return;

	//@{ 2006/10/26 burumal	
	if ( OnAreaTargetingDestPosDetected(pPos) )
		return;
	//@}

	return;
}

void	AgcmTargeting::OnMouseMove(RwV2d*	pPos)
{
	PROFILE("AgcmTargeting::OnMouseMove");

	if(!m_bProcessMouseMove)
		m_bProcessMouseMove = TRUE;
	else 
		return;

	if (m_pAgcmUIManager2 && m_pAgcmUIManager2->GetMouseOnUI())
		return;

	BOOL		bMapPosResult;
	AuPOS		posDestPoint;
	FLOAT		fMapDistance;

	if(!m_pAgcmMap->GetMapPositionFromMousePosition( (int)pPos->x, (int)pPos->y, &posDestPoint.x, &posDestPoint.y, &posDestPoint.z, NULL, NULL, &fMapDistance ))
		bMapPosResult = FALSE;
	else 
		bMapPosResult = TRUE;

	//@{ 2006/10/26 burumal
	if ( m_pAgcmGlyph  && bMapPosResult )
		UpdateAreaTargeting(&posDestPoint);
	//@}

	RpIntersection	intersection	;
	RwLine			pixelRay		;
	RwV2d			pos				;
	AgpdCharacter *	pcsCharacter	;

	PickedObject *	pstPick			;
	PickedObject *	pstPickPrev		;
	PickedObject *	pstPickNext		;

	RpClump *		pstPrevMouseClump = m_pEnemyMouseOnClump;

	m_pEnemyMouseOnClump	= NULL;
	
	pos				= *pPos;
	m_vCurMousePos	= pos;

	RwCameraCalcPixelRay( m_pCamera , &pixelRay , &pos );

	intersection.type	=	rpINTERSECTLINE	;
	intersection.t.line	=	pixelRay		;

	ClearPickList();
	m_iPickedNum = 0;

	BOOL	bTargetLockValid = false;

	{
		PROFILE("AgcmTargeting-intersection");
		if(!m_pApmOcTree || !m_pApmOcTree->m_bOcTreeEnable)
		{
			if(m_pAgcmMap->m_bUseCullMode)
			{
				m_pAgcmMap->LoadingSectorsForAllAtomicsIntersection2( 0 , &intersection, AgcmTargeting::CBIntersectAtomic, (void *) this );
			}
			else
			{
				m_pAgcmRender->RenderWorldForAllIntersections(&intersection, CBIntersectAtomic, (void *) this );
			}
		}
		else	
		{
			m_pAgcmOcTree->OcTreesForAllAtomicsIntersection2(0 , &intersection, AgcmTargeting::CBIntersectAtomic, (void *) this );
		}
	}

	// List에서 제거할 넘들은 미리 제거한다.
	{
		// 유효한 타겟이 리스트에 있으면 죽은넘은 무시되게...(Parn, 2005/2/11)
		BOOL	bRemoveDead	= FALSE;

		pstPick = m_listPicking;
		while(pstPick)
		{
			pstPickNext = pstPick->next;

			if( m_pAgcmCharacter->m_pMainCharacterClump == m_listPicking->pClump || 
				m_pAgcmCharacter->GetSelfRideClump() == m_listPicking->pClump )
			{
				if (pstPick == m_listPicking)
					m_listPicking = pstPick->next;
				else
					pstPickPrev->next = pstPick->next;

				delete pstPick;

				--m_iPickedNum;
			}
			else
			{
				pstPickPrev = pstPick;

				if( (pstPick->eType == ACUOBJECT_TYPE_CHARACTER && 
				    ((AgpdCharacter *) pstPick->pcsBase)->m_unActionStatus != AGPDCHAR_STATUS_DEAD )||
					pstPick->eType == ACUOBJECT_TYPE_ITEM )
				{
					bRemoveDead = TRUE;
					break;
				}
			}

			pstPick = pstPickNext;
		}
		// 죽은넘은 모두 뺀다.
		if (bRemoveDead)
		{
			pstPick = m_listPicking;
			pstPickPrev = NULL;
			while (pstPick)
			{
				pstPickNext = pstPick->next;

				if (pstPick->eType == ACUOBJECT_TYPE_CHARACTER && ((AgpdCharacter *) pstPick->pcsBase)->m_unActionStatus == AGPDCHAR_STATUS_DEAD)
				{
					if (pstPick == m_listPicking)
						m_listPicking = pstPick->next;
					else
						pstPickPrev->next = pstPick->next;

					delete pstPick;

					--m_iPickedNum;
				}
				else
				{
					pstPickPrev = pstPick;
				}

				pstPick = pstPickNext;
			}
		}
	}

//ride처리.
	pstPick = m_listPicking;
	while(pstPick)
	{
		pstPickNext = pstPick->next;
		if( pstPick->eType == ACUOBJECT_TYPE_CHARACTER ) {
            AgcdCharacter *pAgcdCharacter = m_pAgcmCharacter->GetCharacterData( (AgpdCharacter *)pstPick->pcsBase );
			if( pAgcdCharacter->m_pOwner ) {
                pstPick->pcsBase = m_pAgcmCharacter->GetCharacter( pAgcdCharacter->m_pOwner );
				pstPick->lID = pstPick->pcsBase->m_lID;
			}
		}
		pstPick = pstPickNext;
	}

	RwV3d	stRaySub;
	FLOAT	fRayLength;

	RwV3dSub(&stRaySub, &pixelRay.start, &pixelRay.end);
	fRayLength = RwV3dLength(&stRaySub);

	AgpdCharacter * pCharacter = m_pAgcmCharacter->GetSelfCharacter();
	ApmMap::BLOCKINGTYPE eType = m_pAgpmCharacter->GetBlockingType( pCharacter );

	BOOL bWorkLight = FALSE;

	if(m_iPickedNum == 0 || !m_listPicking || !m_listPicking->pcsBase || m_listPicking->fDistance * fRayLength  >= fMapDistance)
	{
		SetTargetBase(NULL);

		BOOL bIsBlockingPosition = ApTileInfo::BLOCKGEOMETRY & m_pApmMap->CheckBlockingInfo( posDestPoint , eType );

		if( bIsBlockingPosition )
		{
			AgpdCharacter * pCharacter = m_pAgcmCharacter->GetSelfCharacter();

			if( pCharacter )
			{
				FLOAT	fDist = AUPOS_DISTANCE_XZ( pCharacter->m_stPos , posDestPoint );

				const	FLOAT cShowBlockingLimitDistance = 5000.0f; // 2미터 제한..

				if( cShowBlockingLimitDistance < fDist )
				{
					// 그냥 지나가라..
					bIsBlockingPosition = FALSE; 
				}
			}
		}

		if ( bMapPosResult && bIsBlockingPosition )
			m_iMouseCursorMode = m_lCursorNoMove;
		else
			m_iMouseCursorMode = m_lCursorMove;
		
		if (m_pAgcmUIManager2)
		{
			m_pAgcmUIManager2->ChangeCursor(m_iMouseCursorMode, this);
			m_pAgcmUIManager2->SetTarget(NULL);
		}
	}
	else
	{
		SetTargetBase(m_listPicking->pcsBase);

		if (m_pAgcmUIManager2)
			m_pAgcmUIManager2->SetTarget(m_listPicking->pcsBase);

		if( m_listPicking && m_listPicking->pClump 
			&& ( m_listPicking->eType == ACUOBJECT_TYPE_CHARACTER 
			|| m_listPicking->eType == ACUOBJECT_TYPE_ITEM  ) )
		{
			//다른 오브젝트를 픽킹했다.
			if(  m_pPrevPickingClump && m_pPrevPickingClump != m_listPicking->pClump )
				m_pAgcmRender->ResetLighting( m_pPrevPickingClump );


			//아이템은 외각선 필요 없지...
			if( m_listPicking->eType != ACUOBJECT_TYPE_ITEM  )
				m_pAgcmGlyph->SetMouseOnTargetClump(m_listPicking->pClump);

			m_pPrevPickingClump = m_listPicking->pClump;

			bWorkLight = TRUE;
		}


		// 이벤트일 경우
		{
			ApdEventAttachData	*pcsAttachData = m_pApmEventManager->GetEventData(m_listPicking->pcsBase);
			if (!pcsAttachData)
			{
				return;
			}

			for (INT32 lIndex = 0; lIndex < pcsAttachData->m_unFunction; ++lIndex)
			{
				if (pcsAttachData->m_astEvent[lIndex].m_eFunction > APDEVENT_FUNCTION_NONE &&
					pcsAttachData->m_astEvent[lIndex].m_eFunction < APDEVENT_MAX_FUNCTION)
				{
					// QUEST의 경우, 현재 Check Point인 Target만 커서를 세팅
					if (pcsAttachData->m_astEvent[lIndex].m_eFunction == APDEVENT_FUNCTION_QUEST)
					{
						AgcmMinimap::MPInfo *	pstMPInfo	= m_pAgcmUICharacter->GetBaseMinimapInfo(m_listPicking->pcsBase);
						if (!pstMPInfo ||
							(pstMPInfo->nType != AgcmMinimap::MPInfo::MPI_QUESTINCOMPLETE &&
							 pstMPInfo->nType != AgcmMinimap::MPInfo::MPI_QUESTINCOMPLETE))
							 continue;

					}
					// pcsAttachData->m_astEvent[lIndex].m_eFunction 이 Event의 Function Type이당.
					// 여기서 해당 Event에 대해서 Cursor가 있으면, 바꿔주자.
					// 나중에 여기에 더 자세한 코드가 있어야 된다. (예를 들면, Teleport Event가 Character에 붙어있으면 여기를 무시한다던가.)
					m_iMouseCursorMode = m_lCursorTalk;
					
					if (m_pAgcmUIManager2)
						m_pAgcmUIManager2->ChangeCursor(m_iMouseCursorMode, this);

					// 아직까지 외곽선 처리가 안되었고, 가이드 오브젝트라면
					if( bWorkLight == FALSE
						&& pcsAttachData->m_astEvent[lIndex].m_eFunction == APDEVENT_FUNCTION_GUIDE )
					{
						//다른 오브젝트를 픽킹했다.
						if(  m_pPrevPickingClump && m_pPrevPickingClump != m_listPicking->pClump )
							m_pAgcmRender->ResetLighting( m_pPrevPickingClump );

						m_pAgcmRender->CustomizeLighting( m_listPicking->pClump, LIGHT_PICKING );

						m_pAgcmGlyph->SetMouseOnTargetClump(m_listPicking->pClump);

						m_pPrevPickingClump = m_listPicking->pClump;

						bWorkLight = TRUE;
					}
	
					return;
				}
			}
		}

		switch (m_listPicking->eType)
		{
		case ACUOBJECT_TYPE_CHARACTER:
			{
				pcsCharacter = (AgpdCharacter *) m_listPicking->pcsBase;

				if (pcsCharacter->m_unActionStatus != AGPDCHAR_STATUS_DEAD)
					m_pEnemyMouseOnClump = m_listPicking->pClump;

				// 마고자 (2005-07-26 오후 6:31:32) : 
				// 매크로 방지를 위해서 몬스커의 공격 커서 변경을 하지 않게 주석처리함.
				//if (m_pAgpmCharacter->IsMonster((AgpdCharacter *) m_listPicking->pcsBase) &&
				//	m_pAgpmCharacter->IsTargetable((AgpdCharacter *) m_listPicking->pcsBase))
				//	m_iMouseCursorMode = m_lCursorAttack;
				//else
				//	m_iMouseCursorMode = m_lCursorMove;

				m_iMouseCursorMode = m_lCursorMove;

				if (m_pAgcmUIManager2)
					m_pAgcmUIManager2->ChangeCursor(m_iMouseCursorMode, this);
			}
			break;

		case ACUOBJECT_TYPE_ITEM:
			{
				m_iMouseCursorMode = m_lCursorPick;

				if (m_pAgcmUIManager2)
					m_pAgcmUIManager2->ChangeCursor(m_iMouseCursorMode, this);
			}
			break;

		case ACUOBJECT_TYPE_OBJECT:
			{
				m_iMouseCursorMode = m_lCursorMove;

				if (m_pAgcmUIManager2)
					m_pAgcmUIManager2->ChangeCursor(m_iMouseCursorMode, this);
			}
		}
	}

	if( FALSE == bWorkLight )
	{
		if( m_pPrevPickingClump )
		{
			m_pAgcmRender->ResetLighting( m_pPrevPickingClump );
			m_pPrevPickingClump = NULL;
			m_pAgcmGlyph->SetMouseOnTargetClump(NULL);
		}
	}

	//if(m_iTargetMode != AGCMTARGETING_TARGET_MODE_ATTACKLOCK)
	if (m_pAgcmTextBoardMng)
	{
		if(m_pEnemyMouseOnClump != pstPrevMouseClump) // RenderCB 복귀
		{
			if(pstPrevMouseClump && m_pEnemyTargetClump != pstPrevMouseClump)
			{
				m_pAgcmTextBoardMng->DisableHPBar(pstPrevMouseClump);
				//. mouse roll over시에 실루엣이 그려질 clump설정
				m_pAgcmGlyph->SetMouseOnTargetClump(NULL);
			}

			if (!m_bCurrentLoginMode && m_listPicking && m_listPicking->pcsBase)
			{
				m_pAgcmTextBoardMng->EnableHPBar(m_listPicking->pcsBase);
				//. mouse roll over시에 실루엣이 그려질 clump설정
				m_pAgcmGlyph->SetMouseOnTargetClump(m_pEnemyMouseOnClump);
			}

			if( m_listPicking && m_listPicking->pcsBase && m_listPicking->eType == ACUOBJECT_TYPE_CHARACTER ) 
			{
				AgcdCharacter *pAgcdCharacter = m_pAgcmCharacter->GetCharacterData( (AgpdCharacter *)m_listPicking->pcsBase );
				if(pAgcdCharacter 
					&& pAgcdCharacter->m_pRide 
					&& pAgcdCharacter->m_pRide->m_pClump == m_pEnemyMouseOnClump)
				{
					//. 탈것의 edge는 무시한다.
					m_pAgcmGlyph->SetMouseOnTargetClump(NULL);
				}
			}
		}
	}
}

RpAtomic*	AgcmTargeting::CBIntersectAtomic(
							 RpIntersection *	intersection	,
							 RpWorldSector *	sector			,
							 RpAtomic *			atomic			,
							 RwReal				distance		,
							 void *				data			)
{
	PROFILE("AgcmTargeting::CBIntersectAtomic");
	AgcmTargeting		*pThis = (AgcmTargeting *) data;
	
	INT32				eType;
	INT32				lID;
	INT32				lObjectType;
	RpClump				*pstClump;
	ApBase				*pcsBase;
	
	RpAtomic			*pstPickAtomic;
	RpGeometry			*pstPickGeometry;

	pstClump = RpAtomicGetClump(atomic);
	if (pstClump)
		lObjectType = AcuObject::GetClumpType(pstClump, &lID, (void **) &pcsBase, NULL, NULL, (void **) &pstPickAtomic);
	else
		lObjectType = AcuObject::GetAtomicType(atomic, &lID, (void **) &pcsBase, NULL, NULL, (void **) &pstPickAtomic);

	eType = AcuObject::GetType(lObjectType);
	//@{ Jaewon 20050912
	// 'ACUOBJECT_TYPE_INVISIBLE' checking added.
	INT32 flag = AcuObject::GetProperty(lObjectType);

	if (eType != ACUOBJECT_TYPE_NONE && eType != ACUOBJECT_TYPE_WORLDSECTOR && !(flag & ACUOBJECT_TYPE_INVISIBLE))
	//@} Jaewon
	{
		/*
		bool	bcheckrealatomic	= true;
		if (!pstPickAtomic || !RpAtomicGetGeometry(pstPickAtomic))
		{
			bcheckrealatomic = false;
			pstPickAtomic = atomic;
		}

		pstPickGeometry = RpAtomicGetGeometry(pstPickAtomic);		
		
		if ( NULL != pstPickGeometry && RpGeometryGetTriangles(pstPickGeometry) )
		{
			// 여기에 intersection을 pstPickAtomic의 LTM으로 변환하는 코드가 필요하다. (연준아...^^;)
			RpIntersection	stIntersection = *(intersection);

			RwMatrix	matrixPickAtomicLTMInverse;

			pThis->LockFrame();
			RwMatrixInvert( &matrixPickAtomicLTMInverse , RwFrameGetLTM( RpAtomicGetFrame( pstPickAtomic ) ) );
			pThis->UnlockFrame();
		
			RwV3dTransformPoint( &(stIntersection.t.line.start), &(stIntersection.t.line.start), &matrixPickAtomicLTMInverse );
			RwV3dTransformPoint( &(stIntersection.t.line.end), &(stIntersection.t.line.end), &matrixPickAtomicLTMInverse );

			// 코드 열라 지저분해진다... 그러나, 귀찮다. -_-; (parn)
			FLOAT	fDistance;
			FLOAT	fDistanceOrg;
			
			// Blocking Object이면, 세로로 된 Blocking Polygon은 무시한다.
			//if (eType == ACUOBJECT_TYPE_OBJECT && (ACUOBJECT_TYPE_BLOCKING & lObjectType))
				//fDistance = 0;
			//else
				fDistance = 1;

			fDistanceOrg = fDistance;

			RpCollisionGeometryForAllIntersections(pstPickGeometry, &stIntersection, CBDetailCollision, &fDistance );
			
			if ( fDistance == fDistanceOrg )
			{
				return atomic;	// List에 추가하지 않고 Return
			}

			//@{ kday 20050722
			// ;)
			// 가까운 넘들은.. 실폴리곤으로 함 더한다..
			// 베틀스퀘어에서.. 황금박쥐 때리다가.. 도망가기 힘들어서..
			if( bcheckrealatomic )
			{
				RwReal	realdistance = fDistance;
				RwV3d	s2e = { intersection->t.line.end.x - intersection->t.line.start.x
					, intersection->t.line.end.y - intersection->t.line.start.y
					, intersection->t.line.end.z - intersection->t.line.start.z
				};
				realdistance *=  RwV3dLength( &s2e );

				if( realdistance < 800.f )
				{
					fDistance = 1;
					fDistanceOrg = fDistance;

					pstPickGeometry = RpAtomicGetGeometry(atomic);

					//. 2005. 12. 27. Nonstopdj
					//. '앵커빌'의 입구쪽 NPC 수비대장의 장비 atomic이 거리에 따라 다시 로딩되지 않을 때 문제가 발생. By Crash Report..
					//. atomic->geometry의 NULL Check추가로 일단 막음.
					if(pstPickGeometry != NULL)
						RpCollisionGeometryForAllIntersections(pstPickGeometry, &stIntersection, CBDetailCollision, &fDistance );
					else
						return (RpAtomic*)NULL;

					//RpCollisionGeometryForAllIntersections(pstPickGeometry, &stIntersection, CBDetailCollision, &fDistance );

					if ( fDistance == fDistanceOrg )
					{
						return atomic;	// List에 추가하지 않고 Return
					}
				}				
			}
			//@} kday
			
			// list에 추가
			pThis->AddPickedObject(pstClump,pcsBase,eType,lID,fDistance);
			
			++(pThis->m_iPickedNum);
		}
		*/

		//@{ 2006/12/15 burumal
		if ( eType == ACUOBJECT_TYPE_CHARACTER )
		{
			ASSERT(pThis->m_pAgcmCharacter);
			ASSERT(pcsBase);

			if ( pstClump && pstClump->stType.pTemplate )
			{				
				if ( pThis->m_pAgcmCharacter->IsNonPickingTID(((AgpdCharacterTemplate*) pstClump->stType.pTemplate)->m_lID) )
					return atomic;
			}
		}
		//@}

		RpAtomic* pFirst = pstPickAtomic;	

		do {

			bool bCheckRealAtomic = true;

			if ( !pstPickAtomic || !RpAtomicGetGeometry(pstPickAtomic) )
			{
				bCheckRealAtomic = false;
				pstPickAtomic = atomic;
			}

			pstPickGeometry = RpAtomicGetGeometry(pstPickAtomic);

			if ( NULL != pstPickGeometry && RpGeometryGetTriangles(pstPickGeometry) )
			{
				// 여기에 intersection을 pstPickAtomic의 LTM으로 변환하는 코드가 필요하다. (연준아...^^;)
				RpIntersection	stIntersection = *(intersection);

				RwMatrix matrixPickAtomicLTMInverse;

				pThis->LockFrame();
				RwMatrixInvert( &matrixPickAtomicLTMInverse , RwFrameGetLTM( RpAtomicGetFrame( pstPickAtomic ) ) );
				pThis->UnlockFrame();

				RwV3dTransformPoint( &(stIntersection.t.line.start), &(stIntersection.t.line.start), &matrixPickAtomicLTMInverse );
				RwV3dTransformPoint( &(stIntersection.t.line.end), &(stIntersection.t.line.end), &matrixPickAtomicLTMInverse );

				// 코드 열라 지저분해진다... 그러나, 귀찮다. -_-; (parn)
				FLOAT	fDistance;
				FLOAT	fDistanceOrg;

				// Blocking Object이면, 세로로 된 Blocking Polygon은 무시한다.
				//if (eType == ACUOBJECT_TYPE_OBJECT && (ACUOBJECT_TYPE_BLOCKING & lObjectType))
					//fDistance = 0;
				//else
					fDistance = 1;

				fDistanceOrg = fDistance;

				RpCollisionGeometryForAllIntersections(pstPickGeometry, &stIntersection, CBDetailCollision, &fDistance );

				bool bAddFlag = true;

				if ( fDistance == fDistanceOrg )
				{
					//return atomic;	// List에 추가하지 않고 Return
					bAddFlag = false;
					pstPickAtomic = pstPickAtomic->next;
					continue;
				}
				
				//@{ kday 20050722
				// ;)
				// 가까운 넘들은.. 실폴리곤으로 함 더한다..
				// 베틀스퀘어에서.. 황금박쥐 때리다가.. 도망가기 힘들어서..
				if ( bAddFlag && bCheckRealAtomic )
				{
					RwReal	realdistance = fDistance;

					RwV3d	s2e = { intersection->t.line.end.x - intersection->t.line.start.x, 
						intersection->t.line.end.y - intersection->t.line.start.y, 
						intersection->t.line.end.z - intersection->t.line.start.z
					};

					realdistance *=  RwV3dLength( &s2e );

					//@{ 2006/05/09 burumal
					/*
					char pTempMsg[256];
					sprintf(pTempMsg, "AgcmTargeting::realdistance %f\n", realdistance);
					OutputDebugString(pTempMsg);
					*/
					//@}

					if( realdistance < 800.f )
					{
						fDistance = 1;
						fDistanceOrg = fDistance;

						pstPickGeometry = RpAtomicGetGeometry(atomic);

						//. 2005. 12. 27. Nonstopdj
						//. '앵커빌'의 입구쪽 NPC 수비대장의 장비 atomic이 거리에 따라 다시 로딩되지 않을 때 문제가 발생. By Crash Report..
						//. atomic->geometry의 NULL Check추가로 일단 막음.
						if ( pstPickGeometry != NULL )
						{
							RpCollisionGeometryForAllIntersections(pstPickGeometry, &stIntersection, CBDetailCollision, &fDistance );
						}
						else
						{
							//return (RpAtomic*) NULL;
							bAddFlag = false;
							pstPickAtomic = pstPickAtomic->next;
							continue;
						}

						//RpCollisionGeometryForAllIntersections(pstPickGeometry, &stIntersection, CBDetailCollision, &fDistance );

						if ( fDistance == fDistanceOrg )
						{
							//return atomic;	// List에 추가하지 않고 Return
							bAddFlag = false;
							pstPickAtomic = pstPickAtomic->next;
							continue;
						}
					}
				}
				//@} kday

				if ( bAddFlag )
				{
					// list에 추가
					pThis->AddPickedObject(pstClump, pstPickAtomic, pcsBase, eType, lID, fDistance);

					++(pThis->m_iPickedNum);
				}
			}

			pstPickAtomic = pstPickAtomic->next;

		} while ( pFirst && pstPickAtomic && (pstPickAtomic != pFirst) );
		//@}
	}

	return atomic;
}	

RpCollisionTriangle*	AgcmTargeting::CBDetailCollision(
							 RpIntersection *	intersection	,
							 RpCollisionTriangle *collTriangle	,
							 RwReal				distance		,
							 void *				data			)
{
	RwReal*		fDistance	=	(RwReal*)data		;

	if ( 0 == *fDistance && 0.001 > MagFABS(collTriangle->normal.y) )
		return collTriangle;
	
	if ( distance < *(fDistance) ) *(fDistance) = distance;

	return collTriangle;
}

//@{ 2006/03/06 burumal
//void	AgcmTargeting::AddPickedObject(RpClump* pClump,ApBase*	pBase,INT32 eType,INT32 lID,FLOAT fDistance)
void AgcmTargeting::AddPickedObject(RpClump* pClump, RpAtomic* pNewFirstPickAtomic, ApBase* pBase, INT32 eType, INT32 lID, FLOAT fDistance)
//@}
{
	PickedObject*	cur_node = m_listPicking; 

	while(cur_node)
	{
		if(cur_node->pcsBase == pBase) // 기존에 있음
			return;

		cur_node = cur_node->next;
	}

	cur_node = m_listPicking; 
	PickedObject*	bef_node = cur_node; 
	while(cur_node)
	{
		if(fDistance < cur_node->fDistance)
		{
			break;
		}

		bef_node = cur_node;
		cur_node = cur_node->next;
	}

	PickedObject*	new_node = new PickedObject;
	new_node->eType = AcuObject::GetType(eType);
	new_node->lID = lID;

	//@{ 2006/03/06 burumal
	// 멀티 PickingAtomic은 Character 타입에만 적용되어 있으므로 다른 타입은 이 pNewFirstPickAtomic를 사용하지 않는다
	new_node->pNewFirstPickAtomic = pNewFirstPickAtomic;
	//@}

	new_node->pClump = pClump;
	new_node->pcsBase = pBase;
	new_node->fDistance = fDistance;
	
	if(cur_node == m_listPicking)
	{
		new_node->next = m_listPicking;
		m_listPicking = new_node;
	}
	else
	{
		bef_node->next = new_node;
		new_node->next = cur_node;
	}
}

void	AgcmTargeting::ClearPickList()
{
	PickedObject*	cur_node = m_listPicking; 
	PickedObject*	del_node;
	while(cur_node)
	{
		del_node = cur_node;
		cur_node = cur_node->next;

		delete del_node;
	}

	m_listPicking = NULL;
}

BOOL	AgcmTargeting::CBLoginProcessEnd( PVOID pData, PVOID pClass, PVOID pCustData )
{
	AgcmTargeting* pThis = (AgcmTargeting*)pClass	;

	pThis->m_bCurrentLoginMode = FALSE			;
	pThis->ClearPickList();
	pThis->m_iPickedNum = 0;
	
	return TRUE;
}

BOOL	AgcmTargeting::CBEncryptCodeSuccess( PVOID pData, PVOID pClass, PVOID pCustData )
{
	static BOOL bInit = TRUE;

	AgcmTargeting* pThis = (AgcmTargeting*)pClass	;

	if ( bInit  && NULL != g_pEngine->m_pCurrentFullUIModule ) 
	{
		g_pEngine->m_pCurrentFullUIModule->AddChild( &(pThis->m_clSenderWindow) )	;
		bInit	 =		FALSE														;
	}

	pThis->m_bCurrentLoginMode = TRUE			;
	
	return TRUE;
}

BOOL	AgcmTargeting::CBRemoveCharacter( PVOID pData, PVOID pClass, PVOID pCustData )
{
	AgcmTargeting*	pThis = (AgcmTargeting*)pClass;
	AgpdCharacter*	ppCharacter = (AgpdCharacter*)pData;

	if (!ppCharacter)
		return TRUE;

	AgcdCharacter*	pcCharacter = pThis->m_pAgcmCharacter->GetCharacterData(ppCharacter);

	if (pThis->m_pEnemyTargetClump == pcCharacter->m_pClump)
	{
		pThis->m_pAgcmGlyph->StopEnemyTarget();
		//. 2006. 5. 18. Nonstopdj
		pThis->m_pAgcmGlyph->CloseEnemyTargetSelectWindow();
		pThis->m_pEnemyTargetClump = NULL;
	}

	if (pThis->m_pEnemyMouseOnClump && pThis->m_pEnemyMouseOnClump == pcCharacter->m_pClump)
	{
		if (pThis->m_pAgcmTextBoardMng)
			pThis->m_pAgcmTextBoardMng->DisableHPBar(pThis->m_pEnemyMouseOnClump);

		pThis->m_pEnemyMouseOnClump = NULL;
		//. 2006. 5. 18. Nonstopdj
		pThis->m_pAgcmGlyph->SetMouseOnTargetClump(NULL);
	}

	return TRUE;
}

BOOL	AgcmTargeting::CB_UnlockTarget( PVOID pData,PVOID pClass,PVOID pCustData)
{
	AgcmTargeting*	pThis = (AgcmTargeting*)pClass;

	if(pThis->m_iTargetMode == AGCMTARGETING_TARGET_MODE_ATTACKLOCK)
	{
		if (pThis->m_pAgcmGlyph && pThis->m_bShowGlyph)
			pThis->m_pAgcmGlyph->StopEnemyTarget();

		pThis->m_pEnemyTargetClump = NULL;

		pThis->m_iTargetMode = AGCMTARGETING_TARGET_MODE_NONE;
	}

	return TRUE;
}

BOOL	AgcmTargeting::CB_LockTarget( PVOID pData,PVOID pClass,PVOID pCustData)
{
	AgcmTargeting*	pThis = (AgcmTargeting*)pClass;

	//@{ 2006/10/26 burumal
	if ( pThis->m_iTargetMode == AGCMTARGETING_TARGET_MODE_AREATARGETING )
		return FALSE;
	//@}

	if(pThis->m_iTargetMode != AGCMTARGETING_TARGET_MODE_ATTACKLOCK)
	{
		if(pThis->m_iTargetMode == AGCMTARGETING_TARGET_MODE_GROUNDCLICK)
		{
			if (pThis->m_pAgcmGlyph && pThis->m_bShowGlyph && pThis->m_pMoveTarget)
			{
				pThis->m_pAgcmGlyph->DeleteGlyph(pThis->m_pMoveTarget);

				// 3D DestMove Target Disable
				pThis->m_pAgcmGlyph->StopDestMoveTarget();
			}
			pThis->m_pMoveTarget = NULL;
		}

		pThis->m_iTargetMode = AGCMTARGETING_TARGET_MODE_ATTACKLOCK;
	}

	return TRUE;
}

BOOL	AgcmTargeting::CBRButtonSkill( PVOID pData,PVOID pClass,PVOID pCustData)
{
	if (!pData || !pClass)
		return TRUE;

	AgcmTargeting*	pThis = (AgcmTargeting*)pClass;

	ApBase*		pBase = (ApBase*) pData;

	if(pThis->m_iTargetMode == AGCMTARGETING_TARGET_MODE_GROUNDCLICK)
	{
		if (pThis->m_pAgcmGlyph && pThis->m_bShowGlyph && pThis->m_pMoveTarget)
		{
			pThis->m_pAgcmGlyph->DeleteGlyph(pThis->m_pMoveTarget);
			// 3D DestMove Target Disable
			pThis->m_pAgcmGlyph->StopDestMoveTarget();
		}

		pThis->m_pMoveTarget = NULL;
		pThis->m_pAgcmCharacter->UnLockTarget(pThis->m_pAgcmCharacter->GetSelfCharacter());
	}
	
	if (pBase->m_eType == APBASE_TYPE_CHARACTER)
	{
		AgcdCharacter *pcsAgcdCharacter = pThis->m_pAgcmCharacter->GetCharacterData((AgpdCharacter*)pBase);
		RpClump*	pClump = pcsAgcdCharacter->m_pClump;

		if(pThis->m_iTargetMode == AGCMTARGETING_TARGET_MODE_NONE ||
			pThis->m_iTargetMode == AGCMTARGETING_TARGET_MODE_GROUNDCLICK)
		{
			pThis->m_iTargetMode = AGCMTARGETING_TARGET_MODE_ATTACK;
		}
		else if(pThis->m_iTargetMode == AGCMTARGETING_TARGET_MODE_ATTACKLOCK && pClump != pThis->m_pEnemyTargetClump)
		{
			if (pThis->m_pAgcmGlyph && pThis->m_bShowGlyph)
				pThis->m_pAgcmGlyph->StopEnemyTarget();

			pThis->m_pAgcmCharacter->UnLockTarget(pThis->m_pAgcmCharacter->GetSelfCharacter());
			pThis->m_iTargetMode = AGCMTARGETING_TARGET_MODE_ATTACK;
		}
		
		if(pThis->m_pEnemyTargetClump && pClump != pThis->m_pEnemyTargetClump)
		{
			RwFrame*	frame = RpClumpGetFrame(pClump);
			INT32		lRange = 0;
			pThis->m_pAgpmFactors->GetValue(&((AgpdCharacter *)pBase)->m_csFactor, &lRange, AGPD_FACTORS_TYPE_RESULT, AGPD_FACTORS_TYPE_ATTACK, AGPD_FACTORS_ATTACK_TYPE_HITRANGE);

			if (pThis->m_pAgcmGlyph && pThis->m_bShowGlyph)
			{
				if (lRange)
				{
					FLOAT		range_val = (FLOAT)lRange * 2.0f;
					pThis->m_pAgcmGlyph->StartEnemyTarget(frame,pThis->m_pEnemyTargetClump,range_val);
						
				}
				else
				{
					pThis->m_pAgcmGlyph->StartEnemyTarget(frame,pThis->m_pEnemyTargetClump,150.0f);
				}
			}

			pThis->m_pEnemyTargetClump = pClump;
		}
	}
			
	return TRUE;
}

void	AgcmTargeting::SetRwCamera(RwCamera*	pCam)
{
	m_pCamera = pCam;

	if( m_pCamera )
	{
		RwRaster * raster = RwCameraGetRaster( m_pCamera );

		if( raster )
			m_clSenderWindow.MoveWindow( 0, 0, RwRasterGetWidth( raster ), RwRasterGetHeight( raster ) );
	}
}

BOOL	AgcmTargeting::StreamReadTemplate(CHAR* szFile, BOOL bDecryption)
{
	int	i;

	if (!szFile)
		return FALSE;

	ApModuleStream	csStream;
	const CHAR		*szValueName = NULL;
	CHAR			szValue[256];
	CHAR			szFileName[256];
	INT32			lIndex;

	csStream.SetMode(APMODULE_STREAM_MODE_NAME_OVERWRITE);

	// szFile을 읽는다.
	csStream.Open(szFile, 0, bDecryption);

	{
		csStream.ReadSectionName(0);
		csStream.SetValueID(-1);

		while(csStream.ReadNextValue())
		{
			szValueName = csStream.GetValueName();

			if(!strcmp(szValueName, AGCMTARGETING_INI_FILENAME))
			{
				csStream.GetValue(szValue, 256);
				sscanf(szValue, "%d:%s", &lIndex, szFileName);

				m_pDestMove[lIndex] = RwTextureRead(szFileName, NULL);
				
				ASSERT(m_pDestMove[lIndex]);
			}
		}

		//@{ 2006/10/26 burumal
		csStream.ReadSectionName(1);
		csStream.SetValueID(-1);

		while(csStream.ReadNextValue())
		{
			szValueName = csStream.GetValueName();

			if(!strcmp(szValueName, AGCMTARGETING_INI_FILENAME))
			{
				csStream.GetValue(szValue, 256);
				sscanf(szValue, "%d:%s", &lIndex, szFileName);

				m_pAreaTargetingTex[lIndex] = RwTextureRead(szFileName, NULL);

				ASSERT(m_pAreaTargetingTex[lIndex]);
			}
		}
		//@}
	}

	HINSTANCE		hInstance	;
	hInstance = ( HINSTANCE ) ::GetWindowLong( g_pEngine->GethWnd() , GWL_HINSTANCE );
	BOOL	b16bitCursor = TRUE;

	// Get the Windows version. highbyte가 1이면 98이더라..
	DWORD	dwVersion = GetVersion();
	if (dwVersion < 0x80000000)              // Windows NT
		b16bitCursor = FALSE;

	if(b16bitCursor)
	{
		lIndex = 0;
		m_hMouseCursorMove[ lIndex++ ]	= ::LoadCursor( hInstance , "CURSOR_M1_16.CUR" );
		//m_hMouseCursorMove[ lIndex++ ]	= ::LoadCursor( hInstance , "CURSOR_M1_16.CUR" );
		
		m_lCursorMove = m_pAgcmUIManager2->AddCursor(m_hMouseCursorMove[0],700);
		for(i=1;i <= lIndex ; ++i)
		{
			m_pAgcmUIManager2->AddCursorFrame(m_lCursorMove,m_hMouseCursorMove[i],i);
		}

		if (m_pAgcmUIManager2)
			m_pAgcmUIManager2->ChangeCursor(m_lCursorMove, this);

		lIndex = 0;
		m_hMouseCursorAttack[ lIndex++ ]	= ::LoadCursor( hInstance , "CURSOR_A1_16.CUR" );
		//m_hMouseCursorAttack[ lIndex++ ]	= ::LoadCursor( hInstance , "CURSOR_A1_16.CUR" );

		m_lCursorAttack = m_pAgcmUIManager2->AddCursor(m_hMouseCursorAttack[0],700);
		for(i=1;i <= lIndex ; ++i)
		{
			m_pAgcmUIManager2->AddCursorFrame(m_lCursorAttack,m_hMouseCursorAttack[i],i);
		}

		lIndex = 0;
		m_hMouseCursorTalk[ lIndex++ ]	= ::LoadCursor( hInstance , "CURSOR_T1_16.CUR" );
		//m_hMouseCursorTalk[ lIndex++ ]	= ::LoadCursor( hInstance , "CURSOR_T1_16.CUR" );

		m_lCursorTalk = m_pAgcmUIManager2->AddCursor(m_hMouseCursorTalk[0],700);
		for(i=1;i <= lIndex ; ++i)
		{
			m_pAgcmUIManager2->AddCursorFrame(m_lCursorTalk,m_hMouseCursorTalk[i],i);
		}

		lIndex = 0;
		m_hMouseCursorPick[ lIndex++ ]	= ::LoadCursor( hInstance , "CURSOR_P1_16.CUR" );
		//m_hMouseCursorPick[ lIndex++ ]	= ::LoadCursor( hInstance , "CURSOR_P1_16.CUR" );

		m_lCursorPick = m_pAgcmUIManager2->AddCursor(m_hMouseCursorPick[0],700);
		for(i=1;i <= lIndex ; ++i)
		{
			m_pAgcmUIManager2->AddCursorFrame(m_lCursorPick,m_hMouseCursorPick[i],i);
		}

		lIndex = 0;
		m_hMouseCursorNoMove[ lIndex++ ]	= ::LoadCursor( hInstance , "CURSOR_D1_16.CUR" );
		//m_hMouseCursorNoMove[ lIndex++ ]	= ::LoadCursor( hInstance , "CURSOR_D1_16.CUR" );

		m_lCursorNoMove = m_pAgcmUIManager2->AddCursor(m_hMouseCursorNoMove[0],700);
		for(i=1;i <= lIndex ; ++i)
		{
			m_pAgcmUIManager2->AddCursorFrame(m_lCursorNoMove,m_hMouseCursorNoMove[i],i);
		}
	}
	else
	{
		lIndex = 0;
		m_hMouseCursorMove[ lIndex++ ]	= ::LoadCursor( hInstance , "CURSOR_M1.CUR" );
		//m_hMouseCursorMove[ lIndex++ ]	= ::LoadCursor( hInstance , "CURSOR_M1.CUR" );
		
		m_lCursorMove = m_pAgcmUIManager2->AddCursor(m_hMouseCursorMove[0],700);
		for(i=1;i <= lIndex ; ++i)
		{
			m_pAgcmUIManager2->AddCursorFrame(m_lCursorMove,m_hMouseCursorMove[i],i);
		}

		if (m_pAgcmUIManager2)
			m_pAgcmUIManager2->ChangeCursor(m_lCursorMove, this);

		lIndex = 0;
		m_hMouseCursorAttack[ lIndex++ ]	= ::LoadCursor( hInstance , "CURSOR_A1.CUR" );
		//m_hMouseCursorAttack[ lIndex++ ]	= ::LoadCursor( hInstance , "CURSOR_A1.CUR" );

		m_lCursorAttack = m_pAgcmUIManager2->AddCursor(m_hMouseCursorAttack[0],700);
		for(i=1;i <= lIndex ; ++i)
		{
			m_pAgcmUIManager2->AddCursorFrame(m_lCursorAttack,m_hMouseCursorAttack[i],i);
		}

		lIndex = 0;
		m_hMouseCursorTalk[ lIndex++ ]	= ::LoadCursor( hInstance , "CURSOR_T1.CUR" );
		//m_hMouseCursorTalk[ lIndex++ ]	= ::LoadCursor( hInstance , "CURSOR_T1.CUR" );

		m_lCursorTalk = m_pAgcmUIManager2->AddCursor(m_hMouseCursorTalk[0],700);
		for(i=1;i <= lIndex ; ++i)
		{
			m_pAgcmUIManager2->AddCursorFrame(m_lCursorTalk,m_hMouseCursorTalk[i],i);
		}

		lIndex = 0;
		m_hMouseCursorPick[ lIndex++ ]	= ::LoadCursor( hInstance , "CURSOR_P1.CUR" );
		//m_hMouseCursorPick[ lIndex++ ]	= ::LoadCursor( hInstance , "CURSOR_P1.CUR" );

		m_lCursorPick = m_pAgcmUIManager2->AddCursor(m_hMouseCursorPick[0],700);
		for(i=1;i <= lIndex ; ++i)
		{
			m_pAgcmUIManager2->AddCursorFrame(m_lCursorPick,m_hMouseCursorPick[i],i);
		}

		lIndex = 0;
		m_hMouseCursorNoMove[ lIndex++ ]	= ::LoadCursor( hInstance , "CURSOR_D1.CUR" );
		//m_hMouseCursorNoMove[ lIndex++ ]	= ::LoadCursor( hInstance , "CURSOR_D1.CUR" );

		m_lCursorNoMove = m_pAgcmUIManager2->AddCursor(m_hMouseCursorNoMove[0],700);
		for(i=1;i <= lIndex ; ++i)
		{
			m_pAgcmUIManager2->AddCursorFrame(m_lCursorNoMove,m_hMouseCursorNoMove[i],i);
		}
	}

	return TRUE;
}

BOOL	AgcmTargeting::StreamWriteTemplate(CHAR* szFile, BOOL bEncryption)
{
	int i;

	if (!szFile)
		return FALSE;

	ApModuleStream		csStream;
	CHAR				szValue[256];
	
	csStream.SetMode(APMODULE_STREAM_MODE_NAME_OVERWRITE);
	
	csStream.SetSection("[Textures1]");
	for(i=0;i<AGCMTARGETING_DESTMOVE_IMAGE_NUM;++i)
	{
		sprintf(szValue,"%d:%s",i,RwTextureGetName (m_pDestMove[i]));
		if(!csStream.WriteValue(AGCMTARGETING_INI_FILENAME, szValue))
			return FALSE;
	}

	//@{ 2006/10/26 burumal
	csStream.SetSection("[Textures2]");
	for(i=0;i<AGCMTARGETING_AREATARGETING_IMAGE_NUM;++i)
	{
		sprintf(szValue,"%d:%s",i,RwTextureGetName (m_pAreaTargetingTex[i]));
		if(!csStream.WriteValue(AGCMTARGETING_INI_FILENAME, szValue))
			return FALSE;
	}
	//@}

	csStream.Write(szFile, 0, bEncryption);

	return TRUE;
}

VOID	AgcmTargeting::SetShowCursor(BOOL bShow)
{
	m_bShowCursor = bShow;
}

/*****************************************************************
*   Function : SetCallbackPreLButtonDown
*   Comment  : Pre LButton Down 시 Pick한것이 있으면 Callback함수를 호출한다 
*   Date&Time : 2003-11-03, 오후 5:46
*   Code By : Seong Yon-jun : NHN Online RPG
*****************************************************************/
BOOL	AgcmTargeting::SetCallbackLButtonDownPickSomething(ApModuleDefaultCallBack pfCallback, PVOID pClass )
{
	return SetCallback(AGCMTARGETING_CB_ID_LBUTTONDOWN_PICK_SOMETHING, pfCallback, pClass );
}

/*****************************************************************
*   Function : SetCallbackPreLButtonDown
*   Comment  : Pre LButton Down 시 Pick한것이 없을때 불러준다.
*   Date&Time : 2003-12-30, 오후 2:41
*   Code By : Kim Tai Heui : NHN Online RPG
*****************************************************************/
BOOL	AgcmTargeting::SetCallbackLButtonDownPickNothing(ApModuleDefaultCallBack pfCallback, PVOID pClass)
{
	return SetCallback(AGCMTARGETING_CB_ID_LBUTTONDOWN_PICK_NOTHING, pfCallback, pClass);
}

RpClump*	AgcmTargeting::GetCharacterClump( INT32	lCID )
{
	AgpdCharacter* ppdCharacter = m_pAgpmCharacter->GetCharacter( lCID );
				
	if ( ppdCharacter )
	{
		AgcdCharacter* pcdCharacter = m_pAgcmCharacter->GetCharacterData( ppdCharacter );

		if ( pcdCharacter && pcdCharacter->m_pClump )
		{
			return pcdCharacter->m_pClump;
		}
	}

	return NULL;
}

BOOL	AgcmTargeting::CB_UpdateActionStatus(PVOID pData,PVOID pClass,PVOID pCustData)
{
	AgcmTargeting	*pThis = (AgcmTargeting*)	pClass;
	AgpdCharacter	*pcsCharacter	= (AgpdCharacter *) pData;
	INT16			nOldStatus		= *(INT16 *)		pCustData;
	AgcdCharacter* pcdCharacter = pThis->m_pAgcmCharacter->GetCharacterData( pcsCharacter );

	/* 죽은넘도 Targeting되어야 된다. (죽었을때 Target Lock 해제 안함) (Parn, 2004/9/23)
	if(pThis->m_iTargetMode == AGCMTARGETING_TARGET_MODE_ATTACKLOCK ||
		pThis->m_iTargetMode == AGCMTARGETING_TARGET_MODE_ATTACK)
	{
		if(pcsCharacter->m_unActionStatus == AGPDCHAR_STATUS_DEAD &&
			pcdCharacter->m_pClump == pThis->m_pEnemyTargetClump)
		{
			// 타겟락 해제
			if (pThis->m_pAgcmGlyph && pThis->m_bShowGlyph)
				pThis->m_pAgcmGlyph->DeleteGlyphA(pThis->m_pEnemyTarget);

			pThis->m_pEnemyTarget = NULL;
			pThis->m_pEnemyTargetClump = NULL;
			pThis->m_uiLDownCTick = 0;
			pThis->m_iChangeTick = 0;

			pThis->m_pAgcmCharacter->UnLockTarget(pThis->m_pAgcmCharacter->GetSelfCharacter());

			pThis->m_iTargetMode = AGCMTARGETING_TARGET_MODE_NONE;
		}
	}
	*/

	return TRUE;
}

VOID	AgcmTargeting::SetTargetBase(ApBase *	pcsBase)
{
	if (!pcsBase)
	{
		m_lTargetType			= APBASE_TYPE_NONE;
		m_lTargetID				= 0;
		m_lTargetTID			= 0;
		m_szTargetName			= "";
	}
	else
	{
		m_lTargetType			= pcsBase->m_eType;
		m_lTargetID				= pcsBase->m_lID;

		switch (pcsBase->m_eType)
		{
		case APBASE_TYPE_OBJECT:
			{
				m_lTargetTID	= ((ApdObject *) pcsBase)->m_lTID;
				m_szTargetName	= ((ApdObject *) pcsBase)->m_pcsTemplate->m_szName;
			}

			break;

		case APBASE_TYPE_CHARACTER:
			{
				m_lTargetTID	= ((AgpdCharacter *) pcsBase)->m_lTID1;
				//m_szTargetName	= ((AgpdCharacter *) pcsBase)->m_pcsCharacterTemplate->m_szTName;
				m_szTargetName.clear();
				m_szTargetName	= ((AgpdCharacter *) pcsBase)->m_pcsCharacterTemplate->m_szTName;
			}

			break;

		case APBASE_TYPE_ITEM:
			{
				m_lTargetTID	= ((AgpdItem *) pcsBase)->m_lTID;
				m_szTargetName	= ((AgpdItemTemplate *) ((AgpdItem *) pcsBase)->m_pcsItemTemplate)->m_szName;
			}

			break;

		default:
			{
				m_lTargetTID		= 0;
				m_szTargetName[0]	= 0;
			}
		}
	}
}

void	AgcmTargeting::ShowTarget()
{
	AgcmUIConsole *	pcsAgcmUIConsole	= (AgcmUIConsole *) GetModule("AgcmUIConsole");
	if (pcsAgcmUIConsole)
	{
		pcsAgcmUIConsole->getConsole().setWatchWidth( 400 );
		pcsAgcmUIConsole->getConsole().registerWatch(0, "AgcmTargeting.m_lTargetType", 0xffffff00);
		pcsAgcmUIConsole->getConsole().registerWatch(1, "AgcmTargeting.m_lTargetTID", 0xffffff00);
		pcsAgcmUIConsole->getConsole().registerWatch(2, "AgcmTargeting.m_lTargetID", 0xffffff00);
		pcsAgcmUIConsole->getConsole().registerWatch(3, "AgcmTargeting.m_szTargetName", 0xffffff00);
	}
}

BOOL	AgcmTargeting::CBReturnToLoginEndProcess( PVOID pData, PVOID pClass, PVOID pCustData )
{
	if (!pClass)
		return FALSE;

	((AgcmTargeting *) pClass)->SetCurrentLoginMode(TRUE);

	return TRUE;
}

BOOL	AgcmTargeting::CBSetTargetMode( PVOID pData, PVOID pClass, PVOID pCustData )
{
	if (!pClass)
		return FALSE;

	//. targetmode 설정.
	((AgcmTargeting *) pClass)->m_iTargetMode = AGCMTARGETING_TARGET_MODE_NONE;

	return TRUE;
}

struct	CID_Distance
{
	INT32	nCID;
	FLOAT	fDist;

	CID_Distance()
	{
		fDist = 99999999999.0f;
	}

	static bool greater ( CID_Distance elem1, CID_Distance elem2 )
	{
	   return elem1.fDist < elem2.fDist;
	}	
};

BOOL	AgcmTargeting::TargetNear		( KeyTargetOption eOption , AgcmTargeting::SearchOption sOption)
{
	/*
	( PC 리스트 쫙 돌면 될까나.. )
	화면 안에있는 PC 취합..  *예외 상황 체크
	InFrustum Check .. with bounding Sphere
	거리별 쏘트.

	현재 타겟 되어있는 ID가 있으면 그 다음것을 타겟함.
	타겟이 된 녀석이 없으면 선두 녀석.
	*/

	INT32					lIndex;
	AgpdCharacter			*ppdCharacter;
	AgcdCharacter			*pcdCharacter;
	AgpdCharacter			*ppdSelfCharacter = m_pAgcmCharacter->GetSelfCharacter();

	pcdCharacter = m_pAgcmCharacter->GetCharacterData( ppdSelfCharacter );
	if( pcdCharacter && pcdCharacter->m_lSelectTargetID == AP_INVALID_CID )
	{
		sOption = AgcmTargeting::KSO_NEAREST;
	}

	RwFrustumTestResult		res;

	RwCamera				* pCamera =	g_pEngine->m_pCamera;
	VERIFY( pCamera );

	INT32	nFilter = 0;

	if( eOption & KTO_PC	) nFilter |= AGPMCHAR_TYPE_PC;
	if( eOption & KTO_NPC	) nFilter |= AGPMCHAR_TYPE_NPC;
	if( eOption & KTO_MOB	) nFilter |= AGPMCHAR_TYPE_MONSTER;
	// if( eOption & KTO_OBJ	) nFilter |= ;

	vector< CID_Distance >	vecTarget;
	RwMatrix	*pMat;
	pMat = RwFrameGetLTM( RwCameraGetFrame( pCamera ) );

	lIndex = 0;
	for (	ppdCharacter = m_pAgpmCharacter->GetCharSequence(&lIndex);
		ppdCharacter;
		ppdCharacter = m_pAgpmCharacter->GetCharSequence(&lIndex))
	{
		AgcdCharacter			*__pcdCharacter;
		__pcdCharacter = m_pAgcmCharacter->GetCharacterData( ppdCharacter );
		if( NULL == __pcdCharacter )
			continue;
		if( ppdSelfCharacter == ppdCharacter )
			continue;

		if( NULL == __pcdCharacter->m_pPickAtomic )
			continue;

		// 타입 체크.. 오브젝트냐 케릭터냐..

		if( !( ppdCharacter->m_ulCharType & nFilter ) )
			continue;

		if(m_pAgpmCharacter->IsSameRace(ppdCharacter, ppdSelfCharacter))
			continue;

		if( ppdCharacter->IsDead() )			continue;	// 죽었으면 스킵.
		if( m_pAgcmCharacter->IsDead( m_pAgcmCharacter->GetCharacterData( ppdCharacter ) ) ) continue;

		//@{ 2006/12/27 burumal		
		ASSERT(ppdCharacter->m_pcsCharacterTemplate);
		if ( m_pAgcmCharacter->IsNonPickingTID(ppdCharacter->m_pcsCharacterTemplate->m_lID) ) continue; // nonpicking 캐릭터 스킵
		//@}

		// 클럼프가 Frustum 안에 있는지 조사함.

		RwSphere stSphere , * pSphere;
		pSphere = RpAtomicGetBoundingSphere(__pcdCharacter->m_pPickAtomic);
		stSphere = *pSphere;
		RwMatrix * pTransfrom = RwFrameGetLTM( RpClumpGetFrame( __pcdCharacter->m_pClump ) );
		RwV3dTransformPoint( &stSphere.center , &stSphere.center , pTransfrom );

		res = RwCameraFrustumTestSphere( pCamera, &stSphere );
		if( res == rwSPHEREOUTSIDE ) continue;

		//
		CID_Distance stData;
		stData.nCID		= ppdCharacter->m_lID;
		stData.fDist	= AUPOS_DISTANCE_XZ( ppdCharacter->m_stPos , pMat->pos );

		if( stData.fDist < m_fKeyTargettingRange )
		{
			vecTarget.push_back( stData );
		}
	}

	if( !vecTarget.size() )
	{
		// 합당한 타겟이 없다.
		return FALSE;
	}

	// 소팅.
	sort( vecTarget.begin( ), vecTarget.end( ), CID_Distance::greater );

	#ifdef _DEBUG
	{
		vector< CID_Distance >::iterator iter;
		vector< CID_Distance >::iterator iterEnd;

		iter	= vecTarget.begin();
		iterEnd	= vecTarget.end();

		CID_Distance stData;

		BOOL bFound = FALSE;
		BOOL bGet = FALSE;

		int i = 0 ;

		char str[ 256 ];
		for( ;
			iter != iterEnd;
			iter ++ , i++ )
		{
			stData = *iter;
			
			sprintf( str , "%d) %d , %.0f\n" , i , stData.nCID , stData.fDist );
			OutputDebugString( str );
		}
	}
	#endif

	switch( sOption )
	{
	case AgcmTargeting::KSO_NEAREST:
		{
			CID_Distance stData = *vecTarget.begin();
				
			// 타겟
			TargetCID( stData.nCID );
			return TRUE;
		}
		break;

	//@{ 2007/01/04 burumal
	//case AgcmTargeting::KSO_NEXT:
	case AgcmTargeting::KSO_PREV:
	//@}
		{
			vector< CID_Distance >::reverse_iterator iter;
			vector< CID_Distance >::reverse_iterator iterEnd;

			iter	= vecTarget.rbegin();
			iterEnd	= vecTarget.rend();

			CID_Distance stData;

			int i = 0 ;

			BOOL bFound = FALSE;
			BOOL bGet = FALSE;
			for( ;
				iter != iterEnd;
				iter ++ , i ++ )
			{
				stData = *iter;
				if( bFound )
				{
					bGet = TRUE;
					break;
				}
				if( ( *iter ).nCID == pcdCharacter->m_lSelectTargetID )
				{
					bFound = TRUE;
				}
			}

			// 타겟

			if( !bGet )
			{
				// 발견이 안돼면.
				stData = *vecTarget.rbegin();
			}

			TargetCID( stData.nCID );
		}
		break;

	//@{ 2007/01/04 burumal	
	//case AgcmTargeting::KSO_PREV:
	case AgcmTargeting::KSO_NEXT:
	//@}	
		{
			vector< CID_Distance >::iterator iter;
			vector< CID_Distance >::iterator iterEnd;

			iter	= vecTarget.begin();
			iterEnd	= vecTarget.end();

			CID_Distance stData;

			int i = 0 ;

			BOOL bFound = FALSE;
			BOOL bGet = FALSE;
			for( ;
				iter != iterEnd;
				iter ++  , i ++ )
			{
				stData = *iter;
				if( bFound )
				{
					bGet = TRUE;
					break;
				}
				if( ( *iter ).nCID == pcdCharacter->m_lSelectTargetID )
				{
					bFound = TRUE;
				}
			}

			// 타겟

			if( !bGet )
			{
				// 발견이 안돼면.
				stData = *vecTarget.begin();
			}

			TargetCID( stData.nCID );

		}
		break;
	}

	return TRUE;
}

BOOL	AgcmTargeting::TargetLast		()
{
	// 구현 구상중.
	return TRUE;
}

void	AgcmTargeting::ClearTarget		()
{
	// 타겟 설정을 없엔다.
	AgpdCharacter * pCharacter = m_pAgcmCharacter->GetSelfCharacter();
	AgcdCharacter	*pcdCharacter;

	pcdCharacter = m_pAgcmCharacter->GetCharacterData( pCharacter );

	if( pCharacter )
	{
		if( pcdCharacter->m_lSelectTargetID != AP_INVALID_CID )
		{
			m_pAgcmCharacter->UnSelectTarget( pCharacter );
		}
	}
}

#ifdef _DEBUG
VOID	AgcmTargeting::TargetNearestDebug()
{
	TargetNear( KTO_MOB , KSO_NEAREST );
}
VOID	AgcmTargeting::TargetNextDebug()
{
	TargetNear( KTO_MOB , KSO_NEXT );
}
#endif // _DEBUG


//@{ 2006/08/11 burumal
BOOL	AgcmTargeting::CBAutoTargetingForward(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl)
{
	AgcmTargeting* pThis	= (AgcmTargeting*) pClass;
	ASSERT(pThis);

	if ( !pThis->m_pAgcmUIManager2 )
		return FALSE;

	AgpdCharacter* pSelfCharacter = pThis->m_pAgcmCharacter->GetSelfCharacter();
	if(!pSelfCharacter)
		return FALSE;

	INT32	eOption = KTO_MOB;
	
	if (pThis->m_pAgpmShrineBattle->IsInShrineRegionByIndex(pSelfCharacter))
		eOption |= KTO_PC;
	
	pThis->TargetNear( (KeyTargetOption)eOption , KSO_NEXT );
	TRACE( "pThis->TargetNear( KTO_MOB , KSO_NEXT );\n" );
	//pThis->TargetNext(); // very simple~

	return TRUE;
}

BOOL	AgcmTargeting::CBAutoTargetingBackward(PVOID pClass, PVOID pData1, PVOID pData2, PVOID pData3, PVOID pData4, PVOID pData5, ApBase* pcsTarget, AgcdUIControl* pcsSourceControl)
{	
	AgcmTargeting* pThis	= (AgcmTargeting*) pClass;
	ASSERT(pThis);
	
	if ( !pThis->m_pAgcmUIManager2 )
		return FALSE;

	AgpdCharacter* pSelfCharacter = pThis->m_pAgcmCharacter->GetSelfCharacter();
	if(!pSelfCharacter)
		return FALSE;

	INT32	eOption = KTO_MOB;

	if (pThis->m_pAgpmShrineBattle->IsInShrineRegionByIndex(pSelfCharacter))
		eOption |= KTO_PC;

	pThis->TargetNear( (KeyTargetOption)eOption , KSO_NEXT );

	TRACE( "pThis->TargetNear( KTO_MOB , KSO_PREV );\n" );
	// pThis->TargetPrev(); // very simple~

	return FALSE;
}
//@}

//@{ 2006/10/26 burumal
BOOL	AgcmTargeting::StartAreaTargeting(FLOAT fAreaWidth, FLOAT fAreaHeight)
{
	if ( m_iTargetMode == AGCMTARGETING_TARGET_MODE_AREATARGETING )		return TRUE;
	if ( fAreaWidth <= 0.0f || fAreaHeight <= 0.0f )					return FALSE;

	switch ( m_iTargetMode )
	{	
	case AGCMTARGETING_TARGET_MODE_GROUNDCLICK :
		{
			if (m_pAgcmGlyph && m_bShowGlyph && m_pMoveTarget )
			{
				m_pAgcmGlyph->DeleteGlyph(m_pMoveTarget);
				m_pAgcmGlyph->StopDestMoveTarget();		// 3D DestMove Target Disable
			}

			m_pMoveTarget = NULL;
		}
		break;		

	case AGCMTARGETING_TARGET_MODE_ATTACK :
	case AGCMTARGETING_TARGET_MODE_ATTACKLOCK :
		{
			// 타겟팅 모드 해제
			if (m_pAgcmGlyph && m_bShowGlyph)
				m_pAgcmGlyph->StopEnemyTarget();
			
			if ( m_pAgcmCharacter )
				m_pAgcmCharacter->UnLockTarget(m_pAgcmCharacter->GetSelfCharacter());
		}	
		break;
	}

	FLOAT fAreaSize		= max( fAreaWidth, fAreaHeight );
	INT32 iTargetImage	= fAreaSize >= 700.f ? 2 : ( fAreaSize >= 400.f ? 1 : 0 );

	//아크로드이면 그대로.. 아니면  + 3을 해준다..
	AgpmArchlord* pcsAgpmArchlord = (AgpmArchlord*)GetModule( "AgpmArchlord" );
	if( pcsAgpmArchlord )
	{
		if( !pcsAgpmArchlord->IsArchlord( m_pAgcmCharacter->GetSelfCharacter()->m_szID ) )
			iTargetImage += 3;
	}

	if ( m_iLastTargetImage != iTargetImage )
	{
		if ( m_pAreaTarget )
		{
			m_pAgcmGlyph->DeleteGlyph( m_pAreaTarget );
			m_pAreaTarget = NULL;
		}
	}

	m_iLastTargetImage	= iTargetImage;
	m_iTargetMode		= AGCMTARGETING_TARGET_MODE_AREATARGETING;
	
	if ( !m_pAreaTarget )
	{
		if ( m_pAgcmCharacter && m_pAgcmCharacter->GetSelfCharacter() )
		{
			RwV3d dPos;
			dPos.x = m_pAgcmCharacter->GetSelfCharacter()->m_stPos.x;
			dPos.y = m_pAgcmCharacter->GetSelfCharacter()->m_stPos.y;
			dPos.z = m_pAgcmCharacter->GetSelfCharacter()->m_stPos.z;
			
			m_pAreaTarget = m_pAgcmGlyph->AddStaticGlyph( m_pAreaTargetingTex[iTargetImage], &dPos, 
							fAreaWidth, fAreaHeight, 0xFFFFFFFF, -99999, FALSE, eGLYPH_FLAG_FORCED_VISIBLE);
		}

		ASSERT( m_pAreaTarget );
	}

	return TRUE;
}

VOID	AgcmTargeting::StopAreaTargeting()
{
	if ( m_iTargetMode != AGCMTARGETING_TARGET_MODE_AREATARGETING )
		return;

	if ( m_pAreaTarget )
	{
		m_pAgcmGlyph->DeleteGlyph(m_pAreaTarget);
		m_pAreaTarget = NULL;
	}

	m_iLastTargetImage	= 0;
	m_iTargetMode		= AGCMTARGETING_TARGET_MODE_NONE;
}

BOOL	AgcmTargeting::OnAreaTargetingDestPosDetected(RwV2d* pScreenPos)
{
	FLOAT fMapDistance = (FLOAT)_I32_MAX;
	AuPOS posDestPoint;
	
	if ( !pScreenPos || !m_pAgcmMap )
		return FALSE;

	if ( m_iTargetMode != AGCMTARGETING_TARGET_MODE_AREATARGETING )
		return FALSE;

	if ( !m_pAgcmMap->GetMapPositionFromMousePosition( (int)pScreenPos->x, (int)pScreenPos->y,  &posDestPoint.x, 
														&posDestPoint.y, &posDestPoint.z, NULL, NULL, &fMapDistance ) )
	{
		return FALSE;
	}	

	StopAreaTargeting();

	// @todo: here. use this -> posDestPoint
	m_pAgcmSkill->CastGroundTargetSkill(posDestPoint);

	return TRUE;
}

BOOL	AgcmTargeting::CBStartGroundTarget(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if(!pData || !pClass)
		return FALSE;

	AgcmTargeting* pThis = static_cast<AgcmTargeting*>(pClass);
	FLOAT fArea = *static_cast<FLOAT*>(pData);

	return pThis->StartAreaTargeting(fArea, fArea);
}

VOID	AgcmTargeting::UpdateAreaTargeting(AuPOS* pCurrentDestPos)
{
	if ( !pCurrentDestPos )
		return;
	
	if ( m_iTargetMode == AGCMTARGETING_TARGET_MODE_AREATARGETING )
	{
		if ( m_pAreaTarget )
			m_pAgcmGlyph->UpdatePosStaticGlyph(m_pAreaTarget, pCurrentDestPos->x, pCurrentDestPos->y, pCurrentDestPos->z);
	}
	else
	{
		if ( m_pAreaTarget )
			StopAreaTargeting();
	}
}
//@}

//@{ 2006/10/31 burumal
BOOL	AgcmTargeting::CBKeydownESC(PVOID pData, PVOID pClass, PVOID pCustData)
{
	if ( !pClass )
		return FALSE;

	AgcmTargeting* pThis = (AgcmTargeting*)	pClass;
	if ( !pThis )
		return FALSE;

	// ESC 키에의해 main 윈도우를 제외한 다른 UI 윈도우가 close 되었는지 여부
	BOOL bIsCloseWindow = *((BOOL*) pData);

	if ( bIsCloseWindow )
		return TRUE;
	
	if ( pThis->GetTargetMode() == AGCMTARGETING_TARGET_MODE_AREATARGETING )
	{
		pThis->StopAreaTargeting();

		// main 윈도우 말고 다른 ui 윈도우가 닫힌게 있도록 강제로 설정해서 exit ui가 뜨지 않도록 만든다
		*((BOOL*) pData) = TRUE;

		pThis->m_pAgcmSkill->SetGroundTargetSkillID(0);
	}
	
	return TRUE;
}
//@}
#include "stdafx.h"
#include "NpcThread.h"
#include "Npc.h"
#define DELAY				250

//////////////////////////////////////////////////////////////////////
// NPC Thread Callback Function
//
uint32 THREADCALL NpcThreadProc(void * pParam /* CNpcThread ptr */)
{
	try
	{
		CNpcThread*	pInfo	= (CNpcThread *)pParam;
		CNpc*				pNpc	= nullptr;

		int					i			= 0;
		time_t				dwDiffTime	= 0;
		time_t				dwTickTime  = 0;
		srand((uint32)UNIXTIME);
		myrand( 1, 10000 ); myrand( 1, 10000 );

		time_t fTime2 = 0;
		int    duration_damage=0;

		if(!pInfo) return 0;

		while(!g_bNpcExit)
		{
			fTime2 = getMSTime();

			foreach (itr, pInfo->m_pNpcs)
			{
				pNpc = *itr;
				dwTickTime = fTime2 - pNpc->m_fDelayTime;

				if (pNpc->m_Delay > (int)dwTickTime && !pNpc->m_bFirstLive && pNpc->m_Delay != 0) 
				{
					if (pNpc->m_Delay < 0) pNpc->m_Delay = 0;

					if (pNpc->m_NpcState == NPC_STANDING 
						&& pNpc->CheckFindEnemy()
						&& pNpc->FindEnemy())
					{
						pNpc->m_NpcState = NPC_ATTACKING;
						pNpc->m_Delay = 0;
					}
					continue;
				}	
			
				dwTickTime = fTime2 - pNpc->m_fHPChangeTime;
				if( 10000 < dwTickTime )	{	// 10초마다 HP를 회복 시켜준다
					pNpc->HpChange();
				}

				uint8 bState = pNpc->m_NpcState;
				time_t tDelay = -1;
				switch (bState)
				{
				case NPC_LIVE:					// 방금 살아난 경우
					tDelay = pNpc->NpcLive();
					break;

				case NPC_STANDING:						// 하는 일 없이 서있는 경우
					tDelay = pNpc->NpcStanding();
					break;
			
				case NPC_MOVING:
					tDelay = pNpc->NpcMoving();
					break;

				case NPC_ATTACKING:
					tDelay = pNpc->NpcAttacking();
					break;

				case NPC_TRACING:
					tDelay = pNpc->NpcTracing();
					break;

				case NPC_FIGHTING:
					tDelay = pNpc->Attack();
					break;

				case NPC_BACK:
					tDelay = pNpc->NpcBack();
					break;

				case NPC_STRATEGY:
					break;

				case NPC_DEAD:
					pNpc->m_NpcState = NPC_LIVE;
					break;

				case NPC_SLEEPING:
					tDelay = pNpc->NpcSleeping();
					break;

				case NPC_FAINTING:
					tDelay = pNpc->NpcFainting();
					break;

				case NPC_HEALING:
					tDelay = pNpc->NpcHealing();
					break;
				}

				// This may not be necessary, but it keeps behaviour identical.
				if (bState != NPC_LIVE && bState != NPC_DEAD
					&& pNpc->m_NpcState != NPC_DEAD)
					pNpc->m_fDelayTime = getMSTime();

				if (tDelay >= 0)
					pNpc->m_Delay = tDelay;
			}	

			sleep(100);
		}
	}
	catch (std::exception & ex)
	{
		printf("Exception occurred: %s\n", ex.what());
	}
	return 0;
}

uint32 THREADCALL ZoneEventThreadProc(void * pParam /* = nullptr */)
{
	while (!g_bNpcExit)
	{
		g_pMain->g_arZone.m_lock.Acquire();
		foreach_stlmap_nolock (itr, g_pMain->g_arZone)
		{
			MAP *pMap = itr->second;
			if (pMap == nullptr
				|| pMap->m_byRoomEvent == 0
				|| pMap->IsRoomStatusCheck()) 
				continue;

			foreach_stlmap (itr, pMap->m_arRoomEventArray)
			{
				CRoomEvent * pRoom = itr->second;
				if (pRoom == nullptr
					|| !pRoom->isInProgress())
					continue;

				pRoom->MainRoom();
			}
		}
		g_pMain->g_arZone.m_lock.Release();

		sleep(1000);
	}

	return 0;
}

CNpcThread::CNpcThread()
{
}

CNpcThread::~CNpcThread()
{
	m_pNpcs.clear();
}

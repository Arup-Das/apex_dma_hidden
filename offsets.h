#define ORIGIN 1
#define STEAM 2

#define VERSION ORIGIN

#if VERSION == ORIGIN

    #define OFFSET_ENTITYLIST								0x1a75038 // 0x1a203b8 // 0x19fbc18 // updated
    #define OFFSET_LOCAL_ENT								0x1e25418 // 0x1dd15e8  // 0x1dac9c8 //LocalPlayer updated 
    #define OFFSET_NAME_LIST            					0xba1e650 // 0xba30ab0  // 0xb9f7e40 // updated
    #define OFFSET_THIRDPERSON          					0x01a5a440 + 0x6c // 0x019e0740 + 0x6c //thirdperson_override + 0x6c
    #define OFFSET_TIMESCALE            					0x0141b2b0  // host_timescale updated

    #define OFFSET_TEAM										0x448 //m_iTeamNum ok
    #define OFFSET_HEALTH									0x438 //m_iHealth ok
    #define OFFSET_SHIELD									0x170 //m_shieldHealth ok
    #define OFFSET_NAME										0x589 //m_iName ok
    #define OFFSET_SIGN_NAME								0x580 //m_iSignifierName ok
    #define OFFSET_ABS_VELOCITY         					0x140 //m_vecAbsVelocity ok
    #define OFFSET_VISIBLE_TIME         					0x1a44 // 0x1ad4 //CPlayer!lastVisibleTime ok updated
    #define OFFSET_ZOOMING      							0x1bc1 // 0x1c51 //m_bZooming ok updated
    #define OFFSET_THIRDPERSON_SV       					0x3608 // 0x36a8 //m_thirdPersonShoulderView updated
    #define OFFSET_YAW                  					0x2200 - 0x8 // 0x22a0 - 0x8 //m_currentFramePlayer.m_ammoPoolCount - 0x8 updated


    #define OFFSET_LIFE_STATE								0x798  //m_lifeState, >0 = dead ok
    #define OFFSET_BLEED_OUT_STATE							0x2688 // 0x2720 //m_bleedoutState, >0 = knocked ok updated

    #define OFFSET_ORIGIN									0x014c //m_vecAbsOrigin ok
    #define OFFSET_BONES									0x0e88 + 0x48 //m_nForceBone + 0x48 // 0x0f50 - 0x18 //m_bConstrainBetweenEndpoints - 0x18
    #define OFFSET_AIMPUNCH									0x23f8 // 0x2498 //m_currentFrameLocalPlayer.m_vecPunchWeapon_Angle ok updated
    #define OFFSET_CAMERAPOS								0x1ea0 // 0x1f40 //CPlayer!camera_origin ok updated
    #define OFFSET_VIEWANGLES								0x24f4 - 0x14 // 0x2594 - 0x14 //m_ammoPoolCapacity - 0x14 ok updated
    #define OFFSET_BREATH_ANGLES							OFFSET_VIEWANGLES - 0x10
    #define OFFSET_OBSERVER_MODE							0x341c // 0x34bc //m_iObserverMode updated
    #define OFFSET_OBSERVING_TARGET							0x3428 //0x34c8 //m_hObserverTarget updated

    #define OFFSET_MATRIX									0x11a210 // ok
    #define OFFSET_RENDER									0x7599758 // 0x7546250 // 0x74bad90 updated

    #define OFFSET_WEAPON									0x19ec // 0x1a6c //m_latestPrimaryWeapons ok updated
    #define OFFSET_BULLET_SPEED         					0x1eb8 // 0x1f28 //CWeaponX!m_flProjectileSpeed updated
    #define OFFSET_BULLET_SCALE         					0x1ec0 // 0x1f30 //CWeaponX!m_flProjectileScale updated
    #define OFFSET_ZOOM_FOV             					0x1698 + 0x00b8 // 0x1718 + 0xb8 //m_playerData + m_curZoomFOV ok updated
    #define OFFSET_AMMO                 					0x1624 //0x1650 // 0x16a4 //m_ammoInClip updated

    #define OFFSET_ITEM_GLOW           						0x02c0 //m_highlightFunctionBits ok

    #define OFFSET_GLOW_T1              					0x262 //16256 = enabled, 0 = disabled 
    #define OFFSET_GLOW_T2              					0x2dc //1193322764 = enabled, 0 = disabled 
    #define OFFSET_GLOW_ENABLE          					0x3c8 //7 = enabled, 2 = disabled
    #define OFFSET_GLOW_THROUGH_WALLS   					0x3d0 //2 = enabled, 5 = disabled
    #define OFFSET_IS_ATTACKING         					0x0759bff8 // kbutton_t in_attack state
    #define OFFSET_AMMO_IN_CLIP         					0x1624 //0x1650 // 0x16d0 // [RecvTable.DT_WeaponX_LocalWeaponData] m_ammoInClip updated
    #define GLOW_FADE                   					0x388 // Script_Highlight_GetCurrentInsideOpacity 3rd result of 3 offsets consecutive or first + 8 
    #define OFFSET_GLOW_ENABLE_GLOW_CONTEXT 				OFFSET_GLOW_ENABLE
    #define OFFSET_GLOW_THROUGH_WALLS_GLOW_VISIBLE_TYPE 	OFFSET_GLOW_THROUGH_WALLS
	#define OFFSET_STUDIOHDR            					0x10d8
	#define GLOW_TYPE 										0x2C0 //0x2C4 // Script_Highlight_GetState + 4 / m_highlightFunctionBits  + 4?　更新
	#define GLOW_COLOR 										0x1D0
	#define GLOW_DISTANCE 									0x3B4 //OK Script_Highlight_SetFarFadeDist / m_highlightServerFadeEndTimes + 52(0x34)
	#define GLOW_LIFE_TIME 									0x3A4 // Script_Highlight_SetLifeTime + 4 / m_highlightServerFadeEndTimes + 36 (0x24)?
	#define OFFSET_WEAPON_NAME_INDEX                		0x17fc // 0x1874 //m_weaponNameIndex updated
	#endif
//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "npcevent.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "game.h"
#include "in_buttons.h"
#include "ai_memory.h"
#include "soundent.h"
#include "rumble_shared.h"
#include "gamestats.h"

#define BURST_BULLETS 3

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponOicw : public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponOicw, CHLSelectFireMachineGun);

	CWeaponOicw();

	DECLARE_SERVERCLASS();

	void	DrawHitmarker(void);

	void	Precache(void);
	void	AddViewKick(void);
	void	PrimaryAttack(void);
	void	SecondaryAttack(void);
	void	DryFire(void);
	void	PrimaryThink(void);
	void	FireRound(void);

	virtual bool	IsWeaponZoomed() { return m_bInZoom; }
	virtual void	ItemPostFrame(void);
	virtual void	ItemBusyFrame(void);
	virtual bool	Holster(CBaseCombatWeapon *pSwitchingTo = NULL);

	int		GetMinBurst() { return 3; }
	int		GetMaxBurst() { return 3; }


	virtual void	Equip(CBaseCombatCharacter *pOwner);
	virtual void	Drop(const Vector &vecVelocity);

	bool	Reload(void);

	float	GetFireRate(void) { return 0.075f; }	// Bullet Shooting Speed
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	Activity	GetPrimaryAttackActivity(void);

	virtual const Vector& GetBulletSpread(void)
	{
		static Vector cone;

		cone = VECTOR_CONE_1DEGREES;

		return cone;
	}

	const WeaponProficiencyInfo_t *GetProficiencyValues();

	void FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir);
	void Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	DECLARE_ACTTABLE();

private:
	bool				m_bInZoom;
	bool				m_bMustReload;

	void	CheckZoomToggle(void);
	void	ToggleZoom(void);

	int		m_iStance;

	float	m_flAttackEnds;

protected:

	Vector	m_vecTossVelocity;
	float	m_flNextGrenadeCheck;
};

IMPLEMENT_SERVERCLASS_ST(CWeaponOicw, DT_WeaponOICW)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_oicw, CWeaponOicw);
PRECACHE_WEAPON_REGISTER(weapon_oicw);

BEGIN_DATADESC(CWeaponOicw)

DEFINE_FIELD(m_vecTossVelocity, FIELD_VECTOR),
DEFINE_FIELD(m_flNextGrenadeCheck, FIELD_TIME),

DEFINE_THINKFUNC(PrimaryThink), //Register new think function

END_DATADESC()

acttable_t	CWeaponOicw::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SMG1, true },
	{ ACT_RELOAD, ACT_RELOAD_SMG1, true },
	{ ACT_IDLE, ACT_IDLE_SMG1, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_SMG1, true },

	{ ACT_WALK, ACT_WALK_RIFLE, true },
	{ ACT_WALK_AIM, ACT_WALK_AIM_RIFLE, true },

	// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED, ACT_IDLE_SMG1_RELAXED, false },//never aims
	{ ACT_IDLE_STIMULATED, ACT_IDLE_SMG1_STIMULATED, false },
	{ ACT_IDLE_AGITATED, ACT_IDLE_ANGRY_SMG1, false },//always aims

	{ ACT_WALK_RELAXED, ACT_WALK_RIFLE_RELAXED, false },//never aims
	{ ACT_WALK_STIMULATED, ACT_WALK_RIFLE_STIMULATED, false },
	{ ACT_WALK_AGITATED, ACT_WALK_AIM_RIFLE, false },//always aims

	{ ACT_RUN_RELAXED, ACT_RUN_RIFLE_RELAXED, false },//never aims
	{ ACT_RUN_STIMULATED, ACT_RUN_RIFLE_STIMULATED, false },
	{ ACT_RUN_AGITATED, ACT_RUN_AIM_RIFLE, false },//always aims

	// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED, ACT_IDLE_SMG1_RELAXED, false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED, ACT_IDLE_AIM_RIFLE_STIMULATED, false },
	{ ACT_IDLE_AIM_AGITATED, ACT_IDLE_ANGRY_SMG1, false },//always aims

	{ ACT_WALK_AIM_RELAXED, ACT_WALK_RIFLE_RELAXED, false },//never aims
	{ ACT_WALK_AIM_STIMULATED, ACT_WALK_AIM_RIFLE_STIMULATED, false },
	{ ACT_WALK_AIM_AGITATED, ACT_WALK_AIM_RIFLE, false },//always aims

	{ ACT_RUN_AIM_RELAXED, ACT_RUN_RIFLE_RELAXED, false },//never aims
	{ ACT_RUN_AIM_STIMULATED, ACT_RUN_AIM_RIFLE_STIMULATED, false },
	{ ACT_RUN_AIM_AGITATED, ACT_RUN_AIM_RIFLE, false },//always aims
	//End readiness activities

	{ ACT_WALK_AIM, ACT_WALK_AIM_RIFLE, true },
	{ ACT_WALK_CROUCH, ACT_WALK_CROUCH_RIFLE, true },
	{ ACT_WALK_CROUCH_AIM, ACT_WALK_CROUCH_AIM_RIFLE, true },
	{ ACT_RUN, ACT_RUN_RIFLE, true },
	{ ACT_RUN_AIM, ACT_RUN_AIM_RIFLE, true },
	{ ACT_RUN_CROUCH, ACT_RUN_CROUCH_RIFLE, true },
	{ ACT_RUN_CROUCH_AIM, ACT_RUN_CROUCH_AIM_RIFLE, true },
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_SMG1, true },
	{ ACT_RANGE_ATTACK1_LOW, ACT_RANGE_ATTACK_SMG1_LOW, true },
	{ ACT_COVER_LOW, ACT_COVER_SMG1_LOW, false },
	{ ACT_RANGE_AIM_LOW, ACT_RANGE_AIM_SMG1_LOW, false },
	{ ACT_RELOAD_LOW, ACT_RELOAD_SMG1_LOW, false },
	{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, true },
};

IMPLEMENT_ACTTABLE(CWeaponOicw);

//=========================================================
CWeaponOicw::CWeaponOicw()
{
	m_fMinRange1 = 0;// No minimum range. 
	m_fMaxRange1 = 4000; //Maximum Range

	m_bAltFiresUnderwater = false; //Allow using underwatter

	m_iFireMode = FIREMODE_3RNDBURST;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponOicw::Precache(void)
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Give this weapon longer range when wielded by an ally NPC.
//-----------------------------------------------------------------------------
void CWeaponOicw::Equip(CBaseCombatCharacter *pOwner)
{
	if (pOwner->Classify() == CLASS_PLAYER_ALLY)
	{
		m_fMaxRange1 = 3000;
	}
	else
	{
		m_fMaxRange1 = 1400;
	}

	BaseClass::Equip(pOwner);
}

void CWeaponOicw::PrimaryAttack(void)
{
	//Msg("%d\n", m_iBurstSize );
	if (m_bFireOnEmpty || m_iClip1 < 1)
	{
		return;
	}
	m_iBurstSize = BURST_BULLETS; //Always returns 3

	// Call the think function directly so that the first round gets fired immediately.
	WeaponSound(BURST);
	SetThink(&CWeaponOicw::PrimaryThink);
	SetNextThink(gpGlobals->curtime);
	m_flNextPrimaryAttack = gpGlobals->curtime + GetBurstCycleRate(); //GetBurstCycleRate returns 0.5
	m_flNextSecondaryAttack = gpGlobals->curtime + GetBurstCycleRate();

	// Pick up the rest of the burst through the think function.
	//SetNextThink(gpGlobals->curtime + GetFireRate()); //GetFireRate returns 0.1
	
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner)
	{
		m_iPrimaryAttacks++;
		gamestats->Event_WeaponFired(pOwner, true, GetClassname());
	}
}

void CWeaponOicw::PrimaryThink(void)
{
	if (m_iBurstSize > 0 && m_iClip1 > 0)
	{
		//Msg("%0.3f, BANG!!!\n", gpGlobals->curtime);
		m_iBurstSize--;
		FireRound();
		SetNextThink(gpGlobals->curtime + GetFireRate());
	}
	else
	{
		//Burst is over. Cancel the fire animation and future bursts.
		SetThink(NULL);
		SetWeaponIdleTime(gpGlobals->curtime);
	}
}

void CWeaponOicw::FireRound(void)
{
	m_nShotsFired++;
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner()); //This gets the current player holding the weapon
	Vector vecSrc = pPlayer->Weapon_ShootPosition();   //This simply just gets the current position of the player.
	Vector vecAim = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);   //This gets where the player is looking, but also corrected by autoaim.
	//Remove a bullet from the clip
	m_iClip1--;
	// Fire the bullets
	FireBulletsInfo_t info;
	//Number of bullets to fire
	info.m_iShots = 1;
	//Where to fire from
	info.m_vecSrc = vecSrc;
	//Where to fire to
	info.m_vecDirShooting = vecAim;
	//Bullet spread cone
	info.m_vecSpread = pPlayer->GetAttackSpread(this);
	//Maximum distance the bullet can travel
	info.m_flDistance = MAX_TRACE_LENGTH;
	//Type of ammo to use
	info.m_iAmmoType = m_iPrimaryAmmoType;
	//How often visible tracers should be drawn
	info.m_iTracerFreq = 1;
	//Weapon damage
	info.m_flDamage = 10.0;
	//Actually fire the bullet now that we've defined its properties
	FireBullets(info);
	/*pPlayer->FireBullets(1, vecSrc, vecAim, GetBulletSpread(), MAX_TRACE_LENGTH, 
		m_iPrimaryAmmoType, 1, entindex(), 0, 10, GetOwner(), true, true);*/
	//SendWeaponAnim(ACT_VM_PRIMARYATTACK); //This sends the animation for us shooting.
	//Instead of primary attack animation, just use the recoil animation
	SendWeaponAnim(ACT_VM_RECOIL3);
	AddViewKick();

	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}
}

void CWeaponOicw::SecondaryAttack(void)
{
	//Zoom is handled by post/pre frames
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponOicw::FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir)
{
	// FIXME: use the returned number of bullets to account for >10hz firerate
	WeaponSoundRealtime(SINGLE_NPC);

	CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());
	pOperator->FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED,
		MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2, entindex(), 0);

	pOperator->DoMuzzleFlash();
	m_iClip1 = m_iClip1 - 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponOicw::Operator_ForceNPCFire(CBaseCombatCharacter *pOperator, bool bSecondary)
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	Vector vecShootOrigin, vecShootDir;
	QAngle	angShootDir;
	GetAttachment(LookupAttachment("muzzle"), vecShootOrigin, angShootDir);
	AngleVectors(angShootDir, &vecShootDir);
	FireNPCPrimaryAttack(pOperator, vecShootOrigin, vecShootDir);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponOicw::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_SMG1:
	{
		Vector vecShootOrigin, vecShootDir;
		QAngle angDiscard;

		// Support old style attachment point firing
		if ((pEvent->options == NULL) || (pEvent->options[0] == '\0') || (!pOperator->GetAttachment(pEvent->options, vecShootOrigin, angDiscard)))
		{
			vecShootOrigin = pOperator->Weapon_ShootPosition();
		}

		CAI_BaseNPC *npc = pOperator->MyNPCPointer();
		ASSERT(npc != NULL);
		vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);

		FireNPCPrimaryAttack(pOperator, vecShootOrigin, vecShootDir);
	}
	break;

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CWeaponOicw::GetPrimaryAttackActivity(void)
{
	if (m_nShotsFired < 2)
	{
		return ACT_VM_PRIMARYATTACK;
	}

	if (m_nShotsFired < 3)
	{
		return ACT_VM_RECOIL1;
	}

	if (m_nShotsFired < 4)
	{
		return ACT_VM_RECOIL2;
	}

	return ACT_VM_RECOIL3;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponOicw::Reload(void)
{
	//NOTE: If you want to override the holster reload, you'll have to override the ItemHolsterFrame function
	//Msg("RELOAD\n");
	if (BaseClass::Reload())
	{
		if (m_bInZoom)
		{
			ToggleZoom();
		}
		m_bMustReload = false;
		WeaponSound(RELOAD);
		return true;
	}

	return false;
}

void CWeaponOicw::DryFire(void)
{
	if (m_bInZoom)
	{
		ToggleZoom();
	}
	WeaponSound(EMPTY);
	SendWeaponAnim(ACT_VM_DRYFIRE);
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}

void CWeaponOicw::Drop(const Vector &vecVelocity)
{
	if (m_bInZoom)
	{
		ToggleZoom();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponOicw::CheckZoomToggle(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer->m_afButtonPressed & IN_ATTACK2)
	{
		ToggleZoom();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponOicw::ItemBusyFrame(void)
{
	// Allow zoom toggling even when we're reloading
	CheckZoomToggle();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponOicw::ItemPostFrame(void)
{

	if (m_bMustReload && HasWeaponIdleTimeElapsed())
	{
		Reload();
	}

	BaseClass::ItemPostFrame();

	if (m_bInReload)
	{
		return;
	}

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
	{
		return;
	}

	if (pOwner->m_nButtons & IN_ATTACK)
	{
		if (m_flAttackEnds < gpGlobals->curtime)
		{
			SendWeaponAnim(ACT_VM_IDLE);
		}
	}
	else
	{
		if ((pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack < gpGlobals->curtime)
			&& (m_iClip1 <= 0))
		{
			DryFire();
		}
	}
	// Allow zoom toggling
	CheckZoomToggle();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponOicw::ToggleZoom(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	if (m_bInZoom)
	{
		if (pPlayer->SetFOV(this, 0, 0.2f))
		{
			m_bInZoom = false;
		}
	}
	else
	{
		if (pPlayer->SetFOV(this, 20, 0.1f))
		{
			m_bInZoom = true;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponOicw::AddViewKick(void)
{
#define	EASY_DAMPEN			0.5f
#define	MAX_VERTICAL_KICK	5.0f	//Degrees
#define	SLIDE_LIMIT			2.0f	//Seconds

	//Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	DoMachineGunKick(pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT);
}

bool CWeaponOicw::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	if (m_bInZoom)
	{
		ToggleZoom();
	}
	return BaseClass::Holster(pSwitchingTo);
}


//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponOicw::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0, 0.75 },
		{ 5.00, 0.75 },
		{ 10.0 / 3.0, 0.75 },
		{ 5.0 / 3.0, 0.75 },
		{ 1.00, 1.0 },
	};

	COMPILE_TIME_ASSERT(ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}
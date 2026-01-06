/*
===========================================================================
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

This file is part of Spearmint Source Code.

Spearmint Source Code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 3 of the License,
or (at your option) any later version.

Spearmint Source Code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Spearmint Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, Spearmint Source Code is also subject to certain additional terms.
You should have received a copy of these additional terms immediately following
the terms and conditions of the GNU General Public License.  If not, please
request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional
terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc.,
Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
//
// cg_weapons.c -- events and effects dealing with weapons
#include "cg_local.h"

#ifdef TA_HOLDSYS/*2*/
/*
===============
CG_NextHoldable
Based on CG_NextWeapon
===============
*/
void CG_NextHoldable_f( int localPlayerNum ) {
	int		i;
	int		original;
	playerState_t	*ps;
	localPlayer_t	*player;

	if ( cg.localPlayers[localPlayerNum].playerNum == -1 ) {
		return;
	}

	ps = &cg.snap->pss[localPlayerNum];
	player = &cg.localPlayers[localPlayerNum];

	if ( ps->pm_flags & PMF_FOLLOW ) {
		return;
	}

	//player->weaponSelectTime = cg.time;
	original = player->holdableSelect;

	if (player->holdableSelect == HI_NO_SELECT) {
		player->holdableSelect = ps->holdableIndex;
	}

	for ( i = 1 ; i < BG_NumHoldableItems() ; i++ ) {
		player->holdableSelect++;
		if ( player->holdableSelect == BG_NumHoldableItems() ) {
			player->holdableSelect = 1;
		}

#ifndef MISSIONPACK // if not MP skip its holdables.
		if (
#ifndef TURTLEARENA // NO_KAMIKAZE_ITEM
		player->holdableSelect == HI_KAMIKAZE ||
#endif
		player->holdableSelect == HI_PORTAL
#ifndef TURTLEARENA // POWERS
			|| player->holdableSelect == HI_INVULNERABILITY
#endif
			) {
			continue;
		}
#endif

		if ( ps->holdable[player->holdableSelect] != 0 ) {
			break;
		}
	}
	if ( i == BG_NumHoldableItems() ) {
		player->holdableSelect = original;
	}
}

/*
===============
CG_PrevHoldable
Based on CG_PrevWeapon
===============
*/
void CG_PrevHoldable_f( int localPlayerNum ) {
	int		i;
	int		original;
	playerState_t	*ps;
	localPlayer_t	*player;

	if ( cg.localPlayers[localPlayerNum].playerNum == -1 ) {
		return;
	}

	ps = &cg.snap->pss[localPlayerNum];
	player = &cg.localPlayers[localPlayerNum];

	if ( ps->pm_flags & PMF_FOLLOW ) {
		return;
	}

	//player->weaponSelectTime = cg.time;
	original = player->holdableSelect;

	if (player->holdableSelect == HI_NO_SELECT) {
		player->holdableSelect = ps->holdableIndex;
	}

	for ( i = 1 ; i < BG_NumHoldableItems() ; i++ ) {
		player->holdableSelect--;
		if ( player->holdableSelect == 0 ) {
			player->holdableSelect = BG_NumHoldableItems() - 1;
		}

#ifndef MISSIONPACK // if not MP skip its holdables.
		if (
#ifndef TURTLEARENA // NO_KAMIKAZE_ITEM
		player->holdableSelect == HI_KAMIKAZE ||
#endif
		player->holdableSelect == HI_PORTAL
#ifndef TURTLEARENA // POWERS
			|| player->holdableSelect == HI_INVULNERABILITY
#endif
			) {
			continue;
		}
#endif

		if ( ps->holdable[player->holdableSelect] != 0 ) {
			break;
		}
	}
	if ( i == BG_NumHoldableItems() ) {
		player->holdableSelect = original;
	}
}

/*
===============
CG_Holdable
Based on CG_Weapon
===============
*/
void CG_Holdable_f( int localPlayerNum ) {
	int		num;
	playerState_t	*ps;
	localPlayer_t	*player;

	if ( cg.localPlayers[localPlayerNum].playerNum == -1 ) {
		return;
	}

	ps = &cg.snap->pss[localPlayerNum];
	player = &cg.localPlayers[localPlayerNum];

	if ( ps->pm_flags & PMF_FOLLOW ) {
		return;
	}

	num = atoi( CG_Argv( 1 ) );

	if ( num < 0 || num >= BG_NumHoldableItems() ) {
		return;
	}

	//if (num != 0 && ps->holdable[num] == 0) {
	//	return;		// don't have the holdable item
	//}

	player->holdableSelect = num;
}
#endif

/*
==========================
CG_MachineGunEjectBrass
==========================
*/
static void CG_MachineGunEjectBrass( centity_t *cent
#ifdef TA_WEAPSYS
	, int handSide
#endif
) {
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			velocity, xvelocity;
	vec3_t			offset, xoffset;
	float			waterScale = 1.0f;
	vec3_t			v[3];

	if ( cg_brassTime.integer <= 0 ) {
		return;
	}

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	velocity[0] = 0;
	velocity[1] = -50 + 40 * crandom();
	velocity[2] = 100 + 50 * crandom();

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + cg_brassTime.integer + ( cg_brassTime.integer / 4 ) * random();

	le->pos.trType = TR_GRAVITY;
	le->pos.trTime = cg.time - (rand()&15);

	AnglesToAxis( cent->lerpAngles, v );

	offset[0] = 8;
#ifdef TA_WEAPSYS
	if (handSide == HS_RIGHT)
		offset[1] = -4;
	else if (handSide == HS_LEFT)
		offset[1] = 4;
	else
		offset[1] = 0;
#else
	offset[1] = -4;
#endif
	offset[2] = 24;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
	VectorAdd( cent->lerpOrigin, xoffset, re->origin );

	VectorCopy( re->origin, le->pos.trBase );

	if ( CG_PointContents( re->origin, -1 ) & CONTENTS_WATER ) {
		waterScale = 0.10f;
	}

	xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
	xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
	xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
	VectorScale( xvelocity, waterScale, le->pos.trDelta );

	AxisCopy( axisDefault, re->axis );
	re->hModel = cgs.media.machinegunBrassModel;

	le->bounceFactor = 0.4 * waterScale;

	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	le->angles.trBase[0] = rand()&31;
	le->angles.trBase[1] = rand()&31;
	le->angles.trBase[2] = rand()&31;
	le->angles.trDelta[0] = 2;
	le->angles.trDelta[1] = 1;
	le->angles.trDelta[2] = 0;

	le->leFlags = LEF_TUMBLE;
	le->leBounceSoundType = LEBS_BRASS;
	le->leMarkType = LEMT_NONE;
}

/*
==========================
CG_ShotgunEjectBrass
==========================
*/
static void CG_ShotgunEjectBrass( centity_t *cent
#ifdef TA_WEAPSYS
	, int handSide
#endif
	)
{
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			velocity, xvelocity;
	vec3_t			offset, xoffset;
	vec3_t			v[3];
	int				i;

	if ( cg_brassTime.integer <= 0 ) {
		return;
	}

	for ( i = 0; i < 2; i++ ) {
		float	waterScale = 1.0f;

		le = CG_AllocLocalEntity();
		re = &le->refEntity;

		velocity[0] = 60 + 60 * crandom();
		if ( i == 0 ) {
			velocity[1] = 40 + 10 * crandom();
		} else {
			velocity[1] = -40 + 10 * crandom();
		}
		velocity[2] = 100 + 50 * crandom();

		le->leType = LE_FRAGMENT;
		le->startTime = cg.time;
		le->endTime = le->startTime + cg_brassTime.integer*3 + cg_brassTime.integer * random();

		le->pos.trType = TR_GRAVITY;
		le->pos.trTime = cg.time;

		AnglesToAxis( cent->lerpAngles, v );

		offset[0] = 8;
#ifdef TA_WEAPSYS
		if (handSide == HS_RIGHT)
			offset[1] = -4;
		else if (handSide == HS_LEFT)
			offset[1] = 4;
		else
			offset[1] = 0;
#else
		offset[1] = 0;
#endif
		offset[2] = 24;

		xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
		xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
		xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
		VectorAdd( cent->lerpOrigin, xoffset, re->origin );
		VectorCopy( re->origin, le->pos.trBase );
		if ( CG_PointContents( re->origin, -1 ) & CONTENTS_WATER ) {
			waterScale = 0.10f;
		}

		xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
		xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
		xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
		VectorScale( xvelocity, waterScale, le->pos.trDelta );

		AxisCopy( axisDefault, re->axis );
		re->hModel = cgs.media.shotgunBrassModel;
		le->bounceFactor = 0.3f;

		le->angles.trType = TR_LINEAR;
		le->angles.trTime = cg.time;
		le->angles.trBase[0] = rand()&31;
		le->angles.trBase[1] = rand()&31;
		le->angles.trBase[2] = rand()&31;
		le->angles.trDelta[0] = 1;
		le->angles.trDelta[1] = 0.5;
		le->angles.trDelta[2] = 0;

		le->leFlags = LEF_TUMBLE;
		le->leBounceSoundType = LEBS_BRASS;
		le->leMarkType = LEMT_NONE;
	}
}


#if defined MISSIONPACK || defined TA_WEAPSYS
/*
==========================
CG_NailgunEjectBrass
==========================
*/
static void CG_NailgunEjectBrass( centity_t *cent
#ifdef TA_WEAPSYS
	, int handSide
#endif
) {
	localEntity_t	*smoke;
	vec3_t			origin;
	vec3_t			v[3];
	vec3_t			offset;
	vec3_t			xoffset;
	vec3_t			up;

	AnglesToAxis( cent->lerpAngles, v );

	offset[0] = 0;
#ifdef TA_WEAPSYS
	if (handSide == HS_RIGHT)
		offset[1] = -12;
	else if (handSide == HS_LEFT)
		offset[1] = 12;
	else
		offset[1] = 0;
#else
	offset[1] = -12;
#endif
	offset[2] = 24;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
	VectorAdd( cent->lerpOrigin, xoffset, origin );

	VectorSet( up, 0, 0, 64 );

	smoke = CG_SmokePuff( origin, up, 32, 1, 1, 1, 0.33f, 700, cg.time, 0, 0, cgs.media.smokePuffShader );
	// use the optimized local entity add
	smoke->leType = LE_SCALE_FADE;
}
#endif


/*
==========================
CG_RailTrail
==========================
*/
#ifdef TA_WEAPSYS
void CG_RailTrail (playerInfo_t *pi, const projectileInfo_t *wi, vec3_t start, vec3_t end)
#else
void CG_RailTrail (playerInfo_t *pi, vec3_t start, vec3_t end)
#endif
{
	vec3_t axis[36], move, move2, vec, temp;
	float  len;
	int    i, j, skip;
 
	localEntity_t *le;
	refEntity_t   *re;
 
#define RADIUS   4
#define ROTATION 1
#define SPACING  5
 
	start[2] -= 4;
 
	le = CG_AllocLocalEntity();
	re = &le->refEntity;
 
	le->leType = LE_FADE_RGB;
	le->startTime = cg.time;
#ifdef TA_WEAPSYS
	le->endTime = cg.time + (cg_railTrailTime.value / 400.0f) * wi->wiTrailTime;
#else
	le->endTime = cg.time + cg_railTrailTime.value;
#endif
	le->lifeRate = 1.0 / (le->endTime - le->startTime);
 
	re->shaderTime = cg.time;
	re->reType = RT_RAIL_CORE;
#ifdef TA_WEAPSYS
	re->customShader = wi->trailShader[0];
	re->radius = wi->trailRadius;
#else
	re->customShader = cgs.media.railCoreShader;
#endif
 
	VectorCopy(start, re->origin);
	VectorCopy(end, re->oldorigin);
 
	re->shaderRGBA[0] = pi->color1[0] * 255;
	re->shaderRGBA[1] = pi->color1[1] * 255;
	re->shaderRGBA[2] = pi->color1[2] * 255;
	re->shaderRGBA[3] = 255;

	le->color[0] = pi->color1[0] * 0.75;
	le->color[1] = pi->color1[1] * 0.75;
	le->color[2] = pi->color1[2] * 0.75;
	le->color[3] = 1.0f;

	AxisClear( re->axis );
 
	if (cg_oldRail.integer) {
#ifndef IOQ3ZTM
		// nudge down a bit so it isn't exactly in center
		re->origin[2] -= 8;
		re->oldorigin[2] -= 8;
#endif
		return;
	}

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);
	PerpendicularVector(temp, vec);
	for (i = 0 ; i < 36; i++)
	{
		RotatePointAroundVector(axis[i], vec, temp, i * 10);//banshee 2.4 was 10
	}

	VectorMA(move, 20, vec, move);
	VectorScale (vec, SPACING, vec);

	skip = -1;
 
	j = 18;
	for (i = 0; i < len; i += SPACING)
	{
		if (i != skip)
		{
			skip = i + SPACING;
			le = CG_AllocLocalEntity();
			re = &le->refEntity;
			le->leFlags = LEF_PUFF_DONT_SCALE;
			le->leType = LE_MOVE_SCALE_FADE;
			le->startTime = cg.time;
			le->endTime = cg.time + (i>>1) + 600;
			le->lifeRate = 1.0 / (le->endTime - le->startTime);

			re->shaderTime = cg.time;
			re->reType = RT_SPRITE;
			re->radius = 1.1f;
#ifdef TA_WEAPSYS
			re->customShader = wi->trailShader[1];
#else
			re->customShader = cgs.media.railRingsShader;
#endif

			re->shaderRGBA[0] = pi->color2[0] * 255;
			re->shaderRGBA[1] = pi->color2[1] * 255;
			re->shaderRGBA[2] = pi->color2[2] * 255;
			re->shaderRGBA[3] = 255;

			le->color[0] = pi->color2[0] * 0.75;
			le->color[1] = pi->color2[1] * 0.75;
			le->color[2] = pi->color2[2] * 0.75;
			le->color[3] = 1.0f;

			le->pos.trType = TR_LINEAR;
			le->pos.trTime = cg.time;

			VectorCopy( move, move2);
			VectorMA(move2, RADIUS , axis[j], move2);
			VectorCopy(move2, le->pos.trBase);

			le->pos.trDelta[0] = axis[j][0]*6;
			le->pos.trDelta[1] = axis[j][1]*6;
			le->pos.trDelta[2] = axis[j][2]*6;
		}

		VectorAdd (move, vec, move);

		j = (j + ROTATION) % 36;
	}
}

/*
==========================
CG_RocketTrail
==========================
*/
#ifdef TA_WEAPSYS
static void CG_RocketTrail( centity_t *ent, const projectileInfo_t *wi )
#else
static void CG_RocketTrail( centity_t *ent, const weaponInfo_t *wi )
#endif
{
	int		step;
	vec3_t	origin, lastPos;
	int		t;
	int		startTime, contents;
	int		lastContents;
	entityState_t	*es;
	vec3_t	up;
	localEntity_t	*smoke;

	if ( cg_noProjectileTrail.integer ) {
		return;
	}

	up[0] = 0;
	up[1] = 0;
	up[2] = 0;

	step = 50;

	es = &ent->currentState;
	startTime = ent->trailTime;
	t = step * ( (startTime + step) / step );

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );
	contents = CG_PointContents( origin, -1 );

	// if object (e.g. grenade) is stationary, don't toss up smoke
	if ( es->pos.trType == TR_STATIONARY ) {
		ent->trailTime = cg.time;
		return;
	}

	BG_EvaluateTrajectory( &es->pos, ent->trailTime, lastPos );
	lastContents = CG_PointContents( lastPos, -1 );

	ent->trailTime = cg.time;

	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		if ( contents & lastContents & CONTENTS_WATER ) {
			CG_BubbleTrail( lastPos, origin, 8 );
		}
		return;
	}

	for ( ; t <= ent->trailTime ; t += step ) {
		BG_EvaluateTrajectory( &es->pos, t, lastPos );

		smoke = CG_SmokePuff( lastPos, up, 
					  wi->trailRadius, 
					  1, 1, 1, 0.33f,
					  wi->wiTrailTime, 
					  t,
					  0,
					  0, 
					  cgs.media.smokePuffShader );
		// use the optimized local entity add
		smoke->leType = LE_SCALE_FADE;
	}

}

#ifdef MISSIONPACK
/*
==========================
CG_NailTrail
==========================
*/
#ifdef TA_WEAPSYS
static void CG_NailTrail( centity_t *ent, const projectileInfo_t *wi )
#else
static void CG_NailTrail( centity_t *ent, const weaponInfo_t *wi )
#endif
{
	int		step;
	vec3_t	origin, lastPos;
	int		t;
	int		startTime, contents;
	int		lastContents;
	entityState_t	*es;
	vec3_t	up;
	localEntity_t	*smoke;

	if ( cg_noProjectileTrail.integer ) {
		return;
	}

	up[0] = 0;
	up[1] = 0;
	up[2] = 0;

	step = 50;

	es = &ent->currentState;
	startTime = ent->trailTime;
	t = step * ( (startTime + step) / step );

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );
	contents = CG_PointContents( origin, -1 );

	// if object (e.g. grenade) is stationary, don't toss up smoke
	if ( es->pos.trType == TR_STATIONARY ) {
		ent->trailTime = cg.time;
		return;
	}

	BG_EvaluateTrajectory( &es->pos, ent->trailTime, lastPos );
	lastContents = CG_PointContents( lastPos, -1 );

	ent->trailTime = cg.time;

	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		if ( contents & lastContents & CONTENTS_WATER ) {
			CG_BubbleTrail( lastPos, origin, 8 );
		}
		return;
	}

	for ( ; t <= ent->trailTime ; t += step ) {
		BG_EvaluateTrajectory( &es->pos, t, lastPos );

		smoke = CG_SmokePuff( lastPos, up, 
					  wi->trailRadius, 
					  1, 1, 1, 0.33f,
					  wi->wiTrailTime, 
					  t,
					  0,
					  0, 
#ifdef TA_WEAPSYS
					  wi->trailShader[0]
#else
					  cgs.media.nailPuffShader
#endif
					  );
		// use the optimized local entity add
		smoke->leType = LE_SCALE_FADE;
	}

}
#endif

/*
==========================
CG_PlasmaTrail
==========================
*/
#ifdef TA_WEAPSYS
static void CG_PlasmaTrail( centity_t *cent, const projectileInfo_t *wi )
#else
static void CG_PlasmaTrail( centity_t *cent, const weaponInfo_t *wi )
#endif
{
	localEntity_t	*le;
	refEntity_t		*re;
	entityState_t	*es;
	vec3_t			velocity, xvelocity, origin;
	vec3_t			offset, xoffset;
	vec3_t			v[3];

	float	waterScale = 1.0f;

	if ( cg_noProjectileTrail.integer || cg_oldPlasma.integer ) {
		return;
	}

	es = &cent->currentState;

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	velocity[0] = 60 - 120 * crandom();
	velocity[1] = 40 - 80 * crandom();
	velocity[2] = 100 - 200 * crandom();

	le->leType = LE_MOVE_SCALE_FADE;
	le->leFlags = LEF_TUMBLE;
	le->leBounceSoundType = LEBS_NONE;
	le->leMarkType = LEMT_NONE;

	le->startTime = cg.time;
	le->endTime = le->startTime + 600;

	le->pos.trType = TR_GRAVITY;
	le->pos.trTime = cg.time;

	AnglesToAxis( cent->lerpAngles, v );

	offset[0] = 2;
	offset[1] = 2;
	offset[2] = 2;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];

	VectorAdd( origin, xoffset, re->origin );
	VectorCopy( re->origin, le->pos.trBase );

	if ( CG_PointContents( re->origin, -1 ) & CONTENTS_WATER ) {
		waterScale = 0.10f;
	}

	xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
	xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
	xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
	VectorScale( xvelocity, waterScale, le->pos.trDelta );

	AxisCopy( axisDefault, re->axis );
	re->shaderTime = cg.time;
	re->reType = RT_SPRITE;
	re->radius = 0.25f;
#ifdef TA_WEAPSYS
	re->customShader = wi->trailShader[0];
#else
	re->customShader = cgs.media.railRingsShader;
#endif
	le->bounceFactor = 0.3f;

#ifdef TA_WEAPSYS
	{
		// ZTM: NOTE: Can't get really flash color
		//                     as it is in weaponInfo_t
		vec3_t flashDlightColor = {0.6, 0.6, 1.0};
		re->shaderRGBA[0] = flashDlightColor[0] * 63;
		re->shaderRGBA[1] = flashDlightColor[1] * 63;
		re->shaderRGBA[2] = flashDlightColor[2] * 63;
		re->shaderRGBA[3] = 63;

		le->color[0] = flashDlightColor[0] * 0.2;
		le->color[1] = flashDlightColor[1] * 0.2;
		le->color[2] = flashDlightColor[2] * 0.2;
		le->color[3] = 0.25f;
	}
#else
	re->shaderRGBA[0] = wi->flashDlightColor[0] * 63;
	re->shaderRGBA[1] = wi->flashDlightColor[1] * 63;
	re->shaderRGBA[2] = wi->flashDlightColor[2] * 63;
	re->shaderRGBA[3] = 63;

	le->color[0] = wi->flashDlightColor[0] * 0.2;
	le->color[1] = wi->flashDlightColor[1] * 0.2;
	le->color[2] = wi->flashDlightColor[2] * 0.2;
	le->color[3] = 0.25f;
#endif

	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	le->angles.trBase[0] = rand()&31;
	le->angles.trBase[1] = rand()&31;
	le->angles.trBase[2] = rand()&31;
	le->angles.trDelta[0] = 1;
	le->angles.trDelta[1] = 0.5;
	le->angles.trDelta[2] = 0;

}
/*
==========================
CG_GrappleTrail
==========================
*/
#ifdef TA_WEAPSYS
void CG_GrappleTrail( centity_t *ent, const projectileInfo_t *wi )
#else
void CG_GrappleTrail( centity_t *ent, const weaponInfo_t *wi )
#endif
{
	vec3_t	origin;
	refEntity_t		beam;
#ifdef TA_WEAPSYS
	int				i;
#endif

	VectorCopy( ent->lerpOrigin, origin );
	ent->trailTime = cg.time;

	memset( &beam, 0, sizeof( beam ) );

#ifdef TA_WEAPSYS
	for (i = 0; i < MAX_HANDS; i++)
	{
		if (bg_weapongroupinfo[cg_entities[ent->currentState.otherEntityNum].currentState.weapon].weapon[i]->projnum == wi-cg_projectiles)
		{
			if ( !cg.cur_lc->renderingThirdPerson && cg.cur_ps->playerNum == ent->currentState.otherEntityNum ) {
				VectorCopy( cg.cur_lc->flashOrigin[i], beam.origin );
			} else {
				VectorCopy( cg_entities[ ent->currentState.otherEntityNum ].pe.flashOrigin[i], beam.origin );
			}
			break;
		}
	}

	if (i == MAX_HANDS)
	{
		// Grappling shuriken or something, use default origin
		VectorCopy(cg_entities[ ent->currentState.otherEntityNum ].lerpOrigin, beam.origin);
		beam.origin[2] += DEFAULT_VIEWHEIGHT - 6;
	}
#else
	if ( !cg.cur_lc->renderingThirdPerson && cg.cur_ps->playerNum == ent->currentState.otherEntityNum ) {
		VectorCopy( cg.cur_lc->flashOrigin, beam.origin );
	} else {
		VectorCopy( cg_entities[ ent->currentState.otherEntityNum ].pe.flashOrigin, beam.origin );
	}
#endif
	VectorCopy( origin, beam.oldorigin );

#ifdef TA_DATA
	// Switch points so chain pulls out of gun not projectile
	VectorCopy(beam.origin, beam.oldorigin);
	VectorCopy(origin, beam.origin);
#endif

	beam.reType = RT_LIGHTNING;
#ifdef TA_WEAPSYS
	beam.customShader = wi->trailShader[0];
	beam.radius = wi->trailRadius;
#else
	beam.customShader = cgs.media.lightningShader;
#endif

	AxisClear( beam.axis );
	beam.shaderRGBA[0] = 0xff;
	beam.shaderRGBA[1] = 0xff;
	beam.shaderRGBA[2] = 0xff;
	beam.shaderRGBA[3] = 0xff;
	CG_AddRefEntityWithMinLight( &beam );
}

/*
==========================
CG_GrenadeTrail
==========================
*/
#ifdef TA_WEAPSYS
static void CG_GrenadeTrail( centity_t *ent, const projectileInfo_t *wi )
#else
static void CG_GrenadeTrail( centity_t *ent, const weaponInfo_t *wi )
#endif
{
	CG_RocketTrail( ent, wi );
}

#ifdef TA_WEAPSYS
/*
==========================
CG_SparkTrail
==========================
*/
static void CG_SparkTrail( centity_t *ent, const projectileInfo_t *wi )
{
	int		step;
	vec3_t	origin, lastPos;
	int		t;
	int		startTime;
	entityState_t	*es;
	vec3_t	up;
	localEntity_t	*smoke;

	if ( cg_noProjectileTrail.integer ) {
		return;
	}

	up[0] = 0;
	up[1] = 0;
	up[2] = 0;

	step = 50;

	es = &ent->currentState;
	startTime = ent->trailTime;
	t = step * ( (startTime + step) / step );

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );

	// if object (e.g. grenade) is stationary, don't toss up smoke
	if ( es->pos.trType == TR_STATIONARY ) {
		ent->trailTime = cg.time;
		return;
	}

	BG_EvaluateTrajectory( &es->pos, ent->trailTime, lastPos );

	ent->trailTime = cg.time;

	for ( ; t <= ent->trailTime ; t += step ) {
		BG_EvaluateTrajectory( &es->pos, t, lastPos );

		smoke = CG_SmokePuff( lastPos, up,
					  wi->trailRadius,
					  1, 1, 1, 0.33f,
					  wi->wiTrailTime,
					  t,
					  0,
					  0,
					  wi->trailShader[0] );
		// use the optimized local entity add
		smoke->leType = LE_SCALE_FADE;

		// ZTM: add dynamic light
		if ( wi->missileDlight*0.66f > 0 ) {
			smoke->light = wi->missileDlight*0.66f; // 2/3 size
			VectorCopy(wi->missileDlightColor, smoke->lightColor);
		}
	}

}
#endif

#ifdef TURTLEARENA // HOLD_SHURIKEN
// Currently this should only be called by CG_RegisterItemVisuals
void CG_RegisterHoldable( int holdableNum )
{
	//CG_RegisterItemVisuals(BG_ItemNumForHoldableNum(holdableNum));

	CG_RegisterProjectile(BG_ProjectileIndexForHoldable(holdableNum));
}
#endif

#ifdef TA_WEAPSYS
void CG_RegisterProjectile( int projectileNum )
{
	projectileInfo_t *projectileInfo;
	bg_projectileinfo_t *bgProj;
	int i;

	projectileInfo = &cg_projectiles[projectileNum];
	bgProj = &bg_projectileinfo[projectileNum];
	if ( projectileInfo->registered ) {
		return;
	}

	memset( projectileInfo, 0, sizeof( *projectileInfo ) );
	projectileInfo->registered = qtrue;

	VectorCopy(bgProj->missileDlightColor, projectileInfo->missileDlightColor);
	projectileInfo->missileDlight = bgProj->missileDlight;

	if (bgProj->missileSoundName[0] != '\0')
		projectileInfo->missileSound = trap_S_RegisterSound( bgProj->missileSoundName, qfalse );

	// Hit mark and sounds
	if (bgProj->hitMarkName[0] != '\0')
		projectileInfo->hitMarkShader = trap_R_RegisterShader(bgProj->hitMarkName);
	projectileInfo->hitMarkRadius = bgProj->hitMarkRadius;

	for (i = 0; i< 3; i++)
	{
		if (bgProj->hitSoundName[i][0] != '\0')
			projectileInfo->hitSound[i] = trap_S_RegisterSound( bgProj->hitSoundName[i], qfalse );
	}
	if (bgProj->hitPlayerSoundName[0] != '\0')
		projectileInfo->hitPlayerSound = trap_S_RegisterSound( bgProj->hitPlayerSoundName, qfalse );
	if (bgProj->hitMetalSoundName[0] != '\0')
		projectileInfo->hitMetalSound = trap_S_RegisterSound( bgProj->hitMetalSoundName, qfalse );

	// Impact mark and sounds
	if (bgProj->impactMarkName[0] != '\0')
		projectileInfo->impactMarkShader = trap_R_RegisterShader(bgProj->impactMarkName);
	projectileInfo->impactMarkRadius = bgProj->impactMarkRadius;

	for (i = 0; i < 3; i++)
	{
		if (bgProj->impactSoundName[i][0] != '\0')
			projectileInfo->impactSound[i] = trap_S_RegisterSound( bgProj->impactSoundName[i], qfalse );
	}
	if (bgProj->impactPlayerSoundName[0] != '\0')
		projectileInfo->impactPlayerSound = trap_S_RegisterSound( bgProj->impactPlayerSoundName, qfalse );
	if (bgProj->impactMetalSoundName[0] != '\0')
		projectileInfo->impactMetalSound = trap_S_RegisterSound( bgProj->impactMetalSoundName, qfalse );

	if (bgProj->triggerSoundName[0] != '\0')
		projectileInfo->triggerSound = trap_S_RegisterSound( bgProj->triggerSoundName, qfalse );


	if (bgProj->model[0] != '\0')
	{
		projectileInfo->missileModel = trap_R_RegisterModel(bgProj->model);
		// Use normal model if blue/red versions are not set.
		projectileInfo->missileModelBlue = projectileInfo->missileModel;
		projectileInfo->missileModelRed = projectileInfo->missileModel;
	}
	if (bgProj->modelBlue[0] != '\0')
		projectileInfo->missileModelBlue = trap_R_RegisterModel(bgProj->modelBlue);
	if (bgProj->modelRed[0] != '\0')
		projectileInfo->missileModelRed = trap_R_RegisterModel(bgProj->modelRed);
	if (bgProj->sprite[0] != '\0')
		projectileInfo->spriteShader = trap_R_RegisterShader(bgProj->sprite);
	projectileInfo->spriteRadius = bgProj->spriteRadius;
	switch (bgProj->trailType)
	{
		default:
		case PT_NONE:
			break;

		case PT_PLASMA:
			projectileInfo->trailShader[0] = trap_R_RegisterShader( "railDisc" );
			projectileInfo->missileTrailFunc = CG_PlasmaTrail;
			break;
		case PT_ROCKET:
			// Uses cgs.media.smokePuffShader but it is used by other
			//    stuff so it is always loaded.
			projectileInfo->trailShader[0] = cgs.media.smokePuffShader;
			projectileInfo->missileTrailFunc = CG_RocketTrail;
			projectileInfo->wiTrailTime = 2000;
			projectileInfo->trailRadius = 64;
			break;
		case PT_GRENADE:
			// Uses CG_RocketTrail ... so don't need to load
			//    cgs.media.smokePuffShader
			projectileInfo->trailShader[0] = cgs.media.smokePuffShader;
			projectileInfo->missileTrailFunc = CG_GrenadeTrail;
			projectileInfo->wiTrailTime = 700;
			projectileInfo->trailRadius = 32;
			break;
		case PT_GRAPPLE:
			projectileInfo->missileTrailFunc = CG_GrappleTrail;
			projectileInfo->trailShader[0] = trap_R_RegisterShader( "GrappleCable" );
#if !defined TURTLEARENA || defined TA_SUPPORTQ3
			if (!projectileInfo->trailShader[0])
				projectileInfo->trailShader[0] = trap_R_RegisterShader( "lightningBoltNew" );
#endif
			projectileInfo->trailRadius = 8;
			break;
		case PT_NAIL:
#ifdef MISSIONPACK
			projectileInfo->trailShader[0] = trap_R_RegisterShader( "nailtrail" );
			projectileInfo->missileTrailFunc = CG_NailTrail;
			projectileInfo->trailRadius = 16;
			projectileInfo->wiTrailTime = 250;
#else // Quake3 can use rocket trail instead...
			// Uses CG_RocketTrail ... so don't need to load
			//    cgs.media.smokePuffShader
			projectileInfo->trailShader[0] = cgs.media.smokePuffShader;
			projectileInfo->missileTrailFunc = CG_RocketTrail;
			projectileInfo->trailRadius = 16;
			projectileInfo->wiTrailTime = 250;
#endif
			break;
		case PT_LIGHTNING:
			projectileInfo->trailShader[0] = trap_R_RegisterShader( "lightningBoltNew");
			projectileInfo->missileTrailFunc = CG_GrappleTrail;
			projectileInfo->trailRadius = 8;
			break;
		case PT_RAIL:
			projectileInfo->trailShader[0] = trap_R_RegisterShader( "railCore" );
			projectileInfo->trailShader[1] = trap_R_RegisterShader( "railDisc" );
			projectileInfo->wiTrailTime = 400;
			projectileInfo->trailRadius = 6;
			break;
		case PT_BULLET:
			break;
		case PT_SPARKS:
			projectileInfo->trailShader[0] = trap_R_RegisterShader( "sparkTrail" );
			projectileInfo->missileTrailFunc = CG_SparkTrail;
			projectileInfo->wiTrailTime = 300;
			projectileInfo->trailRadius = 16;
			break;
	}

	for (i = 0; i < 2; i++) {
		if (bgProj->trailShaderName[i][0] != '\0') {
			projectileInfo->trailShader[i] = trap_R_RegisterShader( bgProj->trailShaderName[i] );
		}
	}
	if (bgProj->trailRadius > 0) {
		projectileInfo->trailRadius = bgProj->trailRadius;
	}
	if (bgProj->trailTime > 0) {
		projectileInfo->wiTrailTime = bgProj->trailTime;
	}

	switch (bgProj->deathType)
	{
		default:
		case PD_NONE:
		case PD_NONE_EXP_PLAYER:
			break;
		case PD_PLASMA:
			cgs.media.ringFlashModel = trap_R_RegisterModel("models/weaphits/ring02.md3");
			cgs.media.plasmaExplosionShader = trap_R_RegisterShader( "plasmaExplosion" );
			cgs.media.sfx_plasmaexp = trap_S_RegisterSound ("sound/weapons/plasma/plasmx1a.wav", qfalse);
			break;
		case PD_ROCKET:
		case PD_ROCKET_SMALL:
			cgs.media.dishFlashModel = trap_R_RegisterModel("models/weaphits/boom01.md3");
			cgs.media.rocketExplosionShader = trap_R_RegisterShader( "rocketExplosion" );
			cgs.media.sfx_rockexp = trap_S_RegisterSound ("sound/weapons/rocket/rocklx1a.wav", qfalse);
			break;
		case PD_BULLET:
			cgs.media.bulletFlashModel = trap_R_RegisterModel("models/weaphits/bullet.md3");
			cgs.media.bulletExplosionShader = trap_R_RegisterShader( "bulletExplosion" );
			cgs.media.sfx_ric1 = trap_S_RegisterSound ("sound/weapons/machinegun/ric1.wav", qfalse);
			cgs.media.sfx_ric2 = trap_S_RegisterSound ("sound/weapons/machinegun/ric2.wav", qfalse);
			cgs.media.sfx_ric3 = trap_S_RegisterSound ("sound/weapons/machinegun/ric3.wav", qfalse);
			break;
		case PD_BULLET_COLORIZE:
			cgs.media.bulletFlashModel = trap_R_RegisterModel("models/weaphits/bullet.md3");
			cgs.media.bulletExplosionColorizeShader = trap_R_RegisterShader( "bulletExplosionColorize" );
			if (!cgs.media.bulletExplosionColorizeShader)
				cgs.media.bulletExplosionColorizeShader = trap_R_RegisterShader( "bulletExplosion" );
			cgs.media.sfx_ric1 = trap_S_RegisterSound ("sound/weapons/machinegun/ric1.wav", qfalse);
			cgs.media.sfx_ric2 = trap_S_RegisterSound ("sound/weapons/machinegun/ric2.wav", qfalse);
			cgs.media.sfx_ric3 = trap_S_RegisterSound ("sound/weapons/machinegun/ric3.wav", qfalse);
			break;
		case PD_GRENADE:
			cgs.media.dishFlashModel = trap_R_RegisterModel("models/weaphits/boom01.md3");
			cgs.media.grenadeExplosionShader = trap_R_RegisterShader( "grenadeExplosion" );
#ifdef TA_DATA
			if (!cgs.media.grenadeExplosionShader)
				cgs.media.grenadeExplosionShader = trap_R_RegisterShader( "rocketExplosion" );
#endif
			break;
		case PD_RAIL:
			cgs.media.ringFlashModel = trap_R_RegisterModel("models/weaphits/ring02.md3");
			cgs.media.railExplosionShader = trap_R_RegisterShader( "railExplosion" );
#ifdef TA_DATA
			if (!cgs.media.railExplosionShader)
				cgs.media.railExplosionShader = trap_R_RegisterShader( "plasmaExplosion" );
#endif
			break;
		case PD_BFG:
			cgs.media.dishFlashModel = trap_R_RegisterModel("models/weaphits/boom01.md3");
			cgs.media.bfgExplosionShader = trap_R_RegisterShader( "bfgExplosion" );
#ifdef TA_DATA
			if (!cgs.media.bfgExplosionShader)
				cgs.media.bfgExplosionShader = trap_R_RegisterShader( "rocketExplosion" );
#endif
			break;
		case PD_LIGHTNING:
			cgs.media.lightningExplosionModel = trap_R_RegisterModel( "models/weaphits/crackle.md3" );
			break;
	}
}

void CG_RegisterWeapon( int weaponNum )
{
	weaponInfo_t *weaponInfo;
	bg_weaponinfo_t *weap;
	char			path[MAX_QPATH];
	int i;

	weaponInfo = &cg_weapons[weaponNum];
	weap = &bg_weaponinfo[weaponNum];

	if ( weaponInfo->registered ) {
		return;
	}

	memset( weaponInfo, 0, sizeof( *weaponInfo ) );
	weaponInfo->registered = qtrue;

	if (weap->model[0] != '\0') {
		weaponInfo->weaponModel = trap_R_RegisterModel( weap->model );

		COM_StripExtension( weap->model, path, sizeof(path) );
		Q_strcat( path, sizeof(path), "_flash.md3" );
		weaponInfo->flashModel = trap_R_RegisterModel( path );

		COM_StripExtension( weap->model, path, sizeof(path) );
		Q_strcat( path, sizeof(path), "_barrel.md3" );
		weaponInfo->barrelModel = trap_R_RegisterModel( path );
	}

	VectorCopy(weap->flashColor, weaponInfo->flashDlightColor);

	// Impact mark
	if (weap->impactMarkName[0] != '\0')
		weaponInfo->impactMarkShader = trap_R_RegisterShader(weap->impactMarkName);
	weaponInfo->impactMarkRadius = weap->impactMarkRadius;

	// Sounds
	if (weap->flashSoundName[0][0] != '\0')
		weaponInfo->flashSound[0] = trap_S_RegisterSound( weap->flashSoundName[0], qfalse );
	if (weap->flashSoundName[1][0] != '\0')
		weaponInfo->flashSound[1] = trap_S_RegisterSound( weap->flashSoundName[1], qfalse );
	if (weap->flashSoundName[2][0] != '\0')
		weaponInfo->flashSound[2] = trap_S_RegisterSound( weap->flashSoundName[2], qfalse );
	if (weap->flashSoundName[3][0] != '\0')
		weaponInfo->flashSound[3] = trap_S_RegisterSound( weap->flashSoundName[3], qfalse );

	for (i = 0; i< 3; i++)
	{
		if (weap->impactSoundName[i][0] != '\0')
			weaponInfo->impactSound[i] = trap_S_RegisterSound( weap->impactSoundName[i], qfalse );
	}
	if (weap->impactPlayerSoundName[0] != '\0')
		weaponInfo->impactPlayerSound = trap_S_RegisterSound( weap->impactPlayerSoundName, qfalse );
	if (weap->impactMetalSoundName[0] != '\0')
		weaponInfo->impactMetalSound = trap_S_RegisterSound( weap->impactMetalSoundName, qfalse );

	// \unused
	//weaponInfo->ejectBrassFunc = NULL;

	CG_RegisterProjectile(weap->projnum);
}
#endif

/*
=================
CG_RegisterWeapon

The server says this item is used on this level
=================
*/
#ifdef TA_WEAPSYS
void CG_RegisterWeaponGroup( int weaponNum )
#else
void CG_RegisterWeapon( int weaponNum )
#endif
{
#ifdef TA_WEAPSYS
	weaponGroupInfo_t	*weaponInfo;
#else
	weaponInfo_t	*weaponInfo;
#endif
#ifdef TURTLEARENA // WEAPONS
	gitem_t			*item;
#else
	gitem_t			*item, *ammo;
#endif
	char			path[MAX_QPATH];
	vec3_t			mins, maxs;
	int				i;

#ifdef TA_WEAPSYS
	weaponInfo = &cg_weapongroups[weaponNum];
#else
	weaponInfo = &cg_weapons[weaponNum];
#endif

#ifdef TA_WEAPSYS // Safety check
	if ( weaponNum <= 0 || weaponNum >= BG_NumWeaponGroups() ) {
		return;
	}
#else
	if ( weaponNum == 0 ) {
		return;
	}
#endif

	if ( weaponInfo->registered ) {
		return;
	}

	memset( weaponInfo, 0, sizeof( *weaponInfo ) );
	weaponInfo->registered = qtrue;

#ifndef TA_WEAPSYS_EX
	weaponInfo->item = /*item on next line */
#endif
	item = BG_FindItemForWeapon( weaponNum );
	if ( !item ) {
		CG_Error( "Couldn't find weapon %i", weaponNum );
	}
	CG_RegisterItemVisuals( BG_ItemNumForItem( item ) );

	// load cmodel before model so filecache works
#ifdef TA_WEAPSYS
	if ( item->classname )
	{
#ifdef TURTLEARENA // NOAMMO
		weaponInfo->weaponModel = trap_R_RegisterModel( item->world_model[0] );

		// calc midpoint for rotation
		trap_R_ModelBounds( weaponInfo->weaponModel, mins, maxs, 0, 0, 0 );

		// Move low weapons like the Bo Staff out of the ground.
		if (mins[2] < -4) {
			float heightOffset = fabs(mins[2]);

			mins[2] -= heightOffset;
			maxs[2] -= heightOffset;

			//CG_Printf("DEBUG: %s: Low weapon, offset=%f\n", bg_weapongroupinfo[weaponNum].name, heightOffset);
		}

		for ( i = 0 ; i < 3 ; i++ ) {
			weaponInfo->weaponMidpoint[i] = mins[i] + 0.5 * ( maxs[i] - mins[i] );
		}
#else
		// calc midpoint for rotation
		trap_R_ModelBounds( trap_R_RegisterModel( item->world_model[0] ), mins, maxs, 0, 0, 0 );
		for ( i = 0 ; i < 3 ; i++ ) {
			weaponInfo->weaponMidpoint[i] = mins[i] + 0.5 * ( maxs[i] - mins[i] );
		}
#endif
	}
#else
	weaponInfo->weaponModel = trap_R_RegisterModel( item->world_model[0] );

	// calc midpoint for rotation
	trap_R_ModelBounds( weaponInfo->weaponModel, mins, maxs, 0, 0, 0 );
	for ( i = 0 ; i < 3 ; i++ ) {
		weaponInfo->weaponMidpoint[i] = mins[i] + 0.5 * ( maxs[i] - mins[i] );
	}
#endif

#ifdef TA_WEAPSYS
	weaponInfo->weaponIcon = trap_R_RegisterShader( bg_weapongroupinfo[weaponNum].item->icon );
#else
	weaponInfo->weaponIcon = trap_R_RegisterShader( item->icon );
#endif
#ifndef TURTLEARENA // WEAPONS
	weaponInfo->ammoIcon = trap_R_RegisterShader( item->icon );

	ammo = BG_FindItemForAmmo( weaponNum );
	if ( ammo && ammo->world_model[0] ) {
		weaponInfo->ammoModel = trap_R_RegisterModel( ammo->world_model[0] );
	}
#endif

#ifndef TA_WEAPSYS
	strcpy( path, item->world_model[0] );
	COM_StripExtension(path, path, sizeof(path));
	strcat( path, "_flash.md3" );
	weaponInfo->flashModel = trap_R_RegisterModel( path );

	strcpy( path, item->world_model[0] );
	COM_StripExtension(path, path, sizeof(path));
	strcat( path, "_barrel.md3" );
	weaponInfo->barrelModel = trap_R_RegisterModel( path );
#endif

#ifdef TA_WEAPSYS
	if (bg_weapongroupinfo[weaponNum].handsModelName[0] != '\0') {
		weaponInfo->handsModel = trap_R_RegisterModel( bg_weapongroupinfo[weaponNum].handsModelName );
	}

	if (item && !weaponInfo->handsModel)
	{
#endif
	strcpy( path, item->world_model[0] );
	COM_StripExtension(path, path, sizeof(path));
	strcat( path, "_hand.md3" );
	weaponInfo->handsModel = trap_R_RegisterModel( path );
#ifdef TA_WEAPSYS
	}
#endif

#ifndef TA_MISC // USE_REAL_TAGS_FOR_VIEW
	if ( !weaponInfo->handsModel ) {
		weaponInfo->handsModel = trap_R_RegisterModel( "models/weapons2/shotgun/shotgun_hand.md3" );
	}
#endif

#ifdef TA_WEAPSYS
	if (bg_weapongroupinfo[weaponNum].readySoundName[0] != '\0')
		weaponInfo->readySound = trap_S_RegisterSound( bg_weapongroupinfo[weaponNum].readySoundName, qfalse );
	if (bg_weapongroupinfo[weaponNum].firingSoundName[0] != '\0')
		weaponInfo->firingSound = trap_S_RegisterSound( bg_weapongroupinfo[weaponNum].firingSoundName, qfalse );
	if (bg_weapongroupinfo[weaponNum].firingStoppedSoundName[0] != '\0')
		weaponInfo->firingStoppedSound = trap_S_RegisterSound( bg_weapongroupinfo[weaponNum].firingStoppedSoundName, qfalse );

	for (i = 0; i < MAX_HANDS; i++) {
		CG_RegisterWeapon(bg_weapongroupinfo[weaponNum].weaponnum[i]);
	}
#else
	switch ( weaponNum ) {
	case WP_GAUNTLET:
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f );
		weaponInfo->firingSound = trap_S_RegisterSound( "sound/weapons/melee/fstrun.wav", qfalse );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/melee/fstatck.wav", qfalse );
		break;

	case WP_LIGHTNING:
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f );
		weaponInfo->readySound = trap_S_RegisterSound( "sound/weapons/melee/fsthum.wav", qfalse );
		weaponInfo->firingSound = trap_S_RegisterSound( "sound/weapons/lightning/lg_hum.wav", qfalse );

		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/lightning/lg_fire.wav", qfalse );
		cgs.media.lightningShader = trap_R_RegisterShader( "lightningBoltNew");
		cgs.media.lightningExplosionModel = trap_R_RegisterModel( "models/weaphits/crackle.md3" );
		cgs.media.sfx_lghit1 = trap_S_RegisterSound( "sound/weapons/lightning/lg_hit.wav", qfalse );
		cgs.media.sfx_lghit2 = trap_S_RegisterSound( "sound/weapons/lightning/lg_hit2.wav", qfalse );
		cgs.media.sfx_lghit3 = trap_S_RegisterSound( "sound/weapons/lightning/lg_hit3.wav", qfalse );

		break;

	case WP_GRAPPLING_HOOK:
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f );
		weaponInfo->missileModel = trap_R_RegisterModel( "models/ammo/rocket/rocket.md3" );
		weaponInfo->missileTrailFunc = CG_GrappleTrail;
		weaponInfo->missileDlight = 200;
		MAKERGB( weaponInfo->missileDlightColor, 1, 0.75f, 0 );
		weaponInfo->readySound = trap_S_RegisterSound( "sound/weapons/melee/fsthum.wav", qfalse );
		weaponInfo->firingSound = trap_S_RegisterSound( "sound/weapons/melee/fstrun.wav", qfalse );
		cgs.media.lightningShader = trap_R_RegisterShader( "lightningBoltNew");
		break;

#ifdef MISSIONPACK
	case WP_CHAINGUN:
		weaponInfo->firingSound = trap_S_RegisterSound( "sound/weapons/vulcan/wvulfire.wav", qfalse );
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/vulcan/vulcanf1b.wav", qfalse );
		weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/vulcan/vulcanf2b.wav", qfalse );
		weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons/vulcan/vulcanf3b.wav", qfalse );
		weaponInfo->flashSound[3] = trap_S_RegisterSound( "sound/weapons/vulcan/vulcanf4b.wav", qfalse );
		weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
		cgs.media.bulletExplosionShader = trap_R_RegisterShader( "bulletExplosion" );
		break;
#endif

	case WP_MACHINEGUN:
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/machinegun/machgf1b.wav", qfalse );
		weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/machinegun/machgf2b.wav", qfalse );
		weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons/machinegun/machgf3b.wav", qfalse );
		weaponInfo->flashSound[3] = trap_S_RegisterSound( "sound/weapons/machinegun/machgf4b.wav", qfalse );
		weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
		cgs.media.bulletExplosionShader = trap_R_RegisterShader( "bulletExplosion" );
		break;

	case WP_SHOTGUN:
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/shotgun/sshotf1b.wav", qfalse );
		weaponInfo->ejectBrassFunc = CG_ShotgunEjectBrass;
		break;

	case WP_ROCKET_LAUNCHER:
		weaponInfo->missileModel = trap_R_RegisterModel( "models/ammo/rocket/rocket.md3" );
		weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/rocket/rockfly.wav", qfalse );
		weaponInfo->missileTrailFunc = CG_RocketTrail;
		weaponInfo->missileDlight = 200;
		weaponInfo->wiTrailTime = 2000;
		weaponInfo->trailRadius = 64;
		
		MAKERGB( weaponInfo->missileDlightColor, 1, 0.75f, 0 );
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.75f, 0 );

		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/rocket/rocklf1a.wav", qfalse );
		cgs.media.rocketExplosionShader = trap_R_RegisterShader( "rocketExplosion" );
		break;

#ifdef MISSIONPACK
	case WP_PROX_LAUNCHER:
		weaponInfo->missileModel = trap_R_RegisterModel( "models/weaphits/proxmine.md3" );
		weaponInfo->missileTrailFunc = CG_GrenadeTrail;
		weaponInfo->wiTrailTime = 700;
		weaponInfo->trailRadius = 32;
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.70f, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/proxmine/wstbfire.wav", qfalse );
		cgs.media.grenadeExplosionShader = trap_R_RegisterShader( "grenadeExplosion" );
		break;
#endif

	case WP_GRENADE_LAUNCHER:
		weaponInfo->missileModel = trap_R_RegisterModel( "models/ammo/grenade1.md3" );
		weaponInfo->missileTrailFunc = CG_GrenadeTrail;
		weaponInfo->wiTrailTime = 700;
		weaponInfo->trailRadius = 32;
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.70f, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/grenade/grenlf1a.wav", qfalse );
		cgs.media.grenadeExplosionShader = trap_R_RegisterShader( "grenadeExplosion" );
		break;

#ifdef MISSIONPACK
	case WP_NAILGUN:
		weaponInfo->ejectBrassFunc = CG_NailgunEjectBrass;
		weaponInfo->missileTrailFunc = CG_NailTrail;
//		weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/nailgun/wnalflit.wav", qfalse );
		weaponInfo->trailRadius = 16;
		weaponInfo->wiTrailTime = 250;
		weaponInfo->missileModel = trap_R_RegisterModel( "models/weaphits/nail.md3" );
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.75f, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/nailgun/wnalfire.wav", qfalse );
		break;
#endif

	case WP_PLASMAGUN:
//		weaponInfo->missileModel = cgs.media.invulnerabilityPowerupModel;
		weaponInfo->missileTrailFunc = CG_PlasmaTrail;
		weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/plasma/lasfly.wav", qfalse );
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/plasma/hyprbf1a.wav", qfalse );
		cgs.media.plasmaExplosionShader = trap_R_RegisterShader( "plasmaExplosion" );
		cgs.media.railRingsShader = trap_R_RegisterShader( "railDisc" );
		break;

	case WP_RAILGUN:
		weaponInfo->readySound = trap_S_RegisterSound( "sound/weapons/railgun/rg_hum.wav", qfalse );
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.5f, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/railgun/railgf1a.wav", qfalse );
		cgs.media.railExplosionShader = trap_R_RegisterShader( "railExplosion" );
		cgs.media.railRingsShader = trap_R_RegisterShader( "railDisc" );
		cgs.media.railCoreShader = trap_R_RegisterShader( "railCore" );
		break;

	case WP_BFG:
		weaponInfo->readySound = trap_S_RegisterSound( "sound/weapons/bfg/bfg_hum.wav", qfalse );
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.7f, 1 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/bfg/bfg_fire.wav", qfalse );
		cgs.media.bfgExplosionShader = trap_R_RegisterShader( "bfgExplosion" );
		weaponInfo->missileModel = trap_R_RegisterModel( "models/weaphits/bfg.md3" );
		weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/rocket/rockfly.wav", qfalse );
		break;

	 default:
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 1 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/rocket/rocklf1a.wav", qfalse );
		break;
	}
#endif
}

/*
=================
CG_RegisterItemVisuals

The server says this item is used on this level
=================
*/
void CG_RegisterItemVisuals( int itemNum ) {
	itemInfo_t	*itemInfo;
	gitem_t		*item;

	if ( itemNum < 0 || itemNum >= BG_NumItems() ) {
		CG_Error( "CG_RegisterItemVisuals: itemNum %d out of range [0-%d]", itemNum, BG_NumItems()-1 );
	}

	itemInfo = &cg_items[ itemNum ];
	if ( itemInfo->registered ) {
		return;
	}

	item = BG_ItemForItemNum( itemNum );

	memset( itemInfo, 0, sizeof( &itemInfo ) );
	itemInfo->registered = qtrue;

	itemInfo->models[0] = trap_R_RegisterModel( item->world_model[0] );

	itemInfo->icon = trap_R_RegisterShader( item->icon );

#ifdef IOQ3ZTM // FLAG_MODEL
	if (item->skin && item->skin[0]) {
		CG_RegisterSkin( item->skin, &itemInfo->skin, qfalse );
	}
#endif

	if ( item->giType == IT_WEAPON ) {
#ifdef TA_WEAPSYS
		CG_RegisterWeaponGroup( item->giTag );
#else
		CG_RegisterWeapon( item->giTag );
#endif
	}
#ifdef TURTLEARENA // HOLDABLE
	if ( item->giType == IT_HOLDABLE ) {
		CG_RegisterHoldable( item->giTag );
	}
	// ZTM: Cache shurikens
	if (item->giType == IT_PERSISTANT_POWERUP
		&& item->giTag == PW_AMMOREGEN)
	{
		CG_RegisterItemVisuals(BG_ItemNumForHoldableNum(HI_SHURIKEN));
		CG_RegisterItemVisuals(BG_ItemNumForHoldableNum(HI_ELECTRICSHURIKEN));
		CG_RegisterItemVisuals(BG_ItemNumForHoldableNum(HI_FIRESHURIKEN));
		CG_RegisterItemVisuals(BG_ItemNumForHoldableNum(HI_LASERSHURIKEN));
	}
#endif

#ifdef TA_DATA // FLAG_MODEL
	if ( item->giType == IT_TEAM )
	{
		if ( item->world_model[1] ) {
			itemInfo->models[1] = trap_R_RegisterModel( item->world_model[1] );
		}
	}
#endif
#ifndef TA_DATA
	//
	// powerups have an accompanying ring or sphere
	//
	if ( item->giType == IT_POWERUP || item->giType == IT_HEALTH ) {
		if ( item->world_model[1] ) {
			itemInfo->models[1] = trap_R_RegisterModel( item->world_model[1] );
		}
	}
#endif
}


/*
========================================================================================

VIEW WEAPON

========================================================================================
*/

/*
=================
CG_MapTorsoToWeaponFrame

=================
*/
static int CG_MapTorsoToWeaponFrame( playerInfo_t *pi, int frame ) {

#ifdef TA_PLAYERSYS
	// change weapon
	if ( frame >= pi->playercfg.animations[TORSO_DROP].firstFrame
		&& frame < pi->playercfg.animations[TORSO_DROP].firstFrame + 9 ) {
		return frame - pi->playercfg.animations[TORSO_DROP].firstFrame + 6;
	}

	// stand attack
	if ( frame >= pi->playercfg.animations[TORSO_ATTACK].firstFrame
		&& frame < pi->playercfg.animations[TORSO_ATTACK].firstFrame + 6 ) {
		return 1 + frame - pi->playercfg.animations[TORSO_ATTACK].firstFrame;
	}

	// stand attack 2
	if ( frame >= pi->playercfg.animations[TORSO_ATTACK2].firstFrame
		&& frame < pi->playercfg.animations[TORSO_ATTACK2].firstFrame + 6 ) {
		return 1 + frame - pi->playercfg.animations[TORSO_ATTACK2].firstFrame;
	}
#else
	// change weapon
	if ( frame >= pi->animations[TORSO_DROP].firstFrame 
		&& frame < pi->animations[TORSO_DROP].firstFrame + 9 ) {
		return frame - pi->animations[TORSO_DROP].firstFrame + 6;
	}

	// stand attack
	if ( frame >= pi->animations[TORSO_ATTACK].firstFrame 
		&& frame < pi->animations[TORSO_ATTACK].firstFrame + 6 ) {
		return 1 + frame - pi->animations[TORSO_ATTACK].firstFrame;
	}

	// stand attack 2
	if ( frame >= pi->animations[TORSO_ATTACK2].firstFrame 
		&& frame < pi->animations[TORSO_ATTACK2].firstFrame + 6 ) {
		return 1 + frame - pi->animations[TORSO_ATTACK2].firstFrame;
	}
#endif
	
	return 0;
}


/*
==============
CG_CalculateWeaponPosition
==============
*/
static void CG_CalculateWeaponPosition( vec3_t origin, vec3_t angles ) {
	float	scale;
	int		delta;
	float	fracsin;

	VectorCopy( cg.refdef.vieworg, origin );
	VectorCopy( cg.refdefViewAngles, angles );

	// on odd legs, invert some angles
	if ( cg.bobcycle & 1 ) {
		scale = -cg.xyspeed;
	} else {
		scale = cg.xyspeed;
	}

	// gun angles from bobbing
	angles[ROLL] += scale * cg.bobfracsin * 0.005;
	angles[YAW] += scale * cg.bobfracsin * 0.01;
	angles[PITCH] += cg.xyspeed * cg.bobfracsin * 0.005;

	// drop the weapon when landing
	delta = cg.time - cg.cur_lc->landTime;
	if ( delta < LAND_DEFLECT_TIME ) {
		origin[2] += cg.cur_lc->landChange*0.25 * delta / LAND_DEFLECT_TIME;
	} else if ( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME ) {
		origin[2] += cg.cur_lc->landChange*0.25 * 
			(LAND_DEFLECT_TIME + LAND_RETURN_TIME - delta) / LAND_RETURN_TIME;
	}

#if 0
	// drop the weapon when stair climbing
	delta = cg.time - cg.stepTime;
	if ( delta < STEP_TIME/2 ) {
		origin[2] -= cg.stepChange*0.25 * delta / (STEP_TIME/2);
	} else if ( delta < STEP_TIME ) {
		origin[2] -= cg.stepChange*0.25 * (STEP_TIME - delta) / (STEP_TIME/2);
	}
#endif

	// idle drift
	scale = cg.xyspeed + 40;
	fracsin = sin( cg.time * 0.001 );
	angles[ROLL] += scale * fracsin * 0.01;
	angles[YAW] += scale * fracsin * 0.01;
	angles[PITCH] += scale * fracsin * 0.01;
}


/*
===============
CG_LightningBolt

Origin will be the exact tag point, which is slightly
different than the muzzle point used for determining hits.
The cent should be the non-predicted cent if it is from the player,
so the endpoint will reflect the simulated strike (lagging the predicted
angle)
===============
*/
static void CG_LightningBolt( centity_t *cent, vec3_t origin
#ifdef TA_WEAPSYS
	, int projnum
#endif
	)
{
	trace_t  trace;
	refEntity_t  beam;
	vec3_t   forward;
	vec3_t   muzzlePoint, endPoint;
	int      anim;
#ifdef TA_WEAPSYS
	int      range;

	if (projnum < 0 || projnum >= BG_NumProjectiles())
	{
		return;
	}

	range = bg_projectileinfo[projnum].speed;
#endif

#ifndef TA_WEAPSYS
	if (cent->currentState.weapon != WP_LIGHTNING) {
		return;
	}
#endif

	memset( &beam, 0, sizeof( beam ) );

	// use our view angles if full lag compensation is enabled
	if ( cent->currentState.number == cg.cur_lc->predictedPlayerState.playerNum && cg_antiLag.integer == 2 ) {
		// always shoot straight forward from our current position
		AngleVectors( cg.cur_lc->predictedPlayerState.viewangles, forward, NULL, NULL );
		VectorCopy( cg.cur_lc->predictedPlayerState.origin, muzzlePoint );
	}
	// CPMA  "true" lightning
	else if ((cent->currentState.number == cg.cur_lc->predictedPlayerState.playerNum) && (cg_trueLightning.value != 0)) {
		vec3_t angle;
		int i;

		for (i = 0; i < 3; i++) {
			float a = cent->lerpAngles[i] - cg.refdefViewAngles[i];
			if (a > 180) {
				a -= 360;
			}
			if (a < -180) {
				a += 360;
			}

			angle[i] = cg.refdefViewAngles[i] + a * (1.0 - cg_trueLightning.value);
			if (angle[i] < 0) {
				angle[i] += 360;
			}
			if (angle[i] > 360) {
				angle[i] -= 360;
			}
		}

		AngleVectors(angle, forward, NULL, NULL );
		VectorCopy(cent->lerpOrigin, muzzlePoint );
//		VectorCopy(cg.refdef.vieworg, muzzlePoint );
	} else {
		// !CPMA
		AngleVectors( cent->lerpAngles, forward, NULL, NULL );
		VectorCopy(cent->lerpOrigin, muzzlePoint );
	}

	anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
	if ( anim == LEGS_WALKCR || anim == LEGS_IDLECR ) {
		muzzlePoint[2] += CROUCH_VIEWHEIGHT;
	} else {
		muzzlePoint[2] += DEFAULT_VIEWHEIGHT;
	}

	VectorMA( muzzlePoint, 14, forward, muzzlePoint );

	// project forward by the lightning range
#ifdef TA_WEAPSYS
	VectorMA( muzzlePoint, range, forward, endPoint );
#else
	VectorMA( muzzlePoint, LIGHTNING_RANGE, forward, endPoint );
#endif

	// see if it hit a wall
	CG_Trace( &trace, muzzlePoint, vec3_origin, vec3_origin, endPoint, 
		cent->currentState.number, MASK_SHOT );

	// this is the endpoint
	VectorCopy( trace.endpos, beam.oldorigin );

	// use the provided origin, even though it may be slightly
	// different than the muzzle origin
	VectorCopy( origin, beam.origin );

	beam.reType = RT_LIGHTNING;
#ifdef TA_WEAPSYS
	beam.customShader = cg_projectiles[projnum].trailShader[0];
	beam.radius = cg_projectiles[projnum].trailRadius;
#else
	beam.customShader = cgs.media.lightningShader;
#endif
	CG_AddRefEntityWithMinLight( &beam );

	// add the impact flare if it hit something
	if ( trace.fraction < 1.0 ) {
#ifdef TA_WEAPSYS // PT_LIGHTNING
		vec3_t	dir;
		vec3_t origin;

		VectorSubtract( beam.oldorigin, beam.origin, dir );
		VectorNormalize( dir );

		VectorMA( trace.endpos, -16, dir, origin );

		// Lightning has to create the explosion each frame
		CG_MissileExplode(projnum, cent->currentState.number, origin, dir, IMPACTSOUND_LIGHTNING_PREDICT);
#else
		vec3_t	angles;
		vec3_t	dir;

		VectorSubtract( beam.oldorigin, beam.origin, dir );
		VectorNormalize( dir );

		memset( &beam, 0, sizeof( beam ) );
		beam.hModel = cgs.media.lightningExplosionModel;

		VectorMA( trace.endpos, -16, dir, beam.origin );

		// make a random orientation
		angles[0] = rand() % 360;
		angles[1] = rand() % 360;
		angles[2] = rand() % 360;
		AnglesToAxis( angles, beam.axis );
		CG_AddRefEntityWithMinLight( &beam );
#endif
	}
}
/*

static void CG_LightningBolt( centity_t *cent, vec3_t origin ) {
	trace_t		trace;
	refEntity_t		beam;
	vec3_t			forward;
	vec3_t			muzzlePoint, endPoint;

	if ( cent->currentState.weapon != WP_LIGHTNING ) {
		return;
	}

	memset( &beam, 0, sizeof( beam ) );

	// find muzzle point for this frame
	VectorCopy( cent->lerpOrigin, muzzlePoint );
	AngleVectors( cent->lerpAngles, forward, NULL, NULL );

	// FIXME: crouch
	muzzlePoint[2] += DEFAULT_VIEWHEIGHT;

	VectorMA( muzzlePoint, 14, forward, muzzlePoint );

	// project forward by the lightning range
	VectorMA( muzzlePoint, LIGHTNING_RANGE, forward, endPoint );

	// see if it hit a wall
	CG_Trace( &trace, muzzlePoint, vec3_origin, vec3_origin, endPoint, 
		cent->currentState.number, MASK_SHOT );

	// this is the endpoint
	VectorCopy( trace.endpos, beam.oldorigin );

	// use the provided origin, even though it may be slightly
	// different than the muzzle origin
	VectorCopy( origin, beam.origin );

	beam.reType = RT_LIGHTNING;
	beam.customShader = cgs.media.lightningShader;
	CG_AddRefEntityWithMinLight( &beam );

	// add the impact flare if it hit something
	if ( trace.fraction < 1.0 ) {
		vec3_t	angles;
		vec3_t	dir;

		VectorSubtract( beam.oldorigin, beam.origin, dir );
		VectorNormalize( dir );

		memset( &beam, 0, sizeof( beam ) );
		beam.hModel = cgs.media.lightningExplosionModel;

		VectorMA( trace.endpos, -16, dir, beam.origin );

		// make a random orientation
		angles[0] = rand() % 360;
		angles[1] = rand() % 360;
		angles[2] = rand() % 360;
		AnglesToAxis( angles, beam.axis );
		CG_AddRefEntityWithMinLight( &beam );
	}
}
*/


/*
======================
CG_MachinegunSpinAngle
======================
*/
#define		SPIN_SPEED	0.9
#define		COAST_TIME	1000
static float	CG_MachinegunSpinAngle( centity_t *cent ) {
	int		delta;
	float	angle;
	float	speed;

	delta = cg.time - cent->pe.barrelTime;
	if ( cent->pe.barrelSpinning ) {
		angle = cent->pe.barrelAngle + delta * SPIN_SPEED;
	} else {
		if ( delta > COAST_TIME ) {
			delta = COAST_TIME;
		}

		speed = 0.5 * ( SPIN_SPEED + (float)( COAST_TIME - delta ) / COAST_TIME );
		angle = cent->pe.barrelAngle + delta * speed;

#ifdef TA_WEAPSYS
		if ((cg.time - cent->pe.barrelTime) <= COAST_TIME)
		{
			int i;

			// Update yawAngles so if the barrel uses gravity it won't jump around
			for (i = 0; i < MAX_HANDS; i++)
			{
				cent->pe.barrel[i].yawAngle = angle;
				cent->pe.barrel[i].yawing = qtrue;
			}
		}
#endif
	}

	if ( cent->pe.barrelSpinning == !(cent->currentState.eFlags & EF_FIRING) ) {
		cent->pe.barrelTime = cg.time;
		cent->pe.barrelAngle = AngleMod( angle );
		cent->pe.barrelSpinning = !!(cent->currentState.eFlags & EF_FIRING);
#ifdef TA_WEAPSYS
		// Stopped firing, play sound
		if ( !cent->pe.barrelSpinning ) {
			trap_S_StartSound( NULL, cent->currentState.number, CHAN_WEAPON, cg_weapongroups[cent->currentState.weapon].firingStoppedSound );
		}
#else
#ifdef MISSIONPACK
		if ( cent->currentState.weapon == WP_CHAINGUN && !cent->pe.barrelSpinning ) {
			trap_S_StartSound( NULL, cent->currentState.number, CHAN_WEAPON, cgs.media.sfx_chgstop );
		}
#endif
#endif
	}

	return angle;
}

#ifdef TA_WEAPSYS
/*
========================
CG_BarrelGravityAngle
========================
*/
float CG_BarrelGravityAngle(centity_t *cent, refEntity_t *gun, lerpFrame_t *lerp)
{
	//qboolean updateangles;
	float ang, d;
	vec3_t angles;
	vec3_t dir;

	//updateangles = qtrue;
	
	//if ( !updateangles ) {
	//	return;
	//}

	VectorCopy( cent->currentState.pos.trDelta, dir );
	// add gravity
	dir[2] += 100;
	VectorNormalize( dir );
	d = DotProduct(gun->axis[2], dir);
	// if there is anough movement orthogonal to the flag pole
	if (fabs(d) < 0.9) {
		//
		d = DotProduct(gun->axis[0], dir);
		if (d > 1.0f) {
			d = 1.0f;
		}
		else if (d < -1.0f) {
			d = -1.0f;
		}
		ang = Q_acos(d);

		d = DotProduct(gun->axis[1], dir);
		if (d < 0) {
			angles[YAW] = 360 - ang * 180 / M_PI;
		}
		else {
			angles[YAW] = ang * 180 / M_PI;
		}
		if (angles[YAW] < 0)
			angles[YAW] += 360;
		if (angles[YAW] > 360)
			angles[YAW] -= 360;

		// change the yaw angle
#ifdef IOQ3ZTM // BG_SWING_ANGLES
		BG_SwingAngles( angles[YAW], 40, 90, BG_SWINGSPEED, &lerp->yawAngle, &lerp->yawing, cg.frametime );
#else
		CG_SwingAngles( angles[YAW], 40, 90, 0.3f, &lerp->yawAngle, &lerp->yawing );
#endif
	}

	return lerp->yawAngle;
}
#endif

/*
========================
CG_AddWeaponWithPowerups
========================
*/
static void CG_AddWeaponWithPowerups( refEntity_t *gun, entityState_t *state ) {
	// add powerup effects
#ifdef TURTLEARENA // POWERS
	if ( state->powerups & ( 1 << PW_FLASHING ) ) {
		if (state->otherEntityNum2 > 0) {
			// Death fade out
			gun->renderfx |= RF_FORCE_ENT_ALPHA;
			gun->shaderRGBA[3] = state->otherEntityNum2;
		}
		CG_AddRefEntityWithMinLight( gun );

		gun->customShader = cgs.media.playerTeleportShader;
		CG_AddRefEntityWithMinLight( gun );
	} else
#endif
	if ( state->powerups & ( 1 << PW_INVIS ) ) {
		gun->customShader = cgs.media.invisShader;
		CG_AddRefEntityWithMinLight( gun );
	} else {
		CG_AddRefEntityWithMinLight( gun );

#ifndef TURTLEARENA // POWERS
		if ( state->powerups & ( 1 << PW_BATTLESUIT ) ) {
			gun->customShader = cgs.media.battleWeaponShader;
			CG_AddRefEntityWithMinLight( gun );
		}
#endif
		if ( state->powerups & ( 1 << PW_QUAD ) ) {
			gun->customShader = cgs.media.quadWeaponShader;
			CG_AddRefEntityWithMinLight( gun );
		}
	}
}



#ifdef IOQ3ZTM // GHOST
static void VectorInterpolate( vec3_t a, vec3_t b, float lerp, vec3_t out ) {
	out[0] = b[0] + (a[0] - b[0]) * lerp;
	out[1] = b[1] + (a[1] - b[1]) * lerp;
	out[2] = b[2] + (a[2] - b[2]) * lerp;
}

static void AxisInterpolate( vec3_t *a, vec3_t *b, float lerp, vec3_t *out ) {
	VectorInterpolate(a[0], b[0], lerp, out[0]);
	VectorInterpolate(a[1], b[1], lerp, out[1]);
	VectorInterpolate(a[2], b[2], lerp, out[2]);
}

/*
=============
CG_GhostRefEntity
=============
*/
void CG_GhostRefEntity(refEntity_t *re, ghostRefData_t *refs, int num, int *ghostTime)
{
	const float		GHOST_TIME = 200.0f;
	int				i;
	ghostRefData_t	*refData = &refs[num];
	vec3_t			dir;
	float			dist;
	float			frac;

	if (refData->time < cg.time) {

		if (*ghostTime >= cg.time)
			return;

		// Check if too close to another ghost?
		for (i = 0; i < NUM_GHOST_REFS; i++) {
			if (refs[i].time >= cg.time) {
				VectorSubtract(re->origin, refs[i].origin, dir);
				dist = VectorLength(dir);
				if (dist < 5.0f) {
					// too close, don't add.
					break;
				}
			}
		}

		if (i < NUM_GHOST_REFS) {
			*ghostTime = cg.time + 50;
			return;
		}

		*ghostTime = cg.time + 100;

		refData->time = cg.time + GHOST_TIME;

		for (i = 0; i < 3; i++)
			VectorCopy(re->axis[i], refData->axis[i]);

		refData->nonNormalizedAxes = re->nonNormalizedAxes;
		refData->frame = re->frame;

		// previous data for frame interpolation
		refData->oldframe = re->oldframe;
		refData->backlerp = re->backlerp;

		refData->shaderTime = re->shaderTime;
		VectorCopy(re->origin, refData->origin);

		frac = ((refData->time - cg.time) / GHOST_TIME);
	} else {
		frac = ((refData->time - cg.time) / GHOST_TIME);

		AxisInterpolate(refData->axis, re->axis, frac, re->axis);

		re->nonNormalizedAxes = refData->nonNormalizedAxes;
		re->frame = refData->frame;

		// previous data for frame interpolation
		re->oldframe = refData->oldframe;
		re->backlerp = refData->backlerp;

		re->shaderTime = refData->shaderTime;

		VectorInterpolate(refData->origin, re->origin, frac, re->origin);
		VectorCopy(re->origin, re->oldorigin);
	}

	re->shaderRGBA[3] = 0x80 * frac;
	re->renderfx |= RF_FORCE_ENT_ALPHA | RF_NOSHADOW;
}
#endif

#ifdef TA_WEAPSYS // MELEE_TRAIL
//
// CG_AddWeaponTrailOld
//
/* Idea
Draw a shader from bottom of weapon attack to top of weapon attack,
     scale length using speed weapon is moving?
Allow the shader to be per-weapon
*/
void CG_AddWeaponTrailOld(centity_t *cent, refEntity_t *gun, int weaponHand)
{
	refEntity_t trail;
	vec3_t		angles, dir;
	float		angle, d;
	float *yawAngle;
	qboolean *yawing;
	int playerNum;
	playerInfo_t *pi;
	float scale;
	int anim;

	static vec3_t weaponMoveDir[MAX_CLIENTS][MAX_HANDS];
	static vec3_t trailOldOrigin[MAX_CLIENTS][MAX_HANDS];
	static int weaponMoveDirInit = 0;
	if (!weaponMoveDirInit)
	{
		weaponMoveDirInit = 1;
		memset(&weaponMoveDir, 0, sizeof (weaponMoveDir));
		memset(&trailOldOrigin, 0, sizeof (trailOldOrigin));
	}

	if (!cg_drawMeleeWeaponTrails.integer)
		return;

	// Only melee weapons have trails.
	if (!BG_WeaponHasMelee(cent->currentState.weapon))
		return;

	anim = ( cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT );

	if (!BG_PlayerAttackAnim(anim))
		return;

	yawAngle = &cent->pe.weaponTrails[weaponHand].yawAngle;
	yawing = &cent->pe.weaponTrails[weaponHand].yawing;

	playerNum = cent->currentState.playerNum;
	if ( playerNum >= 0 && playerNum < MAX_CLIENTS ) {
		pi = &cgs.playerinfo[ playerNum ];

		// Scale up and back down
		scale = (float)(cent->pe.torso.frame-pi->playercfg.animations[anim].firstFrame) / pi->playercfg.animations[anim].numFrames;

		if (scale > 0.5f)
			scale = 1.0f - scale;

		scale *= 2;

		// Set move dir
		if (!VectorLength(weaponMoveDir[playerNum][weaponHand]))
		{
			VectorCopy( cent->currentState.pos.trDelta, weaponMoveDir[playerNum][weaponHand]);
		}

		VectorCopy( weaponMoveDir[playerNum][weaponHand], dir );
	}
	else
	{
		pi = NULL;
		scale = 1.0f;
		VectorCopy( cent->currentState.pos.trDelta, dir );
		// add gravity
		//dir[2] += 100;
		// ZTM: Up not down.
		dir[2] -= 100;
	}

	//
	// Use code from CTF flag so it can turn and stuff
	//

	// show the trail model
	memset( &trail, 0, sizeof(trail) );
	trail.hModel = cgs.media.flagFlapModel;
	trail.customShader = cgs.media.weaponTrailShader;
	VectorCopy( gun->lightingOrigin, trail.lightingOrigin );
	trail.shadowPlane = gun->shadowPlane;
	trail.renderfx = gun->renderfx;

	VectorClear(angles);

	//if ( updateangles )
	{
		VectorNormalize( dir );
		d = DotProduct(gun->axis[2], dir);
		// if there is anough movement orthogonal to the flag pole
		if (fabs(d) < 0.9) {
			//
			d = DotProduct(gun->axis[0], dir);
			if (d > 1.0f) {
				d = 1.0f;
			}
			else if (d < -1.0f) {
				d = -1.0f;
			}
			angle = Q_acos(d);

			d = DotProduct(gun->axis[1], dir);
			if (d < 0) {
				angles[YAW] = 360 - angle * 180 / M_PI;
			}
			else {
				angles[YAW] = angle * 180 / M_PI;
			}
			if (angles[YAW] < 0)
				angles[YAW] += 360;
			if (angles[YAW] > 360)
				angles[YAW] -= 360;

			//vectoangles( cent->currentState.pos.trDelta, tmpangles );
			//angles[YAW] = tmpangles[YAW] + 45 - cent->pe.torso.yawAngle;
			// change the yaw angle
#ifdef IOQ3ZTM // BG_SWING_ANGLES
			BG_SwingAngles( angles[YAW], 25, 90, 0.15f, yawAngle, yawing, cg.frametime );
#else
			CG_SwingAngles( angles[YAW], 25, 90, 0.15f, yawAngle, yawing );
#endif
		}

		/*
		d = DotProduct(pole.axis[2], dir);
		angle = Q_acos(d);

		d = DotProduct(pole.axis[1], dir);
		if (d < 0) {
			angle = 360 - angle * 180 / M_PI;
		}
		else {
			angle = angle * 180 / M_PI;
		}
		if (angle > 340 && angle < 20) {
			flagAnim = FLAG_RUNUP;
		}
		if (angle > 160 && angle < 200) {
			flagAnim = FLAG_RUNDOWN;
		}
		*/
	}

	// set the yaw angle
	angles[YAW] = *yawAngle;
	trail.oldframe = 0;
	trail.frame = 0;
	trail.backlerp = 0;

	AnglesToAxis( angles, trail.axis );
	VectorScale(trail.axis[0], scale, trail.axis[0]);
	CG_PositionRotatedEntityOnTag( &trail, gun, gun->hModel, "tag_weapon" );

	trap_R_AddRefEntityToScene( &trail );

	if ( playerNum >= 0 && playerNum < MAX_CLIENTS )
	{
		VectorSubtract(trail.origin, trailOldOrigin[playerNum][weaponHand], weaponMoveDir[playerNum][weaponHand]);
		VectorCopy(trail.origin, trailOldOrigin[playerNum][weaponHand]);
	}
}

/*
=============
CG_AddWeaponTrail

Add tranparent copies of the model when melee attacking
=============
*/
void CG_AddWeaponTrail(centity_t *cent, refEntity_t *gun, int weaponHand, qboolean barrel)
{
	int				anim;
	playerInfo_t	*pi;
	refEntity_t		ref;
	int				i;

	if (cent->currentState.powerups & ( 1 << PW_INVIS ))
		return;

	// Old type
	if (cg_drawMeleeWeaponTrails.integer == 2)
	{
		if (barrel)
			return;

		CG_AddWeaponTrailOld(cent, gun, weaponHand);
		return;
	}

	if (!cg_drawMeleeWeaponTrails.integer)
		return;

	// Only melee weapons have trails.
	if (!BG_WeaponHasMelee(cent->currentState.weapon))
		return;

	anim = ( cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT );

	pi = &cgs.playerinfo[ cent->currentState.playerNum ];

	if (!BG_PlayerAttackAnim(anim)) {
		if (pi->ghostTime[weaponHand] == 0)
			return;

		pi->ghostTime[weaponHand] = 0;

		// Clear ghosts
		for (i = 0; i < NUM_GHOST_REFS; i++) {
			pi->ghostWeapon[weaponHand][i].time = 0;
		}
		return;
	}

	for (i = 0; i < NUM_GHOST_REFS; i++) {
		ref = *gun;
		CG_GhostRefEntity(&ref, pi->ghostWeapon[weaponHand], i, &pi->ghostTime[weaponHand]);
		CG_AddWeaponWithPowerups(&ref, &cent->currentState);
	}
}
#endif

#ifdef TA_WEAPSYS
/*
=============
CG_AddPlayerDefaultWeapon

Add the default weapon(s) on the player when not in use.

Draw default weapon(s) at "tag_wp_away_primary" and "tag_wp_away_secondary"
	when not in use.
=============
*/
void CG_AddPlayerDefaultWeapon( refEntity_t *parent, centity_t *cent, int team) {
	weapon_t			weaponGroupNum;
	bg_weapongroupinfo_t *weaponGroup;
	qboolean			drawWeapon[MAX_HANDS];
	refEntity_t			gun[MAX_HANDS];
	refEntity_t			barrel;
	vec3_t				angles;
	int					i;
	playerInfo_t		*pi;
	qboolean			foundModel;
	char				*awayTagNames[MAX_HANDS] = { "tag_wp_away_primary", "tag_wp_away_secondary" };

	pi = &cgs.playerinfo[ cent->currentState.playerNum ];

	weaponGroupNum = cgs.playerinfo[cent->currentState.playerNum].playercfg.default_weapon;
	weaponGroup = &bg_weapongroupinfo[weaponGroupNum];

	CG_RegisterWeaponGroup(weaponGroupNum);

	for (i = 0; i < MAX_HANDS; i++) {
		drawWeapon[i] = (weaponGroup->weaponnum[i] > 0);
	}

	// get hands from cent, holding default weaponGroup
	if (cent->currentState.weapon == weaponGroupNum) {
		// If player is holding the weapon, don't draw it "away"
		for (i = 0; i < MAX_HANDS; i++) {
			if (cent->currentState.weaponHands & HAND_TO_HB(i)) {
				drawWeapon[i] = qfalse;
			}
		}
	}

	// Check if we have some where to put it.
	if (drawWeapon[HAND_PRIMARY] && drawWeapon[HAND_SECONDARY]
		&& !(pi->tagInfo & TI_TAG_WP_AWAY_PRIMARY) && !(pi->tagInfo & TI_TAG_WP_AWAY_SECONDARY))
	{
		return;
	}
	if (drawWeapon[HAND_PRIMARY] && !drawWeapon[HAND_SECONDARY]
		&& !(pi->tagInfo & TI_TAG_WP_AWAY_PRIMARY))
	{
		return;
	}
	if (!drawWeapon[HAND_PRIMARY] && drawWeapon[HAND_SECONDARY]
		&& !(pi->tagInfo & TI_TAG_WP_AWAY_SECONDARY))
	{
		return;
	}

	// add the weapons
	memset( &gun, 0, sizeof( gun ) );
	foundModel = qfalse;

	for (i = 0; i < MAX_HANDS; i++)
	{
		VectorCopy( parent->lightingOrigin, gun[i].lightingOrigin );
		gun[i].shadowPlane = parent->shadowPlane;
		gun[i].renderfx = parent->renderfx;
		gun[i].hModel = cg_weapons[weaponGroup->weaponnum[i]].weaponModel;

		if (gun[i].hModel) {
			foundModel = qtrue;
		}
	}

	if (!foundModel) {
		return;
	}

	for (i = 0; i < MAX_HANDS; i++)
	{
		if (awayTagNames[i] == NULL) {
			continue;
		}

		if (drawWeapon[i] && gun[i].hModel) {
			drawWeapon[i] = CG_PositionEntityOnTag( &gun[i], parent, parent->hModel, awayTagNames[i]);
		}

		if (!drawWeapon[i]) {
			continue;
		}

		CG_AddWeaponWithPowerups( &gun[i], &cent->currentState );

		// add the barrel for the weapon
		if (!cg_weapons[weaponGroup->weaponnum[i]].barrelModel) {
			continue;
		}

		memset( &barrel, 0, sizeof( barrel ) );
		VectorCopy( parent->lightingOrigin, barrel.lightingOrigin );
		barrel.shadowPlane = parent->shadowPlane;
		barrel.renderfx = parent->renderfx;

		barrel.hModel = cg_weapons[weaponGroup->weaponnum[i]].barrelModel;
		angles[YAW] = 0;
		angles[PITCH] = 0;
		angles[ROLL] = 0; // Don't spin Gauntlet when not using it...
		AnglesToAxis( angles, barrel.axis );

		CG_PositionRotatedEntityOnTag( &barrel, &gun[i],
			cg_weapons[weaponGroup->weaponnum[i]].weaponModel,
			"tag_barrel" );

		CG_AddWeaponWithPowerups( &barrel, &cent->currentState );
	}
}
#endif

/*
=============
CG_AddPlayerWeapon

Used for both the view weapon (ps is valid) and the world modelother character models (ps is NULL)
The main player will have this called for BOTH cases, so effects like light and
sound should only be done on the world model case.
=============
*/
void CG_AddPlayerWeapon( refEntity_t *parent, playerState_t *ps, centity_t *cent, int team) {
#ifndef TA_WEAPSYS
	refEntity_t	gun;
#endif
	refEntity_t	barrel;
	refEntity_t	flash;
	int flashDLight;
	vec3_t		angles;
	weapon_t	weaponNum;
#ifndef TA_WEAPSYS
	weaponInfo_t	*weapon;
#endif
	centity_t	*nonPredictedCent;
#ifdef TA_WEAPSYS
	float				barrelSpinAngle;
	bg_weapongroupinfo_t *weaponGroup;
	refEntity_t			gun[MAX_HANDS];
	qboolean			drawWeapon[MAX_HANDS];
	int					i;
	playerInfo_t		*pi;
	qboolean			foundModel;
	vec3_t				flashColor;

#ifdef TURTLEARENA // PLAYERS
	char *newTagNames[3] = { "tag_hand_primary", "tag_hand_secondary", NULL };
	int newTagInfo[3] = {TI_TAG_HAND_PRIMARY, TI_TAG_HAND_SECONDARY, 0};
#endif
	char *originalTagNames[3] = { "tag_weapon", "tag_flag", NULL };
	int originalTagInfo[3] = {TI_TAG_WEAPON, TI_TAG_FLAG, 0};
#else
	orientation_t	lerped;
	playerInfo_t *pi;
#endif
	int		anim;

	weaponNum = cent->currentState.weapon;

#ifdef TA_WEAPSYS
	CG_RegisterWeaponGroup( weaponNum );

	weaponGroup = &bg_weapongroupinfo[weaponNum];
#else
	CG_RegisterWeapon( weaponNum );
	weapon = &cg_weapons[weaponNum];
#endif

#ifdef TA_WEAPSYS
	// If world model, add default weapon
	if (ps == NULL) {
		CG_AddPlayerDefaultWeapon(parent, cent, team);
	}

	if (cent->currentState.weapon <= 0 || cent->currentState.weapon >= BG_NumWeaponGroups()) {
		return;
	}
#endif

	pi = &cgs.playerinfo[cent->currentState.playerNum];

	// add the weapon
	memset( &gun, 0, sizeof( gun ) );

#ifndef TA_WEAPSYS
	VectorCopy( parent->lightingOrigin, gun.lightingOrigin );
	gun.shadowPlane = parent->shadowPlane;
	gun.renderfx = parent->renderfx;

	// set custom shading for railgun refire rate
	if( weaponNum == WP_RAILGUN && cent->pe.railFireTime + 1500 > cg.time ) {
		int scale = 255 * ( cg.time - cent->pe.railFireTime ) / 1500;
		gun.shaderRGBA[0] = ( pi->c1RGBA[0] * scale ) >> 8;
		gun.shaderRGBA[1] = ( pi->c1RGBA[1] * scale ) >> 8;
		gun.shaderRGBA[2] = ( pi->c1RGBA[2] * scale ) >> 8;
		gun.shaderRGBA[3] = 255;
	}
	else {
		Byte4Copy( pi->c1RGBA, gun.shaderRGBA );
	}
#endif

	// make sure we aren't looking at cg.cur_lc->predictedPlayerEntity for LG
	if ( cent == &cg.cur_lc->predictedPlayerEntity ) {
		nonPredictedCent = &cg_entities[cent->currentState.playerNum];
	} else {
		nonPredictedCent = cent;
	}

#ifdef TA_WEAPSYS
	foundModel = qfalse;

	// Copy the primary hand weapon to the secondary hand weapon.
	for (i = 0; i < MAX_HANDS; i++)
	{
		VectorCopy( parent->lightingOrigin, gun[i].lightingOrigin );
		gun[i].shadowPlane = parent->shadowPlane;
		gun[i].renderfx = parent->renderfx;

		// set custom shading for railgun refire rate
		if (weaponGroup->weapon[i]->weapontype == WT_GUN)
		{
			if (weaponGroup->weapon[i]->attackDelay > 0 &&
				cent->muzzleFlashTime + weaponGroup->weapon[i]->attackDelay > cg.time)
			{
				int f = 255 * (cg.time - cent->muzzleFlashTime) / weaponGroup->weapon[i]->attackDelay;

				// color2 is secondary color (primary is trail)
				gun[i].shaderRGBA[0] = ( pi->c2RGBA[0] * f ) >> 8;
				gun[i].shaderRGBA[1] = ( pi->c2RGBA[1] * f ) >> 8;
				gun[i].shaderRGBA[2] = ( pi->c2RGBA[2] * f ) >> 8;
				gun[i].shaderRGBA[3] = 255;
			} else {
				Byte4Copy( pi->c2RGBA, gun[i].shaderRGBA );
			}
		} else {
#if 0
			int animTime = BG_AnimationTime(&ci->playercfg.animations[cent->currentState.torsoAnim]);

			if (animTime > 0 && cent->muzzleFlashTime + animTime > cg.time)
			{
				int f = 255 * (cg.time - cent->muzzleFlashTime) / animTime;

				if ((weaponGroup->weapon[i]->flags & WIF_ALWAYS_DAMAGE)) {
					f = 255 * Com_Clamp(0.5f, 1.0f, 0.5f + sin( ( cg.time / 1000 ) ) / 2 );
				}

				// color1 is primary
				gun[i].shaderRGBA[0] = ( pi->c1RGBA[0] * f ) >> 8;
				gun[i].shaderRGBA[1] = ( pi->c1RGBA[1] * f ) >> 8;
				gun[i].shaderRGBA[2] = ( pi->c1RGBA[2] * f ) >> 8;
				gun[i].shaderRGBA[3] = 255;
			} else
#endif
			{
				Byte4Copy( pi->c1RGBA, gun[i].shaderRGBA );
			}
		}

		gun[i].hModel = cg_weapons[weaponGroup->weaponnum[i]].weaponModel;

		if (gun[i].hModel) {
			foundModel = qtrue;
		}

		// set default flashOrigin
		if ( !gun[i].hModel || !cg_weapons[weaponGroup->weaponnum[i]].flashModel ) {
			// use default flash origin when no flash model
			if ( ps ) {
				VectorCopy( cg.firstPersonViewOrg, cg.cur_lc->flashOrigin[i] );
				VectorMA( cg.cur_lc->flashOrigin[i], -8, cg.firstPersonViewAxis[2], cg.cur_lc->flashOrigin[i] );

				VectorCopy( cg.cur_lc->flashOrigin[i], flash.origin );
			} else {
				VectorCopy( cent->lerpOrigin, nonPredictedCent->pe.flashOrigin[i] );

				anim = nonPredictedCent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
				if ( anim == LEGS_WALKCR || anim == LEGS_IDLECR ) {
					nonPredictedCent->pe.flashOrigin[i][2] += CROUCH_VIEWHEIGHT - 6;
				} else {
					nonPredictedCent->pe.flashOrigin[i][2] += DEFAULT_VIEWHEIGHT - 6;
				}

				VectorCopy( nonPredictedCent->pe.flashOrigin[i], flash.origin );
			}

			if ( ( cg.cur_lc->predictedPlayerState.eFlags & EF_FIRING )
				&& ( ps || cg.cur_lc->renderingThirdPerson
						|| cent->currentState.number != cg.cur_lc->predictedPlayerState.playerNum ) ) {
				if (weaponGroup->weapon[i]->proj->trailType == PT_LIGHTNING
					&& weaponGroup->weapon[i]->proj->instantDamage) {
					// add lightning bolt
					CG_LightningBolt( nonPredictedCent, flash.origin, bg_weapongroupinfo[cent->currentState.weapon].weapon[i]->projnum );
				}
			}
		}
	}

	if (!foundModel) {
		return;
	}
#else
	if ( !weapon->weaponModel || !weapon->flashModel ) {
		// use default flash origin when no flash model
		if ( ps ) {
			VectorCopy( cg.firstPersonViewOrg, cg.cur_lc->flashOrigin );
			VectorMA( cg.cur_lc->flashOrigin, -8, cg.firstPersonViewAxis[2], cg.cur_lc->flashOrigin );

			VectorCopy( cg.cur_lc->flashOrigin, flash.origin );
		} else {
			VectorCopy( cent->lerpOrigin, nonPredictedCent->pe.flashOrigin );

			anim = nonPredictedCent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
			if ( anim == LEGS_WALKCR || anim == LEGS_IDLECR ) {
				nonPredictedCent->pe.flashOrigin[2] += CROUCH_VIEWHEIGHT - 6;
			} else {
				nonPredictedCent->pe.flashOrigin[2] += DEFAULT_VIEWHEIGHT - 6;
			}

			VectorCopy( nonPredictedCent->pe.flashOrigin, flash.origin );
		}

		if ( ( nonPredictedCent->currentState.eFlags & EF_FIRING )
			&& ( ps || cg.cur_lc->renderingThirdPerson
					|| cent->currentState.number != cg.cur_lc->predictedPlayerState.playerNum ) ) {
			// special hack for lightning gun...
			CG_LightningBolt( nonPredictedCent, flash.origin );
		}
	}

	gun.hModel = weapon->weaponModel;
	if (!gun.hModel) {
		return;
	}
#endif

	if ( !ps ) {
		// add weapon ready sound
		cent->pe.lightningFiring = qfalse;
		if ( ( cent->currentState.eFlags & EF_FIRING )
#ifndef IOQ3ZTM // IOQ3BUGFIX: Do not play ready sound while firing!
			&& weapon->firingSound
#endif
			) {
#ifdef TA_WEAPSYS
			if (cg_weapongroups[weaponNum].firingSound)
#elif defined IOQ3ZTM // IOQ3BUGFIX: Do not play ready sound while firing!
			if (weapon->firingSound)
#endif
			// lightning gun and guantlet make a different sound when fire is held down
			trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin,
#ifdef TA_WEAPSYS
					cg_weapongroups[weaponNum].firingSound
#else
					weapon->firingSound
#endif
					);
			cent->pe.lightningFiring = qtrue;
		} else if (
#ifdef TA_WEAPSYS
			cg_weapongroups[weaponNum].readySound
#else
			weapon->readySound
#endif
			) {
			trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin,
#ifdef TA_WEAPSYS
					cg_weapongroups[weaponNum].readySound
#else
					weapon->readySound
#endif
					);
		}
	}

#ifdef TA_WEAPSYS
	// get hands from cent
	for (i = 0; i < MAX_HANDS; i++)
	{
		drawWeapon[i] = ((cent->currentState.weaponHands & HAND_TO_HB(i)) && gun[i].hModel);

		if (!originalTagNames[i]
#ifdef TURTLEARENA // PLAYERS
			|| !newTagNames[i]
#endif
			)
		{
			break;
		}

		if (drawWeapon[i])
		{
			// Use ci->tagInfo to speed up tests
			if (
#ifdef TURTLEARENA // PLAYERS
				(!(pi->tagInfo & newTagInfo[i]) || !CG_PositionEntityOnTag(&gun[i], parent, parent->hModel, newTagNames[i])) &&
#endif
				(!(pi->tagInfo & originalTagInfo[i]) || !CG_PositionEntityOnTag(&gun[i], parent, parent->hModel, originalTagNames[i])))
			{
				// Failed to find tag
				continue;
			}

			CG_AddWeaponWithPowerups( &gun[i], &cent->currentState );

			// MELEE_TRAIL
			CG_AddWeaponTrail(cent, &gun[i], HAND_PRIMARY, qfalse);
		}
	}

	// Even if no barrel, check for firingStoppedSound
	barrelSpinAngle = CG_MachinegunSpinAngle( cent );
#else
	trap_R_LerpTag(&lerped, parent->hModel, parent->oldframe, parent->frame,
		1.0 - parent->backlerp, "tag_weapon");
	VectorCopy(parent->origin, gun.origin);

	VectorMA(gun.origin, lerped.origin[0], parent->axis[0], gun.origin);

	// Make weapon appear left-handed for 2 and centered for 3
	if(ps && cg_drawGun[cg.cur_localPlayerNum].integer == 2)
		VectorMA(gun.origin, -lerped.origin[1], parent->axis[1], gun.origin);
	else if(!ps || cg_drawGun[cg.cur_localPlayerNum].integer != 3)
	       	VectorMA(gun.origin, lerped.origin[1], parent->axis[1], gun.origin);

	VectorMA(gun.origin, lerped.origin[2], parent->axis[2], gun.origin);

	MatrixMultiply(lerped.axis, ((refEntity_t *)parent)->axis, gun.axis);
	gun.backlerp = parent->backlerp;

	CG_AddWeaponWithPowerups( &gun, &cent->currentState );
#endif

	// add the spinning barrel
#ifdef TA_WEAPSYS
	for (i = 0; i < MAX_HANDS; i++)
#else
	if ( weapon->barrelModel )
#endif
	{
#ifdef TA_WEAPSYS
		if (!drawWeapon[i] || !cg_weapons[weaponGroup->weaponnum[i]].barrelModel)
			continue;
#endif

		memset( &barrel, 0, sizeof( barrel ) );
		VectorCopy( parent->lightingOrigin, barrel.lightingOrigin );
		barrel.shadowPlane = parent->shadowPlane;
		barrel.renderfx = parent->renderfx;

#ifdef TA_WEAPSYS
		barrel.hModel = cg_weapons[weaponGroup->weaponnum[i]].barrelModel;

		// ZTM: TESTME: In the UI there are checks for gauntlet and BFG
		//                        to spin pitch, but not here?
		// CHECKED: It should always be BS_ROLL in Q3, the `tag_barrel's are
		//		setup that way in the models.
		// So the code in the UI isn't getting run, ui ALWAYS spins ROLL
		//		(even with the checks).
		VectorClear(angles);
		if (weaponGroup->weapon[i]->barrelSpin != BS_NONE)
		{
			if ((weaponGroup->weapon[i]->flags & WIF_BARREL_IDLE_USE_GRAVITY) && !cent->pe.barrelSpinning
				&& (cg.time - cent->pe.barrelTime) > COAST_TIME)
			{
				angles[weaponGroup->weapon[i]->barrelSpin] = CG_BarrelGravityAngle(cent, &gun[i], &cent->pe.barrel[i]);
			}
			else if (i & 1) // Spin other direction
				angles[weaponGroup->weapon[i]->barrelSpin] = 360-barrelSpinAngle;
			else
				angles[weaponGroup->weapon[i]->barrelSpin] = barrelSpinAngle;
		}
#else
		barrel.hModel = weapon->barrelModel;

		angles[YAW] = 0;
		angles[PITCH] = 0;
		angles[ROLL] = CG_MachinegunSpinAngle( cent );
#endif
		AnglesToAxis( angles, barrel.axis );

#ifdef TA_WEAPSYS
		CG_PositionRotatedEntityOnTag( &barrel, &gun[i], gun[i].hModel, "tag_barrel" );
#else
		CG_PositionRotatedEntityOnTag( &barrel, &gun, weapon->weaponModel, "tag_barrel" );
#endif

		CG_AddWeaponWithPowerups( &barrel, &cent->currentState );

#ifdef TA_WEAPSYS // MELEE_TRAIL
		CG_AddWeaponTrail(cent, &barrel, i, qtrue);
#endif
	}

	// add the flash
#ifdef TA_WEAPSYS
	for (i = 0; i < MAX_HANDS; i++)
#endif
	{
		memset( &flash, 0, sizeof( flash ) );
		VectorCopy( parent->lightingOrigin, flash.lightingOrigin );
		flash.shadowPlane = parent->shadowPlane;
		flash.renderfx = parent->renderfx;

#ifdef TA_WEAPSYS
		flash.hModel = cg_weapons[weaponGroup->weaponnum[i]].flashModel;
#else
		flash.hModel = weapon->flashModel;
#endif
		if (!flash.hModel) {
#ifdef TA_WEAPSYS
			continue;
#else
			return;
#endif
		}
		angles[YAW] = 0;
		angles[PITCH] = 0;
		angles[ROLL] = crandom() * 10;
		AnglesToAxis( angles, flash.axis );

#ifdef TA_WEAPSYS
		// color1
		if (cg_weapons[weaponGroup->weaponnum[i]].flashDlightColor[0] == (float)'c'
			&& cg_weapons[weaponGroup->weaponnum[i]].flashDlightColor[1] == 1.0f
			&& cg_weapons[weaponGroup->weaponnum[i]].flashDlightColor[2] == 0.0f)
		{
			VectorCopy(pi->color1, flashColor);
		}
		//color2
		else if (cg_weapons[weaponGroup->weaponnum[i]].flashDlightColor[0] == (float)'c'
			&& cg_weapons[weaponGroup->weaponnum[i]].flashDlightColor[1] == 2.0f
			&& cg_weapons[weaponGroup->weaponnum[i]].flashDlightColor[2] == 0.0f)
		{
			VectorCopy(pi->color2, flashColor);
		}
		else
		{
			VectorCopy(cg_weapons[weaponGroup->weaponnum[i]].flashDlightColor, flashColor);
		}

		// colorize the flash, using color of the flash dlight!
		flash.shaderRGBA[0] = 255 * flashColor[0];
		flash.shaderRGBA[1] = 255 * flashColor[1];
		flash.shaderRGBA[2] = 255 * flashColor[2];
#else
		// colorize the railgun blast
		Byte4Copy( pi->c1RGBA, flash.shaderRGBA );
#endif

#ifdef TA_WEAPSYS
		CG_PositionRotatedEntityOnTag( &flash, &gun[i], gun[i].hModel, "tag_flash");
#else
		CG_PositionRotatedEntityOnTag( &flash, &gun, weapon->weaponModel, "tag_flash");
#endif

		// update muzzle origin
#ifdef TA_WEAPSYS
		if ( ps ) {
			VectorCopy( flash.origin, cg.cur_lc->flashOrigin[i] );
		} else {
			VectorCopy( flash.origin, nonPredictedCent->pe.flashOrigin[i] );
		}
#else
		if ( ps ) {
			VectorCopy( flash.origin, cg.cur_lc->flashOrigin );
		} else {
			VectorCopy( flash.origin, nonPredictedCent->pe.flashOrigin );
		}
#endif

		// add the flash
		if (
#ifdef TA_WEAPSYS
			( weaponGroup->weapon[i]->flags & WIF_CONTINUOUS_FLASH )
#else
			( weaponNum == WP_LIGHTNING || weaponNum == WP_GAUNTLET || weaponNum == WP_GRAPPLING_HOOK )
#endif
			&& ( nonPredictedCent->currentState.eFlags & EF_FIRING ) )
		{
			// continuous flash
		} else {
			// impulse flash
			if ( cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME ) {
				return;
			}
		}

		CG_AddRefEntityWithMinLight( &flash );

		if ( ps || cg.cur_lc->renderingThirdPerson ||
			cent->currentState.number != cg.cur_lc->predictedPlayerState.playerNum ) {
#ifdef TA_WEAPSYS
			if (weaponGroup->weapon[i]->proj->trailType == PT_LIGHTNING
				&& weaponGroup->weapon[i]->proj->instantDamage)
#endif
			{
				// add lightning bolt
#ifdef TA_WEAPSYS
				CG_LightningBolt( nonPredictedCent, flash.origin, bg_weapongroupinfo[cent->currentState.weapon].weapon[i]->projnum );
#else
				CG_LightningBolt( nonPredictedCent, flash.origin );
#endif
			}

#ifdef IOQ3ZTM // SMOOTH_FLASH
			flashDLight = 300 + (cg.time&31);
#else
			flashDLight = 300 + (rand()&31);
#endif
#ifdef TA_WEAPSYS
			if ( flashColor[0] || flashColor[1] || flashColor[2] ) {
				trap_R_AddLightToScene( flash.origin, flashDLight, 1.0f,
					flashColor[0], flashColor[1], flashColor[2], 0 );
			}
#else
			if ( weapon->flashDlightColor[0] || weapon->flashDlightColor[1] || weapon->flashDlightColor[2] ) {
				trap_R_AddLightToScene( flash.origin, flashDLight, 1.0f, weapon->flashDlightColor[0],
					weapon->flashDlightColor[1], weapon->flashDlightColor[2], 0 );
			}
#endif
		}
	}
}

/*
==============
CG_AddViewWeapon

Add the weapon, and flash for the player's view
==============
*/
void CG_AddViewWeapon( playerState_t *ps ) {
	refEntity_t	hand;
	centity_t	*cent;
	playerInfo_t	*pi;
	vec3_t		fovOffset;
	vec3_t		angles;
#ifdef TA_WEAPSYS
	weaponGroupInfo_t	*weapon;
#else
	weaponInfo_t	*weapon;
#endif

	if ( ps->persistant[PERS_TEAM] == TEAM_SPECTATOR ) {
		return;
	}

	if ( ps->pm_type == PM_INTERMISSION ) {
		return;
	}

	// no gun if in third person view or a camera is active
	//if ( cg.cur_lc->renderingThirdPerson || cg.cur_lc->cameraMode) {
	if ( cg.cur_lc->renderingThirdPerson ) {
		return;
	}


	// allow the gun to be completely removed
	if ( !cg_drawGun[cg.cur_localPlayerNum].integer ) {
#ifdef TA_WEAPSYS
		int i;
		vec3_t origin;

		cent = &cg_entities[ps->playerNum];

		// use default flash origin
		for (i = 0; i < MAX_HANDS; i++)
		{
			VectorCopy( cg.firstPersonViewOrg, origin );

			if ( cgs.playerinfo[cent->currentState.playerNum].playercfg.handSide[i] == HS_RIGHT )
				VectorMA( origin, -8, cg.firstPersonViewAxis[2], origin );
			else if ( cgs.playerinfo[cent->currentState.playerNum].playercfg.handSide[i] == HS_LEFT )
				VectorMA( origin, 8, cg.firstPersonViewAxis[2], origin );

			VectorCopy( origin, cent->pe.flashOrigin[i] );
		}
#else
		// use default flash origin
		VectorCopy( cg.firstPersonViewOrg, cg.cur_lc->flashOrigin );
		VectorMA( cg.cur_lc->flashOrigin, -8, cg.firstPersonViewAxis[2], cg.cur_lc->flashOrigin );
#endif

		if ( cg.cur_lc->predictedPlayerState.eFlags & EF_FIRING ) {
			// special hack for lightning gun...
#ifdef TA_WEAPSYS
			int i;

			for (i = 0; i < MAX_HANDS; i++)
			{
				// Check if holding weapon
				if (!(cent->currentState.weaponHands & HAND_TO_HB(i))) {
					continue;
				}

				// Add instant lightning
				if (bg_weapongroupinfo[cent->currentState.weapon].weapon[i]->proj->trailType == PT_LIGHTNING
					&& bg_weapongroupinfo[cent->currentState.weapon].weapon[i]->proj->instantDamage)
				{
					CG_LightningBolt( cent, cent->pe.flashOrigin[i], bg_weapongroupinfo[cent->currentState.weapon].weapon[i]->projnum);
				}
			}
#else
			CG_LightningBolt( &cg_entities[ps->playerNum], cg.cur_lc->flashOrigin );
#endif
		}
		return;
	}

	// don't draw if testing a gun model
	if ( cg.testGun ) {
		return;
	}

	VectorClear(fovOffset);

	if ( cg.viewWeaponFov > 90 ) {
		// drop gun lower at higher fov
		fovOffset[2] = -0.2 * ( cg.viewWeaponFov - 90 ) * cg.refdef.weapon_fov_x / cg.viewWeaponFov;
	} else if ( cg.viewWeaponFov < 90 ) {
		// move gun forward at lowerer fov
		fovOffset[0] = -0.2 * ( cg.viewWeaponFov - 90 ) * cg.refdef.weapon_fov_x / cg.viewWeaponFov;
	}

	cent = &cg.cur_lc->predictedPlayerEntity;	// &cg_entities[cg.snap->ps.playerNum];
#ifdef TA_WEAPSYS
	CG_RegisterWeaponGroup( ps->weapon );
	weapon = &cg_weapongroups[ ps->weapon ];
#else
	CG_RegisterWeapon( ps->weapon );
	weapon = &cg_weapons[ ps->weapon ];
#endif

	memset (&hand, 0, sizeof(hand));

	// set up gun position
	CG_CalculateWeaponPosition( hand.origin, angles );

#ifdef TA_MISC // USE_REAL_TAGS_FOR_VIEW
	if (!weapon->handsModel) {
		fovOffset[0] += 5;
		fovOffset[2] -= 20;
	}
#endif

	VectorMA( hand.origin, (cg_gun_x.value+fovOffset[0]), cg.refdef.viewaxis[0], hand.origin );
	VectorMA( hand.origin, (cg_gun_y.value+fovOffset[1]), cg.refdef.viewaxis[1], hand.origin );
	VectorMA( hand.origin, (cg_gun_z.value+fovOffset[2]), cg.refdef.viewaxis[2], hand.origin );

	AnglesToAxis( angles, hand.axis );

#ifdef TA_MISC // USE_REAL_TAGS_FOR_VIEW
	// get clientinfo
	pi = &cgs.playerinfo[ cent->currentState.playerNum ];
#endif

	// ZTM: FIXME: This doesn't support single model or custom skeleton!

	// map torso animations to weapon animations
	if ( cg_gun_frame.integer ) {
		// development tool
		hand.frame = hand.oldframe = cg_gun_frame.integer;
		hand.backlerp = 0;
	} else {
#ifdef TA_MISC // USE_REAL_TAGS_FOR_VIEW
		if (!weapon->handsModel) {
			hand.frame = cent->pe.torso.frame;
			hand.oldframe = cent->pe.torso.oldFrame;
		} else {
#else
		// get clientinfo for animation map
		pi = &cgs.playerinfo[ cent->currentState.playerNum ];
#endif
		hand.frame = CG_MapTorsoToWeaponFrame( pi, cent->pe.torso.frame );
		hand.oldframe = CG_MapTorsoToWeaponFrame( pi, cent->pe.torso.oldFrame );
#ifdef TA_MISC // USE_REAL_TAGS_FOR_VIEW
		}
#endif
		hand.backlerp = cent->pe.torso.backlerp;
	}

	hand.hModel = weapon->handsModel;
#ifdef TA_MISC // USE_REAL_TAGS_FOR_VIEW
	if (!hand.hModel) {
		hand.hModel = pi->torsoModel;
	}
#endif
	hand.renderfx = RF_DEPTHHACK | RF_NO_MIRROR;

	// add everything onto the hand
	CG_AddPlayerWeapon( &hand, ps, &cg.cur_lc->predictedPlayerEntity, ps->persistant[PERS_TEAM] );
}

#ifndef TA_WEAPSYS_EX
/*
==============================================================================

WEAPON SELECTION

==============================================================================
*/

/*
===================
CG_DrawWeaponSelect
===================
*/
void CG_DrawWeaponSelect( void ) {
	int		i;
	int		bits;
	int		count;
	int		x, y;
	char	*name;
	float	*color;

	CG_SetScreenPlacement(PLACE_CENTER, PLACE_BOTTOM);

	// don't display if dead
	if ( cg.cur_lc->predictedPlayerState.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	color = CG_FadeColor( cg.cur_lc->weaponSelectTime, WEAPON_SELECT_TIME );
	if ( !color ) {
		return;
	}
	trap_R_SetColor( color );

	// showing weapon select clears pickup item display, but not the blend blob
	cg.cur_lc->itemPickupTime = 0;

	// count the number of weapons owned
	bits = cg.cur_ps->stats[ STAT_WEAPONS ];
	count = 0;
	for ( i = 1 ; i < MAX_WEAPONS ; i++ ) {
		if ( bits & ( 1 << i ) ) {
			count++;
		}
	}

	x = 320 - count * 20;
	y = 380;

	for ( i = 1 ; i < MAX_WEAPONS ; i++ ) {
		if ( !( bits & ( 1 << i ) ) ) {
			continue;
		}

#ifdef TA_WEAPSYS
		CG_RegisterWeaponGroup( i );
#else
		CG_RegisterWeapon( i );
#endif

		// draw weapon icon
#ifdef TA_WEAPSYS
		CG_DrawPic( x, y, 32, 32, cg_weapongroups[i].weaponIcon );
#else
		CG_DrawPic( x, y, 32, 32, cg_weapons[i].weaponIcon );
#endif

		// draw selection marker
		if ( i == cg.cur_lc->weaponSelect ) {
			CG_DrawPic( x-4, y-4, 40, 40, cgs.media.selectShader );
		}

		// no ammo cross on top
		if ( !cg.cur_ps->ammo[ i ] ) {
			CG_DrawPic( x, y, 32, 32, cgs.media.noammoShader );
		}

		x += 40;
	}

	// draw the selected name
#ifdef TA_WEAPSYS
		name = bg_weapongroupinfo[ cg.cur_lc->weaponSelect ].item->pickup_name;
#else
	if ( cg_weapons[ cg.cur_lc->weaponSelect ].item ) {
		name = cg_weapons[ cg.cur_lc->weaponSelect ].item->pickup_name;
#endif
		if ( name ) {
			CG_DrawString( SCREEN_WIDTH / 2, y - 6, name, UI_CENTER|UI_VA_BOTTOM|UI_DROPSHADOW|UI_BIGFONT, color );
		}
#ifndef TA_WEAPSYS
	}
#endif

	trap_R_SetColor( NULL );
}


/*
===============
CG_WeaponSelectable
===============
*/
static qboolean CG_WeaponSelectable( playerState_t *ps, int i ) {
	if ( !ps->ammo[i] ) {
		return qfalse;
	}
	if ( ! (ps->stats[ STAT_WEAPONS ] & ( 1 << i ) ) ) {
		return qfalse;
	}

	return qtrue;
}

/*
===============
CG_NextWeapon_f
===============
*/
void CG_NextWeapon_f( int localPlayerNum ) {
	int		i;
	int		original;
	playerState_t	*ps;
	localPlayer_t	*player;

	if ( cg.localPlayers[localPlayerNum].playerNum == -1 ) {
		return;
	}

	ps = &cg.snap->pss[localPlayerNum];
	player = &cg.localPlayers[localPlayerNum];

	if ( ps->pm_flags & PMF_FOLLOW ) {
		return;
	}

	player->weaponSelectTime = cg.time;
	original = player->weaponSelect;

	for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
		player->weaponSelect++;
		if ( player->weaponSelect == MAX_WEAPONS ) {
			player->weaponSelect = 0;
		}
#ifndef TURTLEARENA // WEAPONS
#ifdef TA_WEAPSYS
		if ( player->weaponSelect == ps->stats[STAT_DEFAULTWEAPON] && cg_cyclePastGauntlet[localPlayerNum].integer ) {
			continue;		// never cycle to gauntlet
		}
#else
		if ( player->weaponSelect == WP_GAUNTLET && cg_cyclePastGauntlet[localPlayerNum].integer ) {
			continue;		// never cycle to gauntlet
		}
#endif
#endif
		if ( CG_WeaponSelectable( ps, player->weaponSelect ) ) {
			break;
		}
	}
	if ( i == MAX_WEAPONS ) {
		player->weaponSelect = original;
	}
}

/*
===============
CG_PrevWeapon_f
===============
*/
void CG_PrevWeapon_f( int localPlayerNum ) {
	int		i;
	int		original;
	playerState_t	*ps;
	localPlayer_t	*player;

	if ( cg.localPlayers[localPlayerNum].playerNum == -1 ) {
		return;
	}

	ps = &cg.snap->pss[localPlayerNum];
	player = &cg.localPlayers[localPlayerNum];

	if ( ps->pm_flags & PMF_FOLLOW ) {
		return;
	}

	player->weaponSelectTime = cg.time;
	original = player->weaponSelect;

	for ( i = 0 ; i < MAX_WEAPONS ; i++ ) {
		player->weaponSelect--;
		if ( player->weaponSelect == -1 ) {
			player->weaponSelect = MAX_WEAPONS - 1;
		}
#ifndef TURTLEARENA // WEAPONS
#ifdef TA_WEAPSYS
		if ( player->weaponSelect == ps->stats[STAT_DEFAULTWEAPON] && cg_cyclePastGauntlet[localPlayerNum].integer ) {
			continue;		// never cycle to gauntlet
		}
#else
		if ( player->weaponSelect == WP_GAUNTLET && cg_cyclePastGauntlet[localPlayerNum].integer ) {
			continue;		// never cycle to gauntlet
		}
#endif
#endif
		if ( CG_WeaponSelectable( ps, player->weaponSelect ) ) {
			break;
		}
	}
	if ( i == MAX_WEAPONS ) {
		player->weaponSelect = original;
	}
}

/*
===============
CG_Weapon_f
===============
*/
void CG_Weapon_f( int localPlayerNum ) {
	int		num;
	playerState_t	*ps;
	localPlayer_t	*player;

	if ( cg.localPlayers[localPlayerNum].playerNum == -1 ) {
		return;
	}

	ps = &cg.snap->pss[localPlayerNum];
	player = &cg.localPlayers[localPlayerNum];

	if ( ps->pm_flags & PMF_FOLLOW ) {
		return;
	}

	num = atoi( CG_Argv( 1 ) );

#ifdef TA_WEAPSYS
	// Special case for default
	if (num == WP_DEFAULT) {
		num = ps->stats[STAT_DEFAULTWEAPON];
	}
#endif

	if ( num < 1 || num > MAX_WEAPONS-1 ) {
		return;
	}

	player->weaponSelectTime = cg.time;

	if ( ! ( ps->stats[STAT_WEAPONS] & ( 1 << num ) ) ) {
		return;		// don't have the weapon
	}

	player->weaponSelect = num;
}

/*
===============
CG_WeaponToggle_f

select weapon and store old weapon or if already selecting the weapon
switch back to old weapon
===============
*/
void CG_WeaponToggle_f( int localPlayerNum ) {
	int		num;
	int		weapon, oldweapon;
	playerState_t	*ps;
	localPlayer_t	*player;

	if ( cg.localPlayers[localPlayerNum].playerNum == -1 ) {
		return;
	}

	ps = &cg.snap->pss[localPlayerNum];
	player = &cg.localPlayers[localPlayerNum];

	if ( ps->pm_flags & PMF_FOLLOW ) {
		return;
	}

	num = atoi( CG_Argv( 1 ) );

	if ( num < 1 || num > MAX_WEAPONS-1 ) {
		return;
	}

	player->weaponSelectTime = cg.time;

	if ( player->weaponSelect != num ) {
		weapon = num;
		oldweapon = player->weaponSelect;
	} else {
		weapon = player->weaponToggledFrom;
		oldweapon = WP_NONE;
	}

	if ( ! ( ps->stats[STAT_WEAPONS] & ( 1 << weapon ) ) ) {
		return;		// don't have the weapon
	}

	player->weaponSelect = weapon;
	player->weaponToggledFrom = oldweapon;
}

/*
===================
CG_OutOfAmmoChange

The current weapon has just run out of ammo
===================
*/
void CG_OutOfAmmoChange( int localPlayerNum ) {
	localPlayer_t	*player;
	playerState_t	*ps;
	int				i;

	if ( cg.localPlayers[localPlayerNum].playerNum == -1 ) {
		return;
	}

	player = &cg.localPlayers[localPlayerNum];
	ps = &cg.snap->pss[localPlayerNum];

	player->weaponSelectTime = cg.time;

	for ( i = MAX_WEAPONS-1 ; i > 0 ; i-- ) {
		if ( CG_WeaponSelectable( ps, i ) ) {
			player->weaponSelect = i;
			break;
		}
	}
}
#endif // TA_WEAPSYS_EX



/*
===================================================================================================

WEAPON EVENTS

===================================================================================================
*/

#ifdef TA_WEAPSYS
/*
================
CG_WeaponUseEffect

Weapons can eject brass/smoke
================
*/
void CG_WeaponUseEffect( centity_t *cent, int handSide, int weaponNum )
{
	bg_weaponinfo_t *weap;
	int		contents;

	if (weaponNum < 0 || weaponNum >= BG_NumWeapons())
	{
		return;
	}

	weap = &bg_weaponinfo[weaponNum];

	if (weap->flags & WIF_EJECT_BRASS)
	{
		CG_MachineGunEjectBrass( cent, handSide );
	}
	if (weap->flags & WIF_EJECT_BRASS2)
	{
		CG_ShotgunEjectBrass( cent, handSide );
	}

	contents = CG_PointContents( cent->currentState.pos.trBase, 0 );

	if ( !( contents & CONTENTS_WATER ) )
	{
		if (weap->flags & WIF_EJECT_SMOKE)
		{
			CG_NailgunEjectBrass( cent, handSide );
		}

		// Shotgun smoke (From Q3's CG_ShotgunFire)
		if (weap->flags & WIF_EJECT_SMOKE2)
		{
			vec3_t			up;
			vec3_t			v;

			VectorSet( up, 0, 0, 8 );
			CG_SmokePuff( v, up, 32, 1, 1, 1, 0.33f, 900, cg.time, 0, LEF_PUFF_DONT_SCALE, cgs.media.shotgunSmokePuffShader );
		}
	}

	if (cg_weapons[weaponNum].ejectBrassFunc)
	{
		cg_weapons[weaponNum].ejectBrassFunc( cent );
	}
}
#endif

/*
================
CG_FireWeapon

Caused by an EV_FIRE_WEAPON event
================
*/
void CG_FireWeapon( centity_t *cent ) {
	entityState_t *ent;
	int				c;
#ifdef TA_WEAPSYS
	int				hand;
#ifndef TURTLEARENA // POWERS
	qboolean		firstValid = qtrue;
#endif
#else
	weaponInfo_t	*weap;
#endif

	ent = &cent->currentState;
	if ( ent->weapon == WP_NONE ) {
		return;
	}
#ifdef TA_WEAPSYS
	if ( ent->weapon >= BG_NumWeaponGroups() )
#else
	if ( ent->weapon >= WP_NUM_WEAPONS )
#endif
	{
#ifdef TA_WEAPSYS
		CG_Error( "CG_FireWeapon: ent->weapon >= BG_NumWeaponGroups()" );
#else
		CG_Error( "CG_FireWeapon: ent->weapon >= WP_NUM_WEAPONS" );
#endif
		return;
	}
#ifndef TA_WEAPSYS
	weap = &cg_weapons[ ent->weapon ];
#endif

	// mark the entity as muzzle flashing, so when it is added it will
	// append the flash to the weapon model
	cent->muzzleFlashTime = cg.time;

#ifndef TA_WEAPSYS
	// lightning gun only does this this on initial press
	if ( ent->weapon == WP_LIGHTNING ) {
		if ( cent->pe.lightningFiring ) {
			return;
		}
	}

	if( ent->weapon == WP_RAILGUN ) {
		cent->pe.railFireTime = cg.time;
	}
#endif

#ifdef TA_WEAPSYS
	for (hand = 0; hand < MAX_HANDS; hand++)
	{
		if ((bg_weapongroupinfo[ent->weapon].weapon[hand]->flags & WIF_INITIAL_EFFECT_ONLY)
			&& cent->pe.lightningFiring)
		{
			continue;
		}
#endif
#ifndef TURTLEARENA // POWERS
	// play quad sound if needed
	if ( cent->currentState.powerups & ( 1 << PW_QUAD ) ) {
#ifdef TA_WEAPSYS
		if (firstValid)
#endif
		trap_S_StartSound (NULL, cent->currentState.number, CHAN_ITEM, cgs.media.quadSound );
#ifdef TA_WEAPSYS
		firstValid = qfalse;
#endif
	}
#endif

	// play a sound
	for ( c = 0 ; c < 4 ; c++ ) {
#ifdef TA_WEAPSYS
		if ( !cg_weapons[bg_weapongroupinfo[ent->weapon].weaponnum[hand]].flashSound[c] )
#else
		if ( !weap->flashSound[c] )
#endif
		{
			break;
		}
	}
	if ( c > 0 ) {
		c = rand() % c;
#ifdef TA_WEAPSYS
		if ( cg_weapons[bg_weapongroupinfo[ent->weapon].weaponnum[hand]].flashSound[c] )
#else
		if ( weap->flashSound[c] )
#endif
		{
			trap_S_StartSound( NULL, ent->number, CHAN_WEAPON,
#ifdef TA_WEAPSYS
					cg_weapons[bg_weapongroupinfo[ent->weapon].weaponnum[hand]].flashSound[c]
#else
					weap->flashSound[c]
#endif
					);
		}
	}

	// do brass ejection
#ifdef TA_WEAPSYS
		CG_WeaponUseEffect(cent, cgs.playerinfo[cent->currentState.playerNum].playercfg.handSide[hand], bg_weapongroupinfo[ent->weapon].weaponnum[hand]);
	}
#else
	if ( weap->ejectBrassFunc && cg_brassTime.integer > 0 ) {
		weap->ejectBrassFunc( cent );
	}
#endif
}

#ifdef TA_WEAPSYS
/*
=================
CG_PlayerHitEffect

Spawn hit marker, based on CG_Bleed
=================
*/
void CG_PlayerHitEffect( vec3_t origin, int entityNum, qboolean meleeDamage ) {
	localEntity_t	*ex;
	qhandle_t		hShader;
	int r;

#ifndef NOBLOOD
#ifndef TURTLEARENA // WEAPONS
	if ( !cg_blood.integer ) {
		return;
	}
#endif
#endif

	r = rand()%2;

#ifndef NOBLOOD
	if (cg_blood.integer)
	{
		hShader = cgs.media.bloodExplosionShader;
	}
	else
#endif
	{
#ifdef TURTLEARENA // WEAPONS
		if (meleeDamage)
		{
			if (r >= 1 && cgs.media.meleeHitShader[1])
				hShader = cgs.media.meleeHitShader[1];
			else
				hShader = cgs.media.meleeHitShader[0];
		}
		else
		{
			// missile damage
			if (r >= 1 && cgs.media.missileHitShader[1])
				hShader = cgs.media.missileHitShader[1];
			else
				hShader = cgs.media.missileHitShader[0];
		}
#else
		// quake3 has no non-blood hit effect.
		hShader = 0;
#endif
	}

	if (!hShader) {
		return;
	}

	ex = CG_AllocLocalEntity();
	ex->leType = LE_EXPLOSION;

	ex->startTime = cg.time;
	ex->endTime = ex->startTime + 500;

	VectorCopy ( origin, ex->refEntity.origin);
	ex->refEntity.reType = RT_SPRITE;
	ex->refEntity.rotation = rand() % 360;
	ex->radius = 18 + (rand() & 6);
	ex->refEntity.radius = -(ex->radius - 6); // scale down explosion over time. size = ex->radius + frac * ex->refEntity.radius

	ex->refEntity.customShader = hShader;

	// don't show player's own blood in view
	ex->defaultViewFlags = 0;
	ex->playerEffects[0].playerNum = entityNum;
	ex->playerEffects[0].viewFlags = LEVF_FIRST_PERSON_MIRROR;
	ex->numPlayerEffects = 1;
}
#endif

#ifdef TA_MISC // MATERIALS
/*
==================
CG_LaunchModel
==================
*/
localEntity_t *CG_LaunchModel( vec3_t origin, vec3_t velocity, qhandle_t hModel, qhandle_t hShader, float maxSize ) {
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t angles;
	float frac;

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + 6000 + random() * 6000;

	VectorCopy( origin, re->origin );

	// random angles
	angles[0] = random() * 360;
	angles[1] = random() * 360;
	angles[2] = random() * 360;
	AnglesToAxis( angles, re->axis );

	// random size
	if (maxSize < 2.0f) {
		maxSize = 2.0f;
	}
	frac = (float)(random() * maxSize);
	if (frac < 1.0f) {
		frac = 1.0f;
	}
	VectorScale( re->axis[0], frac, re->axis[0] );
	VectorScale( re->axis[1], frac, re->axis[1] );
	VectorScale( re->axis[2], frac, re->axis[2] );
	re->nonNormalizedAxes = qtrue;

	re->hModel = hModel;
	re->customShader = hShader;

	le->pos.trType = TR_GRAVITY;
	VectorCopy( origin, le->pos.trBase );
	VectorCopy( velocity, le->pos.trDelta );
	le->pos.trTime = cg.time;

	le->bounceFactor = 0.3f; // 0.1f

	le->leBounceSoundType = LEBS_NONE;//LEBS_BRASS;
	le->leMarkType = LEMT_NONE;

	return le;
}

/*
=================
CG_ImpactParticles

Cool wall hit effects, note that the "particles" are models not sprites.
=================
*/
#define	EXP_VELOCITY	100
#define	EXP_JUMP		150
void CG_ImpactParticles( vec3_t origin, vec3_t dir, float radius, int surfaceFlags, int skipNum )
{
	int i, j;
	//int k;
	int numParticles;
	qhandle_t model;
	vec3_t velocity;
	vec3_t newOrigin;
	trace_t trace;
	qhandle_t hSurfShader;

	if (cg_impactDebris.integer <= 0) {
		return;
	}

	if (dir && VectorLength(dir))
	{
		// Move away from surface
		VectorMA( origin, 10, dir, origin );

		// Move into surface (- 10 [return to origin] - 20 [into surface])
		VectorMA( origin, -30, dir, newOrigin );

		CG_Trace( &trace, origin, NULL, NULL, newOrigin, skipNum, MASK_PLAYERSOLID );

		hSurfShader = trap_R_GetSurfaceShader( trace.surfaceNum, LIGHTMAP_NONE );

		if (surfaceFlags == -1) {
			surfaceFlags = trace.surfaceFlags;
		}
	}
	else
	{
		CG_Trace( &trace, origin, NULL, NULL, origin, skipNum, MASK_PLAYERSOLID );

		VectorMA( origin, 10, trace.plane.normal, origin );

		hSurfShader = trap_R_GetSurfaceShader( trace.surfaceNum, LIGHTMAP_NONE );

		if (surfaceFlags == -1) {
			surfaceFlags = trace.surfaceFlags;
		}
	}

	if (radius <= 10)
		numParticles = 3;
	else if (radius <= 18)
		numParticles = 6;
	else if (radius <= 26)
		numParticles = 12;
	else
		numParticles = 24;

	numParticles *= cg_impactDebris.value;

	for (i = 1; i < NUM_MATERIAL_TYPES; i++)
	{
		// Add spark particles
		if ((surfaceFlags & materialInfo[i].surfaceFlag)
			&& (materialInfo[i].surfaceFlag & SURF_SPARKS))
		{
			for (j = 0; j < numParticles; j++)
			{
				if (dir) {
					VectorScale( dir, EXP_JUMP+random()*EXP_VELOCITY, velocity );
					velocity[2] -= 20;
				}

				CG_ParticleSparks (origin, velocity, 1000, radius, radius, -300);
			}
		}

		if (cgs.media.matNumModels[i] > 0
			&& ((surfaceFlags & materialInfo[i].surfaceFlag)
			// default to stone debris if their is no set debris
			/*|| (surfaceFlags == 0 && materialInfo[i].surfaceFlag == SURF_STONE)*/))
		{
			// Impact sounds?
			// Smoke particles?

			for (j = 0; j < numParticles; j++)
			{
				VectorCopy(origin, newOrigin);

				newOrigin[0] += random()*(radius*1.2f)/2 - random()*(radius*1.2f)/2;
				newOrigin[1] += random()*(radius*1.2f)/2 - random()*(radius*1.2f)/2;
				newOrigin[2] += random()*(radius*1.2f)/2 - random()*(radius*1.2f)/2;

				velocity[0] = crandom()*EXP_VELOCITY;
				velocity[1] = crandom()*EXP_VELOCITY;
				velocity[2] = EXP_JUMP + crandom()*EXP_VELOCITY;

				//for (k = 0; k < 3; k++)
				//{
				//	velocity[k] *= dir[k];
				//}

				model = cgs.media.matModels[i][rand()%cgs.media.matNumModels[i]];
				CG_LaunchModel(newOrigin, velocity, model, materialInfo[i].surfaceShader ? hSurfShader : 0, radius/4);
			}
		}
	}
}
#endif

/*
=================
CG_MissileExplode

Formerly known as CG_MissileHitWall

Caused by an EV_MISSILE_MISS event, or directly by local bullet tracing
=================
*/
void CG_MissileExplode( int weapon, int playerNum, vec3_t origin, vec3_t dir, impactSound_t soundType ) {
	qhandle_t		mod;
	qhandle_t		mark;
	qhandle_t		shader;
	sfxHandle_t		sfx;
	float			radius;
	float			light;
	vec3_t			lightColor;
	localEntity_t	*le;
	int				r;
	qboolean		alphaFade;
	qboolean		isSprite;
	int				duration;
	vec3_t			sprOrg;
	vec3_t			sprVel;
#ifdef TA_WEAPSYS
	// SPR_EXP_SCALE
	int exp_base;
	int exp_add;
	qboolean instantLightningBeam;
	qboolean instantLightningImpact;
#endif

	mod = 0;
	shader = 0;
	light = 0;
	lightColor[0] = 1;
	lightColor[1] = 1;
	lightColor[2] = 0;

	// set defaults
	isSprite = qfalse;
	duration = 600;

#ifdef TA_WEAPSYS // SPR_EXP_SCALE
	// Sprite explosion scaling, starts at "exp_base" and add on "exp_add" using time scaling.
	//		so the bigest it will be is "exp_base"+"exp_add".
	exp_base = 30;
	exp_add = 42;
#endif

#ifdef TA_WEAPSYS
	sfx = 0;

	// Predicted instant lightning, explosion only (no wallmark or sounds)
	instantLightningBeam = (soundType == IMPACTSOUND_LIGHTNING_PREDICT);

	// Instant lightning impact, no explosion
	instantLightningImpact = (bg_projectileinfo[weapon].trailType == PT_LIGHTNING
		&& bg_projectileinfo[weapon].instantDamage) && !instantLightningBeam;

	switch (bg_projectileinfo[weapon].deathType)
	{
		default:
		case PD_NONE:
		case PD_NONE_EXP_PLAYER:
			break;
		case PD_PLASMA:
			mod = cgs.media.ringFlashModel;
			shader = cgs.media.plasmaExplosionShader;
			sfx = cgs.media.sfx_plasmaexp;
			break;
		case PD_ROCKET:
#ifdef TA_DATA // EXP_SCALE
			mod = cgs.media.smokeModel;
			shader = 0;
			isSprite = qfalse;
#else
			mod = cgs.media.dishFlashModel;
			shader = cgs.media.rocketExplosionShader;
			isSprite = qtrue;
#endif
			sfx = cgs.media.sfx_rockexp;
			light = 300;
			duration = 1000;
			lightColor[0] = 1;
			lightColor[1] = 0.75;
			lightColor[2] = 0.0;
			if (cg_oldRocket.integer == 0
				&& !instantLightningImpact) {
				// explosion sprite animation
				VectorMA( origin, 24, dir, sprOrg );
				VectorScale( dir, 64, sprVel );

				CG_ParticleExplosion( "explode1", sprOrg, sprVel, 1400, 20, 30 );
			}
			break;
		case PD_ROCKET_SMALL: // Smaller explosion
			VectorScale( dir, 0.5f, dir );

#ifdef TA_DATA // EXP_SCALE
			mod = cgs.media.smokeModel;
			shader = 0;
			isSprite = qfalse;
#else
			mod = cgs.media.dishFlashModel;
			shader = cgs.media.rocketExplosionShader;
			isSprite = qtrue;
			exp_base = 30 / 2;
			exp_add = 42 / 2;
#endif
			sfx = cgs.media.sfx_rockexp;
			light = 300 / 2;
			duration = 1000;
			lightColor[0] = 1;
			lightColor[1] = 0.75;
			lightColor[2] = 0.0;
			break;
		case PD_GRENADE:
			mod = cgs.media.dishFlashModel;
			shader = cgs.media.grenadeExplosionShader;
			sfx = cgs.media.sfx_rockexp;
			light = 300;
			isSprite = qtrue;
			break;
		case PD_BULLET:
		case PD_BULLET_COLORIZE:
			mod = cgs.media.bulletFlashModel;
			if (bg_projectileinfo[weapon].deathType == PD_BULLET_COLORIZE) {
				shader = cgs.media.bulletExplosionColorizeShader;
			} else {
				shader = cgs.media.bulletExplosionShader;
			}
			r = rand() & 3;
			if ( r == 0 ) {
				sfx = cgs.media.sfx_ric1;
			} else if ( r == 1 ) {
				sfx = cgs.media.sfx_ric2;
			} else {
				sfx = cgs.media.sfx_ric3;
			}
			break;
		case PD_RAIL:
			mod = cgs.media.ringFlashModel;
			shader = cgs.media.railExplosionShader;
			sfx = cgs.media.sfx_plasmaexp;
			break;
		case PD_BFG:
			mod = cgs.media.dishFlashModel;
			shader = cgs.media.bfgExplosionShader;
			sfx = cgs.media.sfx_rockexp;
			break;
		case PD_LIGHTNING:
			if (!instantLightningBeam) {
				mod = cgs.media.lightningExplosionModel;
			} else {
				refEntity_t  beam;
				vec3_t	angles;

				memset( &beam, 0, sizeof( beam ) );
				beam.hModel = cgs.media.lightningExplosionModel;

				VectorCopy( origin, beam.origin );

				// make a random orientation
				angles[0] = rand() % 360;
				angles[1] = rand() % 360;
				angles[2] = rand() % 360;
				AnglesToAxis( angles, beam.axis );
				trap_R_AddRefEntityToScene( &beam );
			}
			break;
	}

	if (instantLightningBeam) {
		duration /= 10; // Shorten time
		light = 0; // No light
	}

	if (instantLightningImpact) {
		// No explosion
		mod = shader = light = 0;
	}

	// play sound
	if( soundType == IMPACTSOUND_FLESH && cg_projectiles[weapon].hitPlayerSound ) {
		sfx = cg_projectiles[weapon].hitPlayerSound;
	} else if( soundType == IMPACTSOUND_METAL && cg_projectiles[weapon].hitMetalSound ) {
		sfx = cg_projectiles[weapon].hitMetalSound;
	} else {
		for ( r = 0 ; r < 3 ; r++ ) {
			if ( !cg_projectiles[weapon].hitSound[r] )
			{
				break;
			}
		}
		if ( r > 0 ) {
			r = rand() & 3;
			if ( r < 2 ) {
				sfx = cg_projectiles[weapon].hitSound[1];
			} else if ( r == 2 ) {
				sfx = cg_projectiles[weapon].hitSound[0];
			} else {
				sfx = cg_projectiles[weapon].hitSound[2];
			}
		}
	}

	if (instantLightningBeam) {
		// No sound
		sfx = 0;
	}
/*
	switch ( weapon ) {
	default:
#ifndef TURTLEARENA // WEAPONS
	case WP_SHOTGUN:
		sfx = 0;
		break;
#endif
	}
*/
#else
	switch ( weapon ) {
	default:
#ifdef MISSIONPACK
	case WP_NAILGUN:
		if( soundType == IMPACTSOUND_FLESH ) {
			sfx = cgs.media.sfx_nghitflesh;
		} else if( soundType == IMPACTSOUND_METAL ) {
			sfx = cgs.media.sfx_nghitmetal;
		} else {
			sfx = cgs.media.sfx_nghit;
		}
		mark = cgs.media.holeMarkShader;
		radius = 12;
		break;
#endif
	case WP_LIGHTNING:
		// no explosion at LG impact, it is added with the beam
		r = rand() & 3;
		if ( r < 2 ) {
			sfx = cgs.media.sfx_lghit2;
		} else if ( r == 2 ) {
			sfx = cgs.media.sfx_lghit1;
		} else {
			sfx = cgs.media.sfx_lghit3;
		}
		mark = cgs.media.holeMarkShader;
		radius = 12;
		break;
#ifdef MISSIONPACK
	case WP_PROX_LAUNCHER:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.grenadeExplosionShader;
		sfx = cgs.media.sfx_proxexp;
		mark = cgs.media.burnMarkShader;
		radius = 64;
		light = 300;
		isSprite = qtrue;
		break;
#endif
	case WP_GRENADE_LAUNCHER:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.grenadeExplosionShader;
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		radius = 64;
		light = 300;
		isSprite = qtrue;
		break;
	case WP_ROCKET_LAUNCHER:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.rocketExplosionShader;
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		radius = 64;
		light = 300;
		isSprite = qtrue;
		duration = 1000;
		lightColor[0] = 1;
		lightColor[1] = 0.75;
		lightColor[2] = 0.0;
		if (cg_oldRocket.integer == 0) {
			// explosion sprite animation
			VectorMA( origin, 24, dir, sprOrg );
			VectorScale( dir, 64, sprVel );

			CG_ParticleExplosion( "explode1", sprOrg, sprVel, 1400, 20, 30 );
		}
		break;
	case WP_RAILGUN:
		mod = cgs.media.ringFlashModel;
		shader = cgs.media.railExplosionShader;
		//sfx = cgs.media.sfx_railg;
		sfx = cgs.media.sfx_plasmaexp;
		mark = cgs.media.energyMarkShader;
		radius = 24;
		break;
	case WP_PLASMAGUN:
		mod = cgs.media.ringFlashModel;
		shader = cgs.media.plasmaExplosionShader;
		sfx = cgs.media.sfx_plasmaexp;
		mark = cgs.media.energyMarkShader;
		radius = 16;
		break;
	case WP_BFG:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.bfgExplosionShader;
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		radius = 32;
		isSprite = qtrue;
		break;
	case WP_SHOTGUN:
		mod = cgs.media.bulletFlashModel;
		shader = cgs.media.bulletExplosionShader;
		mark = cgs.media.bulletMarkShader;
		sfx = 0;
		radius = 4;
		break;

#ifdef MISSIONPACK
	case WP_CHAINGUN:
		mod = cgs.media.bulletFlashModel;
		if( soundType == IMPACTSOUND_FLESH ) {
			sfx = cgs.media.sfx_chghitflesh;
		} else if( soundType == IMPACTSOUND_METAL ) {
			sfx = cgs.media.sfx_chghitmetal;
		} else {
			sfx = cgs.media.sfx_chghit;
		}
		mark = cgs.media.bulletMarkShader;

		radius = 8;
		break;
#endif

	case WP_MACHINEGUN:
		mod = cgs.media.bulletFlashModel;
		shader = cgs.media.bulletExplosionShader;
		mark = cgs.media.bulletMarkShader;

		r = rand() & 3;
		if ( r == 0 ) {
			sfx = cgs.media.sfx_ric1;
		} else if ( r == 1 ) {
			sfx = cgs.media.sfx_ric2;
		} else {
			sfx = cgs.media.sfx_ric3;
		}

		radius = 8;
		break;
	}
#endif

#ifdef TA_ENTSYS // EXP_SCALE
	if (!isSprite) {
		exp_base = bg_projectileinfo[weapon].splashRadius / 10.0f * 0.2f;
		exp_add = bg_projectileinfo[weapon].splashRadius / 10.0f * 0.8f;
	}
#endif

	if ( sfx ) {
		trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, sfx );
	}

	//
	// create the explosion
	//
	if ( mod ) {
		le = CG_MakeExplosion( origin, dir,
							   mod,	shader,
							   duration, isSprite );
		le->light = light;
		VectorCopy( lightColor, le->lightColor );

		// colorize with player color
		VectorCopy( cgs.playerinfo[playerNum].color1, le->color );
		le->refEntity.shaderRGBA[0] = le->color[0] * 0xff;
		le->refEntity.shaderRGBA[1] = le->color[1] * 0xff;
		le->refEntity.shaderRGBA[2] = le->color[2] * 0xff;
		le->refEntity.shaderRGBA[3] = 0xff;
#ifdef TA_WEAPSYS // SPR_EXP_SCALE
		le->radius = exp_base;
		le->refEntity.radius = exp_add;
#endif
	}

#ifdef TA_WEAPSYS
	if (instantLightningBeam) {
		// No wallmark
		return;
	}

	mark = cg_projectiles[weapon].hitMarkShader;
	radius = cg_projectiles[weapon].hitMarkRadius;

	if (!mark || radius <= 0)
	{
		// No mark or radius.
		return;
	}
#endif

	//
	// explosion mark
	//
#ifdef TA_WEAPSYS
	// plasma fades alpha, all others fade color
	alphaFade = (bg_projectileinfo[weapon].flags & PF_HITMARK_FADE_ALPHA);

	if (bg_projectileinfo[weapon].flags & PF_HITMARK_COLORIZE)
#else
	alphaFade = (mark == cgs.media.energyMarkShader);	// plasma fades alpha, all others fade color
	if ( weapon == WP_RAILGUN )
#endif
	{
		float	*color;

		// colorize with player color
		color = cgs.playerinfo[playerNum].color1;
#ifdef TA_WEAPSYS
		if (!CG_ImpactMark( mark, origin, dir, random()*360, color[0],color[1], color[2],1, alphaFade, radius, qfalse ))
			return;
#else
		CG_ImpactMark( mark, origin, dir, random()*360, color[0],color[1], color[2],1, alphaFade, radius, qfalse );
#endif
	} else {
#ifdef TA_WEAPSYS
		if (!CG_ImpactMark( mark, origin, dir, random()*360, 1,1,1,1, alphaFade, radius, qfalse ))
			return;
#else
		CG_ImpactMark( mark, origin, dir, random()*360, 1,1,1,1, alphaFade, radius, qfalse );
#endif
	}

#ifdef TA_MISC // MATERIALS
	CG_ImpactParticles(origin, dir, radius, -1, playerNum);
#endif
}

/*
=================
CG_MissileHitPlayer
=================
*/
void CG_MissileHitPlayer( int weapon, vec3_t origin, vec3_t dir, int entityNum ) {
#ifdef TA_WEAPSYS
	CG_PlayerHitEffect( origin, entityNum, qfalse );
#else
#ifndef NOBLOOD
	CG_Bleed( origin, entityNum );
#endif
#endif

	// some weapons will make an explosion with the blood, while
	// others will just make the blood
#ifdef TA_WEAPSYS
	switch (bg_projectileinfo[weapon].deathType)
	{
	case PD_GRENADE:
	case PD_ROCKET:
	case PD_ROCKET_SMALL:
	case PD_PLASMA:
	case PD_BFG:
	case PD_NONE_EXP_PLAYER:
		CG_MissileExplode( weapon, 0, origin, dir, IMPACTSOUND_FLESH );
		break;
	default:
		break;
	}
#else
	switch ( weapon ) {
	case WP_GRENADE_LAUNCHER:
	case WP_ROCKET_LAUNCHER:
	case WP_PLASMAGUN:
	case WP_BFG:
#ifdef MISSIONPACK
	case WP_NAILGUN:
	case WP_CHAINGUN:
	case WP_PROX_LAUNCHER:
#endif
		CG_MissileExplode( weapon, 0, origin, dir, IMPACTSOUND_FLESH );
		break;
	default:
		break;
	}
#endif
}

#ifdef TA_WEAPSYS
/*
=================
CG_MissileImpact

Projectile hit something and did not die/explode
caused by an EV_PROJECTILE_BOUNCE or EV_PROJECTILE_STICK event.
=================
*/
void CG_MissileImpact( int projnum, int playerNum, vec3_t origin, vec3_t dir, impactSound_t soundType )
{
	qhandle_t		mark;
	float			radius;
	qboolean		alphaFade;
	sfxHandle_t		sfx;
	int				r;

	sfx = 0;

	// play sound
	if( soundType == IMPACTSOUND_FLESH && cg_projectiles[projnum].impactPlayerSound ) {
		sfx = cg_projectiles[projnum].impactPlayerSound;
	} else if( soundType == IMPACTSOUND_METAL && cg_projectiles[projnum].impactMetalSound ) {
		sfx = cg_projectiles[projnum].impactMetalSound;
	} else {
		for ( r = 0 ; r < 3 ; r++ ) {
			if ( !cg_projectiles[projnum].impactSound[r] )
			{
				break;
			}
		}
		if ( r > 0 ) {
			r = rand() & 3;
			if ( r < 2 ) {
				sfx = cg_projectiles[projnum].impactSound[1];
			} else if ( r == 2 ) {
				sfx = cg_projectiles[projnum].impactSound[0];
			} else {
				sfx = cg_projectiles[projnum].impactSound[2];
			}
		}
	}

	if ( sfx ) {
		trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, sfx );
	}

	mark = cg_projectiles[projnum].impactMarkShader;
	radius = cg_projectiles[projnum].impactMarkRadius;

	if (!mark || radius <= 0)
	{
		// No mark or radius.
		return;
	}

	//
	// impact mark
	//
	// plasma fades alpha, all others fade color
	alphaFade = (bg_projectileinfo[projnum].flags & PF_IMPACTMARK_FADE_ALPHA);

	if (bg_projectileinfo[projnum].flags & PF_IMPACTMARK_COLORIZE)
	{
		float	*color;

		// colorize with player color
		color = cgs.playerinfo[playerNum].color1;
		if (!CG_ImpactMark( mark, origin, dir, random()*360, color[0],color[1], color[2],1, alphaFade, radius, qfalse ))
			return;
	} else {
		if (!CG_ImpactMark( mark, origin, dir, random()*360, 1,1,1,1, alphaFade, radius, qfalse ))
			return;
	}

#ifdef TA_MISC // MATERIALS
	// Spawn a few particles
	CG_ImpactParticles(origin, dir, radius * 0.3f, -1, playerNum);
#endif
}

/*
=================
CG_WeaponImpact

Melee weapon hit wall, caused by an EV_WEAPON_MISS event.
=================
*/
void CG_WeaponImpact( int weaponGroup, int hand, int playerNum, vec3_t origin, vec3_t dir, impactSound_t soundType )
{
	qhandle_t		mod;
	qhandle_t		mark;
	qhandle_t		shader;
	sfxHandle_t		sfx;
	float			radius;
	float			light;
	vec3_t			lightColor;
	localEntity_t	*le;
	int				r;
	qboolean		alphaFade;
	qboolean		isSprite;
	int				duration;
	int				exp_base;
	int				exp_add;
	int				weaponnum;

	weaponnum = bg_weapongroupinfo[weaponGroup].weaponnum[hand];

	mark = 0;
	radius = 32;
	sfx = 0;
	mod = 0;
	shader = 0;
	light = 0;
	lightColor[0] = 1;
	lightColor[1] = 1;
	lightColor[2] = 0;

	// set defaults
	isSprite = qfalse;
	duration = 600;

	// Sprite explosion scaling, starts at "exp_base" and add on "exp_add" using time scaling.
	//		so the bigest it will be is "exp_base"+"exp_add".
	exp_base = 30;
	exp_add = 42;

#if 0 // #ifdef TURTLEARENA // WEAPONS // ZTM: TODO: Only when hit entity that take_damage and not client.
	if (soundType != IMPACTSOUND_FLESH)
	{
		mod = cgs.media.dishFlashModel;
		isSprite = qtrue;
		// Smaller explosion
		exp_base = 30 / 3;
		exp_add = 42 / 4;

		r = rand() & 2;
		if (r >= 1 && cgs.media.meleeHitShader[1])
			shader = cgs.media.meleeHitShader[1];
		else
			shader = cgs.media.meleeHitShader[0];
	}
#endif

	// play sound
	if( soundType == IMPACTSOUND_FLESH && cg_weapons[weaponnum].impactPlayerSound ) {
		sfx = cg_weapons[weaponnum].impactPlayerSound;
	} else if( soundType == IMPACTSOUND_METAL && cg_weapons[weaponnum].impactMetalSound ) {
		sfx = cg_weapons[weaponnum].impactMetalSound;
	} else {
		for ( r = 0 ; r < 3 ; r++ ) {
			if ( !cg_weapons[weaponnum].impactSound[r] )
			{
				break;
			}
		}
		if ( r > 0 ) {
			r = rand() & 3;
			if ( r < 2 ) {
				sfx = cg_weapons[weaponnum].impactSound[1];
			} else if ( r == 2 ) {
				sfx = cg_weapons[weaponnum].impactSound[0];
			} else {
				sfx = cg_weapons[weaponnum].impactSound[2];
			}
		}
	}

	if ( sfx ) {
		trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, sfx );
	}

	//
	// create the explosion
	//
	if ( mod ) {
		le = CG_MakeExplosion( origin, dir,
							   mod,	shader,
							   duration, isSprite );
		le->light = light;
		VectorCopy( lightColor, le->lightColor );

		// colorize with player color
		VectorCopy( cgs.playerinfo[playerNum].color1, le->color );
		le->refEntity.shaderRGBA[0] = le->color[0] * 0xff;
		le->refEntity.shaderRGBA[1] = le->color[1] * 0xff;
		le->refEntity.shaderRGBA[2] = le->color[2] * 0xff;
		le->refEntity.shaderRGBA[3] = 0xff;

		le->radius = exp_base;
		le->refEntity.radius = exp_add;
	}

	mark = cg_weapons[weaponnum].impactMarkShader;
	radius = cg_weapons[weaponnum].impactMarkRadius;

	if (!mark || radius <= 0)
	{
		// No mark or radius.
		return;
	}

	//
	// impact mark
	//
	// plasma fades alpha, all others fade color
	alphaFade = (bg_weaponinfo[weaponnum].flags & WIF_IMPACTMARK_FADE_ALPHA);

	if (bg_weaponinfo[weaponnum].flags & WIF_IMPACTMARK_COLORIZE)
	{
		float	*color;

		// colorize with player color
		color = cgs.playerinfo[playerNum].color1;
		if (!CG_ImpactMark( mark, origin, dir, random()*360, color[0],color[1], color[2],1, alphaFade, radius, qfalse ))
			return; // no impact
	} else {
		if (!CG_ImpactMark( mark, origin, dir, random()*360, 1,1,1,1, alphaFade, radius, qfalse ))
			return; // no impact
	}

	if (bg_weaponinfo[weaponnum].flags & WIF_ALWAYS_DAMAGE) {
		qhandle_t		mod;
		qhandle_t		shader;
		localEntity_t	*le;

		// PD_BULLET_COLORIZE
		mod = cgs.media.bulletFlashModel;
		shader = cgs.media.bulletExplosionColorizeShader;

		duration = 100;

		//
		// create the explosion
		//
		if ( mod ) {
			le = CG_MakeExplosion( origin, dir,
								   mod,	shader,
								   duration, isSprite );
			le->light = light;
			VectorCopy( lightColor, le->lightColor );

			// colorize with player color
			VectorCopy( cgs.playerinfo[playerNum].color1, le->color );
			le->refEntity.shaderRGBA[0] = le->color[0] * 0xff;
			le->refEntity.shaderRGBA[1] = le->color[1] * 0xff;
			le->refEntity.shaderRGBA[2] = le->color[2] * 0xff;
			le->refEntity.shaderRGBA[3] = 0xff;

#ifdef TA_WEAPSYS // SPR_EXP_SCALE
			if (isSprite) {
				le->radius = exp_base;
				le->refEntity.radius = exp_add;
			}
#endif
		}
	} else {
#ifdef TA_MISC // MATERIALS
		CG_ImpactParticles(origin, dir, radius, -1, playerNum);
#endif
	}
}

/*
=================
CG_WeaponHitPlayer

Melee weapon hit player
=================
*/
void CG_WeaponHitPlayer( int weaponGroup, int hand, vec3_t origin, vec3_t dir, int entityNum )
{
	if ((bg_weapongroupinfo[weaponGroup].weapon[hand]->flags & WIF_CUTS) || ((rand()&20) > 15)) {
		CG_PlayerHitEffect(origin, entityNum, qtrue);
	}
}
#endif


#ifndef TA_WEAPSYS
/*
============================================================================

SHOTGUN TRACING

============================================================================
*/

/*
================
CG_ShotgunPellet
================
*/
static void CG_ShotgunPellet( vec3_t start, vec3_t end, int skipNum ) {
	trace_t		tr;
	int sourceContentType, destContentType;

	CG_Trace( &tr, start, NULL, NULL, end, skipNum, MASK_SHOT );

	sourceContentType = CG_PointContents( start, 0 );
	destContentType = CG_PointContents( tr.endpos, 0 );

	// FIXME: should probably move this cruft into CG_BubbleTrail
	if ( sourceContentType == destContentType ) {
		if ( sourceContentType & CONTENTS_WATER ) {
			CG_BubbleTrail( start, tr.endpos, 32 );
		}
	} else if ( sourceContentType & CONTENTS_WATER ) {
		trace_t trace;

		trap_CM_BoxTrace( &trace, end, start, NULL, NULL, 0, CONTENTS_WATER );
		CG_BubbleTrail( start, trace.endpos, 32 );
	} else if ( destContentType & CONTENTS_WATER ) {
		trace_t trace;

		trap_CM_BoxTrace( &trace, start, end, NULL, NULL, 0, CONTENTS_WATER );
		CG_BubbleTrail( tr.endpos, trace.endpos, 32 );
	}

	if (  tr.surfaceFlags & SURF_NOIMPACT ) {
		return;
	}

	if ( cg_entities[tr.entityNum].currentState.eType == ET_PLAYER ) {
		CG_MissileHitPlayer( WP_SHOTGUN, tr.endpos, tr.plane.normal, tr.entityNum );
	} else {
		if ( tr.surfaceFlags & SURF_NOIMPACT ) {
			// SURF_NOIMPACT will not make a flame puff or a mark
			return;
		}
		if ( tr.surfaceFlags & SURF_METALSTEPS ) {
			CG_MissileExplode( WP_SHOTGUN, 0, tr.endpos, tr.plane.normal, IMPACTSOUND_METAL );
		} else {
			CG_MissileExplode( WP_SHOTGUN, 0, tr.endpos, tr.plane.normal, IMPACTSOUND_DEFAULT );
		}
	}
}

/*
================
CG_ShotgunPattern

Perform the same traces the server did to locate the
hit splashes
================
*/
void CG_ShotgunPattern( vec3_t origin, vec3_t origin2, int seed, int otherEntNum ) {
	int			i;
	float		r, u;
	vec3_t		end;
	vec3_t		forward, right, up;

	// derive the right and up vectors from the forward vector, because
	// the client won't have any other information
	VectorNormalize2( origin2, forward );
	PerpendicularVector( right, forward );
	CrossProduct( forward, right, up );

	// generate the "random" spread pattern
	for ( i = 0 ; i < DEFAULT_SHOTGUN_COUNT ; i++ ) {
		r = Q_crandom( &seed ) * DEFAULT_SHOTGUN_SPREAD * 16;
		u = Q_crandom( &seed ) * DEFAULT_SHOTGUN_SPREAD * 16;
		VectorMA( origin, 8192 * 16, forward, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);

		CG_ShotgunPellet( origin, end, otherEntNum );
	}
}

/*
==============
CG_ShotgunFire
==============
*/
void CG_ShotgunFire( entityState_t *es ) {
	vec3_t	v, up;
	int		contents;

	VectorSubtract( es->origin2, es->pos.trBase, v );
	VectorNormalize( v );
	VectorScale( v, 32, v );
	VectorAdd( es->pos.trBase, v, v );

	contents = CG_PointContents( es->pos.trBase, 0 );
	if ( !( contents & CONTENTS_WATER ) ) {
		VectorSet( up, 0, 0, 8 );
		CG_SmokePuff( v, up, 32, 1, 1, 1, 0.33f, 900, cg.time, 0, LEF_PUFF_DONT_SCALE, cgs.media.shotgunSmokePuffShader );
	}

	CG_ShotgunPattern( es->pos.trBase, es->origin2, es->eventParm, es->otherEntityNum );
}
#endif

/*
============================================================================

BULLETS

============================================================================
*/


/*
===============
CG_Tracer
===============
*/
void CG_Tracer( vec3_t source, vec3_t dest ) {
	vec3_t		forward, right;
	polyVert_t	verts[4];
	vec3_t		line;
	float		len, begin, end;
	vec3_t		start, finish;
	vec3_t		midpoint;

	// tracer
	VectorSubtract( dest, source, forward );
	len = VectorNormalize( forward );

	// start at least a little ways from the muzzle
	if ( len < 100 ) {
		return;
	}
	begin = 50 + random() * (len - 60);
	end = begin + cg_tracerLength.value;
	if ( end > len ) {
		end = len;
	}
	VectorMA( source, begin, forward, start );
	VectorMA( source, end, forward, finish );

	line[0] = DotProduct( forward, cg.refdef.viewaxis[1] );
	line[1] = DotProduct( forward, cg.refdef.viewaxis[2] );

	VectorScale( cg.refdef.viewaxis[1], line[1], right );
	VectorMA( right, -line[0], cg.refdef.viewaxis[2], right );
	VectorNormalize( right );

	VectorMA( finish, cg_tracerWidth.value, right, verts[0].xyz );
	verts[0].st[0] = 0;
	verts[0].st[1] = 1;
	verts[0].modulate[0] = 255;
	verts[0].modulate[1] = 255;
	verts[0].modulate[2] = 255;
	verts[0].modulate[3] = 255;

	VectorMA( finish, -cg_tracerWidth.value, right, verts[1].xyz );
	verts[1].st[0] = 1;
	verts[1].st[1] = 0;
	verts[1].modulate[0] = 255;
	verts[1].modulate[1] = 255;
	verts[1].modulate[2] = 255;
	verts[1].modulate[3] = 255;

	VectorMA( start, -cg_tracerWidth.value, right, verts[2].xyz );
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = 255;
	verts[2].modulate[1] = 255;
	verts[2].modulate[2] = 255;
	verts[2].modulate[3] = 255;

	VectorMA( start, cg_tracerWidth.value, right, verts[3].xyz );
	verts[3].st[0] = 0;
	verts[3].st[1] = 0;
	verts[3].modulate[0] = 255;
	verts[3].modulate[1] = 255;
	verts[3].modulate[2] = 255;
	verts[3].modulate[3] = 255;

	trap_R_AddPolyToScene( cgs.media.tracerShader, 4, verts, 0, 0 );

	midpoint[0] = ( start[0] + finish[0] ) * 0.5;
	midpoint[1] = ( start[1] + finish[1] ) * 0.5;
	midpoint[2] = ( start[2] + finish[2] ) * 0.5;

	// add the tracer sound
	trap_S_StartSound( midpoint, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.tracerSound );

}


/*
======================
CG_CalcMuzzlePoint
======================
*/
static qboolean	CG_CalcMuzzlePoint( int entityNum, vec3_t muzzle ) {
	vec3_t		forward;
	centity_t	*cent;
	int			anim;
	playerState_t *ps;

	ps = CG_LocalPlayerState(entityNum);
	if ( ps ) {
		VectorCopy( ps->origin, muzzle );
		muzzle[2] += ps->viewheight;
		AngleVectors( ps->viewangles, forward, NULL, NULL );
		VectorMA( muzzle, 14, forward, muzzle );
		return qtrue;
	}

	cent = &cg_entities[entityNum];
	if ( !cent->currentValid ) {
		return qfalse;
	}

	VectorCopy( cent->currentState.pos.trBase, muzzle );

	AngleVectors( cent->currentState.apos.trBase, forward, NULL, NULL );
	anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
	if ( anim == LEGS_WALKCR || anim == LEGS_IDLECR ) {
		muzzle[2] += CROUCH_VIEWHEIGHT;
	} else {
		muzzle[2] += DEFAULT_VIEWHEIGHT;
	}

	VectorMA( muzzle, 14, forward, muzzle );

	return qtrue;

}

/*
======================
CG_Bullet

Renders bullet effects.
======================
*/
void CG_Bullet( vec3_t end, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum
#ifdef TA_WEAPSYS
		, int projnum
#endif
		)
{
#ifdef TA_WEAPSYS
	// if the shooter is currently valid, calc a source point and possibly
	// do trail effects
	if ( sourceEntityNum >= 0 )
	{
		vec3_t		start;

		if ( !CG_CalcMuzzlePoint( sourceEntityNum, start ) )
		{
			return;
		}

		// draw a tracer
		if ( cg_tracerChance.value > 0 && random() < cg_tracerChance.value ) {
			CG_Tracer( start, end );
		}

		if (!CG_BulletBubbleTrail(start, end, sourceEntityNum))
		{
			return;
		}
	}
#else
	trace_t trace;
	int sourceContentType, destContentType;
	vec3_t		start;

	// if the shooter is currently valid, calc a source point and possibly
	// do trail effects
	if ( sourceEntityNum >= 0 && cg_tracerChance.value > 0 ) {
		if ( CG_CalcMuzzlePoint( sourceEntityNum, start ) ) {
			sourceContentType = CG_PointContents( start, 0 );
			destContentType = CG_PointContents( end, 0 );

			// do a complete bubble trail if necessary
			if ( ( sourceContentType == destContentType ) && ( sourceContentType & CONTENTS_WATER ) ) {
				CG_BubbleTrail( start, end, 32 );
			}
			// bubble trail from water into air
			else if ( ( sourceContentType & CONTENTS_WATER ) ) {
				trap_CM_BoxTrace( &trace, end, start, NULL, NULL, 0, CONTENTS_WATER );
				CG_BubbleTrail( start, trace.endpos, 32 );
			}
			// bubble trail from air into water
			else if ( ( destContentType & CONTENTS_WATER ) ) {
				trap_CM_BoxTrace( &trace, start, end, NULL, NULL, 0, CONTENTS_WATER );
				CG_BubbleTrail( trace.endpos, end, 32 );
			}

			// draw a tracer
			if ( random() < cg_tracerChance.value ) {
				CG_Tracer( start, end );
			}
		}
	}
#endif

	// impact splash and mark
	if ( flesh ) {
#ifdef TA_WEAPSYS
		CG_MissileHitPlayer(projnum, end, normal, fleshEntityNum );
#else
#ifndef NOBLOOD
		CG_Bleed( end, fleshEntityNum );
#endif
#endif
	} else {
#ifdef TA_WEAPSYS
		CG_MissileExplode( projnum, 0, end, normal, IMPACTSOUND_DEFAULT );
#else
		CG_MissileExplode( WP_MACHINEGUN, 0, end, normal, IMPACTSOUND_DEFAULT );
#endif
	}

}

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
// cg_event.c -- handle entity events at snapshot or playerstate transitions

#include "cg_local.h"

// for the voice chats
#ifdef MISSIONPACK
#include "../../ui/menudef.h"
#endif
//==========================================================================

/*
===================
CG_PlaceString

Also called by scoreboard drawing
===================
*/
const char	*CG_PlaceString( int rank ) {
	static char	str[64];
	char	*s, *t;

	if ( rank & RANK_TIED_FLAG ) {
		rank &= ~RANK_TIED_FLAG;
		t = "Tied for ";
	} else {
		t = "";
	}

	if ( rank == 1 ) {
		s = S_COLOR_BLUE "1st" S_COLOR_WHITE;		// draw in blue
	} else if ( rank == 2 ) {
		s = S_COLOR_RED "2nd" S_COLOR_WHITE;		// draw in red
	} else if ( rank == 3 ) {
		s = S_COLOR_YELLOW "3rd" S_COLOR_WHITE;		// draw in yellow
	} else if ( rank == 11 ) {
		s = "11th";
	} else if ( rank == 12 ) {
		s = "12th";
	} else if ( rank == 13 ) {
		s = "13th";
	} else if ( rank % 10 == 1 ) {
		s = va("%ist", rank);
	} else if ( rank % 10 == 2 ) {
		s = va("%ind", rank);
	} else if ( rank % 10 == 3 ) {
		s = va("%ird", rank);
	} else {
		s = va("%ith", rank);
	}

	Com_sprintf( str, sizeof( str ), "%s%s", t, s );
	return str;
}

/*
=============
CG_Obituary
=============
*/
static void CG_Obituary( entityState_t *ent ) {
	int			mod;
	int			target, attacker;
	char		*message;
	char		*message2;
	const char	*targetInfo;
	const char	*attackerInfo;
	char		targetName[32];
	char		attackerName[32];
	gender_t	gender;
	playerInfo_t	*pi;
	int				i;

	target = ent->otherEntityNum;
	attacker = ent->otherEntityNum2;
	mod = ent->eventParm;

	if ( target < 0 || target >= MAX_CLIENTS ) {
		CG_Error( "CG_Obituary: target out of range" );
	}
	pi = &cgs.playerinfo[target];

	if ( attacker < 0 || attacker >= MAX_CLIENTS ) {
		attacker = ENTITYNUM_WORLD;
		attackerInfo = NULL;
	} else {
		attackerInfo = CG_ConfigString( CS_PLAYERS + attacker );
	}

	targetInfo = CG_ConfigString( CS_PLAYERS + target );
	if ( !targetInfo ) {
		return;
	}
	Q_strncpyz( targetName, Info_ValueForKey( targetInfo, "n" ), sizeof(targetName) - 2);
	strcat( targetName, S_COLOR_WHITE );

	message2 = "";

#ifdef TA_PLAYERSYS
	gender = pi->playercfg.gender;
#else
	gender = pi->gender;
#endif

	// check for single player messages

	switch( mod ) {
	case MOD_SUICIDE:
		message = "suicides";
		break;
	case MOD_FALLING:
#ifdef TA_MISC // MOD
		if ( gender == GENDER_FEMALE )
			message = "fell to her doom";
		else if ( gender == GENDER_NEUTER )
			message = "fell to its doom";
		else
			message = "fell to his doom";
#else
		message = "cratered";
#endif
		break;
	case MOD_CRUSH:
		message = "was squished";
		break;
	case MOD_WATER:
#ifdef TA_MISC // MOD
		message = "drowned";
#else
		message = "sank like a rock";
#endif
		break;
	case MOD_SLIME:
#ifdef TA_MISC // MOD // Copied from SRB2
		message = "fell in some nasty goop";
#else
		message = "melted";
#endif
		break;
	case MOD_LAVA:
#ifdef TA_MISC // MOD
		message = "was fried";
#else
		message = "does a back flip into the lava";
#endif
		break;
	case MOD_TARGET_LASER:
#ifdef TA_MISC // MOD
		message = "died";
#else
		message = "saw the light";
#endif
		break;
	case MOD_TRIGGER_HURT:
		message = "was in the wrong place";
		break;
#ifdef TA_ENTSYS
	case MOD_EXPLOSION:
		message = "was in the explosion";
#endif
	default:
		message = NULL;
		break;
	}

	if (attacker == target) {
		switch (mod) {
#if defined MISSIONPACK && !defined TURTLEARENA // NO_KAMIKAZE_ITEM
		case MOD_KAMIKAZE:
			message = "goes out with a bang";
			break;
#endif
#ifndef TURTLEARENA // MOD
		case MOD_GRENADE_SPLASH:
			if ( gender == GENDER_FEMALE )
				message = "tripped on her own grenade";
			else if ( gender == GENDER_NEUTER )
				message = "tripped on its own grenade";
			else
				message = "tripped on his own grenade";
			break;
		case MOD_ROCKET_SPLASH:
			if ( gender == GENDER_FEMALE )
				message = "blew herself up";
			else if ( gender == GENDER_NEUTER )
				message = "blew itself up";
			else
				message = "blew himself up";
			break;
		case MOD_PLASMA_SPLASH:
			if ( gender == GENDER_FEMALE )
				message = "melted herself";
			else if ( gender == GENDER_NEUTER )
				message = "melted itself";
			else
				message = "melted himself";
			break;
		case MOD_BFG_SPLASH:
			message = "should have used a smaller gun";
			break;
#ifdef MISSIONPACK
		case MOD_PROXIMITY_MINE:
			if( gender == GENDER_FEMALE ) {
				message = "found her prox mine";
			} else if ( gender == GENDER_NEUTER ) {
				message = "found its prox mine";
			} else {
				message = "found his prox mine";
			}
			break;
#endif
#endif
		default:
			if ( gender == GENDER_FEMALE )
				message = "killed herself";
			else if ( gender == GENDER_NEUTER )
				message = "killed itself";
			else
				message = "killed himself";
			break;
		}
	}

	if (message) {
		CG_Printf( "%s %s.\n", targetName, message);
		return;
	}

	// check for kill messages from the current playerNum
	if (
#ifdef TA_SP // Don't show in single player
		cgs.gametype != GT_SINGLE_PLAYER &&
#endif
#ifdef IOQ3ZTM // Main goal isn't to kill others, so do not show kill messages.
		cgs.gametype != GT_CTF &&
#ifdef MISSIONPACK
		cgs.gametype != GT_1FCTF &&
		cgs.gametype != GT_OBELISK &&
#endif
#endif
		CG_LocalPlayerState(attacker) )
	{
		char	*s;
		playerState_t	*ps;

		for (i = 0; i < CG_MaxSplitView(); i++) {
			if ( attacker != cg.snap->pss[i].playerNum ) {
				continue;
			}

			ps = &cg.snap->pss[i];

#ifdef NOTRATEDM // frag to KO
			if ( cgs.gametype < GT_TEAM ) {
				s = va("You knocked out %s\n%s place with %i", targetName,
					CG_PlaceString( ps->persistant[PERS_RANK] + 1 ),
					ps->persistant[PERS_SCORE] );
			} else {
				s = va("You knocked out %s", targetName );
			}
#else
			if ( cgs.gametype < GT_TEAM ) {
				s = va("You fragged %s\n%s place with %i", targetName, 
					CG_PlaceString( ps->persistant[PERS_RANK] + 1 ),
					ps->persistant[PERS_SCORE] );
			} else {
				s = va("You fragged %s", targetName );
			}
#endif
#ifdef MISSIONPACK
			if (!(cg_singlePlayer.integer && cg.localPlayers[i].cameraOrbit)) {
				CG_CenterPrint( i, s, SCREEN_HEIGHT * 0.30, 0.5 );
			} 
#else
			CG_CenterPrint( i, s, SCREEN_HEIGHT * 0.30, 0.5 );
#endif
		}

		// print the text message as well
	}

	// check for double player messages
	if ( !attackerInfo ) {
		attacker = ENTITYNUM_WORLD;
		strcpy( attackerName, "noname" );
	} else {
		Q_strncpyz( attackerName, Info_ValueForKey( attackerInfo, "n" ), sizeof(attackerName) - 2);
		strcat( attackerName, S_COLOR_WHITE );
		// check for kill messages about the current playerNum
		for (i = 0; i < CG_MaxSplitView(); i++) {
			if ( target == cg.snap->pss[i].playerNum ) {
				Q_strncpyz( cg.localPlayers[i].killerName, attackerName, sizeof( cg.localPlayers[i].killerName ) );
			}
		}
	}

	if ( attacker != ENTITYNUM_WORLD ) {
#ifdef TA_WEAPSYS
		if (mod == MOD_PROJECTILE || mod == MOD_PROJECTILE_EXPLOSION) {
			if (mod == MOD_PROJECTILE_EXPLOSION)
				message = "almost dodged";
			else
				message = "was killed by";
			CG_Printf( "%s %s %s's %s\n",
					targetName, message, attackerName, &bg_projectileinfo[ent->weapon].name[2]);
			return;
		}

		if (mod == MOD_WEAPON_PRIMARY || mod == MOD_WEAPON_SECONDARY) {
			message = "was killed by";
			CG_Printf( "%s %s %s's %s\n",
					targetName, message, attackerName, &bg_weapongroupinfo[ent->weapon].weapon[mod-MOD_WEAPON_PRIMARY]->name[2]);
			return;
		}
#endif

		switch (mod) {
		case MOD_GRAPPLE:
			message = "was caught by";
			break;
#ifndef TURTLEARENA // MOD NO_KAMIKAZE_ITEM POWERS
		case MOD_GAUNTLET:
			message = "was pummeled by";
			break;
		case MOD_MACHINEGUN:
			message = "was machinegunned by";
			break;
		case MOD_SHOTGUN:
			message = "was gunned down by";
			break;
		case MOD_GRENADE:
			message = "ate";
			message2 = "'s grenade";
			break;
		case MOD_GRENADE_SPLASH:
			message = "was shredded by";
			message2 = "'s shrapnel";
			break;
		case MOD_ROCKET:
			message = "ate";
			message2 = "'s rocket";
			break;
		case MOD_ROCKET_SPLASH:
			message = "almost dodged";
			message2 = "'s rocket";
			break;
		case MOD_PLASMA:
			message = "was melted by";
			message2 = "'s plasmagun";
			break;
		case MOD_PLASMA_SPLASH:
			message = "was melted by";
			message2 = "'s plasmagun";
			break;
		case MOD_RAILGUN:
			message = "was railed by";
			break;
		case MOD_LIGHTNING:
			message = "was electrocuted by";
			break;
		case MOD_BFG:
		case MOD_BFG_SPLASH:
			message = "was blasted by";
			message2 = "'s BFG";
			break;
#ifdef MISSIONPACK
		case MOD_NAIL:
			message = "was nailed by";
			break;
		case MOD_CHAINGUN:
			message = "got lead poisoning from";
			message2 = "'s Chaingun";
			break;
		case MOD_PROXIMITY_MINE:
			message = "was too close to";
			message2 = "'s Prox Mine";
			break;
		case MOD_KAMIKAZE:
			message = "falls to";
			message2 = "'s Kamikaze blast";
			break;
		case MOD_JUICED:
			message = "was juiced by";
			break;
#endif
#endif
		case MOD_TELEFRAG:
			message = "tried to invade";
			message2 = "'s personal space";
			break;
		default:
			message = "was killed by";
			break;
		}

		if (message) {
			CG_Printf( "%s %s %s%s\n", 
				targetName, message, attackerName, message2);
			return;
		}
	}

	// we don't know what it was
	CG_Printf( "%s died.\n", targetName );
}

//==========================================================================

/*
===============
CG_UseItem
===============
*/
static void CG_UseItem( centity_t *cent ) {
	playerInfo_t *pi;
	int			itemNum, playerNum;
	gitem_t		*item;
	entityState_t *es;
	int			i;

	es = &cent->currentState;
	
	itemNum = (es->event & ~EV_EVENT_BITS) - EV_USE_ITEM0;
	if ( itemNum < 0 || itemNum > BG_NumHoldableItems() ) {
		itemNum = 0;
	}

	// print a message if the local player
	for (i = 0; i < CG_MaxSplitView(); i++) {
		if ( es->number != cg.snap->pss[i].playerNum ) {
			continue;
		}

#ifndef TA_HOLDSYS
		if ( !itemNum ) {
			CG_CenterPrint( i, "No item to use", SCREEN_HEIGHT * 0.30, 0.5 );
		} else
#endif
#ifdef TURTLEARENA // HOLD_SHURIKEN
		if (!BG_ProjectileIndexForHoldable(itemNum))
#endif
		{
			item = BG_FindItemForHoldable( itemNum );
			if (item) {
#ifdef TA_DATA // Eat pizza, don't "use" it.
				if (itemNum == HI_MEDKIT) {
					CG_CenterPrint( i, va("Ate %s", item->pickup_name), SCREEN_HEIGHT * 0.30, 0.5 );
				} else
#endif
				CG_CenterPrint( i, va("Use %s", item->pickup_name), SCREEN_HEIGHT * 0.30, 0.5 );
			}
		}
	}

	switch ( itemNum ) {
	default:
#ifdef TURTLEARENA // HOLD_SHURIKEN
		// ZTM: Play shuriken use sound
		if (BG_ProjectileIndexForHoldable(itemNum)) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.shurikenSound );
			break;
		}
#endif
	case HI_NONE:
		trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.useNothingSound );
		break;

#ifndef TURTLEARENA // HOLDABLE // no q3 teleprter
	case HI_TELEPORTER:
		break;
#endif

	case HI_MEDKIT:
		playerNum = cent->currentState.playerNum;
		if ( playerNum >= 0 && playerNum < MAX_CLIENTS ) {
			pi = &cgs.playerinfo[ playerNum ];
			pi->medkitUsageTime = cg.time;
		}
		trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.medkitSound );
		break;

#ifdef MISSIONPACK
#ifndef TURTLEARENA // NO_KAMIKAZE_ITEM
	case HI_KAMIKAZE:
		break;
#endif

	case HI_PORTAL:
		break;
#ifndef TURTLEARENA // POWERS
	case HI_INVULNERABILITY:
		trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.useInvulnerabilitySound );
		break;
#endif
#endif
	}

}

/*
================
CG_ItemPickup

A new item was picked up this frame
================
*/
static void CG_ItemPickup( int localPlayerNum, int itemNum ) {
	localPlayer_t *player = &cg.localPlayers[localPlayerNum];
	gitem_t *item = BG_ItemForItemNum( itemNum );

#ifdef TURTLEARENA // NIGTHS_ITEMS
	if (item->giType == IT_SCORE) {
		player->scorePickupTime = cg.time;
		return; // Do not count as a pickup item
	}
#endif

	player->itemPickup = itemNum;
	player->itemPickupTime = cg.time;
	player->itemPickupBlendTime = cg.time;

#ifdef TA_HOLDSYS
	if (item->giType == IT_HOLDABLE)
	{
#ifdef TA_HOLDSYS/*2*/
		// Select the holdable
		player->holdableSelect = item->giTag;
#endif

		// holdable is really given in game, but do it anyway...
		if (item->quantity == 0)
			player->predictedPlayerState.holdable[item->giTag] = 1;
		else
			player->predictedPlayerState.holdable[item->giTag] += item->quantity;

		if (player->predictedPlayerState.holdable[item->giTag] > MAX_SHURIKENS)
			player->predictedPlayerState.holdable[item->giTag] = MAX_SHURIKENS;
	}
#endif
	// see if it should be the grabbed weapon
	if ( item->giType == IT_WEAPON ) {
		// select it immediately
#ifdef TA_WEAPSYS_EX
		// always switch
#elif defined TA_WEAPSYS || defined IOQ3ZTM
		if ( cg_autoswitch[localPlayerNum].integer )
#else
		if ( cg_autoswitch[localPlayerNum].integer && item->giTag != WP_MACHINEGUN )
#endif
		{
#ifdef TA_WEAPSYS_EX // The weapon "should" be selected in game and sent in the next snap too
			player->predictedPlayerState.stats[STAT_PENDING_WEAPON] = item->giTag;
#else
			player->weaponSelectTime = cg.time;
			player->weaponSelect = bg_itemlist[itemNum].giTag;
#endif
		}
	}
}

/*
================
CG_WaterLevel

Returns waterlevel for entity origin
================
*/
int CG_WaterLevel(centity_t *cent) {
	vec3_t point;
	int contents, sample1, sample2, anim, waterlevel;
	int viewheight;

	anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;

	if (anim == LEGS_WALKCR || anim == LEGS_IDLECR) {
		viewheight = CROUCH_VIEWHEIGHT;
	} else {
		viewheight = DEFAULT_VIEWHEIGHT;
	}

	//
	// get waterlevel, accounting for ducking
	//
	waterlevel = 0;

	point[0] = cent->lerpOrigin[0];
	point[1] = cent->lerpOrigin[1];
	point[2] = cent->lerpOrigin[2] + MINS_Z + 1;
	contents = CG_PointContents(point, -1);

	if (contents & MASK_WATER) {
		sample2 = viewheight - MINS_Z;
		sample1 = sample2 / 2;
		waterlevel = 1;
		point[2] = cent->lerpOrigin[2] + MINS_Z + sample1;
		contents = CG_PointContents(point, -1);

		if (contents & MASK_WATER) {
			waterlevel = 2;
			point[2] = cent->lerpOrigin[2] + MINS_Z + sample2;
			contents = CG_PointContents(point, -1);

			if (contents & MASK_WATER) {
				waterlevel = 3;
			}
		}
	}

	return waterlevel;
}

/*
================
CG_PainEvent

Also called by playerstate transition
================
*/
void CG_PainEvent( centity_t *cent, int health ) {
	char	*snd;

	// don't do more than two pain sounds a second
	if ( cg.time - cent->pe.painTime < 500 ) {
		return;
	}

	if ( health < 25 ) {
		snd = "*pain25_1.wav";
	} else if ( health < 50 ) {
		snd = "*pain50_1.wav";
	} else if ( health < 75 ) {
		snd = "*pain75_1.wav";
	} else {
		snd = "*pain100_1.wav";
	}
	// play a gurp sound instead of a normal pain sound
	if (CG_WaterLevel(cent) == 3) {
#ifdef IOQ3ZTM // MORE_PLAYER_SOUNDS
		if (rand()&1) {
			trap_S_StartSound(NULL, cent->currentState.number, CHAN_VOICE, CG_CustomSound(cent->currentState.number, "*gurp1.wav"));
		} else {
			trap_S_StartSound(NULL, cent->currentState.number, CHAN_VOICE, CG_CustomSound(cent->currentState.number, "*gurp2.wav"));
		}
#else
		if (rand()&1) {
			trap_S_StartSound(NULL, cent->currentState.number, CHAN_VOICE, CG_CustomSound(cent->currentState.number, "sound/player/gurp1.wav"));
		} else {
			trap_S_StartSound(NULL, cent->currentState.number, CHAN_VOICE, CG_CustomSound(cent->currentState.number, "sound/player/gurp2.wav"));
		}
#endif
	} else {
		trap_S_StartSound(NULL, cent->currentState.number, CHAN_VOICE, CG_CustomSound(cent->currentState.number, snd));
	}
	// save pain time for programitic twitch animation
	cent->pe.painTime = cg.time;
	cent->pe.painDirection ^= 1;
}

#ifdef IOQ3ZTM // DEBUG_ORIGIN
// Based on CG_Item, used to get game model tags working ( TA_WEAPSYS )
void CG_DebugOrigin(centity_t *cent)
{
	entityState_t	*es;
	float			frac;
	localEntity_t	*le;
	refEntity_t		*re;

	es = &cent->currentState;

	// if set to invisible, skip
	if ( es->eFlags & EF_NODRAW ) {
		return;
	}

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

#ifdef MISSIONPACK
	le->leType = LE_SHOWREFENTITY;
#else
	le->leType = LE_FRAGMENT;
#endif
	le->endTime = cg.time + 100;

	le->pos.trType = TR_STATIONARY;
	le->pos.trTime = cg.time;

	// Draw a colored sphere model
#ifdef TA_DATA
	if (cent->currentState.eventParm == 1) // blue
	{
		re->hModel = trap_R_RegisterModel( "models/powerups/instant/speed.md3" );
	}
	else if (cent->currentState.eventParm == 2) // yellow
	{
		re->hModel = trap_R_RegisterModel( "models/powerups/instant/defense.md3" );
	}
	else if (cent->currentState.eventParm == 3) // grey
	{
		re->hModel = trap_R_RegisterModel( "models/powerups/instant/invul.md3" );
	}
	else // red
	{
		re->hModel = trap_R_RegisterModel( "models/powerups/instant/strength.md3" );
	}
#else
	if (cent->currentState.eventParm == 1) // blue
	{
		re->hModel = trap_R_RegisterModel( "models/powerups/health/mega_sphere.md3" );
	}
	else if (cent->currentState.eventParm == 2) // green?
	{
		re->hModel = trap_R_RegisterModel( "models/powerups/health/large_sphere.md3" );
	}
	else if (cent->currentState.eventParm == 3) // clear
	{
		re->hModel = trap_R_RegisterModel( "models/powerups/health/small_sphere.md3" );
	}
	else // orange
	{
		re->hModel = trap_R_RegisterModel( "models/powerups/health/medium_sphere.md3" );
	}
#endif

	VectorCopy( cent->currentState.pos.trBase, re->origin );

	AxisCopy( axisDefault, re->axis );

	// scale down
	frac = 0.20f;
	VectorScale( re->axis[0], frac, re->axis[0] );
	VectorScale( re->axis[1], frac, re->axis[1] );
	VectorScale( re->axis[2], frac, re->axis[2] );
	re->nonNormalizedAxes = qtrue;
}
#endif

#ifdef IOQ3ZTM
extern char *eventnames[];
#endif

#ifdef TA_WEAPSYS
/*
================
CG_ImpactSoundForSurf

Return the proper impact sound for surfaceflags
================
*/
impactSound_t CG_ImpactSoundForSurf(int surfaceflags)
{
	if (surfaceflags & SURF_FLESH)
		return IMPACTSOUND_FLESH;
	else if (surfaceflags & SURF_METALSTEPS)
		return IMPACTSOUND_METAL;
	else
		return IMPACTSOUND_DEFAULT;
}
#endif

/*
==============
CG_EntityEvent

An entity has an event value
also called by CG_CheckPlayerstateEvents
==============
*/
#define	DEBUGNAME(x) if(cg_debugEvents.integer){CG_Printf(x"\n");}
#define	DEBUGNAME2(x, y) if(cg_debugEvents.integer){CG_Printf(x"\n",(y));}
void CG_EntityEvent( centity_t *cent, vec3_t position ) {
	entityState_t	*es;
	int				event;
	vec3_t			dir;
	const char		*s;
	int				playerNum;
	playerInfo_t	*pi;
	int				i;

	es = &cent->currentState;
	event = es->event & ~EV_EVENT_BITS;

	if ( cg_debugEvents.integer ) {
		CG_Printf( "ent:%3i  event:%3i ", es->number, event );
	}

	if ( !event ) {
		DEBUGNAME("ZEROEVENT");
		return;
	}

	playerNum = es->playerNum;
	if ( playerNum < 0 || playerNum >= MAX_CLIENTS ) {
		playerNum = 0;
	}
	pi = &cgs.playerinfo[ playerNum ];

	switch ( event ) {
	//
	// movement generated events
	//
	case EV_FOOTSTEP:
		DEBUGNAME("EV_FOOTSTEP");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
#ifdef TA_PLAYERSYS
				cgs.media.footsteps[ pi->playercfg.footsteps ][rand()&3] );
#else
				cgs.media.footsteps[ pi->footsteps ][rand()&3] );
#endif
		}
		break;
	case EV_FOOTSTEP_METAL:
		DEBUGNAME("EV_FOOTSTEP_METAL");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_METAL ][rand()&3] );
		}
		break;
	case EV_FOOTSPLASH:
		DEBUGNAME("EV_FOOTSPLASH");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;
	case EV_FOOTWADE:
		DEBUGNAME("EV_FOOTWADE");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;
	case EV_SWIM:
		DEBUGNAME("EV_SWIM");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;


	case EV_FALL_SHORT:
		DEBUGNAME("EV_FALL_SHORT");
#ifdef IOQ3ZTM // MORE_PLAYER_SOUNDS
		trap_S_StartSound( NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*land1.wav" ) );
#else
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.landSound );
#endif
		for (i = 0; i < CG_MaxSplitView(); i++) {
			if ( playerNum == cg.snap->pss[i].playerNum ) {
				// smooth landing z changes
				cg.localPlayers[i].landChange = -8;
				cg.localPlayers[i].landTime = cg.time;
			}
		}
		break;
	case EV_FALL_MEDIUM:
		DEBUGNAME("EV_FALL_MEDIUM");
		// use normal pain sound
		trap_S_StartSound( NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*pain100_1.wav" ) );
		for (i = 0; i < CG_MaxSplitView(); i++) {
			if ( playerNum == cg.snap->pss[i].playerNum ) {
				// smooth landing z changes
				cg.localPlayers[i].landChange = -16;
				cg.localPlayers[i].landTime = cg.time;
			}
		}
		break;
	case EV_FALL_FAR:
		DEBUGNAME("EV_FALL_FAR");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, CG_CustomSound( es->number, "*fall1.wav" ) );
		cent->pe.painTime = cg.time;	// don't play a pain sound right after this
		for (i = 0; i < CG_MaxSplitView(); i++) {
			if ( playerNum == cg.snap->pss[i].playerNum ) {
				// smooth landing z changes
				cg.localPlayers[i].landChange = -24;
				cg.localPlayers[i].landTime = cg.time;
			}
		}
		break;

	case EV_STEP_4:
	case EV_STEP_8:
	case EV_STEP_12:
	case EV_STEP_16:		// smooth out step up transitions
		DEBUGNAME("EV_STEP");
	{
		float	oldStep;
		int		delta;
		int		step;
		localPlayer_t *player;
		playerState_t *ps;

		for (i = 0; i < CG_MaxSplitView(); i++) {
			player = &cg.localPlayers[i];
			ps = &cg.snap->pss[i];

			if ( playerNum != ps->playerNum ) {
				continue;
			}

			// if we are interpolating, we don't need to smooth steps
			if ( cg.demoPlayback || (ps->pm_flags & PMF_FOLLOW) ||
				cg_nopredict.integer || cg_synchronousClients.integer ) {
				continue;
			}
			// check for stepping up before a previous step is completed
			delta = cg.time - player->stepTime;
			if (delta < STEP_TIME) {
				oldStep = player->stepChange * (STEP_TIME - delta) / STEP_TIME;
			} else {
				oldStep = 0;
			}

			// add this amount
			step = 4 * (event - EV_STEP_4 + 1 );
			player->stepChange = oldStep + step;
			if ( player->stepChange > MAX_STEP_CHANGE ) {
				player->stepChange = MAX_STEP_CHANGE;
			}
			player->stepTime = cg.time;
		}
		break;
	}

	case EV_JUMP_PAD:
		DEBUGNAME("EV_JUMP_PAD");
//		CG_Printf( "EV_JUMP_PAD w/effect #%i\n", es->eventParm );
		{
			vec3_t			up = {0, 0, 1};


			CG_SmokePuff( cent->lerpOrigin, up, 
						  32, 
						  1, 1, 1, 0.33f,
						  1000, 
						  cg.time, 0,
						  LEF_PUFF_DONT_SCALE, 
						  cgs.media.smokePuffShader );
		}

		// boing sound at origin, jump sound on player
		trap_S_StartSound ( cent->lerpOrigin, -1, CHAN_VOICE, cgs.media.jumpPadSound );
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*jump1.wav" ) );
		break;

	case EV_JUMP:
		DEBUGNAME("EV_JUMP");
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*jump1.wav" ) );
		break;
	case EV_TAUNT:
		DEBUGNAME("EV_TAUNT");
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*taunt.wav" ) );
		break;
#ifdef MISSIONPACK
	case EV_TAUNT_YES:
		DEBUGNAME("EV_TAUNT_YES");
		CG_VoiceChatLocal(~0, SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_YES);
		break;
	case EV_TAUNT_NO:
		DEBUGNAME("EV_TAUNT_NO");
		CG_VoiceChatLocal(~0, SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_NO);
		break;
	case EV_TAUNT_FOLLOWME:
		DEBUGNAME("EV_TAUNT_FOLLOWME");
		CG_VoiceChatLocal(~0, SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_FOLLOWME);
		break;
	case EV_TAUNT_GETFLAG:
		DEBUGNAME("EV_TAUNT_GETFLAG");
		CG_VoiceChatLocal(~0, SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_ONGETFLAG);
		break;
	case EV_TAUNT_GUARDBASE:
		DEBUGNAME("EV_TAUNT_GUARDBASE");
		CG_VoiceChatLocal(~0, SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_ONDEFENSE);
		break;
	case EV_TAUNT_PATROL:
		DEBUGNAME("EV_TAUNT_PATROL");
		CG_VoiceChatLocal(~0, SAY_TEAM, qfalse, es->number, COLOR_CYAN, VOICECHAT_ONPATROL);
		break;
#endif
	case EV_WATER_TOUCH:
		DEBUGNAME("EV_WATER_TOUCH");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrInSound );
		break;
	case EV_WATER_LEAVE:
		DEBUGNAME("EV_WATER_LEAVE");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrOutSound );
		break;
	case EV_WATER_UNDER:
		DEBUGNAME("EV_WATER_UNDER");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrUnSound );
		break;
	case EV_WATER_CLEAR:
		DEBUGNAME("EV_WATER_CLEAR");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, CG_CustomSound( es->number, "*gasp.wav" ) );
		break;

	case EV_ITEM_PICKUP:
		DEBUGNAME("EV_ITEM_PICKUP");
		{
#ifndef TURTLEARENA // POWERS
			gitem_t	*item;
#endif
			int		index;

			index = es->eventParm;		// player predicted

			if ( index < 1 || index >= BG_NumItems() ) {
				break;
			}

#ifdef TURTLEARENA // POWERS
			trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.itemPickupSounds[ index ] );
#else
			item = BG_ItemForItemNum( index );

			// powerups and team items will have a separate global sound, this one
			// will be played at prediction time
			if ( item->giType == IT_POWERUP || item->giType == IT_TEAM) {
				trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.n_healthSound );
			} else if (item->giType == IT_PERSISTANT_POWERUP) {
#ifdef MISSIONPACK
				switch (item->giTag ) {
					case PW_SCOUT:
						trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.scoutSound );
					break;
					case PW_GUARD:
						trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.guardSound );
					break;
					case PW_DOUBLER:
						trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.doublerSound );
					break;
					case PW_AMMOREGEN:
						trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.ammoregenSound );
					break;
				}
#endif
			} else {
				trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.itemPickupSounds[ index ] );
			}
#endif

			// show icon and name on status bar
			for (i = 0; i < CG_MaxSplitView(); i++) {
				if ( es->number == cg.snap->pss[i].playerNum ) {
					CG_ItemPickup( i, index );
				}
			}
		}
		break;

	case EV_GLOBAL_ITEM_PICKUP:
		DEBUGNAME("EV_GLOBAL_ITEM_PICKUP");
		{
			int		index;

			index = es->eventParm;		// player predicted

			if ( index < 1 || index >= BG_NumItems() ) {
				break;
			}
			// powerup pickups are global
			trap_S_StartLocalSound( cgs.media.itemPickupSounds[ index ], CHAN_AUTO );

			// show icon and name on status bar
			for (i = 0; i < CG_MaxSplitView(); i++) {
				if ( es->number == cg.snap->pss[i].playerNum ) {
					CG_ItemPickup( i, index );
				}
			}
		}
		break;

	//
	// weapon events
	//
#ifdef TA_WEAPSYS_EX
	case EV_DROP_WEAPON:
		DEBUGNAME("EV_DROP_WEAPON");
		// Start a sound when a weapon is dropped?
		break;
#else
	case EV_NOAMMO:
		DEBUGNAME("EV_NOAMMO");
//		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.noAmmoSound );
		for (i = 0; i < CG_MaxSplitView(); i++) {
			if ( es->number == cg.snap->pss[i].playerNum ) {
				CG_OutOfAmmoChange(i);
			}
		}
		break;
#endif
	case EV_CHANGE_WEAPON:
		DEBUGNAME("EV_CHANGE_WEAPON");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.selectSound );
		break;
	case EV_FIRE_WEAPON:
		DEBUGNAME("EV_FIRE_WEAPON");
		CG_FireWeapon( cent );
		break;

	case EV_USE_ITEM0:
	case EV_USE_ITEM1:
	case EV_USE_ITEM2:
	case EV_USE_ITEM3:
	case EV_USE_ITEM4:
	case EV_USE_ITEM5:
	case EV_USE_ITEM6:
	case EV_USE_ITEM7:
	case EV_USE_ITEM8:
	case EV_USE_ITEM9:
	case EV_USE_ITEM10:
	case EV_USE_ITEM11:
	case EV_USE_ITEM12:
	case EV_USE_ITEM13:
	case EV_USE_ITEM14:
	case EV_USE_ITEM15:
		DEBUGNAME2("EV_USE_ITEM%d", event - EV_USE_ITEM0);
		CG_UseItem( cent );
		break;

	//=================================================================

	//
	// other events
	//
	case EV_PLAYER_TELEPORT_IN:
		DEBUGNAME("EV_PLAYER_TELEPORT_IN");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.teleInSound );
		CG_SpawnEffect( position);
		break;

	case EV_PLAYER_TELEPORT_OUT:
		DEBUGNAME("EV_PLAYER_TELEPORT_OUT");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.teleOutSound );
		CG_SpawnEffect(  position);
		break;

	case EV_ITEM_POP:
		DEBUGNAME("EV_ITEM_POP");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.respawnSound );
		break;
	case EV_ITEM_RESPAWN:
		DEBUGNAME("EV_ITEM_RESPAWN");
		cent->miscTime = cg.time;	// scale up from this
#ifdef IOQ3ZTM // ITEMS_DISAPPEAR
		if (es->eventParm == 0)
#endif
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.respawnSound );
		break;

#ifdef TA_WEAPSYS
	case EV_PROJECTILE_BOUNCE: // EV_GRENADE_BOUNCE
		DEBUGNAME("EV_PROJECTILE_BOUNCE");
		ByteToDir( es->eventParm, dir );
		CG_MissileImpact( es->weapon, es->playerNum, position, dir, CG_ImpactSoundForSurf(es->time2) );
		break;

	case EV_PROJECTILE_STICK: // EV_PROXIMITY_MINE_STICK
		DEBUGNAME("EV_PROJECTILE_STICK");
		ByteToDir( es->eventParm, dir );
		CG_MissileImpact( es->weapon, es->playerNum, position, dir, CG_ImpactSoundForSurf(es->time2) );
		break;

	case EV_PROJECTILE_TRIGGER: // EV_PROXIMITY_MINE_TRIGGER
		DEBUGNAME("EV_PROJECTILE_TRIGGER");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cg_projectiles[es->weapon].triggerSound );
		break;
#else
	case EV_GRENADE_BOUNCE:
		DEBUGNAME("EV_GRENADE_BOUNCE");
		if ( rand() & 1 ) {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.hgrenb1aSound );
		} else {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.hgrenb2aSound );
		}
		break;
#endif

#ifdef MISSIONPACK
#ifndef TA_WEAPSYS
	case EV_PROXIMITY_MINE_STICK:
		DEBUGNAME("EV_PROXIMITY_MINE_STICK");
		if( es->eventParm & SURF_FLESH ) {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.wstbimplSound );
		} else 	if( es->eventParm & SURF_METALSTEPS ) {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.wstbimpmSound );
		} else {
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.wstbimpdSound );
		}
		break;

	case EV_PROXIMITY_MINE_TRIGGER:
		DEBUGNAME("EV_PROXIMITY_MINE_TRIGGER");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.wstbactvSound );
		break;
#endif
#ifndef TURTLEARENA // NO_KAMIKAZE_ITEM
	case EV_KAMIKAZE:
		DEBUGNAME("EV_KAMIKAZE");
		CG_KamikazeEffect( cent->lerpOrigin );
		break;
#endif
	case EV_OBELISKEXPLODE:
		DEBUGNAME("EV_OBELISKEXPLODE");
		CG_ObeliskExplode( cent->lerpOrigin, es->eventParm );
		break;
	case EV_OBELISKPAIN:
		DEBUGNAME("EV_OBELISKPAIN");
		CG_ObeliskPain( cent->lerpOrigin );
		break;
#ifndef TURTLEARENA // POWERS
	case EV_INVUL_IMPACT:
		DEBUGNAME("EV_INVUL_IMPACT");
		CG_InvulnerabilityImpact( cent->lerpOrigin, cent->currentState.angles );
		break;
	case EV_JUICED:
		DEBUGNAME("EV_JUICED");
		CG_InvulnerabilityJuiced( cent->lerpOrigin );
		break;
	case EV_LIGHTNINGBOLT:
		DEBUGNAME("EV_LIGHTNINGBOLT");
#ifdef TA_WEAPSYS
		if (es->weapon >= 0 && es->weapon < BG_NumProjectiles()) {
			CG_LightningBoltBeam(&cg_projectiles[es->weapon], es->origin2, es->pos.trBase);
		}
#else
		CG_LightningBoltBeam(es->origin2, es->pos.trBase);
#endif
		break;
#endif
#endif
	case EV_SCOREPLUM:
		DEBUGNAME("EV_SCOREPLUM");
		CG_ScorePlum( cent->currentState.otherEntityNum, cent->lerpOrigin, cent->currentState.time );
		break;
#ifdef TURTLEARENA // NIGHTS_ITEMS
	case EV_CHAINPLUM:
		DEBUGNAME("EV_CHAINPLUM");
		CG_ChainPlum( cent->currentState.otherEntityNum, cent->lerpOrigin, cent->currentState.time,
				cent->currentState.generic1, cent->currentState.time2 );
		break;
#endif

#ifdef TA_ENTSYS // BREAKABLE
	case EV_SPAWN_DEBRIS:
		DEBUGNAME("EV_SPAWN_DEBRIS");

		// Check for (explosion) sound
		if (es->generic1 > 0 && es->generic1 < MAX_SOUNDS-1)
		{
			// Play sound
			if ( cgs.gameSounds[ es->generic1 ] ) {
				trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.gameSounds[ es->generic1 ] );
			}
		}
#ifdef TA_MISC // MATERIALS
		// Auto select explosion sound
		else if (es->generic1 == MAX_SOUNDS-1)
		{
			int sfx;

			sfx = 0;

			// Select sound using surfaceFlags (es->time2)
			if (es->time2 & SURF_GLASS) {
				sfx = cgs.media.matExplode[MT_GLASS];
			} else if (es->time2 & SURF_WOOD) {
				sfx = cgs.media.matExplode[MT_WOOD];
			} else if (es->time2 & SURF_METAL) {
				sfx = cgs.media.matExplode[MT_METAL];
			} else if (es->time2 & SURF_STONE) {
				sfx = cgs.media.matExplode[MT_STONE];
			} else if (es->time2 & SURF_DIRT) {
				sfx = cgs.media.matExplode[MT_DIRT];
			} else if (es->time2 & SURF_GRASS) {
				sfx = cgs.media.matExplode[MT_GRASS];
			} else if (es->time2 & SURF_SPARKS) {
				sfx = cgs.media.matExplode[MT_SPARKS];
			}

			if (!sfx) {
				// Generic exploding sound
				sfx = cgs.media.matExplode[MT_NONE];
			}

			if (sfx) {
				trap_S_StartSound (NULL, es->number, CHAN_AUTO, sfx );
			}
		}
#endif

		if ( es->eventParm != 255 ) {
			ByteToDir( es->eventParm, dir );
		} else {
			localEntity_t	*smoke;
			vec3_t			up;

			VectorSet( up, 0, 0, 64 );

			smoke = CG_SmokePuff( position, up, es->otherEntityNum*2, 1, 1, 1, 1, 700, cg.time, 0, 0, cgs.media.smokePuffShader );
			// use the optimized local entity add
			smoke->leType = LE_SCALE_FADE;

			VectorSet(dir, 0, 0, 1);
		}
#ifdef TA_MISC // MATERIALS
		CG_ImpactParticles(position, dir, es->otherEntityNum, es->time2, es->otherEntityNum2);
#endif
		break;
	case EV_EXPLOSION:
		DEBUGNAME("EV_EXPLOSION");
		CG_ExplosionEffect(position, es->time2, es->otherEntityNum);
		break;
#endif

	//
	// missile impacts
	//
	case EV_MISSILE_HIT:
		DEBUGNAME("EV_MISSILE_HIT");
		ByteToDir( es->eventParm, dir );
#ifdef TA_WEAPSYS
		if (bg_projectileinfo[es->weapon].trailType == PT_BULLET)
		{
			CG_Bullet( es->pos.trBase, es->playerNum, dir, qtrue, es->otherEntityNum, es->weapon );
			break;
		}
#endif
		CG_MissileHitPlayer( es->weapon, position, dir, es->otherEntityNum );
		break;

	case EV_MISSILE_MISS:
		DEBUGNAME("EV_MISSILE_MISS");
		ByteToDir( es->eventParm, dir );
#ifdef TA_WEAPSYS
		if (bg_projectileinfo[es->weapon].trailType == PT_BULLET)
		{
			CG_Bullet( es->pos.trBase, es->playerNum, dir, qfalse, es->otherEntityNum, es->weapon );
			break;
		}
		CG_MissileExplode( es->weapon, es->playerNum, position, dir, IMPACTSOUND_DEFAULT );
#else
		CG_MissileExplode( es->weapon, 0, position, dir, IMPACTSOUND_DEFAULT );
#endif
		break;

	case EV_MISSILE_MISS_METAL:
		DEBUGNAME("EV_MISSILE_MISS_METAL");
		ByteToDir( es->eventParm, dir );
#ifdef TA_WEAPSYS
		if (bg_projectileinfo[es->weapon].trailType == PT_BULLET)
		{
			CG_Bullet( es->pos.trBase, es->playerNum, dir, qfalse, es->otherEntityNum, es->weapon );
			break;
		}
		CG_MissileExplode( es->weapon, es->playerNum, position, dir, IMPACTSOUND_METAL );
#else
		CG_MissileExplode( es->weapon, 0, position, dir, IMPACTSOUND_METAL );
#endif
		break;

	case EV_RAILTRAIL:
		DEBUGNAME("EV_RAILTRAIL");
#ifndef TA_WEAPSYS
		cent->currentState.weapon = WP_RAILGUN;

		if ( es->playerNum >= 0 && es->playerNum < MAX_CLIENTS ) {
			for (i = 0; i < CG_MaxSplitView(); i++) {
				if ( es->playerNum == cg.snap->pss[i].playerNum
					&& !cg.localPlayers[i].renderingThirdPerson)
				{
					if(cg_drawGun[i].integer == 2)
						VectorMA(es->origin2, 8, cg.refdef.viewaxis[1], es->origin2);
					else if(cg_drawGun[i].integer == 3)
						VectorMA(es->origin2, 4, cg.refdef.viewaxis[1], es->origin2);
					break;
				}
			}
		}
#endif

#ifdef TA_WEAPSYS
		if (es->playerNum < MAX_CLIENTS && es->weaponHands < MAX_HANDS) {
			VectorCopy(cg_entities[es->playerNum].pe.flashOrigin[es->weaponHands], es->origin2);
			es->origin2[2] += 4;
		}
#elif defined IOQ3ZTM // ATTACH_RAIL_TO_FLASH
		if (es->playerNum < MAX_CLIENTS && es->generic1 == 1) {
			VectorCopy(cg_entities[es->playerNum].pe.flashOrigin, es->origin2);
			es->origin2[2] += 4;
		}
#endif
#ifdef TA_WEAPSYS
		CG_RailTrail(pi, &cg_projectiles[es->weapon], es->origin2, es->pos.trBase);
#else
		CG_RailTrail(pi, es->origin2, es->pos.trBase);

		// if the end was on a nomark surface, don't make an explosion
		if ( es->eventParm != 255 ) {
			ByteToDir( es->eventParm, dir );
			CG_MissileExplode( es->weapon, playerNum, position, dir, IMPACTSOUND_DEFAULT );
		}
#endif
		break;

#ifndef TA_WEAPSYS
	case EV_BULLET_HIT_WALL:
		DEBUGNAME("EV_BULLET_HIT_WALL");
		ByteToDir( es->eventParm, dir );
		CG_Bullet( es->pos.trBase, es->otherEntityNum, dir, qfalse, ENTITYNUM_WORLD );
		break;

	case EV_BULLET_HIT_FLESH:
		DEBUGNAME("EV_BULLET_HIT_FLESH");
		CG_Bullet( es->pos.trBase, es->otherEntityNum, dir, qtrue, es->eventParm );
		break;
#endif

#ifndef TA_WEAPSYS
	case EV_SHOTGUN:
		DEBUGNAME("EV_SHOTGUN");
		CG_ShotgunFire( es );
		break;
#endif

#ifdef TA_WEAPSYS
	//
	// Melee weapon impacts
	//

	case EV_WEAPON_HIT:
		DEBUGNAME("EV_WEAPON_HIT");
		ByteToDir( es->eventParm, dir );
		CG_WeaponHitPlayer( es->weapon, es->weaponHands, position, dir, es->otherEntityNum );
		break;

	case EV_WEAPON_MISS:
		DEBUGNAME("EV_WEAPON_MISS");
		ByteToDir( es->eventParm, dir );
		CG_WeaponImpact( es->weapon, es->weaponHands, es->playerNum, position, dir, IMPACTSOUND_DEFAULT );
		break;

	case EV_WEAPON_MISS_METAL:
		DEBUGNAME("EV_WEAPON_MISS_METAL");
		ByteToDir( es->eventParm, dir );
		CG_WeaponImpact( es->weapon, es->weaponHands, es->playerNum, position, dir, IMPACTSOUND_METAL );
		break;
#endif

	case EV_GENERAL_SOUND:
		DEBUGNAME("EV_GENERAL_SOUND");
		if ( cgs.gameSounds[ es->eventParm ] ) {
			trap_S_StartSound (NULL, es->number, CHAN_VOICE, cgs.gameSounds[ es->eventParm ] );
		} else {
			s = CG_ConfigString( CS_SOUNDS + es->eventParm );
			trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, s ) );
		}
		break;

	case EV_GLOBAL_SOUND:	// play as a local sound so it never diminishes
		DEBUGNAME("EV_GLOBAL_SOUND");
		if ( cgs.gameSounds[ es->eventParm ] ) {
			trap_S_StartLocalSound( cgs.gameSounds[ es->eventParm ], CHAN_AUTO );
		} else {
			s = CG_ConfigString( CS_SOUNDS + es->eventParm );
			trap_S_StartLocalSound( CG_CustomSound( es->number, s ), CHAN_AUTO );
		}
		break;

	case EV_GLOBAL_TEAM_SOUND:	// play as a local sound so it never diminishes
		DEBUGNAME("EV_GLOBAL_TEAM_SOUND");
		{
#ifndef TA_DATA_NEWSOUNDS
			qboolean blueTeam			= qfalse;
			qboolean redTeam			= qfalse;
#endif
			qboolean localHasBlue		= qfalse;
			qboolean localHasRed		= qfalse;
			qboolean localHasNeutral	= qfalse;

			// Check if any local player is on blue/red team or has flags.
			for (i = 0; i < CG_MaxSplitView(); i++) {
				if (cg.snap->playerNums[i] == -1) {
					continue;
				}
#ifndef TA_DATA_NEWSOUNDS
				if (cg.snap->pss[i].persistant[PERS_TEAM] == TEAM_BLUE) {
					blueTeam = qtrue;
				}
				if (cg.snap->pss[i].persistant[PERS_TEAM] == TEAM_RED) {
					redTeam = qtrue;
				}
#endif

				if (cg.snap->pss[i].powerups[PW_BLUEFLAG]) {
					localHasBlue = qtrue;
				}
				if (cg.snap->pss[i].powerups[PW_REDFLAG]) {
					localHasRed = qtrue;
				}
				if (cg.snap->pss[i].powerups[PW_NEUTRALFLAG]) {
					localHasNeutral = qtrue;
				}
			}

#ifndef TA_DATA_NEWSOUNDS
			// ZTM: NOTE: Some of these sounds don't really work with local player on different teams.
			//     New games might want to replace you/enemy sounds with red/blue.
			//     See http://github.com/clover-moe/spearmint/wiki/New-Sounds
#endif

			switch( es->eventParm ) {
				case GTS_RED_CAPTURE: // CTF: red team captured the blue flag, 1FCTF: red team captured the neutral flag
#ifdef TA_DATA
					CG_AddBufferedSound( cgs.media.captureFlagSound );
#else
					if ( redTeam )
						CG_AddBufferedSound( cgs.media.captureYourTeamSound );
					else
						CG_AddBufferedSound( cgs.media.captureOpponentSound );
#endif
					break;
				case GTS_BLUE_CAPTURE: // CTF: blue team captured the red flag, 1FCTF: blue team captured the neutral flag
#ifdef TA_DATA
					CG_AddBufferedSound( cgs.media.captureFlagSound );
#else
					if ( blueTeam )
						CG_AddBufferedSound( cgs.media.captureYourTeamSound );
					else
						CG_AddBufferedSound( cgs.media.captureOpponentSound );
#endif
					break;
				case GTS_RED_RETURN: // CTF: blue flag returned, 1FCTF: never used
#ifdef TA_DATA
					CG_AddBufferedSound( cgs.media.returnFlagSound );
#else
					if ( redTeam )
						CG_AddBufferedSound( cgs.media.returnYourTeamSound );
					else
						CG_AddBufferedSound( cgs.media.returnOpponentSound );
#endif
					//
					CG_AddBufferedSound( cgs.media.blueFlagReturnedSound );
					break;
				case GTS_BLUE_RETURN: // CTF red flag returned, 1FCTF: neutral flag returned
#ifdef TA_DATA
					CG_AddBufferedSound( cgs.media.returnFlagSound );
#ifdef MISSIONPACK
					//
					if (cgs.gametype == GT_1FCTF) {
						CG_AddBufferedSound( cgs.media.neutralFlagReturnedSound );
						break;
					}
#endif
#else
#ifdef MISSIONPACK
					if ( cgs.gametype == GT_1FCTF ) {
						CG_AddBufferedSound( cgs.media.returnOpponentSound );
						CG_AddBufferedSound( cgs.media.neutralFlagReturnedSound );
						break;
					}
#endif

					if ( blueTeam )
						CG_AddBufferedSound( cgs.media.returnYourTeamSound );
					else
						CG_AddBufferedSound( cgs.media.returnOpponentSound );
#endif
					//
					CG_AddBufferedSound( cgs.media.redFlagReturnedSound );
					break;
#ifdef TA_DATA_NEWSOUNDS
				case GTS_RED_TAKEN: // CTF: red team took blue flag, 1FCTF: blue team took the neutral flag
					// if this player picked up the flag then a sound is played in CG_CheckLocalSounds
					if (localHasBlue || localHasNeutral) {
#ifdef MISSIONPACK
					} else if (cgs.gametype == GT_1FCTF) {
						CG_AddBufferedSound( cgs.media.blueTeamTookTheFlagSound );
#endif
					} else {
						CG_AddBufferedSound( cgs.media.redTeamTookBlueFlagSound );
					}
					break;
				case GTS_BLUE_TAKEN: // CTF: blue team took the red flag, 1FCTF red team took the neutral flag
					// if this player picked up the flag then a sound is played in CG_CheckLocalSounds
					if (localHasRed || localHasNeutral) {
#ifdef MISSIONPACK
					} else if (cgs.gametype == GT_1FCTF) {
						CG_AddBufferedSound( cgs.media.redTeamTookTheFlagSound );
#endif
					} else {
						CG_AddBufferedSound( cgs.media.blueTeamTookRedFlagSound );
					}
					break;
#ifdef MISSIONPACK
				case GTS_REDOBELISK_ATTACKED: // Overload: red obelisk is being attacked
					CG_AddBufferedSound( cgs.media.redBaseIsUnderAttackSound );
					break;
				case GTS_BLUEOBELISK_ATTACKED: // Overload: blue obelisk is being attacked
					CG_AddBufferedSound( cgs.media.blueBaseIsUnderAttackSound );
					break;
#endif
#else
				case GTS_RED_TAKEN: // CTF: red team took blue flag, 1FCTF: blue team took the neutral flag
					// if this player picked up the flag then a sound is played in CG_CheckLocalSounds
					if (localHasBlue || localHasNeutral) {
					}
					else if (!(redTeam && blueTeam)) {
						if (blueTeam) {
#ifdef MISSIONPACK
							if (cgs.gametype == GT_1FCTF) 
								CG_AddBufferedSound( cgs.media.yourTeamTookTheFlagSound );
							else
#endif
							CG_AddBufferedSound( cgs.media.enemyTookYourFlagSound );
						}
						else if (redTeam) {
#ifdef MISSIONPACK
							if (cgs.gametype == GT_1FCTF)
								CG_AddBufferedSound( cgs.media.enemyTookTheFlagSound );
							else
#endif
 							CG_AddBufferedSound( cgs.media.yourTeamTookEnemyFlagSound );
						}
					} else {
						// ZTM: NOTE: There are local players on both teams, so have no correct sound to play. New games should fix this.
					}
					break;
				case GTS_BLUE_TAKEN: // CTF: blue team took the red flag, 1FCTF red team took the neutral flag
					// if this player picked up the flag then a sound is played in CG_CheckLocalSounds
					if (localHasRed || localHasNeutral) {
					}
					else if (!(redTeam && blueTeam)) {
						if (redTeam) {
#ifdef MISSIONPACK
							if (cgs.gametype == GT_1FCTF)
								CG_AddBufferedSound( cgs.media.yourTeamTookTheFlagSound );
							else
#endif
							CG_AddBufferedSound( cgs.media.enemyTookYourFlagSound );
						}
						else if (blueTeam) {
#ifdef MISSIONPACK
							if (cgs.gametype == GT_1FCTF)
								CG_AddBufferedSound( cgs.media.enemyTookTheFlagSound );
							else
#endif
							CG_AddBufferedSound( cgs.media.yourTeamTookEnemyFlagSound );
						}
					} else {
						// ZTM: NOTE: There are local players on both teams, so have no correct sound to play. New games should fix this.
					}
					break;
#ifdef MISSIONPACK
				// ZTM: NOTE: These are confusing when there are players on both teams (players don't know which base is attacked). New games should fix this.
				case GTS_REDOBELISK_ATTACKED: // Overload: red obelisk is being attacked
					if (redTeam) {
						CG_AddBufferedSound( cgs.media.yourBaseIsUnderAttackSound );
					}
					break;
				case GTS_BLUEOBELISK_ATTACKED: // Overload: blue obelisk is being attacked
					if (blueTeam) {
						CG_AddBufferedSound( cgs.media.yourBaseIsUnderAttackSound );
					}
					break;
#endif
#endif

				case GTS_REDTEAM_SCORED:
#ifdef TA_MISC // COMIC_ANNOUNCER
					CG_AddAnnouncement( ANNOUNCE_REDSCORES, -1 );
#else
					CG_AddBufferedSound(cgs.media.redScoredSound);
#endif
					break;
				case GTS_BLUETEAM_SCORED:
#ifdef TA_MISC // COMIC_ANNOUNCER
					CG_AddAnnouncement( ANNOUNCE_BLUESCORES, -1 );
#else
					CG_AddBufferedSound(cgs.media.blueScoredSound);
#endif
					break;
				case GTS_REDTEAM_TOOK_LEAD:
					if ( cgs.gametype != GT_TEAM || cg_teamDmLeadAnnouncements.integer ) {
#ifdef TA_MISC // COMIC_ANNOUNCER
						CG_AddAnnouncement( ANNOUNCE_REDLEADS, -1 );
#else
						CG_AddBufferedSound(cgs.media.redLeadsSound);
#endif
					}
					break;
				case GTS_BLUETEAM_TOOK_LEAD:
					if ( cgs.gametype != GT_TEAM || cg_teamDmLeadAnnouncements.integer ) {
#ifdef TA_MISC // COMIC_ANNOUNCER
						CG_AddAnnouncement( ANNOUNCE_BLUELEADS, -1 );
#else
						CG_AddBufferedSound(cgs.media.blueLeadsSound);
#endif
					}
					break;
				case GTS_TEAMS_ARE_TIED:
					if ( cgs.gametype != GT_TEAM || cg_teamDmLeadAnnouncements.integer ) {
#ifdef TA_MISC // COMIC_ANNOUNCER
						CG_AddAnnouncement( ANNOUNCE_TEAMSTIED, -1 );
#else
						CG_AddBufferedSound( cgs.media.teamsTiedSound );
#endif
					}
					break;
#if defined MISSIONPACK && !defined TURTLEARENA // NO_KAMIKAZE_ITEM
				case GTS_KAMIKAZE:
					trap_S_StartLocalSound(cgs.media.kamikazeFarSound, CHAN_ANNOUNCER);
					break;
#endif
				default:
					break;
			}
			break;
		}

	case EV_PAIN:
		// local player sounds are triggered in CG_CheckLocalSounds,
		// so ignore events on the player
		DEBUGNAME("EV_PAIN");
		if ( !CG_LocalPlayerState( es->number ) ) {
			CG_PainEvent( cent, es->eventParm );
		}
		break;

	case EV_DEATH1:
	case EV_DEATH2:
	case EV_DEATH3:
		DEBUGNAME2("EV_DEATH%d", event - EV_DEATH1 + 1);

#ifndef NOTRATEDM // No gibs.
		// check if gibbed
		// eventParm 1 = living player gibbed
		// eventParm 2 = corpse gibbed
		if ( es->eventParm >= 1 ) {
			CG_GibPlayer( cent->lerpOrigin );

			if ( cg_blood.integer && cg_gibs.integer ) {
#ifndef TURTLEARENA // NO_KAMIKAZE_ITEM
				// don't play gib sound when using the kamikaze because it interferes
				// with the kamikaze sound, downside is that the gib sound will also
				// not be played when someone is gibbed while just carrying the kamikaze
				if ( !(es->eFlags & EF_KAMIKAZE) ) {
					trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
				}
#else
				trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
#endif

				// don't play death sound
				break;
			}

			// don't play death sound if already dead
			if ( es->eventParm == 2 ) {
				break;
			}
		}
#endif

		if (CG_WaterLevel(cent) == 3) {
			trap_S_StartSound(NULL, es->number, CHAN_VOICE, CG_CustomSound(es->number, "*drown.wav"));
		} else {
			trap_S_StartSound(NULL, es->number, CHAN_VOICE, CG_CustomSound(es->number, va("*death%i.wav", event - EV_DEATH1 + 1)));
		}

		break;


	case EV_OBITUARY:
		DEBUGNAME("EV_OBITUARY");
#ifndef TURTLEARENA // NOZOOM
#ifdef IOQ3ZTM
		// check for death of the current playerNum
		for (i = 0; i < CG_MaxSplitView(); i++) {
			if ( es->otherEntityNum == cg.snap->pss[i].playerNum ) {
				// if zoomed in, zoom out
				CG_ZoomUp(i);
			}
		}
#endif
#endif
		CG_Obituary( es );
		break;

	//
	// powerup events
	//
	case EV_POWERUP_QUAD:
		DEBUGNAME("EV_POWERUP_QUAD");
		for (i = 0; i < CG_MaxSplitView(); i++) {
			if ( es->number == cg.snap->pss[i].playerNum ) {
				cg.localPlayers[i].powerupActive = PW_QUAD;
				cg.localPlayers[i].powerupTime = cg.time;
			}
		}
		trap_S_StartSound (NULL, es->number, CHAN_ITEM, cgs.media.quadSound );
		break;
	case EV_POWERUP_BATTLESUIT:
		DEBUGNAME("EV_POWERUP_BATTLESUIT");
		for (i = 0; i < CG_MaxSplitView(); i++) {
			if ( es->number == cg.snap->pss[i].playerNum ) {
				cg.localPlayers[i].powerupActive = PW_BATTLESUIT;
				cg.localPlayers[i].powerupTime = cg.time;
			}
		}
		trap_S_StartSound (NULL, es->number, CHAN_ITEM, cgs.media.protectSound );
		break;
	case EV_POWERUP_REGEN:
		DEBUGNAME("EV_POWERUP_REGEN");
#ifndef TURTLEARENA // POWERS
		for (i = 0; i < CG_MaxSplitView(); i++) {
			if ( es->number == cg.snap->pss[i].playerNum ) {
				cg.localPlayers[i].powerupActive = PW_REGEN;
				cg.localPlayers[i].powerupTime = cg.time;
			}
		}
#endif
		trap_S_StartSound (NULL, es->number, CHAN_ITEM, cgs.media.regenSound );
		break;
#ifdef TURTLEARENA // POWERS
	case EV_POWERUP_INVUL:
		DEBUGNAME("EV_POWERUP_INVUL");
		for (i = 0; i < CG_MaxSplitView(); i++) {
			if ( es->number == cg.snap->pss[i].playerNum ) {
				cg.localPlayers[i].powerupActive = PW_INVUL;
				cg.localPlayers[i].powerupTime = cg.time;
			}
		}
		// ZTM: New sound?
		trap_S_StartSound (NULL, es->number, CHAN_ITEM, cgs.media.protectSound );
		break;
#endif

	case EV_STOPLOOPINGSOUND:
		DEBUGNAME("EV_STOPLOOPINGSOUND");
		trap_S_StopLoopingSound( es->number );
		es->loopSound = 0;
		break;

	case EV_DEBUG_LINE:
		DEBUGNAME("EV_DEBUG_LINE");
		CG_Beam( cent );
		break;

#ifdef IOQ3ZTM // DEBUG_ORIGIN
	case EV_DEBUG_ORIGIN:
		DEBUGNAME("EV_DEBUG_LINE");
		CG_DebugOrigin( cent );
		break;
#endif

	default:
		DEBUGNAME("UNKNOWN");
#ifdef IOQ3ZTM
		if (event >= 0 && event < EV_MAX)
			Com_Printf(S_COLOR_YELLOW"Warning: Event '%20s' has no code!\n", eventnames[event]);
		else
		CG_Error( "Unknown event: %i (max=%i)", event, EV_MAX);
#else
		CG_Error( "Unknown event: %i", event );
#endif
		break;
	}

}


/*
==============
CG_CheckEvents

==============
*/
void CG_CheckEvents( centity_t *cent ) {
	// check for event-only entities
	if ( cent->currentState.eType > ET_EVENTS ) {
		if ( cent->previousEvent ) {
			return;	// already fired
		}
		// if this is a player event set the entity number of the player entity number
		if ( cent->currentState.eFlags & EF_PLAYER_EVENT ) {
			cent->currentState.number = cent->currentState.otherEntityNum;
		}

		cent->previousEvent = 1;

		cent->currentState.event = cent->currentState.eType - ET_EVENTS;
	} else {
		// check for events riding with another entity
		if ( cent->currentState.event == cent->previousEvent ) {
			return;
		}
		cent->previousEvent = cent->currentState.event;
		if ( ( cent->currentState.event & ~EV_EVENT_BITS ) == 0 ) {
			return;
		}
	}

	// calculate the position at exactly the frame time
	BG_EvaluateTrajectory( &cent->currentState.pos, cg.snap->serverTime, cent->lerpOrigin );
	CG_SetEntitySoundPosition( cent );

	CG_EntityEvent( cent, cent->lerpOrigin );
}


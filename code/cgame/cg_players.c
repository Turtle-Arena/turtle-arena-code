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
// cg_players.c -- handle the media and animation for player entities
#include "cg_local.h"

char	*cg_customSoundNames[MAX_CUSTOM_SOUNDS] = {
	"*death1.wav",
	"*death2.wav",
	"*death3.wav",
	"*jump1.wav",
	"*pain25_1.wav",
	"*pain50_1.wav",
	"*pain75_1.wav",
	"*pain100_1.wav",
	"*falling1.wav",
	"*gasp.wav",
	"*drown.wav",
	"*fall1.wav",
	"*taunt.wav"
#ifdef IOQ3ZTM // MORE_PLAYER_SOUNDS
	,
	"*gurp1.wav",
	"*gurp2.wav",
	"*land1.wav"
#endif
};


/*
================
CG_CustomSound

================
*/
sfxHandle_t	CG_CustomSound( int clientNum, const char *soundName ) {
	clientInfo_t *ci;
	int			i;

	if ( soundName[0] != '*' ) {
		return trap_S_RegisterSound( soundName, qfalse );
	}

	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		clientNum = 0;
	}
	ci = &cgs.clientinfo[ clientNum ];

	for ( i = 0 ; i < MAX_CUSTOM_SOUNDS && cg_customSoundNames[i] ; i++ ) {
		if ( !strcmp( soundName, cg_customSoundNames[i] ) ) {
			return ci->sounds[i];
		}
	}

	CG_Error( "Unknown custom sound: %s", soundName );
	return 0;
}



/*
=============================================================================

CLIENT INFO

=============================================================================
*/

#ifndef TA_PLAYERSYS // Moved to bg_misc.c BG_ParsePlayerCFGFile
/*
======================
CG_ParseAnimationFile

Read a configuration file containing animation coutns and rates
models/players/visor/animation.cfg, etc
======================
*/
static qboolean	CG_ParseAnimationFile( const char *filename, clientInfo_t *ci ) {
	char		*text_p, *prev;
	int			len;
	int			i;
	char		*token;
	float		fps;
	int			skip;
	char		text[20000];
	fileHandle_t	f;
	animation_t *animations;

	animations = ci->animations;

	// load the file
	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( len <= 0 ) {
		return qfalse;
	}
	if ( len >= sizeof( text ) - 1 ) {
		CG_Printf( "File %s too long\n", filename );
		trap_FS_FCloseFile( f );
		return qfalse;
	}
	trap_FS_Read( text, len, f );
	text[len] = 0;
	trap_FS_FCloseFile( f );

	// parse the text
	text_p = text;
	skip = 0;	// quite the compiler warning

	ci->footsteps = FOOTSTEP_NORMAL;
	VectorClear( ci->headOffset );
	ci->gender = GENDER_MALE;
	ci->fixedlegs = qfalse;
	ci->fixedtorso = qfalse;

	// read optional parameters
	while ( 1 ) {
		prev = text_p;	// so we can unget
		token = COM_Parse( &text_p );
		if ( !token ) {
			break;
		}
		if ( !Q_stricmp( token, "footsteps" ) ) {
			token = COM_Parse( &text_p );
			if ( !token ) {
				break;
			}
			if ( !Q_stricmp( token, "default" ) || !Q_stricmp( token, "normal" ) ) {
				ci->footsteps = FOOTSTEP_NORMAL;
			} else if ( !Q_stricmp( token, "boot" ) ) {
				ci->footsteps = FOOTSTEP_BOOT;
			} else if ( !Q_stricmp( token, "flesh" ) ) {
				ci->footsteps = FOOTSTEP_FLESH;
			} else if ( !Q_stricmp( token, "mech" ) ) {
				ci->footsteps = FOOTSTEP_MECH;
			} else if ( !Q_stricmp( token, "energy" ) ) {
				ci->footsteps = FOOTSTEP_ENERGY;
			} else {
				CG_Printf( "Bad footsteps parm in %s: %s\n", filename, token );
			}
			continue;
		} else if ( !Q_stricmp( token, "headoffset" ) ) {
			for ( i = 0 ; i < 3 ; i++ ) {
				token = COM_Parse( &text_p );
				if ( !token ) {
					break;
				}
				ci->headOffset[i] = atof( token );
			}
			continue;
		} else if ( !Q_stricmp( token, "sex" ) ) {
			token = COM_Parse( &text_p );
			if ( !token ) {
				break;
			}
			if ( token[0] == 'f' || token[0] == 'F' ) {
				ci->gender = GENDER_FEMALE;
			} else if ( token[0] == 'n' || token[0] == 'N' ) {
				ci->gender = GENDER_NEUTER;
			} else {
				ci->gender = GENDER_MALE;
			}
			continue;
		} else if ( !Q_stricmp( token, "fixedlegs" ) ) {
			ci->fixedlegs = qtrue;
			continue;
		} else if ( !Q_stricmp( token, "fixedtorso" ) ) {
			ci->fixedtorso = qtrue;
			continue;
		}

		// if it is a number, start parsing animations
		if ( token[0] >= '0' && token[0] <= '9' ) {
			text_p = prev;	// unget the token
			break;
		}
		Com_Printf( "unknown token '%s' is %s\n", token, filename );
	}

	// read information for each frame
	for ( i = 0 ; i < MAX_ANIMATIONS ; i++ ) {

		token = COM_Parse( &text_p );
		if ( !*token ) {
			if( i >= TORSO_GETFLAG && i <= TORSO_NEGATIVE ) {
				animations[i].firstFrame = animations[TORSO_GESTURE].firstFrame;
				animations[i].frameLerp = animations[TORSO_GESTURE].frameLerp;
				animations[i].initialLerp = animations[TORSO_GESTURE].initialLerp;
				animations[i].loopFrames = animations[TORSO_GESTURE].loopFrames;
				animations[i].numFrames = animations[TORSO_GESTURE].numFrames;
				animations[i].reversed = qfalse;
				animations[i].flipflop = qfalse;
				continue;
			}
			break;
		}
		animations[i].firstFrame = atoi( token );
		// leg only frames are adjusted to not count the upper body only frames
		if ( i == LEGS_WALKCR ) {
			skip = animations[LEGS_WALKCR].firstFrame - animations[TORSO_GESTURE].firstFrame;
		}
		if ( i >= LEGS_WALKCR && i<TORSO_GETFLAG) {
			animations[i].firstFrame -= skip;
		}

		token = COM_Parse( &text_p );
		if ( !*token ) {
			break;
		}
		animations[i].numFrames = atoi( token );

		animations[i].reversed = qfalse;
		animations[i].flipflop = qfalse;
		// if numFrames is negative the animation is reversed
		if (animations[i].numFrames < 0) {
			animations[i].numFrames = -animations[i].numFrames;
			animations[i].reversed = qtrue;
		}

		token = COM_Parse( &text_p );
		if ( !*token ) {
			break;
		}
		animations[i].loopFrames = atoi( token );

		token = COM_Parse( &text_p );
		if ( !*token ) {
			break;
		}
		fps = atof( token );
		if ( fps == 0 ) {
			fps = 1;
		}
		animations[i].frameLerp = 1000 / fps;
		animations[i].initialLerp = 1000 / fps;
	}

	if ( i != MAX_ANIMATIONS ) {
		CG_Printf( "Error parsing animation file: %s\n", filename );
		return qfalse;
	}

	// crouch backward animation
	memcpy(&animations[LEGS_BACKCR], &animations[LEGS_WALKCR], sizeof(animation_t));
	animations[LEGS_BACKCR].reversed = qtrue;
	// walk backward animation
	memcpy(&animations[LEGS_BACKWALK], &animations[LEGS_WALK], sizeof(animation_t));
	animations[LEGS_BACKWALK].reversed = qtrue;
	// flag moving fast
	animations[FLAG_RUN].firstFrame = 0;
	animations[FLAG_RUN].numFrames = 16;
	animations[FLAG_RUN].loopFrames = 16;
	animations[FLAG_RUN].frameLerp = 1000 / 15;
	animations[FLAG_RUN].initialLerp = 1000 / 15;
	animations[FLAG_RUN].reversed = qfalse;
	// flag not moving or moving slowly
	animations[FLAG_STAND].firstFrame = 16;
	animations[FLAG_STAND].numFrames = 5;
	animations[FLAG_STAND].loopFrames = 0;
	animations[FLAG_STAND].frameLerp = 1000 / 20;
	animations[FLAG_STAND].initialLerp = 1000 / 20;
	animations[FLAG_STAND].reversed = qfalse;
	// flag speeding up
	animations[FLAG_STAND2RUN].firstFrame = 16;
	animations[FLAG_STAND2RUN].numFrames = 5;
	animations[FLAG_STAND2RUN].loopFrames = 1;
	animations[FLAG_STAND2RUN].frameLerp = 1000 / 15;
	animations[FLAG_STAND2RUN].initialLerp = 1000 / 15;
	animations[FLAG_STAND2RUN].reversed = qtrue;
	//
	// new anims changes
	//
//	animations[TORSO_GETFLAG].flipflop = qtrue;
//	animations[TORSO_GUARDBASE].flipflop = qtrue;
//	animations[TORSO_PATROL].flipflop = qtrue;
//	animations[TORSO_AFFIRMATIVE].flipflop = qtrue;
//	animations[TORSO_NEGATIVE].flipflop = qtrue;
	//
	return qtrue;
}
#endif

/*
==========================
CG_FileExists
==========================
*/
static qboolean	CG_FileExists(const char *filename) {
	int len;

	len = trap_FS_FOpenFile( filename, NULL, FS_READ );
	if (len>0) {
		return qtrue;
	}
	return qfalse;
}

/*
==========================
CG_FindClientModelFile
==========================
*/
static qboolean	CG_FindClientModelFile( char *filename, int length, clientInfo_t *ci, const char *teamName, const char *modelName, const char *skinName, const char *base, const char *ext ) {
	char *team;
	int i;

	if ( cgs.gametype >= GT_TEAM ) {
		switch ( ci->team ) {
			case TEAM_BLUE: {
				team = "blue";
				break;
			}
			default: {
				team = "red";
				break;
			}
		}
	}
	else {
		team = "default";
	}

	for ( i = 0; i < 2; i++ ) {
		if ( i == 0 && teamName && *teamName ) {
			//								"models/players/james/stroggs/lower_lily_red.skin"
			Com_sprintf( filename, length, "models/players/%s/%s%s_%s_%s.%s", modelName, teamName, base, skinName, team, ext );
		}
		else {
			//								"models/players/james/lower_lily_red.skin"
			Com_sprintf( filename, length, "models/players/%s/%s_%s_%s.%s", modelName, base, skinName, team, ext );
		}
		if ( CG_FileExists( filename ) ) {
			return qtrue;
		}
		if ( cgs.gametype >= GT_TEAM ) {
			if ( i == 0 && teamName && *teamName ) {
				//								"models/players/james/stroggs/lower_red.skin"
				Com_sprintf( filename, length, "models/players/%s/%s%s_%s.%s", modelName, teamName, base, team, ext );
			}
			else {
				//								"models/players/james/lower_red.skin"
				Com_sprintf( filename, length, "models/players/%s/%s_%s.%s", modelName, base, team, ext );
			}
		}
		else {
			if ( i == 0 && teamName && *teamName ) {
				//								"models/players/james/stroggs/lower_lily.skin"
				Com_sprintf( filename, length, "models/players/%s/%s%s_%s.%s", modelName, teamName, base, skinName, ext );
			}
			else {
				//								"models/players/james/lower_lily.skin"
				Com_sprintf( filename, length, "models/players/%s/%s_%s.%s", modelName, base, skinName, ext );
			}
		}
		if ( CG_FileExists( filename ) ) {
			return qtrue;
		}
		if ( !teamName || !*teamName ) {
			break;
		}
	}

	return qfalse;
}

/*
==========================
CG_FindClientHeadFile
==========================
*/
static qboolean	CG_FindClientHeadFile( char *filename, int length, clientInfo_t *ci, const char *teamName, const char *headModelName, const char *headSkinName, const char *base, const char *ext ) {
	char *team, *headsFolder;
	int i;

	if ( cgs.gametype >= GT_TEAM ) {
		switch ( ci->team ) {
			case TEAM_BLUE: {
				team = "blue";
				break;
			}
			default: {
				team = "red";
				break;
			}
		}
	}
	else {
		team = "default";
	}

	if ( headModelName[0] == '*' ) {
		headsFolder = "heads/";
		headModelName++;
	}
	else {
		headsFolder = "";
	}
	while(1) {
		for ( i = 0; i < 2; i++ ) {
			if ( i == 0 && teamName && *teamName ) {
				Com_sprintf( filename, length, "models/players/%s%s/%s/%s%s_%s.%s", headsFolder, headModelName, headSkinName, teamName, base, team, ext );
			}
			else {
				Com_sprintf( filename, length, "models/players/%s%s/%s/%s_%s.%s", headsFolder, headModelName, headSkinName, base, team, ext );
			}

			if (Q_stricmpn(ext, "$image", 6) == 0) {
				COM_StripExtension(filename, filename, length);
 				if (trap_R_RegisterShaderNoMip(filename)) {
					return qtrue;
				}
			} else if ( CG_FileExists( filename ) ) {
				return qtrue;
			}

			if ( cgs.gametype >= GT_TEAM ) {
				if ( i == 0 &&  teamName && *teamName ) {
					Com_sprintf( filename, length, "models/players/%s%s/%s%s_%s.%s", headsFolder, headModelName, teamName, base, team, ext );
				}
				else {
					Com_sprintf( filename, length, "models/players/%s%s/%s_%s.%s", headsFolder, headModelName, base, team, ext );
				}
			}
			else {
				if ( i == 0 && teamName && *teamName ) {
					Com_sprintf( filename, length, "models/players/%s%s/%s%s_%s.%s", headsFolder, headModelName, teamName, base, headSkinName, ext );
				}
				else {
					Com_sprintf( filename, length, "models/players/%s%s/%s_%s.%s", headsFolder, headModelName, base, headSkinName, ext );
				}
			}

			if (Q_stricmpn(ext, "$image", 6) == 0) {
				COM_StripExtension(filename, filename, length);
 				if (trap_R_RegisterShaderNoMip(filename)) {
					return qtrue;
				}
			} else if ( CG_FileExists( filename ) ) {
				return qtrue;
			}

			if ( !teamName || !*teamName ) {
				break;
			}
		}
		// if tried the heads folder first
		if ( headsFolder[0] ) {
			break;
		}
		headsFolder = "heads/";
	}

	return qfalse;
}

/*
==========================
CG_RegisterClientSkin
==========================
*/
static qboolean	CG_RegisterClientSkin( clientInfo_t *ci, const char *teamName, const char *modelName, const char *skinName, const char *headModelName, const char *headSkinName ) {
	char filename[MAX_QPATH];

#ifdef IOQ3ZTM // BONES
	// single model player has single skin
	if (ci->playerModel) {
		if ( CG_FindClientModelFile( filename, sizeof(filename), ci, teamName, modelName, skinName, "player", "skin" ) ) {
			ci->playerSkin = trap_R_RegisterSkin( filename );
		}
		if (!ci->playerSkin) {
			Com_Printf( "Player skin load failure: %s\n", filename );
			return qfalse;
		}

		return qtrue;
	}
#endif

	/*
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/%slower_%s.skin", modelName, teamName, skinName );
	ci->legsSkin = trap_R_RegisterSkin( filename );
	if (!ci->legsSkin) {
		Com_Printf( "Leg skin load failure: %s\n", filename );
	}

	Com_sprintf( filename, sizeof( filename ), "models/players/%s/%supper_%s.skin", modelName, teamName, skinName );
	ci->torsoSkin = trap_R_RegisterSkin( filename );
	if (!ci->torsoSkin) {
		Com_Printf( "Torso skin load failure: %s\n", filename );
	}
	*/
	if ( CG_FindClientModelFile( filename, sizeof(filename), ci, teamName, modelName, skinName, "lower", "skin" ) ) {
		ci->legsSkin = trap_R_RegisterSkin( filename );
	}
	if (!ci->legsSkin) {
		Com_Printf( "Leg skin load failure: %s\n", filename );
	}

	if ( CG_FindClientModelFile( filename, sizeof(filename), ci, teamName, modelName, skinName, "upper", "skin" ) ) {
		ci->torsoSkin = trap_R_RegisterSkin( filename );
	}
	if (!ci->torsoSkin) {
		Com_Printf( "Torso skin load failure: %s\n", filename );
	}

	if ( CG_FindClientHeadFile( filename, sizeof(filename), ci, teamName, headModelName, headSkinName, "head", "skin" ) ) {
		ci->headSkin = trap_R_RegisterSkin( filename );
	}
	if (!ci->headSkin) {
		Com_Printf( "Head skin load failure: %s\n", filename );
	}

	// if any skins failed to load
	if ( !ci->legsSkin || !ci->torsoSkin || !ci->headSkin ) {
		return qfalse;
	}
	return qtrue;
}

/*
==========================
CG_RegisterClientModelname
==========================
*/
static qboolean CG_RegisterClientModelname( clientInfo_t *ci, const char *modelName, const char *skinName, const char *headModelName, const char *headSkinName, const char *teamName ) {
	char	filename[MAX_QPATH*2];
	const char		*headName;
	char newTeamName[MAX_QPATH*2];

	if ( headModelName[0] == '\0' ) {
		headName = modelName;
	}
	else {
		headName = headModelName;
	}

#ifdef IOQ3ZTM // BONES
	// Try loading single model player
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/player.iqm", modelName );
	ci->playerModel = trap_R_RegisterModel( filename );

	// Try loading multimodel player
	if (!ci->playerModel) {
#endif
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/lower.md3", modelName );
	ci->legsModel = trap_R_RegisterModel( filename );
	if ( !ci->legsModel ) {
		Com_Printf( "Failed to load model file %s\n", filename );
		return qfalse;
	}

	Com_sprintf( filename, sizeof( filename ), "models/players/%s/upper.md3", modelName );
	ci->torsoModel = trap_R_RegisterModel( filename );
	if ( !ci->torsoModel ) {
		Com_Printf( "Failed to load model file %s\n", filename );
		return qfalse;
	}

	if( headName[0] == '*' ) {
		Com_sprintf( filename, sizeof( filename ), "models/players/heads/%s/%s.md3", &headModelName[1], &headModelName[1] );
	}
	else {
		Com_sprintf( filename, sizeof( filename ), "models/players/%s/head.md3", headName );
	}
	ci->headModel = trap_R_RegisterModel( filename );
	// if the head model could not be found and we didn't load from the heads folder try to load from there
	if ( !ci->headModel && headName[0] != '*' ) {
		Com_sprintf( filename, sizeof( filename ), "models/players/heads/%s/%s.md3", headModelName, headModelName );
		ci->headModel = trap_R_RegisterModel( filename );
	}
	if ( !ci->headModel ) {
		Com_Printf( "Failed to load model file %s\n", filename );
		return qfalse;
	}
#ifdef IOQ3ZTM // BONES
	}
#endif

	// if any skins failed to load, return failure
	if ( !CG_RegisterClientSkin( ci, teamName, modelName, skinName, headName, headSkinName ) ) {
		if ( teamName && *teamName) {
			Com_Printf( "Failed to load skin file: %s : %s : %s, %s : %s\n", teamName, modelName, skinName, headName, headSkinName );
			if( ci->team == TEAM_BLUE ) {
				Com_sprintf(newTeamName, sizeof(newTeamName), "%s/", DEFAULT_BLUETEAM_NAME);
			}
			else {
				Com_sprintf(newTeamName, sizeof(newTeamName), "%s/", DEFAULT_REDTEAM_NAME);
			}
			if ( !CG_RegisterClientSkin( ci, newTeamName, modelName, skinName, headName, headSkinName ) ) {
				Com_Printf( "Failed to load skin file: %s : %s : %s, %s : %s\n", newTeamName, modelName, skinName, headName, headSkinName );
				return qfalse;
			}
		} else {
			Com_Printf( "Failed to load skin file: %s : %s, %s : %s\n", modelName, skinName, headName, headSkinName );
			return qfalse;
		}
	}

	// load the animations
#ifdef TA_PLAYERSYS
	if (!BG_LoadPlayerCFGFile(&ci->playercfg, modelName, headName)) {
		return qfalse;
	}
#else
	Com_sprintf( filename, sizeof( filename ), "models/players/%s/animation.cfg", modelName );
	if ( !CG_ParseAnimationFile( filename, ci ) ) {
		Com_Printf( "Failed to load animation file %s\n", filename );
		return qfalse;
	}
#endif

	if ( CG_FindClientHeadFile( filename, sizeof(filename), ci, teamName, headName, headSkinName, "icon", "$image" ) ) {
		ci->modelIcon = trap_R_RegisterShaderNoMip( filename );
	}

	if ( !ci->modelIcon ) {
		return qfalse;
	}

	return qtrue;
}

/*
====================
CG_ColorFromString
====================
*/
static void CG_ColorFromString( const char *v, vec3_t color ) {
	int val;

	VectorClear( color );

	val = atoi( v );

#ifdef TA_DATA // MORE_COLOR_EFFECTS
	switch (val)
	{
		case 1: // blue
		case 2: // green
		case 3: // cyen
		case 4: // red
		case 5: // magenta
		case 6: // yellow
		case 7: // white
			if ( val & 1 ) {
				color[2] = 1.0f;
			}
			if ( val & 2 ) {
				color[1] = 1.0f;
			}
			if ( val & 4 ) {
				color[0] = 1.0f;
			}
			break;

		case 8: // orange
			VectorSet( color, 1, 0.5f, 0 );
			break;
		case 9: // Lime
			VectorSet( color, 0.5f, 1, 0 );
			break;
		case 10: // Vivid green
			VectorSet( color, 0, 1, 0.5f );
			break;
		case 11: // light blue
			VectorSet( color, 0, 0.5f, 1 );
			break;
		case 12: // purple
			VectorSet( color, 0.5f, 0, 1 );
			break;
		case 13: // pink
			VectorSet( color, 1, 0, 0.5f );
			break;

		default: // fall back to white
			VectorSet( color, 1, 1, 1 );
			break;
	}
#else
	if ( val < 1 || val > 7 ) {
		VectorSet( color, 1, 1, 1 );
		return;
	}

	if ( val & 1 ) {
		color[2] = 1.0f;
	}
	if ( val & 2 ) {
		color[1] = 1.0f;
	}
	if ( val & 4 ) {
		color[0] = 1.0f;
	}
#endif
}

/*
===================
CG_LoadClientInfo

Load it now, taking the disk hits.
This will usually be deferred to a safe time
===================
*/
static void CG_LoadClientInfo( int clientNum, clientInfo_t *ci ) {
	const char	*dir, *fallback;
	int			i, modelloaded;
	const char	*s;
	char		teamname[MAX_QPATH];
	gender_t	gender;

	teamname[0] = 0;
#if defined MISSIONPACK || defined IOQ3ZTM // Support MissionPack players.
	if( cgs.gametype >= GT_TEAM) {
		if( ci->team == TEAM_BLUE ) {
			Q_strncpyz(teamname, cg_blueTeamName.string, sizeof(teamname) );
		} else {
			Q_strncpyz(teamname, cg_redTeamName.string, sizeof(teamname) );
		}
	}
	if( teamname[0] ) {
		strcat( teamname, "/" );
	}
#endif
	modelloaded = qtrue;
#ifdef IOQ3ZTM // Support MissionPack players in Q3 and Q3 players in MissionPack.
	// Try to loading teamname, for Team Arena players.
	if ( !CG_RegisterClientModelname( ci, ci->modelName, ci->skinName, ci->headModelName, ci->headSkinName, teamname ) ) {
		if (cgs.gametype < GT_TEAM)
		{
			// in non-team, try loading with teamname, for Team Arena players
			if( ci->team == TEAM_BLUE ) {
				Q_strncpyz(teamname, cg_blueTeamName.string, sizeof(teamname) );
			} else {
				Q_strncpyz(teamname, cg_redTeamName.string, sizeof(teamname) );
			}
		}
		else
		{
			// in teamplay, try loading with no teamname, for Q3 players.
			teamname[0] = 0;
		}
		if (!CG_RegisterClientModelname( ci, ci->modelName, ci->skinName, ci->headModelName, ci->headSkinName, teamname ) )
		{
			if (cg_buildScript.integer ) {
				CG_Error( "CG_RegisterClientModelname( %s, %s, %s, %s %s ) failed", ci->modelName, ci->skinName, ci->headModelName, ci->headSkinName, teamname );
			}

			// fall back to default team name
			if( cgs.gametype >= GT_TEAM) {
				// keep skin name
				if( ci->team == TEAM_BLUE ) {
					Q_strncpyz(teamname, DEFAULT_BLUETEAM_NAME, sizeof(teamname) );
				} else {
					Q_strncpyz(teamname, DEFAULT_REDTEAM_NAME, sizeof(teamname) );
				}
				if ( !CG_RegisterClientModelname( ci, DEFAULT_TEAM_MODEL, ci->skinName, DEFAULT_TEAM_HEAD, ci->skinName, teamname ) ) {
					CG_Error( "DEFAULT_TEAM_MODEL / skin (%s/%s) failed to register", DEFAULT_TEAM_MODEL, ci->skinName );
				}
			} else {
				if ( !CG_RegisterClientModelname( ci, DEFAULT_MODEL, "default", DEFAULT_MODEL, "default", teamname ) ) {
					CG_Error( "DEFAULT_MODEL (%s) failed to register", DEFAULT_MODEL );
				}
			}
			modelloaded = qfalse;
		}
	}
#else
	if ( !CG_RegisterClientModelname( ci, ci->modelName, ci->skinName, ci->headModelName, ci->headSkinName, teamname ) ) {
		if ( cg_buildScript.integer ) {
			CG_Error( "CG_RegisterClientModelname( %s, %s, %s, %s %s ) failed", ci->modelName, ci->skinName, ci->headModelName, ci->headSkinName, teamname );
		}

		// fall back to default team name
		if( cgs.gametype >= GT_TEAM) {
			// keep skin name
			if( ci->team == TEAM_BLUE ) {
				Q_strncpyz(teamname, DEFAULT_BLUETEAM_NAME, sizeof(teamname) );
			} else {
				Q_strncpyz(teamname, DEFAULT_REDTEAM_NAME, sizeof(teamname) );
			}
			if ( !CG_RegisterClientModelname( ci, DEFAULT_TEAM_MODEL, ci->skinName, DEFAULT_TEAM_HEAD, ci->skinName, teamname ) ) {
				CG_Error( "DEFAULT_TEAM_MODEL / skin (%s/%s) failed to register", DEFAULT_TEAM_MODEL, ci->skinName );
			}
		} else {
			if ( !CG_RegisterClientModelname( ci, DEFAULT_MODEL, "default", DEFAULT_HEAD, "default", teamname ) ) {
				CG_Error( "DEFAULT_MODEL (%s) failed to register", DEFAULT_MODEL );
			}
		}
		modelloaded = qfalse;
	}
#endif

#ifdef TA_PLAYERSYS
	CG_ColorFromString( va("%d", ci->playercfg.prefcolor2), ci->prefcolor2 );
#endif

#if defined TA_PLAYERSYS && defined TA_WEAPSYS // DEFAULT_DEFAULT_WEAPON
	// If it is the local client update default weapon.
	for (i = 0; i < MAX_SPLITVIEW; i++) {
		if (clientNum == cg.localClients[i].predictedPlayerState.clientNum) {
			cg.localClients[i].predictedPlayerState.stats[STAT_DEFAULTWEAPON] = cgs.clientinfo[clientNum].playercfg.default_weapon;
		}
	}
#endif

#ifdef TA_WEAPSYS
	ci->tagInfo = 0;
#else
	ci->newAnims = qfalse;
#endif
	if ( ci->torsoModel ) {
		orientation_t tag;
#ifdef TA_WEAPSYS
#ifdef TA_SUPPORTQ3
		if ( trap_R_LerpTag( &tag, ci->torsoModel, 0, 0, 1, "tag_weapon" ) ) {
			ci->tagInfo |= TI_TAG_WEAPON;
		}
		if ( trap_R_LerpTag( &tag, ci->torsoModel, 0, 0, 1, "tag_flag" ) ) {
			ci->tagInfo |= TI_TAG_FLAG;
		}
#endif
		if ( trap_R_LerpTag( &tag, ci->torsoModel, 0, 0, 1, "tag_hand_primary" ) ) {
			ci->tagInfo |= TI_TAG_HAND_PRIMARY;
		}
		if ( trap_R_LerpTag( &tag, ci->torsoModel, 0, 0, 1, "tag_hand_secondary" ) ) {
			ci->tagInfo |= TI_TAG_HAND_SECONDARY;
		}
		if ( trap_R_LerpTag( &tag, ci->torsoModel, 0, 0, 1, "tag_wp_away_primary" ) ) {
			ci->tagInfo |= TI_TAG_WP_AWAY_PRIMARY;
		}
		if ( trap_R_LerpTag( &tag, ci->torsoModel, 0, 0, 1, "tag_wp_away_secondary" ) ) {
			ci->tagInfo |= TI_TAG_WP_AWAY_SECONDARY;
		}
#else
		// if the torso model has the "tag_flag"
		if ( trap_R_LerpTag( &tag, ci->torsoModel, 0, 0, 1, "tag_flag" ) ) {
			ci->newAnims = qtrue;
		}
#endif
	}

	// sounds
#ifdef TA_PLAYERSYS // SOUNDPATH
	dir = ci->playercfg.soundpath;
	gender = ci->playercfg.gender;
#else
	dir = ci->modelName;
	gender = ci->gender;
#endif
	if (cgs.gametype >= GT_TEAM) {
		fallback = (gender == GENDER_FEMALE) ? DEFAULT_TEAM_MODEL_FEMALE : DEFAULT_TEAM_MODEL_MALE;
	} else {
		fallback = (gender == GENDER_FEMALE) ? DEFAULT_MODEL_FEMALE : DEFAULT_MODEL_MALE;
	}

	for ( i = 0 ; i < MAX_CUSTOM_SOUNDS ; i++ ) {
		s = cg_customSoundNames[i];
		if ( !s ) {
			break;
		}
		ci->sounds[i] = 0;
		// if the model didn't load use the sounds of the default model
		if (modelloaded) {
			ci->sounds[i] = trap_S_RegisterSound( va("sound/player/%s/%s", dir, s + 1), qfalse );
		}
		if ( !ci->sounds[i] ) {
			ci->sounds[i] = trap_S_RegisterSound( va("sound/player/%s/%s", fallback, s + 1), qfalse );
		}
#ifdef IOQ3ZTM // MORE_PLAYER_SOUNDS
		if ( !ci->sounds[i] ) {
			ci->sounds[i] = trap_S_RegisterSound( va("sound/player/%s", s + 1), qfalse );
		}
#endif
	}

	ci->deferred = qfalse;

	// reset any existing players and bodies, because they might be in bad
	// frames for this new model
	for ( i = 0 ; i < MAX_GENTITIES ; i++ ) {
		if ( cg_entities[i].currentState.clientNum == clientNum
			&& cg_entities[i].currentState.eType == ET_PLAYER ) {
			CG_ResetPlayerEntity( &cg_entities[i] );
		}
	}
}

/*
======================
CG_CopyClientInfoModel
======================
*/
static void CG_CopyClientInfoModel( clientInfo_t *from, clientInfo_t *to ) {
#ifndef TA_PLAYERSYS
	VectorCopy( from->headOffset, to->headOffset );
	to->footsteps = from->footsteps;
	to->gender = from->gender;
#endif

	to->legsModel = from->legsModel;
	to->legsSkin = from->legsSkin;
	to->torsoModel = from->torsoModel;
	to->torsoSkin = from->torsoSkin;
	to->headModel = from->headModel;
	to->headSkin = from->headSkin;
#ifdef IOQ3ZTM // BONES
	to->playerModel = from->playerModel;
	to->playerSkin = from->playerSkin;
#endif
	to->modelIcon = from->modelIcon;

#ifdef TA_WEAPSYS
	to->tagInfo = from->tagInfo;
#else
	to->newAnims = from->newAnims;
#endif

#ifdef TA_PLAYERSYS
	memcpy( &to->playercfg, &from->playercfg, sizeof( to->playercfg ) );
	VectorCopy(from->prefcolor2, to->prefcolor2);
#else
	memcpy( to->animations, from->animations, sizeof( to->animations ) );
#endif
	memcpy( to->sounds, from->sounds, sizeof( to->sounds ) );
}

/*
======================
CG_ScanForExistingClientInfo
======================
*/
static qboolean CG_ScanForExistingClientInfo( clientInfo_t *ci ) {
	int		i;
	clientInfo_t	*match;

	for ( i = 0 ; i < cgs.maxclients ; i++ ) {
		match = &cgs.clientinfo[ i ];
		if ( !match->infoValid ) {
			continue;
		}
		if ( match->deferred ) {
			continue;
		}
		if ( !Q_stricmp( ci->modelName, match->modelName )
			&& !Q_stricmp( ci->skinName, match->skinName )
			&& !Q_stricmp( ci->headModelName, match->headModelName )
			&& !Q_stricmp( ci->headSkinName, match->headSkinName ) 
#ifdef MISSIONPACK
			&& !Q_stricmp( ci->blueTeam, match->blueTeam ) 
			&& !Q_stricmp( ci->redTeam, match->redTeam )
#endif
			&& (cgs.gametype < GT_TEAM || ci->team == match->team) ) {
			// this clientinfo is identical, so use its handles

			ci->deferred = qfalse;

			CG_CopyClientInfoModel( match, ci );

			return qtrue;
		}
	}

	// nothing matches, so defer the load
	return qfalse;
}

/*
======================
CG_SetDeferredClientInfo

We aren't going to load it now, so grab some other
client's info to use until we have some spare time.
======================
*/
static void CG_SetDeferredClientInfo( int clientNum, clientInfo_t *ci ) {
	int		i;
	clientInfo_t	*match;

	// if someone else is already the same models and skins we
	// can just load the client info
	for ( i = 0 ; i < cgs.maxclients ; i++ ) {
		match = &cgs.clientinfo[ i ];
		if ( !match->infoValid || match->deferred ) {
			continue;
		}
		if ( Q_stricmp( ci->skinName, match->skinName ) ||
			 Q_stricmp( ci->modelName, match->modelName ) ||
//			 Q_stricmp( ci->headModelName, match->headModelName ) ||
//			 Q_stricmp( ci->headSkinName, match->headSkinName ) ||
			 (cgs.gametype >= GT_TEAM && ci->team != match->team) ) {
			continue;
		}
		// just load the real info cause it uses the same models and skins
		CG_LoadClientInfo( clientNum, ci );
		return;
	}

	// if we are in teamplay, only grab a model if the skin is correct
	if ( cgs.gametype >= GT_TEAM ) {
		for ( i = 0 ; i < cgs.maxclients ; i++ ) {
			match = &cgs.clientinfo[ i ];
			if ( !match->infoValid || match->deferred ) {
				continue;
			}
			if ( Q_stricmp( ci->skinName, match->skinName ) ||
				(cgs.gametype >= GT_TEAM && ci->team != match->team) ) {
				continue;
			}
			ci->deferred = qtrue;
			CG_CopyClientInfoModel( match, ci );
			return;
		}
		// load the full model, because we don't ever want to show
		// an improper team skin.  This will cause a hitch for the first
		// player, when the second enters.  Combat shouldn't be going on
		// yet, so it shouldn't matter
		CG_LoadClientInfo( clientNum, ci );
		return;
	}

	// find the first valid clientinfo and grab its stuff
	for ( i = 0 ; i < cgs.maxclients ; i++ ) {
		match = &cgs.clientinfo[ i ];
		if ( !match->infoValid ) {
			continue;
		}

		ci->deferred = qtrue;
		CG_CopyClientInfoModel( match, ci );
		return;
	}

	// we should never get here...
	CG_Printf( "CG_SetDeferredClientInfo: no valid clients!\n" );

	CG_LoadClientInfo( clientNum, ci );
}


/*
======================
CG_NewClientInfo
======================
*/
void CG_NewClientInfo( int clientNum ) {
	clientInfo_t *ci;
	clientInfo_t newInfo;
	const char	*configstring;
	const char	*v;
	char		*slash;

	ci = &cgs.clientinfo[clientNum];

	configstring = CG_ConfigString( clientNum + CS_PLAYERS );
	if ( !configstring[0] ) {
		memset( ci, 0, sizeof( *ci ) );
		return;		// player just left
	}

	// build into a temp buffer so the defer checks can use
	// the old value
	memset( &newInfo, 0, sizeof( newInfo ) );

	// isolate the player's name
	v = Info_ValueForKey(configstring, "n");
	Q_strncpyz( newInfo.name, v, sizeof( newInfo.name ) );

	// colors
	v = Info_ValueForKey( configstring, "c1" );
	CG_ColorFromString( v, newInfo.color1 );

	newInfo.c1RGBA[0] = 255 * newInfo.color1[0];
	newInfo.c1RGBA[1] = 255 * newInfo.color1[1];
	newInfo.c1RGBA[2] = 255 * newInfo.color1[2];
	newInfo.c1RGBA[3] = 255;

	v = Info_ValueForKey( configstring, "c2" );
	CG_ColorFromString( v, newInfo.color2 );

	newInfo.c2RGBA[0] = 255 * newInfo.color2[0];
	newInfo.c2RGBA[1] = 255 * newInfo.color2[1];
	newInfo.c2RGBA[2] = 255 * newInfo.color2[2];
	newInfo.c2RGBA[3] = 255;

	// bot skill
	v = Info_ValueForKey( configstring, "skill" );
	newInfo.botSkill = atoi( v );

	// handicap
	v = Info_ValueForKey( configstring, "hc" );
	newInfo.handicap = atoi( v );

	// wins
	v = Info_ValueForKey( configstring, "w" );
	newInfo.wins = atoi( v );

	// losses
	v = Info_ValueForKey( configstring, "l" );
	newInfo.losses = atoi( v );

	// team
	v = Info_ValueForKey( configstring, "t" );
	newInfo.team = atoi( v );

	// team task
	v = Info_ValueForKey( configstring, "tt" );
	newInfo.teamTask = atoi(v);

	// team leader
	v = Info_ValueForKey( configstring, "tl" );
	newInfo.teamLeader = atoi(v);

#ifdef MISSIONPACK
	Q_strncpyz(newInfo.redTeam, cg_redTeamName.string, MAX_TEAMNAME);

	Q_strncpyz(newInfo.blueTeam, cg_blueTeamName.string, MAX_TEAMNAME);
#endif

	// model
	v = Info_ValueForKey( configstring, "model" );
#ifndef TURTLEARENA // NO_CGFORCEMODLE
	if ( cg_forceModel.integer ) {
		// forcemodel makes everyone use a single model
		// to prevent load hitches
		char modelStr[MAX_QPATH];
		char *skin;

		if( cgs.gametype >= GT_TEAM ) {
			Q_strncpyz( newInfo.modelName, DEFAULT_TEAM_MODEL, sizeof( newInfo.modelName ) );
			Q_strncpyz( newInfo.skinName, "default", sizeof( newInfo.skinName ) );
		} else {
#ifdef TA_SP // SPMODEL
			if ( cg_singlePlayerActive.integer )
				trap_Cvar_VariableStringBuffer( "spmodel", modelStr, sizeof( modelStr ) );
			else
#endif
			trap_Cvar_VariableStringBuffer( "model", modelStr, sizeof( modelStr ) );
			if ( ( skin = strchr( modelStr, '/' ) ) == NULL) {
				skin = "default";
			} else {
				*skin++ = 0;
			}

			Q_strncpyz( newInfo.skinName, skin, sizeof( newInfo.skinName ) );
			Q_strncpyz( newInfo.modelName, modelStr, sizeof( newInfo.modelName ) );
		}

		if ( cgs.gametype >= GT_TEAM ) {
			// keep skin name
			slash = strchr( v, '/' );
			if ( slash ) {
				Q_strncpyz( newInfo.skinName, slash + 1, sizeof( newInfo.skinName ) );
			}
		}
	}
	else
#endif
	{
		Q_strncpyz( newInfo.modelName, v, sizeof( newInfo.modelName ) );

		slash = strchr( newInfo.modelName, '/' );
		if ( !slash ) {
			// modelName didn not include a skin name
			Q_strncpyz( newInfo.skinName, "default", sizeof( newInfo.skinName ) );
		} else {
			Q_strncpyz( newInfo.skinName, slash + 1, sizeof( newInfo.skinName ) );
			// truncate modelName
			*slash = 0;
		}
	}

	// head model
	v = Info_ValueForKey( configstring, "hmodel" );
#ifndef TURTLEARENA // NO_CGFORCEMODLE
	if ( cg_forceModel.integer ) {
		// forcemodel makes everyone use a single model
		// to prevent load hitches
		char modelStr[MAX_QPATH];
		char *skin;

		if( cgs.gametype >= GT_TEAM ) {
			Q_strncpyz( newInfo.headModelName, DEFAULT_TEAM_HEAD, sizeof( newInfo.headModelName ) );
			Q_strncpyz( newInfo.headSkinName, "default", sizeof( newInfo.headSkinName ) );
		} else {
			trap_Cvar_VariableStringBuffer( "headmodel", modelStr, sizeof( modelStr ) );
#ifdef IOQ3ZTM // BLANK_HEADMODEL
			if (!modelStr[0]) {
				trap_Cvar_VariableStringBuffer( "model", modelStr, sizeof( modelStr ) );
			}
#endif
			if ( ( skin = strchr( modelStr, '/' ) ) == NULL) {
				skin = "default";
			} else {
				*skin++ = 0;
			}

			Q_strncpyz( newInfo.headSkinName, skin, sizeof( newInfo.headSkinName ) );
			Q_strncpyz( newInfo.headModelName, modelStr, sizeof( newInfo.headModelName ) );
		}

		if ( cgs.gametype >= GT_TEAM ) {
			// keep skin name
			slash = strchr( v, '/' );
			if ( slash ) {
				Q_strncpyz( newInfo.headSkinName, slash + 1, sizeof( newInfo.headSkinName ) );
			}
		}
	}
	else
#endif
	{
		Q_strncpyz( newInfo.headModelName, v, sizeof( newInfo.headModelName ) );

		slash = strchr( newInfo.headModelName, '/' );
		if ( !slash ) {
			// modelName didn not include a skin name
			Q_strncpyz( newInfo.headSkinName, "default", sizeof( newInfo.headSkinName ) );
		} else {
			Q_strncpyz( newInfo.headSkinName, slash + 1, sizeof( newInfo.headSkinName ) );
			// truncate modelName
			*slash = 0;
		}
	}

	// scan for an existing clientinfo that matches this modelname
	// so we can avoid loading checks if possible
	if ( !CG_ScanForExistingClientInfo( &newInfo ) ) {
		qboolean	forceDefer;

		forceDefer = trap_MemoryRemaining() < 4000000;

		// if we are defering loads, just have it pick the first valid
		if ( forceDefer || (cg_deferPlayers.integer && !cg_buildScript.integer && !cg.loading ) ) {
			// keep whatever they had if it won't violate team skins
			CG_SetDeferredClientInfo( clientNum, &newInfo );
			// if we are low on memory, leave them with this model
			if ( forceDefer ) {
				CG_Printf( "Memory is low. Using deferred model.\n" );
				newInfo.deferred = qfalse;
			}
		} else {
			CG_LoadClientInfo( clientNum, &newInfo );
		}
	}

	// replace whatever was there with the new one
	newInfo.infoValid = qtrue;
	*ci = newInfo;
}



/*
======================
CG_LoadDeferredPlayers

Called each frame when a player is dead
and the scoreboard is up
so deferred players can be loaded
======================
*/
void CG_LoadDeferredPlayers( void ) {
	int		i;
	clientInfo_t	*ci;

	// scan for a deferred player to load
	for ( i = 0, ci = cgs.clientinfo ; i < cgs.maxclients ; i++, ci++ ) {
		if ( ci->infoValid && ci->deferred ) {
			// if we are low on memory, leave it deferred
			if ( trap_MemoryRemaining() < 4000000 ) {
				CG_Printf( "Memory is low. Using deferred model.\n" );
				ci->deferred = qfalse;
				continue;
			}
			CG_LoadClientInfo( i, ci );
//			break;
		}
	}
}

/*
=============================================================================

PLAYER ANIMATION

=============================================================================
*/

#ifndef IOQ3ZTM // LERP_FRAME_CLIENT_LESS // Moved to bg_misc.c
/*
===============
CG_SetLerpFrameAnimation

may include ANIM_TOGGLEBIT
===============
*/
static void CG_SetLerpFrameAnimation( clientInfo_t *ci, lerpFrame_t *lf, int newAnimation ) {
	animation_t	*anim;

	lf->animationNumber = newAnimation;
	newAnimation &= ~ANIM_TOGGLEBIT;

	if ( newAnimation < 0 ) {
		CG_Error( "Bad animation number: %i", newAnimation );
	}

#ifdef TA_PLAYERSYS
	anim = &ci->playercfg.animations[ newAnimation ];
#else
	anim = &ci->animations[ newAnimation ];
#endif

	lf->animation = anim;
	lf->animationTime = lf->frameTime + anim->initialLerp;

	if ( cg_debugAnim.integer ) {
		CG_Printf( "Anim: %i\n", newAnimation );
	}
}

/*
===============
CG_RunLerpFrame

Sets cg.snap, cg.oldFrame, and cg.backlerp
cg.time should be between oldFrameTime and frameTime after exit
===============
*/
static void CG_RunLerpFrame( clientInfo_t *ci, lerpFrame_t *lf, int newAnimation, float speedScale ) {
	int			f, numFrames;
	animation_t	*anim;

	// debugging tool to get no animations
	if ( cg_animSpeed.integer == 0 ) {
		lf->oldFrame = lf->frame = lf->backlerp = 0;
		return;
	}

	// see if the animation sequence is switching
	if ( newAnimation != lf->animationNumber || !lf->animation ) {
		CG_SetLerpFrameAnimation( ci, lf, newAnimation );
	}

	// if we have passed the current frame, move it to
	// oldFrame and calculate a new frame
	if ( cg.time >= lf->frameTime ) {
		lf->oldFrame = lf->frame;
		lf->oldFrameTime = lf->frameTime;

		// get the next frame based on the animation
		anim = lf->animation;
		if ( !anim->frameLerp ) {
			return;		// shouldn't happen
		}
		if ( cg.time < lf->animationTime ) {
			lf->frameTime = lf->animationTime;		// initial lerp
		} else {
			lf->frameTime = lf->oldFrameTime + anim->frameLerp;
		}
		f = ( lf->frameTime - lf->animationTime ) / anim->frameLerp;
		f *= speedScale;		// adjust for haste, etc

		numFrames = anim->numFrames;
		if (anim->flipflop) {
			numFrames *= 2;
		}
		if ( f >= numFrames ) {
			f -= numFrames;
			if ( anim->loopFrames ) {
				f %= anim->loopFrames;
				f += anim->numFrames - anim->loopFrames;
			} else {
				f = numFrames - 1;
				// the animation is stuck at the end, so it
				// can immediately transition to another sequence
				lf->frameTime = cg.time;
			}
		}
		if ( anim->reversed ) {
			lf->frame = anim->firstFrame + anim->numFrames - 1 - f;
		}
		else if (anim->flipflop && f>=anim->numFrames) {
			lf->frame = anim->firstFrame + anim->numFrames - 1 - (f%anim->numFrames);
		}
		else {
			lf->frame = anim->firstFrame + f;
		}
		if ( cg.time > lf->frameTime ) {
			lf->frameTime = cg.time;
			if ( cg_debugAnim.integer ) {
				CG_Printf( "Clamp lf->frameTime\n");
			}
		}
	}

	if ( lf->frameTime > cg.time + 200 ) {
		lf->frameTime = cg.time;
	}

	if ( lf->oldFrameTime > cg.time ) {
		lf->oldFrameTime = cg.time;
	}
	// calculate current lerp value
	if ( lf->frameTime == lf->oldFrameTime ) {
		lf->backlerp = 0;
	} else {
		lf->backlerp = 1.0 - (float)( cg.time - lf->oldFrameTime ) / ( lf->frameTime - lf->oldFrameTime );
	}
}


/*
===============
CG_ClearLerpFrame
===============
*/
static void CG_ClearLerpFrame( clientInfo_t *ci, lerpFrame_t *lf, int animationNumber ) {
	lf->frameTime = lf->oldFrameTime = cg.time;
	CG_SetLerpFrameAnimation( ci, lf, animationNumber );
	lf->oldFrame = lf->frame = lf->animation->firstFrame;
}
#endif // IOQ3ZTM // LERP_FRAME_CLIENT_LESS


/*
===============
CG_PlayerAnimation
===============
*/
static void CG_PlayerAnimation( centity_t *cent, int *legsOld, int *legs, float *legsBackLerp,
						int *torsoOld, int *torso, float *torsoBackLerp ) {
	clientInfo_t	*ci;
	int				clientNum;
	float			speedScale;

	clientNum = cent->currentState.clientNum;

	if ( cg_noPlayerAnims.integer ) {
		*legsOld = *legs = *torsoOld = *torso = 0;
		return;
	}

	if ( cent->currentState.powerups & ( 1 << PW_HASTE ) ) {
		speedScale = 1.5;
	} else {
		speedScale = 1;
	}

	ci = &cgs.clientinfo[ clientNum ];

	// do the shuffle turn frames locally
	if ( cent->pe.legs.yawing &&
#ifdef TA_WEAPSYS
		BG_PlayerStandAnim(&ci->playercfg, AP_LEGS, cent->currentState.legsAnim)
#else
		( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_IDLE
#endif
		)
	{
#ifdef IOQ3ZTM // LERP_FRAME_CLIENT_LESS
		BG_RunLerpFrame( &cent->pe.legs,
#ifdef TA_PLAYERSYS
			ci->playercfg.animations,
#else
			ci->animations,
#endif
			LEGS_TURN, cg.time, speedScale );
#else
		CG_RunLerpFrame( ci, &cent->pe.legs, LEGS_TURN, speedScale );
#endif
	} else {
#ifdef IOQ3ZTM // LERP_FRAME_CLIENT_LESS
		BG_RunLerpFrame( &cent->pe.legs,
#ifdef TA_PLAYERSYS
			ci->playercfg.animations,
#else
			ci->animations,
#endif
			cent->currentState.legsAnim, cg.time, speedScale );
#else
		CG_RunLerpFrame( ci, &cent->pe.legs, cent->currentState.legsAnim, speedScale );
#endif
	}

	*legsOld = cent->pe.legs.oldFrame;
	*legs = cent->pe.legs.frame;
	*legsBackLerp = cent->pe.legs.backlerp;

#ifdef IOQ3ZTM // LERP_FRAME_CLIENT_LESS
	BG_RunLerpFrame( &cent->pe.torso,
#ifdef TA_PLAYERSYS
		ci->playercfg.animations,
#else
		ci->animations,
#endif
		cent->currentState.torsoAnim, cg.time, speedScale );
#else
	CG_RunLerpFrame( ci, &cent->pe.torso, cent->currentState.torsoAnim, speedScale );
#endif

	*torsoOld = cent->pe.torso.oldFrame;
	*torso = cent->pe.torso.frame;
	*torsoBackLerp = cent->pe.torso.backlerp;
}

#ifdef IOQ3ZTM // BONES
/*
===============
CG_PlayerSkeleton
===============
*/
static void CG_PlayerSkeleton(clientInfo_t *ci, refEntity_t *legs, refEntity_t *torso,
							refEntity_t *head, refSkeleton_t *absSkeleton)
{
	refSkeleton_t skeleton;

	if (!ci->playerModel) {
		return;
	}

	if (trap_R_SetupPlayerSkeleton(ci->playerModel, &skeleton,
								legs->frame, legs->oldframe, legs->backlerp,
								torso->frame, torso->oldframe, torso->backlerp,
								head->frame, head->oldframe, head->backlerp))
	{
		// ZTM: TODO: Set torso and head axis in playerSkeleton.

		trap_R_MakeSkeletonAbsolute(&skeleton, absSkeleton);
	}
}
#endif

/*
=============================================================================

PLAYER ANGLES

=============================================================================
*/

#ifndef IOQ3ZTM // BG_SWING_ANGLES
/*
==================
CG_SwingAngles
==================
*/
void CG_SwingAngles( float destination, float swingTolerance, float clampTolerance,
					float speed, float *angle, qboolean *swinging ) {
	float	swing;
	float	move;
	float	scale;

	if ( !*swinging ) {
		// see if a swing should be started
		swing = AngleSubtract( *angle, destination );
		if ( swing > swingTolerance || swing < -swingTolerance ) {
			*swinging = qtrue;
		}
	}

	if ( !*swinging ) {
		return;
	}
	
	// modify the speed depending on the delta
	// so it doesn't seem so linear
	swing = AngleSubtract( destination, *angle );
	scale = fabs( swing );
	if ( scale < swingTolerance * 0.5 ) {
		scale = 0.5;
	} else if ( scale < swingTolerance ) {
		scale = 1.0;
	} else {
		scale = 2.0;
	}

	// swing towards the destination angle
	if ( swing >= 0 ) {
		move = cg.frametime * scale * speed;
		if ( move >= swing ) {
			move = swing;
			*swinging = qfalse;
		}
		*angle = AngleMod( *angle + move );
	} else if ( swing < 0 ) {
		move = cg.frametime * scale * -speed;
		if ( move <= swing ) {
			move = swing;
			*swinging = qfalse;
		}
		*angle = AngleMod( *angle + move );
	}

	// clamp to no more than tolerance
	swing = AngleSubtract( destination, *angle );
	if ( swing > clampTolerance ) {
		*angle = AngleMod( destination - (clampTolerance - 1) );
	} else if ( swing < -clampTolerance ) {
		*angle = AngleMod( destination + (clampTolerance - 1) );
	}
}
#endif

/*
=================
CG_AddPainTwitch
=================
*/
static void CG_AddPainTwitch( centity_t *cent, vec3_t torsoAngles ) {
	int		t;
	float	f;

	t = cg.time - cent->pe.painTime;
	if ( t >= PAIN_TWITCH_TIME ) {
		return;
	}

	f = 1.0 - (float)t / PAIN_TWITCH_TIME;

#if 0 // #ifdef TA_MISC // ZTM: TEST
	if (cent->currentState.clientNum == cg.cur_lc->predictedPlayerEntity.currentState.clientNum)
	{
		Com_Printf("DEBUG: damageYaw=%d\n", cg.cur_lc->predictedPlayerState.damageYaw);

		//torsoAngles[PITCH] += cg.cur_lc->predictedPlayerState.damagePitch * f;
		torsoAngles[YAW] -= cg.cur_lc->predictedPlayerState.damageYaw * f;
	}
	else
#endif
	if ( cent->pe.painDirection ) {
		torsoAngles[ROLL] += 20 * f;
	} else {
		torsoAngles[ROLL] -= 20 * f;
	}
}


/*
===============
CG_PlayerAngles

Handles seperate torso motion

  legs pivot based on direction of movement

  head always looks exactly at cent->lerpAngles

  if motion < 20 degrees, show in head only
  if < 45 degrees, also show in torso
===============
*/
static void CG_PlayerAngles( centity_t *cent, vec3_t legs[3], vec3_t torso[3], vec3_t head[3] ) {
	vec3_t		legsAngles, torsoAngles, headAngles;
	float		dest;
	static	int	movementOffsets[8] = { 0, 22, 45, -22, 0, 22, -45, -22 };
	vec3_t		velocity;
	float		speed;
	int			dir, clientNum;
	clientInfo_t	*ci;

	VectorCopy( cent->lerpAngles, headAngles );
	headAngles[YAW] = AngleMod( headAngles[YAW] );
	VectorClear( legsAngles );
	VectorClear( torsoAngles );

#ifdef IOQ3ZTM
	clientNum = cent->currentState.clientNum;
	if ( clientNum >= 0 && clientNum < MAX_CLIENTS ) {
		ci = &cgs.clientinfo[ clientNum ];
	} else {
		ci = NULL;
	}
#endif

	// --------- yaw -------------

	// allow yaw to drift a bit
#ifdef TA_WEAPSYS
	if (ci && (!BG_PlayerStandAnim(&ci->playercfg, AP_LEGS, cent->currentState.legsAnim)
		|| !BG_PlayerStandAnim(&ci->playercfg, AP_TORSO, cent->currentState.torsoAnim)))
#else
	if ( ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) != LEGS_IDLE 
		|| ((cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT) != TORSO_STAND 
		&& (cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT) != TORSO_STAND2))
#endif
	{
		// if not standing still, always point all in the same direction
		cent->pe.torso.yawing = qtrue;	// always center
		cent->pe.torso.pitching = qtrue;	// always center
		cent->pe.legs.yawing = qtrue;	// always center
	}

	// adjust legs for movement dir
	if ( cent->currentState.eFlags & EF_DEAD ) {
		// don't let dead bodies twitch
		dir = 0;
	} else {
		dir = cent->currentState.angles2[YAW];
		if ( dir < 0 || dir > 7 ) {
			CG_Error( "Bad player movement angle" );
		}
	}
	legsAngles[YAW] = headAngles[YAW] + movementOffsets[ dir ];
	torsoAngles[YAW] = headAngles[YAW] + 0.25 * movementOffsets[ dir ];

	// torso
#ifdef IOQ3ZTM // BG_SWING_ANGLES
	BG_SwingAngles( torsoAngles[YAW], 25, 90, cg_swingSpeed.value, &cent->pe.torso.yawAngle, &cent->pe.torso.yawing, cg.frametime );
	BG_SwingAngles( legsAngles[YAW], 40, 90, cg_swingSpeed.value, &cent->pe.legs.yawAngle, &cent->pe.legs.yawing, cg.frametime );
#else
	CG_SwingAngles( torsoAngles[YAW], 25, 90, cg_swingSpeed.value, &cent->pe.torso.yawAngle, &cent->pe.torso.yawing );
	CG_SwingAngles( legsAngles[YAW], 40, 90, cg_swingSpeed.value, &cent->pe.legs.yawAngle, &cent->pe.legs.yawing );
#endif

	torsoAngles[YAW] = cent->pe.torso.yawAngle;
	legsAngles[YAW] = cent->pe.legs.yawAngle;


	// --------- pitch -------------

#ifdef TA_PLAYERSYS // ZTM: If BOTH_* animation, don't have torso pitch
	if (( ci && ci->playercfg.fixedtorso ) || (cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT) == (cent->currentState.legsAnim & ~ANIM_TOGGLEBIT)) {
		dest = 0;
		headAngles[PITCH] = Com_Clamp( -65, 20, headAngles[PITCH] );
	} else
#endif
	// only show a fraction of the pitch angle in the torso
	if ( headAngles[PITCH] > 180 ) {
		dest = (-360 + headAngles[PITCH]) * 0.75f;
	} else {
		dest = headAngles[PITCH] * 0.75f;
	}
#ifdef IOQ3ZTM // BG_SWING_ANGLES
	BG_SwingAngles( dest, 15, 30, 0.1f, &cent->pe.torso.pitchAngle, &cent->pe.torso.pitching, cg.frametime );
#else
	CG_SwingAngles( dest, 15, 30, 0.1f, &cent->pe.torso.pitchAngle, &cent->pe.torso.pitching );
#endif
	torsoAngles[PITCH] = cent->pe.torso.pitchAngle;

#ifndef TA_PLAYERSYS
	//
#ifndef IOQ3ZTM
	clientNum = cent->currentState.clientNum;
	if ( clientNum >= 0 && clientNum < MAX_CLIENTS ) {
		ci = &cgs.clientinfo[ clientNum ];
#endif
		if ( ci->fixedtorso )
		{
			torsoAngles[PITCH] = 0.0f;
		}
#ifndef IOQ3ZTM
	}
#endif
#endif

	// --------- roll -------------


	// lean towards the direction of travel
	VectorCopy( cent->currentState.pos.trDelta, velocity );
	speed = VectorNormalize( velocity );
	if ( speed ) {
		vec3_t	axis[3];
		float	side;

		speed *= 0.05f;

		AnglesToAxis( legsAngles, axis );
		side = speed * DotProduct( velocity, axis[1] );
		legsAngles[ROLL] -= side;

		side = speed * DotProduct( velocity, axis[0] );
		legsAngles[PITCH] += side;
	}

	//
#ifndef IOQ3ZTM
	clientNum = cent->currentState.clientNum;
	if ( clientNum >= 0 && clientNum < MAX_CLIENTS ) {
		ci = &cgs.clientinfo[ clientNum ];
#endif
#ifdef TA_PLAYERSYS
		if ( ci && ci->playercfg.fixedlegs )
#else
		if ( ci->fixedlegs )
#endif
		{
			legsAngles[YAW] = torsoAngles[YAW];
			legsAngles[PITCH] = 0.0f;
			legsAngles[ROLL] = 0.0f;
		}
#ifndef IOQ3ZTM
	}
#endif

#ifdef TA_PLAYERSYS // LADDER
	if (cent->currentState.eFlags & EF_LADDER) {
		// Ladder dir, plaver legs should always face dir
		vectoangles(cent->currentState.origin2, legsAngles);
		// If BOTH_* animation, have torso face ladder too
		if ((cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT) == (cent->currentState.legsAnim & ~ANIM_TOGGLEBIT)) {
			VectorCopy(legsAngles, torsoAngles);
		}
	}
#endif

	// pain twitch
	CG_AddPainTwitch( cent, torsoAngles );

	// pull the angles back out of the hierarchial chain
	AnglesSubtract( headAngles, torsoAngles, headAngles );
	AnglesSubtract( torsoAngles, legsAngles, torsoAngles );
	AnglesToAxis( legsAngles, legs );
	AnglesToAxis( torsoAngles, torso );
	AnglesToAxis( headAngles, head );
}


//==========================================================================

/*
===============
CG_HasteTrail
===============
*/
static void CG_HasteTrail( centity_t *cent ) {
	localEntity_t	*smoke;
	vec3_t			origin;
	int				anim;

	if ( cent->trailTime > cg.time ) {
		return;
	}
#ifdef IOQ3ZTM // LERP_FRAME_CLIENT_LESS
	anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
#else
	anim = cent->pe.legs.animationNumber & ~ANIM_TOGGLEBIT;
#endif
	if ( anim != LEGS_RUN && anim != LEGS_BACK ) {
		return;
	}

	cent->trailTime += 100;
	if ( cent->trailTime < cg.time ) {
		cent->trailTime = cg.time;
	}

	VectorCopy( cent->lerpOrigin, origin );
	origin[2] -= 16;

	smoke = CG_SmokePuff( origin, vec3_origin, 
				  8, 
				  1, 1, 1, 1,
				  500, 
				  cg.time,
				  0,
				  0,
				  cgs.media.hastePuffShader );

	// use the optimized local entity add
	smoke->leType = LE_SCALE_FADE;
}

#ifdef MISSIONPACK
#ifdef IOQ3ZTM // BUBBLES
/*
==================
CG_SpawnBreathBubbles

Based on CG_BubbleTrail
==================
*/
void CG_SpawnBreathBubbles( vec3_t origin ) {
	int			i;
	int			numBubbles;
	float		rnd;
	qboolean	spawnedLarge; // Only one large bubble.

	spawnedLarge = qfalse;
	numBubbles = (int)(3 + random()*5);

	for ( i = 0; i < numBubbles; i++ ) {
		localEntity_t	*le;
		refEntity_t		*re;

		le = CG_AllocLocalEntity();
		le->leFlags = LEF_PUFF_DONT_SCALE;
		le->leType = LE_BUBBLE; // LE_MOVE_SCALE_FADE;
		le->startTime = cg.time;

		// Bubbles should make it to the surface
		le->endTime = cg.time + 8000 + random() * 250;

		le->lifeRate = 1.0 / ( le->endTime - le->startTime );

		re = &le->refEntity;
		re->shaderTime = cg.time / 1000.0f;

		re->reType = RT_SPRITE;
		re->rotation = 0;

		rnd = random();
		if (rnd > 0.9f && !spawnedLarge)
		{
			spawnedLarge = qtrue;
			re->radius = 6;
		}
		else
		{
			re->radius = 1 + rnd * 2;
		}

		re->customShader = cgs.media.waterBubbleShader;
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0xff;
		re->shaderRGBA[2] = 0xff;
		re->shaderRGBA[3] = 0xff;

		le->color[3] = 1.0;

		le->pos.trType = TR_LINEAR;
		le->pos.trTime = cg.time;

		VectorCopy( origin, le->pos.trBase );
		le->pos.trBase[0] += crandom();
		le->pos.trBase[1] += crandom();
		le->pos.trBase[2] += crandom();

		le->pos.trDelta[0] = crandom()*5;
		le->pos.trDelta[1] = crandom()*5;
		le->pos.trDelta[2] = 8 + random()*5;
	}
}
#endif

/*
===============
CG_BreathPuffs
===============
*/
static void CG_BreathPuffs( centity_t *cent, refEntity_t *head) {
	clientInfo_t *ci;
	vec3_t up, origin;
	int contents;

	ci = &cgs.clientinfo[ cent->currentState.number ];

#ifndef IOQ3ZTM // BUBBLES
	if (!cg_enableBreath.integer) {
		return;
	}
#endif
	if ( cent->currentState.number == cg.cur_ps->clientNum && !cg.cur_lc->renderingThirdPerson) {
		return;
	}
	if ( cent->currentState.eFlags & EF_DEAD ) {
		return;
	}
	contents = CG_PointContents( head->origin, 0 );
#ifndef IOQ3ZTM // BUBBLES
	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		return;
	}
#endif
	if ( ci->breathPuffTime > cg.time ) {
		return;
	}

	VectorSet( up, 0, 0, 8 );
	VectorMA(head->origin, 8, head->axis[0], origin);
	VectorMA(origin, -4, head->axis[2], origin);
#ifdef IOQ3ZTM // BUBBLES // ZTM: Bubbles under water! (and slime/lava?)
	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		CG_SpawnBreathBubbles(origin);
	}
	else //if (player is cold...)
	{
		if (cg_enableBreath.integer) {
#endif
	CG_SmokePuff( origin, up, 16, 1, 1, 1, 0.66f, 1500, cg.time, cg.time + 400, LEF_PUFF_DONT_SCALE, cgs.media.shotgunSmokePuffShader );
#ifdef IOQ3ZTM // BUBBLES
		}
	}
#endif
	ci->breathPuffTime = cg.time + 2000;
}

/*
===============
CG_DustTrail
===============
*/
static void CG_DustTrail( centity_t *cent ) {
	int				anim;
	vec3_t end, vel;
	trace_t tr;

	if (!cg_enableDust.integer)
		return;

	if ( cent->dustTrailTime > cg.time ) {
		return;
	}

#ifdef IOQ3ZTM // LERP_FRAME_CLIENT_LESS
	anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
#else
	anim = cent->pe.legs.animationNumber & ~ANIM_TOGGLEBIT;
#endif
	if ( anim != LEGS_LANDB && anim != LEGS_LAND ) {
		return;
	}

	cent->dustTrailTime += 40;
	if ( cent->dustTrailTime < cg.time ) {
		cent->dustTrailTime = cg.time;
	}

	VectorCopy(cent->currentState.pos.trBase, end);
	end[2] -= 64;
	CG_Trace( &tr, cent->currentState.pos.trBase, NULL, NULL, end, cent->currentState.number, MASK_PLAYERSOLID );

	if ( !(tr.surfaceFlags & SURF_DUST) )
		return;

	VectorCopy( cent->currentState.pos.trBase, end );
	end[2] -= 16;

	VectorSet(vel, 0, 0, -30);
	CG_SmokePuff( end, vel,
				  24,
				  .8f, .8f, 0.7f, 0.33f,
				  500,
				  cg.time,
				  0,
				  0,
				  cgs.media.dustPuffShader );
}

#endif

#ifndef TA_DATA // FLAG_MODEL
/*
===============
CG_TrailItem
===============
*/
#ifdef IOQ3ZTM // FLAG
static void CG_TrailItem( centity_t *cent, int itemIndex )
#else
static void CG_TrailItem( centity_t *cent, qhandle_t hModel )
#endif
{
	refEntity_t		ent;
	vec3_t			angles;
	vec3_t			axis[3];

#ifdef IOQ3ZTM // FLAG // Don't draw CTF flag for the holder in third person, blocks view.
	if (cent->currentState.clientNum == cg.cur_lc->predictedPlayerState.clientNum
		&& cg_thirdPerson[cg.cur_localClientNum].integer)
	{
		// if its the current player and their in third person view,
		//  don't draw the flag, it blocks their view.
		return;
	}
#endif

	VectorCopy( cent->lerpAngles, angles );
	angles[PITCH] = 0;
	angles[ROLL] = 0;
	AnglesToAxis( angles, axis );

	memset( &ent, 0, sizeof( ent ) );
	VectorMA( cent->lerpOrigin, -16, axis[0], ent.origin );
	ent.origin[2] += 16;
	angles[YAW] += 90;
	AnglesToAxis( angles, ent.axis );

#ifdef IOQ3ZTM // FLAG
	ent.hModel = cg_items[itemIndex].models[0];
	if (cg_items[itemIndex].skin)
	{
		ent.customSkin = cg_items[itemIndex].skin;
	}
#else
	ent.hModel = hModel;
#endif
	trap_R_AddRefEntityToScene( &ent );
}
#endif


/*
===============
CG_PlayerFlag
===============
*/
#ifdef IOQ3ZTM // FLAG
static void CG_PlayerFlag( centity_t *cent, powerup_t flagPower, refEntity_t *parent, refSkeleton_t *parentSkeleton )
#else
static void CG_PlayerFlag( centity_t *cent, qhandle_t hSkin, refEntity_t *parent, refSkeleton_t *parentSkeleton )
#endif
{
	clientInfo_t	*ci;
	refEntity_t	pole;
	refEntity_t	flag;
	vec3_t		angles, dir;
	int			legsAnim, flagAnim, updateangles;
	float		angle, d;
#ifdef TA_DATA // FLAG_MODEL
	qboolean	trailItem;
#endif
#ifdef IOQ3ZTM // FLAG
	bg_iteminfo_t	*item;
	int				itemIndex;
#ifndef TA_DATA // FLAG_MODEL
	qhandle_t		hSkin = 0;

	if (flagPower == PW_REDFLAG)
		hSkin = cgs.media.redFlagFlapSkin;
	else if (flagPower == PW_BLUEFLAG)
		hSkin = cgs.media.blueFlagFlapSkin;
	else
		hSkin = cgs.media.neutralFlagFlapSkin;
#endif

	item = BG_FindItemForPowerup(flagPower);
	itemIndex = BG_ItemNumForItem(item);
#endif
#ifdef TA_DATA // FLAG_MODEL
	trailItem = qtrue;
#endif

#ifdef IOQ3ZTM // FLAG
	ci = &cgs.clientinfo[ cent->currentState.clientNum ];
#endif

#ifndef TA_DATA // FLAG_MODEL
#ifdef IOQ3ZTM // FLAG
#ifdef TA_WEAPSYS
	if (!(ci->tagInfo & TI_TAG_HAND_SECONDARY)
#ifdef TA_SUPPORTQ3
		&& !(ci->tagInfo & TI_TAG_FLAG)
#endif
		)
#else
	if (!ci->newAnims)
#endif
	{
		CG_TrailItem( cent, itemIndex );
		return;
	}
#endif
#endif

	// show the flag pole model
	memset( &pole, 0, sizeof(pole) );
#ifdef TA_DATA // FLAG_MODEL
	pole.hModel = cg_items[itemIndex].models[0]; // cgs.media.flagPoleModel;
	pole.customSkin = cg_items[itemIndex].skin;
#else
	pole.hModel = cgs.media.flagPoleModel;
#endif
	VectorCopy( parent->lightingOrigin, pole.lightingOrigin );
	pole.shadowPlane = parent->shadowPlane;
	pole.renderfx = parent->renderfx;
#ifdef TA_WEAPSYS
	if (ci->tagInfo & TI_TAG_HAND_SECONDARY)
	{
		if (CG_PositionEntityOnTag( &pole, parent, parent->hModel, parentSkeleton, "tag_hand_secondary" ))
		{
#ifdef TA_DATA // FLAG_MODEL
			trailItem = qfalse;
#endif
		}
	}
#ifdef TA_SUPPORTQ3
	else if (ci->tagInfo & TI_TAG_FLAG)
	{
		if (CG_PositionEntityOnTag( &pole, parent, parent->hModel, parentSkeleton, "tag_flag" ))
		{
#ifdef TA_DATA // FLAG_MODEL
			trailItem = qfalse;
#endif
		}
	}
#endif
#elif defined IOQ3ZTM
	if (CG_PositionEntityOnTag( &pole, parent, parent->hModel, parentSkeleton, "tag_flag" ))
	{
#ifdef TA_DATA // FLAG_MODEL
		trailItem = qfalse;
#endif
	}
#else
	CG_PositionEntityOnTag( &pole, parent, parent->hModel, parentSkeleton, "tag_flag" );
#endif

#ifdef TA_DATA // FLAG_MODEL
	if (trailItem)
	{
		vec3_t			angles;
		vec3_t			axis[3];

#if 0 // #ifdef IOQ3ZTM // FLAG // Don't draw CTF flag for the holder in third person, blocks view.
						// ZTM: Could we make if transparent instead?
						//     RF_FORCE_ENT_ALPHA
		if (cent->currentState.clientNum == cg.cur_lc->predictedPlayerState.clientNum
			&& cg_thirdPerson.integer)
		{
			// if its the current player and their in third person view,
			//  don't draw the flag, it blocks their view.
			return;
		}
#endif

		VectorCopy( cent->lerpAngles, angles );
		angles[PITCH] = 0;
		angles[ROLL] = 0;
		AnglesToAxis( angles, axis );

		VectorMA( cent->lerpOrigin, -16, axis[0], pole.origin );
		pole.origin[2] += 16;
		//angles[YAW] += 90;
		AnglesToAxis( angles, pole.axis );
	}
#endif
	trap_R_AddRefEntityToScene( &pole );

	// show the flag model
	memset( &flag, 0, sizeof(flag) );
#ifdef TA_DATA // FLAG_MODEL
	flag.hModel = cg_items[itemIndex].models[1]; // cgs.media.flagFlapModel;
	flag.customSkin = cg_items[itemIndex].skin;
#else
	flag.hModel = cgs.media.flagFlapModel;
	flag.customSkin = hSkin;
#endif
	VectorCopy( parent->lightingOrigin, flag.lightingOrigin );
	flag.shadowPlane = parent->shadowPlane;
	flag.renderfx = parent->renderfx;

	VectorClear(angles);

	updateangles = qfalse;
	legsAnim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
	if(
#ifdef TA_WEAPSYS
		BG_PlayerStandAnim(&ci->playercfg, AP_LEGS, legsAnim)
#else
		legsAnim == LEGS_IDLE
#endif
		|| legsAnim == LEGS_IDLECR )
	{
		flagAnim = FLAG_STAND;
#ifdef IOQ3ZTM // ZTM: Always update flag angle.
		// ZTM: TODO: Have a idle timer to know if its been awhile since they moved (based on velocity), to NOT updateangles?
		updateangles = qtrue;
#endif
	} else if ( legsAnim == LEGS_WALK || legsAnim == LEGS_WALKCR ) {
		flagAnim = FLAG_STAND;
		updateangles = qtrue;
	} else {
		flagAnim = FLAG_RUN;
		updateangles = qtrue;
	}

	if ( updateangles ) {

		VectorCopy( cent->currentState.pos.trDelta, dir );
		// add gravity
		dir[2] += 100;
		VectorNormalize( dir );
		d = DotProduct(pole.axis[2], dir);
		// if there is enough movement orthogonal to the flag pole
		if (fabs(d) < 0.9) {
			//
			d = DotProduct(pole.axis[0], dir);
			if (d > 1.0f) {
				d = 1.0f;
			}
			else if (d < -1.0f) {
				d = -1.0f;
			}
			angle = Q_acos(d);

			d = DotProduct(pole.axis[1], dir);
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
			BG_SwingAngles( angles[YAW], 25, 90, 0.15f, &cent->pe.flag.yawAngle, &cent->pe.flag.yawing, cg.frametime );
#else
			CG_SwingAngles( angles[YAW], 25, 90, 0.15f, &cent->pe.flag.yawAngle, &cent->pe.flag.yawing );
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
	angles[YAW] = cent->pe.flag.yawAngle;
	// lerp the flag animation frames
#ifndef TA_WEAPSYS
	ci = &cgs.clientinfo[ cent->currentState.clientNum ];
#endif
#ifdef IOQ3ZTM // LERP_FRAME_CLIENT_LESS // FLAG_ANIMATIONS
	BG_RunLerpFrame( &cent->pe.flag, cgs.media.flag_animations, flagAnim, cg.time, 1 );
#else
	CG_RunLerpFrame( ci, &cent->pe.flag, flagAnim, 1 );
#endif
	flag.oldframe = cent->pe.flag.oldFrame;
	flag.frame = cent->pe.flag.frame;
	flag.backlerp = cent->pe.flag.backlerp;

	AnglesToAxis( angles, flag.axis );
	CG_PositionRotatedEntityOnTag( &flag, &pole, pole.hModel, NULL, "tag_flag" );

	trap_R_AddRefEntityToScene( &flag );
}


#ifdef MISSIONPACK_HARVESTER
/*
===============
CG_PlayerTokens
===============
*/
static void CG_PlayerTokens( centity_t *cent, int renderfx ) {
	int			tokens, i, j;
	float		angle;
	refEntity_t	ent;
	vec3_t		dir, origin;
	skulltrail_t *trail;
	trail = &cg.skulltrails[cent->currentState.number];
	tokens = cent->currentState.tokens;
	if ( !tokens ) {
		trail->numpositions = 0;
		return;
	}

	if ( tokens > MAX_SKULLTRAIL ) {
		tokens = MAX_SKULLTRAIL;
	}

	// add skulls if there are more than last time
	for (i = 0; i < tokens - trail->numpositions; i++) {
		for (j = trail->numpositions; j > 0; j--) {
			VectorCopy(trail->positions[j-1], trail->positions[j]);
		}
		VectorCopy(cent->lerpOrigin, trail->positions[0]);
	}
	trail->numpositions = tokens;

	// move all the skulls along the trail
	VectorCopy(cent->lerpOrigin, origin);
	for (i = 0; i < trail->numpositions; i++) {
		VectorSubtract(trail->positions[i], origin, dir);
		if (VectorNormalize(dir) > 30) {
			VectorMA(origin, 30, dir, trail->positions[i]);
		}
		VectorCopy(trail->positions[i], origin);
	}

	memset( &ent, 0, sizeof( ent ) );
	if( cgs.clientinfo[ cent->currentState.clientNum ].team == TEAM_BLUE ) {
		ent.hModel = cgs.media.redCubeModel;
	} else {
		ent.hModel = cgs.media.blueCubeModel;
	}
	ent.renderfx = renderfx;

	VectorCopy(cent->lerpOrigin, origin);
	for (i = 0; i < trail->numpositions; i++) {
		VectorSubtract(origin, trail->positions[i], ent.axis[0]);
		ent.axis[0][2] = 0;
		VectorNormalize(ent.axis[0]);
		VectorSet(ent.axis[2], 0, 0, 1);
		CrossProduct(ent.axis[0], ent.axis[2], ent.axis[1]);

		VectorCopy(trail->positions[i], ent.origin);
		angle = (((cg.time + 500 * MAX_SKULLTRAIL - 500 * i) / 16) & 255) * (M_PI * 2) / 255;
		ent.origin[2] += sin(angle) * 10;
		trap_R_AddRefEntityToScene( &ent );
		VectorCopy(trail->positions[i], origin);
	}
}
#endif


/*
===============
CG_PlayerPowerups
===============
*/
static void CG_PlayerPowerups( centity_t *cent, refEntity_t *torso, refSkeleton_t *torsoSkeleton ) {
	int		powerups;
#ifndef IOQ3ZTM // FLAG
	clientInfo_t	*ci;
#endif

	powerups = cent->currentState.powerups;
	if ( !powerups ) {
		return;
	}

#ifndef TURTLEARENA // POWERS
	// quad gives a dlight
	if ( powerups & ( 1 << PW_QUAD ) ) {
		trap_R_AddLightToScene( cent->lerpOrigin, 200 + (rand()&31), 0.2f, 0.2f, 1 );
	}
#endif

	// flight plays a looped sound
	if ( powerups & ( 1 << PW_FLIGHT ) ) {
		trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, cgs.media.flightSound );
	}

#ifndef IOQ3ZTM // FLAG
	ci = &cgs.clientinfo[ cent->currentState.clientNum ];
#endif
	// redflag
	if ( powerups & ( 1 << PW_REDFLAG ) ) {
#ifdef IOQ3ZTM // FLAG
		CG_PlayerFlag( cent, PW_REDFLAG, torso, torsoSkeleton );
#else
		if (ci->newAnims) {
			CG_PlayerFlag( cent, cgs.media.redFlagFlapSkin, torso, torsoSkeleton );
		} else {
			CG_TrailItem( cent, cgs.media.redFlagModel );
		}
#endif
		trap_R_AddLightToScene( cent->lerpOrigin, 200 + (rand()&31), 1.0, 0.2f, 0.2f );
	}

	// blueflag
	if ( powerups & ( 1 << PW_BLUEFLAG ) ) {
#ifdef IOQ3ZTM // FLAG
		CG_PlayerFlag( cent, PW_BLUEFLAG, torso, torsoSkeleton );
#else
		if (ci->newAnims){
			CG_PlayerFlag( cent, cgs.media.blueFlagFlapSkin, torso, torsoSkeleton );
		} else {
			CG_TrailItem( cent, cgs.media.blueFlagModel );
		}
#endif
		trap_R_AddLightToScene( cent->lerpOrigin, 200 + (rand()&31), 0.2f, 0.2f, 1.0 );
	}

	// neutralflag
	if ( powerups & ( 1 << PW_NEUTRALFLAG ) ) {
#ifdef IOQ3ZTM // FLAG
		CG_PlayerFlag( cent, PW_NEUTRALFLAG, torso, torsoSkeleton );
#else
		if (ci->newAnims) {
			CG_PlayerFlag( cent, cgs.media.neutralFlagFlapSkin, torso, torsoSkeleton );
		} else {
			CG_TrailItem( cent, cgs.media.neutralFlagModel );
		}
#endif
		trap_R_AddLightToScene( cent->lerpOrigin, 200 + (rand()&31), 1.0, 1.0, 1.0 );
	}

	// haste leaves smoke trails
	if ( powerups & ( 1 << PW_HASTE ) ) {
		CG_HasteTrail( cent );
	}
}


/*
===============
CG_PlayerFloatSprite

Float a sprite over the player's head
===============
*/
#ifdef IOQ3ZTM
static void CG_PlayerFloatSprite( vec3_t origin, int rf, qhandle_t shader )
#else
static void CG_PlayerFloatSprite( centity_t *cent, qhandle_t shader )
#endif
{
#ifndef IOQ3ZTM
	int				rf;
#endif
	refEntity_t		ent;

#ifndef IOQ3ZTM
	if ( cent->currentState.number == cg.cur_ps->clientNum && !cg.cur_lc->renderingThirdPerson ) {
		rf = RF_ONLY_MIRROR;
	} else {
		rf = 0;
	}
#endif

	memset( &ent, 0, sizeof( ent ) );
#ifdef IOQ3ZTM
	VectorCopy( origin, ent.origin );
#else
	VectorCopy( cent->lerpOrigin, ent.origin );
	ent.origin[2] += 48;
#endif
	ent.reType = RT_SPRITE;
	ent.customShader = shader;
	ent.radius = 10;
	ent.renderfx = rf;
	ent.shaderRGBA[0] = 255;
	ent.shaderRGBA[1] = 255;
	ent.shaderRGBA[2] = 255;
	ent.shaderRGBA[3] = 255;
	trap_R_AddRefEntityToScene( &ent );
}



/*
===============
CG_PlayerSprites

Float sprites over the player's head
===============
*/
static void CG_PlayerSprites( centity_t *cent
#ifdef IOQ3ZTM
	, refEntity_t *head
#endif
	)
{
	int		team;
#ifdef IOQ3ZTM
	int mirrorFlag;
	vec3_t origin;

	// Lets say the head is 8 units tall.
	VectorMA(head->origin, 8, head->axis[2], origin);

	// Place 16 units above top of head.
	origin[2] += 16;

	// Current client's team sprite should only be shown in mirrors.
	if ( cent->currentState.number == cg.cur_ps->clientNum )
	{
		// IOQ3ZTM // RENDER_FLAGS
		mirrorFlag = RF_ONLY_MIRROR;		// only show in mirrors
	} else {
		mirrorFlag = 0;
	}
#endif

	if ( cent->currentState.eFlags & EF_CONNECTION ) {
#ifdef IOQ3ZTM
		CG_PlayerFloatSprite( origin, 0, cgs.media.connectionShader );
#else
		CG_PlayerFloatSprite( cent, cgs.media.connectionShader );
#endif
		return;
	}

	if ( cent->currentState.eFlags & EF_TALK ) {
#ifdef IOQ3ZTM
		CG_PlayerFloatSprite( origin, 0, cgs.media.balloonShader );
#else
		CG_PlayerFloatSprite( cent, cgs.media.balloonShader );
#endif
		return;
	}

#ifdef IOQ3ZTM
	// Don't draw award if they are drawn on the HUD.
	if ( cent->currentState.number != cg.cur_ps->clientNum
		|| cg_draw2D.integer == 0 )
	{
#endif
#ifndef TURTLEARENA // AWARDS
	if ( cent->currentState.eFlags & EF_AWARD_IMPRESSIVE ) {
#ifdef IOQ3ZTM
		CG_PlayerFloatSprite( origin, 0, cgs.media.medalImpressive );
#else
		CG_PlayerFloatSprite( cent, cgs.media.medalImpressive );
#endif
		return;
	}

	if ( cent->currentState.eFlags & EF_AWARD_EXCELLENT ) {
#ifdef IOQ3ZTM
		CG_PlayerFloatSprite( origin, 0, cgs.media.medalExcellent );
#else
		CG_PlayerFloatSprite( cent, cgs.media.medalExcellent );
#endif
		return;
	}

	if ( cent->currentState.eFlags & EF_AWARD_GAUNTLET ) {
#ifdef IOQ3ZTM
		CG_PlayerFloatSprite( origin, 0, cgs.media.medalGauntlet );
#else
		CG_PlayerFloatSprite( cent, cgs.media.medalGauntlet );
#endif
		return;
	}
#endif

	if ( cent->currentState.eFlags & EF_AWARD_DEFEND ) {
#ifdef IOQ3ZTM
		CG_PlayerFloatSprite( origin, 0, cgs.media.medalDefend );
#else
		CG_PlayerFloatSprite( cent, cgs.media.medalDefend );
#endif
		return;
	}

	if ( cent->currentState.eFlags & EF_AWARD_ASSIST ) {
#ifdef IOQ3ZTM
		CG_PlayerFloatSprite( origin, 0, cgs.media.medalAssist );
#else
		CG_PlayerFloatSprite( cent, cgs.media.medalAssist );
#endif
		return;
	}

	if ( cent->currentState.eFlags & EF_AWARD_CAP ) {
#ifdef IOQ3ZTM
		CG_PlayerFloatSprite( origin, 0, cgs.media.medalCapture );
#else
		CG_PlayerFloatSprite( cent, cgs.media.medalCapture );
#endif
		return;
	}
#ifdef IOQ3ZTM
	}
#endif

#ifdef TURTLEARENA // LOCKON
	// Show local client's target marker over this client
#ifdef IOQ3ZTM
	if (cg.cur_ps->enemyEnt == cent->currentState.number)
	{
#ifdef IOQ3ZTM
		CG_PlayerFloatSprite( origin, 0, cgs.media.targetShader );
#else
		CG_PlayerFloatSprite( cent, cgs.media.targetShader );
#endif
	}
#endif
#endif

	team = cgs.clientinfo[ cent->currentState.clientNum ].team;
	if ( !(cent->currentState.eFlags & EF_DEAD) && 
#ifdef IOQ3ZTM // SHOW_TEAM_FRIENDS
		((cg.cur_ps->persistant[PERS_TEAM] == TEAM_SPECTATOR && cgs.media.blueFriendShader)
			|| cg.cur_ps->persistant[PERS_TEAM] == team) &&
#else
		cg.cur_ps->persistant[PERS_TEAM] == team &&
#endif
		cgs.gametype >= GT_TEAM) {
		if (cg_drawFriend.integer) {
#ifdef IOQ3ZTM // SHOW_TEAM_FRIENDS
			if (team == TEAM_BLUE && cgs.media.blueFriendShader)
			{
				CG_PlayerFloatSprite( origin, mirrorFlag, cgs.media.blueFriendShader );
			}
			else
			{
				CG_PlayerFloatSprite( origin, mirrorFlag, cgs.media.friendShader );
			}
#else
			CG_PlayerFloatSprite( cent, cgs.media.friendShader );
#endif
		}
		return;
	}
}

/*
===============
CG_PlayerShadow

Returns the Z component of the surface being shadowed

  should it return a full plane instead of a Z?
===============
*/
#define	SHADOW_DISTANCE		128
static qboolean CG_PlayerShadow( centity_t *cent, vec3_t start, float *shadowPlane ) {
	vec3_t		end, mins = {-8, -8, 0}, maxs = {8, 8, 2};
	trace_t		trace;
	float		alpha;

	*shadowPlane = 0;

	if ( cg_shadows.integer == 0 ) {
		return qfalse;
	}

	// no shadows when invisible
	if ( cent->currentState.powerups & ( 1 << PW_INVIS ) ) {
		return qfalse;
	}

	// send a trace down from the player to the ground
	VectorCopy( start, end );
	end[2] -= SHADOW_DISTANCE;

	trap_CM_BoxTrace( &trace, start, end, mins, maxs, 0, MASK_PLAYERSOLID );

	// no shadow if too high
	if ( trace.fraction == 1.0 || trace.startsolid || trace.allsolid ) {
		return qfalse;
	}

	*shadowPlane = trace.endpos[2] + 1;

	if ( cg_shadows.integer != 1 ) {	// no mark for stencil or projection shadows
		return qtrue;
	}

	// fade the shadow out with height
	alpha = 1.0 - trace.fraction;

#ifdef TURTLEARENA // POWERS
	if ( (cent->currentState.powerups & ( 1 << PW_FLASHING )) && cent->currentState.otherEntityNum2 > 0) {
		// Fade out shadow when dead body is fading out.
		alpha *= (float)cent->currentState.otherEntityNum2 / 128.0f;
	}
#endif

	// hack / FPE - bogus planes?
	//assert( DotProduct( trace.plane.normal, trace.plane.normal ) != 0.0f ) 

	// add the mark as a temporary, so it goes directly to the renderer
	// without taking a spot in the cg_marks array
	CG_ImpactMark( cgs.media.shadowMarkShader, trace.endpos, trace.plane.normal, 
		cent->pe.legs.yawAngle, alpha,alpha,alpha,1, qfalse, 24, qtrue );

	return qtrue;
}


/*
===============
CG_PlayerSplash

Draw a mark at the water surface
===============
*/
static void CG_PlayerSplash( centity_t *cent ) {
	vec3_t		start, end;
	trace_t		trace;
	int			contents;
	polyVert_t	verts[4];

	if ( !cg_shadows.integer ) {
		return;
	}

	VectorCopy( cent->lerpOrigin, end );
	end[2] -= 24;

	// if the feet aren't in liquid, don't make a mark
	// this won't handle moving water brushes, but they wouldn't draw right anyway...
	contents = CG_PointContents( end, 0 );
	if ( !( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) ) {
		return;
	}

	VectorCopy( cent->lerpOrigin, start );
	start[2] += 32;

	// if the head isn't out of liquid, don't make a mark
	contents = CG_PointContents( start, 0 );
	if ( contents & ( CONTENTS_SOLID | CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		return;
	}

	// trace down to find the surface
	trap_CM_BoxTrace( &trace, start, end, NULL, NULL, 0, ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) );

	if ( trace.fraction == 1.0 ) {
		return;
	}

	// create a mark polygon
	VectorCopy( trace.endpos, verts[0].xyz );
	verts[0].xyz[0] -= 32;
	verts[0].xyz[1] -= 32;
	verts[0].st[0] = 0;
	verts[0].st[1] = 0;
	verts[0].modulate[0] = 255;
	verts[0].modulate[1] = 255;
	verts[0].modulate[2] = 255;
	verts[0].modulate[3] = 255;

	VectorCopy( trace.endpos, verts[1].xyz );
	verts[1].xyz[0] -= 32;
	verts[1].xyz[1] += 32;
	verts[1].st[0] = 0;
	verts[1].st[1] = 1;
	verts[1].modulate[0] = 255;
	verts[1].modulate[1] = 255;
	verts[1].modulate[2] = 255;
	verts[1].modulate[3] = 255;

	VectorCopy( trace.endpos, verts[2].xyz );
	verts[2].xyz[0] += 32;
	verts[2].xyz[1] += 32;
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = 255;
	verts[2].modulate[1] = 255;
	verts[2].modulate[2] = 255;
	verts[2].modulate[3] = 255;

	VectorCopy( trace.endpos, verts[3].xyz );
	verts[3].xyz[0] += 32;
	verts[3].xyz[1] -= 32;
	verts[3].st[0] = 1;
	verts[3].st[1] = 0;
	verts[3].modulate[0] = 255;
	verts[3].modulate[1] = 255;
	verts[3].modulate[2] = 255;
	verts[3].modulate[3] = 255;

	trap_R_AddPolyToScene( cgs.media.wakeMarkShader, 4, verts );
}



#ifdef IOQ3ZTM // BONES
/*
===============
CG_AddRefEntityWithPowerups_CustomSkeleton

Adds a piece with modifications or duplications for powerups
Also called by CG_Missile for quad rockets, but nobody can tell...
===============
*/
void CG_AddRefEntityWithPowerups_CustomSkeleton( refEntity_t *ent, refSkeleton_t *skeleton, entityState_t *state, int team ) {
#ifdef TURTLEARENA // POWERS
	if ( state->powerups & ( 1 << PW_FLASHING ) ) {
		if (state->otherEntityNum2 > 0) {
			// Death fade out
			ent->renderfx |= RF_FORCE_ENT_ALPHA;
			ent->shaderRGBA[3] = state->otherEntityNum2;
		}
		trap_R_AddRefEntityToScene_CustomSkeleton( ent, skeleton );

		ent->customShader = cgs.media.playerTeleportShader;
		trap_R_AddRefEntityToScene_CustomSkeleton( ent, skeleton );
	} else
#endif
	if ( state->powerups & ( 1 << PW_INVIS ) ) {
		ent->customShader = cgs.media.invisShader;
		trap_R_AddRefEntityToScene_CustomSkeleton( ent, skeleton );
	} else {
		/*
		if ( state->eFlags & EF_KAMIKAZE ) {
			if (team == TEAM_BLUE)
				ent->customShader = cgs.media.blueKamikazeShader;
			else
				ent->customShader = cgs.media.redKamikazeShader;
			trap_R_AddRefEntityToScene_CustomSkeleton( ent, skeleton );
		}
		else {*/
			trap_R_AddRefEntityToScene_CustomSkeleton( ent, skeleton );
		//}

#ifndef TURTLEARENA // POWERS
		if ( state->powerups & ( 1 << PW_QUAD ) )
		{
			if (team == TEAM_RED)
				ent->customShader = cgs.media.redQuadShader;
			else
				ent->customShader = cgs.media.quadShader;
			trap_R_AddRefEntityToScene_CustomSkeleton( ent, skeleton );
		}
		if ( state->powerups & ( 1 << PW_REGEN ) ) {
			if ( ( ( cg.time / 100 ) % 10 ) == 1 ) {
				ent->customShader = cgs.media.regenShader;
				trap_R_AddRefEntityToScene_CustomSkeleton( ent, skeleton );
			}
		}
#endif
		if ( state->powerups & ( 1 << PW_BATTLESUIT ) ) {
			ent->customShader = cgs.media.battleSuitShader;
			trap_R_AddRefEntityToScene_CustomSkeleton( ent, skeleton );
		}
	}
}

/*
===============
CG_AddRefEntityWithPowerups
===============
*/
void CG_AddRefEntityWithPowerups( refEntity_t *ent, entityState_t *state, int team ) {
	CG_AddRefEntityWithPowerups_CustomSkeleton(ent, NULL, state, team);
}
#else
/*
===============
CG_AddRefEntityWithPowerups

Adds a piece with modifications or duplications for powerups
Also called by CG_Missile for quad rockets, but nobody can tell...
===============
*/
void CG_AddRefEntityWithPowerups( refEntity_t *ent, entityState_t *state, int team ) {
#ifdef TURTLEARENA // POWERS
	if ( state->powerups & ( 1 << PW_FLASHING ) ) {
		if (state->otherEntityNum2 > 0) {
			// Death fade out
			ent->renderfx |= RF_FORCE_ENT_ALPHA;
			ent->shaderRGBA[3] = state->otherEntityNum2;
		}
		trap_R_AddRefEntityToScene( ent );

		ent->customShader = cgs.media.playerTeleportShader;
		trap_R_AddRefEntityToScene( ent );
	} else
#endif
	if ( state->powerups & ( 1 << PW_INVIS ) ) {
		ent->customShader = cgs.media.invisShader;
		trap_R_AddRefEntityToScene( ent );
	} else {
		/*
		if ( state->eFlags & EF_KAMIKAZE ) {
			if (team == TEAM_BLUE)
				ent->customShader = cgs.media.blueKamikazeShader;
			else
				ent->customShader = cgs.media.redKamikazeShader;
			trap_R_AddRefEntityToScene( ent );
		}
		else {*/
			trap_R_AddRefEntityToScene( ent );
		//}

#ifndef TURTLEARENA // POWERS
		if ( state->powerups & ( 1 << PW_QUAD ) )
		{
			if (team == TEAM_RED)
				ent->customShader = cgs.media.redQuadShader;
			else
				ent->customShader = cgs.media.quadShader;
			trap_R_AddRefEntityToScene( ent );
		}
		if ( state->powerups & ( 1 << PW_REGEN ) ) {
			if ( ( ( cg.time / 100 ) % 10 ) == 1 ) {
				ent->customShader = cgs.media.regenShader;
				trap_R_AddRefEntityToScene( ent );
			}
		}
#endif
		if ( state->powerups & ( 1 << PW_BATTLESUIT ) ) {
			ent->customShader = cgs.media.battleSuitShader;
			trap_R_AddRefEntityToScene( ent );
		}
	}
}
#endif

#ifndef IOQ3ZTM // UNUSED
/*
=================
CG_LightVerts
=================
*/
int CG_LightVerts( vec3_t normal, int numVerts, polyVert_t *verts )
{
	int				i, j;
	float			incoming;
	vec3_t			ambientLight;
	vec3_t			lightDir;
	vec3_t			directedLight;

	trap_R_LightForPoint( verts[0].xyz, ambientLight, directedLight, lightDir );

	for (i = 0; i < numVerts; i++) {
		incoming = DotProduct (normal, lightDir);
		if ( incoming <= 0 ) {
			verts[i].modulate[0] = ambientLight[0];
			verts[i].modulate[1] = ambientLight[1];
			verts[i].modulate[2] = ambientLight[2];
			verts[i].modulate[3] = 255;
			continue;
		} 
		j = ( ambientLight[0] + incoming * directedLight[0] );
		if ( j > 255 ) {
			j = 255;
		}
		verts[i].modulate[0] = j;

		j = ( ambientLight[1] + incoming * directedLight[1] );
		if ( j > 255 ) {
			j = 255;
		}
		verts[i].modulate[1] = j;

		j = ( ambientLight[2] + incoming * directedLight[2] );
		if ( j > 255 ) {
			j = 255;
		}
		verts[i].modulate[2] = j;

		verts[i].modulate[3] = 255;
	}
	return qtrue;
}
#endif


/*
===============
CG_Player
===============
*/
void CG_Player( centity_t *cent ) {
	clientInfo_t	*ci;
	refEntity_t		legs;
	refEntity_t		torso;
	refEntity_t		head;
#ifdef IOQ3ZTM // BONES
	refSkeleton_t	skeleton;
#endif
	int				clientNum;
	int				renderfx;
	qboolean		shadow;
	float			shadowPlane;
	refEntity_t		shadowRef;
	vec3_t			shadowOrigin;
#ifdef TURTLEARENA // POWERS
	refEntity_t		powerup;
#endif
#ifdef MISSIONPACK
#ifndef TURTLEARENA // NO_KAMIKAZE_ITEM POWERS
	refEntity_t		skull;
	refEntity_t		powerup;
#endif
	int				t;
	float			c;
#ifndef TURTLEARENA // NO_KAMIKAZE_ITEM
	float			angle;
	vec3_t			dir, angles;
#else
	vec3_t			angles;
#endif
#endif

	// the client number is stored in clientNum.  It can't be derived
	// from the entity number, because a single client may have
	// multiple corpses on the level using the same clientinfo
	clientNum = cent->currentState.clientNum;
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		CG_Error( "Bad clientNum on player entity");
	}
	ci = &cgs.clientinfo[ clientNum ];

	// it is possible to see corpses from disconnected players that may
	// not have valid clientinfo
	if ( !ci->infoValid ) {
		return;
	}

	// get the player model information
	renderfx = 0;
	if ( cent->currentState.number == cg.cur_ps->clientNum) {
		if (!cg.cur_lc->renderingThirdPerson) {
			renderfx = RF_ONLY_MIRROR;
		} else {
			if (cg_cameraMode.integer) {
				return;
			}
		}
#ifdef TURTLEARENA // LOCKON
		// Show target marker for non-client entities.
#ifdef IOQ3ZTM
		if (cg.cur_ps->enemyEnt >= MAX_CLIENTS && cg.cur_ps->enemyEnt != ENTITYNUM_NONE)
		{
			vec3_t marker;
			
			VectorCopy(cg.cur_ps->enemyOrigin, marker);
			marker[2] += 40;
			CG_PlayerFloatSprite( marker, 0, cgs.media.targetShader );
		}
#endif
#endif
	}


	memset( &legs, 0, sizeof(legs) );
	memset( &torso, 0, sizeof(torso) );
	memset( &head, 0, sizeof(head) );
#ifdef IOQ3ZTM // BONES
	memset( &skeleton, 0, sizeof(skeleton) );
#endif

	// get the rotation information
	CG_PlayerAngles( cent, legs.axis, torso.axis, head.axis );
	
	// get the animation state (after rotation, to allow feet shuffle)
	CG_PlayerAnimation( cent, &legs.oldframe, &legs.frame, &legs.backlerp,
		 &torso.oldframe, &torso.frame, &torso.backlerp );

#ifdef IOQ3ZTM // BONES
	if (ci->playerModel) {
		// get skeleton
		CG_PlayerSkeleton(ci, &legs, &torso, &head, &skeleton);
	}
#endif

#ifndef IOQ3ZTM
	// add the talk baloon or disconnect icon
	CG_PlayerSprites( cent );
#endif

	// cast shadow from torso origin
	memcpy(&shadowRef, &torso, sizeof(shadowRef));
	VectorCopy(cent->lerpOrigin, legs.origin);

	// Shadow from the torso origin
#ifdef IOQ3ZTM // BONES
	if (ci->playerModel) {
		if (CG_PositionRotatedEntityOnTag(&shadowRef, &legs, ci->playerModel, &skeleton, "tag_torso")) {
			VectorCopy(shadowRef.origin, shadowOrigin);
		} else {
			VectorCopy(cent->lerpOrigin, shadowOrigin);
		}
	} else
#endif
	if (CG_PositionRotatedEntityOnTag(&shadowRef, &legs, ci->legsModel, NULL, "tag_torso")) {
		VectorCopy(shadowRef.origin, shadowOrigin);
	} else {
		VectorCopy(cent->lerpOrigin, shadowOrigin);
	}

	// add the shadow
	shadow = CG_PlayerShadow( cent, shadowOrigin, &shadowPlane );

	// add a water splash if partially in and out of water
	CG_PlayerSplash( cent );

	if ( cg_shadows.integer == 3 && shadow ) {
		renderfx |= RF_SHADOW_PLANE;
	}
	renderfx |= RF_LIGHTING_ORIGIN;			// use the same origin for all
#ifdef MISSIONPACK_HARVESTER
	if( cgs.gametype == GT_HARVESTER ) {
		CG_PlayerTokens( cent, renderfx );
	}
#endif

#ifdef IOQ3ZTM // BONES
	if (ci->playerModel) {
		//
		// add the player
		//
		legs.hModel = ci->playerModel;
		legs.customSkin = ci->playerSkin;
#ifdef IOQ3ZTM_NO_COMPAT // DAMAGE_SKINS
		legs.skinFraction = cent->currentState.skinFraction;
#endif

		VectorCopy( cent->lerpOrigin, legs.origin );

		VectorCopy( cent->lerpOrigin, legs.lightingOrigin );
		legs.shadowPlane = shadowPlane;
		legs.renderfx = renderfx;
		VectorCopy (legs.origin, legs.oldorigin);	// don't positionally lerp at all

		CG_AddRefEntityWithPowerups_CustomSkeleton( &legs, &skeleton, &cent->currentState, ci->team );

		VectorCopy( cent->lerpOrigin, torso.lightingOrigin );
		torso.shadowPlane = shadowPlane;
		torso.renderfx = renderfx;
		CG_PositionRotatedEntityOnTag( &torso, &legs, ci->playerModel, &skeleton, "tag_torso");

		VectorCopy( cent->lerpOrigin, head.lightingOrigin );
		head.shadowPlane = shadowPlane;
		head.renderfx = renderfx;
		CG_PositionRotatedEntityOnTag( &head, &torso, ci->playerModel, &skeleton, "tag_head");
	} else {
#endif
	//
	// add the legs
	//
	legs.hModel = ci->legsModel;
	legs.customSkin = ci->legsSkin;
#ifdef IOQ3ZTM_NO_COMPAT // DAMAGE_SKINS
	legs.skinFraction = cent->currentState.skinFraction;
#endif

	VectorCopy( cent->lerpOrigin, legs.origin );

	VectorCopy( cent->lerpOrigin, legs.lightingOrigin );
	legs.shadowPlane = shadowPlane;
	legs.renderfx = renderfx;
	VectorCopy (legs.origin, legs.oldorigin);	// don't positionally lerp at all

	CG_AddRefEntityWithPowerups( &legs, &cent->currentState, ci->team );

	// if the model failed, allow the default nullmodel to be displayed
	if (!legs.hModel) {
		return;
	}

	//
	// add the torso
	//
	torso.hModel = ci->torsoModel;
	if (!torso.hModel) {
		return;
	}

	torso.customSkin = ci->torsoSkin;
#ifdef IOQ3ZTM_NO_COMPAT // DAMAGE_SKINS
	torso.skinFraction = cent->currentState.skinFraction;
#endif

	VectorCopy( cent->lerpOrigin, torso.lightingOrigin );

	CG_PositionRotatedEntityOnTag( &torso, &legs, ci->legsModel, NULL, "tag_torso");

	torso.shadowPlane = shadowPlane;
	torso.renderfx = renderfx;

	CG_AddRefEntityWithPowerups( &torso, &cent->currentState, ci->team );
#ifdef IOQ3ZTM // BONES
	}
#endif

#ifdef MISSIONPACK
#ifndef TURTLEARENA // NO_KAMIKAZE_ITEM
	if ( cent->currentState.eFlags & EF_KAMIKAZE ) {

		memset( &skull, 0, sizeof(skull) );

		VectorCopy( cent->lerpOrigin, skull.lightingOrigin );
		skull.shadowPlane = shadowPlane;
		skull.renderfx = renderfx;

		if ( cent->currentState.eFlags & EF_DEAD ) {
			// one skull bobbing above the dead body
			angle = ((cg.time / 7) & 255) * (M_PI * 2) / 255;
			if (angle > M_PI * 2)
				angle -= (float)M_PI * 2;
			dir[0] = sin(angle) * 20;
			dir[1] = cos(angle) * 20;
			angle = ((cg.time / 4) & 255) * (M_PI * 2) / 255;
			dir[2] = 15 + sin(angle) * 8;
			VectorAdd(torso.origin, dir, skull.origin);
			
			dir[2] = 0;
			VectorCopy(dir, skull.axis[1]);
			VectorNormalize(skull.axis[1]);
			VectorSet(skull.axis[2], 0, 0, 1);
			CrossProduct(skull.axis[1], skull.axis[2], skull.axis[0]);

			skull.hModel = cgs.media.kamikazeHeadModel;
			trap_R_AddRefEntityToScene( &skull );
			skull.hModel = cgs.media.kamikazeHeadTrail;
			trap_R_AddRefEntityToScene( &skull );
		}
		else {
			// three skulls spinning around the player
			angle = ((cg.time / 4) & 255) * (M_PI * 2) / 255;
			dir[0] = cos(angle) * 20;
			dir[1] = sin(angle) * 20;
			dir[2] = cos(angle) * 20;
			VectorAdd(torso.origin, dir, skull.origin);

			angles[0] = sin(angle) * 30;
			angles[1] = (angle * 180 / M_PI) + 90;
			if (angles[1] > 360)
				angles[1] -= 360;
			angles[2] = 0;
			AnglesToAxis( angles, skull.axis );

			/*
			dir[2] = 0;
			VectorInverse(dir);
			VectorCopy(dir, skull.axis[1]);
			VectorNormalize(skull.axis[1]);
			VectorSet(skull.axis[2], 0, 0, 1);
			CrossProduct(skull.axis[1], skull.axis[2], skull.axis[0]);
			*/

			skull.hModel = cgs.media.kamikazeHeadModel;
			trap_R_AddRefEntityToScene( &skull );
			// flip the trail because this skull is spinning in the other direction
			VectorInverse(skull.axis[1]);
			skull.hModel = cgs.media.kamikazeHeadTrail;
			trap_R_AddRefEntityToScene( &skull );

			angle = ((cg.time / 4) & 255) * (M_PI * 2) / 255 + M_PI;
			if (angle > M_PI * 2)
				angle -= (float)M_PI * 2;
			dir[0] = sin(angle) * 20;
			dir[1] = cos(angle) * 20;
			dir[2] = cos(angle) * 20;
			VectorAdd(torso.origin, dir, skull.origin);

			angles[0] = cos(angle - 0.5 * M_PI) * 30;
			angles[1] = 360 - (angle * 180 / M_PI);
			if (angles[1] > 360)
				angles[1] -= 360;
			angles[2] = 0;
			AnglesToAxis( angles, skull.axis );

			/*
			dir[2] = 0;
			VectorCopy(dir, skull.axis[1]);
			VectorNormalize(skull.axis[1]);
			VectorSet(skull.axis[2], 0, 0, 1);
			CrossProduct(skull.axis[1], skull.axis[2], skull.axis[0]);
			*/

			skull.hModel = cgs.media.kamikazeHeadModel;
			trap_R_AddRefEntityToScene( &skull );
			skull.hModel = cgs.media.kamikazeHeadTrail;
			trap_R_AddRefEntityToScene( &skull );

			angle = ((cg.time / 3) & 255) * (M_PI * 2) / 255 + 0.5 * M_PI;
			if (angle > M_PI * 2)
				angle -= (float)M_PI * 2;
			dir[0] = sin(angle) * 20;
			dir[1] = cos(angle) * 20;
			dir[2] = 0;
			VectorAdd(torso.origin, dir, skull.origin);
			
			VectorCopy(dir, skull.axis[1]);
			VectorNormalize(skull.axis[1]);
			VectorSet(skull.axis[2], 0, 0, 1);
			CrossProduct(skull.axis[1], skull.axis[2], skull.axis[0]);

			skull.hModel = cgs.media.kamikazeHeadModel;
			trap_R_AddRefEntityToScene( &skull );
			skull.hModel = cgs.media.kamikazeHeadTrail;
			trap_R_AddRefEntityToScene( &skull );
		}
	}
#endif

#ifdef IOQ3ZTM
	if ( !(cent->currentState.powerups & ( 1 << PW_INVIS ) ) ) {
#endif
	if ( cent->currentState.powerups & ( 1 << PW_GUARD ) ) {
		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.guardPowerupModel;
		powerup.frame = 0;
		powerup.oldframe = 0;
		powerup.customSkin = 0;
		trap_R_AddRefEntityToScene( &powerup );
	}
	if ( cent->currentState.powerups & ( 1 << PW_SCOUT ) ) {
		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.scoutPowerupModel;
		powerup.frame = 0;
		powerup.oldframe = 0;
		powerup.customSkin = 0;
		trap_R_AddRefEntityToScene( &powerup );
	}
	if ( cent->currentState.powerups & ( 1 << PW_DOUBLER ) ) {
		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.doublerPowerupModel;
		powerup.frame = 0;
		powerup.oldframe = 0;
		powerup.customSkin = 0;
		trap_R_AddRefEntityToScene( &powerup );
	}
	if ( cent->currentState.powerups & ( 1 << PW_AMMOREGEN ) ) {
		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.ammoRegenPowerupModel;
		powerup.frame = 0;
		powerup.oldframe = 0;
		powerup.customSkin = 0;
		trap_R_AddRefEntityToScene( &powerup );
	}
#ifdef IOQ3ZTM
	}
#endif
#ifndef TURTLEARENA // POWERS
	if ( cent->currentState.powerups & ( 1 << PW_INVULNERABILITY ) ) {
		if ( !ci->invulnerabilityStartTime ) {
			ci->invulnerabilityStartTime = cg.time;
		}
		ci->invulnerabilityStopTime = cg.time;
	}
	else {
		ci->invulnerabilityStartTime = 0;
	}
	if ( (cent->currentState.powerups & ( 1 << PW_INVULNERABILITY ) ) ||
		cg.time - ci->invulnerabilityStopTime < 250 ) {

		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.invulnerabilityPowerupModel;
		powerup.customSkin = 0;
		// always draw
		powerup.renderfx &= ~RF_ONLY_MIRROR;
		VectorCopy(cent->lerpOrigin, powerup.origin);

		if ( cg.time - ci->invulnerabilityStartTime < 250 ) {
			c = (float) (cg.time - ci->invulnerabilityStartTime) / 250;
		}
		else if (cg.time - ci->invulnerabilityStopTime < 250 ) {
			c = (float) (250 - (cg.time - ci->invulnerabilityStopTime)) / 250;
		}
		else {
			c = 1;
		}
		VectorSet( powerup.axis[0], c, 0, 0 );
		VectorSet( powerup.axis[1], 0, c, 0 );
		VectorSet( powerup.axis[2], 0, 0, c );
		trap_R_AddRefEntityToScene( &powerup );
	}
#endif

	t = cg.time - ci->medkitUsageTime;
	if ( ci->medkitUsageTime && t < 500 ) {
		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.medkitUsageModel;
		powerup.customSkin = 0;
		// always draw
		powerup.renderfx &= ~RF_ONLY_MIRROR;
		VectorClear(angles);
		AnglesToAxis(angles, powerup.axis);
		VectorCopy(cent->lerpOrigin, powerup.origin);
		powerup.origin[2] += -24 + (float) t * 80 / 500;
		if ( t > 400 ) {
			c = (float) (t - 1000) * 0xff / 100;
			powerup.shaderRGBA[0] = 0xff - c;
			powerup.shaderRGBA[1] = 0xff - c;
			powerup.shaderRGBA[2] = 0xff - c;
			powerup.shaderRGBA[3] = 0xff - c;
		}
		else {
			powerup.shaderRGBA[0] = 0xff;
			powerup.shaderRGBA[1] = 0xff;
			powerup.shaderRGBA[2] = 0xff;
			powerup.shaderRGBA[3] = 0xff;
		}
		trap_R_AddRefEntityToScene( &powerup );
	}
#endif // MISSIONPACK

#ifdef IOQ3ZTM // BONES
	if (!ci->playerModel) {
#endif
	//
	// add the head
	//
	head.hModel = ci->headModel;
	if (!head.hModel) {
		return;
	}
	head.customSkin = ci->headSkin;
#ifdef IOQ3ZTM_NO_COMPAT // DAMAGE_SKINS
	head.skinFraction = cent->currentState.skinFraction;
#endif

	VectorCopy( cent->lerpOrigin, head.lightingOrigin );

	CG_PositionRotatedEntityOnTag( &head, &torso, ci->torsoModel, NULL, "tag_head");

	head.shadowPlane = shadowPlane;
	head.renderfx = renderfx;

	CG_AddRefEntityWithPowerups( &head, &cent->currentState, ci->team );
#ifdef IOQ3ZTM // BONES
	}
#endif

#ifdef IOQ3ZTM
	// add the talk baloon or disconnect icon
	CG_PlayerSprites( cent, &head );
#endif

#ifdef MISSIONPACK
	CG_BreathPuffs(cent, &head);

	CG_DustTrail(cent);
#endif

	//
	// add the gun / barrel / flash
	//
#ifdef IOQ3ZTM // BONES
	if (ci->playerModel)
		CG_AddPlayerWeapon( &legs, &skeleton, NULL, cent, ci->team );
	else
#endif
	CG_AddPlayerWeapon( &torso, NULL, NULL, cent, ci->team );

	// add powerups floating behind the player
#ifdef IOQ3ZTM // BONES
	if (ci->playerModel)
		CG_PlayerPowerups( cent, &legs, &skeleton );
	else
#endif
	CG_PlayerPowerups( cent, &torso, NULL );

#if 0 //#ifdef IOQ3ZTM // ZTM: TODO: Add a speed effect?
	if ((cent->currentState.powerups & ( 1 << PW_HASTE )
		|| cent->currentState.powerups & ( 1 << PW_SCOUT ))
		&& !(cent->currentState.powerups & ( 1 << PW_INVIS )))
	{
		// ...
	}
#endif
}


//=====================================================================

/*
===============
CG_ResetPlayerEntity

A player just came into view or teleported, so reset all animation info
===============
*/
void CG_ResetPlayerEntity( centity_t *cent ) {
	cent->errorTime = -99999;		// guarantee no error decay added
	cent->extrapolated = qfalse;	

#ifdef IOQ3ZTM // LERP_FRAME_CLIENT_LESS
#ifdef TA_PLAYERSYS
	BG_ClearLerpFrame( &cent->pe.legs, cgs.clientinfo[ cent->currentState.clientNum ].playercfg.animations, cent->currentState.legsAnim, cg.time );
	BG_ClearLerpFrame( &cent->pe.torso, cgs.clientinfo[ cent->currentState.clientNum ].playercfg.animations, cent->currentState.torsoAnim, cg.time );
#else
	BG_ClearLerpFrame( &cent->pe.legs, cgs.clientinfo[ cent->currentState.clientNum ].animations, cent->currentState.legsAnim, cg.time );
	BG_ClearLerpFrame( &cent->pe.torso, cgs.clientinfo[ cent->currentState.clientNum ].animations, cent->currentState.torsoAnim, cg.time );
#endif
#else
	CG_ClearLerpFrame( &cgs.clientinfo[ cent->currentState.clientNum ], &cent->pe.legs, cent->currentState.legsAnim );
	CG_ClearLerpFrame( &cgs.clientinfo[ cent->currentState.clientNum ], &cent->pe.torso, cent->currentState.torsoAnim );
#endif

	BG_EvaluateTrajectory( &cent->currentState.pos, cg.time, cent->lerpOrigin );
	BG_EvaluateTrajectory( &cent->currentState.apos, cg.time, cent->lerpAngles );

	VectorCopy( cent->lerpOrigin, cent->rawOrigin );
	VectorCopy( cent->lerpAngles, cent->rawAngles );

	memset( &cent->pe.legs, 0, sizeof( cent->pe.legs ) );
	cent->pe.legs.yawAngle = cent->rawAngles[YAW];
	cent->pe.legs.yawing = qfalse;
	cent->pe.legs.pitchAngle = 0;
	cent->pe.legs.pitching = qfalse;

	memset( &cent->pe.torso, 0, sizeof( cent->pe.torso ) );
	cent->pe.torso.yawAngle = cent->rawAngles[YAW];
	cent->pe.torso.yawing = qfalse;
	cent->pe.torso.pitchAngle = cent->rawAngles[PITCH];
	cent->pe.torso.pitching = qfalse;

	if ( cg_debugPosition.integer ) {
		CG_Printf("%i ResetPlayerEntity yaw=%f\n", cent->currentState.number, cent->pe.torso.yawAngle );
	}
}


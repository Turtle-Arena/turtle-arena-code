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
/*
=============================================================================

START SERVER MENU *****

=============================================================================
*/


#include "ui_local.h"

#ifdef TA_SP

#include "ui_local.h"

#define GAMESERVER_BACK0		"menu/art/back_0"
#define GAMESERVER_BACK1		"menu/art/back_1"
#define GAMESERVER_NEXT0		"menu/art/next_0"
#define GAMESERVER_NEXT1		"menu/art/next_1"
#ifdef TA_MISC // NO_MENU_FIGHT
#define GAMESERVER_PLAY0		"menu/art/play_0"
#define GAMESERVER_PLAY1		"menu/art/play_1"
#else
#define GAMESERVER_PLAY0		"menu/art/fight_0"
#define GAMESERVER_PLAY1		"menu/art/fight_1"
#endif
#define GAMESERVER_UNKNOWNMAP	"menu/art/unknownmap"

#define ID_MAXCLIENTS			20
#define ID_DEDICATED			21
#define ID_NEXT					22
#define ID_BACK					23
#define ID_GAMETYPE				24
#define ID_MAP					25
#define ID_RECORDREPLAY			26

#define MAX_MAPNAMELENGTH		16

typedef struct {
	menuframework_s		menu;

	menutext_s			banner;
	menulist_s			gametype;
#if 0
	menutext_s			gtDescription[4];
#endif

	menulist_s			map;
	menubitmap_s		mappic;

	// Arcade
	menuradiobutton_s	recordreplay;
	menutext_s			viewreplay;
	menutext_s			scoreId[NUM_ARCADE_SCORES];
	menutext_s			scoreNum[NUM_ARCADE_SCORES];
	menubitmap_s		scoreCharacter[NUM_ARCADE_SCORES];
	menutext_s			scoreTime[NUM_ARCADE_SCORES];
	menutext_s			scoreName[NUM_ARCADE_SCORES];

	// Server
	menufield_s			timelimit;
	menufield_s			scorelimit;
	menufield_s			flaglimit;
	menuradiobutton_s	friendlyfire;
	//
	menuradiobutton_s	pure;
	menuradiobutton_s	publicserver;
	menuradiobutton_s	dedicated;
	menufield_s			hostname;

	menubitmap_s		back;
	menubitmap_s		next;

	qboolean			multiplayer;
	qboolean			inGame;
	int					maplist[MAX_ARENAS];
	char				*mapNames[MAX_ARENAS];

	arcadeGameData_t	gamedata;

} arcademenu_t;

static arcademenu_t s_arcade;

static char scoreIds[NUM_ARCADE_SCORES][5] = { "#1:", "#2:", "#3:", "#4:", "#5:" };
static char scoreNums[NUM_ARCADE_SCORES][8] = { "0", "0", "0", "0", "0" };
static char scoreTimes[NUM_ARCADE_SCORES][6] = { "0:00", "0:00", "0:00", "0:00", "0:00" };
static char scoreNames[NUM_ARCADE_SCORES][9] = { "Nobody", "Nobody", "Nobody", "Nobody", "Nobody" };

// Order of gametypes in "gametype select list", must match gametype_items
static int listToGametype[] = {
	GT_FFA,
	GT_TOURNAMENT,
	GT_TEAM,
	GT_CTF,
#ifdef MISSIONPACK
	GT_1FCTF,
	GT_OBELISK,
#ifdef MISSIONPACK_HARVESTER
	GT_HARVESTER,
#endif
#endif
	GT_SINGLE_PLAYER,
};

// Order of gametype_items, convert GT_* to "gametype select list" index
static int gametypeToList[] = {
	0, // GT_FFA
	1, // GT_TOURNAMENT
	ARRAY_LEN(listToGametype)-1, // GT_SINGLE_PLAYER
	2, // GT_TEAM
	3, // GT_CTF
#ifdef MISSIONPACK
	4, // GT_1FCTF
	5, // GT_OBELISK
#ifdef MISSIONPACK_HARVESTER
	6, // GT_HARVESTER
#endif
#endif
};

// Names of gametypes in "gametype select list", can be any gametypes in any order
static const char *gametype_items[] = {
	"Free For All",
	"Duel", // tornament to duel // "Tournament",
	"Team Deathmatch",
	"Capture the Flag",
#ifdef MISSIONPACK
	"1 Flag CTF",
	"Overload",
#ifdef MISSIONPACK_HARVESTER
	"Harvester",
#endif
#endif
	"Cooperative",
	NULL
};

// Don't allow sp/coop in arcade mode
static const char *arcade_gametype_items[] = {
	"Free For All",
	"Duel", // tornament to duel // "Tournament",
	"Team Deathmatch",
	"Capture the Flag",
#ifdef MISSIONPACK
	"1 Flag CTF",
	"Overload",
#ifdef MISSIONPACK_HARVESTER
	"Harvester",
#endif
#endif
	"", // No coop in arcade
	NULL
};


void UI_LoadBestScores(const char *map, int game)
{
	char		fileName[MAX_QPATH];
	fileHandle_t f;
	static char *gametypeNames[] = {"ffa", "tourney", "single", "team", "ctf", "oneflag", "overload", "harvester"};
	qboolean	validData;
	int i;
	
	// compose file name
	Com_sprintf(fileName, MAX_QPATH, "scores/%s_%s.score", map, gametypeNames[game]);
	// see if we have one already
	validData = qfalse;
	if (trap_FS_FOpenFile(fileName, &f, FS_READ) >= 0) {
		trap_FS_Read(&s_arcade.gamedata, sizeof(arcadeGameData_t), f);
		trap_FS_FCloseFile(f);

		if (!Q_strncmp(s_arcade.gamedata.magic, ARCADE_GAMEDATA_MAGIC, ARRAY_LEN(s_arcade.gamedata.magic)))
		{
			if (s_arcade.gamedata.version == ARCADE_GAMEDATA_VERSION) {
				validData = qtrue;
			}
		}
	}

	if (!validData) {
		memset(&s_arcade.gamedata, 0, sizeof(arcadeGameData_t));
	}

	// Set scores
	for (i = 0; i < NUM_ARCADE_SCORES; ++i) {
		Com_sprintf(scoreNums[i], sizeof (scoreNums[i]), "%d", s_arcade.gamedata.scores[i].score);
		Com_sprintf(scoreTimes[i], sizeof (scoreTimes[i]), "%d:%.2d",
				s_arcade.gamedata.scores[i].time / 60, s_arcade.gamedata.scores[i].time % 60);
		Q_strncpyz(scoreNames[i], s_arcade.gamedata.scores[i].name, sizeof (scoreNames[i]) );

		s_arcade.scoreCharacter[i].shader = UI_GetScoreIcon( s_arcade.gamedata.scores[i] );

		// Set name for empty scores
		if (scoreNames[i][0] == '\0') {
			Q_strncpyz(scoreNames[i], "Nobody", sizeof (scoreNames[i]) );
		}
	}

	Com_sprintf(fileName, MAX_QPATH, "%s_%d", map, game);
	if ( trap_GetDemoFileInfo( fileName, NULL, NULL, NULL, NULL, NULL ) ) {
		s_arcade.viewreplay.generic.flags &= ~QMF_GRAYED;
	} else {
		s_arcade.viewreplay.generic.flags |= QMF_GRAYED;
	}
}

/*
=================
StartArcade_Update
=================
*/
static void StartArcade_Update(void) {
	static 	char levelshotBuffer[MAX_QPATH];
	const char *info;

	// update levelshot
	info = UI_GetArenaInfoByNumber( s_arcade.maplist[ s_arcade.map.curvalue ]);
	Com_sprintf(levelshotBuffer, sizeof (levelshotBuffer), "levelshots/%s_small", Info_ValueForKey( info, "map"));
	s_arcade.mappic.generic.name = levelshotBuffer;
	s_arcade.mappic.shader = 0;

	if (!s_arcade.multiplayer) {
		UI_LoadBestScores(Info_ValueForKey( info, "map"), listToGametype[s_arcade.gametype.curvalue]);
	}
}

/*
=================
StartArcade_SaveMenuItems
=================
*/
static void StartArcade_SaveMenuItems( int gametype ) {
	int		timelimit;
	int		scorelimit;
	int		flaglimit;
	int		friendlyfire;

	timelimit	 = atoi( MField_Buffer( &s_arcade.timelimit.field ) );
	scorelimit	 = atoi( MField_Buffer( &s_arcade.scorelimit.field ) );
	flaglimit	 = atoi( MField_Buffer( &s_arcade.flaglimit.field ) );
	friendlyfire = s_arcade.friendlyfire.curvalue;

	trap_Cvar_SetValue( "ui_publicServer", Com_Clamp( 0, 1, s_arcade.publicserver.curvalue ) );
	trap_Cvar_SetValue( "sv_pure", s_arcade.pure.curvalue );
	trap_Cvar_Set("sv_hostname", MField_Buffer( &s_arcade.hostname.field ) );

	switch( gametype ) {
	case GT_FFA:
	default:
		trap_Cvar_SetValue( "ui_ffa_scorelimit", scorelimit );
		trap_Cvar_SetValue( "ui_ffa_timelimit", timelimit );
		break;

	case GT_TOURNAMENT:
		trap_Cvar_SetValue( "ui_tourney_scorelimit", scorelimit );
		trap_Cvar_SetValue( "ui_tourney_timelimit", timelimit );
		break;

	case GT_SINGLE_PLAYER:
		// Co-op settings
		break;

	case GT_TEAM:
		trap_Cvar_SetValue( "ui_team_scorelimit", scorelimit );
		trap_Cvar_SetValue( "ui_team_timelimit", timelimit );
		trap_Cvar_SetValue( "ui_team_friendly", friendlyfire );
		break;

	case GT_CTF:
		trap_Cvar_SetValue( "ui_ctf_capturelimit", flaglimit );
		trap_Cvar_SetValue( "ui_ctf_timelimit", timelimit );
		trap_Cvar_SetValue( "ui_ctf_friendly", friendlyfire );
		break;

#ifdef MISSIONPACK
	case GT_1FCTF:
		trap_Cvar_SetValue( "ui_1flag_capturelimit", flaglimit );
		trap_Cvar_SetValue( "ui_1flag_timelimit", timelimit );
		trap_Cvar_SetValue( "ui_1flag_friendly", friendlyfire );
		break;

	case GT_OBELISK:
		trap_Cvar_SetValue( "ui_obelisk_capturelimit", flaglimit );
		trap_Cvar_SetValue( "ui_obelisk_timelimit", timelimit );
		trap_Cvar_SetValue( "ui_obelisk_friendly", friendlyfire );
		break;

#ifdef MISSIONPACK_HARVESTER
	case GT_HARVESTER:
		trap_Cvar_SetValue( "ui_harvester_capturelimit", flaglimit );
		trap_Cvar_SetValue( "ui_harvester_timelimit", timelimit );
		trap_Cvar_SetValue( "ui_harvester_friendly", friendlyfire );
		break;
#endif
#endif
	}
}

/*
=================
StartArcade_SetMenuItems
=================
*/
static void StartArcade_SetMenuItems( void ) {

	s_arcade.publicserver.curvalue = Com_Clamp( 0, 1, trap_Cvar_VariableValue( "ui_publicServer" ) );
	MField_SetText( &s_arcade.hostname.field, CG_Cvar_VariableString( "sv_hostname" ) );
	s_arcade.pure.curvalue = Com_Clamp( 0, 1, trap_Cvar_VariableValue( "sv_pure" ) );

	switch( listToGametype[s_arcade.gametype.curvalue] ) {
	case GT_FFA:
	default:
		MField_SetText( &s_arcade.scorelimit.field, va( "%i", (int)Com_Clamp( 0, 9999, trap_Cvar_VariableValue( "ui_ffa_scorelimit" ) ) ) );
		MField_SetText( &s_arcade.timelimit.field, va( "%i", (int)Com_Clamp( 0, 999, trap_Cvar_VariableValue( "ui_ffa_timelimit" ) ) ) );
		s_arcade.friendlyfire.curvalue = 0;
		break;

	case GT_TOURNAMENT:
		MField_SetText( &s_arcade.scorelimit.field, va( "%i", (int)Com_Clamp( 0, 9999, trap_Cvar_VariableValue( "ui_tourney_scorelimit" ) ) ) );
		MField_SetText( &s_arcade.timelimit.field, va( "%i", (int)Com_Clamp( 0, 999, trap_Cvar_VariableValue( "ui_tourney_timelimit" ) ) ) );
		s_arcade.friendlyfire.curvalue = 0;
		break;

	case GT_SINGLE_PLAYER:
		MField_SetText( &s_arcade.scorelimit.field, "0" );
		MField_SetText( &s_arcade.flaglimit.field, "0" );
		MField_SetText( &s_arcade.timelimit.field, "0" );
		s_arcade.friendlyfire.curvalue = 0;
		break;

	case GT_TEAM:
		MField_SetText( &s_arcade.scorelimit.field, va( "%i", (int)Com_Clamp( 0, 9999, trap_Cvar_VariableValue( "ui_team_scorelimit" ) ) ) );
		MField_SetText( &s_arcade.timelimit.field, va( "%i", (int)Com_Clamp( 0, 999, trap_Cvar_VariableValue( "ui_team_timelimit" ) ) ) );
		s_arcade.friendlyfire.curvalue = (int)Com_Clamp( 0, 1, trap_Cvar_VariableValue( "ui_team_friendly" ) );
		break;

	case GT_CTF:
		MField_SetText( &s_arcade.flaglimit.field, va( "%i", (int)Com_Clamp( 0, 100, trap_Cvar_VariableValue( "ui_ctf_capturelimit" ) ) ) );
		MField_SetText( &s_arcade.timelimit.field, va( "%i", (int)Com_Clamp( 0, 999, trap_Cvar_VariableValue( "ui_ctf_timelimit" ) ) ) );
		s_arcade.friendlyfire.curvalue = (int)Com_Clamp( 0, 1, trap_Cvar_VariableValue( "ui_ctf_friendly" ) );
		break;

#ifdef MISSIONPACK
	case GT_1FCTF:
		MField_SetText( &s_arcade.flaglimit.field, va( "%i", (int)Com_Clamp( 0, 100, trap_Cvar_VariableValue( "ui_1flag_capturelimit" ) ) ) );
		MField_SetText( &s_arcade.timelimit.field, va( "%i", (int)Com_Clamp( 0, 999, trap_Cvar_VariableValue( "ui_1flag_timelimit" ) ) ) );
		s_arcade.friendlyfire.curvalue = (int)Com_Clamp( 0, 1, trap_Cvar_VariableValue( "ui_1flag_friendly" ) );
		break;

	case GT_OBELISK:
		MField_SetText( &s_arcade.flaglimit.field, va( "%i", (int)Com_Clamp( 0, 100, trap_Cvar_VariableValue( "ui_obelisk_capturelimit" ) ) ) );
		MField_SetText( &s_arcade.timelimit.field, va( "%i", (int)Com_Clamp( 0, 999, trap_Cvar_VariableValue( "ui_obelisk_timelimit" ) ) ) );
		s_arcade.friendlyfire.curvalue = (int)Com_Clamp( 0, 1, trap_Cvar_VariableValue( "ui_obelisk_friendly" ) );
		break;

#ifdef MISSIONPACK_HARVESTER
	case GT_HARVESTER:
		MField_SetText( &s_arcade.flaglimit.field, va( "%i", (int)Com_Clamp( 0, 100, trap_Cvar_VariableValue( "ui_harvester_capturelimit" ) ) ) );
		MField_SetText( &s_arcade.timelimit.field, va( "%i", (int)Com_Clamp( 0, 999, trap_Cvar_VariableValue( "ui_harvester_timelimit" ) ) ) );
		s_arcade.friendlyfire.curvalue = (int)Com_Clamp( 0, 1, trap_Cvar_VariableValue( "ui_harvester_friendly" ) );
		break;
#endif
#endif
	}
}

/*
=================
StartArcade_Start
=================
*/
static void StartArcade_Start( void ) {
	int		gametype;
	int		timelimit;
	int		scorelimit;
	int		maxclients;
	int		localClients;
	int		publicserver;
	int		dedicated;
	int		friendlyfire;
	int		flaglimit;
	int		pure;
	int		skill;
	int		numLocalClients;
	int		n;
	char	buf[64];
	const char *info;

	gametype	 = listToGametype[s_arcade.gametype.curvalue];
	timelimit	 = atoi( MField_Buffer( &s_arcade.timelimit.field ) );
	scorelimit	 = atoi( MField_Buffer( &s_arcade.scorelimit.field ) );
	flaglimit	 = atoi( MField_Buffer( &s_arcade.flaglimit.field ) );
	publicserver = s_arcade.publicserver.curvalue;
	dedicated	 = s_arcade.dedicated.curvalue;
	friendlyfire = s_arcade.friendlyfire.curvalue;
	pure		 = s_arcade.pure.curvalue;
	skill		 = 3;
	localClients = trap_Cvar_VariableValue( "cl_localPlayers" );

	for( n = 0, numLocalClients = 0; n < MAX_SPLITVIEW; ++n ) {
		if (localClients & (1<<n)) {
			numLocalClients++;
		}
	}

	if (!s_arcade.multiplayer) {
		if (gametype == GT_TOURNAMENT) {
			maxclients = 2;
		} else if (gametype >= GT_TEAM) {
			maxclients = 8;
		} else {
			maxclients = 4;
		}
	} else {
		maxclients = 8;
	}

	StartArcade_SaveMenuItems(gametype);

	trap_Cvar_SetValue( "g_gametype", gametype );

	trap_Cvar_SetValue( "sv_maxclients", Com_Clamp( 0, 12, maxclients ) );
	if( s_arcade.multiplayer ) {
		trap_Cvar_SetValue( "ui_publicServer", Com_Clamp( 0, 1, publicserver ) );
		trap_Cvar_SetValue( "sv_public", Com_Clamp( 0, 1, publicserver ) );
	} else {
		trap_Cvar_SetValue( "sv_public", 0 );
		trap_Cvar_SetValue( "ui_singlePlayerActive", 1 );
	}
	trap_Cvar_SetValue( "dedicated", Com_Clamp( 0, 1, dedicated ) );
	trap_Cvar_SetValue ("timelimit", Com_Clamp( 0, timelimit, timelimit ) );
	trap_Cvar_SetValue ("scorelimit", Com_Clamp( 0, scorelimit, scorelimit ) );
	trap_Cvar_SetValue ("capturelimit", Com_Clamp( 0, flaglimit, flaglimit ) );
	trap_Cvar_SetValue( "g_friendlyfire", friendlyfire );
	trap_Cvar_SetValue( "sv_pure", pure );
	trap_Cvar_Set("sv_hostname", MField_Buffer( &s_arcade.hostname.field ) );

	// set player's team
	if( !s_arcade.inGame && dedicated == 0 && gametype >= GT_TEAM ) {
		for( n = 0; n < MAX_SPLITVIEW; ++n ) {
			if (localClients & (1<<n)) {
				trap_Cvar_Set( Com_LocalPlayerCvarName( n, "teampref" ), "blue" );
			}
		}
	}

	// the wait commands will allow the dedicated to take effect
	info = UI_GetArenaInfoByNumber( s_arcade.maplist[ s_arcade.map.curvalue ]);
	trap_Cmd_ExecuteText( EXEC_APPEND, va( "wait ; wait ; map %s\n", Info_ValueForKey( info, "map" )));

	trap_Cvar_SetValue( "ui_recordSPDemo", s_arcade.recordreplay.curvalue );
	if (s_arcade.recordreplay.curvalue) {
		char buff[MAX_STRING_CHARS];

		Com_sprintf(buff, MAX_STRING_CHARS, "%s_%i", Info_ValueForKey( info, "map" ), gametype);
		trap_Cvar_Set("ui_recordSPDemoName", buff);

#ifdef MISSIONPACK
		// If not going to do a warmup and then map_restart, tell client to run demo record command
		if (!trap_Cvar_VariableValue("g_doWarmup"))
#endif
		{
			trap_Cvar_Set("activeAction", va("set g_synchronousclients 1 ; record %s \n", buff));
		}
	}

	// add bots
	if (!s_arcade.multiplayer) {
		char	playerCharacter[32];

		// Get user's character
		trap_Cvar_VariableStringBuffer("spmodel", playerCharacter, sizeof(playerCharacter));

		trap_Cmd_ExecuteText( EXEC_APPEND, "wait 3\n" );

		for( n = 0; n < maxclients; n++ ) {
			// Skip bot for character user is playing as.
			if (!Q_stricmpn(playerCharacter, spCharacterNames[n%NUM_SP_CHARACTERS], sizeof(playerCharacter))) {
				if( gametype >= GT_TEAM ) {
					if (n < maxclients/2) {
						continue;
					}
				} else {
					continue;
				}
			}

			if( gametype >= GT_TEAM ) {
				Com_sprintf( buf, sizeof(buf), "addbot %s %i %s\n", spCharacterNames[n%NUM_SP_CHARACTERS], skill,
					n < maxclients/2 ? "blue" : "red" );
			} else {
				Com_sprintf( buf, sizeof(buf), "addbot %s %i\n", spCharacterNames[n%NUM_SP_CHARACTERS], skill );
			}
			trap_Cmd_ExecuteText( EXEC_APPEND, buf );
		}
	}
}

/*
=================
GametypeBits
=================
*/
static int GametypeBits( char *string ) {
	int		bits;
	char	*p;
	char	*token;

	bits = 0;
	p = string;
	while( 1 ) {
		token = COM_ParseExt( &p, qfalse );
		if( token[0] == 0 ) {
			break;
		}

		if( Q_stricmp( token, "ffa" ) == 0 ) {
			bits |= 1 << GT_FFA;
			continue;
		}

		if( Q_stricmp( token, "tourney" ) == 0 ) {
			bits |= 1 << GT_TOURNAMENT;
			continue;
		}

		if( Q_stricmp( token, "single" ) == 0 ) {
			bits |= 1 << GT_SINGLE_PLAYER;
			continue;
		}

		if( Q_stricmp( token, "team" ) == 0 ) {
			bits |= 1 << GT_TEAM;
			continue;
		}

		if( Q_stricmp( token, "ctf" ) == 0 ) {
			bits |= 1 << GT_CTF;
			continue;
		}

#ifdef MISSIONPACK
		if( Q_stricmp( token, "oneflag" ) == 0 ) {
			bits |= 1 << GT_1FCTF;
			continue;
		}

		if( Q_stricmp( token, "overload" ) == 0 ) {
			bits |= 1 << GT_OBELISK;
			continue;
		}

#ifdef MISSIONPACK_HARVESTER
		if( Q_stricmp( token, "harvester" ) == 0 ) {
			bits |= 1 << GT_HARVESTER;
			continue;
		}
#endif
#endif
	}

	return bits;
}

/*
=================
StartArcade_GametypeEvent
=================
*/
static void StartArcade_GametypeEvent( void* ptr, int event ) {
	qboolean	scorelimit;
	qboolean	flaglimit;
	int			gametype;
	int			oldgametype;
	int			i;
	int			count;
	int			gamebits;
	int			matchbits;
	const char	*info;
	static char mapNameBuffer[MAX_ARENAS][MAX_MAPNAMELENGTH];

	if( event != QM_ACTIVATED) {
		return;
	}

	gametype = listToGametype[s_arcade.gametype.curvalue];
	oldgametype = listToGametype[s_arcade.gametype.oldvalue];

	s_arcade.map.curvalue = 0;

	count = UI_GetNumArenas();
	s_arcade.map.numitems = 0;
	matchbits = 1 << gametype;
	for( i = 0; i < count; i++ ) {
		info = UI_GetArenaInfoByNumber( i );
	
		gamebits = GametypeBits( Info_ValueForKey( info, "type") );
		if( !( gamebits & matchbits ) ) {
			continue;
		}

		s_arcade.maplist[ s_arcade.map.numitems ] = i;
		strncpy(mapNameBuffer[s_arcade.map.numitems], Info_ValueForKey( info, "longname"), MAX_MAPNAMELENGTH);
		mapNameBuffer[s_arcade.map.numitems][MAX_MAPNAMELENGTH-1] = 0;
		if (!strlen(mapNameBuffer[s_arcade.map.numitems])) {
			strncpy(mapNameBuffer[s_arcade.map.numitems], Info_ValueForKey( info, "map"), MAX_MAPNAMELENGTH);
			mapNameBuffer[s_arcade.map.numitems][MAX_MAPNAMELENGTH-1] = 0;
		}
		s_arcade.mapNames[s_arcade.map.numitems] = mapNameBuffer[s_arcade.map.numitems];
		s_arcade.map.numitems++;
	}

	s_arcade.map.itemnames = (const char **)s_arcade.mapNames;

	if (gametype == GT_SINGLE_PLAYER) {
		scorelimit = flaglimit = qfalse;
	} else if (gametype <= GT_TEAM) {
		scorelimit = qtrue;
		flaglimit = qfalse;
	} else {
		scorelimit = qfalse;
		flaglimit = qtrue;
	}

	if (gametype != GT_SINGLE_PLAYER)
		s_arcade.timelimit.generic.flags	&= ~(QMF_HIDDEN|QMF_INACTIVE);
	else
		s_arcade.timelimit.generic.flags |= (QMF_HIDDEN|QMF_INACTIVE);

	if (gametype >= GT_TEAM)
		s_arcade.friendlyfire.generic.flags	&= ~(QMF_HIDDEN|QMF_INACTIVE);
	else
		s_arcade.friendlyfire.generic.flags |= (QMF_HIDDEN|QMF_INACTIVE);

	if (scorelimit)
		s_arcade.scorelimit.generic.flags	&= ~(QMF_HIDDEN|QMF_INACTIVE);
	else
		s_arcade.scorelimit.generic.flags	|= (QMF_HIDDEN|QMF_INACTIVE);

	if (flaglimit)
		s_arcade.flaglimit.generic.flags	&= ~(QMF_HIDDEN|QMF_INACTIVE);
	else
		s_arcade.flaglimit.generic.flags	|= (QMF_HIDDEN|QMF_INACTIVE);


	StartArcade_SaveMenuItems(oldgametype);
	StartArcade_SetMenuItems();

	StartArcade_Update();
}

/*
=================
StartArcade_MapEvent
=================
*/
static void StartArcade_MapEvent( void* ptr, int event ) {
	if( event != QM_ACTIVATED) {
		return;
	}
	StartArcade_Update();
}

static void StartArcade_ViewReplayEvent( void* ptr, int event ) {
	const char *info;

	if( event != QM_ACTIVATED) {
		return;
	}

	// Play demo!
	info = UI_GetArenaInfoByNumber( s_arcade.maplist[ s_arcade.map.curvalue ]);
	trap_Cmd_ExecuteText( EXEC_APPEND, va("demo %s_%d\n", Info_ValueForKey( info, "map"),
			listToGametype[s_arcade.gametype.curvalue]));
}

/*
=================
StartArcade_GametypeEvent
=================
*/
static void StartArcade_Event( void* ptr, int event ) {
	if( event != QM_ACTIVATED) {
		return;
	}
	
	switch (((menucommon_s*)ptr)->id) {
		case ID_DEDICATED:
			if (((menuradiobutton_s*)ptr)->curvalue) {
				s_arcade.next.generic.name		= GAMESERVER_PLAY0;
				s_arcade.next.focuspic			= GAMESERVER_PLAY1;
			} else {
				s_arcade.next.generic.name		= GAMESERVER_NEXT0;
				s_arcade.next.focuspic			= GAMESERVER_NEXT1;
			}
			s_arcade.next.shader = 0;
			s_arcade.next.focusshader = 0;
			break;
		case ID_BACK:
			StartArcade_SaveMenuItems(listToGametype[s_arcade.gametype.curvalue]);
			UI_PopMenu();
			break;
		case ID_NEXT:
			if (s_arcade.multiplayer && (s_arcade.dedicated.curvalue || s_arcade.inGame)) {
				StartArcade_Start();
			} else if (s_arcade.multiplayer) {
				UI_PlayerSetupMenu(MAX_SPLITVIEW, StartArcade_Start, qtrue);
			} else {
				UI_SPPlayerMenu(1, StartArcade_Start);
			}
	}
}

/*
=================
StartArcade_StatusBar
=================
*/
static void StartArcade_StatusBar( void* ptr ) {
	switch( ((menucommon_s*)ptr)->id ) {
	case ID_RECORDREPLAY:
		if (!(s_arcade.viewreplay.generic.flags & QMF_GRAYED)) {
			UI_DrawString( 320, 440, "Warning: This will overwrite the existing replay!",
					UI_CENTER|UI_SMALLFONT, colorWhite );
		}
		break;
	default:
		UI_DrawString( 320, 440, "0 = NO LIMIT", UI_CENTER|UI_SMALLFONT, colorWhite );
		break;
	}
}

/*
=================
StartArcade_MenuInit
=================
*/
// right side below levelshot
#define RIGHT_OPTIONS_X	64 //(456+106)

#define LEFT_OPTIONS_X 64

static void StartArcade_MenuInit( qboolean multiplayer ) {
	int		i;
	int		y;

	memset( &s_arcade, 0, sizeof(arcademenu_t) );
	s_arcade.multiplayer = multiplayer;
	s_arcade.inGame = trap_Cvar_VariableValue("sv_running");

	StartServer_Cache();

	s_arcade.menu.wrapAround = qtrue;
	s_arcade.menu.fullscreen = qtrue;

	s_arcade.banner.generic.type		= MTYPE_BTEXT;
	s_arcade.banner.generic.x			= 320;
	s_arcade.banner.generic.y			= 16;
	if (s_arcade.inGame) {
		s_arcade.banner.string  			= "CHANGE MAP";
	} else if (!s_arcade.multiplayer) {
		s_arcade.banner.string  			= "ARCADE MODE";
	} else {
		s_arcade.banner.string  			= "START SERVER";
	}
	s_arcade.banner.color  				= text_banner_color;
	s_arcade.banner.style  				= UI_CENTER;

	s_arcade.gametype.generic.type		= MTYPE_SPINCONTROL;
	s_arcade.gametype.generic.name		= "Game Type:";
	s_arcade.gametype.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_LEFT_JUSTIFY;
	s_arcade.gametype.generic.callback	= StartArcade_GametypeEvent;
	s_arcade.gametype.generic.id		= ID_GAMETYPE;
	s_arcade.gametype.generic.x			= 64;
	s_arcade.gametype.generic.y			= 224 - (SMALLCHAR_HEIGHT+2) * 2; // if gtDescription is added then use y=80
	if (!multiplayer) {
		s_arcade.gametype.itemnames		= arcade_gametype_items;
	} else {
		s_arcade.gametype.itemnames		= gametype_items;
	}

#if 0
	s_arcade.gtDescription[0].generic.type	= MTYPE_TEXT;
	s_arcade.gtDescription[0].generic.flags	= QMF_SMALLFONT|QMF_LEFT_JUSTIFY|QMF_INACTIVE|QMF_GRAYED;
	s_arcade.gtDescription[0].generic.x		= 88;
	s_arcade.gtDescription[0].generic.y		= 80 + SMALLCHAR_HEIGHT;
	s_arcade.gtDescription[0].string  		= "(Insert short gametype description here.)";
	s_arcade.gtDescription[0].color  		= text_banner_color;
	s_arcade.gtDescription[0].style  		= UI_LEFT;

	s_arcade.gtDescription[1].generic.type	= MTYPE_TEXT;
	s_arcade.gtDescription[1].generic.flags	= QMF_SMALLFONT|QMF_LEFT_JUSTIFY|QMF_INACTIVE|QMF_GRAYED;
	s_arcade.gtDescription[1].generic.x		= 88;
	s_arcade.gtDescription[1].generic.y		= 80 + SMALLCHAR_HEIGHT*2;
	s_arcade.gtDescription[1].string  		= "(Insert short gametype description here.)";
	s_arcade.gtDescription[1].color  		= text_banner_color;
	s_arcade.gtDescription[1].style  		= UI_LEFT;

	s_arcade.gtDescription[2].generic.type	= MTYPE_TEXT;
	s_arcade.gtDescription[2].generic.flags	= QMF_SMALLFONT|QMF_LEFT_JUSTIFY|QMF_INACTIVE|QMF_GRAYED;
	s_arcade.gtDescription[2].generic.x		= 88;
	s_arcade.gtDescription[2].generic.y		= 80 + SMALLCHAR_HEIGHT*3;
	s_arcade.gtDescription[2].string  		= "(Insert short gametype description here.)";
	s_arcade.gtDescription[2].color  		= text_banner_color;
	s_arcade.gtDescription[2].style  		= UI_LEFT;

	s_arcade.gtDescription[3].generic.type	= MTYPE_TEXT;
	s_arcade.gtDescription[3].generic.flags	= QMF_SMALLFONT|QMF_LEFT_JUSTIFY|QMF_INACTIVE|QMF_GRAYED;
	s_arcade.gtDescription[3].generic.x		= 88;
	s_arcade.gtDescription[3].generic.y		= 80 + SMALLCHAR_HEIGHT*4;
	s_arcade.gtDescription[3].string  		= "(Insert short gametype description here.)";
	s_arcade.gtDescription[3].color  		= text_banner_color;
	s_arcade.gtDescription[3].style  		= UI_LEFT;
#endif

	s_arcade.map.generic.type			= MTYPE_SPINCONTROL;
	s_arcade.map.generic.name			= "Level:";
	s_arcade.map.generic.flags			= QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_LEFT_JUSTIFY;
	s_arcade.map.generic.callback		= StartArcade_MapEvent;
	s_arcade.map.generic.id				= ID_MAP;
	s_arcade.map.generic.x				= 64;
	s_arcade.map.generic.y				= 224 - (SMALLCHAR_HEIGHT+2);
	s_arcade.map.itemnames				= gametype_items; // Poor fix for SpinControl_Init // (const char **)s_arcade.mapNames;

	s_arcade.mappic.generic.type		= MTYPE_BITMAP;
	s_arcade.mappic.generic.flags		= QMF_LEFT_JUSTIFY|QMF_INACTIVE;
	s_arcade.mappic.generic.x			= 640-64-192;
	s_arcade.mappic.generic.y			= 80;
	s_arcade.mappic.width				= 192;
	s_arcade.mappic.height				= 144;
	s_arcade.mappic.errorpic			= GAMESERVER_UNKNOWNMAP;

	if( s_arcade.multiplayer ) {
		y = 224+SMALLCHAR_HEIGHT+2;

		s_arcade.hostname.generic.type			= MTYPE_FIELD;
		s_arcade.hostname.generic.name			= "Server Name:";
		s_arcade.hostname.generic.flags			= QMF_SMALLFONT|QMF_LEFT_JUSTIFY;
		s_arcade.hostname.generic.x				= RIGHT_OPTIONS_X;
		s_arcade.hostname.generic.y				= y;
		s_arcade.hostname.field.widthInChars	= 20;
		s_arcade.hostname.field.maxchars		= 64;
		y += SMALLCHAR_HEIGHT+2;

		s_arcade.pure.generic.type			= MTYPE_RADIOBUTTON;
		s_arcade.pure.generic.flags			= QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_LEFT_JUSTIFY;
		s_arcade.pure.generic.x				= RIGHT_OPTIONS_X;
		s_arcade.pure.generic.y				= y;
		s_arcade.pure.generic.name			= "Pure Server:";
		if (!trap_Cvar_VariableValue( "fs_pure" )) {
			// Don't let users think they can modify sv_pure, it won't work.
			s_arcade.pure.generic.flags |= QMF_GRAYED;
		}
		y += SMALLCHAR_HEIGHT+2;

		s_arcade.publicserver.generic.type	= MTYPE_RADIOBUTTON;
		s_arcade.publicserver.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_LEFT_JUSTIFY;
		s_arcade.publicserver.generic.x		= RIGHT_OPTIONS_X;
		s_arcade.publicserver.generic.y		= y;
		s_arcade.publicserver.generic.name	= "Advertise on Internet:";
		y += SMALLCHAR_HEIGHT+2;

		s_arcade.dedicated.generic.type		= MTYPE_RADIOBUTTON;
		s_arcade.dedicated.generic.id		= ID_DEDICATED;
		s_arcade.dedicated.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_LEFT_JUSTIFY;
		s_arcade.dedicated.generic.callback	= StartArcade_Event;
		s_arcade.dedicated.generic.x		= RIGHT_OPTIONS_X;
		s_arcade.dedicated.generic.y		= y;
		s_arcade.dedicated.generic.name		= "Dedicated:";

		if (s_arcade.inGame) {
			s_arcade.dedicated.generic.flags |= QMF_GRAYED;
		}

		y += SMALLCHAR_HEIGHT+2;

		// Scorelimit and capturelimit have the same x/y, only show one at a time!
		s_arcade.scorelimit.generic.type       = MTYPE_FIELD;
		s_arcade.scorelimit.generic.name       = "Score Limit:";
		s_arcade.scorelimit.generic.flags      = QMF_NUMBERSONLY|QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_LEFT_JUSTIFY;
		s_arcade.scorelimit.generic.x	       = RIGHT_OPTIONS_X;
		s_arcade.scorelimit.generic.y	       = y;
		s_arcade.scorelimit.generic.statusbar  = StartArcade_StatusBar;
		s_arcade.scorelimit.field.widthInChars = 4;
		s_arcade.scorelimit.field.maxchars     = 4;

		s_arcade.flaglimit.generic.type       = MTYPE_FIELD;
		s_arcade.flaglimit.generic.name       = "Capture Limit:";
		s_arcade.flaglimit.generic.flags      = QMF_NUMBERSONLY|QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_LEFT_JUSTIFY;
		s_arcade.flaglimit.generic.x	      = RIGHT_OPTIONS_X;
		s_arcade.flaglimit.generic.y	      = y;
		s_arcade.flaglimit.generic.statusbar  = StartArcade_StatusBar;
		s_arcade.flaglimit.field.widthInChars = 3;
		s_arcade.flaglimit.field.maxchars     = 3;

		y += SMALLCHAR_HEIGHT+2;
		s_arcade.timelimit.generic.type       = MTYPE_FIELD;
		s_arcade.timelimit.generic.name       = "Time Limit:";
		s_arcade.timelimit.generic.flags      = QMF_NUMBERSONLY|QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_LEFT_JUSTIFY;
		s_arcade.timelimit.generic.x	      = RIGHT_OPTIONS_X;
		s_arcade.timelimit.generic.y	      = y;
		s_arcade.timelimit.generic.statusbar  = StartArcade_StatusBar;
		s_arcade.timelimit.field.widthInChars = 3;
		s_arcade.timelimit.field.maxchars     = 3;

		y += SMALLCHAR_HEIGHT+2;
		s_arcade.friendlyfire.generic.type    = MTYPE_RADIOBUTTON;
		s_arcade.friendlyfire.generic.flags   = QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_LEFT_JUSTIFY;
		s_arcade.friendlyfire.generic.x	      = RIGHT_OPTIONS_X;
		s_arcade.friendlyfire.generic.y	      = y;
		s_arcade.friendlyfire.generic.name	  = "Team Damage:";
	} else {
		// Record demo option
		y = 224+SMALLCHAR_HEIGHT+2;
		s_arcade.recordreplay.generic.type		= MTYPE_RADIOBUTTON;
		s_arcade.recordreplay.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_LEFT_JUSTIFY;
		s_arcade.recordreplay.generic.x			= LEFT_OPTIONS_X;
		s_arcade.recordreplay.generic.y			= y;
		s_arcade.recordreplay.generic.name		= "Record Replay:";
		s_arcade.recordreplay.generic.statusbar	= StartArcade_StatusBar;
		s_arcade.recordreplay.generic.id		= ID_RECORDREPLAY;

		y += SMALLCHAR_HEIGHT+2;
		s_arcade.viewreplay.generic.type		= MTYPE_PTEXT;
		s_arcade.viewreplay.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_LEFT_JUSTIFY|QMF_GRAYED;
		s_arcade.viewreplay.generic.x			= LEFT_OPTIONS_X;
		s_arcade.viewreplay.generic.y			= y;
		s_arcade.viewreplay.generic.callback	= StartArcade_ViewReplayEvent;
		s_arcade.viewreplay.string  			= "Watch Replay";
		s_arcade.viewreplay.color  				= color_white;
		s_arcade.viewreplay.style  				= UI_SMALLFONT|UI_LEFT;

		y = 224+SMALLCHAR_HEIGHT+2;
		for (i = 0; i < NUM_ARCADE_SCORES; ++i) {
			s_arcade.scoreId[i].generic.type		= MTYPE_TEXT;
			s_arcade.scoreId[i].generic.flags		= QMF_SMALLFONT|QMF_LEFT_JUSTIFY;
			s_arcade.scoreId[i].generic.x			= 320 - SMALLCHAR_WIDTH*2;
			s_arcade.scoreId[i].generic.y			= y;
			s_arcade.scoreId[i].string  			= scoreIds[i];
			s_arcade.scoreId[i].color  				= color_white;
			s_arcade.scoreId[i].style  				= UI_SMALLFONT|UI_LEFT;

			s_arcade.scoreNum[i].generic.type		= MTYPE_TEXT;
			s_arcade.scoreNum[i].generic.flags		= QMF_SMALLFONT|QMF_RIGHT_JUSTIFY;
			s_arcade.scoreNum[i].generic.x			= 320+SMALLCHAR_WIDTH*11;
			s_arcade.scoreNum[i].generic.y			= y;
			s_arcade.scoreNum[i].string  			= scoreNums[i];
			s_arcade.scoreNum[i].color  			= color_white;
			s_arcade.scoreNum[i].style  			= UI_SMALLFONT|UI_RIGHT;

			s_arcade.scoreCharacter[i].generic.type		= MTYPE_BITMAP;
			s_arcade.scoreCharacter[i].generic.flags	= QMF_LEFT_JUSTIFY|QMF_INACTIVE;
			s_arcade.scoreCharacter[i].generic.x		= 320+SMALLCHAR_WIDTH*(11+7) - 48;
			s_arcade.scoreCharacter[i].generic.y		= y - 12;
			s_arcade.scoreCharacter[i].width			= 32;
			s_arcade.scoreCharacter[i].height			= 32;

			s_arcade.scoreTime[i].generic.type		= MTYPE_TEXT;
			s_arcade.scoreTime[i].generic.flags		= QMF_SMALLFONT|QMF_RIGHT_JUSTIFY;
			s_arcade.scoreTime[i].generic.x			= 320+32+SMALLCHAR_WIDTH*(11+7);
			s_arcade.scoreTime[i].generic.y			= y;
			s_arcade.scoreTime[i].string  			= scoreTimes[i];
			s_arcade.scoreTime[i].color  			= color_white;
			s_arcade.scoreTime[i].style  			= UI_SMALLFONT|UI_RIGHT;

			s_arcade.scoreName[i].generic.type		= MTYPE_TEXT;
			s_arcade.scoreName[i].generic.flags		= QMF_SMALLFONT|QMF_LEFT_JUSTIFY;
			s_arcade.scoreName[i].generic.x			= 320+32+SMALLCHAR_WIDTH*(11+7+2);
			s_arcade.scoreName[i].generic.y			= y;
			s_arcade.scoreName[i].string  			= scoreNames[i];
			s_arcade.scoreName[i].color  			= color_white;
			s_arcade.scoreName[i].style  			= UI_SMALLFONT|UI_LEFT;

			y += 32;
		}
	}

	s_arcade.back.generic.type		= MTYPE_BITMAP;
	s_arcade.back.generic.name		= GAMESERVER_BACK0;
	s_arcade.back.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_arcade.back.generic.callback	= StartArcade_Event;
	s_arcade.back.generic.id		= ID_BACK;
	s_arcade.back.generic.x			= 0;
	s_arcade.back.generic.y			= 480-64;
	s_arcade.back.width				= 128;
	s_arcade.back.height			= 64;
	s_arcade.back.focuspic			= GAMESERVER_BACK1;

	s_arcade.next.generic.type		= MTYPE_BITMAP;
	s_arcade.next.generic.name		= GAMESERVER_NEXT0;
	s_arcade.next.generic.flags		= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_arcade.next.generic.callback	= StartArcade_Event;
	s_arcade.next.generic.id		= ID_NEXT;
	s_arcade.next.generic.x			= 640;
	s_arcade.next.generic.y			= 480-64;
	s_arcade.next.width				= 128;
	s_arcade.next.height			= 64;
	s_arcade.next.focuspic			= GAMESERVER_NEXT1;

	Menu_AddItem( &s_arcade.menu, &s_arcade.banner );

	Menu_AddItem( &s_arcade.menu, &s_arcade.gametype );
#if 0
	Menu_AddItem( &s_arcade.menu, &s_arcade.gtDescription[0] );
	Menu_AddItem( &s_arcade.menu, &s_arcade.gtDescription[1] );
	Menu_AddItem( &s_arcade.menu, &s_arcade.gtDescription[2] );
	Menu_AddItem( &s_arcade.menu, &s_arcade.gtDescription[3] );
#endif
	Menu_AddItem( &s_arcade.menu, &s_arcade.map );
	Menu_AddItem( &s_arcade.menu, &s_arcade.mappic );

	if( s_arcade.multiplayer ) {
		Menu_AddItem( &s_arcade.menu, &s_arcade.hostname );
		Menu_AddItem( &s_arcade.menu, &s_arcade.pure );
		Menu_AddItem( &s_arcade.menu, &s_arcade.publicserver );
		Menu_AddItem( &s_arcade.menu, &s_arcade.dedicated );

		Menu_AddItem( &s_arcade.menu, &s_arcade.scorelimit );
		Menu_AddItem( &s_arcade.menu, &s_arcade.flaglimit );
		Menu_AddItem( &s_arcade.menu, &s_arcade.timelimit );
		Menu_AddItem( &s_arcade.menu, &s_arcade.friendlyfire );
	} else {
		Menu_AddItem( &s_arcade.menu, &s_arcade.recordreplay);
		Menu_AddItem( &s_arcade.menu, &s_arcade.viewreplay);

		for (i = 0; i < NUM_ARCADE_SCORES; ++i) {
			Menu_AddItem( &s_arcade.menu, &s_arcade.scoreId[i]);
			Menu_AddItem( &s_arcade.menu, &s_arcade.scoreNum[i]);
			Menu_AddItem( &s_arcade.menu, &s_arcade.scoreCharacter[i]);
			Menu_AddItem( &s_arcade.menu, &s_arcade.scoreTime[i]);
			Menu_AddItem( &s_arcade.menu, &s_arcade.scoreName[i]);
		}
	}

	Menu_AddItem( &s_arcade.menu, &s_arcade.back );
	Menu_AddItem( &s_arcade.menu, &s_arcade.next );

	StartArcade_SetMenuItems();

	if (s_arcade.inGame) {
		s_arcade.gametype.curvalue	= gametypeToList[(int)Com_Clamp(0, ARRAY_LEN(gametypeToList) - 1,
												trap_Cvar_VariableValue("g_gametype"))];

		s_arcade.next.generic.name		= GAMESERVER_PLAY0;
		s_arcade.next.focuspic			= GAMESERVER_PLAY1;
	}

	StartArcade_GametypeEvent(&s_arcade.gametype, QM_ACTIVATED);
}

/*
=================
StartArcade_Cache
=================
*/
void StartServer_Cache( void ) {
	trap_R_RegisterShaderNoMip( GAMESERVER_BACK0 );
	trap_R_RegisterShaderNoMip( GAMESERVER_BACK1 );
	trap_R_RegisterShaderNoMip( GAMESERVER_NEXT0 );
	trap_R_RegisterShaderNoMip( GAMESERVER_NEXT1 );
	trap_R_RegisterShaderNoMip( GAMESERVER_UNKNOWNMAP );
}


/*
=================
UI_StartArcade
=================
*/
void UI_StartServerMenu( qboolean multiplayer ) {
	StartArcade_MenuInit( multiplayer );
	UI_PushMenu( &s_arcade.menu );
}
#else
#define GAMESERVER_BACK0		"menu/art/back_0"
#define GAMESERVER_BACK1		"menu/art/back_1"
#define GAMESERVER_NEXT0		"menu/art/next_0"
#define GAMESERVER_NEXT1		"menu/art/next_1"
#define GAMESERVER_FRAMEL		"menu/art/frame2_l"
#define GAMESERVER_FRAMER		"menu/art/frame1_r"
#define GAMESERVER_SELECT		"menu/art/maps_select"
#define GAMESERVER_SELECTED		"menu/art/maps_selected"
#ifdef TA_DATA // MAPS_SELECT_SAME_AS_MAPS_SELECTED
#define GAMESERVER_SELECTED_PULSE "menu/art/maps_selected_pulse"
#endif
#ifdef TA_MISC // NO_MENU_FIGHT
#define GAMESERVER_FIGHT0		"menu/art/play_0"
#define GAMESERVER_FIGHT1		"menu/art/play_1"
#else
#define GAMESERVER_FIGHT0		"menu/art/fight_0"
#define GAMESERVER_FIGHT1		"menu/art/fight_1"
#endif
#define GAMESERVER_UNKNOWNMAP	"menu/art/unknownmap"
#ifdef TA_DATA
#define GAMESERVER_ARROWS		"menu/art/arrows_horz_0"
#define GAMESERVER_ARROWSL		"menu/art/arrows_horz_left"
#define GAMESERVER_ARROWSR		"menu/art/arrows_horz_right"
#define GAMESERVER_ARROWS_WIDTH 128
#define GAMESERVER_ARROWS_HEIGHT 64
#else
#define GAMESERVER_ARROWS		"menu/art/gs_arrows_0"
#define GAMESERVER_ARROWSL		"menu/art/gs_arrows_l"
#define GAMESERVER_ARROWSR		"menu/art/gs_arrows_r"
#endif

#define MAX_MAPROWS		2
#define MAX_MAPCOLS		4
#define MAX_MAPSPERPAGE	8

#define MAX_NAMELENGTH	16
#define ID_GAMETYPE				10
#define ID_PICTURES				11	// 12, 13, 14
#define ID_PREVPAGE				15
#define ID_NEXTPAGE				16
#define ID_STARTSERVERBACK		17
#define ID_STARTSERVERNEXT		18

typedef struct {
	menuframework_s	menu;

	menutext_s		banner;
	menubitmap_s	framel;
	menubitmap_s	framer;

	menulist_s		gametype;
	menubitmap_s	mappics[MAX_MAPSPERPAGE];
	menubitmap_s	mapbuttons[MAX_MAPSPERPAGE];
	menubitmap_s	arrows;
	menubitmap_s	prevpage;
	menubitmap_s	nextpage;
	menutext_s		pagecount;
	menubitmap_s	back;
	menubitmap_s	next;

	menutext_s		mapname;
	menubitmap_s	item_null;

	qboolean		multiplayer;
	int				currentmap;
	char			currentmapname[MAX_QPATH];
	int				nummaps;
	int				page;
	int				maxpages;
	int 			maplist[MAX_ARENAS];
} startserver_t;

static startserver_t s_startserver;

#ifdef TURTLEARENA
// Names of gametypes in "gametype select list", can be any gametypes in any order
static const char *gametype_items[] = {
	"Free For All",
	"Duel", // tornament to duel // "Tournament",
	"Team Deathmatch",
	"Capture the Flag",
#ifdef MISSIONPACK
	"1 Flag CTF",
	"Overload",
#ifdef MISSIONPACK_HARVESTER
	"Harvester",
#endif
#endif
	"Cooperative",
	NULL
};

// Order of gametypes in "gametype select list", must match gametype_items
static int gametype_remap[] = {
	GT_FFA,
	GT_TOURNAMENT,
	GT_TEAM,
	GT_CTF
#ifdef MISSIONPACK
	,GT_1FCTF
	,GT_OBELISK
#ifdef MISSIONPACK_HARVESTER
	,GT_HARVESTER
#endif
#endif
	GT_SINGLE_PLAYER,
};

// Order of gametype_items, convert GT_* to gametype_items index
static int gametype_remap2[] = {
	0,		// Free For All
	1,		// Duel
	ARRAY_LEN(gametype_items)-2, // Cooperative
	2,		// Team Deathmatch
	3		// Capture the Flag
#ifdef MISSIONPACK
	,4		// 1 Flag CTF
	,5		// Overload
#ifdef MISSIONPACK_HARVESTER
	,6		// Harvester
#endif
#endif
};

// Don't allow sp/coop in arcade mode
static const char *arcade_gametype_items[] = {
	"Free For All",
	"Duel", // tornament to duel // "Tournament",
	"Team Deathmatch",
	"Capture the Flag",
#ifdef MISSIONPACK
	"1 Flag CTF",
	"Overload",
#ifdef MISSIONPACK_HARVESTER
	"Harvester",
#endif
#endif
	"", // No coop in arcade
	NULL
};
#else
static const char *gametype_items[] = {
	"Free For All",
	"Team Deathmatch",
#ifdef TA_MISC // tornament to duel
	"Duel",
#else
	"Tournament",
#endif
	"Capture the Flag",
#ifdef MISSIONPACK
	"1 Flag CTF",
	"Overload",
#ifdef MISSIONPACK_HARVESTER
	"Harvester",
#endif
#endif
	NULL
};

static int gametype_remap[] = {GT_FFA, GT_TEAM, GT_TOURNAMENT, GT_CTF
#ifdef MISSIONPACK
,GT_1FCTF,GT_OBELISK
#ifdef MISSIONPACK_HARVESTER
,GT_HARVESTER
#endif
#endif
};
static int gametype_remap2[] = {0, 2, 0, 1, 3
#ifdef MISSIONPACK
,4,5
#ifdef MISSIONPACK_HARVESTER
,6
#endif
#endif
};
#endif

static void UI_ServerOptionsMenu( qboolean multiplayer );


/*
=================
GametypeBits
=================
*/
static int GametypeBits( char *string ) {
	int		bits;
	char	*p;
	char	*token;

	bits = 0;
	p = string;
	while( 1 ) {
		token = COM_ParseExt( &p, qfalse );
		if ( !token[0] ) {
			break;
		}

		if( Q_stricmp( token, "ffa" ) == 0 ) {
			bits |= 1 << GT_FFA;
			continue;
		}

		if( Q_stricmp( token, "tourney" ) == 0 ) {
			bits |= 1 << GT_TOURNAMENT;
			continue;
		}

		if( Q_stricmp( token, "single" ) == 0 ) {
			bits |= 1 << GT_SINGLE_PLAYER;
			continue;
		}

		if( Q_stricmp( token, "team" ) == 0 ) {
			bits |= 1 << GT_TEAM;
			continue;
		}

		if( Q_stricmp( token, "ctf" ) == 0 ) {
			bits |= 1 << GT_CTF;
			continue;
		}

#ifdef MISSIONPACK
		if( Q_stricmp( token, "oneflag" ) == 0 ) {
			bits |= 1 << GT_1FCTF;
			continue;
		}

		if( Q_stricmp( token, "overload" ) == 0 ) {
			bits |= 1 << GT_OBELISK;
			continue;
		}

#ifdef MISSIONPACK_HARVESTER
		if( Q_stricmp( token, "harvester" ) == 0 ) {
			bits |= 1 << GT_HARVESTER;
			continue;
		}
#endif
#endif
	}

	return bits;
}


/*
=================
StartServer_Update
=================
*/
static void StartServer_Update( void ) {
	int				i;
	int				top;
	static	char	picname[MAX_MAPSPERPAGE][64];
	const char		*info;
	char			mapname[MAX_NAMELENGTH];

	top = s_startserver.page*MAX_MAPSPERPAGE;

	for (i=0; i<MAX_MAPSPERPAGE; i++)
	{
		if (top+i >= s_startserver.nummaps)
			break;
		
		info = UI_GetArenaInfoByNumber( s_startserver.maplist[ top + i ]);
		Q_strncpyz( mapname, Info_ValueForKey( info, "map"), MAX_NAMELENGTH );

		Com_sprintf( picname[i], sizeof(picname[i]), "levelshots/%s_small", mapname );
		if ( !trap_R_RegisterShaderNoMip( picname[i] ) ) {
			Com_sprintf( picname[i], sizeof(picname[i]), "levelshots/%s", mapname );
		}

		s_startserver.mappics[i].generic.flags &= ~QMF_HIGHLIGHT;
		s_startserver.mappics[i].generic.name   = picname[i];
		s_startserver.mappics[i].shader         = 0;

		// reset
		s_startserver.mapbuttons[i].generic.flags |= QMF_PULSEIFFOCUS;
		s_startserver.mapbuttons[i].generic.flags &= ~QMF_INACTIVE;
	}

	for (; i<MAX_MAPSPERPAGE; i++)
	{
		s_startserver.mappics[i].generic.flags &= ~QMF_HIGHLIGHT;
		s_startserver.mappics[i].generic.name   = NULL;
		s_startserver.mappics[i].shader         = 0;

		// disable
		s_startserver.mapbuttons[i].generic.flags &= ~QMF_PULSEIFFOCUS;
		s_startserver.mapbuttons[i].generic.flags |= QMF_INACTIVE;
	}


	// no servers to start
	if( !s_startserver.nummaps ) {
		s_startserver.next.generic.flags |= QMF_INACTIVE;

		// set the map name
		strcpy( s_startserver.mapname.string, "NO MAPS FOUND" );
	}
	else {
		// set the highlight
		s_startserver.next.generic.flags &= ~QMF_INACTIVE;
		i = s_startserver.currentmap - top;
		if ( i >=0 && i < MAX_MAPSPERPAGE ) 
		{
			s_startserver.mappics[i].generic.flags    |= QMF_HIGHLIGHT;
#ifndef TA_DATA // MENU
			s_startserver.mapbuttons[i].generic.flags &= ~QMF_PULSEIFFOCUS;
#endif
		}

		// set the map name
		Q_strncpyz( s_startserver.mapname.string, s_startserver.currentmapname, MAX_NAMELENGTH);
	}
	
	Q_strupr( s_startserver.mapname.string );

	Com_sprintf( s_startserver.pagecount.string, 64, "Page %d of %d", s_startserver.page + 1, s_startserver.maxpages );
}


/*
=================
StartServer_MapEvent
=================
*/
static void StartServer_MapEvent( void* ptr, int event ) {
	const char	*info;

	if( event != QM_ACTIVATED) {
		return;
	}

	s_startserver.currentmap = (s_startserver.page*MAX_MAPSPERPAGE) + (((menucommon_s*)ptr)->id - ID_PICTURES);

	info = UI_GetArenaInfoByNumber( s_startserver.maplist[ s_startserver.currentmap ] );
	Q_strncpyz( s_startserver.currentmapname, Info_ValueForKey( info, "map" ),
	            sizeof( s_startserver.currentmapname ) );

	StartServer_Update();
}


/*
=================
StartServer_GametypeEvent
=================
*/
static void StartServer_GametypeEvent( void* ptr, int event ) {
	int			i;
	int			count;
	int			gamebits;
	int			matchbits;
	const char	*info;

	if( event != QM_ACTIVATED) {
		return;
	}

	count = UI_GetNumArenas();
	s_startserver.nummaps = 0;
	s_startserver.currentmap = 0;
	matchbits = 1 << gametype_remap[s_startserver.gametype.curvalue];
#ifndef TA_SP // Single player has own gametype on net now.
	if( gametype_remap[s_startserver.gametype.curvalue] == GT_FFA ) {
		matchbits |= ( 1 << GT_SINGLE_PLAYER );
	}
#endif
	for( i = 0; i < count; i++ ) {
		info = UI_GetArenaInfoByNumber( i );
	
		gamebits = GametypeBits( Info_ValueForKey( info, "type") );
		if( !( gamebits & matchbits ) ) {
			continue;
		}

		if ( !Q_stricmp( s_startserver.currentmapname, Info_ValueForKey( info, "map" ) ) ) {
			s_startserver.currentmap = s_startserver.nummaps;
		}

		s_startserver.maplist[ s_startserver.nummaps ] = i;
		s_startserver.nummaps++;
	}
	s_startserver.maxpages = (s_startserver.nummaps + MAX_MAPSPERPAGE-1)/MAX_MAPSPERPAGE;
	s_startserver.page = s_startserver.currentmap / MAX_MAPSPERPAGE;

	info = UI_GetArenaInfoByNumber( s_startserver.maplist[ s_startserver.currentmap ] );
	Q_strncpyz( s_startserver.currentmapname, Info_ValueForKey( info, "map" ),
	            sizeof( s_startserver.currentmapname ) );

	StartServer_Update();
}


/*
=================
StartServer_MenuEvent
=================
*/
static void StartServer_MenuEvent( void* ptr, int event ) {
	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
	case ID_PREVPAGE:
		if( s_startserver.page > 0 ) {
			s_startserver.page--;
			StartServer_Update();
		}
		break;

	case ID_NEXTPAGE:
		if( s_startserver.page < s_startserver.maxpages - 1 ) {
			s_startserver.page++;
			StartServer_Update();
		}
		break;

	case ID_STARTSERVERNEXT:
#ifdef TA_MISC
		// If ingame, don't go to server options
		if (trap_Cvar_VariableValue("sv_running"))
		{
			// the wait commands will allow the dedicated to take effect
			trap_Cmd_ExecuteText( EXEC_APPEND, va( "wait ; wait ; g_gametype %d ; map %s\n", gametype_remap[s_startserver.gametype.curvalue], s_startserver.currentmapname ) );
		} else {
#endif
		UI_ServerOptionsMenu( s_startserver.multiplayer );
#ifdef TA_MISC
		}
#endif
		break;

	case ID_STARTSERVERBACK:
		UI_PopMenu();
		break;
	}
}


/*
===============
StartServer_LevelshotDraw
===============
*/
static void StartServer_LevelshotDraw( void *self ) {
	menubitmap_s	*b;
	int				x;
	int				y;
	int				w;
	int				h;
	int				n;
	const char		*info;
	char			mapname[ MAX_NAMELENGTH ];

	b = (menubitmap_s *)self;

	if( !b->generic.name ) {
		return;
	}

	if( b->generic.name && !b->shader ) {
		b->shader = trap_R_RegisterShaderNoMip( b->generic.name );
		if( !b->shader && b->errorpic ) {
			b->shader = trap_R_RegisterShaderNoMip( b->errorpic );
		}
	}

	if( b->focuspic && !b->focusshader ) {
		b->focusshader = trap_R_RegisterShaderNoMip( b->focuspic );
	}

	x = b->generic.x;
	y = b->generic.y;
	w = b->width;
	h =	b->height;
	if( b->shader ) {
		CG_DrawPic( x, y, w, h, b->shader );
	}

	x = b->generic.x;
	y = b->generic.y + b->height;
	CG_FillRect( x, y, b->width, 28, colorBlack );

	x += b->width / 2;
	y += 4;
	n = s_startserver.page * MAX_MAPSPERPAGE + b->generic.id - ID_PICTURES;

	info = UI_GetArenaInfoByNumber( s_startserver.maplist[ n ]);
	Q_strncpyz( mapname, Info_ValueForKey( info, "map"), MAX_NAMELENGTH );
	Q_strupr( mapname );
	UI_DrawString( x, y, mapname, UI_CENTER|UI_SMALLFONT, color_orange );

	x = b->generic.x;
	y = b->generic.y;
	w = b->width;
	h =	b->height + 28;
	if( b->generic.flags & QMF_HIGHLIGHT ) {	
		CG_DrawPic( x, y, w, h, b->focusshader );
	}
}


/*
=================
StartServer_MenuInit
=================
*/
static void StartServer_MenuInit( qboolean multiplayer ) {
	int	i;
	int	x;
	int	y;
	static char mapnamebuffer[64];
	static char pagecountbuffer[64];
#ifdef TA_MISC
	int inGame = trap_Cvar_VariableValue("sv_running");
#endif

	// zero set all our globals
	memset( &s_startserver, 0 ,sizeof(startserver_t) );

	StartServer_Cache();

	s_startserver.multiplayer = multiplayer;

	s_startserver.menu.wrapAround = qtrue;
	s_startserver.menu.fullscreen = qtrue;

	s_startserver.banner.generic.type  = MTYPE_BTEXT;
	s_startserver.banner.generic.x	   = 320;
	s_startserver.banner.generic.y	   = 16;
#ifdef TA_MISC
	if (inGame)
		s_startserver.banner.string    = "CHANGE MAP";
	else
#endif
#ifdef TA_SP
	if (!multiplayer)
		s_startserver.banner.string    = "ARCADE";
	else
#endif
	s_startserver.banner.string        = "GAME SERVER";
	s_startserver.banner.color         = text_banner_color;
	s_startserver.banner.style         = UI_CENTER;

	s_startserver.framel.generic.type  = MTYPE_BITMAP;
	s_startserver.framel.generic.name  = GAMESERVER_FRAMEL;
	s_startserver.framel.generic.flags = QMF_INACTIVE;
	s_startserver.framel.generic.x	   = 0;  
	s_startserver.framel.generic.y	   = 78;
	s_startserver.framel.width  	   = 256;
	s_startserver.framel.height  	   = 329;

	s_startserver.framer.generic.type  = MTYPE_BITMAP;
	s_startserver.framer.generic.name  = GAMESERVER_FRAMER;
	s_startserver.framer.generic.flags = QMF_INACTIVE;
	s_startserver.framer.generic.x	   = 376;
	s_startserver.framer.generic.y	   = 76;
	s_startserver.framer.width  	   = 256;
	s_startserver.framer.height  	   = 334;

	s_startserver.gametype.generic.type		= MTYPE_SPINCONTROL;
	s_startserver.gametype.generic.name		= "Game Type:";
	s_startserver.gametype.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_startserver.gametype.generic.callback	= StartServer_GametypeEvent;
	s_startserver.gametype.generic.id		= ID_GAMETYPE;
	s_startserver.gametype.generic.x		= 320 - 24;
	s_startserver.gametype.generic.y		= 368;
#ifdef TA_SP
	if (!multiplayer)
		s_startserver.gametype.itemnames	= arcade_gametype_items;
	else
#endif
	s_startserver.gametype.itemnames		= gametype_items;
#ifdef TA_MISC
	if (inGame)
		s_startserver.gametype.curvalue		= (int)Com_Clamp( 0, ARRAY_LEN(gametype_remap) - 1, trap_Cvar_VariableValue( "g_gametype" ) );
#endif

	for (i=0; i<MAX_MAPSPERPAGE; i++)
	{
		x = (i % MAX_MAPCOLS) * (128+8) + (SCREEN_WIDTH - (MAX_MAPCOLS * (128+8)) - 8) / 2;
		y = ((i / MAX_MAPCOLS) % MAX_MAPROWS) * (128+8) + 96;

		s_startserver.mappics[i].generic.type   = MTYPE_BITMAP;
		s_startserver.mappics[i].generic.flags  = QMF_LEFT_JUSTIFY|QMF_INACTIVE;
		s_startserver.mappics[i].generic.x	    = x;
		s_startserver.mappics[i].generic.y	    = y;
		s_startserver.mappics[i].generic.id		= ID_PICTURES+i;
		s_startserver.mappics[i].width  		= 128;
		s_startserver.mappics[i].height  	    = 96;
		s_startserver.mappics[i].focuspic       = GAMESERVER_SELECTED;
		s_startserver.mappics[i].errorpic       = GAMESERVER_UNKNOWNMAP;
		s_startserver.mappics[i].generic.ownerdraw = StartServer_LevelshotDraw;

		s_startserver.mapbuttons[i].generic.type     = MTYPE_BITMAP;
		s_startserver.mapbuttons[i].generic.flags    = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_NODEFAULTINIT;
		s_startserver.mapbuttons[i].generic.id       = ID_PICTURES+i;
		s_startserver.mapbuttons[i].generic.callback = StartServer_MapEvent;
#ifdef TA_DATA // MAPS_SELECT_SAME_AS_MAPS_SELECTED
		s_startserver.mapbuttons[i].generic.x	     = x;
		s_startserver.mapbuttons[i].generic.y	     = y;
		s_startserver.mapbuttons[i].width  		     = 128;
		s_startserver.mapbuttons[i].height  	     = 124;
#else
		s_startserver.mapbuttons[i].generic.x	     = x - 30;
		s_startserver.mapbuttons[i].generic.y	     = y - 32;
		s_startserver.mapbuttons[i].width  		     = 256;
		s_startserver.mapbuttons[i].height  	     = 248;
#endif
		s_startserver.mapbuttons[i].generic.left     = x;
		s_startserver.mapbuttons[i].generic.top  	 = y;
		s_startserver.mapbuttons[i].generic.right    = x + 128;
		s_startserver.mapbuttons[i].generic.bottom   = y + 128;
#ifdef TA_DATA // MAPS_SELECT_SAME_AS_MAPS_SELECTED
		s_startserver.mapbuttons[i].focuspic         = GAMESERVER_SELECTED_PULSE;
#else
		s_startserver.mapbuttons[i].focuspic         = GAMESERVER_SELECT;
#endif
	}

	s_startserver.arrows.generic.type  = MTYPE_BITMAP;
	s_startserver.arrows.generic.name  = GAMESERVER_ARROWS;
	s_startserver.arrows.generic.flags = QMF_INACTIVE;
	s_startserver.arrows.generic.x	   = 260;
	s_startserver.arrows.generic.y	   = 400;
#ifdef TA_DATA
	s_startserver.arrows.width  	   = GAMESERVER_ARROWS_WIDTH;
	s_startserver.arrows.height  	   = GAMESERVER_ARROWS_HEIGHT;
#else
	s_startserver.arrows.width  	   = 128;
	s_startserver.arrows.height  	   = 32;
#endif

	s_startserver.prevpage.generic.type	    = MTYPE_BITMAP;
	s_startserver.prevpage.generic.flags    = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_startserver.prevpage.generic.callback = StartServer_MenuEvent;
	s_startserver.prevpage.generic.id	    = ID_PREVPAGE;
	s_startserver.prevpage.generic.x		= 260;
	s_startserver.prevpage.generic.y		= 400;
#ifdef TA_DATA
	s_startserver.prevpage.width  		    = GAMESERVER_ARROWS_WIDTH/2;
	s_startserver.prevpage.height  		    = GAMESERVER_ARROWS_HEIGHT;
#else
	s_startserver.prevpage.width  		    = 64;
	s_startserver.prevpage.height  		    = 32;
#endif
	s_startserver.prevpage.focuspic         = GAMESERVER_ARROWSL;

	s_startserver.nextpage.generic.type	    = MTYPE_BITMAP;
	s_startserver.nextpage.generic.flags    = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_startserver.nextpage.generic.callback = StartServer_MenuEvent;
	s_startserver.nextpage.generic.id	    = ID_NEXTPAGE;
#ifdef TA_DATA
	s_startserver.nextpage.generic.x		= 260+GAMESERVER_ARROWS_WIDTH/2;
#else
	s_startserver.nextpage.generic.x		= 321;
#endif
	s_startserver.nextpage.generic.y		= 400;
#ifdef TA_DATA
	s_startserver.nextpage.width  		    = GAMESERVER_ARROWS_WIDTH/2;
	s_startserver.nextpage.height  		    = GAMESERVER_ARROWS_HEIGHT;
#else
	s_startserver.nextpage.width  		    = 64;
	s_startserver.nextpage.height  		    = 32;
#endif
	s_startserver.nextpage.focuspic         = GAMESERVER_ARROWSR;

	s_startserver.pagecount.generic.type  = MTYPE_TEXT;
	s_startserver.pagecount.generic.flags = QMF_RIGHT_JUSTIFY|QMF_INACTIVE;
	s_startserver.pagecount.generic.x	    = (3 % MAX_MAPCOLS) * (128+8) + (SCREEN_WIDTH - (MAX_MAPCOLS * (128+8)) - 8) / 2 + 128;
	s_startserver.pagecount.generic.y	    = 368;
	s_startserver.pagecount.string        = pagecountbuffer;
	s_startserver.pagecount.style         = UI_RIGHT|UI_SMALLFONT;
	s_startserver.pagecount.color         = text_color_normal;

	s_startserver.mapname.generic.type  = MTYPE_PTEXT;
	s_startserver.mapname.generic.flags = QMF_CENTER_JUSTIFY|QMF_INACTIVE;
	s_startserver.mapname.generic.x	    = 320;
	s_startserver.mapname.generic.y	    = 440;
	s_startserver.mapname.string        = mapnamebuffer;
	s_startserver.mapname.style         = UI_CENTER|UI_BIGFONT;
	s_startserver.mapname.color         = text_color_normal;

	s_startserver.back.generic.type	    = MTYPE_BITMAP;
	s_startserver.back.generic.name     = GAMESERVER_BACK0;
	s_startserver.back.generic.flags    = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_startserver.back.generic.callback = StartServer_MenuEvent;
	s_startserver.back.generic.id	    = ID_STARTSERVERBACK;
	s_startserver.back.generic.x		= 0;
	s_startserver.back.generic.y		= 480-64;
	s_startserver.back.width  		    = 128;
	s_startserver.back.height  		    = 64;
	s_startserver.back.focuspic         = GAMESERVER_BACK1;

	s_startserver.next.generic.type	    = MTYPE_BITMAP;
#ifdef TA_MISC
	if (inGame) {
		s_startserver.next.generic.name = GAMESERVER_FIGHT0;
	} else
#endif
	s_startserver.next.generic.name     = GAMESERVER_NEXT0;
	s_startserver.next.generic.flags    = QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_startserver.next.generic.callback = StartServer_MenuEvent;
	s_startserver.next.generic.id	    = ID_STARTSERVERNEXT;
	s_startserver.next.generic.x		= 640;
	s_startserver.next.generic.y		= 480-64;
	s_startserver.next.width  		    = 128;
	s_startserver.next.height  		    = 64;
#ifdef TA_MISC
	if (inGame) {
		s_startserver.next.focuspic     = GAMESERVER_FIGHT1;
	} else
#endif
	s_startserver.next.focuspic         = GAMESERVER_NEXT1;

	s_startserver.item_null.generic.type	= MTYPE_BITMAP;
	s_startserver.item_null.generic.flags	= QMF_LEFT_JUSTIFY|QMF_MOUSEONLY|QMF_SILENT;
	s_startserver.item_null.generic.x		= 0;
	s_startserver.item_null.generic.y		= 0;
	s_startserver.item_null.width			= 640;
	s_startserver.item_null.height			= 480;

	Menu_AddItem( &s_startserver.menu, &s_startserver.banner );
	Menu_AddItem( &s_startserver.menu, &s_startserver.framel );
	Menu_AddItem( &s_startserver.menu, &s_startserver.framer );

	Menu_AddItem( &s_startserver.menu, &s_startserver.gametype );
	for (i=0; i<MAX_MAPSPERPAGE; i++)
	{
		Menu_AddItem( &s_startserver.menu, &s_startserver.mappics[i] );
		Menu_AddItem( &s_startserver.menu, &s_startserver.mapbuttons[i] );
	}

	Menu_AddItem( &s_startserver.menu, &s_startserver.arrows );
	Menu_AddItem( &s_startserver.menu, &s_startserver.prevpage );
	Menu_AddItem( &s_startserver.menu, &s_startserver.nextpage );
	Menu_AddItem( &s_startserver.menu, &s_startserver.pagecount );
	Menu_AddItem( &s_startserver.menu, &s_startserver.back );
	Menu_AddItem( &s_startserver.menu, &s_startserver.next );
	Menu_AddItem( &s_startserver.menu, &s_startserver.mapname );
	Menu_AddItem( &s_startserver.menu, &s_startserver.item_null );

	if ( trap_Cvar_VariableValue("sv_running") ) {
		int gametype;

		gametype = (int) Com_Clamp(0, ARRAY_LEN(gametype_remap2) - 1,
							trap_Cvar_VariableValue("g_gametype"));
		s_startserver.gametype.curvalue = gametype_remap2[gametype];

		Q_strncpyz( s_startserver.currentmapname, CG_Cvar_VariableString( "mapname" ),
		            sizeof( s_startserver.currentmapname ) );
	}

	StartServer_GametypeEvent( NULL, QM_ACTIVATED );
}


/*
=================
StartServer_Cache
=================
*/
void StartServer_Cache( void )
{
	int				i;
	const char		*info;
	qboolean		precache;
	char			picname[64];
	char			mapname[ MAX_NAMELENGTH ];

	trap_R_RegisterShaderNoMip( GAMESERVER_BACK0 );	
	trap_R_RegisterShaderNoMip( GAMESERVER_BACK1 );	
#ifdef TA_MISC
	if (trap_Cvar_VariableValue("sv_running")) {
		trap_R_RegisterShaderNoMip( GAMESERVER_FIGHT0 );	
		trap_R_RegisterShaderNoMip( GAMESERVER_FIGHT1 );	
	} else
#endif
	{
		trap_R_RegisterShaderNoMip( GAMESERVER_NEXT0 );	
		trap_R_RegisterShaderNoMip( GAMESERVER_NEXT1 );	
	}
	trap_R_RegisterShaderNoMip( GAMESERVER_FRAMEL );	
	trap_R_RegisterShaderNoMip( GAMESERVER_FRAMER );	
#ifdef TA_DATA // MAPS_SELECT_SAME_AS_MAPS_SELECTED
	trap_R_RegisterShaderNoMip( GAMESERVER_SELECTED_PULSE );	
#else
	trap_R_RegisterShaderNoMip( GAMESERVER_SELECT );	
#endif
	trap_R_RegisterShaderNoMip( GAMESERVER_SELECTED );	
	trap_R_RegisterShaderNoMip( GAMESERVER_UNKNOWNMAP );
	trap_R_RegisterShaderNoMip( GAMESERVER_ARROWS );
	trap_R_RegisterShaderNoMip( GAMESERVER_ARROWSL );
	trap_R_RegisterShaderNoMip( GAMESERVER_ARROWSR );

	precache = trap_Cvar_VariableValue("com_buildscript");

	if( precache ) {
		for( i = 0; i < UI_GetNumArenas(); i++ ) {
			info = UI_GetArenaInfoByNumber( i );
			Q_strncpyz( mapname, Info_ValueForKey( info, "map"), MAX_NAMELENGTH );
	
			Com_sprintf( picname, sizeof(picname), "levelshots/%s_small", mapname );
			if ( !trap_R_RegisterShaderNoMip( picname ) ) {
				Com_sprintf( picname, sizeof(picname), "levelshots/%s", mapname );
				trap_R_RegisterShaderNoMip(picname);
			}
		}
	}
}


/*
=================
UI_StartServerMenu
=================
*/
void UI_StartServerMenu( qboolean multiplayer ) {
	StartServer_MenuInit(multiplayer);
	UI_PushMenu( &s_startserver.menu );
}



/*
=============================================================================

SERVER OPTIONS MENU *****

=============================================================================
*/

#define ID_MAXCLIENTS			20
#define ID_DEDICATED			21
#define ID_GO					22
#define ID_BACK					23
#define ID_PLAYER_TYPE			24 // Goes up to ID_PLAYER_TYPE+PLAYER_SLOTS

#define PLAYER_SLOTS			12


typedef struct {
	menuframework_s		menu;

	menutext_s			banner;

	menubitmap_s		mappic;
	menubitmap_s		picframe;

	menuradiobutton_s	publicserver;
	menuradiobutton_s	dedicated;
#ifdef IOQ3ZTM // RECORD_SP_DEMO
	menuradiobutton_s	recorddemo;
#endif
	menufield_s			timelimit;
	menufield_s			fraglimit;
	menufield_s			flaglimit;
	menuradiobutton_s	friendlyfire;
	menufield_s			hostname;
#ifndef TA_WEAPSYS
	menuradiobutton_s	instagib;
#endif
	menuradiobutton_s	pure;
	menulist_s			botSkill;

	menutext_s			player0;
	menulist_s			playerType[PLAYER_SLOTS];
	menutext_s			playerName[PLAYER_SLOTS];
	menulist_s			playerTeam[PLAYER_SLOTS];

	menubitmap_s		go;
	menubitmap_s		next;
	menubitmap_s		back;

	qboolean			multiplayer;
	int					gametype;
	char				mapnamebuffer[32];
	char				playerNameBuffers[MAX_SPLITVIEW][16];
	char				botNameBuffers[PLAYER_SLOTS][16];

	qboolean			newBot;
	int					newBotIndex;
	char				newBotName[16];
} serveroptions_t;

static serveroptions_t s_serveroptions;

#define PT_BOT 0
#define PT_CLOSED 1
#define PT_OPEN 2
#define PT_HUMAN 3

static const char *playerType_list[] = {
	"Bot",
	"----",
	"Open",
	NULL
};

static const char *humanPlayerType_list[] = {
	"Bot",
	"----",
	"Open",
	"Human",
	NULL
};

static const char *playerTeam_list[] = {
	"Blue",
	"Red",
	NULL
};

static const char *botSkill_list[] = {
#ifdef TA_MISC // SRB2_SKILLS
	"Easy",
	"Normal",
	"Hard",
	"Very Hard",
	"Ultimate",
	NULL
#else
	"I Can Win",
	"Bring It On",
	"Hurt Me Plenty",
	"Hardcore",
	"Nightmare!",
	NULL
#endif
};


/*
=================
BotAlreadySelected
=================
*/
static qboolean BotAlreadySelected( const char *checkName ) {
	int		n;

	for( n = 1; n < PLAYER_SLOTS; n++ ) {
		if( s_serveroptions.playerType[n].curvalue != PT_BOT ) {
			continue;
		}
		if( (s_serveroptions.gametype >= GT_TEAM) &&
			(s_serveroptions.playerTeam[n].curvalue != s_serveroptions.playerTeam[s_serveroptions.newBotIndex].curvalue ) ) {
			continue;
		}
		if( Q_stricmp( checkName, s_serveroptions.botNameBuffers[n] ) == 0 ) {
			return qtrue;
		}
	}

	return qfalse;
}


/*
=================
ServerOptions_Start
=================
*/
static void ServerOptions_Start( void ) {
	int		timelimit;
	int		fraglimit;
	int		maxplayers;
	int		localPlayerBits;
	int		publicserver;
	int		dedicated;
	int		friendlyfire;
	int		flaglimit;
#ifndef TA_WEAPSYS
	int		instagib;
#endif
	int		pure;
	int		skill;
	int		n;
	char	buf[64];

	timelimit	 = atoi( MField_Buffer( &s_serveroptions.timelimit.field ) );
	fraglimit	 = atoi( MField_Buffer( &s_serveroptions.fraglimit.field ) );
	flaglimit	 = atoi( MField_Buffer( &s_serveroptions.flaglimit.field ) );
	publicserver = s_serveroptions.publicserver.curvalue;
	dedicated	 = s_serveroptions.dedicated.curvalue;
	friendlyfire = s_serveroptions.friendlyfire.curvalue;
#ifndef TA_WEAPSYS
	instagib	 = s_serveroptions.instagib.curvalue;
#endif
	pure		 = s_serveroptions.pure.curvalue;
	skill		 = s_serveroptions.botSkill.curvalue + 1;

	//set maxplayers
	for( n = 0, maxplayers = 0; n < PLAYER_SLOTS; n++ ) {
		if( s_serveroptions.playerType[n].curvalue == PT_CLOSED ) {
			continue;
		}
		if( (s_serveroptions.playerType[n].curvalue == PT_BOT) && (s_serveroptions.botNameBuffers[n][0] == 0) ) {
			continue;
		}
		maxplayers++;
	}
	for( n = 0, localPlayerBits = 1; n < UI_MaxSplitView(); n++ ) {
		if( s_serveroptions.playerType[n].curvalue != PT_HUMAN ) {
			continue;
		}
		localPlayerBits |= (1<<n);
	}

	trap_Cvar_SetValue( "cl_localPlayers", localPlayerBits );

	switch( s_serveroptions.gametype ) {
	case GT_FFA:
	default:
#ifdef NOTRATEDM // frag to score
		trap_Cvar_SetValue( "ui_ffa_scorelimit", fraglimit );
#else
		trap_Cvar_SetValue( "ui_ffa_fraglimit", fraglimit );
#endif
		trap_Cvar_SetValue( "ui_ffa_timelimit", timelimit );
#ifndef TA_WEAPSYS
		trap_Cvar_SetValue( "ui_ffa_instagib", instagib );
#endif
		break;

	case GT_TOURNAMENT:
#ifdef NOTRATEDM // frag to score
		trap_Cvar_SetValue( "ui_tourney_scorelimit", fraglimit );
#else
		trap_Cvar_SetValue( "ui_tourney_fraglimit", fraglimit );
#endif
		trap_Cvar_SetValue( "ui_tourney_timelimit", timelimit );
#ifndef TA_WEAPSYS
		trap_Cvar_SetValue( "ui_tourney_instagib", instagib );
#endif
		break;

#ifdef TA_SP
	case GT_SINGLE_PLAYER:
		// Co-op settings
		break;
#endif

	case GT_TEAM:
#ifdef NOTRATEDM // frag to score
		trap_Cvar_SetValue( "ui_team_scorelimit", fraglimit );
#else
		trap_Cvar_SetValue( "ui_team_fraglimit", fraglimit );
#endif
		trap_Cvar_SetValue( "ui_team_timelimit", timelimit );
		trap_Cvar_SetValue( "ui_team_friendly", friendlyfire );
#ifndef TA_WEAPSYS
		trap_Cvar_SetValue( "ui_team_instagib", instagib );
#endif
		break;

	case GT_CTF:
		trap_Cvar_SetValue( "ui_ctf_capturelimit", flaglimit );
		trap_Cvar_SetValue( "ui_ctf_timelimit", timelimit );
		trap_Cvar_SetValue( "ui_ctf_friendly", friendlyfire );
#ifndef TA_WEAPSYS
		trap_Cvar_SetValue( "ui_ctf_instagib", instagib );
#endif
		break;

#ifdef MISSIONPACK
	case GT_1FCTF:
		trap_Cvar_SetValue( "ui_1flag_capturelimit", flaglimit );
		trap_Cvar_SetValue( "ui_1flag_timelimit", timelimit );
		trap_Cvar_SetValue( "ui_1flag_friendly", friendlyfire );
#ifndef TA_WEAPSYS
		trap_Cvar_SetValue( "ui_1flag_instagib", instagib );
#endif
		break;

	case GT_OBELISK:
		trap_Cvar_SetValue( "ui_obelisk_capturelimit", flaglimit );
		trap_Cvar_SetValue( "ui_obelisk_timelimit", timelimit );
		trap_Cvar_SetValue( "ui_obelisk_friendly", friendlyfire );
#ifndef TA_WEAPSYS
		trap_Cvar_SetValue( "ui_obelisk_instagib", instagib );
#endif
		break;

#ifdef MISSIONPACK_HARVESTER
	case GT_HARVESTER:
		trap_Cvar_SetValue( "ui_harvester_capturelimit", flaglimit );
		trap_Cvar_SetValue( "ui_harvester_timelimit", timelimit );
		trap_Cvar_SetValue( "ui_harvester_friendly", friendlyfire );
#ifndef TA_WEAPSYS
		trap_Cvar_SetValue( "ui_harvester_instagib", instagib );
#endif
		break;
#endif
#endif
	}

	trap_Cvar_SetValue( "sv_maxclients", Com_Clamp( 0, 12, maxplayers ) );
	if( s_serveroptions.multiplayer ) {
		trap_Cvar_SetValue( "ui_publicServer", Com_Clamp( 0, 1, publicserver ) );
		trap_Cvar_SetValue( "sv_public", Com_Clamp( 0, 1, publicserver ) );
	} else {
		trap_Cvar_SetValue( "sv_public", 0 );
#ifdef TA_SP
		// Aracde mode
		trap_Cvar_SetValue( "ui_singlePlayerActive", 1 );
#endif
	}
	trap_Cvar_SetValue( "dedicated", Com_Clamp( 0, 1, dedicated ) );
	trap_Cvar_SetValue ("timelimit", Com_Clamp( 0, timelimit, timelimit ) );
#ifdef NOTRATEDM // frag to score
	trap_Cvar_SetValue ("scorelimit", Com_Clamp( 0, fraglimit, fraglimit ) );
#else
	trap_Cvar_SetValue ("fraglimit", Com_Clamp( 0, fraglimit, fraglimit ) );
#endif
	trap_Cvar_SetValue ("capturelimit", Com_Clamp( 0, flaglimit, flaglimit ) );
	trap_Cvar_SetValue( "g_friendlyfire", friendlyfire );
#ifndef TA_WEAPSYS
	trap_Cvar_SetValue( "g_instagib", instagib );
#endif
	trap_Cvar_SetValue( "sv_pure", pure );
	trap_Cvar_Set("sv_hostname", MField_Buffer( &s_serveroptions.hostname.field ) );

	// set player's team
	if( dedicated == 0 && s_serveroptions.gametype >= GT_TEAM ) {
		for ( n = 0; n < UI_MaxSplitView(); ++n ) {
			if ( n == 0 || s_serveroptions.playerType[n].curvalue == PT_HUMAN ) {
				trap_Cvar_Set( Com_LocalPlayerCvarName( n, "teampref" ), playerTeam_list[s_serveroptions.playerTeam[n].curvalue] );
			}
		}
	}

	// the wait commands will allow the dedicated to take effect
	trap_Cmd_ExecuteText( EXEC_APPEND, va( "wait ; wait ; g_gametype %d ; map %s\n",
	                      s_serveroptions.gametype, s_startserver.currentmapname ) );

	// remove bots
	if ( trap_Cvar_VariableValue("sv_running") ) {
		trap_Cmd_ExecuteText( EXEC_APPEND, "kickbots\n" );
	}

#ifdef IOQ3ZTM // RECORD_SP_DEMO
	trap_Cvar_SetValue( "ui_recordSPDemo", s_serveroptions.recorddemo.curvalue );
	if (s_serveroptions.recorddemo.curvalue) {
		char buff[MAX_STRING_CHARS];

		Com_sprintf(buff, MAX_STRING_CHARS, "%s_%i", Info_ValueForKey( info, "map" ), s_serveroptions.gametype);
		trap_Cvar_Set("ui_recordSPDemoName", buff);

#ifdef MISSIONPACK
		// If not going to do a warmup and then map_restart, tell client to run demo record command
		if (!trap_Cvar_VariableValue("g_doWarmup"))
#endif
		{
			trap_Cvar_Set("activeAction", va("set g_synchronousclients 1 ; record %s \n", buff));
		}
	}
#endif

	// add bots
	trap_Cmd_ExecuteText( EXEC_APPEND, "wait 3\n" );
	for( n = 1; n < PLAYER_SLOTS; n++ ) {
		if( s_serveroptions.playerType[n].curvalue != PT_BOT ) {
			continue;
		}
		if( s_serveroptions.botNameBuffers[n][0] == 0 ) {
			continue;
		}
		if( s_serveroptions.gametype >= GT_TEAM ) {
			Com_sprintf( buf, sizeof(buf), "addbot %s %i %s\n", s_serveroptions.botNameBuffers[n], skill,
				playerTeam_list[s_serveroptions.playerTeam[n].curvalue] );
		}
		else {
			Com_sprintf( buf, sizeof(buf), "addbot %s %i\n", s_serveroptions.botNameBuffers[n], skill );
		}
		trap_Cmd_ExecuteText( EXEC_APPEND, buf );
	}
}


/*
=================
ServerOptions_InitPlayerItems
=================
*/
static void ServerOptions_InitPlayerItems( void ) {
	int		n;
	int		v;

	// init types
	if( s_serveroptions.multiplayer ) {
		v = PT_OPEN;	// open
	}
	else {
		v = PT_BOT;		// bot
	}
	
	for( n = 0; n < PLAYER_SLOTS; n++ ) {
		s_serveroptions.playerType[n].curvalue = v;
	}

	if( s_serveroptions.multiplayer && (s_serveroptions.gametype < GT_TEAM) ) {
		for( n = 8; n < PLAYER_SLOTS; n++ ) {
			s_serveroptions.playerType[n].curvalue = PT_CLOSED;
		}
	}

	// if not a dedicated server, first slot is reserved for the human on the server
	if( s_serveroptions.dedicated.curvalue == 0 ) {
		for (n = 0; n < UI_MaxSplitView(); n++) {
#ifdef TA_SP
			if (!s_serveroptions.multiplayer) {
				trap_Cvar_VariableStringBuffer( Com_LocalPlayerCvarName(n, "spmodel"), s_serveroptions.playerNameBuffers[n], sizeof(s_serveroptions.playerNameBuffers[n]) );
				s_serveroptions.playerNameBuffers[n][0] = toupper(s_serveroptions.playerNameBuffers[n][0]);
			} else
#endif
			trap_Cvar_VariableStringBuffer( Com_LocalPlayerCvarName(n, "name"), s_serveroptions.playerNameBuffers[n], sizeof(s_serveroptions.playerNameBuffers[n]) );
			Q_CleanStr( s_serveroptions.playerNameBuffers[n] );

			s_serveroptions.playerType[n].curvalue = PT_OPEN;

#ifdef TA_SP
			if (s_serveroptions.multiplayer)
#endif
			if (n == 0) {
				// human
				s_serveroptions.playerType[n].generic.flags |= QMF_INACTIVE;
			}
		}
	}

	// init teams
	if( s_serveroptions.gametype >= GT_TEAM ) {
		for( n = 0; n < (PLAYER_SLOTS / 2); n++ ) {
			s_serveroptions.playerTeam[n].curvalue = 0;
		}
		for( ; n < PLAYER_SLOTS; n++ ) {
			s_serveroptions.playerTeam[n].curvalue = 1;
		}
	}
	else {
		for( n = 0; n < PLAYER_SLOTS; n++ ) {
			s_serveroptions.playerTeam[n].generic.flags |= (QMF_INACTIVE|QMF_HIDDEN);
		}
	}
}


/*
=================
ServerOptions_SetPlayerItems
=================
*/
static void ServerOptions_SetPlayerItems( int playerType ) {
	int		start;
	int		n;

	// types
//	for( n = 0; n < PLAYER_SLOTS; n++ ) {
//		if( (!s_serveroptions.multiplayer) && (n > 0) && (s_serveroptions.playerType[n].curvalue == 0) ) {
//			s_serveroptions.playerType[n].curvalue = PT_BOT;
//		}
//	}

	// names
	if( s_serveroptions.dedicated.curvalue == 0 ) {
		s_serveroptions.player0.string = "Human";
#ifdef TA_SP
		if (!s_serveroptions.multiplayer)
			s_serveroptions.playerName[0].generic.flags &= ~(QMF_INACTIVE|QMF_HIDDEN);
		else
#endif
		s_serveroptions.playerName[0].generic.flags &= ~QMF_HIDDEN;

		for (n = 1; n < UI_MaxSplitView(); n++) {
			s_serveroptions.playerType[n].numitems = ARRAY_LEN(humanPlayerType_list) - 1;

			if (playerType != n && playerType != -1) {
				continue;
			}

			if (s_serveroptions.playerType[n].curvalue == PT_BOT) {
				s_serveroptions.playerName[n].string = s_serveroptions.botNameBuffers[n];
			}
			else if (s_serveroptions.playerType[n].curvalue == PT_HUMAN) {
				s_serveroptions.playerName[n].string = s_serveroptions.playerNameBuffers[n];
			}
		}

		start = 1;
	}
	else {
		s_serveroptions.player0.string = "Open";
		start = 0;

		for (n = 0; n < UI_MaxSplitView(); n++) {
			s_serveroptions.playerType[n].numitems = ARRAY_LEN(playerType_list) - 1;

			if (s_serveroptions.playerType[n].curvalue == PT_HUMAN) {
				s_serveroptions.playerType[n].curvalue = PT_OPEN;
			}
		}
	}
	for( n = start; n < PLAYER_SLOTS; n++ ) {
		if( s_serveroptions.playerType[n].curvalue == PT_HUMAN ) {
#ifdef TA_SP
			if (!s_serveroptions.multiplayer)
				s_serveroptions.playerName[n].generic.flags &= ~(QMF_INACTIVE|QMF_HIDDEN);
			else
#endif
			{
				s_serveroptions.playerName[n].generic.flags |= QMF_INACTIVE;
				s_serveroptions.playerName[n].generic.flags &= ~QMF_HIDDEN;
			}
		} else if( s_serveroptions.playerType[n].curvalue == PT_BOT ) {
			s_serveroptions.playerName[n].generic.flags &= ~(QMF_INACTIVE|QMF_HIDDEN);
		}
		else {
			s_serveroptions.playerName[n].generic.flags |= (QMF_INACTIVE|QMF_HIDDEN);
		}
	}

	// teams
	if( s_serveroptions.gametype < GT_TEAM ) {
		return;
	}
	for( n = start; n < PLAYER_SLOTS; n++ ) {
		if( s_serveroptions.playerType[n].curvalue == PT_CLOSED ) {
			s_serveroptions.playerTeam[n].generic.flags |= (QMF_INACTIVE|QMF_HIDDEN);
		}
		else {
			s_serveroptions.playerTeam[n].generic.flags &= ~(QMF_INACTIVE|QMF_HIDDEN);
		}
	}
}


/*
=================
ServerOptions_Event
=================
*/
static void ServerOptions_Event( void* ptr, int event ) {
	if (((menucommon_s*)ptr)->id >= ID_PLAYER_TYPE && ((menucommon_s*)ptr)->id < ID_PLAYER_TYPE+PLAYER_SLOTS) {
		if( event != QM_ACTIVATED ) {
			return;
		}
		ServerOptions_SetPlayerItems(((menucommon_s*)ptr)->id - ID_PLAYER_TYPE);
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
	
	//if( event != QM_ACTIVATED && event != QM_LOSTFOCUS) {
	//	return;
	//}

	case ID_MAXCLIENTS:
	case ID_DEDICATED:
		ServerOptions_SetPlayerItems(-2);
		break;
	case ID_GO:
		if( event != QM_ACTIVATED ) {
			break;
		}
		ServerOptions_Start();
		break;

	case ID_STARTSERVERNEXT:
		if( event != QM_ACTIVATED ) {
			break;
		}
		break;
	case ID_BACK:
		if( event != QM_ACTIVATED ) {
			break;
		}
		UI_PopMenu();
		break;
	}
}


static void ServerOptions_PlayerNameEvent( void* ptr, int event ) {
	int		n;

	if( event != QM_ACTIVATED ) {
		return;
	}
	n = ((menutext_s*)ptr)->generic.id;
#ifdef TA_SP
	if (s_serveroptions.dedicated.curvalue == 0 && n < MAX_SPLITVIEW )
	{
		int i, j;

		for (i = 0, j = 0; i < NUM_SP_CHARACTERS; i++)
		{
			if (Q_stricmp(s_serveroptions.playerNameBuffers[n], spCharacterNames[i]) == 0)
			{
				j = (i+1)%NUM_SP_CHARACTERS; // next model
			}
		}

		trap_Cvar_Set(Com_LocalPlayerCvarName(n, "spmodel"), spCharacterNames[j]);
		strcpy(s_serveroptions.playerNameBuffers[n], spCharacterNames[j]);
		s_serveroptions.playerNameBuffers[n][0] = toupper(spCharacterNames[j][0]);
		return;
	}
#endif
	s_serveroptions.newBotIndex = n;
	UI_BotSelectMenu( s_serveroptions.botNameBuffers[n] );
}


/*
=================
ServerOptions_StatusBar
=================
*/
static void ServerOptions_StatusBar( void* ptr ) {
	switch( ((menucommon_s*)ptr)->id ) {
	default:
		UI_DrawString( 320, 440, "0 = NO LIMIT", UI_CENTER|UI_SMALLFONT, colorWhite );
		break;
	}
}


/*
===============
ServerOptions_LevelshotDraw
===============
*/
static void ServerOptions_LevelshotDraw( void *self ) {
	menubitmap_s	*b;
	int				x;
	int				y;

	// strange place for this, but it works
	if( s_serveroptions.newBot ) {
		Q_strncpyz( s_serveroptions.botNameBuffers[s_serveroptions.newBotIndex], s_serveroptions.newBotName, 16 );
		s_serveroptions.newBot = qfalse;
	}

	b = (menubitmap_s *)self;

	Bitmap_Draw( b );

	x = b->generic.x;
	y = b->generic.y + b->height;
	CG_FillRect( x, y, b->width, 40, colorBlack );

	x += b->width / 2;
	y += 4;
	UI_DrawString( x, y, s_serveroptions.mapnamebuffer, UI_CENTER|UI_SMALLFONT, color_orange );

	y += SMALLCHAR_HEIGHT;
	UI_DrawString( x, y, gametype_items[gametype_remap2[s_serveroptions.gametype]], UI_CENTER|UI_SMALLFONT, color_orange );
}


static void ServerOptions_InitBotNames( void ) {
	int			count;
	int			n;
	const char	*arenaInfo;
	const char	*botInfo;
	char		*p;
	char		*bot;
	char		bots[MAX_INFO_STRING];

	// set the bot slots to random
	for( n = 1; n < PLAYER_SLOTS; n++ ) {
		strcpy( s_serveroptions.botNameBuffers[n], "Random" );
	}

	if( s_serveroptions.gametype >= GT_TEAM ) {
#ifndef TA_DATA
		Q_strncpyz( s_serveroptions.botNameBuffers[1], "Grunt", 16 );
		Q_strncpyz( s_serveroptions.botNameBuffers[2], "Major", 16 );
		if( s_serveroptions.gametype == GT_TEAM ) {
			Q_strncpyz( s_serveroptions.botNameBuffers[3], "Visor", 16 );
		}
		else
#else
		if( s_serveroptions.gametype != GT_TEAM )
#endif
		{
			s_serveroptions.playerType[3].curvalue = PT_CLOSED;
		}
		s_serveroptions.playerType[4].curvalue = PT_CLOSED;
		s_serveroptions.playerType[5].curvalue = PT_CLOSED;

#ifndef TA_DATA
		Q_strncpyz( s_serveroptions.botNameBuffers[6], "Sarge", 16 );
		Q_strncpyz( s_serveroptions.botNameBuffers[7], "Grunt", 16 );
		Q_strncpyz( s_serveroptions.botNameBuffers[8], "Major", 16 );
		if( s_serveroptions.gametype == GT_TEAM ) {
			Q_strncpyz( s_serveroptions.botNameBuffers[9], "Visor", 16 );
		}
		else
#else
		if( s_serveroptions.gametype != GT_TEAM )
#endif
		{
			s_serveroptions.playerType[9].curvalue = PT_CLOSED;
		}
		s_serveroptions.playerType[10].curvalue = PT_CLOSED;
		s_serveroptions.playerType[11].curvalue = PT_CLOSED;

		return;
	}

	count = UI_MaxSplitView();	// skip the first few slots, reserved for humans

#ifdef TA_SP
	if (!s_serveroptions.multiplayer) {
#endif
	// get info for this map
	arenaInfo = UI_GetArenaInfoByMap( s_serveroptions.mapnamebuffer );

	// get the bot info - we'll seed with them if any are listed
	Q_strncpyz( bots, Info_ValueForKey( arenaInfo, "bots" ), sizeof(bots) );
	p = &bots[0];
	while( *p && count < PLAYER_SLOTS ) {
		//skip spaces
		while( *p && *p == ' ' ) {
			p++;
		}
		if( !*p ) {
			break;
		}

		// mark start of bot name
		bot = p;

		// skip until space of null
		while( *p && *p != ' ' ) {
			p++;
		}
		if( *p ) {
			*p++ = 0;
		}

		botInfo = UI_GetBotInfoByName( bot );
		if( !botInfo )
		{
			botInfo = UI_GetBotInfoByNumber( count );
		}
		bot = Info_ValueForKey( botInfo, "name" );

		Q_strncpyz( s_serveroptions.botNameBuffers[count], bot, sizeof(s_serveroptions.botNameBuffers[count]) );
		count++;
	}

#ifdef TA_SP
		// If no bots, open 3 Random bots.
		if (count == UI_MaxSplitView()) {
			for( ;count < UI_MaxSplitView()+3; count++ ) {
				s_serveroptions.playerType[count].curvalue = PT_BOT;
			}
		}
	}
#endif

	// pad up to #8 as open slots
	for( ;count < 8; count++ ) {
		s_serveroptions.playerType[count].curvalue = PT_OPEN;
	}

	// close off the rest by default
	for( ;count < PLAYER_SLOTS; count++ ) {
		if( s_serveroptions.playerType[count].curvalue == PT_BOT ) {
			s_serveroptions.playerType[count].curvalue = PT_CLOSED;
		}
	}
}


/*
=================
ServerOptions_SetMenuItems
=================
*/
static void ServerOptions_SetMenuItems( void ) {
	static char picname[64];

	switch( s_serveroptions.gametype ) {
	case GT_FFA:
	default:
#ifdef NOTRATEDM // frag to score
		MField_SetText( &s_serveroptions.fraglimit.field, va( "%i", (int)Com_Clamp( 0, 9999, trap_Cvar_VariableValue( "ui_ffa_scorelimit" ) ) ) );
#else
		MField_SetText( &s_serveroptions.fraglimit.field, va( "%i", (int)Com_Clamp( 0, 999, trap_Cvar_VariableValue( "ui_ffa_fraglimit" ) ) ) );
#endif
		MField_SetText( &s_serveroptions.timelimit.field, va( "%i", (int)Com_Clamp( 0, 999, trap_Cvar_VariableValue( "ui_ffa_timelimit" ) ) ) );
#ifndef TA_WEAPSYS
		s_serveroptions.instagib.curvalue = (int)Com_Clamp( 0, 1, trap_Cvar_VariableValue( "ui_ffa_instagib" ) );
#endif
		break;

	case GT_TOURNAMENT:
#ifdef NOTRATEDM // frag to score
		MField_SetText( &s_serveroptions.fraglimit.field, va( "%i", (int)Com_Clamp( 0, 9999, trap_Cvar_VariableValue( "ui_tourney_scorelimit" ) ) ) );
#else
		MField_SetText( &s_serveroptions.fraglimit.field, va( "%i", (int)Com_Clamp( 0, 999, trap_Cvar_VariableValue( "ui_tourney_fraglimit" ) ) ) );
#endif
		MField_SetText( &s_serveroptions.timelimit.field, va( "%i", (int)Com_Clamp( 0, 999, trap_Cvar_VariableValue( "ui_tourney_timelimit" ) ) ) );
#ifndef TA_WEAPSYS
		s_serveroptions.instagib.curvalue = (int)Com_Clamp( 0, 1, trap_Cvar_VariableValue( "ui_tourney_instagib" ) );
#endif
		break;

#ifdef TA_SP
	case GT_SINGLE_PLAYER:
		MField_SetText( &s_serveroptions.fraglimit.field, "0" );
		MField_SetText( &s_serveroptions.timelimit.field, "0" );
		s_serveroptions.friendlyfire.curvalue = 0;
#ifndef TA_WEAPSYS
		s_serveroptions.instagib.curvalue = 0;
#endif
		break;
#endif

	case GT_TEAM:
#ifdef NOTRATEDM // frag to score
		MField_SetText( &s_serveroptions.fraglimit.field, va( "%i", (int)Com_Clamp( 0, 9999, trap_Cvar_VariableValue( "ui_team_scorelimit" ) ) ) );
#else
		MField_SetText( &s_serveroptions.fraglimit.field, va( "%i", (int)Com_Clamp( 0, 999, trap_Cvar_VariableValue( "ui_team_fraglimit" ) ) ) );
#endif
		MField_SetText( &s_serveroptions.timelimit.field, va( "%i", (int)Com_Clamp( 0, 999, trap_Cvar_VariableValue( "ui_team_timelimit" ) ) ) );
		s_serveroptions.friendlyfire.curvalue = (int)Com_Clamp( 0, 1, trap_Cvar_VariableValue( "ui_team_friendly" ) );
#ifndef TA_WEAPSYS
		s_serveroptions.instagib.curvalue = (int)Com_Clamp( 0, 1, trap_Cvar_VariableValue( "ui_team_instagib" ) );
#endif
		break;

	case GT_CTF:
		MField_SetText( &s_serveroptions.flaglimit.field, va( "%i", (int)Com_Clamp( 0, 100, trap_Cvar_VariableValue( "ui_ctf_capturelimit" ) ) ) );
		MField_SetText( &s_serveroptions.timelimit.field, va( "%i", (int)Com_Clamp( 0, 999, trap_Cvar_VariableValue( "ui_ctf_timelimit" ) ) ) );
		s_serveroptions.friendlyfire.curvalue = (int)Com_Clamp( 0, 1, trap_Cvar_VariableValue( "ui_ctf_friendly" ) );
#ifndef TA_WEAPSYS
		s_serveroptions.instagib.curvalue = (int)Com_Clamp( 0, 1, trap_Cvar_VariableValue( "ui_ctf_instagib" ) );
#endif
		break;

#ifdef MISSIONPACK
	case GT_1FCTF:
		MField_SetText( &s_serveroptions.flaglimit.field, va( "%i", (int)Com_Clamp( 0, 100, trap_Cvar_VariableValue( "ui_1flag_capturelimit" ) ) ) );
		MField_SetText( &s_serveroptions.timelimit.field, va( "%i", (int)Com_Clamp( 0, 999, trap_Cvar_VariableValue( "ui_1flag_timelimit" ) ) ) );
		s_serveroptions.friendlyfire.curvalue = (int)Com_Clamp( 0, 1, trap_Cvar_VariableValue( "ui_1flag_friendly" ) );
#ifndef TA_WEAPSYS
		s_serveroptions.instagib.curvalue = (int)Com_Clamp( 0, 1, trap_Cvar_VariableValue( "ui_1flag_instagib" ) );
#endif
		break;

	case GT_OBELISK:
		MField_SetText( &s_serveroptions.flaglimit.field, va( "%i", (int)Com_Clamp( 0, 100, trap_Cvar_VariableValue( "ui_obelisk_capturelimit" ) ) ) );
		MField_SetText( &s_serveroptions.timelimit.field, va( "%i", (int)Com_Clamp( 0, 999, trap_Cvar_VariableValue( "ui_obelisk_timelimit" ) ) ) );
		s_serveroptions.friendlyfire.curvalue = (int)Com_Clamp( 0, 1, trap_Cvar_VariableValue( "ui_obelisk_friendly" ) );
#ifndef TA_WEAPSYS
		s_serveroptions.instagib.curvalue = (int)Com_Clamp( 0, 1, trap_Cvar_VariableValue( "ui_obelisk_instagib" ) );
#endif
		break;

#ifdef MISSIONPACK_HARVESTER
	case GT_HARVESTER:
		MField_SetText( &s_serveroptions.flaglimit.field, va( "%i", (int)Com_Clamp( 0, 100, trap_Cvar_VariableValue( "ui_harvester_capturelimit" ) ) ) );
		MField_SetText( &s_serveroptions.timelimit.field, va( "%i", (int)Com_Clamp( 0, 999, trap_Cvar_VariableValue( "ui_harvester_timelimit" ) ) ) );
		s_serveroptions.friendlyfire.curvalue = (int)Com_Clamp( 0, 1, trap_Cvar_VariableValue( "ui_harvester_friendly" ) );
#ifndef TA_WEAPSYS
		s_serveroptions.instagib.curvalue = (int)Com_Clamp( 0, 1, trap_Cvar_VariableValue( "ui_harvester_instagib" ) );
#endif
		break;
#endif
#endif
	}

	s_serveroptions.publicserver.curvalue = Com_Clamp( 0, 1, trap_Cvar_VariableValue( "ui_publicServer" ) );
	MField_SetText( &s_serveroptions.hostname.field, CG_Cvar_VariableString( "sv_hostname" ) );
	s_serveroptions.pure.curvalue = Com_Clamp( 0, 1, trap_Cvar_VariableValue( "sv_pure" ) );

	// set the map pic
	Com_sprintf( picname, sizeof(picname), "levelshots/%s_small", s_startserver.currentmapname );
	if ( !trap_R_RegisterShaderNoMip( picname ) ) {
		Com_sprintf( picname, sizeof(picname), "levelshots/%s", s_startserver.currentmapname );
	}
	s_serveroptions.mappic.generic.name = picname;

	// set the map name
	Q_strncpyz( s_serveroptions.mapnamebuffer, s_startserver.mapname.string, sizeof ( s_serveroptions.mapnamebuffer ) );
	Q_strupr( s_serveroptions.mapnamebuffer );

	// get the player selections initialized
	ServerOptions_InitPlayerItems();
	ServerOptions_SetPlayerItems(-1);

	// seed bot names
	ServerOptions_InitBotNames();
	ServerOptions_SetPlayerItems(-1);
}

/*
=================
PlayerName_Draw
=================
*/
static void PlayerName_Draw( void *item ) {
	menutext_s	*s;
	float		*color;
	int			x, y;
	int			style;
	qboolean	focus;

	s = (menutext_s *)item;

	x = s->generic.x;
	y =	s->generic.y;

	style = UI_SMALLFONT;
	focus = (s->generic.parent->cursor == s->generic.menuPosition);

	if ( s->generic.flags & QMF_GRAYED )
		color = text_color_disabled;
	else if ( focus )
	{
		color = text_color_highlight;
		style |= UI_PULSE;
	}
	else if ( s->generic.flags & QMF_BLINK )
	{
		color = text_color_highlight;
		style |= UI_BLINK;
	}
	else
		color = text_color_normal;

	if ( focus )
	{
		// draw cursor
		CG_FillRect( s->generic.left, s->generic.top, s->generic.right-s->generic.left+1, s->generic.bottom-s->generic.top+1, listbar_color ); 
		UI_DrawChar( x, y, GLYPH_ARROW, UI_CENTER|UI_BLINK|UI_SMALLFONT, color);
	}

	UI_DrawString( x - SMALLCHAR_WIDTH, y, s->generic.name, style|UI_RIGHT, color );
	UI_DrawString( x + SMALLCHAR_WIDTH, y, s->string, style|UI_LEFT, color );
}


/*
=================
ServerOptions_MenuInit
=================
*/
#define OPTIONS_X	456

static void ServerOptions_MenuInit( qboolean multiplayer ) {
	int		y;
	int		n;

	memset( &s_serveroptions, 0 ,sizeof(serveroptions_t) );
	s_serveroptions.multiplayer = multiplayer;
	s_serveroptions.gametype = gametype_remap[s_startserver.gametype.curvalue];

	ServerOptions_Cache();

	s_serveroptions.menu.wrapAround = qtrue;
	s_serveroptions.menu.fullscreen = qtrue;

	s_serveroptions.banner.generic.type			= MTYPE_BTEXT;
	s_serveroptions.banner.generic.x			= 320;
	s_serveroptions.banner.generic.y			= 16;
	s_serveroptions.banner.string  				= "GAME SERVER";
	s_serveroptions.banner.color  				= text_banner_color;
	s_serveroptions.banner.style  				= UI_CENTER;

	s_serveroptions.mappic.generic.type			= MTYPE_BITMAP;
	s_serveroptions.mappic.generic.flags		= QMF_LEFT_JUSTIFY|QMF_INACTIVE;
	s_serveroptions.mappic.generic.x			= 352;
	s_serveroptions.mappic.generic.y			= 80;
	s_serveroptions.mappic.width				= 160;
	s_serveroptions.mappic.height				= 120;
	s_serveroptions.mappic.errorpic				= GAMESERVER_UNKNOWNMAP;
	s_serveroptions.mappic.generic.ownerdraw	= ServerOptions_LevelshotDraw;

	s_serveroptions.picframe.generic.type		= MTYPE_BITMAP;
	s_serveroptions.picframe.generic.flags		= QMF_LEFT_JUSTIFY|QMF_INACTIVE|QMF_HIGHLIGHT;
	s_serveroptions.picframe.generic.x			= 352 - 38;
	s_serveroptions.picframe.generic.y			= 80 - 40;
	s_serveroptions.picframe.width  			= 320;
	s_serveroptions.picframe.height  			= 320;
	s_serveroptions.picframe.focuspic			= GAMESERVER_SELECT;

	y = 272;
#ifdef TA_SP
	if (s_serveroptions.gametype != GT_SINGLE_PLAYER)
	{
#endif
	if( s_serveroptions.gametype <= GT_TEAM ) {
		s_serveroptions.fraglimit.generic.type       = MTYPE_FIELD;
#ifdef NOTRATEDM // frag to score
		s_serveroptions.fraglimit.generic.name       = "Score Limit:";
#else
		s_serveroptions.fraglimit.generic.name       = "Frag Limit:";
#endif
		s_serveroptions.fraglimit.generic.flags      = QMF_NUMBERSONLY|QMF_PULSEIFFOCUS|QMF_SMALLFONT;
		s_serveroptions.fraglimit.generic.x	         = OPTIONS_X;
		s_serveroptions.fraglimit.generic.y	         = y;
		s_serveroptions.fraglimit.generic.statusbar  = ServerOptions_StatusBar;
#ifdef NOTRATEDM // frag to score
		s_serveroptions.fraglimit.field.widthInChars = 4;
		s_serveroptions.fraglimit.field.maxchars     = 4;
#else
		s_serveroptions.fraglimit.field.widthInChars = 3;
		s_serveroptions.fraglimit.field.maxchars     = 3;
#endif
	}
	else {
		s_serveroptions.flaglimit.generic.type       = MTYPE_FIELD;
		s_serveroptions.flaglimit.generic.name       = "Capture Limit:";
		s_serveroptions.flaglimit.generic.flags      = QMF_NUMBERSONLY|QMF_PULSEIFFOCUS|QMF_SMALLFONT;
		s_serveroptions.flaglimit.generic.x	         = OPTIONS_X;
		s_serveroptions.flaglimit.generic.y	         = y;
		s_serveroptions.flaglimit.generic.statusbar  = ServerOptions_StatusBar;
		s_serveroptions.flaglimit.field.widthInChars = 3;
		s_serveroptions.flaglimit.field.maxchars     = 3;
	}

	y += BIGCHAR_HEIGHT+2;
	s_serveroptions.timelimit.generic.type       = MTYPE_FIELD;
	s_serveroptions.timelimit.generic.name       = "Time Limit:";
	s_serveroptions.timelimit.generic.flags      = QMF_NUMBERSONLY|QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_serveroptions.timelimit.generic.x	         = OPTIONS_X;
	s_serveroptions.timelimit.generic.y	         = y;
	s_serveroptions.timelimit.generic.statusbar  = ServerOptions_StatusBar;
	s_serveroptions.timelimit.field.widthInChars = 3;
	s_serveroptions.timelimit.field.maxchars     = 3;

#ifdef TA_SP
	}
#endif

	if( s_serveroptions.gametype >= GT_TEAM ) {
		y += BIGCHAR_HEIGHT+2;
		s_serveroptions.friendlyfire.generic.type     = MTYPE_RADIOBUTTON;
		s_serveroptions.friendlyfire.generic.flags    = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
		s_serveroptions.friendlyfire.generic.x	      = OPTIONS_X;
		s_serveroptions.friendlyfire.generic.y	      = y;
#ifdef IOQ3ZTM
		s_serveroptions.friendlyfire.generic.name	  = "Team Damage:";
#else
		s_serveroptions.friendlyfire.generic.name	  = "Friendly Fire:";
#endif
	}

#ifndef TA_WEAPSYS
	y += BIGCHAR_HEIGHT+2;
	s_serveroptions.instagib.generic.type		= MTYPE_RADIOBUTTON;
	s_serveroptions.instagib.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_serveroptions.instagib.generic.x			= OPTIONS_X;
	s_serveroptions.instagib.generic.y			= y;
	s_serveroptions.instagib.generic.name		= "Instagib:";
#endif

	y += BIGCHAR_HEIGHT+2;
	s_serveroptions.pure.generic.type			= MTYPE_RADIOBUTTON;
	s_serveroptions.pure.generic.flags			= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_serveroptions.pure.generic.x				= OPTIONS_X;
	s_serveroptions.pure.generic.y				= y;
	s_serveroptions.pure.generic.name			= "Pure Server:";
	if (!trap_Cvar_VariableValue( "fs_pure" )) {
		// Don't let users think they can modify sv_pure, it won't work.
		s_serveroptions.pure.generic.flags |= QMF_GRAYED;
	}

	if( s_serveroptions.multiplayer ) {
		y += BIGCHAR_HEIGHT+2;
		s_serveroptions.publicserver.generic.type	= MTYPE_RADIOBUTTON;
		s_serveroptions.publicserver.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
		s_serveroptions.publicserver.generic.x		= OPTIONS_X;
		s_serveroptions.publicserver.generic.y		= y;
		s_serveroptions.publicserver.generic.name	= "Advertise on Internet:";

		y += BIGCHAR_HEIGHT+2;
		s_serveroptions.dedicated.generic.type	= MTYPE_RADIOBUTTON;
		s_serveroptions.dedicated.generic.id		= ID_DEDICATED;
		s_serveroptions.dedicated.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
		s_serveroptions.dedicated.generic.callback	= ServerOptions_Event;
		s_serveroptions.dedicated.generic.x		= OPTIONS_X;
		s_serveroptions.dedicated.generic.y		= y;
		s_serveroptions.dedicated.generic.name	= "Dedicated:";

		y += BIGCHAR_HEIGHT+2;
		s_serveroptions.hostname.generic.type       = MTYPE_FIELD;
		s_serveroptions.hostname.generic.name       = "Hostname:";
		s_serveroptions.hostname.generic.flags      = QMF_SMALLFONT;
		s_serveroptions.hostname.generic.x          = OPTIONS_X;
		s_serveroptions.hostname.generic.y	        = y;
#ifdef IOQ3ZTM // Make it alittle longer so "Turtle Arena server" fits without scrolling.
		s_serveroptions.hostname.field.widthInChars = 20;
#else
		s_serveroptions.hostname.field.widthInChars = 18;
#endif
		s_serveroptions.hostname.field.maxchars     = 64;
	}
#ifdef IOQ3ZTM // RECORD_SP_DEMO
	else
	{
		// Record demo option
		y += BIGCHAR_HEIGHT+2;
		s_serveroptions.recorddemo.generic.type		= MTYPE_RADIOBUTTON;
		s_serveroptions.recorddemo.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
		s_serveroptions.recorddemo.generic.x		= OPTIONS_X;
		s_serveroptions.recorddemo.generic.y		= y;
		s_serveroptions.recorddemo.generic.name		= "Record Game:";
	}
#endif

	y = 80;
	s_serveroptions.botSkill.generic.type			= MTYPE_SPINCONTROL;
	s_serveroptions.botSkill.generic.flags			= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_serveroptions.botSkill.generic.name			= "Bot Skill:";
	s_serveroptions.botSkill.generic.x				= 32 + UI_DrawStrlen( s_serveroptions.botSkill.generic.name, UI_SMALLFONT ) + 2 * SMALLCHAR_WIDTH;
	s_serveroptions.botSkill.generic.y				= y;
	s_serveroptions.botSkill.itemnames				= botSkill_list;
	s_serveroptions.botSkill.curvalue				= 3;

	y += ( 2 * SMALLCHAR_HEIGHT );
	s_serveroptions.player0.generic.type			= MTYPE_TEXT;
	s_serveroptions.player0.generic.flags			= QMF_SMALLFONT;
	s_serveroptions.player0.generic.x				= 32 + SMALLCHAR_WIDTH;
	s_serveroptions.player0.generic.y				= y;
	s_serveroptions.player0.color					= color_orange;
	s_serveroptions.player0.style					= UI_LEFT|UI_SMALLFONT;

	for( n = 0; n < PLAYER_SLOTS; n++ ) {
		s_serveroptions.playerType[n].generic.type		= MTYPE_SPINCONTROL;
		s_serveroptions.playerType[n].generic.flags		= QMF_SMALLFONT;
		s_serveroptions.playerType[n].generic.id		= ID_PLAYER_TYPE+n;
		s_serveroptions.playerType[n].generic.callback	= ServerOptions_Event;
		s_serveroptions.playerType[n].generic.x			= 32;
		s_serveroptions.playerType[n].generic.y			= y;
		if (n < UI_MaxSplitView())
			s_serveroptions.playerType[n].itemnames		= humanPlayerType_list;
		else
			s_serveroptions.playerType[n].itemnames		= playerType_list;

		s_serveroptions.playerName[n].generic.type		= MTYPE_TEXT;
		s_serveroptions.playerName[n].generic.flags		= QMF_SMALLFONT;
		s_serveroptions.playerName[n].generic.x			= 96;
		s_serveroptions.playerName[n].generic.y			= y;
		s_serveroptions.playerName[n].generic.callback	= ServerOptions_PlayerNameEvent;
		s_serveroptions.playerName[n].generic.id		= n;
		s_serveroptions.playerName[n].generic.ownerdraw	= PlayerName_Draw;
		s_serveroptions.playerName[n].color				= color_orange;
		s_serveroptions.playerName[n].style				= UI_SMALLFONT;
		if (n == 0)
			s_serveroptions.playerName[n].string		= s_serveroptions.playerNameBuffers[n];
		else
			s_serveroptions.playerName[n].string		= s_serveroptions.botNameBuffers[n];
		s_serveroptions.playerName[n].generic.top		= s_serveroptions.playerName[n].generic.y;
		s_serveroptions.playerName[n].generic.bottom	= s_serveroptions.playerName[n].generic.y + SMALLCHAR_HEIGHT;
		s_serveroptions.playerName[n].generic.left		= s_serveroptions.playerName[n].generic.x - SMALLCHAR_HEIGHT/ 2;
		s_serveroptions.playerName[n].generic.right		= s_serveroptions.playerName[n].generic.x + 16 * SMALLCHAR_WIDTH;

		s_serveroptions.playerTeam[n].generic.type		= MTYPE_SPINCONTROL;
		s_serveroptions.playerTeam[n].generic.flags		= QMF_SMALLFONT;
		s_serveroptions.playerTeam[n].generic.x			= 240;
		s_serveroptions.playerTeam[n].generic.y			= y;
		s_serveroptions.playerTeam[n].itemnames			= playerTeam_list;

		y += ( SMALLCHAR_HEIGHT + 4 );
	}

	s_serveroptions.back.generic.type	  = MTYPE_BITMAP;
	s_serveroptions.back.generic.name     = GAMESERVER_BACK0;
	s_serveroptions.back.generic.flags    = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_serveroptions.back.generic.callback = ServerOptions_Event;
	s_serveroptions.back.generic.id	      = ID_BACK;
	s_serveroptions.back.generic.x		  = 0;
	s_serveroptions.back.generic.y		  = 480-64;
	s_serveroptions.back.width  		  = 128;
	s_serveroptions.back.height  		  = 64;
	s_serveroptions.back.focuspic         = GAMESERVER_BACK1;

	s_serveroptions.next.generic.type	  = MTYPE_BITMAP;
	s_serveroptions.next.generic.name     = GAMESERVER_NEXT0;
	s_serveroptions.next.generic.flags    = QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_INACTIVE|QMF_GRAYED|QMF_HIDDEN;
	s_serveroptions.next.generic.callback = ServerOptions_Event;
	s_serveroptions.next.generic.id	      = ID_STARTSERVERNEXT;
	s_serveroptions.next.generic.x		  = 640;
	s_serveroptions.next.generic.y		  = 480-64-72;
	s_serveroptions.next.generic.statusbar  = ServerOptions_StatusBar;
	s_serveroptions.next.width  		  = 128;
	s_serveroptions.next.height  		  = 64;
	s_serveroptions.next.focuspic         = GAMESERVER_NEXT1;

	s_serveroptions.go.generic.type	    = MTYPE_BITMAP;
	s_serveroptions.go.generic.name     = GAMESERVER_FIGHT0;
	s_serveroptions.go.generic.flags    = QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_serveroptions.go.generic.callback = ServerOptions_Event;
	s_serveroptions.go.generic.id	    = ID_GO;
	s_serveroptions.go.generic.x		= 640;
	s_serveroptions.go.generic.y		= 480-64;
	s_serveroptions.go.width  		    = 128;
	s_serveroptions.go.height  		    = 64;
	s_serveroptions.go.focuspic         = GAMESERVER_FIGHT1;

	Menu_AddItem( &s_serveroptions.menu, &s_serveroptions.banner );

	Menu_AddItem( &s_serveroptions.menu, &s_serveroptions.mappic );
	Menu_AddItem( &s_serveroptions.menu, &s_serveroptions.picframe );

	Menu_AddItem( &s_serveroptions.menu, &s_serveroptions.botSkill );
	Menu_AddItem( &s_serveroptions.menu, &s_serveroptions.player0 );
	for( n = 0; n < PLAYER_SLOTS; n++ ) {
		if( n != 0 ) {
			Menu_AddItem( &s_serveroptions.menu, &s_serveroptions.playerType[n] );
		}
		Menu_AddItem( &s_serveroptions.menu, &s_serveroptions.playerName[n] );
		if( s_serveroptions.gametype >= GT_TEAM ) {
			Menu_AddItem( &s_serveroptions.menu, &s_serveroptions.playerTeam[n] );
		}
	}

#ifdef TA_SP
	if (s_serveroptions.gametype != GT_SINGLE_PLAYER)
	{
#endif
	if( s_serveroptions.gametype <= GT_TEAM ) {
		Menu_AddItem( &s_serveroptions.menu, &s_serveroptions.fraglimit );
	}
	else {
		Menu_AddItem( &s_serveroptions.menu, &s_serveroptions.flaglimit );
	}
	Menu_AddItem( &s_serveroptions.menu, &s_serveroptions.timelimit );
#ifdef TA_SP
	}
#endif
	if( s_serveroptions.gametype >= GT_TEAM ) {
		Menu_AddItem( &s_serveroptions.menu, &s_serveroptions.friendlyfire );
	}
#ifndef TA_WEAPSYS
	Menu_AddItem( &s_serveroptions.menu, &s_serveroptions.instagib );
#endif
	Menu_AddItem( &s_serveroptions.menu, &s_serveroptions.pure );
	if( s_serveroptions.multiplayer ) {
		Menu_AddItem( &s_serveroptions.menu, &s_serveroptions.publicserver );
		Menu_AddItem( &s_serveroptions.menu, &s_serveroptions.dedicated );
		Menu_AddItem( &s_serveroptions.menu, &s_serveroptions.hostname );
	}
#ifdef IOQ3ZTM // RECORD_SP_DEMO
	else {
		Menu_AddItem( &s_serveroptions.menu, &s_serveroptions.recorddemo);
	}
#endif

	Menu_AddItem( &s_serveroptions.menu, &s_serveroptions.back );
	Menu_AddItem( &s_serveroptions.menu, &s_serveroptions.next );
	Menu_AddItem( &s_serveroptions.menu, &s_serveroptions.go );
	
	ServerOptions_SetMenuItems();
}

/*
=================
ServerOptions_Cache
=================
*/
void ServerOptions_Cache( void ) {
	trap_R_RegisterShaderNoMip( GAMESERVER_BACK0 );
	trap_R_RegisterShaderNoMip( GAMESERVER_BACK1 );
	trap_R_RegisterShaderNoMip( GAMESERVER_FIGHT0 );
	trap_R_RegisterShaderNoMip( GAMESERVER_FIGHT1 );
	trap_R_RegisterShaderNoMip( GAMESERVER_SELECT );
	trap_R_RegisterShaderNoMip( GAMESERVER_UNKNOWNMAP );
}


/*
=================
UI_ServerOptionsMenu
=================
*/
static void UI_ServerOptionsMenu( qboolean multiplayer ) {
	ServerOptions_MenuInit( multiplayer );
	UI_PushMenu( &s_serveroptions.menu );
}



/*
=============================================================================

BOT SELECT MENU *****

=============================================================================
*/


#define BOTSELECT_BACK0			"menu/art/back_0"
#define BOTSELECT_BACK1			"menu/art/back_1"
#define BOTSELECT_ACCEPT0		"menu/art/accept_0"
#define BOTSELECT_ACCEPT1		"menu/art/accept_1"
#define BOTSELECT_SELECT		"menu/art/opponents_select"
#define BOTSELECT_SELECTED		"menu/art/opponents_selected"
#ifdef TA_DATA
#define BOTSELECT_ARROWS		"menu/art/arrows_horz_0"
#define BOTSELECT_ARROWSL		"menu/art/arrows_horz_left"
#define BOTSELECT_ARROWSR		"menu/art/arrows_horz_right"
#define BOTSELECT_ARROWS_WIDTH 128
#define BOTSELECT_ARROWS_HEIGHT 64
#else
#define BOTSELECT_ARROWS		"menu/art/gs_arrows_0"
#define BOTSELECT_ARROWSL		"menu/art/gs_arrows_l"
#define BOTSELECT_ARROWSR		"menu/art/gs_arrows_r"
#endif

#define PLAYERGRID_COLS			4
#define PLAYERGRID_ROWS			4
#define MAX_MODELSPERPAGE		(PLAYERGRID_ROWS * PLAYERGRID_COLS)


typedef struct {
	menuframework_s	menu;

	menutext_s		banner;

	menubitmap_s	pics[MAX_MODELSPERPAGE];
	menubitmap_s	picbuttons[MAX_MODELSPERPAGE];
	menutext_s		picnames[MAX_MODELSPERPAGE];

	menubitmap_s	arrows;
	menubitmap_s	left;
	menubitmap_s	right;

	menubitmap_s	go;
	menubitmap_s	back;

	int				numBots;
	int				modelpage;
	int				numpages;
	int				selectedmodel;
	int				sortedBotNums[MAX_BOTS];
	char			boticons[MAX_MODELSPERPAGE][MAX_QPATH];
	char			botnames[MAX_MODELSPERPAGE][16];
} botSelectInfo_t;

static botSelectInfo_t	botSelectInfo;


/*
=================
UI_BotSelectMenu_SortCompare
=================
*/
static int QDECL UI_BotSelectMenu_SortCompare( const void *arg1, const void *arg2 ) {
	int			num1, num2;
	const char	*info1, *info2;
	const char	*name1, *name2;

	num1 = *(int *)arg1;
	num2 = *(int *)arg2;

	info1 = UI_GetBotInfoByNumber( num1 );
	info2 = UI_GetBotInfoByNumber( num2 );

	name1 = Info_ValueForKey( info1, "name" );
	name2 = Info_ValueForKey( info2, "name" );

	// put random option first
	if (Q_stricmp(name1, "Random") == 0) {
		return -1;
	}
	if (Q_stricmp(name2, "Random") == 0) {
		return 1;
	}

	return Q_stricmp( name1, name2 );
}


/*
=================
UI_BotSelectMenu_BuildList
=================
*/
static void UI_BotSelectMenu_BuildList( void ) {
	int		n;

	botSelectInfo.modelpage = 0;
	botSelectInfo.numBots = UI_GetNumBots();
	botSelectInfo.numpages = botSelectInfo.numBots / MAX_MODELSPERPAGE;
	if( botSelectInfo.numBots % MAX_MODELSPERPAGE ) {
		botSelectInfo.numpages++;
	}

	// initialize the array
	for( n = 0; n < botSelectInfo.numBots; n++ ) {
		botSelectInfo.sortedBotNums[n] = n;
	}

	// now sort it
	qsort( botSelectInfo.sortedBotNums, botSelectInfo.numBots, sizeof(botSelectInfo.sortedBotNums[0]), UI_BotSelectMenu_SortCompare );
}


/*
=================
ServerPlayerIcon
=================
*/
static void ServerPlayerIcon( const char *modelAndSkin, char *iconName, int iconNameMaxSize ) {
	char	*skin;
	char	model[MAX_QPATH];
#ifdef IOQ3ZTM // BOT_HEADMODEL
	qboolean headmodel;
#endif

	// icon for random bot option
	if (Q_stricmp(modelAndSkin, "random") == 0)
	{
		Com_sprintf(iconName, iconNameMaxSize, "menu/art/randombot_icon.tga");

		if (!trap_R_RegisterShaderNoMip( iconName ))
		{
			// If missing random bot icon fallback to unknown map icon
			Com_sprintf(iconName, iconNameMaxSize, "menu/art/unknownmap.tga");
			trap_R_RegisterShaderNoMip( iconName );
		}
		return;
	}

#ifdef IOQ3ZTM // BOT_HEADMODEL
	headmodel = (modelAndSkin[0] == '*');
	if (headmodel) {
		Q_strncpyz( model, &modelAndSkin[1], sizeof(model));
	} else {
		Q_strncpyz( model, modelAndSkin, sizeof(model));
	}
#else
	Q_strncpyz( model, modelAndSkin, sizeof(model));
#endif
	skin = strrchr( model, '/' );
	if ( skin ) {
		*skin++ = '\0';
	}
	else {
		skin = "default";
	}

#ifdef IOQ3ZTM // BOT_HEADMODEL
	if (headmodel)
	{
		Com_sprintf(iconName, iconNameMaxSize, "models/players/heads/%s/icon_%s.tga", model, skin );

		if( !trap_R_RegisterShaderNoMip( iconName ) && Q_stricmp( skin, "default" ) != 0 ) {
			Com_sprintf(iconName, iconNameMaxSize, "models/players/heads/%s/icon_default.tga", model );
		}

		if (trap_R_RegisterShaderNoMip( iconName )) {
			return;
		}
	}
#endif

	Com_sprintf(iconName, iconNameMaxSize, "models/players/%s/icon_%s.tga", model, skin );

	if( !trap_R_RegisterShaderNoMip( iconName ) && Q_stricmp( skin, "default" ) != 0 ) {
		Com_sprintf(iconName, iconNameMaxSize, "models/players/%s/icon_default.tga", model );
	}
}


#ifdef IOQ3ZTM // BOT_HEADMODEL
/*
=================
PlayerIconhandle
=================
*/
static qhandle_t ServerPlayerIconHandle( const char *modelAndSkin, char *iconName, size_t iconSize ) {
	ServerPlayerIcon( modelAndSkin, iconName, iconSize);
	return trap_R_RegisterShaderNoMip( iconName );
}
#endif

/*
=================
UI_BotSelectMenu_UpdateGrid
=================
*/
static void UI_BotSelectMenu_UpdateGrid( void ) {
	const char	*info;
	int			i;
    int			j;

	j = botSelectInfo.modelpage * MAX_MODELSPERPAGE;
	for( i = 0; i < (PLAYERGRID_ROWS * PLAYERGRID_COLS); i++, j++) {
		if( j < botSelectInfo.numBots ) { 
			info = UI_GetBotInfoByNumber( botSelectInfo.sortedBotNums[j] );
#ifdef IOQ3ZTM // BOT_HEADMODEL
			if (!ServerPlayerIconHandle( Info_ValueForKey( info, "headmodel" ), botSelectInfo.boticons[i], MAX_QPATH ))
#endif
			ServerPlayerIcon( Info_ValueForKey( info, "model" ), botSelectInfo.boticons[i], MAX_QPATH );
			Q_strncpyz( botSelectInfo.botnames[i], Info_ValueForKey( info, "name" ), 16 );
			Q_CleanStr( botSelectInfo.botnames[i] );
 			botSelectInfo.pics[i].generic.name = botSelectInfo.boticons[i];
			if( BotAlreadySelected( botSelectInfo.botnames[i] ) ) {
#ifdef TURTLEARENA
				botSelectInfo.picnames[i].color = color_white;
#else
				botSelectInfo.picnames[i].color = color_red;
#endif
			}
			else {
				botSelectInfo.picnames[i].color = color_orange;
			}
			botSelectInfo.picbuttons[i].generic.flags &= ~QMF_INACTIVE;
		}
		else {
			// dead slot
 			botSelectInfo.pics[i].generic.name         = NULL;
			botSelectInfo.picbuttons[i].generic.flags |= QMF_INACTIVE;
			botSelectInfo.botnames[i][0] = 0;
		}

 		botSelectInfo.pics[i].generic.flags       &= ~QMF_HIGHLIGHT;
 		botSelectInfo.pics[i].shader               = 0;
 		botSelectInfo.picbuttons[i].generic.flags |= QMF_PULSEIFFOCUS;
	}

	// set selected model
	i = botSelectInfo.selectedmodel % MAX_MODELSPERPAGE;
	botSelectInfo.pics[i].generic.flags |= QMF_HIGHLIGHT;
	botSelectInfo.picbuttons[i].generic.flags &= ~QMF_PULSEIFFOCUS;

	if( botSelectInfo.numpages > 1 ) {
		if( botSelectInfo.modelpage > 0 ) {
			botSelectInfo.left.generic.flags &= ~QMF_INACTIVE;
		}
		else {
			botSelectInfo.left.generic.flags |= QMF_INACTIVE;
		}

		if( botSelectInfo.modelpage < (botSelectInfo.numpages - 1) ) {
			botSelectInfo.right.generic.flags &= ~QMF_INACTIVE;
		}
		else {
			botSelectInfo.right.generic.flags |= QMF_INACTIVE;
		}
	}
	else {
		// hide left/right markers
		botSelectInfo.left.generic.flags |= QMF_INACTIVE;
		botSelectInfo.right.generic.flags |= QMF_INACTIVE;
	}
}


/*
=================
UI_BotSelectMenu_Default
=================
*/
static void UI_BotSelectMenu_Default( char *bot ) {
	const char	*botInfo;
	const char	*test;
	int			n;
	int			i;

	for( n = 0; n < botSelectInfo.numBots; n++ ) {
		botInfo = UI_GetBotInfoByNumber( n );
		test = Info_ValueForKey( botInfo, "name" );
		if( Q_stricmp( bot, test ) == 0 ) {
			break;
		}
	}
	if( n == botSelectInfo.numBots ) {
		botSelectInfo.selectedmodel = 0;
		return;
	}

	for( i = 0; i < botSelectInfo.numBots; i++ ) {
		if( botSelectInfo.sortedBotNums[i] == n ) {
			break;
		}
	}
	if( i == botSelectInfo.numBots ) {
		botSelectInfo.selectedmodel = 0;
		return;
	}

	botSelectInfo.selectedmodel = i;
}


/*
=================
UI_BotSelectMenu_LeftEvent
=================
*/
static void UI_BotSelectMenu_LeftEvent( void* ptr, int event ) {
	if( event != QM_ACTIVATED ) {
		return;
	}
	if( botSelectInfo.modelpage > 0 ) {
		botSelectInfo.modelpage--;
		botSelectInfo.selectedmodel = botSelectInfo.modelpage * MAX_MODELSPERPAGE;
		UI_BotSelectMenu_UpdateGrid();
	}
}


/*
=================
UI_BotSelectMenu_RightEvent
=================
*/
static void UI_BotSelectMenu_RightEvent( void* ptr, int event ) {
	if( event != QM_ACTIVATED ) {
		return;
	}
	if( botSelectInfo.modelpage < botSelectInfo.numpages - 1 ) {
		botSelectInfo.modelpage++;
		botSelectInfo.selectedmodel = botSelectInfo.modelpage * MAX_MODELSPERPAGE;
		UI_BotSelectMenu_UpdateGrid();
	}
}


/*
=================
UI_BotSelectMenu_BotEvent
=================
*/
static void UI_BotSelectMenu_BotEvent( void* ptr, int event ) {
	int		i;

	if( event != QM_ACTIVATED ) {
		return;
	}

	for( i = 0; i < (PLAYERGRID_ROWS * PLAYERGRID_COLS); i++ ) {
 		botSelectInfo.pics[i].generic.flags &= ~QMF_HIGHLIGHT;
 		botSelectInfo.picbuttons[i].generic.flags |= QMF_PULSEIFFOCUS;
	}

	// set selected
	i = ((menucommon_s*)ptr)->id;
	botSelectInfo.pics[i].generic.flags |= QMF_HIGHLIGHT;
	botSelectInfo.picbuttons[i].generic.flags &= ~QMF_PULSEIFFOCUS;
	botSelectInfo.selectedmodel = botSelectInfo.modelpage * MAX_MODELSPERPAGE + i;
}


/*
=================
UI_BotSelectMenu_BackEvent
=================
*/
static void UI_BotSelectMenu_BackEvent( void* ptr, int event ) {
	if( event != QM_ACTIVATED ) {
		return;
	}
	UI_PopMenu();
}


/*
=================
UI_BotSelectMenu_SelectEvent
=================
*/
static void UI_BotSelectMenu_SelectEvent( void* ptr, int event ) {
	if( event != QM_ACTIVATED ) {
		return;
	}
	UI_PopMenu();

	s_serveroptions.newBot = qtrue;
	Q_strncpyz( s_serveroptions.newBotName, botSelectInfo.botnames[botSelectInfo.selectedmodel % MAX_MODELSPERPAGE], 16 );
}


/*
=================
UI_BotSelectMenu_Cache
=================
*/
void UI_BotSelectMenu_Cache( void ) {
	trap_R_RegisterShaderNoMip( BOTSELECT_BACK0 );
	trap_R_RegisterShaderNoMip( BOTSELECT_BACK1 );
	trap_R_RegisterShaderNoMip( BOTSELECT_ACCEPT0 );
	trap_R_RegisterShaderNoMip( BOTSELECT_ACCEPT1 );
	trap_R_RegisterShaderNoMip( BOTSELECT_SELECT );
	trap_R_RegisterShaderNoMip( BOTSELECT_SELECTED );
	trap_R_RegisterShaderNoMip( BOTSELECT_ARROWS );
	trap_R_RegisterShaderNoMip( BOTSELECT_ARROWSL );
	trap_R_RegisterShaderNoMip( BOTSELECT_ARROWSR );
}


static void UI_BotSelectMenu_Init( char *bot ) {
	int		i, j, k;
	int		x, y;

	memset( &botSelectInfo, 0 ,sizeof(botSelectInfo) );
	botSelectInfo.menu.wrapAround = qtrue;
	botSelectInfo.menu.fullscreen = qtrue;

	UI_BotSelectMenu_Cache();

	botSelectInfo.banner.generic.type	= MTYPE_BTEXT;
	botSelectInfo.banner.generic.x		= 320;
	botSelectInfo.banner.generic.y		= 16;
	botSelectInfo.banner.string			= "SELECT BOT";
	botSelectInfo.banner.color			= text_banner_color;
	botSelectInfo.banner.style			= UI_CENTER;

	y =	80;
	for( i = 0, k = 0; i < PLAYERGRID_ROWS; i++) {
		x =	180;
		for( j = 0; j < PLAYERGRID_COLS; j++, k++ ) {
			botSelectInfo.pics[k].generic.type				= MTYPE_BITMAP;
			botSelectInfo.pics[k].generic.flags				= QMF_LEFT_JUSTIFY|QMF_INACTIVE;
			botSelectInfo.pics[k].generic.x					= x;
			botSelectInfo.pics[k].generic.y					= y;
 			botSelectInfo.pics[k].generic.name				= botSelectInfo.boticons[k];
			botSelectInfo.pics[k].width						= 64;
			botSelectInfo.pics[k].height					= 64;
			botSelectInfo.pics[k].focuspic					= BOTSELECT_SELECTED;
#ifndef TA_DATA
			botSelectInfo.pics[k].focuscolor				= colorRed;
#endif

			botSelectInfo.picbuttons[k].generic.type		= MTYPE_BITMAP;
			botSelectInfo.picbuttons[k].generic.flags		= QMF_LEFT_JUSTIFY|QMF_NODEFAULTINIT|QMF_PULSEIFFOCUS;
			botSelectInfo.picbuttons[k].generic.callback	= UI_BotSelectMenu_BotEvent;
			botSelectInfo.picbuttons[k].generic.id			= k;
			botSelectInfo.picbuttons[k].generic.x			= x - 16;
			botSelectInfo.picbuttons[k].generic.y			= y - 16;
			botSelectInfo.picbuttons[k].generic.left		= x;
			botSelectInfo.picbuttons[k].generic.top			= y;
			botSelectInfo.picbuttons[k].generic.right		= x + 64;
			botSelectInfo.picbuttons[k].generic.bottom		= y + 64;
			botSelectInfo.picbuttons[k].width				= 128;
			botSelectInfo.picbuttons[k].height				= 128;
			botSelectInfo.picbuttons[k].focuspic			= BOTSELECT_SELECT;
#ifndef TA_DATA
			botSelectInfo.picbuttons[k].focuscolor			= colorRed;
#endif

			botSelectInfo.picnames[k].generic.type			= MTYPE_TEXT;
			botSelectInfo.picnames[k].generic.flags			= QMF_SMALLFONT;
			botSelectInfo.picnames[k].generic.x				= x + 32;
			botSelectInfo.picnames[k].generic.y				= y + 64;
			botSelectInfo.picnames[k].string				= botSelectInfo.botnames[k];
			botSelectInfo.picnames[k].color					= color_orange;
			botSelectInfo.picnames[k].style					= UI_CENTER|UI_SMALLFONT;

			x += (64 + 6);
		}
		y += (64 + SMALLCHAR_HEIGHT + 6);
	}

	botSelectInfo.arrows.generic.type		= MTYPE_BITMAP;
	botSelectInfo.arrows.generic.name		= BOTSELECT_ARROWS;
	botSelectInfo.arrows.generic.flags		= QMF_INACTIVE;
	botSelectInfo.arrows.generic.x			= 260;
#ifdef TA_DATA
	botSelectInfo.arrows.generic.y			= 400;
	botSelectInfo.arrows.width				= BOTSELECT_ARROWS_WIDTH;
	botSelectInfo.arrows.height				= BOTSELECT_ARROWS_HEIGHT;
#else
	botSelectInfo.arrows.generic.y			= 440;
	botSelectInfo.arrows.width				= 128;
	botSelectInfo.arrows.height				= 32;
#endif

	botSelectInfo.left.generic.type			= MTYPE_BITMAP;
	botSelectInfo.left.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	botSelectInfo.left.generic.callback		= UI_BotSelectMenu_LeftEvent;
	botSelectInfo.left.generic.x			= 260;
#ifdef TA_DATA
	botSelectInfo.left.generic.y			= 400;
	botSelectInfo.left.width  				= BOTSELECT_ARROWS_WIDTH/2;
	botSelectInfo.left.height  				= BOTSELECT_ARROWS_HEIGHT;
#else
	botSelectInfo.left.generic.y			= 440;
	botSelectInfo.left.width  				= 64;
	botSelectInfo.left.height  				= 32;
#endif
	botSelectInfo.left.focuspic				= BOTSELECT_ARROWSL;

	botSelectInfo.right.generic.type	    = MTYPE_BITMAP;
	botSelectInfo.right.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	botSelectInfo.right.generic.callback	= UI_BotSelectMenu_RightEvent;
#ifdef TA_DATA
	botSelectInfo.right.generic.x			= 260+BOTSELECT_ARROWS_WIDTH/2;
	botSelectInfo.right.generic.y			= 400;
	botSelectInfo.right.width  				= BOTSELECT_ARROWS_WIDTH/2;
	botSelectInfo.right.height  		    = BOTSELECT_ARROWS_HEIGHT;
#else
	botSelectInfo.right.generic.x			= 321;
	botSelectInfo.right.generic.y			= 440;
	botSelectInfo.right.width  				= 64;
	botSelectInfo.right.height  		    = 32;
#endif
	botSelectInfo.right.focuspic			= BOTSELECT_ARROWSR;

	botSelectInfo.back.generic.type		= MTYPE_BITMAP;
	botSelectInfo.back.generic.name		= BOTSELECT_BACK0;
	botSelectInfo.back.generic.flags	= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	botSelectInfo.back.generic.callback	= UI_BotSelectMenu_BackEvent;
	botSelectInfo.back.generic.x		= 0;
	botSelectInfo.back.generic.y		= 480-64;
	botSelectInfo.back.width			= 128;
	botSelectInfo.back.height			= 64;
	botSelectInfo.back.focuspic			= BOTSELECT_BACK1;

	botSelectInfo.go.generic.type		= MTYPE_BITMAP;
	botSelectInfo.go.generic.name		= BOTSELECT_ACCEPT0;
	botSelectInfo.go.generic.flags		= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	botSelectInfo.go.generic.callback	= UI_BotSelectMenu_SelectEvent;
	botSelectInfo.go.generic.x			= 640;
	botSelectInfo.go.generic.y			= 480-64;
	botSelectInfo.go.width				= 128;
	botSelectInfo.go.height				= 64;
	botSelectInfo.go.focuspic			= BOTSELECT_ACCEPT1;

	Menu_AddItem( &botSelectInfo.menu, &botSelectInfo.banner );
	for( i = 0; i < MAX_MODELSPERPAGE; i++ ) {
		Menu_AddItem( &botSelectInfo.menu,	&botSelectInfo.pics[i] );
		Menu_AddItem( &botSelectInfo.menu,	&botSelectInfo.picbuttons[i] );
		Menu_AddItem( &botSelectInfo.menu,	&botSelectInfo.picnames[i] );
	}
	Menu_AddItem( &botSelectInfo.menu, &botSelectInfo.arrows );
	Menu_AddItem( &botSelectInfo.menu, &botSelectInfo.left );
	Menu_AddItem( &botSelectInfo.menu, &botSelectInfo.right );
	Menu_AddItem( &botSelectInfo.menu, &botSelectInfo.back );
	Menu_AddItem( &botSelectInfo.menu, &botSelectInfo.go );

	UI_BotSelectMenu_BuildList();
	UI_BotSelectMenu_Default( bot );
	botSelectInfo.modelpage = botSelectInfo.selectedmodel / MAX_MODELSPERPAGE;
	UI_BotSelectMenu_UpdateGrid();
}


/*
=================
UI_BotSelectMenu
=================
*/
void UI_BotSelectMenu( char *bot ) {
	UI_BotSelectMenu_Init( bot );
	UI_PushMenu( &botSelectInfo.menu );
}
#endif

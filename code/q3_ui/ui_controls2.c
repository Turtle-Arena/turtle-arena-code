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
=======================================================================

CONTROLS MENU

=======================================================================
*/


#include "ui_local.h"

#define ART_BACK0			"menu/art/back_0"
#define ART_BACK1			"menu/art/back_1"
#define ART_FRAMEL			"menu/art/frame2_l"
#define ART_FRAMER			"menu/art/frame1_r"


typedef struct {
	char	*command;
	char	*label;
	int		id;
	int		anim;
	int		defaultbind1;
	int		defaultbind2;
	int		bind1;
	int		bind2;
} bind_t;

#define SAVE_NOOP		0
#define SAVE_YES		1
#define SAVE_NO			2
#define SAVE_CANCEL		3

// control sections
#define C_MOVEMENT		0
#define C_LOOKING		1
#define C_WEAPONS		2
#define C_MISC			3
#ifdef IOQ3ZTM // NEW_CAM
#define C_CAMERA		4
#define	C_JOYSTICK		5
#else
#define	C_JOYSTICK		4
#endif

#define ID_MOVEMENT		100
#define ID_LOOKING		101
#define ID_WEAPONS		102
#define ID_MISC			103
#define ID_DEFAULTS		104
#define ID_BACK			105
#define ID_SAVEANDEXIT	106
#define ID_EXIT			107
#define ID_JOYSTICK		108
#ifdef IOQ3ZTM // NEW_CAM
#define ID_CAMERA		109
#endif

enum {
	// bindable actions
	ID_SHOWSCORES,
#ifdef MISSIONPACK_HUD
	ID_SCORESUP,
	ID_SCORESDOWN,
#endif
	ID_USEITEM,
#ifndef TURTLEARENA // NO_SPEED_KEY
	ID_SPEED,
#endif
	ID_FORWARD,
	ID_BACKPEDAL,
	ID_MOVELEFT,
	ID_MOVERIGHT,
	ID_MOVEUP,
	ID_MOVEDOWN,
	ID_LEFT,
	ID_RIGHT,
	ID_STRAFE,
	ID_LOOKUP,
	ID_LOOKDOWN,
	ID_MOUSELOOK,
	ID_CENTERVIEW,
#ifdef TURTLEARENA // LOCKON
	ID_LOCKON,
#endif
#ifndef TURTLEARENA // NOZOOM
	ID_ZOOMVIEW,
#endif
#ifndef TA_WEAPSYS_EX
	ID_WEAPON1,
	ID_WEAPON2,
	ID_WEAPON3,
	ID_WEAPON4,
	ID_WEAPON5,
	ID_WEAPON6,
	ID_WEAPON7,
	ID_WEAPON8,
	ID_WEAPON9,
#ifdef MISSIONPACK
	ID_WEAPON11,
	ID_WEAPON12,
	ID_WEAPON13,
#endif
#endif
	ID_ATTACK,
#ifdef TA_WEAPSYS_EX
	ID_WEAPDROP,
#else
	ID_WEAPPREV,
	ID_WEAPNEXT,
#endif
	ID_GESTURE,
	ID_CHAT,
	ID_CHAT2,
	ID_CHAT3,
	ID_CHAT4,
	ID_TOGGLEMENU,
#ifdef TA_HOLDSYS
	ID_NEXTITEM,
	ID_PREVITEM,
#endif
#ifdef TA_MISC // DROP_FLAG
	ID_DROPFLAG,
#endif
#ifdef IOQ3ZTM
	ID_VOTEYES,
	ID_VOTENO,
	ID_FOLLOWPLAYER,
	ID_TAKESCREENSHOT,
#endif
#ifdef IOQ3ZTM // NEW_CAM
	ID_CAMRESET,
	ID_CAMTOGGLE,
	ID_CAMRIGHT,
	ID_CAMLEFT,
	ID_CAMZOOMIN,
	ID_CAMZOOMOUT,
#endif

	// all others
	ID_FREELOOK,
	ID_INVERTMOUSE,
#ifndef TURTLEARENA // ALWAYS_RUN
	ID_ALWAYSRUN,
#endif
#ifndef TA_WEAPSYS_EX
	ID_CYCLEPASTGAUNTLET,
	ID_AUTOSWITCH,
#endif
	ID_MOUSESPEED,
	ID_SELECTJOY,
	ID_JOYTHRESHOLD,
	ID_JOYANALOG,
	ID_SMOOTHMOUSE
};

enum {
	ANIM_IDLE,
	ANIM_RUN,
	ANIM_WALK,
	ANIM_BACK,
	ANIM_JUMP,
	ANIM_CROUCH,
	ANIM_STEPLEFT,
	ANIM_STEPRIGHT,
	ANIM_TURNLEFT,
	ANIM_TURNRIGHT,
	ANIM_LOOKUP,
	ANIM_LOOKDOWN,
#ifndef TA_WEAPSYS_EX
	ANIM_WEAPON1,
	ANIM_WEAPON2,
	ANIM_WEAPON3,
	ANIM_WEAPON4,
	ANIM_WEAPON5,
	ANIM_WEAPON6,
	ANIM_WEAPON7,
	ANIM_WEAPON8,
	ANIM_WEAPON9,
	ANIM_WEAPON10,
#ifdef MISSIONPACK
	ANIM_WEAPON11,
	ANIM_WEAPON12,
	ANIM_WEAPON13,
#endif
#endif // TA_WEAPSYS_EX
	ANIM_ATTACK,
	ANIM_GESTURE,
	ANIM_DIE,
	ANIM_CHAT
};

typedef struct
{
	menuframework_s		menu;

	menutext_s			banner;
	menubitmap_s		framel;
	menubitmap_s		framer;
#ifndef TA_MISC
	menubitmap_s		player;
#endif

	menutext_s			movement;
	menutext_s			looking;
	menutext_s			weapons;
	menutext_s			misc;
#ifdef IOQ3ZTM // NEW_CAM
	menutext_s			camera;
#endif
	menutext_s			joystick;
#ifdef IOQ3ZTM
	menutext_s			defaults;
#endif

	menuaction_s		walkforward;
	menuaction_s		backpedal;
	menuaction_s		stepleft;
	menuaction_s		stepright;
	menuaction_s		moveup;
	menuaction_s		movedown;
	menuaction_s		turnleft;
	menuaction_s		turnright;
	menuaction_s		sidestep;
#ifndef TURTLEARENA // NO_SPEED_KEY
	menuaction_s		run;
#endif
#ifndef TA_WEAPSYS_EX
	menuaction_s		machinegun;
	menuaction_s		gauntlet;
	menuaction_s		shotgun;
	menuaction_s		grenadelauncher;
	menuaction_s		rocketlauncher;
	menuaction_s		lightning;
	menuaction_s		railgun;
	menuaction_s		plasma;
	menuaction_s		bfg;
#ifdef MISSIONPACK
	menuaction_s		nailgun;
	menuaction_s		proxylauncher;
	menuaction_s		chaingun;
#endif
#endif
	menuaction_s		attack;
#ifdef TA_WEAPSYS_EX
	menuaction_s		dropweapon;
#else
	menuaction_s		prevweapon;
	menuaction_s		nextweapon;
#endif
	menuaction_s		lookup;
	menuaction_s		lookdown;
	menuaction_s		mouselook;
	menuradiobutton_s	freelook;
	menuaction_s		centerview;
#ifdef TURTLEARENA // LOCKON
	menuaction_s		lockon;
#endif
#ifndef TURTLEARENA // NOZOOM
	menuaction_s		zoomview;
#endif
	menuaction_s		gesture;
	menuradiobutton_s	invertmouse;
	menuslider_s		sensitivity;
	menuradiobutton_s	smoothmouse;
#ifndef TURTLEARENA // ALWAYS_RUN
	menuradiobutton_s	alwaysrun;
#endif
	menuaction_s		showscores;
#ifdef MISSIONPACK_HUD
	menuaction_s		scoresup;
	menuaction_s		scoresdown;
#endif
#ifndef TA_WEAPSYS_EX
	menuradiobutton_s	cyclepastgauntlet;
	menuradiobutton_s	autoswitch;
#endif
#ifdef TA_MISC // DROP_FLAG
	menuaction_s		dropflag;
#endif
	menuaction_s		useitem;
#ifdef TA_HOLDSYS/*2*/
	menuaction_s		nextitem;
	menuaction_s		previtem;
#endif
#ifndef TA_MISC
	uiPlayerInfo_t		playerinfo;
#endif
	qboolean			changesmade;
	menuaction_s		chat;
	menuaction_s		chat2;
	menuaction_s		chat3;
	menuaction_s		chat4;
	menuaction_s		togglemenu;
#ifdef IOQ3ZTM
	menuaction_s		voteYes;
	menuaction_s		voteNo;
	menuaction_s		followPlayer;
	menuaction_s		takeScreenshot;
#endif
#ifdef IOQ3ZTM // NEW_CAM
	menuaction_s		camreset;
	menuaction_s		camtoggle;
	menuaction_s		camright;
	menuaction_s		camleft;
	menuaction_s		camzoomin;
	menuaction_s		camzoomout;
#endif

	menutext_s			selectjoy;
	menuslider_s		joythreshold;
	menuradiobutton_s	joyanalog;
	int					section;
	char				playerModel[MAX_QPATH];
	char				playerHead[MAX_QPATH];
	vec3_t				playerViewangles;
	vec3_t				playerMoveangles;
	int					playerLegs;
	int					playerTorso;
	weapon_t			playerWeapon;
	qboolean			playerChat;

	menubitmap_s		back;
#ifndef TA_MISC
	menutext_s			name;
#endif

	int					localPlayerNum;
	menucommon_s		***controls;
	bind_t				*bindings;
} controls_t; 	

static controls_t s_controls;

static qboolean waitingforkey = qfalse;

static bind_t g_bindings[] = 
{
	{"+scores",			"show scores",		ID_SHOWSCORES,	ANIM_IDLE,		K_TAB,			-1,		-1, -1},
#ifdef MISSIONPACK_HUD
	{"scoresUp",		"scroll scores up", ID_SCORESUP,	ANIM_IDLE,		K_KP_PGUP,		-1,		-1, -1},
	{"scoresDown",		"scroll scores down",ID_SCORESDOWN,	ANIM_IDLE,		K_KP_PGDN,		-1,		-1, -1},
#endif
#ifdef IOQ3ZTM
	{"+button2",		"use item",			ID_USEITEM,		ANIM_IDLE,		K_ENTER,	K_MOUSE2,	-1, -1},
#else
	{"+button2",		"use item",			ID_USEITEM,		ANIM_IDLE,		K_ENTER,		-1,		-1, -1},
#endif
#ifdef TURTLEARENA // ALWAYS_RUN // NO_SPEED_KEY
	{"+forward", 		"forward",			ID_FORWARD,		ANIM_RUN,		K_UPARROW,		-1,		-1, -1},
#else
	{"+speed", 			"run / walk",		ID_SPEED,		ANIM_RUN,		K_LEFTSHIFT,	K_RIGHTSHIFT, -1, -1},
	{"+forward", 		"walk forward",		ID_FORWARD,		ANIM_WALK,		K_UPARROW,		-1,		-1, -1},
#endif
	{"+back", 			"backpedal",		ID_BACKPEDAL,	ANIM_BACK,		K_DOWNARROW,	-1,		-1, -1},
#ifdef IOQ3ZTM
	{"+moveleft", 		"step left",		ID_MOVELEFT,	ANIM_STEPLEFT,	'a',			-1,		-1, -1},
	{"+moveright", 		"step right",		ID_MOVERIGHT,	ANIM_STEPRIGHT,	'd',			-1,		-1, -1},
	{"+moveup",			"up / jump",		ID_MOVEUP,		ANIM_JUMP,		K_SPACE,		'/',	-1, -1},
	{"+movedown",		"down / crouch",	ID_MOVEDOWN,	ANIM_CROUCH,	'c',			'.',	-1, -1},
#else
	{"+moveleft", 		"step left",		ID_MOVELEFT,	ANIM_STEPLEFT,	',',			-1,		-1, -1},
	{"+moveright", 		"step right",		ID_MOVERIGHT,	ANIM_STEPRIGHT,	'.',			-1,		-1, -1},
	{"+moveup",			"up / jump",		ID_MOVEUP,		ANIM_JUMP,		K_SPACE,		-1,		-1, -1},
	{"+movedown",		"down / crouch",	ID_MOVEDOWN,	ANIM_CROUCH,	'c',			-1,		-1, -1},
#endif
	{"+left", 			"turn left",		ID_LEFT,		ANIM_TURNLEFT,	K_LEFTARROW,	-1,		-1, -1},
	{"+right", 			"turn right",		ID_RIGHT,		ANIM_TURNRIGHT,	K_RIGHTARROW,	-1,		-1, -1},
	{"+strafe", 		"sidestep / turn",	ID_STRAFE,		ANIM_IDLE,		K_LEFTALT,		K_RIGHTALT, -1, -1},
	{"+lookup", 		"look up",			ID_LOOKUP,		ANIM_LOOKUP,	K_PGDN,			-1,		-1, -1},
	{"+lookdown", 		"look down",		ID_LOOKDOWN,	ANIM_LOOKDOWN,	K_DEL,			-1,		-1, -1},
#ifdef IOQ3ZTM
	{"+mlook", 			"mouse look",		ID_MOUSELOOK,	ANIM_IDLE,		'\\',			-1,		-1, -1},
#else
	{"+mlook", 			"mouse look",		ID_MOUSELOOK,	ANIM_IDLE,		'/',			-1,		-1, -1},
#endif
	{"centerview", 		"center view",		ID_CENTERVIEW,	ANIM_IDLE,		K_END,			-1,		-1, -1},
#ifdef TURTLEARENA // LOCKON
	{"+lockon", 		"lock-on",			ID_LOCKON,		ANIM_IDLE,		K_LEFTSHIFT,	K_RIGHTSHIFT, -1, -1},
#endif
#ifndef TURTLEARENA // NOZOOM
	{"+zoom", 			"zoom view",		ID_ZOOMVIEW,	ANIM_IDLE,		-1,				-1,		-1, -1},
#endif
#ifndef TA_WEAPSYS_EX
	{"weapon 1",		"gauntlet",			ID_WEAPON1,		ANIM_WEAPON1,	'1',			-1,		-1, -1},
	{"weapon 2",		"machinegun",		ID_WEAPON2,		ANIM_WEAPON2,	'2',			-1,		-1, -1},
	{"weapon 3",		"shotgun",			ID_WEAPON3,		ANIM_WEAPON3,	'3',			-1,		-1, -1},
	{"weapon 4",		"grenade launcher",	ID_WEAPON4,		ANIM_WEAPON4,	'4',			-1,		-1, -1},
	{"weapon 5",		"rocket launcher",	ID_WEAPON5,		ANIM_WEAPON5,	'5',			-1,		-1, -1},
	{"weapon 6",		"lightning",		ID_WEAPON6,		ANIM_WEAPON6,	'6',			-1,		-1, -1},
	{"weapon 7",		"railgun",			ID_WEAPON7,		ANIM_WEAPON7,	'7',			-1,		-1, -1},
	{"weapon 8",		"plasma gun",		ID_WEAPON8,		ANIM_WEAPON8,	'8',			-1,		-1, -1},
	{"weapon 9",		"BFG",				ID_WEAPON9,		ANIM_WEAPON9,	'9',			-1,		-1, -1},
#ifdef MISSIONPACK
	{"weapon 11",		"nail gun",			ID_WEAPON11,	ANIM_WEAPON11,	-1,				-1,		-1, -1},
	{"weapon 12",		"proximity mine",	ID_WEAPON12,	ANIM_WEAPON12,	-1,				-1,		-1, -1},
	{"weapon 13",		"chain gun",		ID_WEAPON13,	ANIM_WEAPON13,	-1,				-1,		-1, -1},
#endif
#endif
#ifdef IOQ3ZTM
	{"+attack", 		"attack",			ID_ATTACK,		ANIM_ATTACK,	K_LEFTCTRL,		K_MOUSE1,	-1, -1},
#else
	{"+attack", 		"attack",			ID_ATTACK,		ANIM_ATTACK,	K_LEFTCTRL,		K_RIGHTCTRL, -1, -1},
#endif
#ifdef TA_WEAPSYS_EX
	{"+dropweapon",		"drop weapon",		ID_WEAPDROP,	ANIM_IDLE,		'e',			';',	-1, -1},
#else
	{"weapprev",		"previous weapon",	ID_WEAPPREV,	ANIM_IDLE,		'[',			-1,		-1, -1},
	{"weapnext", 		"next weapon",		ID_WEAPNEXT,	ANIM_IDLE,		']',			-1,		-1, -1},
#endif
#ifdef IOQ3ZTM
	{"+button3", 		"gesture",			ID_GESTURE,		ANIM_GESTURE,	-1,				-1,		-1, -1},
#else
	{"+button3", 		"gesture",			ID_GESTURE,		ANIM_GESTURE,	K_MOUSE3,		-1,		-1, -1},
#endif
	{"messagemode", 	"chat",				ID_CHAT,		ANIM_CHAT,		't',			-1,		-1, -1},
#ifdef TA_MISC // team chat
	{"messagemode2", 	"chat - team",		ID_CHAT2,		ANIM_CHAT,		'y',			-1,		-1, -1},
#else
	{"messagemode2", 	"chat - team",		ID_CHAT2,		ANIM_CHAT,		-1,				-1,		-1, -1},
#endif
	{"messagemode3", 	"chat - target",	ID_CHAT3,		ANIM_CHAT,		-1,				-1,		-1, -1},
	{"messagemode4", 	"chat - attacker",	ID_CHAT4,		ANIM_CHAT,		-1,				-1,		-1, -1},
	{"togglemenu", 		"toggle menu",		ID_TOGGLEMENU,	ANIM_IDLE,		K_ESCAPE,		-1,		-1, -1},
#ifdef TA_HOLDSYS/*2*/
	{"holdnext",		"next item",		ID_NEXTITEM,	ANIM_IDLE,		K_MWHEELUP,		']',	-1, -1},
	{"holdprev",		"prev item",		ID_PREVITEM,	ANIM_IDLE,		K_MWHEELDOWN,	'[',	-1, -1},
#endif
#ifdef TA_MISC // DROP_FLAG
	{"dropflag",		"drop flag",		ID_DROPFLAG,	ANIM_IDLE,		'f',			'\'',	-1, -1},
#endif
#ifdef IOQ3ZTM
	{"vote yes",		"vote yes",			ID_VOTEYES,		ANIM_IDLE,		K_F1,			-1,		-1, -1},
	{"vote no",			"vote no",			ID_VOTENO,		ANIM_IDLE,		K_F2,			-1,		-1, -1},
	{"follownext",		"view another player",ID_FOLLOWPLAYER,ANIM_IDLE,	K_F12,			-1,		-1, -1},
	{"screenshot",		"take screenshot",	ID_TAKESCREENSHOT,ANIM_IDLE,	K_F11,			-1,		-1, -1},
#endif
#ifdef IOQ3ZTM // NEW_CAM
	{"camreset",		"reset camera",		ID_CAMRESET,	ANIM_IDLE,		K_KP_5,			-1,		-1, -1},
	{"toggle cg_thirdperson","toggle third person",ID_CAMTOGGLE,ANIM_IDLE,	K_KP_HOME,		-1,		-1, -1},
	{"+camright",		"turn camera right",ID_CAMRIGHT,	ANIM_IDLE,		K_KP_RIGHTARROW,-1,		-1, -1},
	{"+camleft",		"turn camera left",	ID_CAMLEFT,		ANIM_IDLE,		K_KP_LEFTARROW,	-1,		-1, -1},
	{"+camZoomIn",		"zoom in",			ID_CAMZOOMIN,	ANIM_IDLE,		K_KP_UPARROW,	-1,		-1, -1},
	{"+camZoomOut",		"zoom out",			ID_CAMZOOMOUT,	ANIM_IDLE,		K_KP_DOWNARROW,	-1,		-1, -1},
#endif
	{(char*)NULL,		(char*)NULL,		0,				0,				-1,				-1,		-1,	-1},
};

#define MINIBIND(id, d1,d2) {(char*)NULL, (char*)NULL, (id), 0, (d1), (d2), -1, -1}

static bind_t g_bindings2[] =
{
	MINIBIND(ID_SHOWSCORES, -1, -1),
#ifdef MISSIONPACK_HUD
	MINIBIND(ID_SCORESUP, -1, -1),
	MINIBIND(ID_SCORESDOWN, -1, -1),
#endif
	MINIBIND(ID_USEITEM, -1, -1),
#ifndef TURTLEARENA // ALWAYS_RUN // NO_SPEED_KEY
	MINIBIND(ID_SPEED, -1, -1),
#endif
	MINIBIND(ID_FORWARD, -1, -1),
	MINIBIND(ID_BACKPEDAL, -1, -1),
	MINIBIND(ID_MOVELEFT, -1, -1),
	MINIBIND(ID_MOVERIGHT, -1, -1),
	MINIBIND(ID_MOVEUP, -1, -1),
	MINIBIND(ID_MOVEDOWN, -1, -1),
	MINIBIND(ID_LEFT, -1, -1),
	MINIBIND(ID_RIGHT, -1, -1),
	MINIBIND(ID_STRAFE, -1, -1),
	MINIBIND(ID_LOOKUP, -1, -1),
	MINIBIND(ID_LOOKDOWN, -1, -1),
	MINIBIND(ID_MOUSELOOK, -1, -1),
	MINIBIND(ID_CENTERVIEW, -1, -1),
#ifdef TURTLEARENA // LOCKON
	MINIBIND(ID_LOCKON, -1, -1),
#endif
#ifndef TURTLEARENA // NOZOOM
	MINIBIND(ID_ZOOMVIEW, -1, -1),
#endif
#ifndef TA_WEAPSYS_EX
	MINIBIND(ID_WEAPON1, -1, -1),
	MINIBIND(ID_WEAPON2, -1, -1),
	MINIBIND(ID_WEAPON3, -1, -1),
	MINIBIND(ID_WEAPON4, -1, -1),
	MINIBIND(ID_WEAPON5, -1, -1),
	MINIBIND(ID_WEAPON6, -1, -1),
	MINIBIND(ID_WEAPON7, -1, -1),
	MINIBIND(ID_WEAPON8, -1, -1),
	MINIBIND(ID_WEAPON9, -1, -1),
#ifdef MISSIONPACK
	MINIBIND(ID_WEAPON11, -1, -1),
	MINIBIND(ID_WEAPON12, -1, -1),
	MINIBIND(ID_WEAPON13, -1, -1),
#endif
#endif
	MINIBIND(ID_ATTACK, -1, -1),
#ifdef TA_WEAPSYS_EX
	MINIBIND(ID_WEAPDROP, -1, -1),
#else
	MINIBIND(ID_WEAPPREV, -1, -1),
	MINIBIND(ID_WEAPNEXT, -1, -1),
#endif
	MINIBIND(ID_GESTURE, -1, -1),
	MINIBIND(ID_CHAT, -1, -1),
	MINIBIND(ID_CHAT2, -1, -1),
	MINIBIND(ID_CHAT3, -1, -1),
	MINIBIND(ID_CHAT4, -1, -1),
#ifdef TA_HOLDSYS/*2*/
	MINIBIND(ID_NEXTITEM, -1, -1),
	MINIBIND(ID_PREVITEM, -1, -1),
#endif
#ifdef TA_MISC // DROP_FLAG
	MINIBIND(ID_DROPFLAG, -1, -1),
#endif
#ifdef IOQ3ZTM
	MINIBIND(ID_VOTEYES, -1, -1),
	MINIBIND(ID_VOTENO, -1, -1),
	MINIBIND(ID_FOLLOWPLAYER, -1, -1),
	MINIBIND(ID_TAKESCREENSHOT, -1, -1),
#endif
#ifdef IOQ3ZTM // NEW_CAM
	MINIBIND(ID_CAMRESET, -1, -1),
	MINIBIND(ID_CAMTOGGLE, -1, -1),
	MINIBIND(ID_CAMRIGHT, -1, -1),
	MINIBIND(ID_CAMLEFT, -1, -1),
	MINIBIND(ID_CAMZOOMIN, -1, -1),
	MINIBIND(ID_CAMZOOMOUT, -1, -1),
#endif
	MINIBIND(0, -1, -1),
};

static bind_t g_bindings3[] =
{
	MINIBIND(ID_SHOWSCORES, -1, -1),
#ifdef MISSIONPACK_HUD
	MINIBIND(ID_SCORESUP, -1, -1),
	MINIBIND(ID_SCORESDOWN, -1, -1),
#endif
	MINIBIND(ID_USEITEM, -1, -1),
#ifndef TURTLEARENA // ALWAYS_RUN // NO_SPEED_KEY
	MINIBIND(ID_SPEED, -1, -1),
#endif
	MINIBIND(ID_FORWARD, -1, -1),
	MINIBIND(ID_BACKPEDAL, -1, -1),
	MINIBIND(ID_MOVELEFT, -1, -1),
	MINIBIND(ID_MOVERIGHT, -1, -1),
	MINIBIND(ID_MOVEUP, -1, -1),
	MINIBIND(ID_MOVEDOWN, -1, -1),
	MINIBIND(ID_LEFT, -1, -1),
	MINIBIND(ID_RIGHT, -1, -1),
	MINIBIND(ID_STRAFE, -1, -1),
	MINIBIND(ID_LOOKUP, -1, -1),
	MINIBIND(ID_LOOKDOWN, -1, -1),
	MINIBIND(ID_MOUSELOOK, -1, -1),
	MINIBIND(ID_CENTERVIEW, -1, -1),
#ifdef TURTLEARENA // LOCKON
	MINIBIND(ID_LOCKON, -1, -1),
#endif
#ifndef TURTLEARENA // NOZOOM
	MINIBIND(ID_ZOOMVIEW, -1, -1),
#endif
#ifndef TA_WEAPSYS_EX
	MINIBIND(ID_WEAPON1, -1, -1),
	MINIBIND(ID_WEAPON2, -1, -1),
	MINIBIND(ID_WEAPON3, -1, -1),
	MINIBIND(ID_WEAPON4, -1, -1),
	MINIBIND(ID_WEAPON5, -1, -1),
	MINIBIND(ID_WEAPON6, -1, -1),
	MINIBIND(ID_WEAPON7, -1, -1),
	MINIBIND(ID_WEAPON8, -1, -1),
	MINIBIND(ID_WEAPON9, -1, -1),
#ifdef MISSIONPACK
	MINIBIND(ID_WEAPON11, -1, -1),
	MINIBIND(ID_WEAPON12, -1, -1),
	MINIBIND(ID_WEAPON13, -1, -1),
#endif
#endif
	MINIBIND(ID_ATTACK, -1, -1),
#ifdef TA_WEAPSYS_EX
	MINIBIND(ID_WEAPDROP, -1, -1),
#else
	MINIBIND(ID_WEAPPREV, -1, -1),
	MINIBIND(ID_WEAPNEXT, -1, -1),
#endif
	MINIBIND(ID_GESTURE, -1, -1),
	MINIBIND(ID_CHAT, -1, -1),
	MINIBIND(ID_CHAT2, -1, -1),
	MINIBIND(ID_CHAT3, -1, -1),
	MINIBIND(ID_CHAT4, -1, -1),
#ifdef TA_HOLDSYS/*2*/
	MINIBIND(ID_NEXTITEM, -1, -1),
	MINIBIND(ID_PREVITEM, -1, -1),
#endif
#ifdef TA_MISC // DROP_FLAG
	MINIBIND(ID_DROPFLAG, -1, -1),
#endif
#ifdef IOQ3ZTM
	MINIBIND(ID_VOTEYES, -1, -1),
	MINIBIND(ID_VOTENO, -1, -1),
	MINIBIND(ID_FOLLOWPLAYER, -1, -1),
	MINIBIND(ID_TAKESCREENSHOT, -1, -1),
#endif
#ifdef IOQ3ZTM // NEW_CAM
	MINIBIND(ID_CAMRESET, -1, -1),
	MINIBIND(ID_CAMTOGGLE, -1, -1),
	MINIBIND(ID_CAMRIGHT, -1, -1),
	MINIBIND(ID_CAMLEFT, -1, -1),
	MINIBIND(ID_CAMZOOMIN, -1, -1),
	MINIBIND(ID_CAMZOOMOUT, -1, -1),
#endif
	MINIBIND(0, -1, -1),
};

static bind_t g_bindings4[] =
{
	MINIBIND(ID_SHOWSCORES, -1, -1),
#ifdef MISSIONPACK_HUD
	MINIBIND(ID_SCORESUP, -1, -1),
	MINIBIND(ID_SCORESDOWN, -1, -1),
#endif
	MINIBIND(ID_USEITEM, -1, -1),
#ifndef TURTLEARENA // ALWAYS_RUN // NO_SPEED_KEY
	MINIBIND(ID_SPEED, -1, -1),
#endif
	MINIBIND(ID_FORWARD, -1, -1),
	MINIBIND(ID_BACKPEDAL, -1, -1),
	MINIBIND(ID_MOVELEFT, -1, -1),
	MINIBIND(ID_MOVERIGHT, -1, -1),
	MINIBIND(ID_MOVEUP, -1, -1),
	MINIBIND(ID_MOVEDOWN, -1, -1),
	MINIBIND(ID_LEFT, -1, -1),
	MINIBIND(ID_RIGHT, -1, -1),
	MINIBIND(ID_STRAFE, -1, -1),
	MINIBIND(ID_LOOKUP, -1, -1),
	MINIBIND(ID_LOOKDOWN, -1, -1),
	MINIBIND(ID_MOUSELOOK, -1, -1),
	MINIBIND(ID_CENTERVIEW, -1, -1),
#ifdef TURTLEARENA // LOCKON
	MINIBIND(ID_LOCKON, -1, -1),
#endif
#ifndef TURTLEARENA // NOZOOM
	MINIBIND(ID_ZOOMVIEW, -1, -1),
#endif
#ifndef TA_WEAPSYS_EX
	MINIBIND(ID_WEAPON1, -1, -1),
	MINIBIND(ID_WEAPON2, -1, -1),
	MINIBIND(ID_WEAPON3, -1, -1),
	MINIBIND(ID_WEAPON4, -1, -1),
	MINIBIND(ID_WEAPON5, -1, -1),
	MINIBIND(ID_WEAPON6, -1, -1),
	MINIBIND(ID_WEAPON7, -1, -1),
	MINIBIND(ID_WEAPON8, -1, -1),
	MINIBIND(ID_WEAPON9, -1, -1),
#ifdef MISSIONPACK
	MINIBIND(ID_WEAPON11, -1, -1),
	MINIBIND(ID_WEAPON12, -1, -1),
	MINIBIND(ID_WEAPON13, -1, -1),
#endif
#endif
	MINIBIND(ID_ATTACK, -1, -1),
#ifdef TA_WEAPSYS_EX
	MINIBIND(ID_WEAPDROP, -1, -1),
#else
	MINIBIND(ID_WEAPPREV, -1, -1),
	MINIBIND(ID_WEAPNEXT, -1, -1),
#endif
	MINIBIND(ID_GESTURE, -1, -1),
	MINIBIND(ID_CHAT, -1, -1),
	MINIBIND(ID_CHAT2, -1, -1),
	MINIBIND(ID_CHAT3, -1, -1),
	MINIBIND(ID_CHAT4, -1, -1),
#ifdef TA_HOLDSYS/*2*/
	MINIBIND(ID_NEXTITEM, -1, -1),
	MINIBIND(ID_PREVITEM, -1, -1),
#endif
#ifdef TA_MISC // DROP_FLAG
	MINIBIND(ID_DROPFLAG, -1, -1),
#endif
#ifdef IOQ3ZTM
	MINIBIND(ID_VOTEYES, -1, -1),
	MINIBIND(ID_VOTENO, -1, -1),
	MINIBIND(ID_FOLLOWPLAYER, -1, -1),
	MINIBIND(ID_TAKESCREENSHOT, -1, -1),
#endif
#ifdef IOQ3ZTM // NEW_CAM
	MINIBIND(ID_CAMRESET, -1, -1),
	MINIBIND(ID_CAMTOGGLE, -1, -1),
	MINIBIND(ID_CAMRIGHT, -1, -1),
	MINIBIND(ID_CAMLEFT, -1, -1),
	MINIBIND(ID_CAMZOOMIN, -1, -1),
	MINIBIND(ID_CAMZOOMOUT, -1, -1),
#endif
	MINIBIND(0, -1, -1),
};

bind_t *g_bindings_list[MAX_SPLITVIEW] =
{
	g_bindings,
	g_bindings2,
	g_bindings3,
	g_bindings4
};

static menucommon_s *g_movement_controls[] =
{
#ifdef TURTLEARENA // LOCKON
	(menucommon_s *)&s_controls.lockon,
#endif
#ifndef TURTLEARENA // ALWAYS_RUN
	(menucommon_s *)&s_controls.alwaysrun,     
#endif
#ifndef TURTLEARENA // NO_SPEED_KEY
	(menucommon_s *)&s_controls.run,    
#endif        
	(menucommon_s *)&s_controls.walkforward,
	(menucommon_s *)&s_controls.backpedal,
	(menucommon_s *)&s_controls.stepleft,      
	(menucommon_s *)&s_controls.stepright,     
	(menucommon_s *)&s_controls.moveup,        
	(menucommon_s *)&s_controls.movedown,      
	(menucommon_s *)&s_controls.turnleft,      
	(menucommon_s *)&s_controls.turnright,     
	(menucommon_s *)&s_controls.sidestep,
	NULL
};

static menucommon_s *g_weapons_controls[] = {
	(menucommon_s *)&s_controls.attack,           
#ifdef TA_WEAPSYS_EX
	(menucommon_s *)&s_controls.dropweapon,
#else
	(menucommon_s *)&s_controls.nextweapon,
	(menucommon_s *)&s_controls.prevweapon,
	(menucommon_s *)&s_controls.cyclepastgauntlet,
	(menucommon_s *)&s_controls.autoswitch,    
	(menucommon_s *)&s_controls.gauntlet,         
	(menucommon_s *)&s_controls.machinegun,
	(menucommon_s *)&s_controls.shotgun,          
	(menucommon_s *)&s_controls.grenadelauncher,
	(menucommon_s *)&s_controls.rocketlauncher,   
	(menucommon_s *)&s_controls.lightning,   
	(menucommon_s *)&s_controls.railgun,          
	(menucommon_s *)&s_controls.plasma,           
	(menucommon_s *)&s_controls.bfg,              
#ifdef MISSIONPACK
	(menucommon_s *)&s_controls.nailgun,
	(menucommon_s *)&s_controls.proxylauncher,
	(menucommon_s *)&s_controls.chaingun,
#endif
#endif
#ifdef TA_MISC // DROP_FLAG
	(menucommon_s *)&s_controls.dropflag,
#endif
#ifdef TA_MISC
	(menucommon_s *)&s_controls.useitem,
#endif
#ifdef TA_HOLDSYS/*2*/
	(menucommon_s *)&s_controls.nextitem,
	(menucommon_s *)&s_controls.previtem,
#endif
	NULL,
};

static menucommon_s *g_looking_controls[] = {
	(menucommon_s *)&s_controls.sensitivity,
	(menucommon_s *)&s_controls.smoothmouse,
	(menucommon_s *)&s_controls.invertmouse,
	(menucommon_s *)&s_controls.lookup,
	(menucommon_s *)&s_controls.lookdown,
	(menucommon_s *)&s_controls.mouselook,
	(menucommon_s *)&s_controls.freelook,
	(menucommon_s *)&s_controls.centerview,
#ifndef TURTLEARENA // NOZOOM
	(menucommon_s *)&s_controls.zoomview,
#endif
	NULL,
};

static menucommon_s *g_misc_controls[] = {
	(menucommon_s *)&s_controls.showscores, 
#ifdef MISSIONPACK_HUD
	(menucommon_s *)&s_controls.scoresup,
	(menucommon_s *)&s_controls.scoresdown,
#endif
#ifndef TA_MISC
	(menucommon_s *)&s_controls.useitem,
#endif
	(menucommon_s *)&s_controls.gesture,
	(menucommon_s *)&s_controls.chat,
	(menucommon_s *)&s_controls.chat2,
	(menucommon_s *)&s_controls.chat3,
	(menucommon_s *)&s_controls.chat4,
#ifdef IOQ3ZTM
	(menucommon_s *)&s_controls.voteYes,
	(menucommon_s *)&s_controls.voteNo,
	(menucommon_s *)&s_controls.followPlayer,
	(menucommon_s *)&s_controls.takeScreenshot,
#endif
	(menucommon_s *)&s_controls.togglemenu,
	NULL,
};

#ifdef IOQ3ZTM // NEW_CAMERA
static menucommon_s *g_camera_controls[] = {
	(menucommon_s *)&s_controls.camreset,
	(menucommon_s *)&s_controls.camtoggle,
	(menucommon_s *)&s_controls.camright,
	(menucommon_s *)&s_controls.camleft,
	(menucommon_s *)&s_controls.camzoomin,
	(menucommon_s *)&s_controls.camzoomout,
	NULL,
};
#endif

static menucommon_s *g_joystick_controls[] = {
	(menucommon_s *)&s_controls.selectjoy,
	(menucommon_s *)&s_controls.joythreshold,
	(menucommon_s *)&s_controls.joyanalog,
	NULL,
};

static menucommon_s **g_controls[] = {
	g_movement_controls,
	g_looking_controls,
	g_weapons_controls,
	g_misc_controls,
#ifdef IOQ3ZTM // NEW_CAMERA
	g_camera_controls,
#endif
	g_joystick_controls,
	NULL
};

static menucommon_s *g_looking_mini_controls[] = {
	(menucommon_s *)&s_controls.lookup,
	(menucommon_s *)&s_controls.lookdown,
	(menucommon_s *)&s_controls.centerview,
#ifndef TURTLEARENA // NOZOOM
	(menucommon_s *)&s_controls.zoomview,
#endif
	NULL,
};

static menucommon_s *g_misc_mini_controls[] = {
	(menucommon_s *)&s_controls.showscores,
#ifdef MISSIONPACK_HUD
	(menucommon_s *)&s_controls.scoresup,
	(menucommon_s *)&s_controls.scoresdown,
#endif
#ifndef TA_MISC
	(menucommon_s *)&s_controls.useitem,
#endif
	(menucommon_s *)&s_controls.gesture,
#ifdef IOQ3ZTM
	(menucommon_s *)&s_controls.voteYes,
	(menucommon_s *)&s_controls.voteNo,
	(menucommon_s *)&s_controls.followPlayer,
#endif
	NULL,
};

static menucommon_s *g_unused_controls[] = {
	// looking
	(menucommon_s *)&s_controls.sensitivity,
	(menucommon_s *)&s_controls.smoothmouse,
	(menucommon_s *)&s_controls.invertmouse,
	(menucommon_s *)&s_controls.mouselook,
	(menucommon_s *)&s_controls.freelook,

	// misc
	(menucommon_s *)&s_controls.chat,
	(menucommon_s *)&s_controls.chat2,
	(menucommon_s *)&s_controls.chat3,
	(menucommon_s *)&s_controls.chat4,
#ifdef IOQ3ZTM
	(menucommon_s *)&s_controls.takeScreenshot,
#endif
	NULL,
};

static menucommon_s **g_mini_controls[] = {
	g_movement_controls,
	g_looking_mini_controls,
	g_weapons_controls,
	g_misc_mini_controls,
#ifdef IOQ3ZTM // NEW_CAMERA
	g_camera_controls,
#endif
	g_joystick_controls,
	g_unused_controls, // dummy controls that are not used but are disabled so they are not seen.
	NULL
};

/*
=================
Controls_GetCvarDefault
=================
*/
static float Controls_GetCvarDefault( char* name )
{
	char defaultvaluebuf[MAX_CVAR_VALUE_STRING];

	trap_Cvar_DefaultVariableStringBuffer( name, defaultvaluebuf, sizeof( defaultvaluebuf ) );

	return atof( defaultvaluebuf );
}

/*
=================
Controls_GetCvarValue
=================
*/
static float Controls_GetCvarValue( char* name )
{
	return trap_Cvar_VariableValue( name );
}


#ifndef TA_MISC
/*
=================
Controls_UpdateModel
=================
*/
static void Controls_UpdateModel( int anim ) {
	VectorClear( s_controls.playerViewangles );
	VectorClear( s_controls.playerMoveangles );
	s_controls.playerViewangles[YAW] = 180 - 30;
	s_controls.playerMoveangles[YAW] = s_controls.playerViewangles[YAW];
#ifdef TA_WEAPSYS
	s_controls.playerLegs		     = BG_LegsStandForWeapon(&s_controls.playerinfo.playercfg,
												s_controls.playerinfo.playercfg.default_weapon);
	s_controls.playerTorso			 = BG_TorsoStandForWeapon(s_controls.playerinfo.playercfg.default_weapon);
#else
	s_controls.playerLegs		     = LEGS_IDLE;
	s_controls.playerTorso			 = TORSO_STAND;
#endif
	s_controls.playerWeapon			 = WP_NUM_WEAPONS;
	s_controls.playerChat			 = qfalse;

	switch( anim ) {
	case ANIM_RUN:	
		s_controls.playerLegs = LEGS_RUN;
		break;

	case ANIM_WALK:	
		s_controls.playerLegs = LEGS_WALK;
		break;

	case ANIM_BACK:	
		s_controls.playerLegs = LEGS_BACK;
		break;

	case ANIM_JUMP:	
		s_controls.playerLegs = LEGS_JUMP;
		break;

	case ANIM_CROUCH:	
		s_controls.playerLegs = LEGS_IDLECR;
		break;

	case ANIM_TURNLEFT:
		s_controls.playerViewangles[YAW] += 90;
		break;

	case ANIM_TURNRIGHT:
		s_controls.playerViewangles[YAW] -= 90;
		break;

	case ANIM_STEPLEFT:
		s_controls.playerLegs = LEGS_WALK;
		s_controls.playerMoveangles[YAW] = s_controls.playerViewangles[YAW] + 90;
		break;

	case ANIM_STEPRIGHT:
		s_controls.playerLegs = LEGS_WALK;
		s_controls.playerMoveangles[YAW] = s_controls.playerViewangles[YAW] - 90;
		break;

	case ANIM_LOOKUP:
		s_controls.playerViewangles[PITCH] = -45;
		break;

	case ANIM_LOOKDOWN:
		s_controls.playerViewangles[PITCH] = 45;
		break;

#ifndef TA_WEAPSYS_EX
#ifdef TA_WEAPSYS
	case ANIM_WEAPON1:
	case ANIM_WEAPON2:
	case ANIM_WEAPON3:
	case ANIM_WEAPON4:
	case ANIM_WEAPON5:
	case ANIM_WEAPON6:
	case ANIM_WEAPON7:
	case ANIM_WEAPON8:
	case ANIM_WEAPON9:
	case ANIM_WEAPON10:
	// ZTM: TODO?: Support upto 16 weapons in menu?
		s_controls.playerWeapon = anim - ANIM_WEAPON1 + 1;
		break;
#else
	case ANIM_WEAPON1:
		s_controls.playerWeapon = WP_GAUNTLET;
		break;

	case ANIM_WEAPON2:
		s_controls.playerWeapon = WP_MACHINEGUN;
		break;

	case ANIM_WEAPON3:
		s_controls.playerWeapon = WP_SHOTGUN;
		break;

	case ANIM_WEAPON4:
		s_controls.playerWeapon = WP_GRENADE_LAUNCHER;
		break;

	case ANIM_WEAPON5:
		s_controls.playerWeapon = WP_ROCKET_LAUNCHER;
		break;

	case ANIM_WEAPON6:
		s_controls.playerWeapon = WP_LIGHTNING;
		break;

	case ANIM_WEAPON7:
		s_controls.playerWeapon = WP_RAILGUN;
		break;

	case ANIM_WEAPON8:
		s_controls.playerWeapon = WP_PLASMAGUN;
		break;

	case ANIM_WEAPON9:
		s_controls.playerWeapon = WP_BFG;
		break;

	case ANIM_WEAPON10:
		s_controls.playerWeapon = WP_GRAPPLING_HOOK;
		break;
#endif
#endif

#ifdef MISSIONPACK
	case ANIM_WEAPON11:
		s_controls.playerWeapon = WP_NAILGUN;
		break;

	case ANIM_WEAPON12:
		s_controls.playerWeapon = WP_PROX_LAUNCHER;
		break;

	case ANIM_WEAPON13:
		s_controls.playerWeapon = WP_CHAINGUN;
		break;
#endif

	case ANIM_ATTACK:
		s_controls.playerTorso = TORSO_ATTACK;
		break;

	case ANIM_GESTURE:
		s_controls.playerTorso = TORSO_GESTURE;
		break;

	case ANIM_DIE:
		s_controls.playerLegs = BOTH_DEATH1;
		s_controls.playerTorso = BOTH_DEATH1;
		s_controls.playerWeapon = WP_NONE;
		break;

	case ANIM_CHAT:
		s_controls.playerChat = qtrue;
		break;

	default:
		break;
	}

	UI_PlayerInfo_SetInfo( &s_controls.playerinfo, s_controls.localPlayerNum, s_controls.playerLegs, s_controls.playerTorso, s_controls.playerViewangles, s_controls.playerMoveangles, s_controls.playerWeapon, s_controls.playerChat );
}
#endif


/*
=================
Controls_Update
=================
*/
static void Controls_Update( void ) {
	int		i;
	int		j;
	int		y;
	menucommon_s	**controls;
	menucommon_s	*control;

	// disable all controls in all groups
	for( i = 0; s_controls.controls[i] != NULL; i++ ) {
		controls = s_controls.controls[i];
		for( j = 0;  (control = controls[j]) ; j++ ) {
			control->flags |= (QMF_HIDDEN|QMF_INACTIVE);
		}
	}

	controls = s_controls.controls[s_controls.section];

	// enable controls in active group (and count number of items for vertical centering)
	for( j = 0;  (control = controls[j]) ; j++ ) {
		control->flags &= ~(QMF_GRAYED|QMF_HIDDEN|QMF_INACTIVE);
	}

	// position controls
	y = ( SCREEN_HEIGHT - j * SMALLCHAR_HEIGHT ) / 2;
	for( j = 0;	(control = controls[j]) ; j++, y += SMALLCHAR_HEIGHT ) {
#ifdef TA_MISC
		control->x      = 384;
#else
		control->x      = 320;
#endif
		control->y      = y;
#ifdef TA_MISC
		control->left   = control->x - 21*SMALLCHAR_WIDTH;
		control->right  = control->x + 21*SMALLCHAR_WIDTH;
#else
		control->left   = 320 - 19*SMALLCHAR_WIDTH;
		control->right  = 320 + 21*SMALLCHAR_WIDTH;
#endif
		control->top    = y;
		control->bottom = y + SMALLCHAR_HEIGHT;
	}

	if( waitingforkey ) {
		// disable everybody
		for( i = 0; i < s_controls.menu.nitems; i++ ) {
			((menucommon_s*)(s_controls.menu.items[i]))->flags |= QMF_GRAYED;
		}

		// enable action item
		((menucommon_s*)(s_controls.menu.items[s_controls.menu.cursor]))->flags &= ~QMF_GRAYED;

#ifndef TA_MISC
		// don't gray out player's name
		s_controls.name.generic.flags &= ~QMF_GRAYED;
#endif

		return;
	}

	// enable everybody
	for( i = 0; i < s_controls.menu.nitems; i++ ) {
		((menucommon_s*)(s_controls.menu.items[i]))->flags &= ~QMF_GRAYED;
	}

	// makes sure flags are right on the group selection controls
	s_controls.looking.generic.flags  &= ~(QMF_GRAYED|QMF_HIGHLIGHT|QMF_HIGHLIGHT_IF_FOCUS);
	s_controls.movement.generic.flags &= ~(QMF_GRAYED|QMF_HIGHLIGHT|QMF_HIGHLIGHT_IF_FOCUS);
	s_controls.weapons.generic.flags  &= ~(QMF_GRAYED|QMF_HIGHLIGHT|QMF_HIGHLIGHT_IF_FOCUS);
	s_controls.misc.generic.flags     &= ~(QMF_GRAYED|QMF_HIGHLIGHT|QMF_HIGHLIGHT_IF_FOCUS);
#ifdef IOQ3ZTM // NEW_CAM
	s_controls.camera.generic.flags     &= ~(QMF_GRAYED|QMF_HIGHLIGHT|QMF_HIGHLIGHT_IF_FOCUS);
#endif

	s_controls.looking.generic.flags  |= QMF_PULSEIFFOCUS;
	s_controls.movement.generic.flags |= QMF_PULSEIFFOCUS;
	s_controls.weapons.generic.flags  |= QMF_PULSEIFFOCUS;
	s_controls.misc.generic.flags     |= QMF_PULSEIFFOCUS;
#ifdef IOQ3ZTM // NEW_CAM
	s_controls.camera.generic.flags     |= QMF_PULSEIFFOCUS;
#endif

	// set buttons
	switch( s_controls.section ) {
	case C_MOVEMENT:
		s_controls.movement.generic.flags &= ~QMF_PULSEIFFOCUS;
		s_controls.movement.generic.flags |= (QMF_HIGHLIGHT|QMF_HIGHLIGHT_IF_FOCUS);
		break;
	
	case C_LOOKING:
		s_controls.looking.generic.flags &= ~QMF_PULSEIFFOCUS;
		s_controls.looking.generic.flags |= (QMF_HIGHLIGHT|QMF_HIGHLIGHT_IF_FOCUS);
		break;
	
	case C_WEAPONS:
		s_controls.weapons.generic.flags &= ~QMF_PULSEIFFOCUS;
		s_controls.weapons.generic.flags |= (QMF_HIGHLIGHT|QMF_HIGHLIGHT_IF_FOCUS);
		break;		

	case C_MISC:
		s_controls.misc.generic.flags &= ~QMF_PULSEIFFOCUS;
		s_controls.misc.generic.flags |= (QMF_HIGHLIGHT|QMF_HIGHLIGHT_IF_FOCUS);
		break;

#ifdef IOQ3ZTM // NEW_CAM
	case C_CAMERA:
		s_controls.camera.generic.flags &= ~QMF_PULSEIFFOCUS;
		s_controls.camera.generic.flags |= (QMF_HIGHLIGHT|QMF_HIGHLIGHT_IF_FOCUS);
		break;
#endif
	}
}


/*
=================
Controls_DrawKeyBinding
=================
*/
static void Controls_DrawKeyBinding( void *self )
{
	menuaction_s*	a;
	int				x;
	int				y;
	int				b1;
	int				b2;
	qboolean		c;
	char			name[32];
	char			name2[32];
	bind_t			*bindptr;

	a = (menuaction_s*) self;

	x =	a->generic.x;
	y = a->generic.y;

	c = (Menu_ItemAtCursor( a->generic.parent ) == a);

	bindptr = &s_controls.bindings[a->generic.id];

	b1 = bindptr->bind1;
	if (b1 == -1)
		strcpy(name,"???");
	else
	{
		trap_Key_KeynumToStringBuf( b1, name, 32 );
		Q_strupr(name);

		b2 = bindptr->bind2;
		if (b2 != -1)
		{
			trap_Key_KeynumToStringBuf( b2, name2, 32 );
			Q_strupr(name2);

			strcat( name, " or " );
			strcat( name, name2 );
		}
	}

	if (c)
	{
		CG_FillRect( a->generic.left, a->generic.top, a->generic.right-a->generic.left+1, a->generic.bottom-a->generic.top+1, listbar_color ); 

		UI_DrawString( x - SMALLCHAR_WIDTH, y, g_bindings[a->generic.id].label, UI_RIGHT|UI_SMALLFONT, text_color_highlight );
		UI_DrawString( x + SMALLCHAR_WIDTH, y, name, UI_LEFT|UI_SMALLFONT|UI_PULSE, text_color_highlight );

		if (waitingforkey)
		{
			UI_DrawChar( x, y, '=', UI_CENTER|UI_BLINK|UI_SMALLFONT, text_color_highlight);
			UI_DrawString(SCREEN_WIDTH * 0.50, SCREEN_HEIGHT * 0.80, "Waiting for new key ... ESCAPE to cancel", UI_SMALLFONT|UI_CENTER|UI_PULSE, colorWhite );
		}
		else
		{
			UI_DrawChar( x, y, GLYPH_ARROW, UI_CENTER|UI_BLINK|UI_SMALLFONT, text_color_highlight);
			UI_DrawString(SCREEN_WIDTH * 0.50, SCREEN_HEIGHT * 0.78, "Press ENTER or CLICK to change", UI_SMALLFONT|UI_CENTER, colorWhite );
			UI_DrawString(SCREEN_WIDTH * 0.50, SCREEN_HEIGHT * 0.82, "Press BACKSPACE to clear", UI_SMALLFONT|UI_CENTER, colorWhite );
		}
	}
	else
	{
		if (a->generic.flags & QMF_GRAYED)
		{
			UI_DrawString( x - SMALLCHAR_WIDTH, y, g_bindings[a->generic.id].label, UI_RIGHT|UI_SMALLFONT, text_color_disabled );
			UI_DrawString( x + SMALLCHAR_WIDTH, y, name, UI_LEFT|UI_SMALLFONT, text_color_disabled );
		}
		else
		{
			UI_DrawString( x - SMALLCHAR_WIDTH, y, g_bindings[a->generic.id].label, UI_RIGHT|UI_SMALLFONT, text_color_normal );
			UI_DrawString( x + SMALLCHAR_WIDTH, y, name, UI_LEFT|UI_SMALLFONT, text_color_normal );
		}
	}
}

/*
=================
Controls_DrawSmallText
=================
*/
static void Controls_DrawSmallText( void *self )
{
	menutext_s *text;
	int x, y;
	qboolean c;

	text = (menutext_s*) self;

	c = (Menu_ItemAtCursor( text->generic.parent ) == text);

	x =	text->generic.x;
	y = text->generic.y;

	if (c)
	{
		CG_FillRect( text->generic.left, text->generic.top, text->generic.right-text->generic.left+1, text->generic.bottom-text->generic.top+1, listbar_color ); 

		UI_DrawString( x - SMALLCHAR_WIDTH, y, text->string, UI_RIGHT|UI_SMALLFONT, text_color_highlight );

		UI_DrawString(SCREEN_WIDTH * 0.50, SCREEN_HEIGHT * 0.80, "CLICK to change", UI_SMALLFONT|UI_CENTER, colorWhite );
	}
	else
	{
		if (text->generic.flags & QMF_GRAYED)
		{
			UI_DrawString( x - SMALLCHAR_WIDTH, y, text->string, UI_RIGHT|UI_SMALLFONT, text_color_disabled );
		}
		else
		{
			UI_DrawString( x - SMALLCHAR_WIDTH, y, text->string, UI_RIGHT|UI_SMALLFONT, text_color_normal );
		}
	}
}


/*
=================
Controls_StatusBar
=================
*/
static void Controls_StatusBar( void *self )
{
	UI_DrawString(SCREEN_WIDTH * 0.50, SCREEN_HEIGHT * 0.80, "Use Arrow Keys or CLICK to change", UI_SMALLFONT|UI_CENTER, colorWhite );
}


#ifndef TA_MISC
/*
=================
Controls_DrawPlayer
=================
*/
static void Controls_DrawPlayer( void *self ) {
	menubitmap_s	*b;
	char			model[MAX_QPATH], headmodel[MAX_QPATH];

	trap_Cvar_VariableStringBuffer( Com_LocalPlayerCvarName(s_controls.localPlayerNum, "model"), model, sizeof( model ) );
	trap_Cvar_VariableStringBuffer( Com_LocalPlayerCvarName(s_controls.localPlayerNum, "headmodel"), headmodel, sizeof( headmodel ) );
	if ( strcmp( model, s_controls.playerModel ) != 0 || strcmp( headmodel, s_controls.playerHead ) != 0 ) {
		UI_PlayerInfo_SetModel( &s_controls.playerinfo, model, headmodel, NULL );
		strcpy( s_controls.playerModel, model );
		strcpy( s_controls.playerHead, headmodel );
		Controls_UpdateModel( ANIM_IDLE );
	}

	b = (menubitmap_s*) self;
	UI_DrawPlayer( b->generic.x, b->generic.y, b->width, b->height, &s_controls.playerinfo, uis.realtime/2 );
}
#endif


/*
=================
Controls_GetKeyAssignment
=================
*/
static void Controls_GetKeyAssignment (char *command, int *twokeys)
{
	int		key;
	int		i;

	twokeys[0] = twokeys[1] = -1;
	key = 0;

	for ( i = 0; i < 2; i++ )
	{
		key = trap_Key_GetKey( command, key );
		if ( key == -1 ) {
			break;
		}

		twokeys[i] = key;
		key++;
	}
}

/*
=================
Controls_GetConfig
=================
*/
static void Controls_GetConfig( void )
{
	int		i;
	int		j;
	int		twokeys[2];
	bind_t*	bindptr;

	// put the bindings into a local store
	bindptr = g_bindings;

	// iterate each command, get its numeric binding
	for (i=0; ;i++,bindptr++)
	{
		if (!bindptr->label)
			break;

		for (j = 0; j < UI_MaxSplitView(); j++) {
			Controls_GetKeyAssignment(Com_LocalPlayerCvarName(j, bindptr->command), twokeys);

			g_bindings_list[j][i].bind1 = twokeys[0];
			g_bindings_list[j][i].bind2 = twokeys[1];
		}
	}

	if (s_controls.localPlayerNum == 0) {
		s_controls.invertmouse.curvalue  = Controls_GetCvarValue( "m_pitch" ) < 0;
		s_controls.smoothmouse.curvalue  = Com_Clamp( 0, 1, Controls_GetCvarValue( "m_filter" ) );
		s_controls.sensitivity.curvalue  = Com_Clamp( 2, 30, Controls_GetCvarValue( "sensitivity" ) );
		s_controls.freelook.curvalue     = Com_Clamp( 0, 1, Controls_GetCvarValue( "cl_freelook" ) );
	}

#ifndef TURTLEARENA // ALWAYS_RUN
	s_controls.alwaysrun.curvalue = Com_Clamp( 0, 1, Controls_GetCvarValue( Com_LocalPlayerCvarName(s_controls.localPlayerNum, "cl_run" ) ) );
#endif
#ifndef TA_WEAPSYS_EX
	s_controls.cyclepastgauntlet.curvalue = Com_Clamp( 0, 1, Controls_GetCvarValue( Com_LocalPlayerCvarName(s_controls.localPlayerNum, "cg_cyclePastGauntlet" ) ) );
	s_controls.autoswitch.curvalue = Com_Clamp( 0, 1, Controls_GetCvarValue( Com_LocalPlayerCvarName(s_controls.localPlayerNum, "cg_autoswitch" ) ) );
#endif
	s_controls.joythreshold.curvalue = Com_Clamp( 0.05f, 0.75f, Controls_GetCvarValue( Com_LocalPlayerCvarName(s_controls.localPlayerNum, "in_joystickThreshold" ) ) );
	s_controls.joyanalog.curvalue = Com_Clamp( 0, 1, Controls_GetCvarValue( Com_LocalPlayerCvarName(s_controls.localPlayerNum, "in_joystickUseAnalog" ) ) );
}

/*
=================
Controls_SetConfig
=================
*/
static void Controls_SetConfig( void )
{
	int		i;
	int		j;
	bind_t*	bindptr;

	// set the bindings from the local store
	bindptr = g_bindings;

	// iterate each command, get its numeric binding
	for (i=0; ;i++,bindptr++)
	{
		if (!bindptr->label)
			break;

		for (j = 0; j < UI_MaxSplitView(); j++) {
			if (g_bindings_list[j][i].bind1 != -1)
			{
				trap_Key_SetBinding( g_bindings_list[j][i].bind1, Com_LocalPlayerCvarName(j, bindptr->command) );

				if (g_bindings_list[j][i].bind2 != -1)
					trap_Key_SetBinding( g_bindings_list[j][i].bind2, Com_LocalPlayerCvarName(j, bindptr->command) );
			}
		}
	}

	if (s_controls.localPlayerNum == 0) {
		if ( s_controls.invertmouse.curvalue )
			trap_Cvar_SetValue( "m_pitch", -fabs( trap_Cvar_VariableValue( "m_pitch" ) ) );
		else
			trap_Cvar_SetValue( "m_pitch", fabs( trap_Cvar_VariableValue( "m_pitch" ) ) );

		trap_Cvar_SetValue( "m_filter", s_controls.smoothmouse.curvalue );
		trap_Cvar_SetValue( "sensitivity", s_controls.sensitivity.curvalue );
		trap_Cvar_SetValue( "cl_freelook", s_controls.freelook.curvalue );
	}

#ifndef TURTLEARENA // ALWAYS_RUN
	trap_Cvar_SetValue( Com_LocalPlayerCvarName( s_controls.localPlayerNum, "cl_run" ), s_controls.alwaysrun.curvalue );
#endif
#ifndef TA_WEAPSYS_EX
	trap_Cvar_SetValue( Com_LocalPlayerCvarName( s_controls.localPlayerNum, "cg_cyclePastGauntlet" ), s_controls.cyclepastgauntlet.curvalue );
	trap_Cvar_SetValue( Com_LocalPlayerCvarName( s_controls.localPlayerNum, "cg_autoswitch" ), s_controls.autoswitch.curvalue );
#endif
	trap_Cvar_SetValue( Com_LocalPlayerCvarName( s_controls.localPlayerNum, "in_joystickThreshold" ), s_controls.joythreshold.curvalue );
	trap_Cvar_SetValue( Com_LocalPlayerCvarName( s_controls.localPlayerNum, "in_joystickUseAnalog" ), s_controls.joyanalog.curvalue );
}

/*
=================
Controls_SetDefaults
=================
*/
static void Controls_SetDefaults( void )
{
	int	i;
	bind_t*	bindptr;

	// set the bindings from the local store
	bindptr = s_controls.bindings;

	// iterate each command, set its default binding
	for (i=0; ;i++,bindptr++)
	{
		if (!g_bindings[i].label)
			break;

#ifdef IOQ3ZTM
		if ( bindptr->defaultbind1 == -1 && bindptr->bind1 != -1 ) {
			trap_Key_SetBinding( bindptr->bind1, "" );
			bindptr->bind1 = -1;
		}
		if ( bindptr->defaultbind2 == -1 && bindptr->bind2 != -1 ) {
			trap_Key_SetBinding( bindptr->bind2, "" );
			bindptr->bind2 = -1;
		}
#endif
		bindptr->bind1 = bindptr->defaultbind1;
		bindptr->bind2 = bindptr->defaultbind2;
	}

	if (s_controls.localPlayerNum == 0) {
		s_controls.invertmouse.curvalue  = Controls_GetCvarDefault( "m_pitch" ) < 0;
		s_controls.smoothmouse.curvalue  = Controls_GetCvarDefault( "m_filter" );
		s_controls.sensitivity.curvalue  = Controls_GetCvarDefault( "sensitivity" );
		s_controls.freelook.curvalue     = Controls_GetCvarDefault( "cl_freelook" );
	}

#ifndef TURTLEARENA // ALWAYS_RUN
	s_controls.alwaysrun.curvalue = Controls_GetCvarDefault( Com_LocalPlayerCvarName(s_controls.localPlayerNum, "cl_run" ) );
#endif
#ifndef TA_WEAPSYS_EX
	s_controls.cyclepastgauntlet.curvalue = Controls_GetCvarDefault( Com_LocalPlayerCvarName(s_controls.localPlayerNum, "cg_cyclePastGauntlet" ) );
	s_controls.autoswitch.curvalue = Controls_GetCvarDefault( Com_LocalPlayerCvarName(s_controls.localPlayerNum, "cg_autoswitch" ) );
#endif
	trap_Cvar_SetValue(Com_LocalPlayerCvarName(s_controls.localPlayerNum, "in_joystick"), 0);
	trap_Cvar_SetValue(Com_LocalPlayerCvarName(s_controls.localPlayerNum, "in_joystickNo"), 0);
	s_controls.joythreshold.curvalue = Controls_GetCvarDefault( Com_LocalPlayerCvarName(s_controls.localPlayerNum, "in_joystickThreshold" ) );
	s_controls.joyanalog.curvalue    = Controls_GetCvarDefault( Com_LocalPlayerCvarName(s_controls.localPlayerNum, "in_joystickUseAnalog" ) );
}

/*
=================
Controls_MenuKey
=================
*/
static sfxHandle_t Controls_MenuKey( int key )
{
	int			id;
	int			i;
	int			startJoyKey;
	qboolean	found;
	bind_t*		bindptr;
	found = qfalse;

	if (!waitingforkey)
	{
		switch (key)
		{
			case K_BACKSPACE:
			case K_DEL:
			case K_KP_DEL:
				key = -1;
				break;
		
#ifndef TA_MISC // MENU: Right Mouse button = left arrow
			case K_MOUSE2:
#endif
			case K_ESCAPE:
				if (s_controls.changesmade)
					Controls_SetConfig();
				goto ignorekey;	

			default:
				goto ignorekey;
		}
	}
	else
	{
		if (key & K_CHAR_FLAG)
			goto ignorekey;

		switch (key)
		{
			case K_ESCAPE:
				waitingforkey = qfalse;
				Controls_Update();
				return (menu_out_sound);
	
			case '`':
				goto ignorekey;
		}

		// Bind control to joystick for the active player instead of whatever
		// joystick the key was pressed on. This seems more intuitive since
		// the user might not know which is the correct joystick and it's unlikely
		// that a user wants binds for multiple players on a single joystick.
		switch ( s_controls.localPlayerNum ) {
			default:
			case 0:
				startJoyKey = K_FIRST_JOY;
				break;
			case 1:
				startJoyKey = K_FIRST_2JOY;
				break;
			case 2:
				startJoyKey = K_FIRST_3JOY;
				break;
			case 3:
				startJoyKey = K_FIRST_4JOY;
				break;
		}

		if ( key >= K_FIRST_JOY && key <= K_LAST_JOY ) {
			key = startJoyKey + key - K_FIRST_JOY;
		}
		else if ( key >= K_FIRST_2JOY && key <= K_LAST_2JOY ) {
			key = startJoyKey + key - K_FIRST_2JOY;
		}
		else if ( key >= K_FIRST_3JOY && key <= K_LAST_3JOY ) {
			key = startJoyKey + key - K_FIRST_3JOY;
		}
		else if ( key >= K_FIRST_4JOY && key <= K_LAST_4JOY ) {
			key = startJoyKey + key - K_FIRST_4JOY;
		}
	}

	s_controls.changesmade = qtrue;
	
	if (key != -1)
	{
		// remove from any other bind
		int j;

		for (j = 0; j < UI_MaxSplitView(); j++) {
			bindptr = g_bindings_list[j];

			for (i=0; ;i++,bindptr++)
			{
				if (!g_bindings[i].label)
					break;

				if (bindptr->bind2 == key)
					bindptr->bind2 = -1;

				if (bindptr->bind1 == key)
				{
					bindptr->bind1 = bindptr->bind2;	
					bindptr->bind2 = -1;
				}
			}
		}
	}

	// assign key to local store
	id      = ((menucommon_s*)(s_controls.menu.items[s_controls.menu.cursor]))->id;
	bindptr = s_controls.bindings;
	for (i=0; ;i++,bindptr++)
	{
		if (!g_bindings[i].label)
			break;
		
		if (bindptr->id == id)
		{
			found = qtrue;
			if (key == -1)
			{
				if( bindptr->bind1 != -1 ) {
					trap_Key_SetBinding( bindptr->bind1, "" );
					bindptr->bind1 = -1;
				}
				if( bindptr->bind2 != -1 ) {
					trap_Key_SetBinding( bindptr->bind2, "" );
					bindptr->bind2 = -1;
				}
			}
			else if (bindptr->bind1 == -1) {
				bindptr->bind1 = key;
			}
			else if (bindptr->bind1 != key && bindptr->bind2 == -1) {
				bindptr->bind2 = key;
			}
			else
			{
				trap_Key_SetBinding( bindptr->bind1, "" );
				trap_Key_SetBinding( bindptr->bind2, "" );
				bindptr->bind1 = key;
				bindptr->bind2 = -1;
			}						
			break;
		}
	}				
		
	waitingforkey = qfalse;

	if (found)
	{	
		Controls_Update();
		return (menu_out_sound);
	}

ignorekey:
	return Menu_DefaultKey( &s_controls.menu, key );
}

/*
=================
Controls_ResetDefaults_Action
=================
*/
static void Controls_ResetDefaults_Action( qboolean result ) {
	if( !result ) {
		return;
	}

	s_controls.changesmade = qtrue;
	Controls_SetDefaults();
	Controls_Update();
}

/*
=================
Controls_ResetDefaults_Draw
=================
*/
static void Controls_ResetDefaults_Draw( void ) {
	UI_DrawProportionalString( SCREEN_WIDTH/2, 356 + PROP_HEIGHT * 0, "WARNING: This will reset all", UI_CENTER|UI_SMALLFONT, color_yellow );
	UI_DrawProportionalString( SCREEN_WIDTH/2, 356 + PROP_HEIGHT * 1, "controls to their default values.", UI_CENTER|UI_SMALLFONT, color_yellow );
}

/*
=================
Controls_MenuEvent
=================
*/
static void Controls_MenuEvent( void* ptr, int event )
{
	switch (((menucommon_s*)ptr)->id)
	{
		case ID_MOVEMENT:
			if (event == QM_ACTIVATED)
			{
				s_controls.section = C_MOVEMENT; 
				Controls_Update();
			}
			break;

		case ID_LOOKING:
			if (event == QM_ACTIVATED)
			{
				s_controls.section = C_LOOKING; 
				Controls_Update();
			}
			break;

		case ID_WEAPONS:
			if (event == QM_ACTIVATED)
			{
				s_controls.section = C_WEAPONS; 
				Controls_Update();
			}
			break;

		case ID_MISC:
			if (event == QM_ACTIVATED)
			{
				s_controls.section = C_MISC; 
				Controls_Update();
			}
			break;


#ifdef IOQ3ZTM // NEW_CAM
		case ID_CAMERA:
			if (event == QM_ACTIVATED)
			{
				s_controls.section = C_CAMERA; 
				Controls_Update();
			}
			break;
#endif

		case ID_JOYSTICK:
			if (event == QM_ACTIVATED)
			{
				s_controls.section = C_JOYSTICK; 
				Controls_Update();
			}
			break;

		case ID_DEFAULTS:
			if (event == QM_ACTIVATED)
			{
				UI_ConfirmMenu( "SET TO DEFAULTS?", Controls_ResetDefaults_Draw, Controls_ResetDefaults_Action );
			}
			break;

		case ID_BACK:
			if (event == QM_ACTIVATED)
			{
				if (s_controls.changesmade)
					Controls_SetConfig();
				UI_PopMenu();
			}
			break;

		case ID_SAVEANDEXIT:
			if (event == QM_ACTIVATED)
			{
				Controls_SetConfig();
				UI_PopMenu();
			}
			break;

		case ID_EXIT:
			if (event == QM_ACTIVATED)
			{
				UI_PopMenu();
			}
			break;

		case ID_FREELOOK:
		case ID_MOUSESPEED:
		case ID_INVERTMOUSE:
		case ID_SMOOTHMOUSE:
#ifndef TURTLEARENA // ALWAYS_RUN
		case ID_ALWAYSRUN:
#endif
#ifndef TA_WEAPSYS_EX
		case ID_CYCLEPASTGAUNTLET:
		case ID_AUTOSWITCH:
#endif
		case ID_JOYTHRESHOLD:
		case ID_JOYANALOG:
			if (event == QM_ACTIVATED)
			{
				s_controls.changesmade = qtrue;
			}
			break;		

		case ID_SELECTJOY:
			if (event == QM_ACTIVATED)
			{
				UI_JoystickMenu(s_controls.localPlayerNum);
			}
			break;
	}
}

/*
=================
Controls_ActionEvent
=================
*/
static void Controls_ActionEvent( void* ptr, int event )
{
#ifndef TA_MISC
	if (event == QM_LOSTFOCUS)
	{
		Controls_UpdateModel( ANIM_IDLE );
	}
	else if (event == QM_GOTFOCUS)
	{
		Controls_UpdateModel( g_bindings[((menucommon_s*)ptr)->id].anim );
	}
	else
#endif
	if ((event == QM_ACTIVATED) && !waitingforkey)
	{
		waitingforkey = qtrue;
		Controls_Update();
	}
}

#ifndef TA_MISC
/*
=================
Controls_InitModel
=================
*/
static void Controls_InitModel( void )
{
	char model[MAX_QPATH], headmodel[MAX_QPATH];

	memset( &s_controls.playerinfo, 0, sizeof(uiPlayerInfo_t) );

	trap_Cvar_VariableStringBuffer( Com_LocalPlayerCvarName(s_controls.localPlayerNum, "model"), model, sizeof ( model ) );
	trap_Cvar_VariableStringBuffer( Com_LocalPlayerCvarName(s_controls.localPlayerNum, "headmodel"), headmodel, sizeof ( headmodel ) );

	UI_PlayerInfo_SetModel( &s_controls.playerinfo, model, headmodel, NULL );

	Controls_UpdateModel( ANIM_IDLE );
}

/*
=================
Controls_InitWeapons
=================
*/
static void Controls_InitWeapons( void ) {
#ifndef TA_WEAPSYS_EX // Don't pre-load weapons in controls menu, there could be a lot that are not seen in menu...
	gitem_t *	item;
	int			i;

	for ( i = 1; i < BG_NumItems(); i++ ) {
		item = BG_ItemForItemNum( i );
		if ( item->giType != IT_WEAPON ) {
			continue;
		}
#ifdef TA_WEAPSYS
		// weap1
		if (item->world_model[2]) {
			trap_R_RegisterModel( item->world_model[2] );
		}
		else if (item->world_model[0]) {
			trap_R_RegisterModel( item->world_model[0] );
		}

		// weap2
		if (item->world_model[3]) {
			trap_R_RegisterModel( item->world_model[3] );
		}
		else {
			// uses weap1
		}
#else
		trap_R_RegisterModel( item->world_model[0] );
#endif
	}
#endif
}
#endif

/*
=================
Controls_MenuInit
=================
*/
static void Controls_MenuInit( int localPlayerNum )
{
#ifdef TA_MISC
	static char bannertext[32];
#else
	static char playername[32];
#endif
	int			y;

	// zero set all our globals
	memset( &s_controls, 0 ,sizeof(controls_t) );

	s_controls.localPlayerNum = localPlayerNum;

	if (s_controls.localPlayerNum == 0) {
		s_controls.controls = g_controls;
		s_controls.bindings = g_bindings;
	} else {
		s_controls.controls = g_mini_controls;

		if (s_controls.localPlayerNum == 1) {
			s_controls.bindings = g_bindings2;
		} else if (s_controls.localPlayerNum == 2) {
			s_controls.bindings = g_bindings3;
		} else {
			s_controls.bindings = g_bindings4;
		}
	}

	Controls_Cache();

#ifdef TA_MISC
	Com_sprintf(bannertext, sizeof (bannertext), "PLAYER %d CONTROLS", s_controls.localPlayerNum+1);
#endif

	s_controls.menu.key        = Controls_MenuKey;
	s_controls.menu.wrapAround = qtrue;
	s_controls.menu.fullscreen = qtrue;

	s_controls.banner.generic.type	= MTYPE_BTEXT;
	s_controls.banner.generic.flags	= QMF_CENTER_JUSTIFY;
	s_controls.banner.generic.x		= 320;
	s_controls.banner.generic.y		= 16;
#ifdef TA_MISC
	s_controls.banner.string		= bannertext;
#else
	s_controls.banner.string		= "CONTROLS";
#endif
	s_controls.banner.color			= text_banner_color;
	s_controls.banner.style			= UI_CENTER;

	s_controls.framel.generic.type  = MTYPE_BITMAP;
	s_controls.framel.generic.name  = ART_FRAMEL;
	s_controls.framel.generic.flags = QMF_LEFT_JUSTIFY|QMF_INACTIVE;
	s_controls.framel.generic.x     = 0;
	s_controls.framel.generic.y     = 78;
	s_controls.framel.width  	    = 256;
	s_controls.framel.height  	    = 329;

	s_controls.framer.generic.type  = MTYPE_BITMAP;
	s_controls.framer.generic.name  = ART_FRAMER;
	s_controls.framer.generic.flags = QMF_LEFT_JUSTIFY|QMF_INACTIVE;
	s_controls.framer.generic.x     = 376;
	s_controls.framer.generic.y     = 76;
	s_controls.framer.width  	    = 256;
	s_controls.framer.height  	    = 334;

#ifdef IOQ3ZTM // NEW_CAM // DEFAULTS
	y = 240 - 3.5f * PROP_HEIGHT;
#else
	y = 240 - 2 * PROP_HEIGHT;
#endif

	s_controls.looking.generic.type     = MTYPE_PTEXT;
	s_controls.looking.generic.flags    = QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_controls.looking.generic.id	    = ID_LOOKING;
	s_controls.looking.generic.callback	= Controls_MenuEvent;
	s_controls.looking.generic.x	    = 152;
	s_controls.looking.generic.y	    = y;
	s_controls.looking.string			= "LOOK";
	s_controls.looking.style			= UI_RIGHT;
	s_controls.looking.color			= text_big_color;

	y += PROP_HEIGHT;
	s_controls.movement.generic.type     = MTYPE_PTEXT;
	s_controls.movement.generic.flags    = QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_controls.movement.generic.id	     = ID_MOVEMENT;
	s_controls.movement.generic.callback = Controls_MenuEvent;
	s_controls.movement.generic.x	     = 152;
	s_controls.movement.generic.y	     = y;
	s_controls.movement.string			= "MOVE";
	s_controls.movement.style			= UI_RIGHT;
	s_controls.movement.color			= text_big_color;

	y += PROP_HEIGHT;
	s_controls.weapons.generic.type	    = MTYPE_PTEXT;
	s_controls.weapons.generic.flags    = QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_controls.weapons.generic.id	    = ID_WEAPONS;
	s_controls.weapons.generic.callback	= Controls_MenuEvent;
	s_controls.weapons.generic.x	    = 152;
	s_controls.weapons.generic.y	    = y;
#ifdef TA_MISC
	s_controls.weapons.string			= "ITEMS";
#else
	s_controls.weapons.string			= "SHOOT";
#endif
	s_controls.weapons.style			= UI_RIGHT;
	s_controls.weapons.color			= text_big_color;

	y += PROP_HEIGHT;
	s_controls.misc.generic.type	 = MTYPE_PTEXT;
	s_controls.misc.generic.flags    = QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_controls.misc.generic.id	     = ID_MISC;
	s_controls.misc.generic.callback = Controls_MenuEvent;
	s_controls.misc.generic.x		 = 152;
	s_controls.misc.generic.y		 = y;
	s_controls.misc.string			= "MISC";
	s_controls.misc.style			= UI_RIGHT;
	s_controls.misc.color			= text_big_color;

#ifdef IOQ3ZTM // NEW_CAM
	y += PROP_HEIGHT;
	s_controls.camera.generic.type		= MTYPE_PTEXT;
	s_controls.camera.generic.flags		= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_controls.camera.generic.id		= ID_CAMERA;
	s_controls.camera.generic.callback	= Controls_MenuEvent;
	s_controls.camera.generic.x			= 152;
	s_controls.camera.generic.y			= y;
	s_controls.camera.string			= "CAMERA";
	s_controls.camera.style				= UI_RIGHT;
	s_controls.camera.color				= text_big_color;
#endif

	y += PROP_HEIGHT;
	s_controls.joystick.generic.type	 = MTYPE_PTEXT;
	s_controls.joystick.generic.flags    = QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_controls.joystick.generic.id	     = ID_JOYSTICK;
	s_controls.joystick.generic.callback = Controls_MenuEvent;
	s_controls.joystick.generic.x		 = 152;
	s_controls.joystick.generic.y		 = y;
	s_controls.joystick.string			= "JOY";
	s_controls.joystick.style			= UI_RIGHT;
	s_controls.joystick.color			= text_big_color;

#ifdef IOQ3ZTM
	y += PROP_HEIGHT;
	s_controls.defaults.generic.type		= MTYPE_PTEXT;
	s_controls.defaults.generic.flags		= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_controls.defaults.generic.id			= ID_DEFAULTS;
	s_controls.defaults.generic.callback	= Controls_MenuEvent;
	s_controls.defaults.generic.x			= 152;
	s_controls.defaults.generic.y			= y;
	s_controls.defaults.string				= "RESET";
	s_controls.defaults.style				= UI_RIGHT;
	s_controls.defaults.color				= text_big_color;
#endif

	s_controls.back.generic.type	 = MTYPE_BITMAP;
	s_controls.back.generic.name     = ART_BACK0;
	s_controls.back.generic.flags    = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_controls.back.generic.x		 = 0;
	s_controls.back.generic.y		 = 480-64;
	s_controls.back.generic.id	     = ID_BACK;
	s_controls.back.generic.callback = Controls_MenuEvent;
	s_controls.back.width  		     = 128;
	s_controls.back.height  		 = 64;
	s_controls.back.focuspic         = ART_BACK1;

#ifndef TA_MISC
	s_controls.player.generic.type      = MTYPE_BITMAP;
	s_controls.player.generic.flags     = QMF_INACTIVE;
	s_controls.player.generic.ownerdraw = Controls_DrawPlayer;
	s_controls.player.generic.x	        = 400;
	s_controls.player.generic.y	        = -40;
	s_controls.player.width	            = 32*10;
	s_controls.player.height            = 56*10;
#endif

	s_controls.walkforward.generic.type	     = MTYPE_ACTION;
	s_controls.walkforward.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.walkforward.generic.callback  = Controls_ActionEvent;
	s_controls.walkforward.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.walkforward.generic.id 	     = ID_FORWARD;

	s_controls.backpedal.generic.type	   = MTYPE_ACTION;
	s_controls.backpedal.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.backpedal.generic.callback  = Controls_ActionEvent;
	s_controls.backpedal.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.backpedal.generic.id 	   = ID_BACKPEDAL;

	s_controls.stepleft.generic.type	  = MTYPE_ACTION;
	s_controls.stepleft.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.stepleft.generic.callback  = Controls_ActionEvent;
	s_controls.stepleft.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.stepleft.generic.id 		  = ID_MOVELEFT;

	s_controls.stepright.generic.type	   = MTYPE_ACTION;
	s_controls.stepright.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.stepright.generic.callback  = Controls_ActionEvent;
	s_controls.stepright.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.stepright.generic.id        = ID_MOVERIGHT;

	s_controls.moveup.generic.type	    = MTYPE_ACTION;
	s_controls.moveup.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.moveup.generic.callback  = Controls_ActionEvent;
	s_controls.moveup.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.moveup.generic.id        = ID_MOVEUP;

	s_controls.movedown.generic.type	  = MTYPE_ACTION;
	s_controls.movedown.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.movedown.generic.callback  = Controls_ActionEvent;
	s_controls.movedown.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.movedown.generic.id        = ID_MOVEDOWN;

	s_controls.turnleft.generic.type	  = MTYPE_ACTION;
	s_controls.turnleft.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.turnleft.generic.callback  = Controls_ActionEvent;
	s_controls.turnleft.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.turnleft.generic.id        = ID_LEFT;

	s_controls.turnright.generic.type	   = MTYPE_ACTION;
	s_controls.turnright.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.turnright.generic.callback  = Controls_ActionEvent;
	s_controls.turnright.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.turnright.generic.id        = ID_RIGHT;

	s_controls.sidestep.generic.type	  = MTYPE_ACTION;
	s_controls.sidestep.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.sidestep.generic.callback  = Controls_ActionEvent;
	s_controls.sidestep.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.sidestep.generic.id        = ID_STRAFE;

#ifndef TURTLEARENA // NO_SPEED_KEY
	s_controls.run.generic.type	     = MTYPE_ACTION;
	s_controls.run.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.run.generic.callback  = Controls_ActionEvent;
	s_controls.run.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.run.generic.id        = ID_SPEED;
#endif

#ifndef TA_WEAPSYS_EX
	s_controls.gauntlet.generic.type	  = MTYPE_ACTION;
	s_controls.gauntlet.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.gauntlet.generic.callback  = Controls_ActionEvent;
	s_controls.gauntlet.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.gauntlet.generic.id        = ID_WEAPON1;

	s_controls.machinegun.generic.type	    = MTYPE_ACTION;
	s_controls.machinegun.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.machinegun.generic.callback  = Controls_ActionEvent;
	s_controls.machinegun.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.machinegun.generic.id        = ID_WEAPON2;

	s_controls.shotgun.generic.type	     = MTYPE_ACTION;
	s_controls.shotgun.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.shotgun.generic.callback  = Controls_ActionEvent;
	s_controls.shotgun.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.shotgun.generic.id        = ID_WEAPON3;

	s_controls.grenadelauncher.generic.type	     = MTYPE_ACTION;
	s_controls.grenadelauncher.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.grenadelauncher.generic.callback  = Controls_ActionEvent;
	s_controls.grenadelauncher.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.grenadelauncher.generic.id        = ID_WEAPON4;

	s_controls.rocketlauncher.generic.type	    = MTYPE_ACTION;
	s_controls.rocketlauncher.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.rocketlauncher.generic.callback  = Controls_ActionEvent;
	s_controls.rocketlauncher.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.rocketlauncher.generic.id        = ID_WEAPON5;

	s_controls.lightning.generic.type	   = MTYPE_ACTION;
	s_controls.lightning.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.lightning.generic.callback  = Controls_ActionEvent;
	s_controls.lightning.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.lightning.generic.id        = ID_WEAPON6;

	s_controls.railgun.generic.type	     = MTYPE_ACTION;
	s_controls.railgun.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.railgun.generic.callback  = Controls_ActionEvent;
	s_controls.railgun.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.railgun.generic.id        = ID_WEAPON7;

	s_controls.plasma.generic.type	    = MTYPE_ACTION;
	s_controls.plasma.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.plasma.generic.callback  = Controls_ActionEvent;
	s_controls.plasma.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.plasma.generic.id        = ID_WEAPON8;

	s_controls.bfg.generic.type	     = MTYPE_ACTION;
	s_controls.bfg.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.bfg.generic.callback  = Controls_ActionEvent;
	s_controls.bfg.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.bfg.generic.id        = ID_WEAPON9;

#ifdef MISSIONPACK
	s_controls.nailgun.generic.type	     = MTYPE_ACTION;
	s_controls.nailgun.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.nailgun.generic.callback  = Controls_ActionEvent;
	s_controls.nailgun.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.nailgun.generic.id        = ID_WEAPON11;

	s_controls.proxylauncher.generic.type	   = MTYPE_ACTION;
	s_controls.proxylauncher.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.proxylauncher.generic.callback  = Controls_ActionEvent;
	s_controls.proxylauncher.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.proxylauncher.generic.id        = ID_WEAPON12;

	s_controls.chaingun.generic.type	  = MTYPE_ACTION;
	s_controls.chaingun.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.chaingun.generic.callback  = Controls_ActionEvent;
	s_controls.chaingun.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.chaingun.generic.id        = ID_WEAPON13;
#endif
#endif

	s_controls.attack.generic.type	    = MTYPE_ACTION;
	s_controls.attack.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.attack.generic.callback  = Controls_ActionEvent;
	s_controls.attack.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.attack.generic.id        = ID_ATTACK;

#ifdef TA_WEAPSYS_EX
	s_controls.dropweapon.generic.type	    = MTYPE_ACTION;
	s_controls.dropweapon.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.dropweapon.generic.callback  = Controls_ActionEvent;
	s_controls.dropweapon.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.dropweapon.generic.id        = ID_WEAPDROP;
#else
	s_controls.prevweapon.generic.type	    = MTYPE_ACTION;
	s_controls.prevweapon.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.prevweapon.generic.callback  = Controls_ActionEvent;
	s_controls.prevweapon.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.prevweapon.generic.id        = ID_WEAPPREV;

	s_controls.nextweapon.generic.type	    = MTYPE_ACTION;
	s_controls.nextweapon.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.nextweapon.generic.callback  = Controls_ActionEvent;
	s_controls.nextweapon.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.nextweapon.generic.id        = ID_WEAPNEXT;
#endif

	s_controls.lookup.generic.type	    = MTYPE_ACTION;
	s_controls.lookup.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.lookup.generic.callback  = Controls_ActionEvent;
	s_controls.lookup.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.lookup.generic.id        = ID_LOOKUP;

	s_controls.lookdown.generic.type	  = MTYPE_ACTION;
	s_controls.lookdown.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.lookdown.generic.callback  = Controls_ActionEvent;
	s_controls.lookdown.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.lookdown.generic.id        = ID_LOOKDOWN;

	s_controls.mouselook.generic.type	   = MTYPE_ACTION;
	s_controls.mouselook.generic.flags     = QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.mouselook.generic.callback  = Controls_ActionEvent;
	s_controls.mouselook.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.mouselook.generic.id        = ID_MOUSELOOK;

	s_controls.freelook.generic.type		= MTYPE_RADIOBUTTON;
	s_controls.freelook.generic.flags		= QMF_SMALLFONT;
	s_controls.freelook.generic.x			= SCREEN_WIDTH/2;
	s_controls.freelook.generic.name		= "free look";
	s_controls.freelook.generic.id			= ID_FREELOOK;
	s_controls.freelook.generic.callback	= Controls_MenuEvent;
	s_controls.freelook.generic.statusbar	= Controls_StatusBar;

	s_controls.centerview.generic.type	    = MTYPE_ACTION;
	s_controls.centerview.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.centerview.generic.callback  = Controls_ActionEvent;
	s_controls.centerview.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.centerview.generic.id        = ID_CENTERVIEW;

#ifdef TURTLEARENA // LOCKON
	s_controls.lockon.generic.type	  = MTYPE_ACTION;
	s_controls.lockon.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.lockon.generic.callback  = Controls_ActionEvent;
	s_controls.lockon.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.lockon.generic.id        = ID_LOCKON;
#endif

#ifndef TURTLEARENA // NOZOOM
	s_controls.zoomview.generic.type	  = MTYPE_ACTION;
	s_controls.zoomview.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.zoomview.generic.callback  = Controls_ActionEvent;
	s_controls.zoomview.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.zoomview.generic.id        = ID_ZOOMVIEW;
#endif

#ifdef TA_MISC // DROP_FLAG
	s_controls.dropflag.generic.type		= MTYPE_ACTION;
	s_controls.dropflag.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.dropflag.generic.callback	= Controls_ActionEvent;
	s_controls.dropflag.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_controls.dropflag.generic.id			= ID_DROPFLAG;
#endif

	s_controls.useitem.generic.type	     = MTYPE_ACTION;
	s_controls.useitem.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.useitem.generic.callback  = Controls_ActionEvent;
	s_controls.useitem.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.useitem.generic.id        = ID_USEITEM;

#ifdef TA_HOLDSYS/*2*/
	s_controls.nextitem.generic.type	  = MTYPE_ACTION;
	s_controls.nextitem.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.nextitem.generic.callback  = Controls_ActionEvent;
	s_controls.nextitem.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.nextitem.generic.id        = ID_NEXTITEM;

	s_controls.previtem.generic.type	  = MTYPE_ACTION;
	s_controls.previtem.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.previtem.generic.callback  = Controls_ActionEvent;
	s_controls.previtem.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.previtem.generic.id        = ID_PREVITEM;
#endif

	s_controls.showscores.generic.type	    = MTYPE_ACTION;
	s_controls.showscores.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.showscores.generic.callback  = Controls_ActionEvent;
	s_controls.showscores.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.showscores.generic.id        = ID_SHOWSCORES;

#ifdef MISSIONPACK_HUD
	s_controls.scoresup.generic.type      = MTYPE_ACTION;
	s_controls.scoresup.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.scoresup.generic.callback  = Controls_ActionEvent;
	s_controls.scoresup.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.scoresup.generic.id        = ID_SCORESUP;

	s_controls.scoresdown.generic.type      = MTYPE_ACTION;
	s_controls.scoresdown.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.scoresdown.generic.callback  = Controls_ActionEvent;
	s_controls.scoresdown.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.scoresdown.generic.id        = ID_SCORESDOWN;
#endif

	s_controls.invertmouse.generic.type      = MTYPE_RADIOBUTTON;
	s_controls.invertmouse.generic.flags	 = QMF_SMALLFONT;
	s_controls.invertmouse.generic.x	     = SCREEN_WIDTH/2;
	s_controls.invertmouse.generic.name	     = "invert mouse";
	s_controls.invertmouse.generic.id        = ID_INVERTMOUSE;
	s_controls.invertmouse.generic.callback  = Controls_MenuEvent;
	s_controls.invertmouse.generic.statusbar = Controls_StatusBar;

	s_controls.smoothmouse.generic.type      = MTYPE_RADIOBUTTON;
	s_controls.smoothmouse.generic.flags	 = QMF_SMALLFONT;
	s_controls.smoothmouse.generic.x	     = SCREEN_WIDTH/2;
	s_controls.smoothmouse.generic.name	     = "smooth mouse";
	s_controls.smoothmouse.generic.id        = ID_SMOOTHMOUSE;
	s_controls.smoothmouse.generic.callback  = Controls_MenuEvent;
	s_controls.smoothmouse.generic.statusbar = Controls_StatusBar;

#ifndef TURTLEARENA // ALWAYS_RUN
	s_controls.alwaysrun.generic.type      = MTYPE_RADIOBUTTON;
	s_controls.alwaysrun.generic.flags	   = QMF_SMALLFONT;
	s_controls.alwaysrun.generic.x	       = SCREEN_WIDTH/2;
	s_controls.alwaysrun.generic.name	   = "always run";
	s_controls.alwaysrun.generic.id        = ID_ALWAYSRUN;
	s_controls.alwaysrun.generic.callback  = Controls_MenuEvent;
	s_controls.alwaysrun.generic.statusbar = Controls_StatusBar;
#endif

#ifndef TA_WEAPSYS_EX
	s_controls.cyclepastgauntlet.generic.type      = MTYPE_RADIOBUTTON;
	s_controls.cyclepastgauntlet.generic.flags     = QMF_SMALLFONT;
	s_controls.cyclepastgauntlet.generic.x         = SCREEN_WIDTH/2;
	s_controls.cyclepastgauntlet.generic.name      = "skip gauntlet";
	s_controls.cyclepastgauntlet.generic.id        = ID_CYCLEPASTGAUNTLET;
	s_controls.cyclepastgauntlet.generic.callback  = Controls_MenuEvent;
	s_controls.cyclepastgauntlet.generic.statusbar = Controls_StatusBar;

	s_controls.autoswitch.generic.type      = MTYPE_RADIOBUTTON;
	s_controls.autoswitch.generic.flags	    = QMF_SMALLFONT;
	s_controls.autoswitch.generic.x	        = SCREEN_WIDTH/2;
	s_controls.autoswitch.generic.name	    = "autoswitch weapons";
	s_controls.autoswitch.generic.id        = ID_AUTOSWITCH;
	s_controls.autoswitch.generic.callback  = Controls_MenuEvent;
	s_controls.autoswitch.generic.statusbar = Controls_StatusBar;
#endif

	s_controls.sensitivity.generic.type	     = MTYPE_SLIDER;
	s_controls.sensitivity.generic.x		 = SCREEN_WIDTH/2;
	s_controls.sensitivity.generic.flags	 = QMF_SMALLFONT;
	s_controls.sensitivity.generic.name	     = "mouse speed";
	s_controls.sensitivity.generic.id 	     = ID_MOUSESPEED;
	s_controls.sensitivity.generic.callback  = Controls_MenuEvent;
	s_controls.sensitivity.minvalue		     = 2;
	s_controls.sensitivity.maxvalue		     = 30;
	s_controls.sensitivity.generic.statusbar = Controls_StatusBar;

	s_controls.gesture.generic.type	     = MTYPE_ACTION;
	s_controls.gesture.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.gesture.generic.callback  = Controls_ActionEvent;
	s_controls.gesture.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.gesture.generic.id        = ID_GESTURE;

	s_controls.chat.generic.type	  = MTYPE_ACTION;
	s_controls.chat.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.chat.generic.callback  = Controls_ActionEvent;
	s_controls.chat.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.chat.generic.id        = ID_CHAT;

	s_controls.chat2.generic.type	   = MTYPE_ACTION;
	s_controls.chat2.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.chat2.generic.callback  = Controls_ActionEvent;
	s_controls.chat2.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.chat2.generic.id        = ID_CHAT2;

	s_controls.chat3.generic.type	   = MTYPE_ACTION;
	s_controls.chat3.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.chat3.generic.callback  = Controls_ActionEvent;
	s_controls.chat3.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.chat3.generic.id        = ID_CHAT3;

	s_controls.chat4.generic.type	   = MTYPE_ACTION;
	s_controls.chat4.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.chat4.generic.callback  = Controls_ActionEvent;
	s_controls.chat4.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.chat4.generic.id        = ID_CHAT4;

	s_controls.togglemenu.generic.type		= MTYPE_ACTION;
	s_controls.togglemenu.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.togglemenu.generic.callback  = Controls_ActionEvent;
	s_controls.togglemenu.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.togglemenu.generic.id        = ID_TOGGLEMENU;

#ifdef IOQ3ZTM
	s_controls.voteYes.generic.type	     = MTYPE_ACTION;
	s_controls.voteYes.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.voteYes.generic.callback  = Controls_ActionEvent;
	s_controls.voteYes.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.voteYes.generic.id        = ID_VOTEYES;

	s_controls.voteNo.generic.type	    = MTYPE_ACTION;
	s_controls.voteNo.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.voteNo.generic.callback  = Controls_ActionEvent;
	s_controls.voteNo.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.voteNo.generic.id        = ID_VOTENO;

	s_controls.followPlayer.generic.type	  = MTYPE_ACTION;
	s_controls.followPlayer.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.followPlayer.generic.callback  = Controls_ActionEvent;
	s_controls.followPlayer.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.followPlayer.generic.id        = ID_FOLLOWPLAYER;

	s_controls.takeScreenshot.generic.type	    = MTYPE_ACTION;
	s_controls.takeScreenshot.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.takeScreenshot.generic.callback  = Controls_ActionEvent;
	s_controls.takeScreenshot.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.takeScreenshot.generic.id        = ID_TAKESCREENSHOT;
#endif

#ifdef IOQ3ZTM // NEW_CAM
	s_controls.camreset.generic.type	  = MTYPE_ACTION;
	s_controls.camreset.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.camreset.generic.callback  = Controls_ActionEvent;
	s_controls.camreset.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.camreset.generic.id        = ID_CAMRESET;

	s_controls.camtoggle.generic.type      = MTYPE_ACTION;
	s_controls.camtoggle.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.camtoggle.generic.callback  = Controls_ActionEvent;
	s_controls.camtoggle.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.camtoggle.generic.id        = ID_CAMTOGGLE;

	s_controls.camright.generic.type      = MTYPE_ACTION;
	s_controls.camright.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.camright.generic.callback  = Controls_ActionEvent;
	s_controls.camright.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.camright.generic.id        = ID_CAMRIGHT;

	s_controls.camleft.generic.type      = MTYPE_ACTION;
	s_controls.camleft.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.camleft.generic.callback  = Controls_ActionEvent;
	s_controls.camleft.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.camleft.generic.id        = ID_CAMLEFT;

	s_controls.camzoomin.generic.type      = MTYPE_ACTION;
	s_controls.camzoomin.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.camzoomin.generic.callback  = Controls_ActionEvent;
	s_controls.camzoomin.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.camzoomin.generic.id        = ID_CAMZOOMIN;

	s_controls.camzoomout.generic.type      = MTYPE_ACTION;
	s_controls.camzoomout.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.camzoomout.generic.callback  = Controls_ActionEvent;
	s_controls.camzoomout.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.camzoomout.generic.id        = ID_CAMZOOMOUT;
#endif

	s_controls.selectjoy.generic.type		= MTYPE_PTEXT;
	s_controls.selectjoy.generic.flags		= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_controls.selectjoy.generic.x			= SCREEN_WIDTH/2;
	s_controls.selectjoy.generic.id			= ID_SELECTJOY;
	s_controls.selectjoy.generic.callback	= Controls_MenuEvent;
	s_controls.selectjoy.generic.ownerdraw	= Controls_DrawSmallText;
	s_controls.selectjoy.string				= "select joystick...";
	s_controls.selectjoy.color				= text_color_normal;
	s_controls.selectjoy.style				= UI_RIGHT|UI_SMALLFONT;

	s_controls.joythreshold.generic.type	  = MTYPE_SLIDER;
	s_controls.joythreshold.generic.x		  = SCREEN_WIDTH/2;
	s_controls.joythreshold.generic.flags	  = QMF_SMALLFONT;
	s_controls.joythreshold.generic.name	  = "joystick threshold";
	s_controls.joythreshold.generic.id 	      = ID_JOYTHRESHOLD;
	s_controls.joythreshold.generic.callback  = Controls_MenuEvent;
	s_controls.joythreshold.minvalue		  = 0.05f;
	s_controls.joythreshold.maxvalue		  = 0.75f;
	s_controls.joythreshold.generic.statusbar = Controls_StatusBar;

	s_controls.joyanalog.generic.type      = MTYPE_RADIOBUTTON;
	s_controls.joyanalog.generic.flags	   = QMF_SMALLFONT;
	s_controls.joyanalog.generic.x	       = SCREEN_WIDTH/2;
	s_controls.joyanalog.generic.name	   = "analog input";
	s_controls.joyanalog.generic.id        = ID_JOYANALOG;
	s_controls.joyanalog.generic.callback  = Controls_MenuEvent;
	s_controls.joyanalog.generic.statusbar = Controls_StatusBar;

#ifndef TA_MISC
	s_controls.name.generic.type	= MTYPE_PTEXT;
	s_controls.name.generic.flags	= QMF_CENTER_JUSTIFY|QMF_INACTIVE;
	s_controls.name.generic.x		= 320;
	s_controls.name.generic.y		= 440;
	s_controls.name.string			= playername;
	s_controls.name.style			= UI_CENTER;
	s_controls.name.color			= text_color_normal;
#endif

	Menu_AddItem( &s_controls.menu, &s_controls.banner );
	Menu_AddItem( &s_controls.menu, &s_controls.framel );
	Menu_AddItem( &s_controls.menu, &s_controls.framer );
#ifndef TA_MISC
	Menu_AddItem( &s_controls.menu, &s_controls.player );
	Menu_AddItem( &s_controls.menu, &s_controls.name );
#endif

	Menu_AddItem( &s_controls.menu, &s_controls.looking );
	Menu_AddItem( &s_controls.menu, &s_controls.movement );
	Menu_AddItem( &s_controls.menu, &s_controls.weapons );
	Menu_AddItem( &s_controls.menu, &s_controls.misc );
#ifdef IOQ3ZTM // NEW_CAM
	Menu_AddItem( &s_controls.menu, &s_controls.camera );
#endif
	Menu_AddItem( &s_controls.menu, &s_controls.joystick );
#ifdef IOQ3ZTM
	Menu_AddItem( &s_controls.menu, &s_controls.defaults );
#endif

	Menu_AddItem( &s_controls.menu, &s_controls.sensitivity );
	Menu_AddItem( &s_controls.menu, &s_controls.smoothmouse );
	Menu_AddItem( &s_controls.menu, &s_controls.invertmouse );
	Menu_AddItem( &s_controls.menu, &s_controls.lookup );
	Menu_AddItem( &s_controls.menu, &s_controls.lookdown );
	Menu_AddItem( &s_controls.menu, &s_controls.mouselook );
	Menu_AddItem( &s_controls.menu, &s_controls.freelook );
	Menu_AddItem( &s_controls.menu, &s_controls.centerview );
#ifdef TURTLEARENA // LOCKON
	Menu_AddItem( &s_controls.menu, &s_controls.lockon );
#endif
#ifndef TURTLEARENA // NOZOOM
	Menu_AddItem( &s_controls.menu, &s_controls.zoomview );
#endif
	Menu_AddItem( &s_controls.menu, &s_controls.selectjoy );
	Menu_AddItem( &s_controls.menu, &s_controls.joythreshold );
	Menu_AddItem( &s_controls.menu, &s_controls.joyanalog );

#ifndef TURTLEARENA // ALWAYS_RUN
	Menu_AddItem( &s_controls.menu, &s_controls.alwaysrun );
#endif
#ifndef TURTLEARENA // NO_SPEED_KEY
	Menu_AddItem( &s_controls.menu, &s_controls.run );
#endif
	Menu_AddItem( &s_controls.menu, &s_controls.walkforward );
	Menu_AddItem( &s_controls.menu, &s_controls.backpedal );
	Menu_AddItem( &s_controls.menu, &s_controls.stepleft );
	Menu_AddItem( &s_controls.menu, &s_controls.stepright );
	Menu_AddItem( &s_controls.menu, &s_controls.moveup );
	Menu_AddItem( &s_controls.menu, &s_controls.movedown );
	Menu_AddItem( &s_controls.menu, &s_controls.turnleft );
	Menu_AddItem( &s_controls.menu, &s_controls.turnright );
	Menu_AddItem( &s_controls.menu, &s_controls.sidestep );

	Menu_AddItem( &s_controls.menu, &s_controls.attack );
#ifdef TA_WEAPSYS_EX
	Menu_AddItem( &s_controls.menu, &s_controls.dropweapon );
#else
	Menu_AddItem( &s_controls.menu, &s_controls.nextweapon );
	Menu_AddItem( &s_controls.menu, &s_controls.prevweapon );
	Menu_AddItem( &s_controls.menu, &s_controls.cyclepastgauntlet );
	Menu_AddItem( &s_controls.menu, &s_controls.autoswitch );
	Menu_AddItem( &s_controls.menu, &s_controls.gauntlet );
	Menu_AddItem( &s_controls.menu, &s_controls.machinegun );
	Menu_AddItem( &s_controls.menu, &s_controls.shotgun );
	Menu_AddItem( &s_controls.menu, &s_controls.grenadelauncher );
	Menu_AddItem( &s_controls.menu, &s_controls.rocketlauncher );
	Menu_AddItem( &s_controls.menu, &s_controls.lightning );
	Menu_AddItem( &s_controls.menu, &s_controls.railgun );
	Menu_AddItem( &s_controls.menu, &s_controls.plasma );
	Menu_AddItem( &s_controls.menu, &s_controls.bfg );
#ifdef MISSIONPACK
	Menu_AddItem( &s_controls.menu, &s_controls.nailgun );
	Menu_AddItem( &s_controls.menu, &s_controls.proxylauncher );
	Menu_AddItem( &s_controls.menu, &s_controls.chaingun );
#endif
#endif

	Menu_AddItem( &s_controls.menu, &s_controls.showscores );
#ifdef MISSIONPACK_HUD
	Menu_AddItem( &s_controls.menu, &s_controls.scoresup );
	Menu_AddItem( &s_controls.menu, &s_controls.scoresdown );
#endif
#ifdef TA_MISC // DROP_FLAG
	Menu_AddItem( &s_controls.menu, &s_controls.dropflag );
#endif
	Menu_AddItem( &s_controls.menu, &s_controls.useitem );
#ifdef TA_HOLDSYS/*2*/
	Menu_AddItem( &s_controls.menu, &s_controls.nextitem );
	Menu_AddItem( &s_controls.menu, &s_controls.previtem );
#endif
	Menu_AddItem( &s_controls.menu, &s_controls.gesture );
	Menu_AddItem( &s_controls.menu, &s_controls.chat );
	Menu_AddItem( &s_controls.menu, &s_controls.chat2 );
	Menu_AddItem( &s_controls.menu, &s_controls.chat3 );
	Menu_AddItem( &s_controls.menu, &s_controls.chat4 );
	Menu_AddItem( &s_controls.menu, &s_controls.togglemenu );
#ifdef IOQ3ZTM
	Menu_AddItem( &s_controls.menu, &s_controls.voteYes );
	Menu_AddItem( &s_controls.menu, &s_controls.voteNo );
	Menu_AddItem( &s_controls.menu, &s_controls.followPlayer );
	Menu_AddItem( &s_controls.menu, &s_controls.takeScreenshot );
#endif
#ifdef IOQ3ZTM // NEW_CAM
	Menu_AddItem( &s_controls.menu, &s_controls.camreset );
	Menu_AddItem( &s_controls.menu, &s_controls.camtoggle );
	Menu_AddItem( &s_controls.menu, &s_controls.camright );
	Menu_AddItem( &s_controls.menu, &s_controls.camleft );
	Menu_AddItem( &s_controls.menu, &s_controls.camzoomin );
	Menu_AddItem( &s_controls.menu, &s_controls.camzoomout );
#endif

	Menu_AddItem( &s_controls.menu, &s_controls.back );

#ifndef TA_MISC
	trap_Cvar_VariableStringBuffer( Com_LocalPlayerCvarName(s_controls.localPlayerNum, "name"), s_controls.name.string, 16 );
	Q_CleanStr( s_controls.name.string );
#endif

	// initialize the current config
	Controls_GetConfig();

#ifndef TA_MISC
	// intialize the model
	Controls_InitModel();

	// intialize the weapons
	Controls_InitWeapons ();
#endif

	// initial default section
	s_controls.section = C_LOOKING;

	// update the ui
	Controls_Update();
}


/*
=================
Controls_WantsBindKeys
=================
*/
qboolean Controls_WantsBindKeys( void ) {
	return waitingforkey;
}

/*
=================
Controls_Cache
=================
*/
void Controls_Cache( void ) {
	trap_R_RegisterShaderNoMip( ART_BACK0 );
	trap_R_RegisterShaderNoMip( ART_BACK1 );
	trap_R_RegisterShaderNoMip( ART_FRAMEL );
	trap_R_RegisterShaderNoMip( ART_FRAMER );
}


/*
=================
UI_ControlsMenu
=================
*/
void UI_ControlsMenu( int localPlayerNum ) {
	Controls_MenuInit( localPlayerNum );
	UI_PushMenu( &s_controls.menu );
}

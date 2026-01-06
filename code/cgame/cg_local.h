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
#include "../qcommon/q_shared.h"
#include "../renderercommon/tr_types.h"
#include "../game/bg_public.h"
#include "../client/keycodes.h"
#include "cg_public.h"
#include "cg_syscalls.h"

// The entire cgame module is unloaded and reloaded on each level change,
// so there is NO persistant data between levels on the client side.
// If you absolutely need something stored, it can either be kept
// by the server in the server stored userinfos, or stashed in a cvar.

// all drawing is done to a 640*480 virtual screen size
// and will be automatically scaled to the real resolution
#define	SCREEN_WIDTH		640
#define	SCREEN_HEIGHT		480

#define TINYCHAR_WIDTH		8
#define TINYCHAR_HEIGHT		8

#define SMALLCHAR_WIDTH		8
#define SMALLCHAR_HEIGHT	16

#define BIGCHAR_WIDTH		16
#define BIGCHAR_HEIGHT		16

#define	GIANTCHAR_WIDTH		32
#define	GIANTCHAR_HEIGHT	48

#define	POWERUP_BLINKS		5

#define	POWERUP_BLINK_TIME	1000
#define	FADE_TIME			200
#define	PULSE_TIME			200
#define	DAMAGE_DEFLECT_TIME	100
#define	DAMAGE_RETURN_TIME	400
#define DAMAGE_TIME			500
#define	LAND_DEFLECT_TIME	150
#define	LAND_RETURN_TIME	300
#define	STEP_TIME			200
#define	DUCK_TIME			100
#define	PAIN_TWITCH_TIME	200
#define	WEAPON_SELECT_TIME	1400
#define	ITEM_SCALEUP_TIME	1000
#ifndef TURTLEARENA // NOZOOM
#define	ZOOM_TIME			150
#endif
#define	ITEM_BLOB_TIME		200
#define	MUZZLE_FLASH_TIME	20
#define	SINK_TIME			1000		// time for fragments to sink into ground before going away
#define	ATTACKER_HEAD_TIME	10000
#define	REWARD_TIME			3000

#define	PULSE_SCALE			1.5			// amount to scale up the icons when activating

#define	MAX_STEP_CHANGE		32

#define	MAX_VERTS_ON_POLY	10
#define	MAX_MARK_POLYS		256

#define STAT_MINUS			10	// num frame for '-' stats digit

#define	ICON_SIZE			48
#undef	CHAR_WIDTH				// ignore define from libc
#define	CHAR_WIDTH			32
#define	CHAR_HEIGHT			48
#define	TEXT_ICON_SPACE		4

#define	TEAMCHAT_WIDTH		80
#define TEAMCHAT_HEIGHT		8

#ifdef TA_DATA
#define	NUM_CROSSHAIRS		4
#else
#define	NUM_CROSSHAIRS		10
#endif

#define TEAM_OVERLAY_MAXNAME_WIDTH	12
#define TEAM_OVERLAY_MAXLOCATION_WIDTH	16

#ifndef TA_PLAYERSYS // Moved to bg_public.h
typedef enum {
	FOOTSTEP_NORMAL,
	FOOTSTEP_BOOT,
	FOOTSTEP_FLESH,
	FOOTSTEP_MECH,
	FOOTSTEP_ENERGY,
	FOOTSTEP_METAL,
	FOOTSTEP_SPLASH,

	FOOTSTEP_TOTAL
} footstep_t;
#endif

typedef enum {
	IMPACTSOUND_DEFAULT,
	IMPACTSOUND_METAL,
	IMPACTSOUND_FLESH
#ifdef TA_WEAPSYS
	,IMPACTSOUND_LIGHTNING_PREDICT
#endif
} impactSound_t;

// for CG_EventHandling
enum {
  CGAME_EVENT_NONE,
  CGAME_EVENT_TEAMMENU,
  CGAME_EVENT_SCOREBOARD,
  CGAME_EVENT_EDITHUD
};

#ifdef TA_MISC // MATERIALS
// Models Per Material type
#define NUM_MATERIAL_MODELS		5
#endif

//=================================================

// The menu code needs to get both key and char events, but
// to avoid duplicating the paths, the char events are just
// distinguished by or'ing in K_CHAR_FLAG (ugly)
#define	K_CHAR_FLAG		(1<<31)

#define	MAX_EDIT_LINE	256
typedef struct {
	int		cursor;
	int		scroll;					// display buffer offset
	int		widthInChars;			// display width
	int		buffer[MAX_EDIT_LINE];	// buffer holding Unicode code points
	int		len;					// length of buffer
	int		maxchars;				// max number of characters to insert in buffer
} mfield_t;

void	MField_Clear( mfield_t *edit );
void	MField_KeyDownEvent( mfield_t *edit, int key );
void	MField_CharEvent( mfield_t *edit, int ch );
void	MField_SetText( mfield_t *edit, const char *text );
const char *MField_Buffer( mfield_t *edit );
void	MField_Draw( mfield_t *edit, int x, int y, int style, const fontInfo_t *font, vec4_t color, qboolean drawCursor );

//=================================================

// player entities need to track more information
// than any other type of entity.

// note that not every ET_PLAYER entity is a player,
// because corpses after respawn are outside the normal
// player numbering range

#ifndef IOQ3ZTM // LERP_FRAME_CLIENT_LESS
// when changing animation, set animationTime to frameTime + lerping time
// The current lerp will finish out, then it will lerp to the new animation
typedef struct {
	int			oldFrame;
	int			oldFrameTime;		// time when ->oldFrame was exactly on

	int			frame;
	int			frameTime;			// time when ->frame will be exactly on

	float		backlerp;

	float		yawAngle;
	qboolean	yawing;
	float		pitchAngle;
	qboolean	pitching;

	int			animationNumber;	// may include ANIM_TOGGLEBIT
	animation_t	*animation;
	int			animationTime;		// time when the first frame of the animation will be exact
} lerpFrame_t;
#endif

#ifdef TA_WEAPSYS // MELEE_TRAIL
typedef struct
{
	float yawAngle;
	qboolean yawing;
} meleeTrail_t;

// Currently limited to one trail per-weapon
//   It would be nice for Sais to have three.
#define MAX_WEAPON_TRAILS MAX_HANDS
#endif

typedef struct {
	lerpFrame_t		legs, torso, flag;
#ifdef TA_WEAPSYS
	lerpFrame_t		barrel[MAX_HANDS];
#endif
	int				painTime;
	int				painDirection;	// flip from 0 to 1
	int				lightningFiring;

#ifdef TA_WEAPSYS // MELEE_TRAIL
	// melee weapon trails
	meleeTrail_t weaponTrails[MAX_WEAPON_TRAILS];
#endif

#ifndef TA_WEAPSYS
	int				railFireTime;
#endif

	// machinegun spinning
	float			barrelAngle;
	int				barrelTime;
	qboolean		barrelSpinning;

	// third person gun flash origin
#ifdef TA_WEAPSYS
	vec3_t			flashOrigin[MAX_HANDS];
#else
	vec3_t			flashOrigin;
#endif
} playerEntity_t;


// skin surfaces array shouldn't be dynamically allocated because players reuse the same skin structure when changing models
#define MAX_CG_SKIN_SURFACES 100
typedef struct {
	int numSurfaces;
	qhandle_t surfaces[MAX_CG_SKIN_SURFACES];
} cgSkin_t;

//=================================================


#ifdef TA_ENTSYS // MISC_OBJECT
// misc_object data
enum
{
	MOF_SETUP			= 1, // true if did one time setup.
};

typedef struct
{
	qhandle_t		model;
	cgSkin_t		skin;

	lerpFrame_t		lerp;
	int				anim; // current animation ( may have ANIM_TOGGLEBIT )
	float			speed; // Allow speeding up the animations?

	// Sounds
	int				lastSoundFrame;
	bg_sounds_t		sounds;

	int				flags; // Special flags.
} objectEntity_t;
#endif

// centity_t have a direct corespondence with gentity_t in the game, but
// only the entityState_t is directly communicated to the cgame
typedef struct centity_s {
	entityState_t	currentState;	// from cg.frame
	entityState_t	nextState;		// from cg.nextFrame, if available
	qboolean		interpolate;	// true if next is valid to interpolate to
	qboolean		currentValid;	// true if cg.frame holds this entity

	int				muzzleFlashTime;	// move to playerEntity?
	int				previousEvent;

	int				trailTime;		// so missile trails can handle dropped initial packets
	int				dustTrailTime;
	int				miscTime;

	int				snapShotTime;	// last time this entity was found in a snapshot

	playerEntity_t	pe;
#ifdef TA_ENTSYS // MISC_OBJECT
	bg_objectcfg_t	*objectcfg;
	objectEntity_t	oe; // misc_object data
#endif

	int				errorTime;		// decay the error from this time

	vec3_t			rawOrigin;
	vec3_t			rawAngles;

	// exact interpolated position of entity on this frame
	vec3_t			lerpOrigin;
	vec3_t			lerpAngles;

	// client side dlights
	int				dl_frame;
	int				dl_oldframe;
	float			dl_backlerp;
	int				dl_time;
	char			dl_stylestring[64];
	int				dl_sound;
	int				dl_atten;

} centity_t;


//======================================================================

// local entities are created as a result of events or predicted actions,
// and live independently from all server transmitted entities

typedef struct markPoly_s {
	struct markPoly_s	*prevMark, *nextMark;
	int			time;
	qhandle_t	markShader;
	qboolean	alphaFade;		// fade alpha instead of rgb
	float		color[4];
	int			numVerts;
	polyVert_t	verts[MAX_VERTS_ON_POLY];
	int			bmodelNum;
} markPoly_t;


typedef enum {
	LE_MARK,
	LE_EXPLOSION,
	LE_SPRITE_EXPLOSION,
	LE_FRAGMENT,
	LE_MOVE_SCALE_FADE,
	LE_FALL_SCALE_FADE,
	LE_FADE_RGB,
	LE_SCALE_FADE,
	LE_SCOREPLUM,
	LE_BUBBLE,
#ifdef TURTLEARENA // NIGHTS_ITEMS
	LE_CHAINPLUM,
#endif
#ifdef MISSIONPACK
#ifndef TURTLEARENA // NO_KAMIKAZE_ITEM POWERS
	LE_KAMIKAZE,
	LE_INVULIMPACT,
	LE_INVULJUICED,
#endif
	LE_SHOWREFENTITY
#endif
} leType_t;

typedef enum {
	LEF_PUFF_DONT_SCALE  = 0x0001,			// do not scale size over time
	LEF_TUMBLE			 = 0x0002,			// tumble over time, used for ejecting shells
#ifndef TURTLEARENA // NO_KAMIKAZE_ITEM
	LEF_SOUND1			 = 0x0004,			// sound 1 for kamikaze
	LEF_SOUND2			 = 0x0008			// sound 2 for kamikaze
#endif
} leFlag_t;

typedef enum {
	LEMT_NONE,
	LEMT_BURN
#ifndef NOTRATEDM // No gibs.
	,LEMT_BLOOD
#endif
} leMarkType_t;			// fragment local entities can leave marks on walls

typedef enum {
	LEBS_NONE,
#ifndef NOTRATEDM // No gibs.
	LEBS_BLOOD,
#endif
	LEBS_BRASS
} leBounceSoundType_t;	// fragment local entities can make sounds on impacts

typedef enum {
	LEVF_NO_DRAW				= 0x0001,	// do not draw for this player
	LEVF_FIRST_PERSON_MIRROR	= 0x0002,	// if player is first person, draw in mirrors only
	LEVF_FIRST_PERSON_NO_DRAW	= 0x0004,	// if player is first person, do not draw
	LEVF_THIRD_PERSON_NO_DRAW	= 0x0008	// if player is third person, do not draw
} leViewFlags_t;

typedef struct {
	int				playerNum;
	leViewFlags_t	viewFlags;
} lePlayerEfx_t;

#define MAX_PLAYER_EFX MAX_SPLITVIEW

typedef struct localEntity_s {
	struct localEntity_s	*prev, *next;
	leType_t		leType;
	int				leFlags;

	int				startTime;
	int				endTime;
	int				fadeInTime;

	float			lifeRate;			// 1.0 / (endTime - startTime)

	trajectory_t	pos;
	trajectory_t	angles;

	float			bounceFactor;		// 0.0 = no bounce, 1.0 = perfect

	float			color[4];

	float			radius;

	float			light;
	vec3_t			lightColor;

	leMarkType_t		leMarkType;		// mark to leave on fragment impact
	leBounceSoundType_t	leBounceSoundType;

	refEntity_t		refEntity;		

	int				groundEntityNum;

	int				defaultViewFlags; // view flags for players not listed in playerEffects
	lePlayerEfx_t	playerEffects[MAX_PLAYER_EFX];	// specific how to draw for specific players
	int				numPlayerEffects;
} localEntity_t;

//======================================================================


typedef struct {
	int				playerNum;
	int				score;
	int				ping;
	int				time;
	int				scoreFlags;
	int				accuracy;
#ifndef TURTLEARENA // AWARDS
	int				impressiveCount;
	int				excellentCount;
	int				guantletCount;
#endif
	int				defendCount;
	int				assistCount;
	int				captures;
	qboolean	perfect;
	int				team;
} score_t;

// each player has an associated playerInfo_t
// that contains media references necessary to present the
// player model and other color coded effects
// this is regenerated each time a player's configstring changes,
// usually as a result of a userinfo (name, model, etc) change
#define	MAX_CUSTOM_SOUNDS	32

#ifdef TA_WEAPSYS
// Support using "tag_weapon" for the primary weapon,
//           and "tag_flag" for the secondary weapon / flag.
// So it will allow team arena models to be ported easier?
//   and allow Turtle Arena players in Quake3/Team Arena?
enum
{
#ifdef TA_SUPPORTQ3
	TI_TAG_WEAPON = 1,
	TI_TAG_FLAG = 2,
#endif
	TI_TAG_HAND_PRIMARY = 4,
	TI_TAG_HAND_SECONDARY = 8,

	TI_TAG_WP_AWAY_PRIMARY = 16,
	TI_TAG_WP_AWAY_SECONDARY = 32,
};
#endif

#ifdef IOQ3ZTM // GHOST
#define NUM_GHOST_REFS 10
typedef struct
{
	int time;

	vec3_t		axis[3];			// rotation vectors
	qboolean	nonNormalizedAxes;	// axis are not normalized, i.e. they have scale
	int			frame;				// also used as MODEL_BEAM's diameter

	// previous data for frame interpolation
	int			oldframe;
	float		backlerp;			// 0.0 = current, 1.0 = old

	float		shaderTime;
	vec3_t		origin;

} ghostRefData_t;
#endif

typedef struct {
	qboolean		infoValid;

	char			name[MAX_QPATH];
	team_t			team;

	int				botSkill;		// 0 = not bot, 1-5 = bot

	vec3_t			color1;
	vec3_t			color2;
	
	byte c1RGBA[4];
	byte c2RGBA[4];

#ifdef TA_PLAYERSYS
	vec3_t			prefcolor2;
#endif

	int				score;			// updated by score servercmds
	int				location;		// location index for team mode
	int				health;			// you only get this info about your teammates
#ifndef TURTLEARENA // NOARMOR
	int				armor;
#endif
	int				curWeapon;

	int				handicap;
	int				wins, losses;	// in tourney mode

	int				teamTask;		// task in teamplay (offence/defence)
	qboolean		teamLeader;		// true when this is a team leader

	int				powerups;		// so can display quad/flag status

	int				medkitUsageTime;
#ifndef TURTLEARENA // POWERS
	int				invulnerabilityStartTime;
	int				invulnerabilityStopTime;
#endif
#ifdef IOQ3ZTM // GHOST
	int				ghostTime[MAX_HANDS];
	ghostRefData_t	ghostWeapon[MAX_HANDS][NUM_GHOST_REFS];
#endif

	int				breathPuffTime;

	// when playerinfo is changed, the loading of models/skins/sounds
	// can be deferred until you are dead, to prevent hitches in
	// gameplay
	char			modelName[MAX_QPATH];
	char			skinName[MAX_QPATH];
	char			headModelName[MAX_QPATH];
	char			headSkinName[MAX_QPATH];
#ifdef MISSIONPACK
	char			redTeam[MAX_TEAMNAME];
	char			blueTeam[MAX_TEAMNAME];
#endif
	qboolean		deferred;

#ifdef TA_WEAPSYS
	int				tagInfo;
#else
	qboolean		newAnims;		// true if using the new mission pack animations
#endif
#ifndef TA_PLAYERSYS
	qboolean		fixedlegs;		// true if legs yaw is always the same as torso yaw
	qboolean		fixedtorso;		// true if torso never changes yaw

	vec3_t			headOffset;		// move head in icon views
	footstep_t		footsteps;
	gender_t		gender;			// from model
#endif

	qhandle_t		legsModel;
	qhandle_t		torsoModel;
	qhandle_t		headModel;

	cgSkin_t		modelSkin;

	qhandle_t		modelIcon;

#ifdef TA_PLAYERSYS
	bg_playercfg_t  playercfg;
#else
	animation_t		animations[MAX_TOTALANIMATIONS];
#endif

	sfxHandle_t		sounds[MAX_CUSTOM_SOUNDS];
} playerInfo_t;


#ifdef TA_WEAPSYS
typedef struct projectileInfo_s {
	qboolean		registered;

	qhandle_t		missileModel;
	sfxHandle_t		missileSound;
	void			(*missileTrailFunc)( centity_t *, const struct projectileInfo_s *wi );
	float			missileDlight;
	vec3_t			missileDlightColor;

	qhandle_t		trailShader[2];
	float			trailRadius;
	float			wiTrailTime;

	//
	qhandle_t		missileModelBlue;
	qhandle_t		missileModelRed;
	qhandle_t		spriteShader;
	int				spriteRadius;


	// Hit mark and sounds
	qhandle_t		hitMarkShader;
	int				hitMarkRadius;
	sfxHandle_t		hitSound[3]; // Normal hit sounds, random select
	sfxHandle_t		hitPlayerSound;
	sfxHandle_t		hitMetalSound;

	// Impact mark and sounds
	qhandle_t		impactMarkShader;
	int				impactMarkRadius;
	sfxHandle_t		impactSound[3]; // Impact sounds, random select
	sfxHandle_t		impactPlayerSound;
	sfxHandle_t		impactMetalSound;

	// PE_PROX trigger sound
	sfxHandle_t		triggerSound;

} projectileInfo_t;

typedef struct weaponInfo_s {
	qboolean		registered;

	qhandle_t		weaponModel;
	qhandle_t		barrelModel;
	qhandle_t		flashModel;

	float			flashDlight;
	vec3_t			flashDlightColor;
	sfxHandle_t		flashSound[4];		// fast firing weapons randomly choose

	void			(*ejectBrassFunc)( centity_t * );

	// Impact mark and impact sounds for melee weapons
	qhandle_t		impactMarkShader;
	int				impactMarkRadius;
	sfxHandle_t		impactSound[3]; // Impact sounds, random select
	sfxHandle_t		impactPlayerSound;
	sfxHandle_t		impactMetalSound;

} weaponInfo_t;

// each WP_* weapon enum has an associated weaponGroupInfo_t
// that contains media references necessary to present the
// weapon and its effects
typedef struct weaponGroupInfo_s {
	qboolean		registered;

	qhandle_t		handsModel;			// the hands don't actually draw, they just position the weapon

	vec3_t			weaponMidpoint;		// so it will rotate centered instead of by tag

	qhandle_t		weaponIcon;
#ifdef TURTLEARENA // NOAMMO
	qhandle_t		weaponModel; // Pickup model, only used by new UI.
#else
	qhandle_t		ammoIcon;

	qhandle_t		ammoModel;
#endif

	// loopped sounds
	sfxHandle_t		readySound;
	sfxHandle_t		firingSound;

	// sounds played once
	sfxHandle_t		firingStoppedSound; // gun barrel stopped spining

} weaponGroupInfo_t;
#else
// each WP_* weapon enum has an associated weaponInfo_t
// that contains media references necessary to present the
// weapon and its effects
typedef struct weaponInfo_s {
	qboolean		registered;
	gitem_t			*item;

	qhandle_t		handsModel;			// the hands don't actually draw, they just position the weapon
	qhandle_t		weaponModel;
	qhandle_t		barrelModel;
	qhandle_t		flashModel;

	vec3_t			weaponMidpoint;		// so it will rotate centered instead of by tag

	float			flashDlight;
	vec3_t			flashDlightColor;
	sfxHandle_t		flashSound[4];		// fast firing weapons randomly choose

	qhandle_t		weaponIcon;
	qhandle_t		ammoIcon;

	qhandle_t		ammoModel;

	qhandle_t		missileModel;
	sfxHandle_t		missileSound;
	void			(*missileTrailFunc)( centity_t *, const struct weaponInfo_s *wi );
	float			missileDlight;
	vec3_t			missileDlightColor;

	void			(*ejectBrassFunc)( centity_t * );

	float			trailRadius;
	float			wiTrailTime;

	sfxHandle_t		readySound;
	sfxHandle_t		firingSound;
} weaponInfo_t;
#endif

// each IT_* item has an associated itemInfo_t
// that constains media references necessary to present the
// item and its effects
typedef struct {
	qboolean		registered;
	qhandle_t		models[MAX_ITEM_MODELS];
	qhandle_t		icon;
#ifdef IOQ3ZTM // FLAG_MODEL
	cgSkin_t		skin;
#endif
} itemInfo_t;


#ifdef MISSIONPACK_HARVESTER
#define MAX_SKULLTRAIL		10

typedef struct {
	vec3_t positions[MAX_SKULLTRAIL];
	int numpositions;
} skulltrail_t;
#endif


typedef enum {
	LEAD_NONE,

	LEAD_LOST,
	LEAD_TIED,
	LEAD_TAKEN,

	LEAD_IGNORE // a player played a reward sound, so no lead change sfx this frame.
} leadChange_t;


#define MAX_REWARDSTACK		10
#define MAX_SOUNDBUFFER		20

//======================================================================

typedef struct
{
  int time;
  int length;
} consoleLine_t;

#define MAX_CONSOLE_LINES 4
#define MAX_CONSOLE_TEXT  ( 256 * MAX_CONSOLE_LINES )

// all cg.stepTime, cg.duckTime, cg.landTime, etc are set to cg.time when the action
// occurs, and they will have visible effects for #define STEP_TIME or whatever msec after

#define MAX_PREDICTED_EVENTS	16
 
typedef struct {

	int			playerNum;

	// prediction state
	qboolean	hyperspace;				// true if prediction has hit a trigger_teleport
	playerState_t	predictedPlayerState;
	centity_t		predictedPlayerEntity;
	qboolean	validPPS;				// clear until the first call to CG_PredictPlayerState
	int			predictedErrorTime;
	vec3_t		predictedError;

	int			eventSequence;
	int			predictableEvents[MAX_PREDICTED_EVENTS];

	float		stepChange;				// for stair up smoothing
	int			stepTime;

	float		duckChange;				// for duck viewheight smoothing
	int			duckTime;

	float		landChange;				// for landing hard
	int			landTime;

	// set by joystick events
	int			joystickAxis[MAX_JOYSTICK_AXIS];
	byte		joystickHats[MAX_JOYSTICK_AXIS];

	// used for input state sent to server
#ifndef TA_WEAPSYS_EX
	int			weaponSelect;
#endif
#ifdef TA_HOLDSYS/*2*/
	int			holdableSelect;
#endif

	// the client maintains its own idea of view angles, which are
	// sent to the server each frame.  It is cleared to 0 upon entering each level.
	// the server sends a delta each frame which is added to the locally
	// tracked view angles to account for standing on rotating objects,
	// and teleport direction changes
	vec3_t		viewangles;


	// centerprinting
	int			centerPrintTime;
	float		centerPrintCharScale;
	int			centerPrintY;
	char		centerPrint[1024];

#ifdef TA_MISC // COMIC_ANNOUNCER
#define MAX_ANNOUNCEMENTS 8
	int			announcement;
	int			announcementTime[MAX_ANNOUNCEMENTS];
	char		announcementMessage[MAX_ANNOUNCEMENTS][128];
#endif

#ifndef TURTLEARENA // NO_AMMO_WARNINGS
	// low ammo warning state
	int			lowAmmoWarning;		// 1 = low, 2 = empty
#endif

	// crosshair player ID
	int			crosshairPlayerNum;
	int			crosshairPlayerTime;

	// powerup active flashing
	int			powerupActive;
	int			powerupTime;

	// attacking player
	int			attackerTime;

#ifdef MISSIONPACK
	// voice chat head
	int			voiceTime;
	int			currentVoicePlayerNum;

	// proxy mine warning
	int			proxTime;
#endif

	// orders
	int			currentOrder;
	qboolean	orderPending;
	int			orderTime;
	int			acceptOrderTime;
	int			acceptTask;
	int			acceptLeader;
	char		acceptVoice[MAX_NAME_LENGTH];

	// reward medals
	int			rewardStack;
	int			rewardTime;
	int			rewardCount[MAX_REWARDSTACK];
	qhandle_t	rewardShader[MAX_REWARDSTACK];
#ifdef TA_MISC // COMIC_ANNOUNCER
	int			rewardAnnoucement[MAX_REWARDSTACK];
#else
	qhandle_t	rewardSound[MAX_REWARDSTACK];
#endif

#ifdef TURTLEARENA // LOCKON
	// lockon key
	qboolean	lockedOn;
	int			lockonTime;
#endif

#ifndef TURTLEARENA // NOZOOM
	// zoom key
	qboolean	zoomed;
	int			zoomTime;
	float		zoomSensitivity;
#endif

	int			itemPickup;
	int			itemPickupTime;
	int			itemPickupBlendTime;	// the pulse around the crosshair is timed separately

#ifdef TURTLEARENA // NIGHTS_ITEMS
	int			scorePickupTime;
#endif

#ifndef TA_WEAPSYS_EX
	int			weaponSelectTime;
#endif
	int			weaponAnimation;
	int			weaponAnimationTime;
	int			weaponToggledFrom;

	// blend blobs
	float		damageTime;
	float		damageX, damageY, damageValue;

	// status bar head
	float		headEndPitch;
	float		headEndYaw;
	int			headEndTime;
	float		headStartPitch;
	float		headStartYaw;
	int			headStartTime;

	// view movement
	float		v_dmg_time;
	float		v_dmg_pitch;
	float		v_dmg_roll;

	vec3_t		kick_angles;	// weapon kicks
	vec3_t		kick_origin;

#ifdef IOQ3ZTM // NEW_CAM
	float camZoomDir;
	qboolean camZoomIn;
	qboolean camZoomOut;
	float camRotDir;
	qboolean camLeft;
	qboolean camRight;
	qboolean camReseting;
	float camDistance; // Distance from client to put camera
#endif

	qboolean	renderingThirdPerson;		// during deaths, chasecams, etc

	// first person gun flash origin
#ifdef TA_WEAPSYS
	vec3_t		flashOrigin[MAX_HANDS];
#else
	vec3_t		flashOrigin;
#endif

	//qboolean cameraMode;		// if rendering from a loaded camera

	// orbit camera around player
	float cameraOrbit;			// angles per second to orbit, forces third person.
	float cameraOrbitAngle;
	float cameraOrbitRange;

	vec3_t		lastViewPos;
	vec3_t		lastViewAngles;

#ifdef IOQ3ZTM // LETTERBOX
	// Use CG_ToggleLetterbox to change letterbox mode
	qboolean letterbox;	// qtrue if moving onto the screen, or is done moving on.
						// qfalse if moving off, or is off
	int		letterboxTime; // Time that the letter box move was started, or -1 if instant.
#endif

	// scoreboard
	qboolean	showScores;
	qboolean	scoreBoardShowing;
	int			scoreFadeTime;
	char		killerName[MAX_NAME_LENGTH];
	int			selectedScore;

#ifdef TURTLEARENA
	int			waterlevel;
	int			airBarFadeTime; // Air bar start fade time
	qboolean	airBarDrawn; // Drew air bar last frame
#endif

	// console
	char			consoleText[ MAX_CONSOLE_TEXT ];
	consoleLine_t	consoleLines[ MAX_CONSOLE_LINES ];
	int				numConsoleLines;

	// teamchat width is *3 because of embedded color codes
	char			teamChatMsgs[TEAMCHAT_HEIGHT][TEAMCHAT_WIDTH*3+1];
	int				teamChatMsgTimes[TEAMCHAT_HEIGHT];
	int				teamChatPos;
	int				teamLastChatPos;

} localPlayer_t;
 
#define MAX_SPAWN_VARS          64
#define MAX_SPAWN_VARS_CHARS    2048
 
typedef struct {
	connstate_t	connState;
	qboolean	connected;			// connected to a server
	
	int			clientFrame;		// incremented each frame

	int			cinematicPlaying;	// playing fullscreen cinematic
	int			cinematicHandle;	// handle for fullscreen cinematic
	qboolean	demoPlayback;
	qboolean	levelShot;			// taking a level menu screenshot
	int			deferredPlayerLoading;
	qboolean	loading;			// don't defer players at initial startup
	qboolean	intermissionStarted;	// don't play voice rewards, because game will end shortly

	// there are only one or two snapshot_t that are relevant at a time
	int			latestSnapshotNum;	// the number of snapshots the client system has received
	int			latestSnapshotTime;	// the time from latestSnapshotNum, so we don't need to read the snapshot yet

	snapshot_t	*snap;				// cg.snap->serverTime <= cg.time
	snapshot_t	*nextSnap;			// cg.nextSnap->serverTime > cg.time, or NULL
	snapshot_t	activeSnapshots[2];

	float		frameInterpolation;	// (float)( cg.time - cg.frame->serverTime ) / (cg.nextFrame->serverTime - cg.frame->serverTime)

	qboolean	thisFrameTeleport;
	qboolean	nextFrameTeleport;

	int			realTime;
	int			realFrameTime;

	int			frametime;		// cg.time - cg.oldTime

	int			time;			// this is the server time value that the client
								// is rendering at.
	int			oldTime;		// time at last frame, used for missile trails and prediction checking

	int			physicsTime;	// either cg.snap->time or cg.nextSnap->time

	leadChange_t bestLeadChange;
	int			timelimitWarnings;	// 5 min, 1 min, overtime
	int			fraglimitWarnings;

	qboolean	mapRestart;			// set on a map restart to set back the weapon

	// auto rotating items
	vec3_t		autoAngles;
	vec3_t		autoAxis[3];
	vec3_t		autoAnglesFast;
	vec3_t		autoAxisFast[3];

	// view rendering
	refdef_t	refdef;
	vec3_t		refdefViewAngles;		// will be converted to refdef.viewaxis
	float		viewWeaponFov;			// either range checked cg_weaponFov or forced value

	// first person view pos, set even when rendering third person view
	vec3_t		firstPersonViewOrg;
	vec3_t		firstPersonViewAngles;
	vec3_t		firstPersonViewAxis[3];

	// spawn variables
	qboolean spawning;                  // the CG_Spawn*() functions are valid
	int numSpawnVars;
	char        *spawnVars[MAX_SPAWN_VARS][2];  // key / value pairs
	int numSpawnVarChars;
	char spawnVarChars[MAX_SPAWN_VARS_CHARS];
	int spawnEntityOffset;

	vec2_t mapcoordsMins;
	vec2_t mapcoordsMaxs;
	qboolean mapcoordsValid;

	int			numMiscGameModels;
	qboolean	hasSkyPortal;
	vec3_t		skyPortalOrigin;
	vec3_t		skyPortalFogColor;
	int			skyPortalFogDepthForOpaque;

	float		skyAlpha;

	int			numViewports;
	int			viewport;

	// Viewport coords in window-coords (don't use with CG_AdjustFrom640!)
	int			viewportX;
	int			viewportY;
	int			viewportWidth;
	int			viewportHeight;

	qboolean	singleCamera; // Rending multiple players using one viewport
#ifdef TURTLEARENA
	qboolean	allLocalClientsAtIntermission;
#endif

	// information screen text during loading
	char		infoScreenText[MAX_STRING_CHARS];

	qboolean	lightstylesInited;

	// global centerprinting (drawn over all viewports)
	int			centerPrintTime;
	float		centerPrintCharScale;
	int			centerPrintY;
	char		centerPrint[1024];

	// say, say_team, ...
	char		messageCommand[32];
	char		messagePrompt[64];
	mfield_t		messageField;

	// scoreboard
	int			scoresRequestTime;
	int			numScores;
	int			intermissionSelectedScore;
	int			teamScores[2];
	score_t		scores[MAX_CLIENTS];
	clientList_t	readyPlayers;
#ifdef MISSIONPACK_HUD
	char			spectatorList[MAX_STRING_CHARS];		// list of names
	int				spectatorTime;							// last time offset
	float			spectatorOffset;						// current offset from start
#endif

	// sound buffer mainly for announcer sounds
	int			soundBufferIn;
	int			soundBufferOut;
	int			soundTime;
	qhandle_t	soundBuffer[MAX_SOUNDBUFFER];

#ifdef MISSIONPACK
	// for voice chat buffer
	int			voiceChatTime;
	int			voiceChatBufferIn;
	int			voiceChatBufferOut;
#endif

	// warmup countdown
	int			warmup;
	int			warmupCount;

	//==========================

#ifdef MISSIONPACK_HARVESTER
	// skull trails
	skulltrail_t	skulltrails[MAX_CLIENTS];
#endif

	// temp working variables for player view
	float		bobfracsin;
	int			bobcycle;
	float		xyspeed;

	// development tool
	refEntity_t		testModelEntity;
	char			testModelName[MAX_QPATH];
	qboolean		testGun;

	// Local player data, from events and such
	localPlayer_t	*cur_lc;	// Current local player data we are working with
	playerState_t	*cur_ps; // Like cur_lc, but for player state
	int				cur_localPlayerNum;
	localPlayer_t	localPlayers[MAX_SPLITVIEW];

} cg_t;

#ifdef IOQ3ZTM // FLAG_ANIMATIONS
typedef enum
{
	FLAG_RUN,
	FLAG_STAND,
	FLAG_STAND2RUN,
	//FLAG_RUNUP,
	//FLAG_RUNDOWN,

	MAX_FLAG_ANIMATIONS
} flagAnimNumber_t;
#endif

// all of the model, shader, and sound references that are
// loaded at gamestate time are stored in cgMedia_t
// Other media that can be tied to players, weapons, or items are
// stored in the playerInfo_t, weaponInfo_t, or itemInfo_t
typedef struct {
	fontInfo_t	tinyFont;
	fontInfo_t	smallFont;
	fontInfo_t	textFont;
	fontInfo_t	bigFont;
	fontInfo_t	numberFont; // status bar giant number font
	fontInfo_t	consoleFont;

	qhandle_t	whiteShader;
	qhandle_t	consoleShader;
	qhandle_t	nodrawShader;
	qhandle_t	whiteDynamicShader;

#ifdef MISSIONPACK
	qhandle_t	redCubeModel;
	qhandle_t	blueCubeModel;
	qhandle_t	redCubeIcon;
	qhandle_t	blueCubeIcon;
#endif
#ifndef TA_DATA // FLAG_MODEL
	qhandle_t	redFlagModel;
	qhandle_t	blueFlagModel;
	qhandle_t	neutralFlagModel;
#endif
	qhandle_t	redFlagShader[3];
	qhandle_t	blueFlagShader[3];
	qhandle_t	flagShader[4];

#ifndef TA_DATA // FLAG_MODEL
	qhandle_t	flagPoleModel;
	qhandle_t	flagFlapModel;

	cgSkin_t	redFlagFlapSkin;
	cgSkin_t	blueFlagFlapSkin;
	cgSkin_t	neutralFlagFlapSkin;
#elif defined TA_WEAPSYS // MELEE_TRAIL
	qhandle_t	flagFlapModel;
#endif

	qhandle_t	redFlagBaseModel;
	qhandle_t	blueFlagBaseModel;
	qhandle_t	neutralFlagBaseModel;

#ifdef IOQ3ZTM // FLAG_ANIMATIONS
	// CTF Flag Animations
	animation_t flag_animations[MAX_FLAG_ANIMATIONS];
#endif

#ifdef MISSIONPACK
	qhandle_t	overloadBaseModel;
	qhandle_t	overloadTargetModel;
	qhandle_t	overloadLightsModel;
	qhandle_t	overloadEnergyModel;

	qhandle_t	harvesterModel;
	cgSkin_t	harvesterRedSkin;
	cgSkin_t	harvesterBlueSkin;
	qhandle_t	harvesterNeutralModel;
#endif

#ifndef TURTLEARENA // NOARMOR
	qhandle_t	armorModel;
	qhandle_t	armorIcon;
#endif

	qhandle_t	teamStatusBar;

	qhandle_t	deferShader;

#ifndef NOTRATEDM // No gibs.
	// gib explosions
	qhandle_t	gibAbdomen;
	qhandle_t	gibArm;
	qhandle_t	gibChest;
	qhandle_t	gibFist;
	qhandle_t	gibFoot;
	qhandle_t	gibForearm;
	qhandle_t	gibIntestine;
	qhandle_t	gibLeg;
	qhandle_t	gibSkull;
	qhandle_t	gibBrain;

	qhandle_t	smoke2;
#endif

	qhandle_t	machinegunBrassModel;
	qhandle_t	shotgunBrassModel;

#ifndef TA_WEAPSYS
	qhandle_t	railRingsShader;
	qhandle_t	railCoreShader;

	qhandle_t	lightningShader;
#endif

#ifdef IOQ3ZTM // SHOW_TEAM_FRIENDS
	qhandle_t	blueFriendShader;
#endif
	qhandle_t	friendShader;
#ifdef TURTLEARENA // LOCKON
	qhandle_t	targetShader;
#endif

	qhandle_t	balloonShader;
	qhandle_t	connectionShader;

	qhandle_t	coronaShader;

	qhandle_t	selectShader;
#ifndef NOBLOOD
	qhandle_t	viewBloodShader;
#endif
	qhandle_t	tracerShader;
	qhandle_t	crosshairShader[NUM_CROSSHAIRS];
	qhandle_t	lagometerShader;
	qhandle_t	backTileShader;
	qhandle_t	noammoShader;

	qhandle_t	smokePuffShader;
	qhandle_t	shotgunSmokePuffShader;
#ifndef TA_WEAPSYS
	qhandle_t	plasmaBallShader;
#endif
	qhandle_t	waterBubbleShader;
#ifndef NOTRATEDM // No gibs.
	qhandle_t	bloodTrailShader;
#endif
#ifdef MISSIONPACK
#ifndef TA_WEAPSYS
	qhandle_t	nailPuffShader;
	qhandle_t	blueProxMine;
#endif
#endif

	qhandle_t	shadowMarkShader;

	qhandle_t	botSkillShaders[5];

	// wall mark shaders
	qhandle_t	wakeMarkShader;
#ifndef NOTRATEDM // No gibs.
	qhandle_t	bloodMarkShader;
#endif
	qhandle_t	bulletMarkShader;
	qhandle_t	burnMarkShader;
	qhandle_t	holeMarkShader;
	qhandle_t	energyMarkShader;

	// powerup shaders
#ifndef TURTLEARENA // POWERS
	qhandle_t	quadShader;
	qhandle_t	redQuadShader;
#endif
	qhandle_t	quadWeaponShader;
	qhandle_t	invisShader;
#ifndef TURTLEARENA // POWERS
	qhandle_t	regenShader;
#endif
	qhandle_t	battleSuitShader;
#ifndef TURTLEARENA // POWERS
	qhandle_t	battleWeaponShader;
#endif
	qhandle_t	hastePuffShader;
#ifdef MISSIONPACK
#ifndef TURTLEARENA // NO_KAMIKAZE_ITEM
	qhandle_t	redKamikazeShader;
	qhandle_t	blueKamikazeShader;
#endif
#endif
#ifdef TURTLEARENA // POWERS // PW_FLASHING
	qhandle_t	playerTeleportShader;
#endif

	// weapon effect models
	qhandle_t	bulletFlashModel;
	qhandle_t	ringFlashModel;
	qhandle_t	dishFlashModel;
	qhandle_t	lightningExplosionModel;

	// weapon effect shaders
	qhandle_t	railExplosionShader;
	qhandle_t	plasmaExplosionShader;
	qhandle_t	bulletExplosionShader;
#ifdef TA_WEAPSYS
	qhandle_t	bulletExplosionColorizeShader;
#endif
	qhandle_t	rocketExplosionShader;
	qhandle_t	grenadeExplosionShader;
	qhandle_t	bfgExplosionShader;
#ifndef NOBLOOD
	qhandle_t	bloodExplosionShader;
#endif
#ifdef TURTLEARENA // WEAPONS
	qhandle_t	meleeHitShader[2];
	qhandle_t	missileHitShader[2];
#endif

	// special effects models
	qhandle_t	teleportEffectModel;
#if !defined MISSIONPACK && !defined TURTLEARENA // ZTM: MP removes loading and using...
	qhandle_t	teleportEffectShader;
#endif
#ifdef MISSIONPACK
	qhandle_t	kamikazeEffectModel;
	qhandle_t	kamikazeShockWave;
	qhandle_t	kamikazeHeadModel;
	qhandle_t	kamikazeHeadTrail;
	qhandle_t	guardPowerupModel;
	qhandle_t	scoutPowerupModel;
	qhandle_t	doublerPowerupModel;
	qhandle_t	ammoRegenPowerupModel;
#ifndef TURTLEARENA // POWERS
	qhandle_t	invulnerabilityImpactModel;
	qhandle_t	invulnerabilityJuicedModel;
#endif
	qhandle_t	medkitUsageModel;
	qhandle_t	dustPuffShader;
#ifndef TURTLEARENA // POWERS
	qhandle_t	invulnerabilityPowerupModel;
#endif
#endif

#ifdef TA_DATA // EXP_SCALE
	qhandle_t	smokeModel;
#endif

	// scoreboard headers
	qhandle_t	scoreboardName;
	qhandle_t	scoreboardPing;
	qhandle_t	scoreboardScore;
	qhandle_t	scoreboardTime;

	// medals shown during gameplay
#ifndef TURTLEARENA // AWARDS
	qhandle_t	medalImpressive;
	qhandle_t	medalExcellent;
	qhandle_t	medalGauntlet;
#endif
	qhandle_t	medalDefend;
	qhandle_t	medalAssist;
	qhandle_t	medalCapture;

#ifdef TA_MISC // MATERIALS
	qhandle_t	matModels[NUM_MATERIAL_TYPES][NUM_MATERIAL_MODELS];
	int			matNumModels[NUM_MATERIAL_TYPES];
#endif

	// sounds
	sfxHandle_t	itemPickupSounds[MAX_ITEMS];
	sfxHandle_t	quadSound;
	sfxHandle_t	tracerSound;
	sfxHandle_t	selectSound;
	sfxHandle_t	useNothingSound;
	sfxHandle_t	wearOffSound;
	sfxHandle_t	footsteps[FOOTSTEP_TOTAL][4];
#ifdef TA_MISC // MATERIALS
	sfxHandle_t matExplode[NUM_MATERIAL_TYPES];
#endif
#ifndef TA_WEAPSYS
	sfxHandle_t	sfx_lghit1;
	sfxHandle_t	sfx_lghit2;
	sfxHandle_t	sfx_lghit3;
#endif
	sfxHandle_t	sfx_ric1;
	sfxHandle_t	sfx_ric2;
	sfxHandle_t	sfx_ric3;
	//sfxHandle_t	sfx_railg;
	sfxHandle_t	sfx_rockexp;
	sfxHandle_t	sfx_plasmaexp;
#ifdef MISSIONPACK
#ifndef TURTLEARENA // WEAPONS NO_KAMIKAZE_ITEM POWERS
	sfxHandle_t	sfx_proxexp;
	sfxHandle_t	sfx_nghit;
	sfxHandle_t	sfx_nghitflesh;
	sfxHandle_t	sfx_nghitmetal;
	sfxHandle_t	sfx_chghit;
	sfxHandle_t	sfx_chghitflesh;
	sfxHandle_t	sfx_chghitmetal;
	sfxHandle_t	sfx_chgstop;
	sfxHandle_t kamikazeExplodeSound;
	sfxHandle_t kamikazeImplodeSound;
	sfxHandle_t kamikazeFarSound;
	sfxHandle_t useInvulnerabilitySound;
	sfxHandle_t invulnerabilityImpactSound1;
	sfxHandle_t invulnerabilityImpactSound2;
	sfxHandle_t invulnerabilityImpactSound3;
	sfxHandle_t invulnerabilityJuicedSound;
#endif
	sfxHandle_t obeliskHitSound1;
	sfxHandle_t obeliskHitSound2;
	sfxHandle_t obeliskHitSound3;
	sfxHandle_t	obeliskRespawnSound;
#ifndef TA_SP
	sfxHandle_t	winnerSound;
	sfxHandle_t	loserSound;
#endif
#endif
#ifndef NOTRATEDM // No gibs.
	sfxHandle_t	gibSound;
	sfxHandle_t	gibBounce1Sound;
	sfxHandle_t	gibBounce2Sound;
	sfxHandle_t	gibBounce3Sound;
#endif
	sfxHandle_t	teleInSound;
	sfxHandle_t	teleOutSound;
	sfxHandle_t	noAmmoSound;
	sfxHandle_t	respawnSound;
	sfxHandle_t talkSound;
#ifndef IOQ3ZTM // MORE_PLAYER_SOUNDS
	sfxHandle_t landSound;
	sfxHandle_t fallSound;
#endif
	sfxHandle_t jumpPadSound;

#ifdef IOQ3ZTM // LETTERBOX
	sfxHandle_t letterBoxOnSound;
	sfxHandle_t letterBoxOffSound;
#endif

	sfxHandle_t oneMinuteSound;
	sfxHandle_t fiveMinuteSound;
	sfxHandle_t suddenDeathSound;

	sfxHandle_t threeFragSound;
	sfxHandle_t twoFragSound;
	sfxHandle_t oneFragSound;

	sfxHandle_t hitSound;
#ifndef TURTLEARENA // NOARMOR
	sfxHandle_t hitSoundHighArmor;
	sfxHandle_t hitSoundLowArmor;
#endif
	sfxHandle_t hitTeamSound;
#ifndef TURTLEARENA // AWARDS
	sfxHandle_t impressiveSound;
	sfxHandle_t excellentSound;
	sfxHandle_t deniedSound;
	sfxHandle_t humiliationSound;
#endif
	sfxHandle_t assistSound;
	sfxHandle_t defendSound;
#ifndef TURTLEARENA // AWARDS
	sfxHandle_t firstImpressiveSound;
	sfxHandle_t firstExcellentSound;
	sfxHandle_t firstHumiliationSound;
#endif

	sfxHandle_t takenLeadSound;
	sfxHandle_t tiedLeadSound;
	sfxHandle_t lostLeadSound;

	sfxHandle_t voteNow;
	sfxHandle_t votePassed;
	sfxHandle_t voteFailed;

	sfxHandle_t watrInSound;
	sfxHandle_t watrOutSound;
	sfxHandle_t watrUnSound;

	sfxHandle_t flightSound;
	sfxHandle_t medkitSound;
#ifdef TURTLEARENA // HOLDABLE
	sfxHandle_t shurikenSound;
#endif

#ifdef MISSIONPACK
	sfxHandle_t weaponHoverSound;
#endif

	// teamplay sounds
	sfxHandle_t captureAwardSound;
	sfxHandle_t redScoredSound;
	sfxHandle_t blueScoredSound;
	sfxHandle_t redLeadsSound;
	sfxHandle_t blueLeadsSound;
	sfxHandle_t teamsTiedSound;

#ifdef TA_DATA
	sfxHandle_t	captureFlagSound;
	sfxHandle_t	returnFlagSound;
#else
	sfxHandle_t	captureYourTeamSound;
	sfxHandle_t	captureOpponentSound;
	sfxHandle_t	returnYourTeamSound;
	sfxHandle_t	returnOpponentSound;
	sfxHandle_t	takenYourTeamSound;
	sfxHandle_t	takenOpponentSound;
#endif

	sfxHandle_t redFlagReturnedSound;
	sfxHandle_t blueFlagReturnedSound;
#ifdef MISSIONPACK
	sfxHandle_t neutralFlagReturnedSound;
#endif
#ifdef TA_DATA_NEWSOUNDS
	sfxHandle_t redTeamTookBlueFlagSound;
	sfxHandle_t blueTeamTookRedFlagSound;
	sfxHandle_t	youHaveFlagSound;
	sfxHandle_t	playerHasFlagSound[MAX_SPLITVIEW];
#ifdef MISSIONPACK
	sfxHandle_t	redTeamTookTheFlagSound;
	sfxHandle_t	blueTeamTookTheFlagSound;
	sfxHandle_t redBaseIsUnderAttackSound;
	sfxHandle_t blueBaseIsUnderAttackSound;
#endif
#else
	sfxHandle_t	enemyTookYourFlagSound;
	sfxHandle_t yourTeamTookEnemyFlagSound;
	sfxHandle_t	youHaveFlagSound;
#ifdef MISSIONPACK
	sfxHandle_t	enemyTookTheFlagSound;
	sfxHandle_t yourTeamTookTheFlagSound;
	sfxHandle_t yourBaseIsUnderAttackSound;
#endif
#endif
#ifndef NOTRATEDM // Disable strong lang.
	sfxHandle_t holyShitSound;
#endif

	// tournament sounds
	sfxHandle_t	count3Sound;
	sfxHandle_t	count2Sound;
	sfxHandle_t	count1Sound;
	sfxHandle_t	countFightSound;
	sfxHandle_t	countPrepareSound;

#ifdef MISSIONPACK
	// new stuff
	qhandle_t patrolShader;
	qhandle_t assaultShader;
	qhandle_t campShader;
	qhandle_t followShader;
	qhandle_t defendShader;
	qhandle_t teamLeaderShader;
	qhandle_t retrieveShader;
	qhandle_t escortShader;
	sfxHandle_t	countPrepareTeamSound;

#ifndef TURTLEARENA // POWERS
	sfxHandle_t ammoregenSound;
	sfxHandle_t doublerSound;
	sfxHandle_t guardSound;
	sfxHandle_t scoutSound;
#endif
#endif

#ifdef MISSIONPACK_HUD
	qhandle_t cursor;
	qhandle_t selectCursor;
	qhandle_t sizeCursor;
	qhandle_t heartShader;
#endif
#if defined MISSIONPACK || defined MISSIONPACK_HUD
	qhandle_t flagShaders[3];
#endif

	sfxHandle_t	regenSound;
	sfxHandle_t	protectSound;
#ifndef TURTLEARENA // POWERS
	sfxHandle_t	n_healthSound;
#endif
#ifndef TA_WEAPSYS
	sfxHandle_t	hgrenb1aSound;
	sfxHandle_t	hgrenb2aSound;
	sfxHandle_t	wstbimplSound;
	sfxHandle_t	wstbimpmSound;
	sfxHandle_t	wstbimpdSound;
	sfxHandle_t	wstbactvSound;
#endif
#ifdef TA_WEAPSYS // MELEE_TRAIL
	qhandle_t	weaponTrailShader;
#endif

#ifdef TURTLEARENA
	qhandle_t	hudHeadBackgroundShader;
	qhandle_t	hudBarShader;
	qhandle_t	hudBar2Shader;
	qhandle_t	hudBarBackgroundShader;
#endif

} cgMedia_t;

#define MAX_STATIC_GAMEMODELS   1024

typedef struct cg_gamemodel_s {
	qhandle_t model;
	cgSkin_t skin;
	vec3_t org;
	vec3_t axes[3];
	vec_t radius;
} cg_gamemodel_t;

// The client game static (cgs) structure hold everything
// loaded or calculated from the gamestate.  It will NOT
// be cleared when a tournement restart is done, allowing
// all clients to begin playing instantly
typedef struct {
	gameState_t		gameState;			// gamestate from server
	glconfig_t		glconfig;			// rendering configuration
	float			screenXScale;		// derived from glconfig
	float			screenYScale;
	float			screenXBias;
	float			screenYBias;
	float			screenXScaleStretch;
	float			screenYScaleStretch;
	float			screenFakeWidth;	// width in fake 640x480 coords, it can be more than 640

	int				maxSplitView;

	int				serverCommandSequence;	// reliable command stream counter
	int				processedSnapshotNum;// the number of snapshots cgame has requested
	int				previousSnapshotNum;// the snapshot from before vid_restart

	qboolean		localServer;		// detected on startup by checking sv_running

	// parsed from serverinfo
	char			gametypeName[MAX_NAME_LENGTH];
	gametype_t		gametype;
	int				dmflags;
	int				fraglimit;
	int				capturelimit;
	int				timelimit;
	int				maxplayers;
	char			mapname[MAX_QPATH];

	int				voteTime;
	int				voteYes;
	int				voteNo;
	qboolean		voteModified;			// beep whenever changed
	char			voteString[MAX_STRING_TOKENS];

	int				teamVoteTime[2];
	int				teamVoteYes[2];
	int				teamVoteNo[2];
	qboolean		teamVoteModified[2];	// beep whenever changed
	char			teamVoteString[2][MAX_STRING_TOKENS];

	int				levelStartTime;

	int				scores1, scores2;		// from configstrings
	int				redflag, blueflag;		// flag status from configstrings
	int				flagStatus;

	qboolean  newHud;

	//
	// locally derived information from gamestate
	//
	qhandle_t		gameModels[MAX_MODELS];
	sfxHandle_t		gameSounds[MAX_SOUNDS];

	int				numInlineModels;
	qhandle_t		inlineDrawModel[MAX_SUBMODELS];
	vec3_t			inlineModelMidpoints[MAX_SUBMODELS];

	playerInfo_t	playerinfo[MAX_CLIENTS];

	int cursorX;
	int cursorY;
	qboolean eventHandling;
	qboolean mouseCaptured;
	qboolean sizingHud;
	void *capturedItem;
	qhandle_t activeCursor;

	// default global fog from bsp or fogvars in a shader
	fogType_t	globalFogType;
	vec3_t		globalFogColor;
	float		globalFogDepthForOpaque;
	float		globalFogDensity;
	float		globalFogFarClip;

	cg_gamemodel_t miscGameModels[MAX_STATIC_GAMEMODELS];

	// media
	cgMedia_t		media;

} cgs_t;

//==============================================================================

extern	cgs_t			cgs;
extern	cg_t			cg;
extern	centity_t		cg_entities[MAX_GENTITIES];
#ifdef TA_WEAPSYS
extern	projectileInfo_t	cg_projectiles[MAX_BG_PROJ];
extern	weaponInfo_t		cg_weapons[MAX_BG_WEAPONS];
extern	weaponGroupInfo_t	cg_weapongroups[MAX_BG_WEAPON_GROUPS];
#else
extern	weaponInfo_t	cg_weapons[MAX_WEAPONS];
#endif
extern	itemInfo_t		cg_items[MAX_ITEMS];
extern	markPoly_t		cg_markPolys[MAX_MARK_POLYS];

extern	vmCvar_t		con_conspeed;
extern	vmCvar_t		con_autochat;
extern	vmCvar_t		con_autoclear;
extern	vmCvar_t		cg_dedicated;

extern	vmCvar_t		cg_centertime;
extern	vmCvar_t		cg_viewbob;
extern	vmCvar_t		cg_viewkick;
extern	vmCvar_t		cg_runpitch;
extern	vmCvar_t		cg_runroll;
extern	vmCvar_t		cg_bobup;
extern	vmCvar_t		cg_bobpitch;
extern	vmCvar_t		cg_bobroll;
extern	vmCvar_t		cg_swingSpeed;
extern	vmCvar_t		cg_shadows;
#ifndef NOTRATEDM // No gibs.
extern	vmCvar_t		cg_gibs;
#endif
#ifdef IOQ3ZTM // DRAW_SPEED
extern	vmCvar_t		cg_drawSpeed;
#endif
extern	vmCvar_t		cg_drawTimer;
#ifdef TA_MISC // COMIC_ANNOUNCER
extern	vmCvar_t		cg_announcerText;
extern	vmCvar_t		cg_announcerVoice;
#endif
extern	vmCvar_t		cg_drawFPS;
extern	vmCvar_t		cg_drawSnapshot;
extern	vmCvar_t		cg_draw3dIcons;
extern	vmCvar_t		cg_drawIcons;
#ifndef TURTLEARENA // NO_AMMO_WARNINGS
extern	vmCvar_t		cg_drawAmmoWarning;
#endif
extern	vmCvar_t		cg_drawCrosshair;
extern	vmCvar_t		cg_drawCrosshairNames;
extern	vmCvar_t		cg_drawRewards;
extern	vmCvar_t		cg_drawTeamOverlay;
extern	vmCvar_t		cg_teamOverlayUserinfo;
extern	vmCvar_t		cg_crosshairX;
extern	vmCvar_t		cg_crosshairY;
extern	vmCvar_t		cg_crosshairSize;
extern	vmCvar_t		cg_crosshairHealth;
extern	vmCvar_t		cg_drawStatus;
extern	vmCvar_t		cg_draw2D;
#ifndef IOQ3ZTM // LERP_FRAME_CLIENT_LESS
extern	vmCvar_t		cg_animSpeed;
extern	vmCvar_t		cg_debugAnim;
#endif
extern	vmCvar_t		cg_debugPosition;
extern	vmCvar_t		cg_debugEvents;
extern	vmCvar_t		cg_railTrailTime;
extern	vmCvar_t		cg_errorDecay;
extern	vmCvar_t		cg_nopredict;
extern	vmCvar_t		cg_noPlayerAnims;
extern	vmCvar_t		cg_showmiss;
extern	vmCvar_t		cg_footsteps;
extern	vmCvar_t		cg_addMarks;
extern	vmCvar_t		cg_brassTime;
extern	vmCvar_t		cg_gun_frame;
extern	vmCvar_t		cg_gun_x;
extern	vmCvar_t		cg_gun_y;
extern	vmCvar_t		cg_gun_z;
extern	vmCvar_t		cg_viewsize;
extern	vmCvar_t		cg_tracerChance;
extern	vmCvar_t		cg_tracerWidth;
extern	vmCvar_t		cg_tracerLength;
extern	vmCvar_t		cg_ignore;
extern	vmCvar_t		cg_simpleItems;
extern	vmCvar_t		cg_fov;
#ifndef TURTLEARENA // NOZOOM
extern	vmCvar_t		cg_zoomFov;
#endif
extern	vmCvar_t		cg_weaponFov;
extern	vmCvar_t		cg_splitviewVertical;
extern	vmCvar_t		cg_splitviewThirdEqual;
extern	vmCvar_t		cg_splitviewTextScale;
extern	vmCvar_t		cg_hudTextScale;
extern	vmCvar_t		cg_drawLagometer;
extern	vmCvar_t		cg_drawAttacker;
extern	vmCvar_t		cg_synchronousClients;
extern	vmCvar_t		cg_singlePlayer;
#ifndef MISSIONPACK_HUD
extern	vmCvar_t		cg_teamChatTime;
extern	vmCvar_t		cg_teamChatHeight;
#endif
extern	vmCvar_t		cg_stats;
#ifndef TURTLEARENA // NO_CGFORCEMODLE
extern	vmCvar_t 		cg_forceModel;
#endif
extern	vmCvar_t 		cg_buildScript;
extern	vmCvar_t		cg_paused;
#ifndef NOBLOOD
extern	vmCvar_t		cg_blood;
#endif
extern	vmCvar_t		cg_predictItems;
extern	vmCvar_t		cg_deferPlayers;
extern	vmCvar_t		cg_drawFriend;
extern	vmCvar_t		cg_teamChatsOnly;
#ifdef MISSIONPACK
extern	vmCvar_t		cg_noVoiceChats;
extern	vmCvar_t		cg_noVoiceText;
#endif
extern  vmCvar_t		cg_scorePlum;
extern	vmCvar_t		cg_smoothClients;
extern	vmCvar_t		pmove_overbounce;
extern	vmCvar_t		pmove_fixed;
extern	vmCvar_t		pmove_msec;
//extern	vmCvar_t		cg_pmove_fixed;
extern	vmCvar_t		cg_timescaleFadeEnd;
extern	vmCvar_t		cg_timescaleFadeSpeed;
extern	vmCvar_t		cg_timescale;
extern	vmCvar_t		cg_cameraMode;
#ifdef MISSIONPACK_HUD
extern  vmCvar_t		cg_smallFont;
extern  vmCvar_t		cg_bigFont;
#endif
#ifdef MISSIONPACK
extern	vmCvar_t		cg_noTaunt;
#endif
extern	vmCvar_t		cg_noProjectileTrail;
extern	vmCvar_t		cg_oldRail;
extern	vmCvar_t		cg_oldRocket;
extern	vmCvar_t		cg_oldPlasma;
extern	vmCvar_t		cg_trueLightning;
extern	vmCvar_t		cg_railWidth;
extern	vmCvar_t		cg_railCoreWidth;
extern	vmCvar_t		cg_railSegmentLength;
extern	vmCvar_t		cg_atmosphericEffects;
extern	vmCvar_t		cg_teamDmLeadAnnouncements;
extern	vmCvar_t		cg_voipShowMeter;
extern	vmCvar_t		cg_voipShowCrosshairMeter;
extern	vmCvar_t		cg_consoleLatency;
extern	vmCvar_t		cg_drawShaderInfo;
extern	vmCvar_t		cg_coronafardist;
extern	vmCvar_t		cg_coronas;
extern	vmCvar_t		cg_fovAspectAdjust;
extern	vmCvar_t		cg_fadeExplosions;
extern	vmCvar_t		cg_skybox;
#ifndef MISSIONPACK_HUD
extern	vmCvar_t		cg_drawScores;
#endif
extern	vmCvar_t		cg_drawPickupItems;
extern	vmCvar_t		cg_oldBubbles;
extern	vmCvar_t		cg_smoothBodySink;
extern	vmCvar_t		cg_antiLag;
extern	vmCvar_t		cg_forceBitmapFonts;
extern	vmCvar_t		cg_drawGrappleHook;
extern	vmCvar_t		cg_drawBBox;
extern	vmCvar_t		cg_consoleFont;
extern	vmCvar_t		cg_consoleFontSize;
extern	vmCvar_t		cg_hudFont;
extern	vmCvar_t		cg_hudFontBorder;
extern	vmCvar_t		cg_numberFont;
extern	vmCvar_t		cg_numberFontBorder;
extern	vmCvar_t		ui_stretch;
#if !defined MISSIONPACK && defined IOQ3ZTM // Support MissionPack players.
extern	vmCvar_t		cg_redTeamName;
extern	vmCvar_t		cg_blueTeamName;
#endif
#ifdef MISSIONPACK
extern	vmCvar_t		cg_redTeamName;
extern	vmCvar_t		cg_blueTeamName;
extern	vmCvar_t		cg_enableDust;
extern	vmCvar_t		cg_enableBreath;
extern  vmCvar_t		cg_recordSPDemo;
extern  vmCvar_t		cg_recordSPDemoName;
extern	vmCvar_t		cg_obeliskRespawnDelay;
#endif
#ifdef TA_WEAPSYS // MELEE_TRAIL
extern	vmCvar_t		cg_drawMeleeWeaponTrails;
#endif
#ifdef TA_MISC // MATERIALS 
extern	vmCvar_t		cg_impactDebris;
#endif
#ifdef TA_PATHSYS // 2DMODE
extern vmCvar_t			cg_2dmode;
extern vmCvar_t			cg_2dmodeOverride;
#endif

extern	vmCvar_t		cg_defaultModelGender;
extern	vmCvar_t		cg_defaultMaleModel;
extern	vmCvar_t		cg_defaultMaleHeadModel;
extern	vmCvar_t		cg_defaultFemaleModel;
extern	vmCvar_t		cg_defaultFemaleHeadModel;

extern	vmCvar_t		cg_defaultTeamModelGender;
extern	vmCvar_t		cg_defaultMaleTeamModel;
extern	vmCvar_t		cg_defaultMaleTeamHeadModel;
extern	vmCvar_t		cg_defaultFemaleTeamModel;
extern	vmCvar_t		cg_defaultFemaleTeamHeadModel;

extern	vmCvar_t		cg_color1[MAX_SPLITVIEW];
extern	vmCvar_t		cg_color2[MAX_SPLITVIEW];
extern	vmCvar_t		cg_handicap[MAX_SPLITVIEW];
extern	vmCvar_t		cg_teamtask[MAX_SPLITVIEW];
extern	vmCvar_t		cg_teampref[MAX_SPLITVIEW];
#ifndef TA_WEAPSYS_EX
extern	vmCvar_t		cg_autoswitch[MAX_SPLITVIEW];
extern	vmCvar_t		cg_cyclePastGauntlet[MAX_SPLITVIEW];
#endif
extern	vmCvar_t		cg_drawGun[MAX_SPLITVIEW];
extern	vmCvar_t		cg_thirdPerson[MAX_SPLITVIEW];
extern	vmCvar_t		cg_thirdPersonRange[MAX_SPLITVIEW];
extern	vmCvar_t		cg_thirdPersonAngle[MAX_SPLITVIEW];
extern	vmCvar_t		cg_thirdPersonHeight[MAX_SPLITVIEW];
extern	vmCvar_t		cg_thirdPersonSmooth[MAX_SPLITVIEW];
#ifdef IOQ3ZTM // ANALOG
extern	vmCvar_t		cg_thirdPersonAnalog[MAX_SPLITVIEW];
#endif

#ifdef MISSIONPACK_HUD
extern	vmCvar_t		cg_currentSelectedPlayer[MAX_SPLITVIEW];
extern	vmCvar_t		cg_currentSelectedPlayerName[MAX_SPLITVIEW];
#endif

//
// cg_main.c
//
const char *CG_ConfigString( int index );
const char *CG_Argv( int arg );
char *CG_Cvar_VariableString( const char *var_name );

int CG_MaxSplitView(void);

void QDECL CG_DPrintf( const char *msg, ... ) __attribute__ ((format (printf, 1, 2)));
void QDECL CG_Printf( const char *msg, ... ) __attribute__ ((format (printf, 1, 2)));
void QDECL CG_Error( const char *msg, ... ) __attribute__ ((noreturn, format (printf, 1, 2)));

void QDECL CG_NotifyPrintf( int localPlayerNum, const char *msg, ... ) __attribute__ ((format (printf, 2, 3)));
void QDECL CG_NotifyBitsPrintf( int localPlayerNum, const char *msg, ... ) __attribute__ ((format (printf, 2, 3)));

void CG_LocalPlayerAdded(int localPlayerNum, int playerNum);
void CG_LocalPlayerRemoved(int localPlayerNum);

void CG_StartMusic( void );

void CG_UpdateCvars( void );

int CG_CrosshairPlayer( int localPlayerNum );
int CG_LastAttacker( int localPlayerNum );
void CG_LoadMenus(const char *menuFile);
void CG_HudMenuHacks( void );
void CG_DistributeKeyEvent( int key, qboolean down, unsigned time, connstate_t state, int joystickNum, int axisNum );
void CG_DistributeCharEvent( int key, connstate_t state );
void CG_KeyEvent(int key, qboolean down);
void CG_MouseEvent(int localPlayerNum, int x, int y);
void CG_JoystickAxisEvent( int localPlayerNum, int axis, int value, unsigned time, connstate_t state );
void CG_JoystickButtonEvent( int localPlayerNum, int button, qboolean down, unsigned time, connstate_t state );
void CG_JoystickHatEvent( int localPlayerNum, int hat, int value, unsigned time, connstate_t state );
void CG_EventHandling(int type);
void CG_RankRunFrame( void );
score_t *CG_GetSelectedScore( void );
#ifdef MISSIONPACK_HUD
void CG_BuildSpectatorString( void );
#endif

void CG_RemoveNotifyLine( localPlayer_t *player );
void CG_AddNotifyText( int realTime, qboolean restoredText );

void CG_SetupDlightstyles( void );

void CG_KillServer( void );

void CG_UpdateMouseState( int localPlayerNum );

// in order from highest priority to lowest
// if none of the catchers are active, bound key strings will be executed
#define	KEYCATCH_CONSOLE		0x0001
#define	KEYCATCH_UI				0x0002
#define	KEYCATCH_MESSAGE		0x0004
#define	KEYCATCH_CGAME			0x0008

int Key_GetCatcher( void );
void Key_SetCatcher( int catcher );


//
// cg_view.c
//
void CG_TestModel_f (void);
void CG_TestModelComplete( char *args, int argNum );
void CG_TestGun_f (void);
void CG_TestModelNextFrame_f (void);
void CG_TestModelPrevFrame_f (void);
void CG_TestModelNextSkin_f (void);
void CG_TestModelPrevSkin_f (void);
#ifndef TURTLEARENA // NOZOOM
void CG_ZoomUp_f( int localPlayerNum );
void CG_ZoomDown_f( int localPlayerNum );
#endif
void CG_AddBufferedSound( sfxHandle_t sfx);
void CG_StepOffset( vec3_t vieworg );

#ifdef TA_MISC // COMIC_ANNOUNCER
typedef enum
{
	ANNOUNCE_PREPAREYOURSELFS,
	ANNOUNCE_PREPAREYOURTEAM,

	ANNOUNCE_VOTINGBEGUN,
	ANNOUNCE_VOTEPASS,
	ANNOUNCE_VOTEFAIL,

	ANNOUNCE_YOUHAVETAKENTHELEAD,
	ANNOUNCE_YOURTIEDFORTHELEAD,
	ANNOUNCE_YOULOSTTHELEAD,

	ANNOUNCE_CAPTURE,
	ANNOUNCE_ASSIST,
	ANNOUNCE_DEFENSE,

	// TEAM/CTF/1FCTF/Overload
	ANNOUNCE_REDLEADS,
	ANNOUNCE_BLUELEADS,
	ANNOUNCE_TEAMSTIED,

	// CTF and one flag CTF
	ANNOUNCE_YOUHAVETHEFLAG,
	ANNOUNCE_REDSCORES,
	ANNOUNCE_BLUESCORES,
	ANNOUNCE_TEAMCAPTURE,
	ANNOUNCE_ENEMYCAPTURE,
	ANNOUNCE_YOURFLAGRETURNED,
	ANNOUNCE_ENEMYFLAGRETURNED,
	ANNOUNCE_YOURTEAMHASTAKENTHEFLAG,
	ANNOUNCE_THEENEMYHASTAKENTHEFLAG,

	// CTF
	ANNOUNCE_REDFLAGRETURNED,
	ANNOUNCE_BLUEFLAGRETURNED,
	ANNOUNCE_ENEMYHASYOURFLAG,
	ANNOUNCE_TEAMHASENEMYFLAG,

	// One flag CTF
	ANNOUNCE_TEAMHASTHEFLAG,
	ANNOUNCE_ENEMYHASTHEFLAG,

	// Overload
	ANNOUNCE_BASEUNDERATTACK,

	ANNOUNCE_MAX
} announcement_e;

void CG_AddAnnouncement(int announcement, int localPlayerNum);
void CG_AddAnnouncementEx(localPlayer_t *player, qhandle_t sfx, qboolean bufferedSfx, const char *message);
#endif

void CG_CalcVrect( void );

void CG_SetupFrustum( void );
qboolean CG_CullPoint( vec3_t pt );
qboolean CG_CullPointAndRadius( const vec3_t pt, vec_t radius );

void CG_DrawActiveFrame( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback );


//
// cg_drawtools.c
//
typedef enum {
	PLACE_STRETCH,
	PLACE_CENTER,

	// horizontal only
	PLACE_LEFT,
	PLACE_RIGHT,

	// vertical only
	PLACE_TOP,
	PLACE_BOTTOM
} screenPlacement_e;

void CG_SetScreenPlacement(screenPlacement_e hpos, screenPlacement_e vpos);
void CG_PopScreenPlacement(void);
screenPlacement_e CG_GetScreenHorizontalPlacement(void);
screenPlacement_e CG_GetScreenVerticalPlacement(void);
void CG_AdjustFrom640( float *x, float *y, float *w, float *h );
void CG_FillRect( float x, float y, float width, float height, const float *color );
void CG_DrawPic( float x, float y, float width, float height, qhandle_t hShader );
void CG_DrawPicColor( float x, float y, float width, float height, qhandle_t hShader, const float *color );
void CG_DrawNamedPic( float x, float y, float width, float height, const char *picname );
void CG_SetClipRegion( float x, float y, float w, float h );
void CG_ClearClipRegion( void );
void CG_LerpColor( const vec4_t a, const vec4_t b, vec4_t c, float t );

void CG_DrawString( int x, int y, const char* str, int style, const vec4_t color );
void CG_DrawStringWithCursor( int x, int y, const char* str, int style, const fontInfo_t *font, const vec4_t color, int cursorPos, int cursorChar );
void CG_DrawStringExt( int x, int y, const char* str, int style, const vec4_t color, float scale, int maxChars, float shadowOffset );
void CG_DrawStringAutoWrap( int x, int y, const char* str, int style, const vec4_t color, float scale, float shadowOffset, float gradient, float wrapX );
void CG_DrawStringCommon( int x, int y, const char* str, int style, const fontInfo_t *font, const vec4_t color, float scale, int maxChars, float shadowOffset, float gradient, int cursorPos, int cursorChar, float wrapX );
void CG_DrawBigString( int x, int y, const char *s, float alpha );
void CG_DrawBigStringColor( int x, int y, const char *s, vec4_t color );
void CG_DrawSmallString( int x, int y, const char *s, float alpha );
void CG_DrawSmallStringColor( int x, int y, const char *s, vec4_t color );

float CG_DrawStrlenCommon( const char *str, int style, const fontInfo_t *font, int maxchars );
float CG_DrawStrlenMaxChars( const char *str, int style, int maxchars );
float CG_DrawStrlen( const char *str, int style );
int CG_DrawStringLineHeight( int style );

void CG_MField_Draw( mfield_t *edit, int x, int y, int style, vec4_t color, qboolean drawCursor );

float	*CG_FadeColor( int startMsec, int totalMsec );
#ifdef TURTLEARENA // NIGHTS_ITEMS
void	CG_ColorForChain(int val, vec3_t color);
#endif
float *CG_TeamColor( int team );
void CG_TileClear( void );
void CG_KeysStringForBinding(const char *binding, char *string, int stringSize );
void CG_ColorForHealth( vec4_t hcolor );
#ifdef TURTLEARENA // NOARMOR
void CG_GetColorForHealth( int health, vec4_t hcolor );
#else
void CG_GetColorForHealth( int health, int armor, vec4_t hcolor );
#endif

void CG_DrawRect( float x, float y, float width, float height, float size, const float *color );
void CG_DrawSides(float x, float y, float w, float h, float size);
void CG_DrawTopBottom(float x, float y, float w, float h, float size);
void CG_ClearViewport( void );


//
// cg_draw.c, cg_newDraw.c
//
typedef enum {
  SYSTEM_PRINT,
  CHAT_PRINT,
  TEAMCHAT_PRINT
} q3print_t;

extern	int sortedTeamPlayers[TEAM_NUM_TEAMS][TEAM_MAXOVERLAY];
extern	int	numSortedTeamPlayers[TEAM_NUM_TEAMS];
extern	int	sortedTeamPlayersTime[TEAM_NUM_TEAMS];
extern	int drawTeamOverlayModificationCount;
extern  char systemChat[256];
extern  char teamChat1[256];
extern  char teamChat2[256];

void CG_AddLagometerFrameInfo( void );
void CG_AddLagometerSnapshotInfo( snapshot_t *snap );
void CG_CenterPrint( int localPlayerNum, const char *str, int y, float charScale );
void CG_GlobalCenterPrint( const char *str, int y, float charScale );
void CG_DrawHead( float x, float y, float w, float h, int playerNum, vec3_t headAngles );
void CG_DrawActive( stereoFrame_t stereoView );
void CG_DrawScreen2D( stereoFrame_t stereoView );
void CG_DrawFlagModel( float x, float y, float w, float h, int team, qboolean force2D );
void CG_DrawTeamBackground( int x, int y, int w, int h, float alpha, int team );
void CG_OwnerDraw(float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, int align, float special, float scale, vec4_t color, qhandle_t shader, int textStyle);
void CG_SelectPrevPlayer( int localPlayerNum );
void CG_SelectNextPlayer( int localPlayerNum );
float CG_GetValue(int ownerDraw);
qboolean CG_OwnerDrawVisible(int flags);
void CG_RunMenuScript(char **args);
void CG_SetPrintString(q3print_t type, const char *p);
void CG_InitTeamChat( void );
void CG_GetTeamColor(vec4_t *color);
const char *CG_GetGameStatusText( void );
const char *CG_GetKillerText( void );
void CG_Draw3DModel(float x, float y, float w, float h, qhandle_t model, cgSkin_t *skin, vec3_t origin, vec3_t angles);
#ifdef IOQ3ZTM
void CG_Draw3DHeadModel( int clientNum, float x, float y, float w, float h, vec3_t origin, vec3_t angles );
#endif
#ifdef MISSIONPACK
void CG_CheckOrderPending( int localPlayerNum );
#endif
const char *CG_GameTypeString( void );
qboolean CG_YourTeamHasFlag( void );
qboolean CG_OtherTeamHasFlag( void );
#ifdef MISSIONPACK
qhandle_t CG_StatusHandle(int task);
#endif
qboolean CG_AnyScoreboardShowing( void );



//
// cg_text.c
//
#define GLYPH_INSERT 10
#define GLYPH_OVERSTRIKE 11
#define GLYPH_ARROW 13

void CG_HudTextInit( void );
void CG_InitBitmapFont( fontInfo_t *font, int charHeight, int charWidth );
void CG_InitBitmapNumberFont( fontInfo_t *font, int charHeight, int charWidth );
qboolean CG_InitTrueTypeFont( const char *name, int pointSize, float borderWidth, fontInfo_t *font );
fontInfo_t *CG_FontForScale( float scale );

const glyphInfo_t *Text_GetGlyph( const fontInfo_t *font, unsigned long index );
float Text_Width( const char *text, const fontInfo_t *font, float scale, int limit );
float Text_Height( const char *text, const fontInfo_t *font, float scale, int limit );
void Text_PaintGlyph( float x, float y, float w, float h, const glyphInfo_t *glyph, float *gradientColor );
void Text_Paint( float x, float y, const fontInfo_t *font, float scale, const vec4_t color, const char *text, float adjust, int limit, float shadowOffset, float gradient, qboolean forceColor, qboolean textInMotion );
void Text_PaintWithCursor( float x, float y, const fontInfo_t *font, float scale, const vec4_t color, const char *text, int cursorPos, char cursor, float adjust, int limit, float shadowOffset, float gradient, qboolean forceColor, qboolean textInMotion );
void Text_Paint_Limit( float *maxX, float x, float y, const fontInfo_t *font, float scale, const vec4_t color, const char* text, float adjust, int limit, qboolean textInMotion );
void Text_Paint_AutoWrapped( float x, float y, const fontInfo_t *font, float scale, const vec4_t color, const char *str, float adjust, int limit, float shadowOffset, float gradient, qboolean forceColor, qboolean textInMotion, float xmax, float ystep, int style );

void CG_Text_Paint( float x, float y, float scale, const vec4_t color, const char *text, float adjust, int limit, int textStyle );
void CG_Text_PaintWithCursor( float x, float y, float scale, const vec4_t color, const char *text, int cursorPos, char cursor, int limit, int textStyle );
void CG_Text_Paint_Limit( float *maxX, float x, float y, float scale, const vec4_t color, const char* text, float adjust, int limit );
int CG_Text_Width( const char *text, float scale, int limit );
int CG_Text_Height( const char *text, float scale, int limit );


//
// cg_players.c
//
void CG_Player( centity_t *cent );
void CG_ResetPlayerEntity( centity_t *cent );
void CG_AddRefEntityWithPowerups( refEntity_t *ent, entityState_t *state );
qhandle_t CG_AddSkinToFrame( const cgSkin_t *skin );
qboolean CG_RegisterSkin( const char *name, cgSkin_t *skin, qboolean append );
void CG_NewPlayerInfo( int playerNum );
sfxHandle_t	CG_CustomSound( int playerNum, const char *soundName );
void CG_CachePlayerSounds( const char *modelName );
void CG_CachePlayerModels( const char *modelName, const char *headModelName );
void CG_PlayerColorFromIndex( int val, vec3_t color );

//
// cg_predict.c
//
void CG_BuildSolidList( void );
int	CG_PointContents( const vec3_t point, int passEntityNum );
void CG_Trace( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, 
					 int skipNumber, int mask );
void CG_PredictPlayerState( void );
void CG_LoadDeferredPlayers( void );


//
// cg_events.c
//
void CG_CheckEvents( centity_t *cent );
const char	*CG_PlaceString( int rank );
void CG_EntityEvent( centity_t *cent, vec3_t position );
void CG_PainEvent( centity_t *cent, int health );


//
// cg_ents.c
//
void CG_AddRefEntityWithMinLight( const refEntity_t *entity );
void CG_SetEntitySoundPosition( centity_t *cent );
void CG_AddPacketEntities( void );
void CG_Beam( centity_t *cent );
void CG_AdjustPositionForMover(const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out, vec3_t angles_in, vec3_t angles_out);

qboolean CG_PositionEntityOnTag( refEntity_t *entity, const refEntity_t *parent,
							qhandle_t parentModel, char *tagName );
qboolean CG_PositionRotatedEntityOnTag( refEntity_t *entity, const refEntity_t *parent,
							qhandle_t parentModel, char *tagName );



//
// cg_weapons.c
//
#ifdef TA_HOLDSYS/*2*/
void CG_NextHoldable_f( int localPlayerNum );
void CG_PrevHoldable_f( int localPlayerNum );
void CG_Holdable_f( int localPlayerNum );
#endif

#ifndef TA_WEAPSYS_EX
void CG_NextWeapon_f( int localPlayerNum );
void CG_PrevWeapon_f( int localPlayerNum );
void CG_Weapon_f( int localPlayerNum );
void CG_WeaponToggle_f( int localPlayerNum );
#endif

#ifdef TURTLEARENA // HOLD_SHURIKEN
void CG_RegisterHoldable( int holdableNum );
#endif
#ifdef TA_WEAPSYS
void CG_RegisterProjectile( int projectileNum );
#endif
void CG_RegisterWeapon( int weaponNum );
#ifdef TA_WEAPSYS
void CG_RegisterWeaponGroup( int weaponNum );
#endif
void CG_RegisterItemVisuals( int itemNum );

void CG_FireWeapon( centity_t *cent );
#ifdef TA_MISC // MATERIALS
void CG_ImpactParticles( vec3_t origin, vec3_t dir, float radius, int surfaceFlags, int skipNum );
#endif
void CG_MissileExplode( int weapon, int playerNum, vec3_t origin, vec3_t dir, impactSound_t soundType );
void CG_MissileHitPlayer( int weapon, vec3_t origin, vec3_t dir, int entityNum );
#ifdef TA_WEAPSYS
void CG_MissileImpact( int projnum, int clientNum, vec3_t origin, vec3_t dir, impactSound_t soundType );
void CG_WeaponImpact( int weaponGroup, int hand, int clientNum, vec3_t origin, vec3_t dir, impactSound_t soundType );
void CG_WeaponHitPlayer( int weaponGroup, int hand, vec3_t origin, vec3_t dir, int entityNum );
#else
void CG_ShotgunPattern( vec3_t origin, vec3_t origin2, int seed, int otherEntNum );
void CG_ShotgunFire( entityState_t *es );
#endif
#ifdef TA_WEAPSYS
void CG_Bullet( vec3_t end, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum, int projnum);
#else
void CG_Bullet( vec3_t end, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum);
#endif

#ifdef TA_WEAPSYS
void CG_RailTrail( playerInfo_t *pi, const projectileInfo_t *wi, vec3_t start, vec3_t end );
void CG_GrappleTrail( centity_t *ent, const projectileInfo_t *wi );
#else
void CG_RailTrail( playerInfo_t *pi, vec3_t start, vec3_t end );
void CG_GrappleTrail( centity_t *ent, const weaponInfo_t *wi );
#endif
void CG_AddViewWeapon (playerState_t *ps);
void CG_AddPlayerWeapon( refEntity_t *parent, playerState_t *ps, centity_t *cent, int team );
#ifndef TA_WEAPSYS_EX
void CG_DrawWeaponSelect( void );

void CG_OutOfAmmoChange( int localPlayerNum );	// should this be in pmove?
#endif
#ifdef IOQ3ZTM // GHOST
void CG_GhostRefEntity(refEntity_t *refEnt, ghostRefData_t *refs, int num, int *ghostTime);
#endif

//
// cg_marks.c
//
void	CG_InitMarkPolys( void );
void	CG_AddMarks( void );
#ifdef TA_WEAPSYS
qboolean	CG_ImpactMark( qhandle_t markShader,
				    const vec3_t origin, const vec3_t dir, 
					float orientation, 
				    float r, float g, float b, float a, 
					qboolean alphaFade, 
					float radius, qboolean temporary );
#else
void	CG_ImpactMark( qhandle_t markShader,
				    const vec3_t origin, const vec3_t dir,
					float orientation,
				    float r, float g, float b, float a,
					qboolean alphaFade,
					float radius, qboolean temporary );
#endif

//
// cg_localents.c
//
void	CG_InitLocalEntities( void );
localEntity_t	*CG_AllocLocalEntity( void );
void	CG_AddLocalEntities( void );

//
// cg_effects.c
//
localEntity_t *CG_SmokePuff( const vec3_t p, 
				   const vec3_t vel, 
				   float radius,
				   float r, float g, float b, float a,
				   float duration,
				   int startTime,
				   int fadeInTime,
				   int leFlags,
				   qhandle_t hShader );
void CG_BubbleTrail( vec3_t start, vec3_t end, float spacing );
#ifdef TA_WEAPSYS
qboolean CG_BulletBubbleTrail( vec3_t start, vec3_t end, int skipNum );
#endif
int CG_SpawnBubbles( localEntity_t **bubbles, vec3_t origin, float baseSize, int numBubbles );
void CG_SpawnEffect( vec3_t org );
#ifdef MISSIONPACK
#ifndef TURTLEARENA // NO_KAMIKAZE_ITEM
void CG_KamikazeEffect( vec3_t org );
#endif
void CG_ObeliskExplode( vec3_t org, int entityNum );
void CG_ObeliskPain( vec3_t org );
#ifndef TURTLEARENA // POWERS
void CG_InvulnerabilityImpact( vec3_t org, vec3_t angles );
void CG_InvulnerabilityJuiced( vec3_t org );
#ifdef TA_WEAPSYS
void CG_LightningBoltBeam( projectileInfo_t *wi, vec3_t start, vec3_t end );
#else
void CG_LightningBoltBeam( vec3_t start, vec3_t end );
#endif
#endif
#endif
void CG_ScorePlum( int playerNum, vec3_t org, int score );
#ifdef TURTLEARENA // NIGHTS_ITEMS
void CG_ChainPlum( int playerNum, vec3_t org, int score, int chain, qboolean bonus );
#endif

#ifndef NOTRATEDM // No gibs.
void CG_GibPlayer( vec3_t playerOrigin );
void CG_BigExplode( vec3_t playerOrigin );
#endif

#ifndef TA_WEAPSYS
#ifndef NOBLOOD
void CG_Bleed( vec3_t origin, int entityNum );
#endif
#endif

localEntity_t *CG_MakeExplosion( vec3_t origin, vec3_t dir,
								qhandle_t hModel, qhandle_t shader, int msec,
								qboolean isSprite );
#ifdef TA_ENTSYS // EXP_SCALE
void CG_ExplosionEffect(vec3_t origin, int radius, int entity);
#endif
#ifdef IOQ3ZTM // LETTERBOX
void CG_ToggleLetterbox(qboolean onscreen, qboolean instant);
void CG_DrawLetterbox(void);
#endif

//
// cg_snapshot.c
//
void CG_ProcessSnapshots( qboolean initialOnly );
void CG_RestoreSnapshot( void );
void CG_TransitionEntity( centity_t *cent );
playerState_t *CG_LocalPlayerState( int playerNum );
int CG_NumLocalPlayers( void );


//
// cg_surface.c
//
qboolean CG_AddCustomSurface( const refEntity_t *re );
void CG_SurfaceSprite( const refEntity_t *re );
void CG_SurfaceRailRings( const refEntity_t *re );
void CG_SurfaceRailCore( const refEntity_t *re );
void CG_SurfaceLightningBolt( const refEntity_t *re );
void CG_SurfaceBeam( const refEntity_t *re );
void CG_SurfaceText( const refEntity_t *re, const fontInfo_t *font, float scale, const char *text, float adjust, int limit, float gradient, qboolean forceColor );

//
// cg_spawn.c
//
qboolean    CG_SpawnString( const char *key, const char *defaultString, char **out );
// spawn string returns a temporary reference, you must CopyString() if you want to keep it
qboolean    CG_SpawnFloat( const char *key, const char *defaultString, float *out );
qboolean    CG_SpawnInt( const char *key, const char *defaultString, int *out );
qboolean    CG_SpawnVector( const char *key, const char *defaultString, float *out );
void        CG_ParseEntitiesFromString( void );

//
// cg_info.c
//
void CG_LoadingString( const char *s );
#ifndef TURTLEARENA // NO_LOADING_ICONS
void CG_LoadingItem( int itemNum );
void CG_LoadingPlayer( int playerNum );
#endif
void CG_DrawInformation( void );

//
// cg_scoreboard.c
//
qboolean CG_DrawOldScoreboard( void );
void CG_DrawTourneyScoreboard( void );

//
// cg_console.c
//
void CG_ConsoleInit( void );
void CG_ConsoleResized( void );
void CG_ConsolePrint( const char *text );
void CG_CloseConsole( void );
void Con_ClearConsole_f( void );
void Con_ToggleConsole_f( void );
void CG_RunConsole( connstate_t state );
void Console_Key ( int key, qboolean down );

//
// cg_consolecmds.c
//
#define CMD_INGAME	1 // only usable while in-game
#define CMD_MENU	2 // only usable while at main menu

typedef struct {
	char	*cmd;
	void	(*function)(void);
	int		flags;
	void	(*complete)(char *, int);
} consoleCommand_t;

qboolean CG_ConsoleCommand( connstate_t state, int realTime );
qboolean CG_ConsoleCompleteArgument( connstate_t state, int realTime, int completeArgument );
void CG_InitConsoleCommands( void );

void CG_StopCinematic_f( void );

//
// cg_servercmds.c
//
void CG_ExecuteNewServerCommands( int latestSequence );
void CG_ParseServerinfo( void );
void CG_SetConfigValues( void );
void CG_ShaderStateChanged(void);
#ifdef MISSIONPACK
void CG_LoadVoiceChats( void );
void CG_VoiceChatLocal( int localPlayerBits, int mode, qboolean voiceOnly, int playerNum, int color, const char *cmd );
void CG_PlayBufferedVoiceChats( void );
#endif
int CG_LocalPlayerBitsForTeam( team_t );
void CG_ReplaceCharacter( char *str, char old, char new );

//
// cg_playerstate.c
//
void CG_Respawn( int playerNum );
void CG_TransitionPlayerState( playerState_t *ps, playerState_t *ops );
void CG_CheckChangedPredictableEvents( playerState_t *ps );
void CG_CheckGameSounds( void );

//
// cg_atmospheric.c
//
void CG_EffectParse(const char *effectstr);
void CG_AddAtmosphericEffects(void);

//
// cg_polybus.c
//
polyBuffer_t* CG_PB_FindFreePolyBuffer( qhandle_t shader, int numVerts, int numIndicies );
void CG_PB_ClearPolyBuffers( void );
void CG_PB_RenderPolyBuffers( void );

//
// cg_particles.c
//
void	CG_ClearParticles (void);
void	CG_AddParticles (void);
void	CG_ParticleSnow (qhandle_t pshader, vec3_t origin, vec3_t origin2, int turb, float range, int snum);
void	CG_ParticleSmoke (qhandle_t pshader, centity_t *cent);
void	CG_AddParticleShrapnel (localEntity_t *le);
void	CG_ParticleSnowFlurry (qhandle_t pshader, centity_t *cent);
void	CG_ParticleBulletDebris (vec3_t	org, vec3_t vel, int duration);
void	CG_ParticleSparks (vec3_t org, vec3_t vel, int duration, float x, float y, float speed);
void	CG_ParticleDust (centity_t *cent, vec3_t origin, vec3_t dir);
void	CG_ParticleMisc (qhandle_t pshader, vec3_t origin, int size, int duration, float alpha);
void	CG_ParticleExplosion (char *animStr, vec3_t origin, vec3_t vel, int duration, int sizeStart, int sizeEnd);
extern qboolean		initparticles;
int CG_NewParticleArea ( int num );

//
// cg_input.c
//
void CG_RegisterInputCvars( void );
void CG_UpdateInputCvars( void );
usercmd_t *CG_CreateUserCmd( int localPlayerNum, int frameTime, unsigned frameMsec, float mx, float my, qboolean anykeydown );

void IN_CenterView( int localPlayerNum );

void IN_MLookDown( int localPlayerNum );
void IN_MLookUp( int localPlayerNum );

void IN_UpDown( int localPlayerNum );
void IN_UpUp( int localPlayerNum );
void IN_DownDown( int localPlayerNum );
void IN_DownUp( int localPlayerNum );
void IN_LeftDown( int localPlayerNum );
void IN_LeftUp( int localPlayerNum );
void IN_RightDown( int localPlayerNum );
void IN_RightUp( int localPlayerNum );
void IN_ForwardDown( int localPlayerNum );
void IN_ForwardUp( int localPlayerNum );
void IN_BackDown( int localPlayerNum );
void IN_BackUp( int localPlayerNum );
void IN_LookupDown( int localPlayerNum );
void IN_LookupUp( int localPlayerNum );
void IN_LookdownDown( int localPlayerNum );
void IN_LookdownUp( int localPlayerNum );
void IN_MoveleftDown( int localPlayerNum );
void IN_MoveleftUp( int localPlayerNum );
void IN_MoverightDown( int localPlayerNum );
void IN_MoverightUp( int localPlayerNum );

#ifndef TURTLEARENA // NO_SPEED_KEY
void IN_SpeedDown( int localPlayerNum );
void IN_SpeedUp( int localPlayerNum );
#endif
void IN_StrafeDown( int localPlayerNum );
void IN_StrafeUp( int localPlayerNum );
#ifdef TURTLEARENA // LOCKON
void IN_LockonDown( int localPlayerNum );
void IN_LockonUp( int localPlayerNum );
#endif

void IN_Button0Down( int localPlayerNum );
void IN_Button0Up( int localPlayerNum );
void IN_Button1Down( int localPlayerNum );
void IN_Button1Up( int localPlayerNum );
void IN_Button2Down( int localPlayerNum );
void IN_Button2Up( int localPlayerNum );
void IN_Button3Down( int localPlayerNum );
void IN_Button3Up( int localPlayerNum );
void IN_Button4Down( int localPlayerNum );
void IN_Button4Up( int localPlayerNum );
void IN_Button5Down( int localPlayerNum );
void IN_Button5Up( int localPlayerNum );
void IN_Button6Down( int localPlayerNum );
void IN_Button6Up( int localPlayerNum );
void IN_Button7Down( int localPlayerNum );
void IN_Button7Up( int localPlayerNum );
void IN_Button8Down( int localPlayerNum );
void IN_Button8Up( int localPlayerNum );
void IN_Button9Down( int localPlayerNum );
void IN_Button9Up( int localPlayerNum );
void IN_Button10Down( int localPlayerNum );
void IN_Button10Up( int localPlayerNum );
void IN_Button11Down( int localPlayerNum );
void IN_Button11Up( int localPlayerNum );
void IN_Button12Down( int localPlayerNum );
void IN_Button12Up( int localPlayerNum );
void IN_Button13Down( int localPlayerNum );
void IN_Button13Up( int localPlayerNum );
void IN_Button14Down( int localPlayerNum );
void IN_Button14Up( int localPlayerNum );

//
// cg_unlagged.c
//
void CG_DrawBBox( centity_t *cent, float *color );

//
// cg_audio.c
//
void CG_InitAudio( void );
void CG_GetMusicForIntro( char *intro, char *loop, float *volume, float *loopVolume );
void CG_SetMusic( const char *intro, const char *loop );
void CG_StopMusic( void );


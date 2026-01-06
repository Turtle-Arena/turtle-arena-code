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
// cg_view.c -- setup all the parameters (position, angle, etc)
// for a 3D rendering
#include "cg_local.h"


/*
=============================================================================

  MODEL TESTING

The viewthing and gun positioning tools from Q2 have been integrated and
enhanced into a single model testing facility.

Model viewing can begin with either "testmodel <modelname>" or "testgun <modelname>".

The names must be the full pathname after the basedir, like 
"models/weapons/v_launch/tris.md3" or "players/male/tris.md3"

Testmodel will create a fake entity 100 units in front of the current view
position, directly facing the viewer.  It will remain immobile, so you can
move around it to view it from different angles.

Testgun will cause the model to follow the player around and suppress the real
view weapon model.  The default frame 0 of most guns is completely off screen,
so you will probably have to cycle a couple frames to see it.

"nextframe", "prevframe", "nextskin", and "prevskin" commands will change the
frame or skin of the testmodel.  These are bound to F5, F6, F7, and F8 in
q3default.cfg.

If a gun is being tested, the "gun_x", "gun_y", and "gun_z" variables will let
you adjust the positioning.

Note that none of the model testing features update while the game is paused, so
it may be convenient to test with deathmatch set to 1 so that bringing down the
console doesn't pause the game.

=============================================================================
*/

/*
=================
CG_TestModel_f

Creates an entity in front of the current position, which
can then be moved around
=================
*/
void CG_TestModel_f (void) {
	vec3_t		angles;

	cg.testGun = qfalse;
	memset( &cg.testModelEntity, 0, sizeof(cg.testModelEntity) );
	if ( trap_Argc() < 2 ) {
		return;
	}

	Q_strncpyz (cg.testModelName, CG_Argv( 1 ), MAX_QPATH );
	cg.testModelEntity.hModel = trap_R_RegisterModel( cg.testModelName );

	if ( trap_Argc() == 3 ) {
		cg.testModelEntity.backlerp = atof( CG_Argv( 2 ) );
		cg.testModelEntity.frame = 1;
		cg.testModelEntity.oldframe = 0;
	}
	if (! cg.testModelEntity.hModel ) {
		CG_Printf( "Can't register model\n" );
		return;
	}

	VectorMA( cg.refdef.vieworg, 100, cg.refdef.viewaxis[0], cg.testModelEntity.origin );

	angles[PITCH] = 0;
	angles[YAW] = 180 + cg.refdefViewAngles[1];
	angles[ROLL] = 0;

	AnglesToAxis( angles, cg.testModelEntity.axis );
}

/*
=================
CG_TestModelComplete
=================
*/
void CG_TestModelComplete( char *args, int argNum ) {
	if ( argNum == 2 ) {
		trap_Field_CompleteFilename( "", "$models", qfalse, qfalse );
	}
}

/*
=================
CG_TestGun_f

Replaces the current view weapon with the given model
=================
*/
void CG_TestGun_f (void) {
	CG_TestModel_f();

	if ( !cg.testModelEntity.hModel ) {
		return;
	}

	cg.testGun = qtrue;
	cg.testModelEntity.renderfx = RF_DEPTHHACK | RF_NO_MIRROR;
}


void CG_TestModelNextFrame_f (void) {
	cg.testModelEntity.frame++;
	CG_Printf( "frame %i\n", cg.testModelEntity.frame );
}

void CG_TestModelPrevFrame_f (void) {
	cg.testModelEntity.frame--;
	if ( cg.testModelEntity.frame < 0 ) {
		cg.testModelEntity.frame = 0;
	}
	CG_Printf( "frame %i\n", cg.testModelEntity.frame );
}

void CG_TestModelNextSkin_f (void) {
	cg.testModelEntity.skinNum++;
	CG_Printf( "skin %i\n", cg.testModelEntity.skinNum );
}

void CG_TestModelPrevSkin_f (void) {
	cg.testModelEntity.skinNum--;
	if ( cg.testModelEntity.skinNum < 0 ) {
		cg.testModelEntity.skinNum = 0;
	}
	CG_Printf( "skin %i\n", cg.testModelEntity.skinNum );
}

static void CG_AddTestModel (void) {
	int		i;

	// re-register the model, because the level may have changed
	cg.testModelEntity.hModel = trap_R_RegisterModel( cg.testModelName );
	if (! cg.testModelEntity.hModel ) {
		CG_Printf ("Can't register model\n");
		return;
	}

	// if testing a gun, set the origin relative to the view origin
	if ( cg.testGun ) {
		VectorCopy( cg.refdef.vieworg, cg.testModelEntity.origin );
		VectorCopy( cg.refdef.viewaxis[0], cg.testModelEntity.axis[0] );
		VectorCopy( cg.refdef.viewaxis[1], cg.testModelEntity.axis[1] );
		VectorCopy( cg.refdef.viewaxis[2], cg.testModelEntity.axis[2] );

		// allow the position to be adjusted
		for (i=0 ; i<3 ; i++) {
			cg.testModelEntity.origin[i] += cg.refdef.viewaxis[0][i] * cg_gun_x.value;
			cg.testModelEntity.origin[i] += cg.refdef.viewaxis[1][i] * cg_gun_y.value;
			cg.testModelEntity.origin[i] += cg.refdef.viewaxis[2][i] * cg_gun_z.value;
		}
	}

	CG_AddRefEntityWithMinLight( &cg.testModelEntity );
}



//============================================================================


/*
=================
CG_CalcVrect

Sets the coordinates of the viewport and rendered window
=================
*/
void CG_CalcVrect (void) {
	int		size;

	// Viewport size
	cg.viewportX = cg.viewportY = 0;
	cg.viewportWidth = cgs.glconfig.vidWidth;
	cg.viewportHeight = cgs.glconfig.vidHeight;

	// Splitscreen viewports
	if (cg.numViewports == 2) {
		if (cg_splitviewVertical.integer) {
			cg.viewportWidth *= 0.5f;

			if (cg.viewport == 1) {
				cg.viewportX += cg.viewportWidth;
			}
		} else {
			cg.viewportHeight *= 0.5f;

			if (cg.viewport == 1) {
				cg.viewportY += cg.viewportHeight;
			}
		}
	}
	// if third view port should be half of the window instead of a quarter
	else if (cg.numViewports == 3 && !cg_splitviewThirdEqual.integer) {
		if (cg_splitviewVertical.integer) {
			if (cg.viewport == 2) {
				cg.viewportWidth *= 0.5f;
				cg.viewportX += cg.viewportWidth;
			} else {
				cg.viewportWidth *= 0.5f;
				cg.viewportHeight *= 0.5f;

				if (cg.viewport == 1) {
					cg.viewportY += cg.viewportHeight;
				}
			}
		} else {
			if (cg.viewport == 2) {
				cg.viewportHeight *= 0.5f;
				cg.viewportY += cg.viewportHeight;
			} else {
				cg.viewportWidth *= 0.5f;
				cg.viewportHeight *= 0.5f;

				if (cg.viewport == 1) {
					cg.viewportX += cg.viewportWidth;
				}
			}
		}
	}
	else if (cg.numViewports > 1 && cg.numViewports <= 4) {
		cg.viewportWidth *= 0.5f;
		cg.viewportHeight *= 0.5f;

		if (cg.viewport == 1 || cg.viewport == 3) {
			cg.viewportX += cg.viewportWidth;
		}

		if (cg.viewport == 2 || cg.viewport == 3) {
			cg.viewportY += cg.viewportHeight;
		}
	}

	// Viewport scale and offset
	cgs.screenXScaleStretch = cg.viewportWidth * (1.0/640.0);
	cgs.screenYScaleStretch = cg.viewportHeight * (1.0/480.0);
	if ( cg.viewportWidth * 480 > cg.viewportHeight * 640 ) {
		cgs.screenXScale = cgs.screenXScaleStretch;
		cgs.screenYScale = cgs.screenYScaleStretch;
		// wide screen
		cgs.screenXBias = 0.5 * ( cg.viewportWidth - ( cg.viewportHeight * (640.0/480.0) ) );
		cgs.screenXScale = cgs.screenYScale;
		// no narrow screen
		cgs.screenYBias = 0;
	} else {
		cgs.screenXScale = cgs.screenXScaleStretch;
		cgs.screenYScale = cgs.screenYScaleStretch;
		// narrow screen
		cgs.screenYBias = 0.5 * ( cg.viewportHeight - ( cg.viewportWidth * (480.0/640.0) ) );
		cgs.screenYScale = cgs.screenXScale;
		// no wide screen
		cgs.screenXBias = 0;
	}

	cgs.screenFakeWidth = cg.viewportWidth / cgs.screenXScale;

	// the intermission should always be full screen
	if ( !cg.cur_ps || cg.cur_ps->pm_type == PM_INTERMISSION ) {
		size = 100;
	} else {
		size = cg_viewsize.integer;
	}

	// Rendered window for drawing world
	cg.refdef.width = cg.viewportWidth*size/100;
	cg.refdef.width &= ~1;

	cg.refdef.height = cg.viewportHeight*size/100;
	cg.refdef.height &= ~1;

	cg.refdef.x = cg.viewportX + (cg.viewportWidth - cg.refdef.width)/2;
	cg.refdef.y = cg.viewportY + (cg.viewportHeight - cg.refdef.height)/2;
}

//==============================================================================

#ifdef IOQ3ZTM // NEW_CAM
#ifdef TURTLEARENA // FOV
#define DEFAULT_CAM_DIST 120
#else
#define DEFAULT_CAM_DIST 40
#endif

/*
=================
CG_CamUpdate

Update camera.
=================
*/
void CG_CamUpdate(void)
{
	qboolean thirdPerson;
	float angle, dist;
	float f = cg.frametime * 0.001f;
	qboolean angleReset, distReset;

	thirdPerson = (cg_thirdPerson[cg.cur_localPlayerNum].value != 0);
	angle = cg_thirdPersonAngle[cg.cur_localPlayerNum].value;
	dist = cg_thirdPersonRange[cg.cur_localPlayerNum].value;

	if (!cg.cur_lc->camDistance) {
		cg.cur_lc->camDistance = dist;
	}

	if (cg.cur_lc->cameraOrbit) {
		// cg_cameraOrbit holds angle to move in one second
		angle += cg.cur_lc->cameraOrbit * f;
	}

	if (cg.cur_lc->camReseting)
	{
		float speed = 200.0f * f;

		angleReset = (angle >= 360 - speed || angle <= speed);
		if (angleReset) {
			angle = 0;
			cg.cur_lc->camRotDir = 0;
		} else if (angle > 180) {
			angle += speed;
		} else if (angle > speed) {
			angle -= speed;
		}

		distReset = (dist >= DEFAULT_CAM_DIST - speed && dist <= DEFAULT_CAM_DIST + speed);
		if (distReset) {
			dist = DEFAULT_CAM_DIST;
			cg.cur_lc->camZoomDir = 0;
		} else if (dist < DEFAULT_CAM_DIST) {
			dist += speed;
		} else if (dist > DEFAULT_CAM_DIST) {
			dist -= speed;
		}
	}
	else
	{
		float speed1x = 200.0f * f;
		float speed2x = speed1x * 2.0f;

		angleReset = distReset = qfalse;

		// Spin
		if (cg.cur_lc->camLeft)
			cg.cur_lc->camRotDir += speed2x;
		else if (cg.cur_lc->camRotDir >= speed1x)
			cg.cur_lc->camRotDir -= speed1x;

		if (cg.cur_lc->camRight)
			cg.cur_lc->camRotDir -= speed2x;
		else if (cg.cur_lc->camRotDir <= -speed1x)
			cg.cur_lc->camRotDir += speed1x;

		if (!cg.cur_lc->camLeft && !cg.cur_lc->camRight &&
				cg.cur_lc->camRotDir >= -speed2x && cg.cur_lc->camRotDir <= speed2x)
			cg.cur_lc->camRotDir = 0;

		if (cg.cur_lc->camRotDir > 100)
			cg.cur_lc->camRotDir = 100;
		else if (cg.cur_lc->camRotDir < -100)
			cg.cur_lc->camRotDir = -100;

		angle += cg.cur_lc->camRotDir * f;

		// Zoom
		if (cg.cur_lc->camZoomOut)
			cg.cur_lc->camZoomDir += speed2x;
		else if (cg.cur_lc->camZoomDir >= speed1x)
			cg.cur_lc->camZoomDir -= speed1x;

		if (cg.cur_lc->camZoomIn)
			cg.cur_lc->camZoomDir -= speed2x;
		else if (cg.cur_lc->camZoomDir <= -speed1x)
			cg.cur_lc->camZoomDir += speed1x;

		if (!cg.cur_lc->camZoomOut && !cg.cur_lc->camZoomIn &&
				cg.cur_lc->camZoomDir >= -speed2x && cg.cur_lc->camZoomDir <= speed2x)
			cg.cur_lc->camZoomDir = 0;

		if (cg.cur_lc->camZoomDir > 100)
			cg.cur_lc->camZoomDir = 100;
		else if (cg.cur_lc->camZoomDir < -100)
			cg.cur_lc->camZoomDir = -100;

		dist += cg.cur_lc->camZoomDir * f;
	}

	if (angle > 360)
		angle -= 360;
	else if (angle < 0)
		angle += 360;

	if (dist > 500)
		dist = 500;
	else if (dist < 10)
		dist = 10;

	// Update the cvars...
	if (cg_thirdPersonAngle[cg.cur_localPlayerNum].value != angle) {
		trap_Cvar_Set(Com_LocalPlayerCvarName(cg.cur_localPlayerNum, "cg_thirdPersonAngle"), va("%f", angle));
	}
	if (cg_thirdPersonRange[cg.cur_localPlayerNum].value != dist) {
		trap_Cvar_Set(Com_LocalPlayerCvarName(cg.cur_localPlayerNum, "cg_thirdPersonRange"), va("%f", dist));
	}

	if (thirdPerson)
	{
		float speed1x = 200.0f * f;
		float speed2x = speed1x * 2.0f;

		// Third person range was made shorter, zoom in
		if (cg.cur_lc->camDistance > dist + speed2x)
			cg.cur_lc->camDistance -= speed2x;
		else if (cg.cur_lc->camDistance > dist)
			cg.cur_lc->camDistance = dist;
		// Zoom back out
		else if (cg.cur_lc->camDistance < dist)
		{
			// NiGHTS: Journey of Dreams Visitor style camera distance.
			//  When next to a wall the camera is closer (like Quake3),
			//  but only zooms back out when the player is moving

			// If player is moving on xy zoom out a little
			if (cg.xyspeed != 0 || cg.cur_lc->camReseting || cg.cur_lc->camZoomDir > 0)
			{
				if (cg.cur_lc->camDistance < dist - speed1x)
					cg.cur_lc->camDistance += speed1x;
				else
					cg.cur_lc->camDistance = dist;
			}
		}

		distReset = (cg.cur_lc->camDistance >= DEFAULT_CAM_DIST - speed1x && cg.cur_lc->camDistance <= DEFAULT_CAM_DIST + speed1x);
	}

	if (angleReset && distReset) {
		cg.cur_lc->camReseting = qfalse;
	}
}
#endif

// this causes a compiler bug on mac MrC compiler
void CG_StepOffset( vec3_t vieworg ) {
	int		timeDelta;

	// smooth out stair climbing
	timeDelta = cg.time - cg.cur_lc->stepTime;
	if ( timeDelta < STEP_TIME ) {
		vieworg[2] -= cg.cur_lc->stepChange
			* (STEP_TIME - timeDelta) / STEP_TIME;
	}
}

/*
===============
CG_OffsetThirdPersonView

===============
*/
#ifndef IOQ3ZTM // BETTER_THIRD_PERSON
#define	FOCUS_DISTANCE	512
#endif
static void CG_OffsetThirdPersonView( void ) {
	vec3_t		forward, right, up;
	vec3_t		view;
	vec3_t		focusAngles;
	trace_t		trace;
	static vec3_t	mins = { -5, -5, -5 };
	static vec3_t	maxs = { 5, 5, 5 };
	vec3_t		focusPoint;
	float		focusDist;
#ifdef IOQ3ZTM // BETTER_THIRD_PERSON
	float		distance;
#else
	float		forwardScale, sideScale;
	float		thirdPersonAngle, thirdPersonRange;
#endif

	cg.refdef.vieworg[2] += cg.cur_lc->predictedPlayerState.viewheight;
#ifdef IOQ3ZTM // BETTER_THIRD_PERSON
	distance = cg.cur_lc->camDistance;
#endif

#ifdef TA_PATHSYS
	// Move farther away when using a path
	if (cg.cur_ps->pathMode) {
		distance *= 1.3f;
	}
#endif

#ifdef TURTLEARENA // LOCKON
	if (cg.snap && cg.cur_ps->enemyEnt != ENTITYNUM_NONE) {
		int i;
		vec3_t dir;
		vec3_t targetAngles;
		vec3_t angDiff;
		float f;
		float time;
		qboolean completedMove;
		static float max_fraction = 1.0f;

		time = LOCKON_TIME;
		if (!cg.cur_lc->lockedOn)
			time *= max_fraction;

		// Setup fraction
		f = ( cg.time - cg.cur_lc->lockonTime ) / time;
		if (f > 1.0f) {
			f = 1.0f;
			completedMove = qtrue;
		} else {
			completedMove = qfalse;
		}

		if (cg.cur_lc->lockedOn) {
			max_fraction = f;
		} else {
			f = max_fraction - f * max_fraction;
		}

		if (cg.cur_lc->lockedOn || !completedMove ) {
			// Move camera back
			distance += distance*0.3f*f;

			// Get view angles to look at target
			VectorSubtract( cg.cur_ps->enemyOrigin, cg.cur_ps->origin, dir );
			vectoangles( dir, targetAngles );

			if ( completedMove ) {
				for (i = 0; i < 3; i++) {
					targetAngles[i] = AngleNormalize180(targetAngles[i]);
				}

				VectorCopy(targetAngles, cg.refdefViewAngles);
			} else {
				// get diff of normal and target view angles.
				VectorSubtract(targetAngles, cg.refdefViewAngles, angDiff);

				// Stop camera from spinning 360 degrees.
				for (i = 0; i < 3; i++)
				{
					angDiff[i] = AngleNormalize180(angDiff[i]);
				}

				// Scale the difference of the angles
				VectorScale(angDiff, f, angDiff);

				// Add the scaled angle difference
				VectorAdd(cg.refdefViewAngles, angDiff, cg.refdefViewAngles);
			}
		}
	}
#endif
	VectorCopy( cg.refdefViewAngles, focusAngles );

	if ( cg.cur_lc->cameraOrbit ) {
#ifdef IOQ3ZTM // BETTER_THIRD_PERSON
		focusAngles[YAW] -= cg_thirdPersonAngle[cg.cur_localPlayerNum].value;
		cg.refdefViewAngles[YAW] -= cg_thirdPersonAngle[cg.cur_localPlayerNum].value;
#else
		thirdPersonAngle = cg.cur_lc->cameraOrbitAngle;
		thirdPersonRange = cg.cur_lc->cameraOrbitRange;
#endif

		// make camera orbit horizontal
		focusAngles[PITCH] = 0;
		cg.refdefViewAngles[PITCH] = 0;
	} else {
#ifndef IOQ3ZTM // BETTER_THIRD_PERSON
		thirdPersonAngle = cg_thirdPersonAngle[cg.cur_localPlayerNum].value;
		thirdPersonRange = cg_thirdPersonRange[cg.cur_localPlayerNum].value;
#endif

		// if dead, look at killer
		if ( cg.cur_lc->predictedPlayerState.stats[STAT_HEALTH] <= 0 ) {
			focusAngles[YAW] = cg.cur_lc->predictedPlayerState.stats[STAT_DEAD_YAW];
			cg.refdefViewAngles[YAW] = cg.cur_lc->predictedPlayerState.stats[STAT_DEAD_YAW];
		}
#ifdef TA_PATHSYS // 2DMODE
		else if (cg.cur_ps->pathMode == PATHMODE_SIDE || cg.cur_ps->pathMode == PATHMODE_BACK)
		{
			focusAngles[YAW] = cg.cur_lc->predictedPlayerState.stats[STAT_DEAD_YAW];
			cg.refdefViewAngles[YAW] = cg.cur_lc->predictedPlayerState.stats[STAT_DEAD_YAW];
		}
#endif
#ifdef IOQ3ZTM // BETTER_THIRD_PERSON
		else
		{
			focusAngles[YAW] -= cg_thirdPersonAngle[cg.cur_localPlayerNum].value;
			cg.refdefViewAngles[YAW] -= cg_thirdPersonAngle[cg.cur_localPlayerNum].value;
		}
#endif
	}

	if ( focusAngles[PITCH] > 45 ) {
		focusAngles[PITCH] = 45;		// don't go too far overhead
	}

	AngleVectors( focusAngles, forward, NULL, NULL );

	if ( cg_thirdPersonSmooth[cg.cur_localPlayerNum].integer ) {
		CG_StepOffset( cg.refdef.vieworg );
	}

#ifdef IOQ3ZTM // BETTER_THIRD_PERSON
	// Focus on player
	VectorMA( cg.refdef.vieworg, distance, forward, focusPoint );
#else
	VectorMA( cg.refdef.vieworg, FOCUS_DISTANCE, forward, focusPoint );
#endif

	VectorCopy( cg.refdef.vieworg, view );

	view[2] += cg_thirdPersonHeight[cg.cur_localPlayerNum].value;

	cg.refdefViewAngles[PITCH] *= 0.5;

	AngleVectors( cg.refdefViewAngles, forward, right, up );

#ifdef IOQ3ZTM // BETTER_THIRD_PERSON
	// Move camera behind player
	VectorMA( view, -distance, forward, view );
#else
	forwardScale = cos( thirdPersonAngle / 180 * M_PI );
	sideScale = sin( thirdPersonAngle / 180 * M_PI );
	VectorMA( view, -thirdPersonRange * forwardScale, forward, view );
	VectorMA( view, -thirdPersonRange * sideScale, right, view );
#endif

	// trace a ray from the origin to the viewpoint to make sure the view isn't
	// in a solid block.  Use a 10 by 10 block to prevent the view from near clipping anything

	if (!cg_cameraMode.integer) {
		CG_Trace( &trace, cg.refdef.vieworg, mins, maxs, view, cg.cur_lc->predictedPlayerState.playerNum, MASK_SOLID );

		if ( trace.fraction != 1.0 ) {
			VectorCopy( trace.endpos, view );
#ifdef IOQ3ZTM // NEW_CAM
			cg.cur_lc->camDistance = cg.cur_lc->camDistance * trace.fraction;
			if (cg.cur_lc->camDistance < 10) {
				cg.cur_lc->camDistance = 10;
			}
#else
			view[2] += (1.0 - trace.fraction) * 32;
			// try another trace to this position, because a tunnel may have the ceiling
			// close enough that this is poking out

			CG_Trace( &trace, cg.refdef.vieworg, mins, maxs, view, cg.cur_lc->predictedPlayerState.playerNum, MASK_SOLID );
			VectorCopy( trace.endpos, view );
#endif
		}
	}


	VectorCopy( view, cg.refdef.vieworg );

	// select pitch to look at focus point from vieword
	VectorSubtract( focusPoint, cg.refdef.vieworg, focusPoint );
	focusDist = sqrt( focusPoint[0] * focusPoint[0] + focusPoint[1] * focusPoint[1] );
	if ( focusDist < 1 ) {
		focusDist = 1;	// should never happen
	}
	cg.refdefViewAngles[PITCH] = -180 / M_PI * atan2( focusPoint[2], focusDist );
#ifndef IOQ3ZTM // BETTER_THIRD_PERSON
	cg.refdefViewAngles[YAW] -= thirdPersonAngle;
#endif
}

/*
===============
CG_OffsetFirstPersonView

===============
*/
static void CG_OffsetFirstPersonView( void ) {
	float			*origin;
	float			*angles;
	float			bob;
	float			ratio;
	float			delta;
	float			speed;
	float			f;
	vec3_t			predictedVelocity;
	int				timeDelta;
	
	if ( cg.cur_ps->pm_type == PM_INTERMISSION ) {
		return;
	}

	VectorCopy( cg.refdef.vieworg, cg.firstPersonViewOrg );
	VectorCopy( cg.refdefViewAngles, cg.firstPersonViewAngles );

	origin = cg.firstPersonViewOrg;
	angles = cg.firstPersonViewAngles;

	// if dead, fix the angle and don't add any kick
	if ( cg.cur_ps->stats[STAT_HEALTH] <= 0 ) {
		angles[ROLL] = 40;
		angles[PITCH] = -15;
		angles[YAW] = cg.cur_ps->stats[STAT_DEAD_YAW];
		origin[2] += cg.cur_lc->predictedPlayerState.viewheight;
		return;
	}

	// add angles based on damage kick
	if ( cg.cur_lc->damageTime && !cg.cur_lc->renderingThirdPerson && cg_viewkick.integer ) {
		ratio = cg.time - cg.cur_lc->damageTime;
		if ( ratio < DAMAGE_DEFLECT_TIME ) {
			ratio /= DAMAGE_DEFLECT_TIME;
			angles[PITCH] += ratio * cg.cur_lc->v_dmg_pitch;
			angles[ROLL] += ratio * cg.cur_lc->v_dmg_roll;
		} else {
			ratio = 1.0 - ( ratio - DAMAGE_DEFLECT_TIME ) / DAMAGE_RETURN_TIME;
			if ( ratio > 0 ) {
				angles[PITCH] += ratio * cg.cur_lc->v_dmg_pitch;
				angles[ROLL] += ratio * cg.cur_lc->v_dmg_roll;
			}
		}
	}

	// add pitch based on fall kick
#if 0
	ratio = ( cg.time - cg.cur_lc->landTime) / FALL_TIME;
	if (ratio < 0)
		ratio = 0;
	angles[PITCH] += ratio * cg.fall_value;
#endif

	if ( !cg.cur_lc->renderingThirdPerson && cg_viewbob.integer ) {
		// add angles based on velocity
		VectorCopy( cg.cur_lc->predictedPlayerState.velocity, predictedVelocity );

		delta = DotProduct ( predictedVelocity, cg.refdef.viewaxis[0]);
		angles[PITCH] += delta * cg_runpitch.value;
	
		delta = DotProduct ( predictedVelocity, cg.refdef.viewaxis[1]);
		angles[ROLL] -= delta * cg_runroll.value;

		// add angles based on bob

		// make sure the bob is visible even at low speeds
		speed = cg.xyspeed > 200 ? cg.xyspeed : 200;

		delta = cg.bobfracsin * cg_bobpitch.value * speed;
		if (cg.cur_lc->predictedPlayerState.pm_flags & PMF_DUCKED)
			delta *= 3;		// crouching
		angles[PITCH] += delta;
		delta = cg.bobfracsin * cg_bobroll.value * speed;
		if (cg.cur_lc->predictedPlayerState.pm_flags & PMF_DUCKED)
			delta *= 3;		// crouching accentuates roll
		if (cg.bobcycle & 1)
			delta = -delta;
		angles[ROLL] += delta;

		// add bob height
		bob = cg.bobfracsin * cg.xyspeed * cg_bobup.value;
		if (bob > 6) {
			bob = 6;
		}

		origin[2] += bob;
	}

//===================================

	// add view height
	origin[2] += cg.cur_lc->predictedPlayerState.viewheight;

	// smooth out duck height changes
	timeDelta = cg.time - cg.cur_lc->duckTime;
	if ( timeDelta < DUCK_TIME) {
		origin[2] -= cg.cur_lc->duckChange 
			* (DUCK_TIME - timeDelta) / DUCK_TIME;
	}


	// add fall height
	delta = cg.time - cg.cur_lc->landTime;
	if ( delta < LAND_DEFLECT_TIME ) {
		f = delta / LAND_DEFLECT_TIME;
		origin[2] += cg.cur_lc->landChange * f;
	} else if ( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME ) {
		delta -= LAND_DEFLECT_TIME;
		f = 1.0 - ( delta / LAND_RETURN_TIME );
		origin[2] += cg.cur_lc->landChange * f;
	}

	// add step offset
	CG_StepOffset( origin );

	// pivot the eye based on a neck length
#if 0
	{
#define	NECK_LENGTH		8
	vec3_t			forward, up;
 
	origin[2] -= NECK_LENGTH;
	AngleVectors( cg.refdefViewAngles, forward, NULL, up );
	VectorMA( origin, 3, forward, origin );
	VectorMA( origin, NECK_LENGTH, up, origin );
	}
#endif
}

#ifdef TA_PATHSYS // 2DMODE
/*
===============
CG_OffsetMultiple2dModeView

Based on SRB2FLAME's P_SSBSSCamera
ZTM: TODO: Support focusing on all players if everyone is qtrue.
===============
*/
static void CG_OffsetMultiple2dModeView(qboolean everyone) {
	const int		ORG_X = 0;
	const int		ORG_Y = 1;
	const int		ORG_Z = 2;
	static vec3_t	org = {0,0,0}; // camera current origin
	static vec3_t	mins = { -4, -4, -4 };
	static vec3_t	maxs = { 4, 4, 4 };
	vec3_t			center = {0, 0, 0};
	size_t			i, numPlayers;
	float			farthestplayerX, farthestplayerZ;
	float			dist;
	float			newDist;
	float			f;
	float			maxMove;
	vec3_t			focusAngles = {0, 0, 0};
	int				numYaw;
	trace_t			trace;
#if 0
	mobj_t *cammin = P_GetMobjForType(MT_CAMMIN);
	mobj_t *cammax = P_GetMobjForType(MT_CAMMAX);
#endif

	f = ((float)cg.frametime) / 1000;
	maxMove = 340.0f * f;

	// Find the center of all of the players and use that as the focus point.
	// Calculate central X, Y, and Z
	for (i = 0, numPlayers = 0; i < CG_MaxSplitView(); i++) {
		if ( cg.localPlayers[i].playerNum == -1 ) {
			continue;
		}
		if (cg.snap->pss[i].persistant[PERS_TEAM] == TEAM_SPECTATOR || cg.snap->pss[i].stats[STAT_HEALTH] <= 0) {
			continue;
		}

		numPlayers++;

		center[ORG_X] += cg.snap->pss[i].origin[ORG_X];
		center[ORG_Y] += cg.snap->pss[i].origin[ORG_Y];
		center[ORG_Z] += cg.snap->pss[i].origin[ORG_Z];

		if (cg.cur_lc->predictedPlayerState.stats[STAT_HEALTH] > 0
#ifdef TA_PATHSYS // 2DMODE
			&& (cg.cur_ps->pathMode == PATHMODE_SIDE || cg.cur_ps->pathMode == PATHMODE_BACK)
#endif
			)
		{
			focusAngles[YAW] += cg.cur_lc->predictedPlayerState.stats[STAT_DEAD_YAW];
			numYaw++;
		}
	}

	// Don't move the camera when all of the players are dead.
	if (numPlayers == 0) {
		return;
	}

	center[ORG_X] /= numPlayers;
	center[ORG_Y] /= numPlayers;
	center[ORG_Z] /= numPlayers;
	focusAngles[YAW] /= numPlayers;

	// Move the camera's X
	dist = center[ORG_X]-org[ORG_X];

	if (dist > 0)
	{
		if (dist > maxMove)
			org[ORG_X] += maxMove;
		else
			org[ORG_X] += dist;
	}
	else if (dist < 0)
	{
		if (dist < -maxMove)
			org[ORG_X] += -maxMove;
		else
			org[ORG_X] += dist;
	}

#if 0
	if (cammin && org[ORG_X] < cammin->x)
		org[ORG_X] = cammin->x;
	else if (cammax && org[ORG_X] > cammax->x)
		org[ORG_X] = cammax->x;
#endif

	// Move the camera's Z
	//centerZ += cv_cam_height.value;
	dist = center[ORG_Z]-org[ORG_Z];

	if (dist > 0)
	{
		if (dist > maxMove)
			org[ORG_Z] += maxMove;
		else
			org[ORG_Z] += dist;
	}
	else if (dist < 0)
	{
		if (dist < -maxMove)
			org[ORG_Z] += -maxMove;
		else
			org[ORG_Z] += dist;
	}

#if 0
	if (cammin && org[ORG_Z] < cammin->z) {
		org[ORG_Z] = cammin->z;
	} else if (cammax && org[ORG_Z] > cammax->z) {
		org[ORG_Z] = cammax->z;
	}
#endif

	// Calculate farthest players
	farthestplayerX = farthestplayerZ = 0;
	for (i = 0; i < CG_MaxSplitView(); i++) {
		if ( cg.localPlayers[i].playerNum == -1 ) {
			continue;
		}
		if (cg.snap->pss[i].persistant[PERS_TEAM] == TEAM_SPECTATOR || cg.snap->pss[i].stats[STAT_HEALTH] <= 0)
			continue;

		dist = abs(cg.snap->pss[i].origin[ORG_X] - center[ORG_X]);

		if (dist > farthestplayerX)
			farthestplayerX = dist;

		dist = abs(cg.snap->pss[i].origin[ORG_Z] - center[ORG_Z]);

		if (dist > farthestplayerZ)
			farthestplayerZ = dist;
	}

	// Subtract a little so the player isn't right on the edge of the camera.
	newDist = center[ORG_Y] - (farthestplayerX + farthestplayerZ + 256);

	// Move the camera's Y
	dist = newDist - org[ORG_Y];

	if (dist > 0)
	{
		if (dist > maxMove)
			org[ORG_Y] += maxMove;
		else
			org[ORG_Y] += dist;
	}
	else if (dist < 0)
	{
		if (dist < -maxMove)
			org[ORG_Y] += -maxMove;
		else
			org[ORG_Y] += dist;
	}

#if 0
	// This may seem backward but its not.
	if (cammin && org[ORG_Y] > cammin->y)
	{
		org[ORG_Y] = cammin->y;
	}
	else if (cammax && org[ORG_Y] < cammax->y)
	{
		org[ORG_Y] = cammax->y;
	}
#endif

	// Position camera
	VectorCopy(focusAngles, cg.refdefViewAngles);

	// trace a ray from the origin to the viewpoint to make sure the view isn't
	// in a solid block.  Use an 8 by 8 block to prevent the view from near clipping anything
	if (!cg_cameraMode.integer) {
		VectorCopy(center, cg.refdef.vieworg);

		CG_Trace( &trace, cg.refdef.vieworg, mins, maxs, org, -1, MASK_SOLID );

		if ( trace.fraction != 1.0 ) {
			VectorCopy( trace.endpos, org );
			org[2] += (1.0 - trace.fraction) * 32;
			// try another trace to this position, because a tunnel may have the ceiling
			// close enogh that this is poking out

			CG_Trace( &trace, cg.refdef.vieworg, mins, maxs, org, -1, MASK_SOLID );
			VectorCopy( trace.endpos, org );
		}
	}

	VectorCopy( org, cg.refdef.vieworg );
}
#endif

//======================================================================

#ifndef TURTLEARENA // NOZOOM
void CG_ZoomDown_f( int localPlayerNum ) {
	localPlayer_t *player = &cg.localPlayers[localPlayerNum];

	if ( player->zoomed ) {
		return;
	}

	player->zoomed = qtrue;
	player->zoomTime = cg.time;
#ifdef IOQ3ZTM // LETTERBOX
	CG_ToggleLetterbox(qtrue, qfalse);
#endif
}

void CG_ZoomUp_f( int localPlayerNum ) {
	localPlayer_t *player = &cg.localPlayers[localPlayerNum];

	if ( !player->zoomed ) {
		return;
	}

	player->zoomed = qfalse;
	player->zoomTime = cg.time;
#ifdef IOQ3ZTM // LETTERBOX
	CG_ToggleLetterbox(qfalse, qfalse);
#endif
}
#endif


/*
====================
CG_CalcFov

Fixed fov at intermissions, otherwise account for fov variable and zooms.
====================
*/
#define	WAVE_AMPLITUDE	1
#define	WAVE_FREQUENCY	0.4

static void CG_CalcFov2( const refdef_t *refdef, float *input_fov, float *out_fov_x, float *out_fov_y ) {
	float	x;
	float	phase;
	float	v;
	float	fov_x, fov_y;
#ifndef TURTLEARENA // NOZOOM
	float	zoomFov;
	float	f;
#endif

	if ( cg.cur_lc->predictedPlayerState.pm_type == PM_INTERMISSION ) {
		// if in intermission, use a fixed value
#ifdef TURTLEARENA // FOV
		fov_x = 70;
		*input_fov = 70;
#else
		fov_x = 90;
		*input_fov = 90;
#endif
	} else {
		// user selectable
		if ( cgs.dmflags & DF_FIXED_FOV ) {
			// dmflag to prevent wide fov for all players
#ifdef TURTLEARENA // FOV
			fov_x = 70;
			*input_fov = 70;
#else
			fov_x = 90;
			*input_fov = 90;
#endif
		} else {
			fov_x = *input_fov;
		}

#ifndef TURTLEARENA // NOZOOM
		// account for zooms
		zoomFov = cg_zoomFov.value;

		if ( cg.cur_lc->zoomed ) {
			f = ( cg.time - cg.cur_lc->zoomTime ) / (float)ZOOM_TIME;
			if ( f > 1.0 ) {
				fov_x = zoomFov;
			} else {
				fov_x = fov_x + f * ( zoomFov - fov_x );
			}
		} else {
			f = ( cg.time - cg.cur_lc->zoomTime ) / (float)ZOOM_TIME;
			if ( f <= 1.0 ) {
				fov_x = zoomFov + f * ( fov_x - zoomFov );
			}
		}
#endif
	}

	if ( cg_fovAspectAdjust.integer ) {
		// Based on LordHavoc's code for Darkplaces
		// http://www.quakeworld.nu/forum/topic/53/what-does-your-qw-look-like/page/30
		const float baseAspect = 0.75f; // 3/4
		const float aspect = (float)refdef->width/(float)refdef->height;
		const float desiredFov = fov_x;

		fov_x = atan( tan( desiredFov*M_PI / 360.0f ) * baseAspect*aspect )*360.0f / M_PI;
	}

	x = refdef->width / tan( fov_x / 360 * M_PI );
	fov_y = atan2( refdef->height, x );
	fov_y = fov_y * 360 / M_PI;

	// warp if underwater
	if ( refdef->rdflags & RDF_UNDERWATER ) {
		phase = cg.time / 1000.0 * WAVE_FREQUENCY * M_PI * 2;
		v = WAVE_AMPLITUDE * sin( phase );
		fov_x += v;
		fov_y -= v;
	}

	*out_fov_x = fov_x;
	*out_fov_y = fov_y;
}

static int CG_CalcFov( void ) {
	float fov;
	int contents;

	// check if underwater
	contents = CG_PointContents( cg.refdef.vieworg, -1 );
	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ){
		cg.refdef.rdflags |= RDF_UNDERWATER;
	}
	else {
		cg.refdef.rdflags &= ~RDF_UNDERWATER;
	}

	// set world fov
	fov = cg_fov.value;
	CG_CalcFov2( &cg.refdef, &fov, &cg.refdef.fov_x, &cg.refdef.fov_y );

	// set view weapon fov
	cg.viewWeaponFov = cg_weaponFov.value ? cg_weaponFov.value : cg_fov.value;
	CG_CalcFov2( &cg.refdef, &cg.viewWeaponFov, &cg.refdef.weapon_fov_x, &cg.refdef.weapon_fov_y );

#ifndef TURTLEARENA // NOZOOM
	if ( !cg.cur_lc->zoomed ) {
		cg.cur_lc->zoomSensitivity = 1;
	} else {
		cg.cur_lc->zoomSensitivity = cg.refdef.fov_y / 75.0;
	}
#endif

	return (cg.refdef.rdflags & RDF_UNDERWATER);
}


/*
==============
CG_DrawSkyBoxPortal
==============
*/
void CG_DrawSkyBoxPortal( void ) {
	refdef_t backuprefdef;

	if ( !cg_skybox.integer || !cg.hasSkyPortal ) {
		return;
	}

	backuprefdef = cg.refdef;

	VectorCopy( cg.skyPortalOrigin, cg.refdef.vieworg );

	if ( cg.skyPortalFogDepthForOpaque > 0 ) {
		cg.refdef.fogType = FT_LINEAR;
		cg.refdef.fogDensity = 1.0f;
		cg.refdef.fogDepthForOpaque = cg.skyPortalFogDepthForOpaque;
		cg.refdef.farClip = 0;
	}
	VectorCopy( cg.skyPortalFogColor, cg.refdef.fogColor );

	cg.refdef.skyAlpha = cg.skyAlpha;

	cg.refdef.time = cg.time;

	if ( cg_skybox.integer == 2 ) {
		cg.refdef.rdflags |= RDF_ONLYSKY;
	}

	// draw the skybox
	trap_R_RenderScene( &cg.refdef );

	cg.refdef = backuprefdef;
	cg.refdef.rdflags |= RDF_NOSKY;
}


#ifndef NOBLOOD
/*
===============
CG_DamageBlendBlob

===============
*/
static void CG_DamageBlendBlob( void ) {
	int			t;
	int			maxTime;
	refEntity_t		ent;

	if (!cg_blood.integer) {
		return;
	}

	if ( !cgs.media.viewBloodShader ) {
		return;
	}

	if ( !cg.cur_lc->damageValue ) {
		return;
	}

	//if (cg.cameraMode) {
	//	return;
	//}

	maxTime = DAMAGE_TIME;
	t = cg.time - cg.cur_lc->damageTime;
	if ( t <= 0 || t >= maxTime ) {
		return;
	}


	memset( &ent, 0, sizeof( ent ) );
	ent.reType = RT_SPRITE;
	ent.renderfx = RF_NO_MIRROR;

	VectorMA( cg.refdef.vieworg, 8, cg.refdef.viewaxis[0], ent.origin );
	VectorMA( ent.origin, cg.cur_lc->damageX * -8, cg.refdef.viewaxis[1], ent.origin );
	VectorMA( ent.origin, cg.cur_lc->damageY * 8, cg.refdef.viewaxis[2], ent.origin );

	ent.radius = cg.cur_lc->damageValue * 3;
	ent.customShader = cgs.media.viewBloodShader;
	ent.shaderRGBA[0] = 255;
	ent.shaderRGBA[1] = 255;
	ent.shaderRGBA[2] = 255;
	ent.shaderRGBA[3] = 200 * ( 1.0 - ((float)t / maxTime) );
	trap_R_AddRefEntityToScene( &ent );
}
#endif


/*
===============
CG_CalcViewValues

Sets cg.refdef view values
===============
*/
static int CG_CalcViewValues( void ) {
	playerState_t	*ps;

	memset( &cg.refdef, 0, sizeof( cg.refdef ) );

	// strings for in game rendering
	// Q_strncpyz( cg.refdef.text[0], "Park Ranger", sizeof(cg.refdef.text[0]) );
	// Q_strncpyz( cg.refdef.text[1], "19", sizeof(cg.refdef.text[1]) );

	// calculate size of 3D view
	CG_CalcVrect();

	ps = &cg.cur_lc->predictedPlayerState;
/*
	if (cg.cameraMode) {
		vec3_t origin, angles;
		if (trap_getCameraInfo(cg.time, &origin, &angles)) {
			VectorCopy(origin, cg.refdef.vieworg);
			angles[ROLL] = 0;
			VectorCopy(angles, cg.refdefViewAngles);
			AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );
			return CG_CalcFov();
		} else {
			cg.cameraMode = qfalse;
		}
	}
*/
	// intermission view
	if ( ps->pm_type == PM_INTERMISSION
#ifdef TA_SP
		&& cgs.gametype != GT_SINGLE_PLAYER
#endif
	) {
		VectorCopy( ps->origin, cg.refdef.vieworg );
		VectorCopy( ps->viewangles, cg.refdefViewAngles );
		AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );
		return CG_CalcFov();
	}

	cg.bobcycle = ( ps->bobCycle & 128 ) >> 7;
	cg.bobfracsin = fabs( sin( ( ps->bobCycle & 127 ) / 127.0 * M_PI ) );
	cg.xyspeed = sqrt( ps->velocity[0] * ps->velocity[0] +
		ps->velocity[1] * ps->velocity[1] );


	VectorCopy( ps->origin, cg.refdef.vieworg );
	VectorCopy( ps->viewangles, cg.refdefViewAngles );

#ifdef IOQ3ZTM // NEW_CAM
	// Update player's camera
	CG_CamUpdate();
#else
	if ( cg.cur_lc->cameraOrbit ) {
		cg.cur_lc->cameraOrbitAngle += cg.cur_lc->cameraOrbit * cg.frametime * 0.001f;
	}
#endif

	// add error decay
	if ( cg_errorDecay.value > 0 ) {
		int		t;
		float	f;

		t = cg.time - cg.cur_lc->predictedErrorTime;
		f = ( cg_errorDecay.value - t ) / cg_errorDecay.value;
		if ( f > 0 && f < 1 ) {
			VectorMA( cg.refdef.vieworg, f, cg.cur_lc->predictedError, cg.refdef.vieworg );
		} else {
			cg.cur_lc->predictedErrorTime = 0;
		}
	}

	// offset for local bobbing and kicks
	CG_OffsetFirstPersonView();

#ifdef TA_PATHSYS // 2DMODE
	if (cg.singleCamera) {
		if (cg_2dmode.integer == 2) {
			// Focus on all clients
			CG_OffsetMultiple2dModeView(qtrue);
		} else if (cg_2dmode.integer) {
			// Focus on all local clients
			CG_OffsetMultiple2dModeView(qfalse);
		}
	} else
#endif
	if ( cg.cur_lc->renderingThirdPerson ) {
		// back away from character
		CG_OffsetThirdPersonView();
	} else {
		VectorCopy( cg.firstPersonViewOrg, cg.refdef.vieworg );
		VectorCopy( cg.firstPersonViewAngles, cg.refdefViewAngles );
	}

	// position eye relative to origin
	AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );
	AnglesToAxis( cg.firstPersonViewAngles, cg.firstPersonViewAxis );

	if ( cg.cur_lc->hyperspace ) {
		cg.refdef.rdflags |= RDF_NOWORLDMODEL | RDF_HYPERSPACE;
	}

	cg.cur_lc->lastViewPos[0] = cg.refdef.vieworg[0];
	cg.cur_lc->lastViewPos[1] = cg.refdef.vieworg[1];
	cg.cur_lc->lastViewPos[2] = cg.refdef.vieworg[2];

	cg.cur_lc->lastViewAngles[YAW] = cg.refdefViewAngles[YAW];
	cg.cur_lc->lastViewAngles[PITCH] = cg.refdefViewAngles[PITCH];
	cg.cur_lc->lastViewAngles[ROLL] = cg.refdefViewAngles[ROLL];

	// field of view
	return CG_CalcFov();
}


/*
=====================
CG_PowerupTimerSounds
=====================
*/
static void CG_PowerupTimerSounds( void ) {
	int		i;
	int		t;

	// powerup timers going away
	for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {
		t = cg.cur_ps->powerups[i];
#ifdef TURTLEARENA // POWERS
		if (i == PW_FLASHING)
		{
			continue;
		}
#endif
		if ( t <= cg.time ) {
			continue;
		}
		if ( t - cg.time >= POWERUP_BLINKS * POWERUP_BLINK_TIME ) {
			continue;
		}
		if ( ( t - cg.time ) / POWERUP_BLINK_TIME != ( t - cg.oldTime ) / POWERUP_BLINK_TIME ) {
			trap_S_StartSound( NULL, cg.cur_ps->playerNum, CHAN_ITEM, cgs.media.wearOffSound );
		}
	}
}

/*
=====================
CG_AddBufferedSound
=====================
*/
void CG_AddBufferedSound( sfxHandle_t sfx ) {
	if ( !sfx )
		return;
	cg.soundBuffer[cg.soundBufferIn] = sfx;
	cg.soundBufferIn = (cg.soundBufferIn + 1) % MAX_SOUNDBUFFER;
	if (cg.soundBufferIn == cg.soundBufferOut) {
		cg.soundBufferOut++;
	}
}

/*
=====================
CG_PlayBufferedSounds
=====================
*/
static void CG_PlayBufferedSounds( void ) {
	if ( cg.soundTime < cg.time ) {
		if (cg.soundBufferOut != cg.soundBufferIn && cg.soundBuffer[cg.soundBufferOut]) {
			trap_S_StartLocalSound(cg.soundBuffer[cg.soundBufferOut], CHAN_ANNOUNCER);
			cg.soundBuffer[cg.soundBufferOut] = 0;
			cg.soundBufferOut = (cg.soundBufferOut + 1) % MAX_SOUNDBUFFER;
			cg.soundTime = cg.time + 750;
		}
	}
}

#ifdef TA_MISC // COMIC_ANNOUNCER
const char *cg_announcementMessages[ANNOUNCE_MAX] = {
	"Prepare your selfs.",	// ANNOUNCE_PREPAREYOURSELFS
	"Prepare your team.",	// ANNOUNCE_PREPAREYOURTEAM

	"Voting has begun.",	// ANNOUNCE_VOTINGBEGUN
	"Vote passed.",			// ANNOUNCE_VOTEPASS
	"Vote failed.",			// ANNOUNCE_VOTEFAIL

	"You have taken the lead.",	// ANNOUNCE_YOUHAVETAKENTHELEAD
	"Your tied for the lead.",	// ANNOUNCE_YOURTIEDFORTHELEAD
	"You lost the lead.",		// ANNOUNCE_YOULOSTTHELEAD

	"Capture!",				// ANNOUNCE_CAPTURE
	"Assist.",				// ANNOUNCE_ASSIST
	"Defense.",				// ANNOUNCE_DEFENSE

	// TEAM/CTF/1FCTF/Overload
	"Red leads.",			// ANNOUNCE_REDLEADS
	"Blue Leads.",			// ANNOUNCE_BLUELEADS
	"Teams are tied.",		// ANNOUNCE_TEAMSTIED
	"You have the flag!",	// ANNOUNCE_YOUHAVETHEFLAG
	"Red scores.",			// ANNOUNCE_REDSCORES
	"Blue scores.",			// ANNOUNCE_BLUESCORES
	// ZTM: TODO: Fill in these announcer messages
	"(missing message.)",	// ANNOUNCE_TEAMCAPTURE
	"(missing message.)",	// ANNOUNCE_ENEMYCAPTURE
	"(missing message.)",	// ANNOUNCE_YOURFLAGRETURNED
	"(missing message.)",	// ANNOUNCE_ENEMYFLAGRETURNED
	"(missing message.)",	// ANNOUNCE_YOURTEAMHASTAKENTHEFLAG
	"(missing message.)",	// ANNOUNCE_THEENEMYHASTAKENTHEFLAG

	// CTF
	"(missing message.)",	// ANNOUNCE_REDFLAGRETURNED
	"(missing message.)",	// ANNOUNCE_BLUEFLAGRETURNED
	"(missing message.)",	// ANNOUNCE_ENEMYHASYOURFLAG
	"(missing message.)",	// ANNOUNCE_TEAMHASENEMYFLAG

	// One flag CTF
	"(missing message.)",	// ANNOUNCE_TEAMHASTHEFLAG
	"(missing message.)",	// ANNOUNCE_ENEMYHASTHEFLAG

	// Overload
	"(missing message.)"	// ANNOUNCE_BASEUNDERATTACK
};

/*
=================
CG_AddAnnouncement

Add announcement using ANNOUNCE_* enum.
=================
*/
void CG_AddAnnouncement(int announcement, int localPlayerNum)
{
	qhandle_t		sfx;
	qboolean		bufferedSfx;
	localPlayer_t	*player;

	if (announcement < 0 || announcement >= ANNOUNCE_MAX) {
		return;
	}

	bufferedSfx = qfalse;

	switch (announcement)
	{
		case ANNOUNCE_PREPAREYOURSELFS:
			sfx = cgs.media.countPrepareSound;
			break;
		case ANNOUNCE_PREPAREYOURTEAM:
			sfx = cgs.media.countPrepareTeamSound;
			break;
		case ANNOUNCE_VOTINGBEGUN:
			sfx = cgs.media.voteNow;
			break;
		case ANNOUNCE_VOTEPASS:
			sfx = cgs.media.votePassed;
			break;
		case ANNOUNCE_VOTEFAIL:
			sfx = cgs.media.voteFailed;
			break;
		case ANNOUNCE_YOUHAVETAKENTHELEAD:
			sfx = cgs.media.takenLeadSound;
			bufferedSfx = qtrue;
			break;
		case ANNOUNCE_YOURTIEDFORTHELEAD:
			sfx = cgs.media.tiedLeadSound;
			bufferedSfx = qtrue;
			break;
		case ANNOUNCE_YOULOSTTHELEAD:
			sfx = cgs.media.lostLeadSound;
			bufferedSfx = qtrue;
			break;
		case ANNOUNCE_CAPTURE:
			sfx = cgs.media.captureAwardSound;
			break;
		case ANNOUNCE_ASSIST:
			sfx = cgs.media.assistSound;
			break;
		case ANNOUNCE_DEFENSE:
			sfx = cgs.media.defendSound;
			break;
		case ANNOUNCE_REDLEADS:
			sfx = cgs.media.redLeadsSound;
			bufferedSfx = qtrue;
			break;
		case ANNOUNCE_BLUELEADS:
			sfx = cgs.media.blueLeadsSound;
			bufferedSfx = qtrue;
			break;
		case ANNOUNCE_TEAMSTIED:
			sfx = cgs.media.teamsTiedSound;
			bufferedSfx = qtrue;
			break;
		case ANNOUNCE_YOUHAVETHEFLAG:
			sfx = cgs.media.youHaveFlagSound;
			break;
		case ANNOUNCE_REDSCORES:
			sfx = cgs.media.redScoredSound;
			bufferedSfx = qtrue;
			break;
		case ANNOUNCE_BLUESCORES:
			sfx = cgs.media.blueScoredSound;
			bufferedSfx = qtrue;
			break;
		// ZTM: TODO: Fill in these announcer sound handles
		case ANNOUNCE_TEAMCAPTURE:
			sfx = 0;
			break;
		case ANNOUNCE_ENEMYCAPTURE:
			sfx = 0;
			break;
		case ANNOUNCE_YOURFLAGRETURNED:
			sfx = 0;
			break;
		case ANNOUNCE_ENEMYFLAGRETURNED:
			sfx = 0;
			break;
		case ANNOUNCE_YOURTEAMHASTAKENTHEFLAG:
			sfx = 0;
			break;
		case ANNOUNCE_THEENEMYHASTAKENTHEFLAG:
			sfx = 0;
			break;
		case ANNOUNCE_REDFLAGRETURNED:
			sfx = 0;
			break;
		case ANNOUNCE_BLUEFLAGRETURNED:
			sfx = 0;
			break;
		case ANNOUNCE_ENEMYHASYOURFLAG:
			sfx = 0;
			break;
		case ANNOUNCE_TEAMHASENEMYFLAG:
			sfx = 0;
			break;
		case ANNOUNCE_TEAMHASTHEFLAG:
			sfx = 0;
			break;
		case ANNOUNCE_ENEMYHASTHEFLAG:
			sfx = 0;
			break;
		case ANNOUNCE_BASEUNDERATTACK:
			sfx = 0;
			break;
		default:
			sfx = 0;
			break;
	}

	// Add for all local clients
	if (localPlayerNum < 0 || localPlayerNum >= MAX_SPLITVIEW) {
		int i;

		for (i = 0; i < MAX_SPLITVIEW; i++) {
			player = &cg.localPlayers[i];

			if (player->playerNum != -1) {
				CG_AddAnnouncementEx(player, sfx, bufferedSfx, cg_announcementMessages[announcement]);
			}
		}

		return;
	}

	player = &cg.localPlayers[localPlayerNum];

	CG_AddAnnouncementEx(player, sfx, bufferedSfx, cg_announcementMessages[announcement]);
}

/*
=================
CG_AddAnnouncementEx

Add announcement for local client
=================
*/
void CG_AddAnnouncementEx(localPlayer_t *player, qhandle_t sfx, qboolean bufferedSfx, const char *message)
{
	if (cg_announcerVoice.integer && sfx) {
		if (bufferedSfx) {
			CG_AddBufferedSound( sfx );
		} else {
			trap_S_StartLocalSound( sfx, CHAN_ANNOUNCER );
		}
	}

	if (message[0] != '\0') {
		if (cg_announcerText.integer) {
			CG_Printf("[skipnotify]" S_COLOR_YELLOW "Annoucement: %s\n", message);
		}
		player->announcementTime[player->announcement] = cg.time;
		Q_strncpyz(player->announcementMessage[player->announcement], message, sizeof (player->announcementMessage[player->announcement]));

		player->announcement = (player->announcement + 1) % MAX_ANNOUNCEMENTS;
	}
}
#endif

//=========================================================================

/*
**  Frustum code
*/

// some culling bits
typedef struct plane_s {
	vec3_t normal;
	float dist;
} plane_t;

static plane_t frustum[4];

//
//	CG_SetupFrustum
//
void CG_SetupFrustum( void ) {
	int i;
	float xs, xc;
	float ang;

	ang = cg.refdef.fov_x / 180 * M_PI * 0.5f;
	xs = sin( ang );
	xc = cos( ang );

	VectorScale( cg.refdef.viewaxis[0], xs, frustum[0].normal );
	VectorMA( frustum[0].normal, xc, cg.refdef.viewaxis[1], frustum[0].normal );

	VectorScale( cg.refdef.viewaxis[0], xs, frustum[1].normal );
	VectorMA( frustum[1].normal, -xc, cg.refdef.viewaxis[1], frustum[1].normal );

	ang = cg.refdef.fov_y / 180 * M_PI * 0.5f;
	xs = sin( ang );
	xc = cos( ang );

	VectorScale( cg.refdef.viewaxis[0], xs, frustum[2].normal );
	VectorMA( frustum[2].normal, xc, cg.refdef.viewaxis[2], frustum[2].normal );

	VectorScale( cg.refdef.viewaxis[0], xs, frustum[3].normal );
	VectorMA( frustum[3].normal, -xc, cg.refdef.viewaxis[2], frustum[3].normal );

	for ( i = 0 ; i < 4 ; i++ ) {
		frustum[i].dist = DotProduct( cg.refdef.vieworg, frustum[i].normal );
	}
}

//
//	CG_CullPoint - returns true if culled
//
qboolean CG_CullPoint( vec3_t pt ) {
	int i;
	plane_t *frust;

	// check against frustum planes
	for ( i = 0 ; i < 4 ; i++ ) {
		frust = &frustum[i];

		if ( ( DotProduct( pt, frust->normal ) - frust->dist ) < 0 ) {
			return( qtrue );
		}
	}

	return( qfalse );
}

qboolean CG_CullPointAndRadius( const vec3_t pt, vec_t radius ) {
	int i;
	plane_t *frust;

	// check against frustum planes
	for ( i = 0 ; i < 4 ; i++ ) {
		frust = &frustum[i];

		if ( ( DotProduct( pt, frust->normal ) - frust->dist ) < -radius ) {
			return( qtrue );
		}
	}

	return( qfalse );
}

//=========================================================================

/*
=================
CG_DrawActiveFrame

Generates and draws a game scene and status information at the given time.
=================
*/
void CG_DrawActiveFrame( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback ) {
	int		inwater;
	qboolean renderPlayerViewport[MAX_SPLITVIEW];
	int		i;
#ifdef TURTLEARENA
	qboolean	twodMode;
#endif

	cg.time = serverTime;
	cg.demoPlayback = demoPlayback;

	// if we are only updating the screen as a loading
	// pacifier, don't even try to read snapshots
	if ( cg.infoScreenText[0] != 0 ) {
		CG_DrawInformation();
		return;
	}

	// any looped sounds will be respecified as entities
	// are added to the render list
	trap_S_ClearLoopingSounds(qfalse);

	// clear all the render lists
	trap_R_ClearScene();

	// clear poly buffers for this frame
	CG_PB_ClearPolyBuffers();

	// set up cg.snap and possibly cg.nextSnap
	CG_ProcessSnapshots( qfalse );

	// if we haven't received any snapshots yet, all
	// we can draw is the information screen
	if ( !cg.snap || ( cg.snap->snapFlags & SNAPFLAG_NOT_ACTIVE ) ) {
		CG_DrawInformation();
		return;
	}

	if ( !cg.lightstylesInited ) {
		CG_SetupDlightstyles();
	}

	// this counter will be bumped for every valid scene we generate
	cg.clientFrame++;

#ifdef TURTLEARENA
	// Use single camera/viewport at intermission
	for (i = 0; i < CG_MaxSplitView(); i++) {
		if ( cg.localPlayers[i].playerNum != -1 && cg.snap->pss[i].pm_type != PM_INTERMISSION ) {
			// client present and not at intermission, keep viewports separate.
			break;
		}
	}
	cg.allLocalClientsAtIntermission = (i == CG_MaxSplitView());

	// 2D mode views all local clients or all clients on server.
	twodMode = (cg_2dmode.integer && !(cg_2dmodeOverride.integer && cg_2dmode.integer != 2));

	// Use single camera/viewport in 2d mode and (non-Coop) intermission.
	cg.singleCamera = (CG_NumLocalPlayers() > 1) && (twodMode || (cgs.gametype != GT_SINGLE_PLAYER && cg.allLocalClientsAtIntermission));
#else
	// Use single camera/viewport at intermission
	for (i = 0; i < CG_MaxSplitView(); i++) {
		if ( cg.localPlayers[i].playerNum != -1 && cg.snap->pss[i].pm_type != PM_INTERMISSION ) {
			// player present and not at intermission, keep viewports separate.
			break;
		}
	}
	cg.singleCamera = ( CG_NumLocalPlayers() > 1 && i == CG_MaxSplitView() );
#endif

	cg.numViewports = 0;
	for (i = 0; i < CG_MaxSplitView(); i++) {
		if ( cg.localPlayers[i].playerNum == -1 ) {
			renderPlayerViewport[i] = qfalse;
			continue;
		}
		cg.cur_localPlayerNum = i;
		cg.cur_lc = &cg.localPlayers[i];
		cg.cur_ps = &cg.snap->pss[i];

		// Check if viewport should be drawn.
		if ( cg.singleCamera && cg.numViewports >= 1 ) {
			renderPlayerViewport[i] = qfalse;
		} else {
			cg.numViewports++;
			renderPlayerViewport[i] = qtrue;
		}

		// update cg.predictedPlayerState
		CG_PredictPlayerState();

#ifdef TURTLEARENA // LOCKON
		if (!cg.cur_lc->lockedOn && (cg.cur_lc->predictedPlayerState.eFlags & EF_LOCKON))
		{
			cg.cur_lc->lockedOn = qtrue;
			cg.cur_lc->lockonTime = cg.time;
#ifdef IOQ3ZTM // LETTERBOX
			CG_ToggleLetterbox(qtrue, qfalse);
#endif
		}
		else if (cg.cur_lc->lockedOn && !(cg.cur_lc->predictedPlayerState.eFlags & EF_LOCKON))
		{
			cg.cur_lc->lockedOn = qfalse;
			cg.cur_lc->lockonTime = cg.time;
#ifdef IOQ3ZTM // LETTERBOX
			CG_ToggleLetterbox(qfalse, qfalse);
#endif
		}
#endif

		// Remove expired console lines
		if( cg.cur_lc->consoleLines[ 0 ].time + cg_consoleLatency.integer < cg.time && cg_consoleLatency.integer > 0 ) {
			CG_RemoveNotifyLine( cg.cur_lc );
		}
	}

	cg.cur_localPlayerNum = -1;
	cg.cur_lc = NULL;
	cg.cur_ps = NULL;

	// If all local players dropped out from playing still draw main local player.
	if (cg.numViewports == 0) {
		cg.numViewports = 1;
		renderPlayerViewport[0] = qtrue;
	}

	// play lead change annoucement and time/frag limit warnings
	CG_CheckGameSounds();

	// add buffered sounds
	CG_PlayBufferedSounds();

#ifdef MISSIONPACK
	// play buffered voice chats
	CG_PlayBufferedVoiceChats();
#endif

	for (i = 0, cg.viewport = -1; i < CG_MaxSplitView(); i++) {
		if (!renderPlayerViewport[i]) {
			continue;
		}
		cg.viewport++;
		cg.cur_localPlayerNum = i;
		cg.cur_lc = &cg.localPlayers[i];
		cg.cur_ps = &cg.snap->pss[i];

		// decide on third person view
		cg.cur_lc->renderingThirdPerson = cg.cur_ps->persistant[PERS_TEAM] != TEAM_SPECTATOR
							&& (cg_thirdPerson[cg.cur_localPlayerNum].integer || (cg.cur_ps->stats[STAT_HEALTH] <= 0) || cg.cur_lc->cameraOrbit);

		// build cg.refdef
		inwater = CG_CalcViewValues();
		CG_SetupFrustum();

		CG_DrawSkyBoxPortal();

#ifndef NOBLOOD
		// first person blend blobs, done after AnglesToAxis
		if ( !cg.cur_lc->renderingThirdPerson ) {
			CG_DamageBlendBlob();
		}
#endif

		// build the render lists
		if ( !cg.cur_lc->hyperspace ) {
			CG_AddPacketEntities();			// adter calcViewValues, so predicted player state is correct
			CG_AddMarks();
			CG_AddParticles ();
			CG_AddLocalEntities();
			CG_AddAtmosphericEffects();
		}
		CG_AddViewWeapon( &cg.cur_lc->predictedPlayerState );

		// finish up the rest of the refdef
		if ( cg.testModelEntity.hModel ) {
			CG_AddTestModel();
		}
		cg.refdef.time = cg.time;
		memcpy( cg.refdef.areamask, cg.snap->areamask[i], sizeof( cg.refdef.areamask ) );

		// warning sounds when powerup is wearing off
		CG_PowerupTimerSounds();

		// update audio positions
		trap_S_Respatialize( cg.cur_ps->playerNum, cg.refdef.vieworg, cg.refdef.viewaxis, inwater, !cg.cur_lc->renderingThirdPerson );

		// make sure the lagometerSample and frame timing isn't done twice when in stereo
		if ( stereoView != STEREO_RIGHT && cg.viewport == 0 ) {
			cg.frametime = cg.time - cg.oldTime;
			if ( cg.frametime < 0 ) {
				cg.frametime = 0;
			}
			cg.oldTime = cg.time;
			CG_AddLagometerFrameInfo();
		}
		if (cg_timescale.value != cg_timescaleFadeEnd.value) {
			if (cg_timescale.value < cg_timescaleFadeEnd.value) {
				cg_timescale.value += cg_timescaleFadeSpeed.value * ((float)cg.frametime) / 1000;
				if (cg_timescale.value > cg_timescaleFadeEnd.value)
					cg_timescale.value = cg_timescaleFadeEnd.value;
			}
			else {
				cg_timescale.value -= cg_timescaleFadeSpeed.value * ((float)cg.frametime) / 1000;
				if (cg_timescale.value < cg_timescaleFadeEnd.value)
					cg_timescale.value = cg_timescaleFadeEnd.value;
			}
			if (cg_timescaleFadeSpeed.value) {
				trap_Cvar_SetValue("timescale", cg_timescale.value);
			}
		}

		// actually issue the rendering calls
		CG_DrawActive( stereoView );
	}

	cg.cur_localPlayerNum = -1;
	cg.cur_lc = NULL;
	cg.cur_ps = NULL;

	// load any models that have been deferred if a scoreboard is shown
	if ( !CG_AnyScoreboardShowing() ) {
		cg.deferredPlayerLoading = 0;
	} else if ( ++cg.deferredPlayerLoading > 10 ) {
		CG_LoadDeferredPlayers();
	}

	// if there are 3 viewports of equal size, fill the fourth quarter of the window
	if ( cg.numViewports == 3 && cg_splitviewThirdEqual.integer ) {
		// Setup forth viewport
		cg.numViewports = 4;
		cg.viewport = 3;

		// calculate size of viewport
		CG_CalcVrect();

		CG_ClearViewport();
		CG_DrawTourneyScoreboard();
	}

	if (cg.numViewports != 1) {
		// Setup single viewport
		cg.numViewports = 1;
		cg.viewport = 0;

		// calculate size of viewport
		CG_CalcVrect();
	}

	// Draw over all viewports
	CG_DrawScreen2D( stereoView );

	if ( cg_stats.integer ) {
		CG_Printf( "cg.clientFrame:%i\n", cg.clientFrame );
	}


}


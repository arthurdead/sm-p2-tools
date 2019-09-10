/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod (C)2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This file is part of the SourceMod/SourcePawn SDK.
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#if defined _portal2_included
 #endinput
#endif
#define _portal2_included

enum P2Paint
{
	P2Paint_Bounce,
	P2Paint_Reflect,
	P2Paint_Speed,
	P2Paint_Portal,
	P2Paint_None,
	P2Paint_Count,
	P2Paint_Invalid,
};

native bool P2_HasPaintmap();
native bool P2_SpherePaintSurface(int model, const float pos[3], int color, float radius, float coat);
native void P2_SphereTracePaintSurface(int model, const float pos[3], const float norm[3], float radius, ArrayList colors);
native void P2_RemoveAllPaint();
native void P2_PaintAllSurfaces(int color);
native void P2_RemovePaint(int model);
native void P2_GetPaintmapDataRLE(ArrayList data);
native void P2_LoadPaintmapDataRLE(ArrayList data);
native void P2_SendPaintmapDataToClient(int client);

native void P2_Paint(int entity, const float pos[3]);
native void P2_CleansePaint(int entity);
native P2Paint P2_GetPaintPowerAtPoint(int entity, const float pos[3]);
native P2Paint P2_GetPaintedPower(int entity);

enum P2PaintedState
{
	P2PaintedState_Activate,
	P2PaintedState_Use,
	P2PaintedState_Deactivate,
};

forward void P2_OnPainted(int entity, P2Paint paint, P2PaintedState state);

enum P2Cond
{
	P2Cond_Tauting,
	P2Cond_Poiting,
	P2Cond_Drowing,
	P2Cond_DeathCrush,
	P2Cond_DeathGib,
};

const float P2CondDuration_Infinite = -1.0;

/**
 * Adds a condition to a player
 *
 * @param client        Player's index.
 * @param condition     Integer identifier of condition to apply.
 * @param duration      Duration of condition (does not apply to all conditions).
 *                      Pass P2CondDuration_Infinite to apply until manually removed.
 * @error               Invalid client index, client not in game, or no mod support.
 */
native void P2_AddCondition(int client, P2Cond condition, float duration=P2CondDuration_Infinite);

/**
 * Removes a condition from a player
 *
 * @param client        Player's index.
 * @param condition     Integer identifier of condition to remove.
 * @error               Invalid client index, client not in game, or no mod support.
 */
native void P2_RemoveCondition(int client, P2Cond condition);

/**
 * Called after a condition is added to a player
 *
 * @param client        Index of the client to which the condition is being added.
 * @param condition     Condition that is being added.
 */
forward void P2_OnConditionAdded(int client, P2Cond condition);

/**
 * Called after a condition is removed from a player
 *
 * @param client        Index of the client to which the condition is being removed.
 * @param condition     Condition that is being removed.
 */
forward void P2_OnConditionRemoved(int client, P2Cond condition);

/**
 * Do not edit below this line!
 */
public Extension __ext_portal2 = 
{
	name = "Portal2 Tools",
	file = "game.portal2.ext",
	autoload = 0,
#if defined REQUIRE_EXTENSIONS
	required = 1,
#else
	required = 0,
#endif
};

#if !defined REQUIRE_EXTENSIONS
public void __ext_portal2_SetNTVOptional()
{
	MarkNativeAsOptional("P2_AddCondition");
	MarkNativeAsOptional("P2_RemoveCondition");
	MarkNativeAsOptional("P2_HasPaintmap");
	MarkNativeAsOptional("P2_SpherePaintSurface");
	MarkNativeAsOptional("P2_SphereTracePaintSurface");
	MarkNativeAsOptional("P2_RemoveAllPaint");
	MarkNativeAsOptional("P2_PaintAllSurfaces");
	MarkNativeAsOptional("P2_RemovePaint");
	MarkNativeAsOptional("P2_GetPaintmapDataRLE");
	MarkNativeAsOptional("P2_LoadPaintmapDataRLE");
	MarkNativeAsOptional("P2_SendPaintmapDataToClient");
	MarkNativeAsOptional("P2_Paint");
	MarkNativeAsOptional("P2_CleansePaint");
	MarkNativeAsOptional("P2_GetPaintPowerAtPoint");
	MarkNativeAsOptional("P2_GetPaintedPower");
}
#endif

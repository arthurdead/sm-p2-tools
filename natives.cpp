/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod Team Fortress 2 Extension
 * Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
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

#include "extension.h"
#include "util.h"
#include "time.h"
#include "RegNatives.h"
#include "ivmodelinfo.h"

#include <ISDKTools.h>
#include <sm_argbuffer.h>

extern IVModelInfo *modelinfo;

cell_t P2_AddCondition(IPluginContext *pContext, const cell_t *params)
{
	static ICallWrapper *pWrapper = NULL;

	// CPortalPlayerShared::AddCond(int, float)
	if (!pWrapper)
	{
		REGISTER_NATIVE_ADDR("AddCondition", 
			PassInfo pass[3]; \
			pass[0].flags = PASSFLAG_BYVAL; \
			pass[0].size = sizeof(int); \
			pass[0].type = PassType_Basic; \
			pass[1].flags = PASSFLAG_BYVAL; \
			pass[1].size = sizeof(float); \
			pass[1].type = PassType_Float; \
			pWrapper = g_pBinTools->CreateCall(addr, CallConv_ThisCall, NULL, pass, 2))
	}

	CBaseEntity *pEntity;
	if (!(pEntity = UTIL_GetCBaseEntity(params[1], true)))
	{
		return pContext->ThrowNativeError("Client index %d is not valid", params[1]);
	}

	void *obj = (void *)((uint8_t *)pEntity + playerSharedOffset->actual_offset);
	ArgBuffer<void*, int, float> vstk(obj, params[2], sp_ctof(params[3]));

	pWrapper->Execute(vstk, nullptr);
	return 1;
}

cell_t P2_RemoveCondition(IPluginContext *pContext, const cell_t *params)
{
	static ICallWrapper *pWrapper = NULL;

	// CPortalPlayerShared::RemoveCond(int)
	if (!pWrapper)
	{
		REGISTER_NATIVE_ADDR("RemoveCondition", 
			PassInfo pass[2]; \
			pass[0].flags = PASSFLAG_BYVAL; \
			pass[0].size = sizeof(int); \
			pass[0].type = PassType_Basic; \
			pWrapper = g_pBinTools->CreateCall(addr, CallConv_ThisCall, NULL, pass, 1))
	}

	CBaseEntity *pEntity;
	if (!(pEntity = UTIL_GetCBaseEntity(params[1], true)))
	{
		return pContext->ThrowNativeError("Client index %d is not valid", params[1]);
	}

	void *obj = (void *)((uint8_t *)pEntity + playerSharedOffset->actual_offset);
	ArgBuffer<void*, int> vstk(obj, params[2]);

	pWrapper->Execute(vstk, nullptr);
	return 1;
}

cell_t P2_HasPaintmap(IPluginContext *pContext, const cell_t *params)
{
	return engine->HasPaintmap();
}

cell_t P2_SpherePaintSurface(IPluginContext *pContext, const cell_t *params)
{
	const model_t *model = modelinfo->GetModel(params[1]);

	cell_t *vec;
	pContext->LocalToPhysAddr(params[2], &vec);

	const Vector v(
		sp_ctof(vec[0]),
		sp_ctof(vec[1]),
		sp_ctof(vec[2]));

	return engine->SpherePaintSurface(model, v, params[3], sp_ctof(params[4]), sp_ctof(params[5]));
}

cell_t P2_SphereTracePaintSurface(IPluginContext *pContext, const cell_t *params)
{
	const model_t *model = modelinfo->GetModel(params[1]);

	cell_t *vec1;
	pContext->LocalToPhysAddr(params[2], &vec1);

	cell_t *vec2;
	pContext->LocalToPhysAddr(params[3], &vec2);

	const Vector v1(
		sp_ctof(vec1[0]),
		sp_ctof(vec1[1]),
		sp_ctof(vec1[2]));

	const Vector v2(
		sp_ctof(vec2[0]),
		sp_ctof(vec2[1]),
		sp_ctof(vec2[2]));

	CUtlVector<unsigned char> colors{};

	engine->SphereTracePaintSurface(model, v1, v2, sp_ctof(params[4]), colors);
	return 0;
}

cell_t P2_RemoveAllPaint(IPluginContext *pContext, const cell_t *params)
{
	engine->RemoveAllPaint();
	return 0;
}

cell_t P2_PaintAllSurfaces(IPluginContext *pContext, const cell_t *params)
{
	engine->PaintAllSurfaces(params[1]);
	return 0;
}

cell_t P2_RemovePaint(IPluginContext *pContext, const cell_t *params)
{
	const model_t *model = modelinfo->GetModel(params[1]);

	engine->RemovePaint(model);
	return 0;
}

cell_t P2_SendPaintmapDataToClient(IPluginContext *pContext, const cell_t *params)
{
	edict_t *pEntity = gamehelpers->EdictOfIndex(params[1]);
	if (!pEntity)
	{
		return pContext->ThrowNativeError("Entity %d (%d) is invalid", gamehelpers->ReferenceToIndex(params[1]), params[1]);
	}

	engine->SendPaintmapDataToClient(pEntity);
	return 0;
}

sp_nativeinfo_t g_P2Natives[] = 
{
	{"P2_AddCondition",			P2_AddCondition},
	{"P2_RemoveCondition",			P2_RemoveCondition},

	{"P2_HasPaintmap",				P2_HasPaintmap},
	{"P2_SpherePaintSurface",		P2_SpherePaintSurface},
	{"P2_SphereTracePaintSurface",	P2_SphereTracePaintSurface},
	{"P2_RemoveAllPaint",			P2_RemoveAllPaint},
	{"P2_PaintAllSurfaces",			P2_PaintAllSurfaces},
	{"P2_RemovePaint",				P2_RemovePaint},
	//{"P2_GetPaintmapDataRLE",		P2_GetPaintmapDataRLE},
	//{"P2_LoadPaintmapDataRLE",		P2_LoadPaintmapDataRLE},
	{"P2_SendPaintmapDataToClient",	P2_SendPaintmapDataToClient},

	//{"P2_Paint",					P2_Paint},
	//{"P2_CleansePaint",				P2_CleansePaint},
	//{"P2_GetPaintPowerAtPoint",		P2_GetPaintPowerAtPoint},
	//{"P2_GetPaintedPower",			P2_GetPaintedPower},

	{NULL,							NULL}
};

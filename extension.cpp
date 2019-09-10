/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod Team Fortress 2 Extension
 * Copyright (C) 2004-2015 AlliedModders LLC.  All rights reserved.
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

#include <sourcemod_version.h>
#include "extension.h"
#include "util.h"
#include "RegNatives.h"
#include "iplayerinfo.h"
#include "sm_trie_tpl.h"
#include "conditions.h"
#include "CDetour/detours.h"
#include "ISDKHooks.h"
#include "ISDKTools.h"
#include "ivmodelinfo.h"

ICvar *icvar = NULL;

#include "compat_wrappers.h"

/**
 * @file extension.cpp
 * @brief Implement extension code here.
 */


Portal2Tools g_Portal2Tools;
IGameConfig *g_pGameConf = NULL;

IBinTools *g_pBinTools = NULL;
ISDKHooks *g_pSDKHooks = NULL;
ISDKTools *g_pSDKTools = NULL;

SMEXT_LINK(&g_Portal2Tools);

CGlobalVars *gpGlobals = NULL;
IVModelInfo *modelinfo = NULL;

sm_sendprop_info_t *playerSharedOffset;

extern sp_nativeinfo_t g_P2Natives[];

IForward *g_paintForward = NULL;

SH_DECL_HOOK3_void(IServerGameDLL, ServerActivate, SH_NOATTRIB, 0, edict_t *, int , int);

bool Portal2Tools::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
	if (strcmp(g_pSM->GetGameFolderName(), "portal2") != 0)
	{
		UTIL_Format(error, maxlength, "Cannot Load Portal2 Extension on mods other than Portal2");
		return false;
	}

	ServerClass *sc = UTIL_FindServerClass("CPortal_Player");
	if (sc == NULL)
	{
		UTIL_Format(error, maxlength, "Could not find CPortal_Player server class");
		return false;
	}

	playerSharedOffset = new sm_sendprop_info_t;

	if (!UTIL_FindDataTable(sc->m_pTable, "DT_PortalPlayerShared", playerSharedOffset, 0))
	{
		UTIL_Format(error, maxlength, "Could not find DT_PortalPlayerShared data table");
		return false;
	}

	sharesys->AddDependency(myself, "bintools.ext", true, true);
	sharesys->AddDependency(myself, "sdkhooks.ext", true, true);
	sharesys->AddDependency(myself, "sdktools.ext", false, true);

	char conf_error[255] = "";
	if (!gameconfs->LoadGameConfigFile("sm-portal2.games", &g_pGameConf, conf_error, sizeof(conf_error)))
	{
		if (conf_error[0])
		{
			UTIL_Format(error, maxlength, "Could not read sm-portal2.games.txt: %s", conf_error);
		}
		return false;
	}

	CDetourManager::Init(g_pSM->GetScriptingEngine(), g_pGameConf);

	sharesys->AddNatives(myself, g_P2Natives);
	sharesys->RegisterLibrary(myself, "portal2");

	plsys->AddPluginsListener(this);

	g_addCondForward = forwards->CreateForward("P2_OnConditionAdded", ET_Ignore, 2, NULL, Param_Cell, Param_Cell);
	g_removeCondForward = forwards->CreateForward("P2_OnConditionRemoved", ET_Ignore, 2, NULL, Param_Cell, Param_Cell);
	g_paintForward = forwards->CreateForward("P2_OnPainted", ET_Ignore, 3, NULL, Param_Cell, Param_Cell, Param_Cell);

	g_pCVar = icvar;

	m_CondChecksEnabled = false;

	return true;
}

const char *Portal2Tools::GetExtensionVerString()
{
	return SOURCEMOD_VERSION;
}

const char *Portal2Tools::GetExtensionDateString()
{
	return SOURCEMOD_BUILD_TIME;
}

bool Portal2Tools::SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);

	gpGlobals = ismm->GetCGlobals();

	SH_ADD_HOOK(IServerGameDLL, ServerActivate, gamedll, SH_STATIC(OnServerActivate), true);

	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);

	GET_V_IFACE_CURRENT(GetEngineFactory, modelinfo, IVModelInfo, VMODELINFO_SERVER_INTERFACE_VERSION);

	return true;
}

void Portal2Tools::SDK_OnUnload()
{
	SH_REMOVE_HOOK(IServerGameDLL, ServerActivate, gamedll, SH_STATIC(OnServerActivate), true);

	g_RegNatives.UnregisterAll();
	gameconfs->CloseGameConfigFile(g_pGameConf);

	plsys->RemovePluginsListener(this);

	forwards->ReleaseForward(g_addCondForward);
	forwards->ReleaseForward(g_removeCondForward);
	forwards->ReleaseForward(g_paintForward);
}

void Portal2Tools::SDK_OnAllLoaded()
{
	SM_GET_LATE_IFACE(BINTOOLS, g_pBinTools);
	SM_GET_LATE_IFACE(SDKHOOKS, g_pSDKHooks);
	SM_GET_LATE_IFACE(SDKTOOLS, g_pSDKTools);
}

bool Portal2Tools::RegisterConCommandBase(ConCommandBase *pVar)
{
	return g_SMAPI->RegisterConCommandBase(g_PLAPI, pVar);
}

bool Portal2Tools::QueryRunning(char *error, size_t maxlength)
{
	SM_CHECK_IFACE(BINTOOLS, g_pBinTools);
	SM_GET_LATE_IFACE(SDKHOOKS, g_pSDKHooks);

	return true;
}

bool Portal2Tools::QueryInterfaceDrop(SMInterface *pInterface)
{
	if (pInterface == g_pBinTools)
	{
		return false;
	}

	if (pInterface == g_pSDKHooks)
	{
		return false;
	}

	if (pInterface == g_pSDKTools)
	{
		g_pSDKTools = NULL;
	}

	return IExtensionInterface::QueryInterfaceDrop(pInterface);
}

void Portal2Tools::NotifyInterfaceDrop(SMInterface *pInterface)
{
	g_RegNatives.UnregisterAll();
}

void OnServerActivate(edict_t *pEdictList, int edictCount, int clientMax)
{
	
}

void Portal2Tools::OnPluginLoaded(IPlugin *plugin)
{
	if (!m_CondChecksEnabled
		&& ( g_addCondForward->GetFunctionCount() || g_removeCondForward->GetFunctionCount() )
		)
	{
		m_CondChecksEnabled = g_CondMgr.Init();
	}
}

void Portal2Tools::OnPluginUnloaded(IPlugin *plugin)
{
	if (m_CondChecksEnabled)
	{
		if (!g_addCondForward->GetFunctionCount() && !g_removeCondForward->GetFunctionCount())
		{
			g_CondMgr.Shutdown();
			m_CondChecksEnabled = false;
		}
	}
}

int FindEntityByNetClass(int start, const char *classname)
{
	edict_t *current;

	for (int i = ((start != -1) ? start : 0); i < gpGlobals->maxEntities; i++)
	{
		current = PEntityOfEntIndex(i);
		if (current == NULL || current->IsFree())
		{
			continue;
		}

		IServerNetworkable *network = current->GetNetworkable();

		if (network == NULL)
		{
			continue;
		}

		ServerClass *sClass = network->GetServerClass();
		const char *name = sClass->GetName();
		

		if (strcmp(name, classname) == 0)
		{
			return i;
		}
	}

	return -1;
}



/**
 * A picture of a blue crab given to me as a gift and stored here for safe keeping
 *
 * http://sourcemod.net/Clown%20car.jpg
 */


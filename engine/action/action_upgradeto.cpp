//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name action_upgradeto.cpp - The unit upgrading to new action. */
//
//      (c) Copyright 1998-2005 by Lutz Sammer and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//
//      $Id$

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
#include "player.h"
#include "unittype.h"
#include "animation.h"
#include "unit.h"
#include "actions.h"
#include "ai.h"
#include "interface.h"
#include "map.h"
#include "spells.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Transform an unit in another.
**
**  @param unit     unit to transform.
**  @param newtype  new type of the unit.
**
**  @return 0 on error, 1 if nothing happens, 2 else.
*/
int TransformUnitIntoType(CUnit *unit, CUnitType *newtype)
{
	CPlayer *player;
	CUnitType *oldtype;
	const CUnitStats *newstats;
	int x;
	int y;
	int i;
	CUnit *container;

	Assert(unit);
	Assert(newtype);

	oldtype = unit->Type;
	if (oldtype == newtype) { // nothing to do
		return 1;
	}
	x = unit->X + (oldtype->TileWidth - newtype->TileWidth) / 2;
	y = unit->Y + (oldtype->TileHeight - newtype->TileHeight) / 2;

	container = unit->Container;
	if (container) {
		MapUnmarkUnitSight(unit);
	} else {
		SaveSelection();
		unit->Remove(NULL);
		if (!UnitTypeCanBeAt(newtype, x, y)) {
			unit->Place(unit->X, unit->Y);
			RestoreSelection();
			// FIXME unit is not modified, try later ?
			return 0;
		}
	}
	player = unit->Player;
	player->UnitTypesCount[oldtype->Slot]--;
	player->UnitTypesCount[newtype->Slot]++;

	player->Demand += newtype->Demand - oldtype->Demand;
	player->Supply += newtype->Supply - oldtype->Supply;

	//  adjust Variables with percent.
	newstats = &newtype->Stats[player->Index];

	for (i = 0; i < UnitTypeVar.NumberVariable; ++i) {
		if (unit->Variable[i].Max) {
			unit->Variable[i].Value = newstats->Variables[i].Max *
				unit->Variable[i].Value / unit->Variable[i].Max;
		} else {
			unit->Variable[i].Value = newstats->Variables[i].Value;
		}
		unit->Variable[i].Max = newstats->Variables[i].Max;
		unit->Variable[i].Increase = newstats->Variables[i].Increase;
		unit->Variable[i].Enable = newstats->Variables[i].Enable;
	}

	unit->Type = newtype;
	unit->Stats = &newtype->Stats[player->Index];

	if (newtype->CanCastSpell && !unit->AutoCastSpell) {
		unit->AutoCastSpell = new char[SpellTypeTable.size()];
		memset(unit->AutoCastSpell, 0, SpellTypeTable.size() * sizeof(char));
	}

	UpdateForNewUnit(unit, 1);
	//  Update Possible sight range change
	UpdateUnitSightRange(unit);
	if (!container) {
		unit->Place(x, y);
		RestoreSelection();
	} else {
		MapMarkUnitSight(unit);
	}
	//
	// Update possible changed buttons.
	//
	if (IsOnlySelected(unit) || unit->Player == ThisPlayer) {
		// could affect the buttons of any selected unit
		SelectedUnitChanged();
	}
	return 1;
}

/**
**  Unit transform into unit.
**
**  @param unit  Pointer to unit.
*/
void HandleActionTransformInto(CUnit *unit)
{
	// What to do if an error occurs ?
	TransformUnitIntoType(unit, unit->CriticalOrder.Type);
	unit->CriticalOrder.Action = UnitActionStill;
}

/**
**  Unit upgrades unit!
**
**  @param unit  Pointer to unit.
*/
void HandleActionUpgradeTo(CUnit *unit)
{
	CPlayer *player;
	CUnitType *newtype;
	const CUnitStats *newstats;

	Assert(unit);

	if (!unit->SubAction) { // first entry
		unit->Data.UpgradeTo.Ticks = 0;
		unit->SubAction = 1;
	}

	unit->Type->Animations->Upgrade ?
		UnitShowAnimation(unit, unit->Type->Animations->Upgrade) :
		UnitShowAnimation(unit, unit->Type->Animations->Still);
	if (unit->Wait) {
		unit->Wait--;
		return;
	}

	player = unit->Player;
	newtype = unit->Orders[0]->Type;
	newstats = &newtype->Stats[player->Index];

	// FIXME: Should count down here
	unit->Data.UpgradeTo.Ticks += SpeedUpgrade;
	if (unit->Data.UpgradeTo.Ticks < newstats->Costs[TimeCost]) {
		unit->Wait = CYCLES_PER_SECOND / 6;
		return;
	}

	unit->Orders[0]->Action = UnitActionStill;
	unit->SubAction = unit->State = 0;

	if (TransformUnitIntoType(unit, newtype) == 0) {
		player->Notify(NotifyGreen, unit->X, unit->Y,
			_("Upgrade to %s canceled"), newtype->Name.c_str());
		return ;
	}
	//  Warn AI.
	if (player->AiEnabled) {
		AiUpgradeToComplete(unit, newtype);
	}
	player->Notify(NotifyGreen, unit->X, unit->Y,
		_("Upgrade to %s complete"), unit->Type->Name.c_str());
}

//@}

//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name action_resource.cpp - The generic resource action. */
//
//      (c) Copyright 2001-2005 by Crestez Leonard and Jimmy Salmon
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
#include <string.h>

#include "stratagus.h"
#include "player.h"
#include "unit.h"
#include "unittype.h"
#include "animation.h"
#include "actions.h"
#include "pathfinder.h"
#include "interface.h"
#include "sound.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

#define SUB_START_RESOURCE 0
#define SUB_MOVE_TO_RESOURCE 5
#define SUB_UNREACHABLE_RESOURCE 31
#define SUB_START_GATHERING 55
#define SUB_GATHER_RESOURCE 60
#define SUB_STOP_GATHERING 65
#define SUB_MOVE_TO_DEPOT 70
#define SUB_UNREACHABLE_DEPOT 100
#define SUB_RETURN_RESOURCE 120

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Move unit to resource.
**
**  @param unit  Pointer to unit.
**
**  @return      TRUE if reached, otherwise FALSE.
*/
static int MoveToResource(CUnit *unit)
{
	CUnit *goal;

	goal = unit->Orders[0]->Goal;
	Assert(goal);
	switch (DoActionMove(unit)) { // reached end-point?
		case PF_UNREACHABLE:
			return -1;
		case PF_REACHED:
			break;
		default:
			// Goal gone or something.
			if (unit->Anim.Unbreakable || goal->IsVisibleAsGoal(unit->Player)) {
				return 0;
			}
			break;
	}
	return 1;
}

/**
**  Start harvesting the resource.
**
**  @param unit  Pointer to unit.
**
**  @return      TRUE if ready, otherwise FALSE.
*/
static int StartGathering(CUnit *unit)
{
	CUnit *goal;
	ResourceInfo *resinfo;

	resinfo = unit->Type->ResInfo[unit->CurrentResource];
	Assert(!unit->IX);
	Assert(!unit->IY);

	goal = unit->Orders[0]->Goal;
	//
	// Target is dead, stop getting resources.
	//
	if (!goal->IsVisibleAsGoal(unit->Player)) {
		goal->RefsDecrease();
		// Find an alternative, but don't look too far.
		unit->Orders[0]->X = unit->Orders[0]->Y = -1;
		if ((goal = UnitFindResource(unit, unit->X, unit->Y, 10, unit->CurrentResource))) {
			unit->SubAction = SUB_START_RESOURCE;
			unit->Orders[0]->Goal = goal;
			goal->RefsIncrease();
		} else {
			unit->Orders[0]->Action = UnitActionStill;
			unit->Orders[0]->Goal = NoUnitP;
			unit->SubAction = 0;
		}
		return 0;
	}

	// FIXME: 0 can happen, if to near placed by map designer.
	Assert(MapDistanceBetweenUnits(unit, goal) <= 1);

	//
	// Update the heading of a harvesting unit to looks straight at the resource.
	//
	if (goal) {
		UnitHeadingFromDeltaXY(unit,
			2 * (goal->X - unit->X) + goal->Type->TileWidth,
			2 * (goal->Y - unit->Y) + goal->Type->TileHeight);
	}

	//
	// If resource is still under construction, wait!
	//
	if ((goal->Type->MaxOnBoard && goal->Data.Resource.Active >= goal->Type->MaxOnBoard) ||
			goal->Orders[0]->Action == UnitActionBuilt) {
		// FIXME: Determine somehow when the resource will be free to use
		// FIXME: Could we somehow find another resource? Think minerals
		// FIXME: We should add a flag for that, and a limited range.
		// FIXME: Think minerals in st*rcr*ft!!
		// However the CPU usage is really low (no pathfinding stuff).
		unit->Wait = 10;
		return 0;
	}

	// Activate the resource
	goal->Data.Resource.Active++;

	//
	// Place unit inside the resource
	//
	if (!resinfo->HarvestFromOutside) {
		goal->RefsDecrease();
		unit->Orders[0]->Goal = NoUnitP;

		unit->Remove(goal);
	}

	unit->Data.ResWorker.TimeToHarvest = resinfo->WaitAtResource /
		SpeedResourcesHarvest[resinfo->ResourceId];

	unit->Data.ResWorker.DoneHarvesting = 0;

	return 1;
}

/**
**  Animate A unit that is harvesting
**
**  @param unit  Unit to animate
*/
static void AnimateActionHarvest(CUnit *unit)
{
	Assert(unit->Type->Animations->Harvest[unit->CurrentResource]);
	UnitShowAnimation(unit, unit->Type->Animations->Harvest[unit->CurrentResource]);
}

/**
**  Find something else to do when the resource is exhausted.
**  This is called from GatherResorce when the resource is empty.
**
**  @param unit    pointer to harvester unit.
**  @param source  pointer to resource unit.
*/
static void LoseResource(CUnit *unit, const CUnit *source)
{
	CUnit *depot;
	ResourceInfo *resinfo;

	resinfo = unit->Type->ResInfo[unit->CurrentResource];

	Assert((unit->Container && !resinfo->HarvestFromOutside) ||
		(!unit->Container && resinfo->HarvestFromOutside));

	if (resinfo->HarvestFromOutside) {
		unit->Orders[0]->Goal->RefsDecrease();
		unit->Orders[0]->Goal = NoUnitP;
	}

	//
	// If we are loaded first search for a depot.
	//
	if (unit->ResourcesHeld && (depot = FindDeposit(unit, unit->X, unit->Y,
			1000, unit->CurrentResource))) {
		if (unit->Container) {
			DropOutNearest(unit, depot->X + depot->Type->TileWidth / 2,
				depot->Y + depot->Type->TileHeight / 2,
				source->Type->TileWidth, source->Type->TileHeight);
		}
		//
		// Remember were it mined, so it can look around for another resource.
		//
		unit->Orders[0]->Arg1.ResourcePos = (unit->X << 16) | unit->Y;
		unit->Orders[0]->Goal = depot;
		depot->RefsIncrease();
		NewResetPath(unit);
		unit->SubAction = SUB_MOVE_TO_DEPOT;
		unit->State = 0;
		DebugPrint("Sent unit %d to depot\n" _C_ unit->Slot);
		return;
	}
	//
	// No depot found, or harvester empty
	// Dump the unit outside and look for something to do.
	//
	if (unit->Container) {
		Assert(!resinfo->HarvestFromOutside);
		DropOutOnSide(unit, LookingW, source->Type->TileWidth,
			source->Type->TileHeight);
	}
	unit->Orders[0]->X = unit->Orders[0]->Y = -1;
	if ((unit->Orders[0]->Goal = UnitFindResource(unit, unit->X, unit->Y,
			10, unit->CurrentResource))) {
		DebugPrint("Unit %d found another resource.\n" _C_ unit->Slot);
		unit->SubAction = SUB_START_RESOURCE;
		unit->State = 0;
		unit->Orders[0]->Goal->RefsIncrease();
	} else {
		DebugPrint("Unit %d just sits around confused.\n" _C_ unit->Slot);
		unit->Orders[0]->Action = UnitActionStill;
		unit->SubAction = 0;
		unit->State = 0;
	}
}

/**
**  Gather the resource
**
**  @param unit  Pointer to unit.
**
**  @return      non-zero if ready, otherwise zero.
*/
static int GatherResource(CUnit *unit)
{
	CUnit *source;
	CUnit *uins;
	ResourceInfo *resinfo;
	int i;
	int addload;

	resinfo = unit->Type->ResInfo[unit->CurrentResource];
	source = 0;

	if (resinfo->HarvestFromOutside) {
		AnimateActionHarvest(unit);
	} else {
		unit->Anim.CurrAnim = NULL;
	}

	unit->Data.ResWorker.TimeToHarvest--;

	if (unit->Data.ResWorker.DoneHarvesting) {
		Assert(resinfo->HarvestFromOutside);
		return !unit->Anim.Unbreakable;
	}

	while (!unit->Data.ResWorker.DoneHarvesting &&
			unit->Data.ResWorker.TimeToHarvest < 0) {
		unit->Data.ResWorker.TimeToHarvest += resinfo->WaitAtResource /
			SpeedResourcesHarvest[resinfo->ResourceId];

		//
		// Calculate how much we can load.
		//
		if (resinfo->ResourceStep) {
			addload = resinfo->ResourceStep;
		} else {
			addload = resinfo->ResourceCapacity;
		}
		// Make sure we don't bite more than we can chew.
		if (unit->ResourcesHeld + addload > resinfo->ResourceCapacity) {
			addload = resinfo->ResourceCapacity - unit->ResourcesHeld;
		}

		if (resinfo->HarvestFromOutside) {
			source = unit->Orders[0]->Goal;
		} else {
			source = unit->Container;
		}

		Assert(source);
		Assert(source->ResourcesHeld <= 655350);

		//
		// Target is not dead, getting resources.
		//
		if (source->IsVisibleAsGoal(unit->Player)) {
			// Don't load more that there is.
			if (addload > source->ResourcesHeld) {
				addload = source->ResourcesHeld;
			}

			unit->ResourcesHeld += addload;
			source->ResourcesHeld -= addload;
		}

		//
		// End of resource: destroy the resource.
		// FIXME: implement depleted resources.
		//
		if ((!source->IsVisibleAsGoal(unit->Player)) || (source->ResourcesHeld == 0)) {
			if (unit->Anim.Unbreakable) {
				return 0;
			}
			DebugPrint("Resource is destroyed for unit %d\n" _C_ unit->Slot);
			uins = source->UnitInside;
			//
			// Improved version of DropOutAll that makes workers go to the depot.
			//
			LoseResource(unit,source);
			for (i = source->InsideCount; i; --i, uins = uins->NextContained) {
				if (uins->Orders[0]->Action == UnitActionResource) {
					LoseResource(uins,source);
				}
			}

			// Don't destroy the resource twice.
			// This only happens when it's empty.
			if (source->IsVisibleAsGoal(unit->Player)) {
				LetUnitDie(source);
				// FIXME: make the workers inside look for a new resource.
			}
			source = NULL;
			return 0;
		}

		if (resinfo->HarvestFromOutside) {
			if ((unit->ResourcesHeld == resinfo->ResourceCapacity) || (source == NULL)) {
				// Mark as complete.
				unit->Data.ResWorker.DoneHarvesting = 1;
			}
			return 0;
		}

		if (!resinfo->HarvestFromOutside) {
			return unit->ResourcesHeld == resinfo->ResourceCapacity && source;
		}
	}

	return 0;
}

/**
**  Stop gathering from the resource, go home.
**
**  @param unit  Poiner to unit.
**
**  @return      TRUE if ready, otherwise FALSE.
*/
static int StopGathering(CUnit *unit)
{
	CUnit *depot;
	CUnit *source;
	ResourceInfo *resinfo;

	resinfo = unit->Type->ResInfo[unit->CurrentResource];

	if (resinfo->HarvestFromOutside) {
		source = unit->Orders[0]->Goal;
		source->RefsDecrease();
		unit->Orders[0]->Goal = NoUnitP;
	} else {
		source = unit->Container;
	}
	source->Data.Resource.Active--;
	Assert(source->Data.Resource.Active >= 0);


	// Store resource position.
	// FIXME: is this the best way?
	unit->Orders[0]->Arg1.ResourcePos = (unit->X << 16) | unit->Y;

#ifdef DEBUG
	if (!unit->ResourcesHeld) {
		DebugPrint("Unit %d is empty???\n" _C_ unit->Slot);
	}
#endif
	// Find and send to resource deposit.
	if (!(depot = FindDeposit(unit, unit->X, unit->Y, 1000, unit->CurrentResource)) ||
			!unit->ResourcesHeld) {
		if (!resinfo->HarvestFromOutside) {
			Assert(unit->Container);
			DropOutOnSide(unit, LookingW, source->Type->TileWidth,
				source->Type->TileHeight);
		}
		DebugPrint("Can't find a resource deposit for unit %d.\n" _C_ unit->Slot);
		unit->Orders[0]->Action = UnitActionStill;
		unit->Orders[0]->Goal = NoUnitP;
		unit->SubAction = 0;
		// should return 0, done below!
	} else {
		if (!resinfo->HarvestFromOutside) {
			Assert(unit->Container);
			DropOutNearest(unit, depot->X + depot->Type->TileWidth / 2,
				depot->Y + depot->Type->TileHeight / 2,
				source->Type->TileWidth, source->Type->TileHeight);
		}
		unit->Orders[0]->Goal = depot;
		depot->RefsIncrease();
		unit->Orders[0]->Range = 1;
		unit->Orders[0]->X = unit->Orders[0]->Y = -1;
		unit->SubAction = SUB_MOVE_TO_DEPOT;
		NewResetPath(unit);
	}

	if (IsOnlySelected(unit)) {
		SelectedUnitChanged();
	}

	return unit->Orders[0]->Action != UnitActionStill;
}

/**
**  Move to resource depot
**
**  @param unit  Pointer to unit.
**
**  @return      TRUE if reached, otherwise FALSE.
*/
static int MoveToDepot(CUnit *unit)
{
	CUnit *goal;
	ResourceInfo *resinfo;

	resinfo = unit->Type->ResInfo[unit->CurrentResource];

	goal = unit->Orders[0]->Goal;
	Assert(goal);

	switch (DoActionMove(unit)) { // reached end-point?
		case PF_UNREACHABLE:
			return -1;
		case PF_REACHED:
			break;
		default:
			if (unit->Anim.Unbreakable || goal->IsVisibleAsGoal(unit->Player)) {
				return 0;
			}
			break;
	}

	//
	// Target is dead, stop getting resources.
	//
	if (!goal->IsVisibleAsGoal(unit->Player)) {
		DebugPrint("Destroyed depot\n");
		goal->RefsDecrease();
		unit->Orders[0]->Goal = NoUnitP;
		// FIXME: perhaps we should choose an alternative
		unit->Orders[0]->Action = UnitActionStill;
		unit->SubAction = 0;
		return 0;
	}

	//
	// If resource depot is still under construction, wait!
	//
	if (goal->Orders[0]->Action == UnitActionBuilt) {
		unit->Wait = 10;
		return 0;
	}

	goal->RefsDecrease();
	unit->Orders[0]->Goal = NoUnitP;

	//
	// Place unit inside the depot
	//
	unit->Remove(goal);
	unit->Anim.CurrAnim = NULL;

	//
	// Update resource.
	//
	unit->Player->Resources[resinfo->FinalResource] +=
		(unit->ResourcesHeld * unit->Player->Incomes[resinfo->FinalResource]) / 100;
	unit->Player->TotalResources[resinfo->FinalResource] +=
		(unit->ResourcesHeld * unit->Player->Incomes[resinfo->FinalResource]) / 100;
	unit->ResourcesHeld = 0;

	unit->Wait = resinfo->WaitAtDepot / SpeedResourcesReturn[resinfo->ResourceId];
	if (unit->Wait) {
		unit->Wait--;
	}

	return 1;
}

/**
**  Wait in depot, for the resources stored.
**
**  @param unit  Pointer to unit.
**
**  @return      TRUE if ready, otherwise FALSE.
*/
static int WaitInDepot(CUnit *unit)
{
	const CUnit *depot;
	CUnit *goal;
	ResourceInfo *resinfo;
	int x;
	int y;

	resinfo = unit->Type->ResInfo[unit->CurrentResource];

	depot = ResourceDepositOnMap(unit->X, unit->Y, resinfo->ResourceId);
	Assert(depot);
	// Could be destroyed, but then we couldn't be in?

	if (unit->Orders[0]->Arg1.ResourcePos == -1) {
		x = unit->X;
		y = unit->Y;
	} else {
		x = unit->Orders[0]->Arg1.ResourcePos >> 16;
		y = unit->Orders[0]->Arg1.ResourcePos & 0xFFFF;
	}
	// Range hardcoded. don't stray too far though
	if ((goal = UnitFindResource(unit, x, y, 10, unit->CurrentResource))) {
		DropOutNearest(unit, goal->X + goal->Type->TileWidth / 2,
			goal->Y + goal->Type->TileHeight / 2,
			depot->Type->TileWidth, depot->Type->TileHeight);
		unit->Orders[0]->Goal = goal;
		goal->RefsIncrease();
		unit->Orders[0]->Range = 1;
		unit->Orders[0]->X = unit->Orders[0]->Y = -1;
	} else {
		DebugPrint("Unit %d Resource gone. Sit and play dumb.\n" _C_ unit->Slot);
		DropOutOnSide(unit, LookingW, depot->Type->TileWidth, depot->Type->TileHeight);
		unit->Orders[0]->Action = UnitActionStill;
		unit->SubAction = 0;
	}

	return unit->Orders[0]->Action != UnitActionStill;
}

/**
**  Give up on gathering.
**
**  @param unit  Pointer to unit.
*/
void ResourceGiveUp(CUnit *unit)
{
	DebugPrint("Unit %d gave up on resource gathering.\n" _C_ unit->Slot);
	if (unit->Orders[0]->Goal) {
		unit->Orders[0]->Goal->RefsDecrease();
		unit->Orders[0]->Goal = NULL;
	}
	unit->Orders[0]->Init();
	unit->Orders[0]->Action = UnitActionStill;
	unit->SubAction = 0;
	if (unit->CurrentResource &&
			unit->Type->ResInfo[unit->CurrentResource]->LoseResources &&
			unit->ResourcesHeld < unit->Type->ResInfo[unit->CurrentResource]->ResourceCapacity) {
		unit->ResourcesHeld = 0;
		unit->CurrentResource = 0;
	}
}

/**
**  Control the unit action: getting a resource.
**
**  This the generic function for oil, gold, ...
**
**  @param unit  Pointer to unit.
*/
void HandleActionResource(CUnit *unit)
{
	int ret;
	int newres;

	if (unit->Wait) {
		// FIXME: show idle animation while we wait?
		unit->Wait--;
		return;
	}

	// Let's start mining.
	if (unit->SubAction == SUB_START_RESOURCE) {
		if (unit->Orders[0]->Goal) {
			newres = unit->Orders[0]->Goal->Type->GivesResource;
		} else {
			newres = WoodCost;
		}
		if (newres != unit->CurrentResource) {
			// Drop other resources.
			unit->ResourcesHeld = 0;
		}
		if ((unit->CurrentResource = newres)) {
			NewResetPath(unit);
			unit->SubAction = SUB_MOVE_TO_RESOURCE;
		} else {
			unit->ResourcesHeld = 0;
			ResourceGiveUp(unit);
			return;
		}
	}

	// Move to the resource location.
	if (unit->SubAction >= SUB_MOVE_TO_RESOURCE &&
			unit->SubAction < SUB_UNREACHABLE_RESOURCE) {
		// -1 failure, 0 not yet reached, 1 reached
		if ((ret = MoveToResource(unit))) {
			if (ret == -1) {
				// Can't Reach
				unit->SubAction++;
				unit->Wait = 10;
				return;
			} else {
				// Reached
				unit->SubAction = SUB_START_GATHERING;
			}
		} else {
			// Move along.
			return;
		}
	}

	// Resource seems to be unreachable
	if (unit->SubAction == SUB_UNREACHABLE_RESOURCE) {
		ResourceGiveUp(unit);
		return;
	}

	// Start gathering the resource
	if (unit->SubAction == SUB_START_GATHERING) {
		if (StartGathering(unit)) {
			unit->SubAction = SUB_GATHER_RESOURCE;
		} else {
			return;
		}
	}

	// Gather the resource.
	if (unit->SubAction == SUB_GATHER_RESOURCE) {
		if (GatherResource(unit)) {
			unit->SubAction = SUB_STOP_GATHERING;
		} else {
			return;
		}
	}

	// Stop gathering the resource.
	if (unit->SubAction == SUB_STOP_GATHERING) {
		if (StopGathering(unit)) {
			unit->SubAction = SUB_MOVE_TO_DEPOT;
		}
	}

	// Move back home.
	if (unit->SubAction >= SUB_MOVE_TO_DEPOT &&
			unit->SubAction < SUB_UNREACHABLE_DEPOT) {
		// -1 failure, 0 not yet reached, 1 reached
		if ((ret = MoveToDepot(unit))) {
			if (ret == -1) {
				// Can't Reach
				unit->SubAction++;
				unit->Wait = 10;
			} else {
				unit->SubAction = SUB_RETURN_RESOURCE;
			}
		}
		return;
	}

	// Depot seems to be unreachable
	if (unit->SubAction == SUB_UNREACHABLE_DEPOT) {
		ResourceGiveUp(unit);
		return;
	}

	// Unload resources at the depot.
	if (unit->SubAction == SUB_RETURN_RESOURCE) {
		if (WaitInDepot(unit)) {
			unit->SubAction = SUB_START_RESOURCE;
			//
			// It's posible, though very rare that the unit's goal blows up
			// this cycle, but after this unit. Thus, next frame the unit
			// will start mining a destroyed site. If, on the otherhand we
			// are already in SUB_MOVE_TO_RESOURCE then we can handle it.
			// So, we pass through SUB_START_RESOURCE the very instant it
			// goes out of the depot.
			//
			HandleActionResource(unit);
		}
		return;
	}
}

//@}

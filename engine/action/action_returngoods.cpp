//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name action_returngoods.cpp - The return goods action. */
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
--  Include
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "actions.h"
#include "pathfinder.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Return goods to gold/wood deposit.
**
**  @param unit  pointer to unit.
**
**  @todo  FIXME: move this into action_resource?
*/
void HandleActionReturnGoods(CUnit *unit)
{
	Assert(unit->Type->Harvester);

	// Select target to return goods.
	if (!unit->CurrentResource || unit->ResourcesHeld == 0 ||
			(unit->ResourcesHeld != unit->Type->ResInfo[unit->CurrentResource]->ResourceCapacity &&
				unit->Type->ResInfo[unit->CurrentResource]->LoseResources)) {
		DebugPrint("Unit can't return resources, it doesn't carry any.\n");
		unit->Player->Notify(NotifyYellow, unit->X, unit->Y, _("No Resources to Return."));

		if (unit->Orders[0]->Goal) { // Depot (if not destroyed)
			unit->Orders[0]->Goal->RefsDecrease();
			unit->Orders[0]->Goal = NULL;
		}
		unit->Orders[0]->Init();
		unit->Orders[0]->Action = UnitActionStill;
		return;
	}

	// If depot was destroyed search for another one.
	if (!unit->Orders[0]->Goal) {
		CUnit *destu;

		if (!(destu = FindDeposit(unit, unit->X, unit->Y, 1000,
				unit->CurrentResource))) {
			unit->Orders[0]->Init();
			unit->Orders[0]->Action = UnitActionStill;
			return;
		}
		unit->Orders[0]->Goal = destu;
		destu->RefsIncrease();
	}

	unit->Orders[0]->Action = UnitActionResource;
	// Somewhere on the way the loaded worker could have change Arg1
	// Bummer, go get the closest resource to the depot
	unit->Orders[0]->Arg1.ResourcePos = -1;
	NewResetPath(unit);
	unit->SubAction = 70; // FIXME : Define value.
}

//@}

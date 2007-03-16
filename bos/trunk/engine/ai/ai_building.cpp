//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name ai_building.cpp - AI building functions. */
//
//      (c) Copyright 2001-2007 by Lutz Sammer
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
#include "unit.h"
#include "unittype.h"
#include "map.h"
#include "pathfinder.h"
#include "ai_local.h"
#include "player.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Check if the surrounding are free. Depending on the value of flag, it will check :
**  0: the building will not block any way
**  1: all surrounding is free
**
**  @param worker  Worker to build.
**  @param type    Type of building.
**  @param x       X map tile position for the building.
**  @param y       Y map tile position for the building.
**  @param flag    0: only check that building will not block anything.
**
**  @return        True if the surrounding is free, false otherwise.
**
**  @note          Can be faster written.
*/
static int AiCheckSurrounding(const CUnit *worker, const CUnitType *type, int x, int y, int flag)
{
	static int dirs[5][2] = {{1,0},{0,1},{-1,0},{0,-1},{0,0}};
	int surrounding[1024]; // Max criconference for building
	int surroundingnb;
	int x0, y0, x1, y1;
	int i;
	int lastval;
	int dir;
	int obstacle;

	x0 = x - 1;
	y0 = y - 1;
	x1 = x0 + type->TileWidth + 1;
	y1 = y0 + type->TileWidth + 1;


	x = x0;
	y = y0;
	dir = -1;
	surroundingnb = 0;
	while (dir < 4) {
		if ((unsigned)x < (unsigned)Map.Info.MapWidth && (unsigned)y < (unsigned)Map.Info.MapHeight) {
			if (worker && x == worker->X && y == worker->Y) {
				surrounding[surroundingnb++] = 1;
			} else if (Map.Fields[x + y * Map.Info.MapWidth].Flags &
					(MapFieldUnpassable | MapFieldBuilding)) {
				surrounding[surroundingnb++] = 0;
			} else{
				// Can pass there
				surrounding[surroundingnb++] = (Map.Fields[x + y * Map.Info.MapWidth].Flags &
					(MapFieldWaterAllowed + MapFieldCoastAllowed + MapFieldLandAllowed)) != 0;;
			}
		} else {
			surrounding[surroundingnb++] = 0;
		}

		if ((x == x0 || x == x1) && (y == y0 || y == y1)) {
			dir++;
		}

		x += dirs[dir][0];
		y += dirs[dir][1];
	}

	lastval = surrounding[surroundingnb - 1];
	obstacle = 0;
	for (i = 0 ; i < surroundingnb; ++i) {
		if (lastval && !surrounding[i]) {
			++obstacle;
		}
		lastval = surrounding[i];
	}

	if (obstacle == 0) {
		obstacle = !surrounding[0];
	}

	if (flag) {
		return obstacle == 0;
	} else if (!type->ShoreBuilding) {
		return obstacle < 2;
	} else {
		// Shore building haves at least 2 obstacles : sea->ground & ground->sea
		return obstacle < 3;
	}
}

/**
**  Find free building place. (flood fill version)
**
**  @param worker  Worker to build building.
**  @param type    Type of building.
**  @param ox      Original X position to try building
**  @param oy      Original Y position to try building
**  @param dx      Pointer for X position returned.
**  @param dy      Pointer for Y position returned.
**  @param flag    Flag if surrounding must be free.
**
**  @return        True if place found, false if no found.
*/
static int AiFindBuildingPlace2(const CUnit *worker, const CUnitType *type,
	int ox, int oy, int *dx, int *dy, int flag)
{
	static const int xoffset[] = { 0, -1, +1, 0, -1, +1, -1, +1 };
	static const int yoffset[] = { -1, 0, 0, +1, -1, -1, +1, +1 };
	struct p {
		unsigned short X;
		unsigned short Y;
	} *points;
	int size;
	int x;
	int y;
	int rx;
	int ry;
	int mask;
	int wp;
	int rp;
	int ep;
	int i;
	int w;
	unsigned char *m;
	unsigned char *matrix;

	size = Map.Info.MapWidth * Map.Info.MapHeight / 4;
	points = new p[size];

	x = ox;
	y = oy;
	//
	// Look if we can build at current place.
	//
	if (CanBuildUnitType(worker, type, x, y, 1) &&
		(/*!flag || */AiCheckSurrounding(worker, type, x, y, flag))) {
		*dx = x;
		*dy = y;
		delete[] points;
		return 1;
	}
	//
	//  Make movement matrix.
	//
	matrix = CreateMatrix();
	w = Map.Info.MapWidth + 2;

	mask = worker->Type->MovementMask;
	// Ignore all possible mobile units.
	mask &= ~(MapFieldLandUnit | MapFieldAirUnit | MapFieldSeaUnit);

	points[0].X = x;
	points[0].Y = y;
	// also use the bottom right
	if ((type->TileWidth > 1 || type->TileHeight > 1) &&
			x + type->TileWidth - 1 < Map.Info.MapWidth &&
			y + type->TileHeight - 1 < Map.Info.MapHeight) {
		points[1].X = x + type->TileWidth - 1;
		points[1].Y = y + type->TileHeight - 1;
		ep = wp = 2; // start with two points
	} else {
		ep = wp = 1; // start with one point
	}
	matrix += w + w + 2;
	rp = 0;
	matrix[x + y * w] = 1; // mark start point

	//
	// Pop a point from stack, push all neighbours which could be entered.
	//
	for (;;) {
		while (rp != ep) {
			rx = points[rp].X;
			ry = points[rp].Y;
			for (i = 0; i < 8; ++i) { // mark all neighbors
				x = rx + xoffset[i];
				y = ry + yoffset[i];
				m = matrix + x + y * w;
				if (*m) { // already checked
					continue;
				}

				//
				// Look if we can build here.
				//
				if (CanBuildUnitType(worker, type, x, y, 1) &&
					(/*!flag ||*/ AiCheckSurrounding(worker, type, x, y, flag))) {
					*dx = x;
					*dy = y;
					delete[] points;
					return 1;
				}

				if (CanMoveToMask(x, y, mask)) { // reachable
					*m = 1;
					points[wp].X = x; // push the point
					points[wp].Y = y;
					if (++wp >= size) { // round about
						wp = 0;
					}
				} else { // unreachable
					*m = 99;
				}
			}

			if (++rp >= size) { // round about
				rp = 0;
			}
		}

		//
		// Continue with next frame.
		//
		if (rp == wp) { // unreachable, no more points available
			break;
		}
		ep = wp;
	}

	delete[] points;

	return 0;
}

/**
**  Find building place for hall. (flood fill version)
**
**  The best place:
**  1) near to goldmine.
**  !2) near to wood.
**  !3) near to worker and must be reachable.
**  4) no enemy near it.
**  5) no hall already near
**  !6) enough gold in mine
**
**  @param worker  Worker to build building.
**  @param type    Type of building.
**  @param dx      Pointer for X position returned.
**  @param dy      Pointer for Y position returned.
**
**  @return        True if place found, false if not found.
**
**  @todo          FIXME: This is slow really slow, using
**                 two flood fills, is not a perfect solution.
*/
static int AiFindHallPlace(const CUnit *worker, const CUnitType *type, int *dx, int *dy)
{
	static const int xoffset[] = { 0, -1, +1, 0, -1, +1, -1, +1 };
	static const int yoffset[] = { -1, 0, 0, +1, -1, -1, +1, +1 };
	struct p {
		unsigned short X;
		unsigned short Y;
	} *points;
	int size;
	int x;
	int y;
	int rx;
	int ry;
	int mask;
	int wp;
	int rp;
	int ep;
	int i;
	int w;
	unsigned char *m;
	unsigned char *morg;
	unsigned char *matrix;
	CUnit *mine;
	int destx;
	int desty;

	destx = x = worker->X;
	desty = y = worker->Y;
	size = Map.Info.MapWidth * Map.Info.MapHeight / 4;
	points = new p[size];

	//
	// Make movement matrix. FIXME: can create smaller matrix.
	//
	morg = MakeMatrix();
	w = Map.Info.MapWidth + 2;
	matrix = morg + w + w + 2;

	points[0].X = x;
	points[0].Y = y;
	rp = 0;
	matrix[x + y * w] = 1; // mark start point
	ep = wp = 1; // start with one point

	mask = worker->Type->MovementMask;

	//
	// Pop a point from stack, push all neighbors which could be entered.
	//
	for (;;) {
		while (rp != ep) {
			rx = points[rp].X;
			ry = points[rp].Y;
			for (i = 0; i < 8; ++i) { // mark all neighbors
				x = rx + xoffset[i];
				y = ry + yoffset[i];
				m = matrix + x + y * w;
				if (*m) { // already checked
					continue;
				}
				//
				// Look if there is a mine
				//
				if ((mine = ResourceOnMap(x, y, GoldCost))) {
					int buildings;
					int j;
					int minx;
					int maxx;
					int miny;
					int maxy;
					int nunits;
					CUnit *units[UnitMax];

					buildings = 0;

					//
					// Check units around mine
					//
					minx = mine->X - 5;
					if (minx < 0) {
						minx = 0;
					}
					miny = mine->Y - 5;
					if (miny < 0) {
						miny = 0;
					}
					maxx = mine->X + mine->Type->TileWidth + 5;
					if (maxx > Map.Info.MapWidth) {
						maxx = Map.Info.MapWidth;
					}
					maxy = mine->Y + mine->Type->TileHeight + 5;
					if (maxy > Map.Info.MapHeight) {
						maxy = Map.Info.MapHeight;
					}

					nunits = UnitCacheSelect(minx, miny, maxx, maxy, units);
					for (j = 0; j < nunits; ++j) {
						// Enemy near mine
						if (AiPlayer->Player->Enemy & (1 << units[j]->Player->Index)) {
							break;
						}
						// Town hall near mine
						if (units[j]->Type->CanStore[GoldCost]) {
							break;
						}
						// Town hall may not be near but we may be using it, check
						// for 2 buildings near it and assume it's been used
						if (units[j]->Type->Building &&
							!units[j]->Type->GivesResource == GoldCost) {
							++buildings;
							if (buildings == 2) {
								break;
							}
						}
					}
					if (j == nunits) {
						if (AiFindBuildingPlace2(worker, type, x, y, dx, dy, 0)) {
							delete[] morg;
							delete[] points;
							return 1;
						}
					}
				}

				if (CanMoveToMask(x, y, mask)) { // reachable
					*m = 1;
					points[wp].X = x; // push the point
					points[wp].Y = y;
					if (++wp >= size) { // round about
						wp = 0;
					}
				} else { // unreachable
					*m = 99;
				}
			}
			if (++rp >= size) { // round about
				rp = 0;
			}
		}

		//
		// Continue with next frame.
		//
		if (rp == wp) { // unreachable, no more points available
			break;
		}
		ep = wp;
	}

	delete[] morg;
	delete[] points;
	return 0;
}

/**
**  Find free building place.
**
**  @param worker  Worker to build building.
**  @param type    Type of building.
**  @param dx      Pointer for X position returned.
**  @param dy      Pointer for Y position returned.
**
**  @return        True if place found, false if no found.
**
**  @todo          Better and faster way to find building place of oil
**                 platforms Special routines for special buildings.
*/
int AiFindBuildingPlace(const CUnit *worker, const CUnitType *type, int *dx, int *dy)
{
	//
	// Find a good place for a new hall
	//
	DebugPrint("Want to build a %s(%s)\n" _C_ type->Ident.c_str() _C_ type->Name.c_str());
	if (type->CanStore[GoldCost] && AiFindHallPlace(worker, type, dx, dy)) {
		DebugPrint("Found place for town hall (%s,%s)\n" _C_ type->Ident.c_str() _C_
			type->Name.c_str());
		return 1;
	}

	// First try to find a place with free surroundings
	if (AiFindBuildingPlace2(worker, type, worker->X, worker->Y, dx, dy, 1)) {
		return 1;
	}

	// FIXME: Should do this if all units can't build better!
	// No place with free surroundings, try anything
	return AiFindBuildingPlace2(worker, type, worker->X, worker->Y, dx, dy, 0);
}

//@}

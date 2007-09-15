//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name edmap.cpp - Editor map functions. */
//
//      (c) Copyright 2002-2006 by Lutz Sammer
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
#include <limits.h>

#include "stratagus.h"
#include "editor.h"
#include "map.h"
#include "tileset.h"
#include "ui.h"
#include "player.h"
#include "unit.h"
#include "unittype.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

#define TH_QUAD_M 0xFFFF0000 /// Top half quad mask
#define BH_QUAD_M 0x0000FFFF /// Bottom half quad mask
#define LH_QUAD_M 0xFF00FF00 /// Left half quad mask
#define RH_QUAD_M 0x00FF00FF /// Right half quad mask

	/// Callback for changed tile (with direction mask)
static void EditorTileChanged2(int x, int y, int d);

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Get quad from tile.
**
**  A quad is a 32 bit value defining the content of the tile.
**
**  A tile is split into 4 parts, the basic tile type of this part
**    is stored as 8bit value in the quad.
**
**  ab
**  cd -> abcd
**
**  If the tile is 100% light grass the value is 0x5555.
**  If the tile is 3/4 light grass and dark grass in upper left corner
**    the value is 0x6555.
**
**  @param x  X map tile position
**  @param y  Y map tile position
**
**  @return   the 'quad' of the tile.
**
**  @todo     Make a lookup table to speed up the things.
*/
static unsigned QuadFromTile(int x, int y)
{
	int tile;
	int i;
	unsigned base;
	unsigned mix;

	//
	// find the abstact tile number
	//
	tile = Map.Field(x, y)->Tile;
	for (i = 0; i < Map.Tileset.NumTiles; ++i) {
		if (tile == Map.Tileset.Table[i]) {
			break;
		}
	}
	Assert(i != Map.Tileset.NumTiles);

	base = Map.Tileset.Tiles[i].BaseTerrain;
	mix = Map.Tileset.Tiles[i].MixTerrain;

	if (!mix) { // a solid tile
		return base | (base << 8) | (base << 16) | (base << 24);
	}
	//
	// Mixed tiles, mix together
	//
	switch ((i & 0x00F0) >> 4) {
		case 0:
			return (base << 24) | (mix << 16) | (mix << 8) | mix;
		case 1:
			return (mix << 24) | (base << 16) | (mix << 8) | mix;
		case 2:
			return (base << 24) | (base << 16) | (mix << 8) | mix;
		case 3:
			return (mix << 24) | (mix << 16) | (base << 8) | mix;
		case 4:
			return (base << 24) | (mix << 16) | (base << 8) | mix;
		case 5:
			return (base << 24) | (base << 16) | (base << 8) | mix;
		case 6:
			return (base << 24) | (base << 16) | (base << 8) | mix;
		case 7:
			return (mix << 24) | (mix << 16) | (mix << 8) | base;
		case 8:
			return (base << 24) | (mix << 16) | (mix << 8) | base;
		case 9:
			return (mix << 24) | (base << 16) | (mix << 8) | base;
		case 10:
			return (base << 24) | (base << 16) | (mix << 8) | base;
		case 11:
			return (mix << 24) | (mix << 16) | (base << 8) | base;
		case 12:
			return (base << 24) | (mix << 16) | (base << 8) | base;
		case 13:
			return (mix << 24) | (base << 16) | (base << 8) | base;
	}

	Assert(0);

	return base | (base << 8) | (base << 16) | (base << 24);
}

/**
**  Find a tile path.
**
**  @param base    Start tile type.
**  @param goal    Goal tile type.
**  @param length  Best found path length.
**  @param marks   Already visited tile types.
**  @param tile    Tile pointer.
*/
static int FindTilePath(int base, int goal, int length, char *marks, int *tile)
{
	int i;
	int l;
	int j;
	int n;

	//
	// Find any mixed tile
	//
	l = INT_MAX;
	for (i = 0; i < Map.Tileset.NumTiles;) {
		// goal found.
		if (base == Map.Tileset.Tiles[i].BaseTerrain &&
				goal == Map.Tileset.Tiles[i].MixTerrain) {
			*tile = i;
			return length;
		}
		// goal found.
		if (goal == Map.Tileset.Tiles[i].BaseTerrain &&
				base == Map.Tileset.Tiles[i].MixTerrain) {
			*tile = i;
			return length;
		}

		// possible path found
		if (base == Map.Tileset.Tiles[i].BaseTerrain &&
				Map.Tileset.Tiles[i].MixTerrain) {
			j = Map.Tileset.Tiles[i].MixTerrain;
			if (!marks[j]) {
				marks[j] = j;
				n = FindTilePath(j, goal, length + 1, marks, &n);
				marks[j] = 0;
				if (n < l) {
					*tile = i;
					l = n;
				}
			}
		// possible path found
		} else if (Map.Tileset.Tiles[i].BaseTerrain &&
				base == Map.Tileset.Tiles[i].MixTerrain) {
			j = Map.Tileset.Tiles[i].BaseTerrain;
			if (!marks[j]) {
				marks[j] = j;
				n = FindTilePath(j, goal, length + 1, marks, &n);
				marks[j] = 0;
				if (n < l) {
					*tile = i;
					l = n;
				}
			}
		}
		// Advance solid or mixed.
		if (!Map.Tileset.Tiles[i].MixTerrain) {
			i += 16;
		} else {
			i += 256;
		}
	}
	return l;
}

/**
**  Get tile from quad.
**
**  @param fixed  Part can't be changed.
**  @param quad   Quad of the tile type.
**  @return       Best matching tile.
*/
static int TileFromQuad(unsigned fixed, unsigned quad)
{
	int i;
	unsigned type1;
	unsigned type2;
	int base;
	int direction;
	// 0  1  2  3   4  5  6  7   8  9  A   B  C   D  E  F
	char table[16] = { 0, 7, 3, 11, 1, 9, 5, 13, 0, 8, 4, 12, 2, 10, 6, 0 };

	//
	// Get tile type from fixed.
	//
	while (!(type1 = (fixed & 0xFF))) {
		fixed >>= 8;
		if (!fixed) {
			abort();
		}
	}
	fixed >>= 8;
	while (!(type2 = (fixed & 0xFF)) && fixed) {
		fixed >>= 8;
	}
	//
	// Need an second type.
	//
	if (!type2 || type2 == type1) {
		fixed = quad;
		while ((type2 = (fixed & 0xFF)) == type1 && fixed) {
			fixed >>= 8;
		}
		if (type1 == type2) { // Oooh a solid tile.
find_solid:
			//
			// Find the solid tile
			//
			for (i = 0; i < Map.Tileset.NumTiles;) {
				if (type1 == Map.Tileset.Tiles[i].BaseTerrain &&
						!Map.Tileset.Tiles[i].MixTerrain) {
					break;
				}
				// Advance solid or mixed.
				if (!Map.Tileset.Tiles[i].MixTerrain) {
					i += 16;
				} else {
					i += 256;
				}
			}
			Assert(i < Map.Tileset.NumTiles);
			return i;
		}
	} else {
		char *marks;

		marks = new char[Map.Tileset.NumTerrainTypes];
		memset(marks, 0, Map.Tileset.NumTerrainTypes);
		marks[type1] = type1;
		marks[type2] = type2;

		//
		// What fixed tile-type should replace the non useable tile-types.
		// FIXME: write a loop.
		//
		fixed = (quad >> 0) & 0xFF;
		if (fixed != type1 && fixed != type2) {
			quad &= 0xFFFFFF00;
			if (FindTilePath(type1, fixed, 0, marks, &i) <
					FindTilePath(type2, fixed, 0, marks, &i)) {
				quad |= type1 << 0;
			} else {
				quad |= type2 << 0;
			}
		}
		fixed = (quad >> 8) & 0xFF;
		if (fixed != type1 && fixed != type2) {
			quad &= 0xFFFF00FF;
			if (FindTilePath(type1, fixed, 0, marks, &i) <
					FindTilePath(type2, fixed, 0, marks, &i)) {
				quad |= type1 << 8;
			} else {
				quad |= type2 << 8;
			}
		}
		fixed = (quad >> 16) & 0xFF;
		if (fixed != type1 && fixed != type2) {
			quad &= 0xFF00FFFF;
			if (FindTilePath(type1, fixed, 0, marks, &i) <
					FindTilePath(type2, fixed, 0, marks, &i)) {
				quad |= type1 << 16;
			} else {
				quad |= type2 << 16;
			}
		}
		fixed = (quad >> 24) & 0xFF;
		if (fixed != type1 && fixed != type2) {
			quad &= 0x00FFFFFF;
			if (FindTilePath(type1, fixed, 0, marks, &i) <
					FindTilePath(type2, fixed, 0, marks, &i)) {
				quad |= type1 << 24;
			} else {
				quad |= type2 << 24;
			}
		}

		delete[] marks;
	}

	//
	// Need a mixed tile
	//
	for (i = 0; i < Map.Tileset.NumTiles;) {
		if (type1 == Map.Tileset.Tiles[i].BaseTerrain &&
				type2 == Map.Tileset.Tiles[i].MixTerrain) {
			break;
		}
		if (type2 == Map.Tileset.Tiles[i].BaseTerrain &&
				type1 == Map.Tileset.Tiles[i].MixTerrain) {
			// Other mixed
			type1 ^= type2;
			type2 ^= type1;
			type1 ^= type2;
			break;
		}
		// Advance solid or mixed.
		if (!Map.Tileset.Tiles[i].MixTerrain) {
			i += 16;
		} else {
			i += 256;
		}
	}

	if (i >= Map.Tileset.NumTiles) {
		char *marks;

		//
		// Find the best tile path.
		//
		marks = new char[Map.Tileset.NumTerrainTypes];
		memset(marks, 0, Map.Tileset.NumTerrainTypes);
		marks[type1] = type1;
		if (FindTilePath(type1, type2, 0, marks, &i) == INT_MAX) {
			DebugPrint("Huch, no mix found!!!!!!!!!!!\n");
			delete[] marks;
			goto find_solid;
		}
		if (type1 == Map.Tileset.Tiles[i].MixTerrain) {
			// Other mixed
			type1 ^= type2;
			type2 ^= type1;
			type1 ^= type2;
		}

		delete[] marks;
	}

	base = i;

	direction = 0;
	if (((quad >> 24) & 0xFF) == type1) {
		direction |= 8;
	}
	if (((quad >> 16) & 0xFF) == type1) {
		direction |= 4;
	}
	if (((quad >> 8) & 0xFF) == type1) {
		direction |= 2;
	}
	if (((quad >> 0) & 0xFF) == type1) {
		direction |= 1;
	}

	return base | (table[direction] << 4);
}

/**
**  Change tile from abstract tile-type.
**
**  @param x     X map tile coordinate.
**  @param y     Y map tile coordinate.
**  @param tile  Abstract tile type to edit.
**
**  @note  this is a rather dumb function, doesn't do any tile fixing.
*/
void ChangeTile(int x, int y, int tile)
{
	CMapField *mf;

	Assert(x >= 0 && y >= 0 && x < Map.Info.MapWidth && y < Map.Info.MapHeight);
	Assert(tile >= 0 && tile < Map.Tileset.NumTiles);

	mf = Map.Field(x, y);
	mf->Tile = mf->SeenTile = Map.Tileset.Table[tile];
}

#define DIR_UP     8 /// Go up allowed
#define DIR_DOWN   4 /// Go down allowed
#define DIR_LEFT   2 /// Go left allowed
#define DIR_RIGHT  1 /// Go right allowed

/**
**  Editor change tile.
**
**  @param x     X map tile coordinate.
**  @param y     Y map tile coordinate.
**  @param tile  Tile type to edit.
**  @param d     Fix direction flag 8 up, 4 down, 2 left, 1 right.
*/
static void EditorChangeTile(int x, int y, int tile, int d)
{
	CMapField *mf;

	Assert(x >= 0 && y >= 0 && x < Map.Info.MapWidth && y < Map.Info.MapHeight);

	ChangeTile(x, y, tile);

	//
	// Change the flags
	//
	mf = Map.Field(x, y);
	mf->Flags &= ~(MapFieldLandAllowed | MapFieldCoastAllowed |
		MapFieldWaterAllowed | MapFieldNoBuilding | MapFieldUnpassable);

	mf->Flags |= Map.Tileset.FlagsTable[tile];

	UI.Minimap.UpdateSeenXY(x, y);
	UI.Minimap.UpdateXY(x, y);

	EditorTileChanged2(x, y, d);
}

/**
**  Update surroundings for tile changes.
**
**  @param x  Map X tile position of change.
**  @param y  Map Y tile position of change.
**  @param d  Fix direction flag 8 up, 4 down, 2 left, 1 right.
*/
static void EditorTileChanged2(int x, int y, int d)
{
	unsigned quad;
	unsigned q2;
	unsigned u;
	int tile;

	quad = QuadFromTile(x, y);

	//
	// Change the surrounding
	//

	//
	// How this works:
	//  first get the quad of the neighbouring tile, then
	//  check if the margin matches. otherwise, call
	//  EditorChangeTile again.
	//
	if (d & DIR_UP && y) {
		//
		// Insert into the bottom the new tile.
		//
		q2 = QuadFromTile(x, y - 1);
		u = (q2 & TH_QUAD_M) | ((quad >> 16) & BH_QUAD_M);
		if (u != q2) {
			tile = TileFromQuad(u & BH_QUAD_M, u);
			EditorChangeTile(x, y - 1, tile, d&~DIR_DOWN);
		}
	}
	if (d & DIR_DOWN && y < Map.Info.MapHeight - 1) {
		//
		// Insert into the top the new tile.
		//
		q2 = QuadFromTile(x, y + 1);
		u = (q2 & BH_QUAD_M) | ((quad << 16) & TH_QUAD_M);
		if (u != q2) {
			tile = TileFromQuad(u & TH_QUAD_M, u);
			EditorChangeTile(x, y + 1, tile, d&~DIR_UP);
		}
	}
	if (d & DIR_LEFT && x) {
		//
		// Insert into the left the new tile.
		//
		q2 = QuadFromTile(x - 1, y);
		u = (q2 & LH_QUAD_M) | ((quad >> 8) & RH_QUAD_M);
		if (u != q2) {
			tile = TileFromQuad(u & RH_QUAD_M, u);
			EditorChangeTile(x - 1, y, tile, d&~DIR_RIGHT);
		}
	}
	if (d & DIR_RIGHT && x < Map.Info.MapWidth - 1) {
		//
		// Insert into the right the new tile.
		//
		q2 = QuadFromTile(x + 1, y);
		u = (q2 & RH_QUAD_M) | ((quad << 8) & LH_QUAD_M);
		if (u != q2) {
			tile = TileFromQuad(u & LH_QUAD_M, u);
			EditorChangeTile(x + 1, y, tile, d&~DIR_LEFT);
		}
	}
}

/**
**  Update surroundings for tile changes.
**
**  @param x  Map X tile position of change.
**  @param y  Map Y tile position of change.
*/
void EditorTileChanged(int x, int y)
{
	EditorTileChanged2(x, y, 0xF);
}

/**
**  Make random map
**  FIXME: vladi: we should have parameters control here...
*/

/**
**  TileFill
**
**  @param x     X map tile coordinate for area center.
**  @param y     Y map tile coordinate for area center.
**  @param tile  Tile type to edit.
**  @param size  Size of surrounding rectangle.
**
**  TileFill(centerx, centery, tile_type_water, map_width)
**  will fill map with water...
*/
static void TileFill(int x, int y, int tile, int size)
{
	int ix;
	int ax;
	int iy;
	int ay;


	ix = x - size / 2;
	ax = x + size / 2;
	iy = y - size / 2;
	ay = y + size / 2;

	if (ix < 0) {
		ix = 0;
	}
	if (ax >= Map.Info.MapWidth) {
		ax = Map.Info.MapWidth - 1;
	}
	if (iy < 0) {
		iy = 0;
	}
	if (ay >= Map.Info.MapHeight) {
		ay = Map.Info.MapHeight - 1;
	}

	for (x = ix; x <= ax; ++x) {
		for (y = iy; y <= ay; ++y) {
			EditorChangeTile(x, y, tile, 15);
		}
	}
}

#define WATER_TILE  0x10
#define COAST_TILE  0x30
#define GRASS_TILE  0x50
#define WOOD_TILE   0x70
#define ROCK_TILE   0x80

/**
**  Randomize tiles and fill in map
**
**  @param tile      tile number to use
**  @param count     number of times to apply randomization
**  @param max_size  maximum size of the fill rectangle
*/
static void EditorRandomizeTile(int tile, int count, int max_size)
{
	int mx;
	int my;
	int i;
	int rx;
	int ry;
	int rz;

	mx = Map.Info.MapWidth;
	my = Map.Info.MapHeight;

	for (i = 0; i < count; ++i) {
		rx = rand() % (mx / 2);
		ry = rand() % (my / 2);
		rz = rand() % max_size + 1;

		TileFill(rx, ry, tile, rz);
		TileFill(mx - rx - 1, ry, tile, rz);
		TileFill(rx, my - ry - 1, tile, rz);
		TileFill(mx - rx - 1, mx - ry - 1, tile, rz);
	}
}

/**
**  Destroy all units
*/
static void EditorDestroyAllUnits(void)
{
	CUnit *unit;

	while (NumUnits != 0) {
		unit = Units[0];
		unit->Remove(NULL);
		UnitLost(unit);
		UnitClearOrders(unit);
		unit->Release();
	}
}

/**
**  Create a random map
*/
void CEditor::CreateRandomMap(void) const
{
	int mz;

	mz = std::max(Map.Info.MapWidth, Map.Info.MapHeight);

	// make water-base
	TileFill(0, 0, WATER_TILE, mz * 3);
	// remove all units
	EditorDestroyAllUnits();

	EditorRandomizeTile(COAST_TILE, 10, 16);
	EditorRandomizeTile(GRASS_TILE, 20, 16);
	EditorRandomizeTile(WOOD_TILE,  60,  4);
	EditorRandomizeTile(ROCK_TILE,  30,  2);
}

//@}

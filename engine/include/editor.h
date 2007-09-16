//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name editor.h - The editor file. */
//
//      (c) Copyright 2002-2007 by Lutz Sammer
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

#ifndef __EDITOR_H__
#define __EDITOR_H__

//@{

#include <vector>
#include <string>
#include "icons.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnitType;

enum EditorRunningType {
	EditorNotRunning = 0,    /// Not Running
	EditorStarted = 1,       /// Editor Enabled at all
	EditorCommandLine = 2,   /// Called from Command Line
	EditorEditing = 4,       /// Editor is fully running
};

enum EditorStateType {
	EditorSelecting,         /// Select
	EditorEditTile,          /// Edit tiles
	EditorEditUnit,          /// Edit units
	EditorSetStartLocation   /// Set the start location
};

class CEditor {
public:
	CEditor() : TerrainEditable(true), StartUnit(NULL),
		UnitIndex(0), CursorUnitIndex(-1), SelectedUnitIndex(-1),
		CursorPlayer(-1), SelectedPlayer(-1),
		MapLoaded(false), WriteCompressedMaps(true)
		{};
	~CEditor() {};

	void Init();
	/// Make random map
	void CreateRandomMap() const;


	std::vector<std::string> UnitTypes;             /// Sorted editor unit-type table.
	std::vector<const CUnitType *> ShownUnitTypes;  /// Shown editor unit-type table.

	bool TerrainEditable;        /// Is the terrain editable ?
	IconConfig Select;           /// Editor's select icon.
	IconConfig Units;            /// Editor's units icon.
	std::string StartUnitName;   /// name of the Unit used to display the start location.
	const CUnitType *StartUnit;  /// Unit used to display the start location.

	int UnitIndex;               /// Unit icon draw index.
	int CursorUnitIndex;         /// Unit icon under cursor.
	int SelectedUnitIndex;       /// Unit type to draw.

	int CursorPlayer;            /// Player under the cursor.
	int SelectedPlayer;          /// Player selected for draw.

	bool MapLoaded;              /// Map loaded in editor
	bool WriteCompressedMaps;    /// Use compression when saving

	EditorRunningType Running;   /// Editor is running

	EditorStateType State;       /// Current editor state
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern CEditor Editor;

extern const char *EditorStartFile;  /// Editor CCL start file

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

	/// Start the editor
extern void StartEditor(const char *filename);

	/// Editor main event loop
extern void EditorMainLoop(void);
	/// Update editor display
extern void EditorUpdateDisplay(void);

	/// Save a map from editor
extern int EditorSaveMap(const char *file);

	/// Register ccl features
extern void EditorCclRegister(void);

	/// Edit tile
extern void EditTile(int x, int y, int tile);
	/// Edit tiles
extern void EditTiles(int x, int y, int tile, int size);

	/// Change the view of a tile
extern void ChangeTile(int x, int y, int tile);
	/// Update surroundings for tile changes
extern void EditorTileChanged(int x, int y);


//@}

#endif // !__EDITOR_H__

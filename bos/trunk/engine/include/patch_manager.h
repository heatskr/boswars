//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name patch.h - The patch manager header. */
//
//      (c) Copyright 2008 by Jimmy Salmon
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

#ifndef _PATCH_MANAGER_H_
#define _PATCH_MANAGER_H_

//@{

#include <list>
#include <map>
#include <vector>

class CPatchType;
class CPatch;
class CFile;


class CPatchManager
{
public:
	/**
	**  Patch manager constructor
	*/
	CPatchManager();

	/**
	**  Patch manager destructor
	*/
	~CPatchManager();

	/**
	**  Create a new patch of type typeName at location x, y
	*/
	CPatch *add(const std::string &typeName, int x, int y);

	/**
	**  Move a patch to the top
	*/
	void moveToTop(CPatch *patch);

	/**
	**  Move a patch to the bottom
	*/
	void moveToBottom(CPatch *patch);

	/**
	**  Get the patch at location x, y
	*/
	CPatch *getPatch(int x, int y) const;

	/**
	**  Get all of the patches
	*/
	std::list<CPatch *> getPatches() const;

	/**
	**  Get all of the patch names
	*/
	std::vector<std::string> getPatchTypeNames() const;

	/**
	**  Load the patches used in the map
	*/
	void load();

	/**
	**  Clear the patches used in the map
	*/
	void clear();

	/**
	**  Define a new patch type.
	**  Types should be created only once and last for the duration of the game.
	*/
	CPatchType *newPatchType(const std::string &name, const std::string &file,
		int tileWidth, int tileHeight, int *flags);

	/**
	**  Define a new patch type.
	**  Types should be created only once and last for the duration of the game.
	*/
	CPatchType *newPatchType(const std::string &name, const std::string &file,
		int tileWidth, int tileHeight, unsigned short *flags);

	/**
	**  Get a patch type
	*/
	CPatchType *getPatchType(const std::string &name);

	/**
	**  Save a patch type
	*/
	void savePatchType(CFile *file, CPatchType *patchType);

private:
	std::list<CPatch *> patches;
	std::map<std::string, CPatchType *> patchTypesMap;
};

//@}

#endif /* _PATCH_MANAGER_H_ */


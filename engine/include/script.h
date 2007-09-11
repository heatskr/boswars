//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name script.h - The clone configuration language headerfile. */
//
//      (c) Copyright 1998-2007 by Lutz Sammer and Jimmy Salmon
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

#ifndef __SCRIPT_H__
#define __SCRIPT_H__

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <string>

#ifdef __cplusplus
extern "C" {
#endif
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnit;
class CFile;
class CFont;

typedef struct _lua_user_data_ {
	int Type;
	void *Data;
} LuaUserData;

enum {
	LuaUnitType = 100,
	LuaSoundType,
};

extern lua_State *Lua;

extern int LuaLoadFile(const std::string &file);
extern int LuaCall(int narg, int clear);

#define LuaError(l, args) \
	do { \
		fprintf(stdout, "%s:%d: ", __FILE__, __LINE__); \
		fprintf(stdout, args); \
		fprintf(stdout, "\n"); \
		lua_pushfstring(l, args); lua_error(l); \
	} while (0)

#define LuaCheckArgs(l, args) \
	do { \
		if (lua_gettop(l) != args) { \
			LuaError(l, "incorrect argument"); \
		} \
	} while (0)


typedef enum {
	ENumber_Lua,         /// a lua function.
	ENumber_Dir,         /// directly a number.
	ENumber_Add,         /// a + b.
	ENumber_Sub,         /// a - b.
	ENumber_Mul,         /// a * b.
	ENumber_Div,         /// a / b.
	ENumber_Min,         /// Min(a, b).
	ENumber_Max,         /// Max(a, b).
	ENumber_Rand,        /// Rand(a) : number in [0..a-1].

	ENumber_Gt,          /// a  > b.
	ENumber_GtEq,        /// a >= b.
	ENumber_Lt,          /// a  < b.
	ENumber_LtEq,        /// a <= b.
	ENumber_Eq,          /// a == b.
	ENumber_NEq,         /// a <> b.

	ENumber_VideoTextLength, /// VideoTextLength(font, string).
	ENumber_StringFind,      /// strchr(string, char) - s.


	ENumber_UnitStat     /// Property of Unit.
// FIXME: add others.
} ENumber; /// All possible value for a number.


typedef enum {
	EUnit_Ref           /// Unit direct reference.
// FIXME: add others.
} EUnit; /// All possible value for a unit.

typedef enum {
	EString_Lua,          /// a lua function.
	EString_Dir,          /// directly a string.
	EString_Concat,       /// a + b [+ c ...].
	EString_String,       /// Convert number in string.
	EString_InverseVideo, /// Inverse video for the string ("a" -> "~<a~>").
	EString_If,           /// If cond then String1 else String2.
	EString_UnitName,     /// UnitType Name.
	EString_SubString,    /// SubString.
	EString_Line          /// line n of the string.
// FIXME: add others.
} EString; /// All possible value for a string.

typedef enum {
	ES_GameInfo_Objectives       /// All Objectives of the game.
} ES_GameInfo; /// All possible value for a game info string.

/**
**  Enumeration to know which variable to be selected.
*/
typedef enum {
	VariableValue = 0,  /// Value of the variable.
	VariableMax,        /// Max of the variable.
	VariableIncrease,   /// Increase value of the variable.
	VariableDiff,       /// (Max - Value)
	VariablePercent,    /// (100 * Value / Max)
	VariableName        /// Name of the variable.
} EnumVariable;

/**
**  Enumeration of unit
*/

typedef enum {
	UnitRefItSelf = 0,      /// unit.
	UnitRefInside,          /// unit->Inside.
	UnitRefContainer,       /// Unit->Container.
	UnitRefWorker,          /// unit->Data.Built.Worker
	UnitRefGoal,            /// unit->Goal
} EnumUnit;


/**
**  Number description.
**  Use to describe complex number in script to use when game running.
*/
typedef struct _NumberDesc_ NumberDesc;

/**
** Unit description
**  Use to describe complex unit in script to use when game running.
*/
typedef struct _UnitDesc_ UnitDesc;


/**
** String description
**  Use to describe complex string in script to use when game running.
*/
typedef struct _StringDesc_ StringDesc;


typedef struct _binop_ {
	NumberDesc *Left;           /// Left operand.
	NumberDesc *Right;          /// Right operand.
} BinOp;  /// for Bin operand  a ?? b

/**
**  Number description.
*/
struct _NumberDesc_ {
	ENumber e;       /// which number.
	union {
		unsigned int Index; /// index of the lua function.
		int Val;       /// Direct value.
		NumberDesc *N; /// Other number.
		struct _binop_ BinOp;   /// For binary operand.
		struct {
			UnitDesc *Unit;            /// Which unit.
			int Index;                 /// Which index variable.
			EnumVariable Component;    /// Which component.
			int Loc;                   /// Location of Variables[].
		} UnitStat;
		struct {
			StringDesc *String; /// String.
			CFont *Font;        /// Font.
		} VideoTextLength;
		struct {
			StringDesc *String; /// String.
			char C;             /// Char.
		} StringFind;

	} D;
};

/**
**  Unit description.
*/
struct _UnitDesc_ {
	EUnit e;       /// which unit;
	union {
		CUnit **AUnit; /// Adress of the unit.
	} D;
};

/**
**  String description.
*/
struct _StringDesc_ {
	EString e;       /// which number.
	union {
		unsigned int Index; /// index of the lua function.
		char *Val;       /// Direct value.
		struct {
			StringDesc **Strings;  /// Array of operands.
			int n;                 /// number of operand to concat
		} Concat; /// for Concat two string.
		NumberDesc *Number;  /// Number.
		StringDesc *String;  /// String.
		UnitDesc *Unit;      /// Unit desciption.
		struct {
			NumberDesc *Cond;  /// Branch condition.
			StringDesc *True;  /// String if Cond is true.
			StringDesc *False; /// String if Cond is false.
		} If; /// conditional string.
		struct {
			StringDesc *String;  /// Original string.
			NumberDesc *Begin;   /// Begin of result string.
			NumberDesc *End;     /// End of result string.
		} SubString; /// For extract a substring
		struct {
			StringDesc *String;  /// Original string.
			NumberDesc *Line;    /// Line number.
			NumberDesc *MaxLen;  /// Max lenght of line.
			CFont *Font;         /// Font to consider (else (-1) consider just char).
		} Line; /// For specific line.
		ES_GameInfo GameInfoType;
	} D;
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern std::string CclStartFile;   /// CCL start file
extern int CclInConfigFile;        /// True while config file parsing

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern const char *LuaToString(lua_State *l, int narg);
extern int LuaToNumber(lua_State *l, int narg);
extern bool LuaToBoolean(lua_State *l, int narg);

extern void CclGarbageCollect(int fast);  /// Perform garbage collection
extern void InitCcl(void);                /// Initialise ccl
extern void LoadCcl(void);                /// Load ccl config file
extern void SaveCcl(CFile *file);        /// Save CCL module
extern void SavePreferences(void);        /// Save user preferences
extern int CclCommand(const char *command);/// Execute a ccl command

/// transform string in corresponding index.
extern EnumVariable Str2EnumVariable(lua_State *l, const char *s);
extern NumberDesc *CclParseNumberDesc(lua_State *l); /// Parse a number description.
extern UnitDesc *CclParseUnitDesc(lua_State *l);     /// Parse a unit description.
StringDesc *CclParseStringDesc(lua_State *l);        /// Parse a string description.

StringDesc *NewStringDesc(const char *s);            /// Create a StringDesc with const string.
extern int EvalNumber(const NumberDesc *numberdesc); /// Evaluate the number.
extern CUnit *EvalUnit(const UnitDesc *unitdesc);    /// Evaluate the unit.
char *EvalString(const StringDesc *s);               /// Evaluate the string.

void FreeNumberDesc(NumberDesc *number);  /// Free number description content. (no pointer itself).
void FreeUnitDesc(UnitDesc *unitdesc);    /// Free unit description content. (no pointer itself).
void FreeStringDesc(StringDesc *s);       /// Frre string description content. (no pointer itself).


//@}

#endif // !__CCL_H__

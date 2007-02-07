--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	unit-assault.lua	-	Define the assault unit
--
--	(c) Copyright 2001 - 2005 by Fran�ois Beerten, Lutz Sammer and Crestez Leonard
--
--      This program is free software; you can redistribute it and/or modify
--      it under the terms of the GNU General Public License as published by
--      the Free Software Foundation; either version 2 of the License, or
--      (at your option) any later version.
--  
--      This program is distributed in the hope that it will be useful,
--      but WITHOUT ANY WARRANTY; without even the implied warranty of
--      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--      GNU General Public License for more details.
--  
--      You should have received a copy of the GNU General Public License
--      along with this program; if not, write to the Free Software
--      Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
--
--	$Id$

DefineIcon({
        Name = "icon-assault",
        Size = {46, 38},
        Frame = 0,
        File = GetCurrentLuaPath().."/ico_assault.png"})

DefineAnimations("animations-assault", {
    Still = {"frame 0", "wait 1", },
    Move = {"unbreakable begin", "frame 5", "move 2", "wait 1", 
        "frame 5", "move 2", "wait 1", "frame 10", "move 2", "wait 1", 
        "frame 10", "move 2", "wait 1", "frame 15", "move 2", "wait 1", 
        "frame 15", "move 2", "wait 1", "frame 20", "move 2", "wait 1", 
        "frame 20", "move 2", "wait 1", "frame 5", "move 2", "wait 1", 
        "frame 5", "move 2", "wait 1", "frame 10", "move 2", "wait 1", 
        "frame 10", "move 2", "wait 1", "frame 15", "move 2", "wait 1", 
        "frame 15", "move 2", "wait 1", "frame 20", "move 2", "wait 1", 
        "frame 20", "move 2", "wait 1", "frame 20", "unbreakable end", "wait 1", },
    Attack = {"unbreakable begin", 
        "frame 25", "sound assault-attack", "attack", "wait 1", 
        "frame 0", "wait 1", "frame 25", "wait 1", "frame 0", "wait 1", 
        "frame 25", "wait 1", "frame 0", "wait 1", "frame 25", "wait 1", 
        "frame 0", "wait 1", "frame 25", "wait 1", "frame 0", "wait 1", 
        "frame 25", "attack", "wait 1", "frame 0", "wait 24", 
        "frame 0", "unbreakable end", "wait 1", },
    Death = {"unbreakable begin", "frame 30", "wait 5", "frame 35", "wait 5", 
        "frame 40", "wait 5", "frame 45", "unbreakable end", "wait 5", },
    })

MakeSound("assault-selected", GetCurrentLuaPath().."/assault_select.wav")
MakeSound("assault-acknowledge", GetCurrentLuaPath().."/assault_action.wav")
MakeSound("assault-ready", GetCurrentLuaPath().."/assault.unit.ready.wav")
MakeSound("assault-help", GetCurrentLuaPath().."/assault.unit.underattack.wav")
MakeSound("assault-die", GetCurrentLuaPath().."/assault_die.wav")
MakeSound("assault-attack", GetCurrentLuaPath().."/assault_attack.wav")

DefineUnitType("unit-assault", {
	Name = "Assault Unit",
	Image = {"file", GetCurrentLuaPath().."/unit_assault.png", "size", {64, 64}},
	Shadow = {"file", GetCurrentLuaPath().."/unit_assault_s.png", "size", {64, 64}},
	Animations = "animations-assault", Icon = "icon-assault",
	Costs = {"time", 30, "titanium", 25, "crystal", 50},
	Speed = 10, HitPoints = 30, DrawLevel = 25,
	TileSize = {1, 1}, BoxSize = {17, 28},
	RightMouseAction = "attack",
	SightRange = 6, ComputerReactionRange = 6, PersonReactionRange = 6,
	Armor = 3, BasicDamage = 4, PiercingDamage = 0, Missile = "missile-none",
	MaxAttackRange = 5, Priority = 60, Points = 50,
	Corpse = "unit-dead-body1", Type = "land", Demand = 0,
	CanAttack = true, CanTargetLand = true, CanTargetAir = true,
	LandUnit = true, organic = true, SelectableByRectangle = true,
	Sounds = {
		"selected", "assault-selected",
		"acknowledge", "assault-acknowledge",
		"ready", "assault-ready",
		"help", "assault-help",
		"dead", "assault-die"}
	})
DefineHumanCorpse("assault")

DefineAllow("unit-assault", "AAAAAAAA")

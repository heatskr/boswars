--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	unit-rocksfield.lua	-	Define the rocks-field.
--
--	(c) Copyright 1998 - 2005 by Lutz Sammer, Crestez Leonard, Fran�ois Beerten
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
        Name = "icon-rocks_field",
        Size = {46, 38},
        Frame = 0,
        File = "units/rocksfield/ico_rocks_field.png"})

-- Note: the name of the unit is hardcoded as unit-gold-mine in the editor.
DefineUnitType("unit-gold-mine", {
	Name = "Energy Field",
	Image = {"file", "units/rocksfield/rocks_field.png", "size", {128, 128}},
	Shadow = {"file", "units/rocksfield/rocks_field_s.png", "size", {128, 128}, "offset", {0, 0}},
	Animations = "animations-building", Icon = "icon-rocks_field",
	ProductionCosts = {},--[["time", 150},]] VisibleUnderFog = true,
	Construction = "construction-land2",
	NeutralMinimapColor = {196, 196, 196},
	DrawLevel = 40, TileSize = {4, 4}, BoxSize = {96, 96},
	SightRange = 1, Speed = 0, HitPoints = 25500, Priority = 0,
	Armor = 20, BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	Corpse = "unit-destroyed-4x4-place", ExplodeWhenKilled = "missile-explosion",
	Type = "land", Building = true, CanHarvestFrom = true
    --[[
	Sounds = {
		"selected", "gold-mine-selected",
		"acknowledge", "gold-mine-acknowledge",
		"ready", "gold-mine-ready",
		"help", "gold-mine-help",
		"dead", "building destroyed"}
    ]]
    })


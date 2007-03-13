--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	units.lua	-	Define the used unit-types.
--
--	(c) Copyright 1998 - 2007 by Lutz Sammer, Crestez Leonard
--                               and Francois Beerten.
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

-- Load the animations for the units.
Load("scripts/anim.lua")

AllowedUnits = {}

local oldDefineAllow = DefineAllow
function DefineAllow(unit, access)
   AllowedUnits[unit] = access
end

function AllowDefaultUnits()
   for unit, default in pairs(AllowedUnits) do
      DefineAllow(unit, default)
   end 
end

function DisallowAllUnits()
   for unit, default in pairs(AllowedUnits) do
      DefineAllow(unit, "FFFFFFFF")
   end
end

function AllowAllUnits()
   for unit, default in pairs(AllowedUnits) do
      DefineAllow(unit, "AAAAAAAA")
   end
end

function DefineCommonButtons(forUnits) 
   DefineButton({
        Pos = 1, Level = 0, Icon = "icon-move",
        Action = "move", Key = "m", Hint = "~!MOVE",
        ForUnit = forUnits})
   DefineButton({
        Pos = 2, Level = 0, Icon = "icon-stop",
        Action = "stop", Key = "s", Hint = "~!STOP",
        ForUnit = forUnits})
   DefineButton({
        Pos = 3, Level = 0, Icon = "icon-attack",
        Action = "attack", Key = "a", Hint = "~!ATTACK", ForUnit = forUnits})
   DefineButton({
        Pos = 4, Level = 0, Icon = "icon-patrol",
        Action = "patrol", Key = "p", Hint = "~!PATROL", ForUnit = forUnits})
   DefineButton({
        Pos = 5, Level = 0, Icon = "icon-stand-ground",
        Action = "stand-ground", Key = "t", Hint = "S~!TAND GROUND", 
        ForUnit = forUnits})
end

DefineAnimations("animations-elitecorpse1", {
    Death = {"unbreakable begin", "wait 1", "frame 20", "wait 2000", 
        "frame 0", "wait 200", "frame 5", "wait 200", "frame 10", "wait 200", 
        "frame 15", "wait 200", "frame 15", "wait 1", "unbreakable end", "wait 1", },
    })

function DefineHumanCorpse(livingunit, size)
   if (size == nil) then
    size = {64, 64}
   end

   DefineUnitType("unit-dead-" .. livingunit, {
	Name = livingunit .. "body",
	Image = {"file", GetCurrentLuaPath().."/unit_" .. livingunit .. "_c.png", "size", size},
	Animations = "animations-elitecorpse1", Icon = "icon-cancel",
	Speed = 0, HitPoints = 999, DrawLevel = 10, TileSize = {1, 1},
	BoxSize = {31, 31}, SightRange = 1, BasicDamage = 0,
	PiercingDamage = 0, Missile = "missile-none",
	Priority = 0, Type = "land", Vanishes = true})
end


DefineUnitType("unit-dead-body", {
	Name= "Dead Body",
	Image = {"file", "neutral/units/corpses.png", "size", {72, 72}},
	Animations = "animations-dead-body", Icon = "icon-cancel",
	Speed = 0, HitPoints = 255, DrawLevel = 30, Priority = 0,
	TileSize = {1, 1}, BoxSize = {31, 31}, SightRange = 1,
	BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	Type = "land", Vanishes = true})

DefineUnitType("unit-destroyed-1x1-place", { Name = "Destroyed 1x1 Place",
	Image = {"file", "neutral/small_destroyed_site.png", "size", {32, 32}},
	Animations = "animations-destroyed-place", Icon = "icon-cancel",
	Speed = 0, HitPoints = 255, DrawLevel = 10,
	TileSize = {1, 1}, BoxSize = {31, 31}, SightRange = 2,
	BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	Priority = 0, Type = "land",
	Building = true, VisibleUnderFog = true, Vanishes = true})

DefineUnitType("unit-destroyed-2x2-place", { Name = "Destroyed 2x2 Place",
	Image = {"file", "neutral/destroyed_site.png", "size", {64, 64}},
	Animations = "animations-destroyed-place", Icon = "icon-cancel",
	Speed = 0, HitPoints = 255, DrawLevel = 10,
	TileSize = {2, 2}, BoxSize = {63, 63}, SightRange = 2,
	BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	Priority = 0, Type = "land",
	Building = true, VisibleUnderFog = true, Vanishes = true})

DefineUnitType("unit-destroyed-3x3-place", { Name = "Destroyed 3x3 Place",
	Image = {"file", "neutral/destroyed_site.png", "size", {64, 64}},
	Animations = "animations-destroyed-place", Icon = "icon-cancel",
	Speed = 0, HitPoints = 255, DrawLevel = 10,
	TileSize = {3, 3}, BoxSize = {95, 95}, SightRange = 3,
	BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	Priority = 0, Type = "land",
	Building = true, VisibleUnderFog = true, Vanishes = true})

DefineUnitType("unit-destroyed-4x4-place", { Name = "Destroyed 4x4 Place",
	Image = {"file", "neutral/destroyed_site.png", "size", {64, 64}},
	Animations = "animations-destroyed-place", Icon = "icon-cancel",
	Speed = 0, HitPoints = 255, DrawLevel = 10,
	TileSize = {4, 4}, BoxSize = {127, 127}, SightRange = 3,
	BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	Priority = 0, Type = "land",
	Building = true, VisibleUnderFog = true, Vanishes = true})

DefineUnitType("unit-revealer", {
	Name = "Dummy unit",
	Animations = "animations-building", Icon = "icon-cancel",
	Speed = 0, HitPoints = 0,
	TileSize = {1, 1}, BoxSize = {1, 1},
	SightRange = 12,
	BasicDamage = 0, PiercingDamage = 0, Missile = "missile-none",
	Priority = 0, DecayRate = 1, Type = "land",
	Building = true, Revealer = true})

DefineAllow("unit-dead-body", "AAAAAAAA")
DefineAllow("unit-destroyed-1x1-place", "AAAAAAAA")
DefineAllow("unit-destroyed-2x2-place", "AAAAAAAA")
DefineAllow("unit-destroyed-3x3-place", "AAAAAAAA")
DefineAllow("unit-destroyed-4x4-place", "AAAAAAAA")

-- Load production buildings
Load("units/vault/vault.lua")
Load("units/engineer/engineer.lua")
Load("units/vehiclefactory/vehiclefactory.lua")

-- Find and load all other units
local list
local i
local f
local ff

list = ListDirsInDirectory("units/")
for i,f in ipairs(list) do
  if not(string.find(f, "^%.")) then
     local subdirlist = ListFilesInDirectory("units/" .. f)
     for ii,ff in ipairs(subdirlist) do
        if (string.find(ff, "^unit-.*%.lua$")) then
          print("Loading unit: " .. ff)
          Load("units/"..f.."/"..ff)
        end
     end
  end
end

-- restore the old DefineAllow function
-- TODO: use another name in the unit scripts
DefineAllow = oldDefineAllow

--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--	fonts.lua	-	Define the used fonts.
--
--	(c) Copyright 2000-2003 by Lutz Sammer, Jimmy Salmon and Crestez Leonard
--
--      This program is free software-- you can redistribute it and/or modify
--      it under the terms of the GNU General Public License as published by
--      the Free Software Foundation-- either version 2 of the License, or
--      (at your option) any later version.
--  
--      This program is distributed in the hope that it will be useful,
--      but WITHOUT ANY WARRANTY-- without even the implied warranty of
--      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--      GNU General Public License for more details.
--  
--      You should have received a copy of the GNU General Public License
--      along with this program-- if not, write to the Free Software
--      Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
--
--	$Id$

-- in CGraphic:New, with png files generated by ttf2png, numbers are: 
-- width/32 and height/16

CFont:New("small", CGraphic:New("general/dejavusans10.png", 13, 14))
--CFont:New("small", CGraphic:New("general/dejavusansbold10.png", 15, 14))
CFont:New("game", CGraphic:New("general/dejavusans12.png", 17, 16))
--CFont:New("game", CGraphic:New("general/dejavusansbold12.png", 19, 16))
--CFont:New("large", CGraphic:New("general/freesansbold14.png", 18, 19))
CFont:New("large", CGraphic:New("general/dejavusans14.png", 19, 17))
--CFont:New("large", CGraphic:New("general/dejavusansbold14.png", 22, 18))

--	FIXME: only yellow, white, and grey are correct.

function DefineFontColor(id, t)
  fc = CFontColor:New(id)
  for i = 0,(table.getn(t) / 3 - 1) do
    fc.Colors[i] = CColor(t[i * 3 + 1], t[i * 3 + 2], t[i * 3 + 3])
  end
end

DefineFontColor("black",
  {    0,   0,   0,
      40,  40,  60,
      40,  40,  60,
      40,  40,  60,
      40,  40,  60,
       0,   0,   0,
       0,   0,   0})
DefineFontColor("red",
  {    0,   0,   0,
     164,   0,   0,
     124,   0,   0,
      92,   4,   0,
      68,   4,   0,
       0,   0,   0,
       0,   0,   0})
DefineFontColor("green",
  {    0,   0,   0,
      44, 180, 148,
      44, 180, 148,
      44, 180, 148,
      44, 180, 148,
       0,   0,   0,
       0,   0,   0})
DefineFontColor("yellow",
  {  252, 248, 240,
     244, 224,  32,
     208, 192,  28,
     168, 140,  16,
      92,  48,   0,
       0,   0,   0,
     108, 108, 108})
DefineFontColor("blue",
  {    0,   0,   0,
       0, 148, 252,
       0, 148, 252,
       0, 148, 252,
       0, 148, 252,
       0,   0,   0,
       0,   0,   0})
DefineFontColor("magenta",
  {    0,   0,   0,
     152,  72, 176,
     152,  72, 176,
     152,  72, 176,
     152,  72, 176,
       0,   0,   0,
       0,   0,   0})
DefineFontColor("cyan",
  {    0,   0,   0,
     248, 140, 140,
     248, 140, 140,
     248, 140, 140,
     248, 140, 140,
       0,   0,   0,
       0,   0,   0})
DefineFontColor("white",
  {    0,   0,   0,
     168, 168, 168,
     252, 248, 240,
     252, 248, 240,
     108, 108, 108,
       0,   0,   0,
       0,   0,   0})
DefineFontColor("grey",
  {    0,   0,   0,
     192, 192, 192,
     180, 180, 180,
     168, 168, 168,
     108, 108, 108,
       0,   0,   0,
       0,   0,   0})
DefineFontColor("light-red",
  {    0,   0,   0,
     255,   0,   0,
     255,   0,   0,
     255,   0,   0,
     255,   0,   0,
       0,   0,   0,
       0,   0,   0})
DefineFontColor("light-green",
  {    0,   0,   0,
      44, 180,  44,
      44, 180,  44,
      44, 180,  44,
      44, 180,  44,
       0,   0,   0,
       0,   0,   0})
DefineFontColor("light-yellow",
  {  252, 248, 240,
     244, 224,  32,
     208, 192,  28,
     168, 140,  16,
      92,  48,   0,
       0,   0,   0,
     108, 108, 108})
DefineFontColor("light-blue",
  {    0,   0,   0,
     100, 158, 255,
     100, 158, 255,
     100, 158, 255,
     100, 158, 255,
       0,   0,   0,
       0,   0,   0})
DefineFontColor("light-magenta",
  {    0,   0,   0,
     152,  72, 176,
     152,  72, 176,
     152,  72, 176,
     152,  72, 176,
       0,   0,   0,
       0,   0,   0})
DefineFontColor("light-cyan",
  {    0,   0,   0,
     248, 140,  20,
     248, 140,  20,
     248, 140,  20,
     248, 140,  20,
       0,   0,   0,
       0,   0,   0})
DefineFontColor("light-grey",
  {    0,   0,   0,
     192, 192, 192,
     180, 180, 180,
     168, 168, 168,
     108, 108, 108,
       0,   0,   0,
       0,   0,   0})

DefineFontColor("violet",
  {    0,   0,   0,
     152,  72, 176,
     152,  72, 176,
     152,  72, 176,
     152,  72, 176,
       0,   0,   0,
       0,   0,   0})
DefineFontColor("orange",
  {    0,   0,   0,
     248, 140,  20,
     248, 140,  20,
     248, 140,  20,
     248, 140,  20,
       0,   0,   0,
       0,   0,   0})


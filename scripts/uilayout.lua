--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--      uilayout.lua - Define the ingame layout and user interface
--
--      (c) Copyright 2001-2006 by Lutz Sammer, Jimmy Salmon, Crestez Leonard,
--                                 and Francois Beerten.
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
--      $Id$


function DefineElitePanels(info_panel_x, info_panel_y) 
   local min_damage = Max(1, Div(ActiveUnitVar("PiercingDamage"), 2))
   local max_damage = Add(ActiveUnitVar("PiercingDamage"), ActiveUnitVar("BasicDamage"))
   local damage_bonus = Sub(ActiveUnitVar("PiercingDamage", "Value", "Type"),
                            ActiveUnitVar("PiercingDamage", "Value", "Initial"))

   DefinePanelContents(
-- Default presentation. ------------------------
  {
  Ident = "panel-general-contents",
  Pos = {info_panel_x, info_panel_y}, DefaultFont = "game",
  Contents = {
	{ Pos = {14, 56}, Condition = {ShowOpponent = false, HideNeutral = true},
		More = {"LifeBar", {Variable = "HitPoints", Height = 7, Width = 45}}
	},
	{ Pos = {38, 62}, Condition = {ShowOpponent = false, HideNeutral = true},
		More = {"FormattedText2", {
			Font = "small", Variable = "HitPoints", Format = "%d/%d",
			Component1 = "Value", Component2 = "Max", Centered = true}}
	},
	{ Pos = {114, 25}, More = {"Text", {ShowName = true}} }, -- FIXME:split for long name
	--{ Pos = {10, 158}, More = {"Text", {Variable = "Slot"}},
	--	Condition = {Slot = "only"} }, -- DEBUG ONLY.
-- Resource Left
	{ Pos = {76, 86}, Condition = {ShowOpponent = false, GiveResource = "only"},
		More = {"FormattedText2", {Format = "%s: %d", Variable = "GiveResource",
					Component1 = "Name", Component2 = "Value", Centered = true}}
	}

  } },
-- All own unit -----------------
  {
  Ident = "panel-all-unit-contents",
  Pos = {info_panel_x, info_panel_y},
  DefaultFont = "game",
  Condition = {ShowOpponent = false, HideNeutral = true, Build = "false"},
  Contents = {
         { Pos = {16, 138}, Condition = {BasicDamage = "only"},
            More = {"Text", {Text = Concat("Damage: ", String(min_damage), 
                                           "-", String(max_damage),
                                           If(Equal(0, damage_bonus), "",
                                              InverseVideo(Concat("+", String(damage_bonus)))) 
                                          )}}
        },
	{ Pos = {16, 111}, Condition = {AttackRange = "only"},
		More = {"Text", {
			 Text = "Range: ", Variable = "AttackRange" , Stat = true}}
	},
-- Construction
	{ Pos = {12, 153}, Condition = {Build = "only"},
		More = {"CompleteBar", {Variable = "Build", Width = 152, Height = 12}}
	},
	{ Pos = {50, 154}, Condition = {Build = "only"}, More = {"Text", "% Complete"}},
	{ Pos = {107, 78}, Condition = {Build = "only"}, More = {"Icon", {Unit = "Worker"}}},
-- Training
	{ Pos = {14, 141}, Condition = {Training = "only"},
		More = {"CompleteBar", {Variable = "Training", Width = 136, Height = 12}}
	},
	{ Pos = {44, 141}, Condition = {Training = "only"}, More = {"Text", "% Complete"}},
-- Mana
	{ Pos = {16, 141}, Condition = {Mana = "only"},
		More = {"CompleteBar", {Variable = "Mana", Height = 16, Width = 140, Border = true}}
	},
	{ Pos = {86, 141}, More = {"Text", {Variable = "Mana"}}, Condition = {Mana = "only"} },

  } },
-- Attack Unit -----------------------------
  {
  Ident = "panel-attack-unit-contents",
  Pos = {info_panel_x, info_panel_y},
  DefaultFont = "game",
  Condition = {ShowOpponent = false, HideNeutral = true, Building = "false", Build = "false"},
  Contents = {
-- Unit caracteristics
	{ Pos = {114, 37},
		More = {"FormattedText", {Centered = true, 
			Variable = "Level", Format = "Level ~<%d~>"}}
	},
	{ Pos = {114, 52},
		More = {"FormattedText", {Centered = true,
			Variable = "Kill", Format = "Kills:~<%d~>"}}
	},
	{ Pos = {16, 84}, Condition = {Armor = "only"},
		More = {"Text", {Text = "Armor: ", Variable = "Armor", Stat = true}}
	},
	{ Pos = {16, 125}, Condition = {SightRange = "only"},
		More = {"Text", {Text = "Sight: ", Variable = "SightRange", Stat = true}}
	},
	{ Pos = {16, 97}, Condition = {Speed = "only"},
		More = {"Text", {Text = "Speed: ", Variable = "Speed", Stat = true}}
	} } })
end


--DefineElitePanels(Video.Width - 200, 152)


UI.NormalFontColor = "light-blue"
UI.ReverseFontColor = "yellow"


function AddFiller(file, x, y)
	b = CFiller:new_local()
	b.G = CGraphic:New(file)
	b.X = x
	b.Y = y
	UI.Fillers:push_back(b)
end

UI.Fillers:clear()
AddFiller("graphics/ui/ui_minimap.png", Video.Width - 200, 24 - 8)
if (Editor.Running == EditorNotRunning) then
  local w = Video.Width
  -- FIXME: remove this when we have images for these resolutions
  if (w == 1440 or w == 1680 or w == 1920) then
    w = 1600
  elseif (w == 1280) then
    w = 1024
  end
  AddFiller("graphics/ui/ui_" .. w .. "_bpanel.png",
    Video.Width - 200, 336 - 8 - 8)
end

b = CFiller:new_local()
b.G = CGraphic:New("graphics/ui/ui_info.png", 200, 176)
b.X = Video.Width - 200
b.Y = 152
UI.Fillers:push_back(b)

UI.InfoPanel.X = Video.Width - 200
UI.InfoPanel.Y = 152

b = CUIButton:new()
b.X = Video.Width - 200 + 13
b.Y = 152 + 16
b.Style = FindButtonStyle("icon")
UI.SingleSelectedButton = b

function AddSelectedButton(x, y)
	b = CUIButton:new_local()
	b.X = x
	b.Y = y
	b.Style = FindButtonStyle("icon")
	UI.SelectedButtons:push_back(b)
end

UI.SelectedButtons:clear()
AddSelectedButton(Video.Width - 200 +  13, 152 + 16)
AddSelectedButton(Video.Width - 200 +  28, 152 + 16)
AddSelectedButton(Video.Width - 200 +  43, 152 + 16)
AddSelectedButton(Video.Width - 200 +  58, 152 + 16)
AddSelectedButton(Video.Width - 200 +  73, 152 + 16)
AddSelectedButton(Video.Width - 200 +  88, 152 + 16)
AddSelectedButton(Video.Width - 200 + 103, 152 + 16)
AddSelectedButton(Video.Width - 200 + 118, 152 + 16)
AddSelectedButton(Video.Width - 200 + 133, 152 + 16)

UI.MaxSelectedFont = Fonts["game"]
UI.MaxSelectedTextX = Video.Width - 200 + 10
UI.MaxSelectedTextY = 152 + 10

b = CUIButton:new()
b.X = Video.Width - 200 + 13
b.Y = 152 + 91
b.Style = FindButtonStyle("icon")
UI.SingleTrainingButton = b

function AddTrainingButton(x, y)
	b = CUIButton:new_local()
	b.X = x
	b.Y = y
	b.Style = FindButtonStyle("icon")
	UI.TrainingButtons:push_back(b)
end

UI.TrainingButtons:clear()
AddTrainingButton(Video.Width - 200 + 13, 152 + 91)
AddTrainingButton(Video.Width - 200 + 28, 152 + 91)
AddTrainingButton(Video.Width - 200 + 43, 152 + 91)
AddTrainingButton(Video.Width - 200 + 58, 152 + 91)
AddTrainingButton(Video.Width - 200 + 73, 152 + 91)
AddTrainingButton(Video.Width - 200 + 88, 152 + 91)

function AddTransportingButton(x, y)
	b = CUIButton:new_local()
	b.X = x
	b.Y = y
	b.Style = FindButtonStyle("icon")
	UI.TransportingButtons:push_back(b)
end

UI.TransportingButtons:clear()
AddTransportingButton(Video.Width - 200 + 13, 152 + 91)
AddTransportingButton(Video.Width - 200 + 28, 152 + 91)
AddTransportingButton(Video.Width - 200 + 43, 152 + 91)
AddTransportingButton(Video.Width - 200 + 58, 152 + 91)
AddTransportingButton(Video.Width - 200 + 73, 152 + 91)
AddTransportingButton(Video.Width - 200 + 88, 152 + 91)

function AddButtonPanelButton(x, y)
	b = CUIButton:new_local()
	b.X = x
	b.Y = y
	b.Style = FindButtonStyle("icon")
	UI.ButtonPanel.Buttons:push_back(b)
end

UI.ButtonPanel.Buttons:clear()
AddButtonPanelButton(Video.Width - 177 + 4, 340 - 12)
AddButtonPanelButton(Video.Width - 122 + 4, 340 - 12)
AddButtonPanelButton(Video.Width - 67  + 4, 340 - 12)
AddButtonPanelButton(Video.Width - 177 + 4, 385 - 12)
AddButtonPanelButton(Video.Width - 122 + 4, 385 - 12)
AddButtonPanelButton(Video.Width - 67  + 4, 385 - 12)
AddButtonPanelButton(Video.Width - 177 + 4, 430 - 12)
AddButtonPanelButton(Video.Width - 122 + 4, 430 - 12)
AddButtonPanelButton(Video.Width - 67  + 4, 430 - 12)

UI.ButtonPanel.X = Video.Width - 200
UI.ButtonPanel.Y = 336 - 8 - 8
UI.ButtonPanel.AutoCastBorderColorRGB = CColor(0, 0, 252)


UI.MapArea.X = 0
UI.MapArea.Y = 16
UI.MapArea.EndX = UI.MapArea.X + (Video.Width - 200) - 1
UI.MapArea.EndY = UI.MapArea.Y + (Video.Height - 32) - 1

UI.MapArea.ScrollPaddingLeft = 0
UI.MapArea.ScrollPaddingRight = 0
UI.MapArea.ScrollPaddingTop = 0
UI.MapArea.ScrollPaddingBottom = 0

UI.Minimap.X = Video.Width - 200 + 46
UI.Minimap.Y = 24 + 17 - 8
UI.Minimap.W = 121
UI.Minimap.H = 105

UI.StatusLine.TextX = 38
UI.StatusLine.TextY = Video.Height - 14
UI.StatusLine.Width = Video.Width - 38
UI.StatusLine.Font = Fonts["game"]

UI.Timer.X = UI.MapArea.EndY - 70
UI.Timer.Y = UI.MapArea.Y + 15
UI.Timer.Font = Fonts["game"]

-- energy
UI.Resources[0].G = CGraphic:New("graphics/ui/ui_res_icons.png", 14, 14)
UI.Resources[0].IconFrame = 0
UI.Resources[0].IconX = 67 + 0
UI.Resources[0].IconY = 0
UI.Resources[0].TextX = 85 + 0
UI.Resources[0].TextY = 1

-- magma
UI.Resources[1].G = CGraphic:New("graphics/ui/ui_res_icons.png", 14, 14)
UI.Resources[1].IconFrame = 1
UI.Resources[1].IconX = 67 + 75
UI.Resources[1].IconY = 0
UI.Resources[1].TextX = 85 + 75
UI.Resources[1].TextY = 1

-- score
UI.Resources[ScoreCost].G = CGraphic:New("graphics/ui/ui_res_icons.png", 14, 14)
UI.Resources[ScoreCost].IconFrame = 4
UI.Resources[ScoreCost].IconX = 67 + 250
UI.Resources[ScoreCost].IconY = 0
UI.Resources[ScoreCost].TextX = 85 + 250
UI.Resources[ScoreCost].TextY = 1

UI.PieMenu.G = CGraphic:New("ui/rosace1.png")
UI.PieMenu.MouseButton = MiddleButton
UI.PieMenu:SetRadius(70)

UI.MenuButton.X = Video.Width - 130
UI.MenuButton.Y = 0
UI.MenuButton.Text = "Menu (~<F10~>)"
UI.MenuButton.Style = FindButtonStyle("network")
if (Editor.Running == EditorNotRunning) then
  UI.MenuButton:SetCallback(function() RunGameMenu() end)
else
  UI.MenuButton:SetCallback(function() RunEditorIngameMenu() end)
end

UI.NetworkMenuButton.X = Video.Width - 200
UI.NetworkMenuButton.Y = 0
UI.NetworkMenuButton.Text = "Menu"
UI.NetworkMenuButton.Style = FindButtonStyle("network")
UI.NetworkMenuButton:SetCallback(function() RunGameMenu() end)

UI.NetworkDiplomacyButton.X = Video.Width - 100
UI.NetworkDiplomacyButton.Y = 0
UI.NetworkDiplomacyButton.Text = "Diplomacy"
UI.NetworkDiplomacyButton.Style = FindButtonStyle("network")
UI.NetworkDiplomacyButton:SetCallback(function() RunDiplomacyMenu() end)


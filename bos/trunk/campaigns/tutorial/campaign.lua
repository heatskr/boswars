--     ____                _       __               
--    / __ )____  _____   | |     / /___ ___________
--   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
--  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
-- /_____/\____/____/     |__/|__/\__,_/_/  /____/  
--                                              
--       A futuristic real-time strategy game.
--          This file is part of Bos Wars.
--
--      campaign.lua  -  Tutorial campaign.
--
--      (c) Copyright 2005-2006 by François Beerten
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


campaign_steps = {
  CreateMapStep("campaigns/tutorial/level01.smp",
      "Move your units to the vault in the center of the map",
      "Order from the Central Command:\n\n"..
      "Attacks on the base E411 destroyed all its defenses.\n"..
      "Reports from our spies say the vault is still unharmed.\n"..
      "Move your troops to the vault as fast as possible.\n"..
      "-- End of transmission 1. Commander GK --"),
  CreateMapStep("campaigns/tutorial/level02.smp",
      "Train 2 engineers and build 1 power plant",
      "News from the Front:\n\n"..
      "We finally arrived at our camp.\n"..
      "Despite the recent attacks, the vault is still "..
      "in good condition. "..
      "With the vault, we will train 2 extra "..
      "engineers to reinforce our group. "..
      "We also need a power plant in order to "..
      "allow us to build more units ".. 
      "for our base.\n"..
      "-- End of transmission 2. Sergeant TJ --"),
  CreateMapStep("campaigns/tutorial/level03.smp", 
      "Harvest until you have 3000 energy and 1700 magma in storage",
      "News from the Front:\n\n"..
      "The abandoned vault has few resources left. "..
      "For the extension of the base, we need 200 "..
      "more energy and 100 magma. "..
      "Our engineer detected large amounts of "..
      "magma in the rocks.\n".. 
      "-- End of transmission. Sergeant TJ --"),

}


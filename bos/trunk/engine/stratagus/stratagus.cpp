//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name stratagus.cpp - The main file. */
//
//      (c) Copyright 1998-2007 by Lutz Sammer, Francois Beerten, and
//                                 Jimmy Salmon
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

/**
** @mainpage
**
** @section Introduction Introduction
**
** Welcome to the source code documentation of the Stratagus engine.
** Extract the source documentation with doxygen (http://www.doxygen.org)
** or doc++ (http://www.zib.de/Visual/software/doc++/index.html) tools.
**
** Any help to improve this documention is welcome. If you didn't
** understand something or you found an error or a wrong spelling
** or wrong grammer please write an email (including a patch :).
**
** @section Informations Informations
**
** Visit the http://www.boswars.org web page for the latest news and
** ../doc/readme.html for other documentations.
**
** @section Modules Modules
**
** This are the main modules of the engine.
**
** @subsection Map Map
**
** Handles the map. A map is made from tiles.
**
** @see map.h @see map.cpp @see tileset.h @see tileset.cpp
**
** @subsection Unit Unit
**
** Handles units. Units are ships, flyers, buildings, creatures,
** machines.
**
** @see unit.h @see unit.cpp @see unittype.h @see unittype.cpp
**
** @subsection Missile Missile
**
** Handles missiles. Missiles are all other sprites on map
** which are no unit.
**
** @see missile.h @see missile.cpp
**
** @subsection Player Player
**
** Handles players, all units are owned by a player. A player
** could be controlled by a human or a computer.
**
** @see player.h @see player.cpp @see ::CPlayer
**
** @subsection Sound Sound
**
** Handles the high and low level of the sound. There are the
** background music support, voices and sound effects.
** Following low level backends are supported: OSS and SDL.
**
** @see sound.h @see sound.cpp
** @see script_sound.cpp @see sound_id.cpp @see sound_server.cpp
** @see unitsound.cpp
** @see sdl_audio.cpp
** @see ogg.cpp @see wav.cpp
**
** @subsection Video Video
**
** Handles the high and low level of the graphics.
** This also contains the sprite and linedrawing routines.
**
** See page @ref VideoModule for more information upon supported
** features and video platforms.
**
** @see video.h @see video.cpp
**
** @subsection Network Network
**
** Handles the high and low level of the network protocol.
** The network protocol is needed for multiplayer games.
**
** See page @ref NetworkModule for more information upon supported
** features and API.
**
** @see network.h @see network.cpp
**
** @subsection Pathfinder Pathfinder
**
** @see pathfinder.h @see pathfinder.cpp
**
** @subsection AI AI
**
** There are currently two AI's. The old one is very hardcoded,
** but does things like placing buildings better than the new.
** The old AI shouldn't be used.  The new is very flexible, but
** very basic. It includes none optimations.
**
** See page @ref AiModule for more information upon supported
** features and API.
**
** @see ai_local.h
** @see ai.h @see ai.cpp
**
** @subsection CCL CCL
**
** CCL is Craft Configuration Language, which is used to
** configure and customize Stratagus.
**
** @see script.h @see script.cpp
**
** @subsection Icon Icon
**
** @see icons.h @see icons.cpp
**
** @subsection Editor Editor
**
** This is the integrated editor, it shouldn't be a perfect
** editor. It is used to test new features of the engine.
**
** See page @ref EditorModule for more information upon supported
** features and API.
**
** @see editor.h @see editor.cpp
*/

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sstream>

#ifndef _MSC_VER
#include <unistd.h>
#endif
#ifdef __CYGWIN__
#include <getopt.h>
#endif
#if defined(_MSC_VER) || defined(__MINGW32__)
extern char *optarg;
extern int optind;
extern int getopt(int argc, char *const *argv, const char *opt);
#endif

#ifdef MAC_BUNDLE
#define Button ButtonOSX
#include <Carbon/Carbon.h>
#undef Button
#endif

#include "SDL.h"

#include "stratagus.h"
#include "video.h"
#include "font.h"
#include "cursor.h"
#include "ui.h"
#include "interface.h"
#include "menus.h"
#include "sound_server.h"
#include "sound.h"
#include "settings.h"
#include "script.h"
#include "network.h"
#include "netconnect.h"
#include "ai.h"
#include "commands.h"
#include "results.h"
#include "editor.h"
#include "movie.h"
#include "pathfinder.h"
#include "widgets.h"
#include "iolib.h"
#include "util.h"
#include "guichan.h"

extern void CreateUserDirectories(void);

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

TitleScreen **TitleScreens;          /// Title screens to show at startup
std::string StratagusLibPath;        /// Path for data directory
std::string LocalPlayerName;         /// Name of local player

	/// Name, Version, Copyright
std::string NameLine =
	"Bos Wars V" VERSION ", (c) 1998-2007 by the Bos Wars and Stratagus Project.";

static char *MapName;                /// Filename of the map to load
std::string CompileOptions;          /// Compile options.

/*----------------------------------------------------------------------------
--  Speedups FIXME: Move to some other more logic place
----------------------------------------------------------------------------*/

int SpeedResourcesHarvest[MaxCosts]; /// speed factor for harvesting resources
int SpeedResourcesReturn[MaxCosts];  /// speed factor for returning resources
int SpeedBuild = 1;                  /// speed factor for building
int SpeedTrain = 1;                  /// speed factor for training

/*============================================================================
==  DISPLAY
============================================================================*/

unsigned long GameCycle;             /// Game simulation cycle counter
unsigned long FastForwardCycle;      /// Cycle to fastforward to in a replay

/*============================================================================
==  MAIN
============================================================================*/

static bool WaitNoEvent;                     /// Flag got an event

/**
**  Callback for input.
*/
static void WaitCallbackButtonPressed(unsigned dummy)
{
	WaitNoEvent = false;
}

/**
**  Callback for input.
*/
static void WaitCallbackButtonReleased(unsigned dummy)
{
}

/**
**  Callback for input.
*/
static void WaitCallbackKeyPressed(unsigned dummy1, unsigned dummy2)
{
	WaitNoEvent = false;
}

/**
**  Callback for input.
*/
static void WaitCallbackKeyReleased(unsigned dummy1, unsigned dummy2)
{
}

/**
**  Callback for input.
*/
static void WaitCallbackKeyRepeated(unsigned dummy1, unsigned dummy2)
{
}

/**
**  Callback for input.
*/
static void WaitCallbackMouse(int x, int y)
{
}

/**
**  Callback for exit.
*/
static void WaitCallbackExit(void)
{
}

/**
**  Show a title image
*/
static void ShowTitleImage(TitleScreen *t)
{
	const EventCallback *old_callbacks;
	EventCallback callbacks;
	CGraphic *g;

	WaitNoEvent = true;

	callbacks.ButtonPressed = WaitCallbackButtonPressed;
	callbacks.ButtonReleased = WaitCallbackButtonReleased;
	callbacks.MouseMoved = WaitCallbackMouse;
	callbacks.MouseExit = WaitCallbackExit;
	callbacks.KeyPressed = WaitCallbackKeyPressed;
	callbacks.KeyReleased = WaitCallbackKeyReleased;
	callbacks.KeyRepeated = WaitCallbackKeyRepeated;
	callbacks.NetworkEvent = NetworkEvent;

	old_callbacks = GetCallbacks();
	SetCallbacks(&callbacks);

	g = CGraphic::New(t->File);
	g->Load();
	g->Resize(Video.Width, Video.Height);

	int timeout = t->Timeout * CYCLES_PER_SECOND;
	if (!timeout) {
		timeout = -1;
	}

	while (timeout-- && WaitNoEvent) {
		g->DrawSubClip(0, 0, g->Width, g->Height,
			(Video.Width - g->Width) / 2, (Video.Height - g->Height) / 2);
		TitleScreenLabel **labels = t->Labels;
		if (labels && labels[0] && labels[0]->Font &&
				labels[0]->Font->IsLoaded()) {
			for (int j = 0; labels[j]; ++j) {
				// offsets are for 640x480, scale up to actual resolution
				int x = labels[j]->Xofs * Video.Width / 640;
				int y = labels[j]->Yofs * Video.Width / 640;
				if (labels[j]->Flags & TitleFlagCenter) {
					x -= labels[j]->Font->Width(labels[j]->Text) / 2;
				}
				VideoDrawText(x, y, labels[j]->Font, labels[j]->Text);
			}
		}

		Invalidate();
		RealizeVideoMemory();
		WaitEventsOneFrame();
	}

	SetCallbacks(old_callbacks);
	CGraphic::Free(g);
}

/**
**  Show the title screens
*/
static void ShowTitleScreens(void)
{
	if (!TitleScreens) {
		return;
	}

	SetVideoSync();

	for (int i = 0; TitleScreens[i]; ++i) {
		if (!TitleScreens[i]->Music.empty()) {
			if (TitleScreens[i]->Music == "none" ||
					PlayMusic(TitleScreens[i]->Music) == -1) {
				StopMusic();
			}
		}

		if (PlayMovie(TitleScreens[i]->File)) {
			ShowTitleImage(TitleScreens[i]);
		}

		Video.ClearScreen();
	}
	Invalidate();
}

/**
**  Show load progress.
**
**  @param fmt  printf format string.
*/
void ShowLoadProgress(const char *fmt, ...)
{
	va_list va;
	char temp[4096];

	va_start(va, fmt);
	vsnprintf(temp, sizeof(temp) - 1, fmt, va);
	temp[sizeof(temp) - 1] = '\0';
	va_end(va);

	if (Video.Depth && GameFont && GameFont->IsLoaded()) {
		// Remove non printable chars
		for (char *s = temp; *s; ++s) {
			if (*s < 32) {
				*s = ' ';
			}
		}
		Video.FillRectangle(ColorBlack, 5, Video.Height - 18, Video.Width - 10, 18);
		VideoDrawTextCentered(Video.Width / 2, Video.Height - 16, GameFont, temp);
		InvalidateArea(5, Video.Height - 18, Video.Width - 10, 18);
		RealizeVideoMemory();
	} else {
		DebugPrint("!!!!%s\n" _C_ temp);
	}
}

//----------------------------------------------------------------------------

/**
**  Pre menu setup.
*/
void PreMenuSetup(void)
{
	//
	//  Initial menus require some gfx.
	//
	SetDefaultTextColors(FontYellow, FontWhite);

	LoadFonts();

	InitVideoCursors();

	LoadCursors();
	InitSettings();

	InitUserInterface();
	UI.Load();
}

extern void InitDefinedVariables();
/**
**  Run the guichan main menus loop.
**
**  @param filename  map filename
**  @param map       map loaded
**
**  @return      0 in success, else exit.
*/
static int MenuLoop(const char *filename, CMap *map)
{
	char buf[1024];
	int status;

	initGuichan(Video.Width, Video.Height);
	InterfaceState = IfaceStateMenu;
	//  Clear screen
	Video.ClearScreen();
	Invalidate();

	ButtonUnderCursor = -1;
	CursorState = CursorStatePoint;
	GameCursor = UI.Point.Cursor;

	// FIXME delete this when switching to full guichan GUI
	LibraryFileName("scripts/guichan.lua", buf, sizeof(buf));
	status = LuaLoadFile(buf);

	// We clean up later in Exit

	return status;
}

extern void CleanMissiles();
extern void CleanTriggers();
/**
**  Cleanup game.
**
**  Call each module to clean up.
**  Contrary to CleanModules, maps can be restarted
**  without reloading all lua files.
*/
void CleanGame(void)
{
	EndReplayLog();
	CleanMessages();

	CleanTriggers();
	CleanUnits();
	CleanAi();
	CleanSelections();
	CleanGroups();
	CleanMissiles();
	CleanTilesets();
	Map.Clean();
	CleanReplayLog();
	FreeVisionTable();
	FreeAStar();
}

static void ExpandPath(char *newpath, const char *path)
{
	if (*path == '~') {
		++path;
		sprintf(newpath, "%s%s", UserDirectory.c_str(), path);
	} else {
		sprintf(newpath, "%s/%s", StratagusLibPath.c_str(), path);
	}
}

extern gcn::Gui *Gui;

void StartMap(const std::string &filename, bool clean) 
{
	std::string nc, rc;

	gcn::Widget *oldTop = Gui->getTop();
	gcn::Container *container = new gcn::Container();
	container->setDimension(gcn::Rectangle(0, 0, Video.Width, Video.Height));
	container->setOpaque(false);
	Gui->setTop(container);

	NetConnectRunning = 0;
	InterfaceState = IfaceStateNormal;

	//  Create the game.
	DebugPrint("Creating game with map: %s\n" _C_ filename.c_str());
	if (clean) {
		CleanPlayers();
	}
	GetDefaultTextColors(nc, rc);

	CreateGame(filename, &Map);

	UI.StatusLine.Set(NameLine);

	//  Play the game.
	GameMainLoop();

	//  Clear screen
	Video.ClearScreen();
	Invalidate();

	CleanGame();
	InterfaceState = IfaceStateMenu;
	SetDefaultTextColors(nc, rc);

	Gui->setTop(oldTop);
	delete container;
}

void StartSavedGame(const std::string &filename) 
{
	char path[512];

	SaveGameLoading = 1;
	CleanPlayers();
	ExpandPath(path, filename.c_str());
	LoadGame(path);

	StartMap(filename, false);
}
	
void StartReplay(const std::string &filename, bool reveal)
{
	char replay[PATH_MAX];

	CleanPlayers();
	ExpandPath(replay, filename.c_str());
	LoadReplay(replay);

	ReplayRevealMap = reveal;

	StartMap(CurrentMapPath, false);
}

/**
**  Save the replay
**
**  @param filename  Name of the file to save to
**
**  @return          0 for success, -1 for failure
*/
int SaveReplay(const std::string &filename)
{
	FILE *fd;
	char *buf;
	std::stringstream logfile;
	std::string destination;
	struct stat sb;

	if (filename.find_first_of("\\/") != std::string::npos) {
		fprintf(stderr, "\\ or / not allowed in SaveReplay filename\n");
		return -1;
	}

	destination = UserDirectory + "logs/" + filename;

	logfile << UserDirectory << "logs/log_of_stratagus_" << ThisPlayer->Index << ".log";

	if (stat(logfile.str().c_str(), &sb)) {
		fprintf(stderr, "stat failed\n");
		return -1;
	}
	buf = new char[sb.st_size];
	if (!buf) {
		fprintf(stderr, "Out of memory\n");
		return -1;
	}
	fd = fopen(logfile.str().c_str(), "rb");
	if (!fd) {
		fprintf(stderr, "fopen failed\n");
		delete[] buf;
		return -1;
	}
	fread(buf, sb.st_size, 1, fd);
	fclose(fd);

	fd = fopen(destination.c_str(), "wb");
	if (!fd) {
		fprintf(stderr, "Can't save to `%s'\n", destination.c_str());
		delete[] buf;
		return -1;
	}
	fwrite(buf, sb.st_size, 1, fd);
	fclose(fd);

	delete[] buf;

	return 0;
}

//----------------------------------------------------------------------------

/**
**  Print headerline, copyright, ...
*/
static void PrintHeader(void)
{
	fprintf(stdout, "%s\n  written by Lutz Sammer, Fabrice Rossi, Vladi Shabanski, Patrice Fortier,\n"
		"Jon Gabrielson, Andreas Arens, Nehal Mistry, Jimmy Salmon, and others.\n"
		"\t(http://www.boswars.org)"
		"\nCompile options %s", NameLine.c_str(), CompileOptions.c_str());
}

/**
**  Main1, called from main.
**
**  @param argc  Number of arguments.
**  @param argv  Vector of arguments.
*/
static int main1(int argc, char **argv)
{
	PrintHeader();
	printf(
	"\n"
	"\n"
	"Bos Wars may be copied only under the terms of the GNU General Public License\n"
	"which may be found in the Bos Wars source kit.\n"
	"\n"
	"DISCLAIMER:\n"
	"This software is provided as-is.  The author(s) can not be held liable for any\n"
	"damage that might arise from the use of this software.\n"
	"Use it at your own risk.\n"
	"\n");

	// Setup video display
	InitVideo();

	// Setup sound card
	if (!InitSound()) {
		InitMusic();
	}

#ifndef DEBUG           // For debug it's better not to have:
	srand(time(NULL));  // Random counter = random each start
#endif

	//
	//  Show title screens.
	//
	SetDefaultTextColors(FontYellow, FontWhite);
	LoadFonts();
	SetClipping(0, 0, Video.Width - 1, Video.Height - 1);
	Video.ClearScreen();
	ShowTitleScreens();

	// Init player data
	ThisPlayer = NULL;
	//Don't clear the Players strucure as it would erase the allowed units.
	// memset(Players, 0, sizeof(Players));
	NumPlayers = 0;

	InitUnitsMemory();  // Units memory management
	PreMenuSetup();     // Load everything needed for menus

	MenuLoop(MapName, &Map);

	return 0;
}

/**
**  Exit the game.
**
**  @param err  Error code to pass to shell.
*/
void Exit(int err)
{
	StopMusic();
	QuitSound();
	NetworkQuit();

	ExitNetwork1();
#ifdef DEBUG
	CleanModules();
	FreeSounds();
	FreeGraphics();
	FreePlayerColors();
	freeGuichan();
	DebugPrint("Frames %lu, Slow frames %d = %ld%%\n" _C_
		FrameCounter _C_ SlowFrameCounter _C_
		(SlowFrameCounter * 100) / (FrameCounter ? FrameCounter : 1));
	lua_settop(Lua, 0);
	lua_close(Lua);
#endif

	fprintf(stdout, _("Thanks for playing Bos Wars.\n"));
	exit(err);
}

/**
**  Do a fatal exit.
**  Called on out of memory or crash.
**
**  @param err  Error code to pass to shell.
*/
void ExitFatal(int err)
{
	exit(err);
}

/**
**  Display the usage.
*/
static void Usage(void)
{
	PrintHeader();
	printf(
"\n\nUsage: boswars [OPTIONS]\n\
\t-c file.lua\tconfiguration start file (default stratagus.lua)\n\
\t-d datapath\tpath to stratagus data\n\
\t-e\t\tStart editor\n\
\t-h\t\tHelp shows this page\n\
\t-l\t\tDisable command log\n\
\t-P port\t\tNetwork port to use\n\
\t-n server\tNetwork server host preset\n\
\t-L lag\t\tNetwork lag in # frames (default 10 = 333ms)\n\
\t-U update\tNetwork update rate in # frames (default 5=6x per s)\n\
\t-N name\t\tName of the player\n\
\t-s sleep\tNumber of frames for the AI to sleep before it starts\n\
\t-v mode\t\tVideo mode (0=default,1=640x480,2=800x600,\n\
\t\t\t\t3=1024x768,4=1280x960,5=1600x1200)\n\
\t-D\t\tVideo mode depth = pixel per point (for Win32/TNT)\n\
\t-F\t\tFull screen video mode\n\
\t-S\t\tSync speed (100 = 30 frames/s)\n\
\t-W\t\tWindowed video mode\n\
map is relative to StratagusLibPath=datapath, use ./map for relative to cwd\n\
");
}

/**
**  The main program: initialise, parse options and arguments.
**
**  @param argc  Number of arguments.
**  @param argv  Vector of arguments.
*/
int main(int argc, char **argv)
{
	char *p;

	CompileOptions =
#ifdef DEBUG
		"DEBUG "
#endif
#ifdef USE_VORBIS
		"VORBIS "
#endif
#ifdef USE_THEORA
		"THEORA "
#endif
#ifdef USE_OPENGL
		"OPENGL "
#endif
		""
	;

	//
	//  Setup some defaults.
	//
#ifndef MAC_BUNDLE
	StratagusLibPath = STRATAGUS_LIB_PATH;
#else
	freopen("/tmp/stdout.txt", "w", stdout);
	freopen("/tmp/stderr.txt", "w", stderr);
	// Look for the specified data set inside the application bundle
	// This should be a subdir of the Resources directory
	CFURLRef pluginRef = CFBundleCopyResourceURL(CFBundleGetMainBundle(),
		CFSTR(MAC_BUNDLE_DATADIR), NULL, NULL);
	CFStringRef macPath = CFURLCopyFileSystemPath(pluginRef,
		 kCFURLPOSIXPathStyle);
	const char *pathPtr = CFStringGetCStringPtr(macPath,
		CFStringGetSystemEncoding());
	Assert(pathPtr);
	StratagusLibPath = pathPtr;
#endif
	CclStartFile = "scripts/stratagus.lua";
	EditorStartFile = "scripts/editor.lua";

	//  Default player name to username on unix systems.
	LocalPlayerName.clear();
#ifdef USE_WIN32
	LocalPlayerName = "Anonymous";
#else
	if (getenv("USER")) {
		LocalPlayerName = getenv("USER");
	} else {
		LocalPlayerName = "Anonymous";
	}
#endif

	// FIXME: Parse options before or after scripts?

	//
	//  Parse commandline
	//
	for (;;) {
		switch (getopt(argc, argv, "c:d:ef:hln:P:s:t:v:wD:N:E:FL:S:U:W?")) {
			case 'c':
				CclStartFile = optarg;
				continue;
			case 'd':
			{
				char *libpath = new_strdup(optarg);
				for (p = libpath; *p; ++p) {
					if (*p == '\\') {
						*p = '/';
					}
				}
				StratagusLibPath = libpath;
				delete[] libpath;
				continue;
			}
			case 'e':
				Editor.Running = EditorCommandLine;
				continue;
			case 'E':
				EditorStartFile = optarg;
				continue;
			case 'l':
				CommandLogDisabled = 1;
				continue;
			case 'P':
				NetworkPort = atoi(optarg);
				continue;
			case 'n':
				NetworkArg = optarg;
				continue;
			case 'N':
				LocalPlayerName = optarg;
				continue;
			case 's':
				AiSleepCycles = atoi(optarg);
				continue;
			case 'v':
				switch (atoi(optarg)) {
					case 0:
						continue;
					case 1:
						Video.Width = 640;
						Video.Height = 480;
						continue;
					case 2:
						Video.Width = 800;
						Video.Height = 600;
						continue;
					case 3:
						Video.Width = 1024;
						Video.Height = 768;
						continue;
					case 4:
						Video.Width = 1280;
						Video.Height = 960;
						continue;
					case 5:
						Video.Width = 1600;
						Video.Height = 1200;
						continue;
					default:
						Usage();
						ExitFatal(-1);
				}
				continue;

			case 'L':
				NetworkLag = atoi(optarg);
				if (!NetworkLag) {
					fprintf(stderr, "FIXME: zero lag not supported\n");
					Usage();
					ExitFatal(-1);
				}
				continue;
			case 'U':
				NetworkUpdates = atoi(optarg);
				continue;

			case 'F':
				VideoForceFullScreen = 1;
				Video.FullScreen = 1;
				continue;
			case 'W':
				VideoForceFullScreen = 1;
				Video.FullScreen = 0;
				continue;
			case 'D':
				Video.Depth = atoi(optarg);
				continue;
			case 'S':
				VideoSyncSpeed = atoi(optarg);
				continue;

			case -1:
				break;
			case '?':
			case 'h':
			default:
				Usage();
				ExitFatal(-1);
		}
		break;
	}

	if (argc - optind > 1) {
		fprintf(stderr, "too many files\n");
		Usage();
		ExitFatal(-1);
	}

	if (argc - optind) {
		MapName = argv[optind];
		for (p = MapName; *p; ++p) {
			if (*p == '\\') {
				*p = '/';
			}
		}
		--argc;
	}

	// Init the random number generator.
	InitSyncRand();

	CreateUserDirectories();

	// Init CCL and load configurations!
	InitCcl();

	// Initialise AI module
	InitAiModule();

	LoadCcl();

	main1(argc, argv);

	Exit(0);
	return 0;
}

//@}

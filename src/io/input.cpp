/*
 * input.cpp
 *
 * Copyright (C) 2001 Matt Ownby
 *
 * This file is part of DAPHNE, a laserdisc arcade game emulator
 *
 * DAPHNE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * DAPHNE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// Handles SDL input functions (low-level keyboard/joystick input)

#ifdef WIN32
#include <windows.h> // for ScreenToClient -- rdg
#endif
#include "SDL_syswm.h"
#include <time.h>
#include "input.h"
#include "conout.h"
#include "homedir.h"
#include "../video/video.h"
#include "../video/SDL_Console.h"
#include "../daphne.h"
#include "../timer/timer.h"
#include "../game/game.h"
//#include "../game/thayers.h"
#include "../game/singe.h" // by RDG2010
#include "../ldp-out/ldp.h"
#include "fileparse.h"
#include "../manymouse/manymouse.h" // by RDG

#ifdef UNIX
#include <fcntl.h>	// for non-blocking i/o
#endif

#include <queue>	// STL queue for coin queue

using namespace std;

// Win32 doesn't use strcasecmp, it uses stricmp (lame)
#ifdef WIN32
#define strcasecmp stricmp
#endif

const int JOY_AXIS_MID = (int) (32768 * (0.75));		// how far they have to move the joystick before it 'grabs'

SDL_Joystick *G_joystick = NULL;	// pointer to joystick object
bool g_use_joystick = true;	// use a joystick by default
bool g_consoledown = false;			// whether the console is down or not
bool g_alt_pressed = false;	// whether the ALT key is presssed (for ALT-Enter combo)
unsigned int idle_timer; // added by JFA for -idleexit

string g_inputini_file = "singeinput.ini"; // Default keymap file
bool m_altInputFileSet = false;

const double STICKY_COIN_SECONDS = 0.125;	// how many seconds a coin acceptor is forced to be "depressed" and how many seconds it is forced to be "released"
Uint32 g_sticky_coin_cycles = 0;	// STICKY_COIN_SECONDS * get_cpu_hz(0), cannot be calculated statically
queue<struct coin_input> g_coin_queue;	// keeps track of coin input to guarantee that coins don't get missed if the cpu is busy (during seeks for example)
Uint64 g_last_coin_cycle_used = 0;	// the cycle value that our last coin press used

static int available_mice = 0; //rdg
static ManyMouseEvent mm_event;
static int bolGrab = false;

#define SDL_MOUSE 100
#define MANY_MOUSE 200

static int g_mouse_mode = SDL_MOUSE;


// the ASCII key words that the parser looks at for the key values
// NOTE : these are in a specific order, corresponding to the enum in daphne.h
const char *g_key_names[] = 
{
	"KEY_UP", "KEY_LEFT", "KEY_DOWN", "KEY_RIGHT", "KEY_START1", "KEY_START2", "KEY_BUTTON1",
	"KEY_BUTTON2", "KEY_BUTTON3", "KEY_COIN1", "KEY_COIN2", "KEY_SKILL1", "KEY_SKILL2", 
	"KEY_SKILL3", "KEY_SERVICE", "KEY_TEST", "KEY_RESET", "KEY_SCREENSHOT", "KEY_QUIT", "KEY_PAUSE",
	"KEY_CONSOLE", "KEY_TILT"
};

// default key assignments, in case .ini file is missing
// Notice each switch can have two keys assigned to it
// NOTE : These are in a specific order, corresponding to the enum in daphne.h
int g_key_defs[SWITCH_COUNT][2] =
{
	{ SDLK_UP, SDLK_KP8 },	// up
	{ SDLK_LEFT, SDLK_KP4 },	// left
	{ SDLK_DOWN, SDLK_KP2 },	// down
	{ SDLK_RIGHT,  SDLK_KP6 },	// right
	{ SDLK_1,	0 }, // 1 player start
	{ SDLK_2,	0 }, // 2 player start
	{ SDLK_SPACE, SDLK_LCTRL }, // action button 1
	{ SDLK_LALT,	0 }, // action button 2
	{ SDLK_LSHIFT,	0 }, // action button 3
	{ SDLK_5, SDLK_c }, // coin chute left
	{ SDLK_6, 0 }, // coin chute right
	{ SDLK_KP_DIVIDE, 0 }, // skill easy
	{ SDLK_KP_MULTIPLY, 0 },	// skill medium
	{ SDLK_KP_MINUS, 0 },	// skill hard
	{ SDLK_9, 0 }, // service coin
	{ SDLK_F2, 0 },	// test mode
	{ SDLK_F3, 0 },	// reset cpu
	{ SDLK_F12, SDLK_F11 },	// take screenshot
	{ SDLK_ESCAPE, SDLK_q },	// Quit DAPHNE
	{ SDLK_p, 0 },	// pause game
	{ SDLK_BACKQUOTE, 0 },	// toggle console
	{ SDLK_t, 0 },	// Tilt/Slam switch
};

////////////

#ifndef GP2X
// added by Russ
// global button mapping array. just hardcoded room for 10 buttons max
// not a fixed system dont initialize button functions unless its 
// from dapinput.sh

int joystick_buttons_map[18] = {
	-1,	// button 0
	-1,	// button 1
	-1,	// button 2
	-1,	// button 3
	-1,	// button 4
	-1,	// button 5
	-1,	// button 6
	-1,	// button 7
	-1,	// button 8
	-1,	// button 9
	-1,	// button 10
	-1,	// button 11
	-1,	// button 12
	-1,	// button 13
	-1,	// button 14
	-1,	// button 15
	-1,	// button 16
	-1	// button 17
};

#else
// button mapping for gp2x
int joystick_buttons_map[18] =
{
	SWITCH_UP,	// 0 (up)
	SWITCH_UP,	// 1 (up-left)
	SWITCH_LEFT,	// 2 (left)
	SWITCH_DOWN,	// 3 (down-left)
	SWITCH_DOWN,	// 4 (down)
	SWITCH_DOWN,	// 5 (down-right)
	SWITCH_RIGHT,	// 6 (right)
	SWITCH_UP,	// 7 (up-right)
	SWITCH_START1,	// 8 (start)
	SWITCH_COIN1,	// 9 (select)
	SWITCH_QUIT,	// 10 (left)
	SWITCH_PAUSE,	// 11 (right)
	SWITCH_BUTTON1,	// 12 (A)
	SWITCH_BUTTON2, // 13 (B)
	SWITCH_BUTTON3,	// 14 (X)
	SWITCH_CONSOLE,	// 15 (Y)
	SWITCH_BUTTON1, // 16 is vol +
	SWITCH_BUTTON1 // 17 is vol -
};
#endif

// Mouse button to key mappings
// Added by ScottD for Singe

int mouse_buttons_map[6] =
{
	SWITCH_BUTTON1,  // 0 (Left Button)
	SWITCH_BUTTON3,  // 1 (Middle Button)
	SWITCH_BUTTON2,  // 2 (Right Button)
	SWITCH_BUTTON1,  // 3 (Wheel Up)
	SWITCH_BUTTON2,   // 4 (Wheel Down)
	SWITCH_MOUSE_DISCONNECT // this is a dummy value so SDL mouse array matches Many Mouse array
};

// ManyMouse button mapping to be backwards compatible with Scott's original.
/*
int mouse_buttons_map[6] =
{
	SWITCH_BUTTON3,  // 0 (Left Button)
	SWITCH_BUTTON1,  // 1 (Right Button)
	SWITCH_BUTTON2,  // 2 (Middle Up)
	SWITCH_MOUSE_SCROLL_UP,     // 4 (Wheel UP)
	SWITCH_MOUSE_SCROLL_DOWN,   // 3 (Wheel Down)
	SWITCH_MOUSE_DISCONNECT
};
*/
////////////

void CFG_Keys()
{
	struct mpo_io *io;
	string cur_line = "";
	string key_name = "", sval1 = "", sval2 = "", sval3 = "", eq_sign = "";
	int val1 = 0, val2 = 0, val3 = 0;

	string keyinput_notice;

	if (m_altInputFileSet) {
		keyinput_notice = "Loading alternate keymap file: ";
	} else {
		keyinput_notice = "Looking for: ";
	}

	keyinput_notice += g_inputini_file.c_str();
	printline(keyinput_notice.c_str());

	// find where the keymap ini file is (if the file doesn't exist, this string will be empty)
	string strDapInput = g_homedir.find_file(g_inputini_file.c_str(), true);

	int max_buttons = (int) (sizeof(joystick_buttons_map) / sizeof(int));
	io = mpo_open(strDapInput.c_str(), MPO_OPEN_READONLY);
	if (io)
	{
		printline("Remapping input ...");

		cur_line = "";

		// read lines until we find the keyboard header or we hit EOF
		while (strcasecmp(cur_line.c_str(), "[KEYBOARD]") != 0)
		{
			read_line(io, cur_line);
			if (io->eof)
			{
				printline("CFG_Keys() : never found [KEYBOARD] header, aborting");
				break;
			}
		}

		// go until we hit EOF, or we break inside this loop (which is the expected behavior)
		while (!io->eof)
		{
			// if we read in something besides a blank line
			if (read_line(io, cur_line) > 0)
			{
				bool corrupt_file = true;	// we use this to avoid doing multiple if/else/break statements

				// if we are able to read in the key name
				if (find_word(cur_line.c_str(), key_name, cur_line))
				{
					if (strcasecmp(key_name.c_str(), "END") == 0) break;	// if we hit the 'END' keyword, we're done

					// equals sign
					if (find_word(cur_line.c_str(), eq_sign, cur_line) && (eq_sign == "="))
					{
						if (find_word(cur_line.c_str(), sval1, cur_line))
						{
							if (find_word(cur_line.c_str(), sval2, cur_line))
							{
								if (find_word(cur_line.c_str(), sval3, cur_line))
								{
									val1 = atoi(sval1.c_str());
									val2 = atoi(sval2.c_str());
									val3 = atoi(sval3.c_str());
									corrupt_file = false;	// looks like we're good

									bool found_match = false;
									for (int i = 0; i < SWITCH_COUNT; i++)
									{
										// if we can match up a key name (see list above) ...
										if (strcasecmp(key_name.c_str(), g_key_names[i])==0)
										{
											g_key_defs[i][0] = val1;
											g_key_defs[i][1] = val2;

											// if zero then no mapping necessary, just use default, if any
											if ( val3 -1  < max_buttons)
											{
												if (val3 > 0 ) joystick_buttons_map[val3 - 1] = i;
											}
											else printf("sorry we only support %d joystick buttons we could not map button %d please check your dapinput.ini\n",max_buttons, val3 - 1 );
											found_match = true;
											break;
										}
									}

									// if the key line was unknown
									if (!found_match)
									{
										cur_line = "CFG_Keys() : Unrecognized key name " + key_name;
										printline(cur_line.c_str());
										corrupt_file = true;
									}

								}
								else printline("CFG_Keys() : Expected 3 integers, only found 2");
							}
							else printline("CFG_Keys() : Expected 3 integers, only found 1");
						}
						else printline("CFG_Keys() : Expected 3 integers, found none");
					} // end equals sign
					else printline("CFG_Keys() : Expected an '=' sign, didn't find it");
				} // end if we found key_name
				else printline("CFG_Keys() : Weird unexpected error happened");	// this really shouldn't ever happen

				if (corrupt_file)
				{
					printline("CFG_Keys() : input remapping file was not in proper format, so we are aborting");
					break;
				}
			} // end if we didn't find a blank line
			// else it's a blank line so we just ignore it
		} // end while not EOF

		mpo_close(io);
	} // end if file was opened successfully
}


// By RDG == ManyMouse code follows

static void ManyMouse_Init_Mice(void)
{
    
	printline("Using ManyMouse for mice input.");
	available_mice = ManyMouse_Init();
	g_game->set_mice_detected(available_mice);
	static Mouse mice[MAX_MICE];

    if (available_mice > MAX_MICE)
        available_mice = MAX_MICE;

    if (available_mice <= 0)
		printline("No mice detected!\n");
    else
    {	

		int i;
		char s1[255];	
		if (available_mice == 1)
			printline("Only 1 mouse found.");
		else
		{
			sprintf(s1,"Found %d mice devices:",available_mice);
			printline(s1);
		}
		
        for (i = 0; i < available_mice; i++)
        {
            const char *name = ManyMouse_DeviceName(i);			
            strncpy(mice[i].name, name, sizeof (mice[i].name));			
            mice[i].name[sizeof (mice[i].name) - 1] = '\0';
            mice[i].connected = 1;			
			sprintf(s1,"#%d: %s", i, mice[i].name);
            printline(s1);			
        }
		SDL_WM_GrabInput(SDL_GRAB_ON);
    }
}

static void ManyMouse_Update_Mice()
{
    
    while (ManyMouse_PollEvent(&mm_event))
    {
        Mouse *mouse;
        if (mm_event.device >= (unsigned int) available_mice)
            continue;

        static Mouse mice[MAX_MICE];
        mouse = &mice[mm_event.device];

        if (mm_event.type == MANYMOUSE_EVENT_RELMOTION)
        {
            if (mm_event.item == 0)
                mouse->relx += mm_event.value;
            else if (mm_event.item == 1)
				mouse->rely += mm_event.value;

			if (mouse->relx < 0) mouse->relx = 0;
			else if (mouse->relx >= get_video_width()) mouse->relx = get_video_width();
			if (mouse->rely < 0) mouse->rely = 0;
			else if (mouse->rely >= get_video_height()) mouse->rely = get_video_height();		

			g_game->OnMouseMotion(mouse->x, mouse->y, mouse->relx, mouse->rely, mm_event.device);
			
        }

        else if (mm_event.type == MANYMOUSE_EVENT_ABSMOTION)
        {
			
            if (mm_event.item == 0)
                //mouse->x = (val / maxval) * screen_w;
				mouse->x = mm_event.value; 
            else if (mm_event.item == 1)
                //mouse->y = (val / maxval) * screen_h;
				mouse->y = mm_event.value;

			g_game->OnMouseMotion(mouse->x, mouse->y, mouse->relx, mouse->rely, mm_event.device);
			

			/*
			LPPOINT absPoint = new POINT;
			// Access to SDL's OS specific structure.
			SDL_SysWMinfo windowInfo;
			SDL_VERSION(&windowInfo.version);
			if(SDL_GetWMInfo(&windowInfo))
			{
				// Retrieve the Windows handle to the SDL window.
				HWND handle = windowInfo.window;

				if (mm_event.item == 0)					
				{
					absPoint->x = mm_event.value;
					absPoint->y = 0;
					//if (ClientToScreen(handle, absPoint) != 0)
					MapWindowPoints(NULL, handle,absPoint,2);
						mouse->x = absPoint->x;
					
				}
				else if (mm_event.item == 1)
				{
					absPoint.y = mm_event.value;
					if (ScreenToClient(handle,LPPOINT) absPoint.y) == 0)
						mouse->y = absPoint.y;
				}
				

			}
			
			*/
        }

        else if (mm_event.type == MANYMOUSE_EVENT_BUTTON)
        {
            if (mm_event.item < 32)
            {
				
                if (mm_event.value == 1) // 0 == release, 1 == pressed
				{
					input_enable((Uint8)mouse_buttons_map[mm_event.item], mm_event.device);
					mouse->buttons |= (1 << mm_event.item);
					
				}
                else
				{
					input_disable((Uint8)mouse_buttons_map[mm_event.item], mm_event.device);
					mouse->buttons &= ~(1 << mm_event.item);
					
				}
            }
        }

        else if (mm_event.type == MANYMOUSE_EVENT_SCROLL)
        {
            if (mm_event.item == 0)  // vertical scroll wheel
            {
                if (mm_event.value > 0) // scroll up
                    input_disable(SWITCH_MOUSE_SCROLL_UP, mm_event.device); 
                else // scroll down
                    input_disable(SWITCH_MOUSE_SCROLL_DOWN, mm_event.device);
            }
            
        }

        else if (mm_event.type == MANYMOUSE_EVENT_DISCONNECT)
        {
            mice[mm_event.device].connected = 0;
			input_disable(SWITCH_MOUSE_DISCONNECT, mm_event.device);
        }
    }

}


////// end manymouse implementation

int SDL_input_init()
// initializes the keyboard (and joystick if one is present)
// returns 1 if successful, 0 on error
// NOTE: Video has to be initialized BEFORE this is or else it won't work
{

	int result = 0;

	// make sure the coin queue is empty (it should be, this is just a safety check)
	while (!g_coin_queue.empty())
	{
		g_coin_queue.pop();
	}
	g_sticky_coin_cycles = 0;//(Uint32) (STICKY_COIN_SECONDS * get_cpu_hz(0));	// only needs to be calculated once

	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) >= 0)
	{
		// if joystick usage is enabled
		if (g_use_joystick)
		{
			// if there is at least 1 joystick and we are authorized to use the joystick for input
			if (SDL_NumJoysticks() > 0)
			{
				G_joystick = SDL_JoystickOpen(0);	// FIXME: right now we automatically choose the first joystick
				if (G_joystick != NULL)
				{
					printline("Joystick #0 was successfully opened");
				}
				else
				{
					printline("Error opening joystick!");
				}
			}
			else
			{
				printline("No joysticks detected");
			}
		}
		// notify user that their attempt to disable the joystick is successful
		else
		{
			printline("Joystick usage disabled");
		}
		
		CFG_Keys();	// NOTE : for some freak reason, this should not be done BEFORE the joystick is initialized, I don't know why!
		
	
		result = 1;
	}
	else
	{
		printline("Input initialization failed!");
	}

	idle_timer = refresh_ms_time(); // added by JFA for -idleexit

#ifdef UNIX
	// enable non-blocking stdin (see conin.cpp UNIX stdin stuff)
	/*int iRes =*/ fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);
#endif
/*
	// if the mouse is disabled, then filter mouse events out ...
	if (!g_game->get_mouse_enabled())
	{
		FilterMouseEvents(true);		
	}
*/	
	// Manymouse implementation by RDG
	if (!g_game->get_mouse_enabled())
	{
		FilterMouseEvents(true);			
		if (g_game->get_mouse_enabled()) ManyMouse_Init_Mice();		
	}
	else
		FilterMouseEvents(false);

	return(result);

}

void FilterMouseEvents(bool bFilteredOut)
{
	int iState = SDL_ENABLE;

	if (bFilteredOut)
	{
		iState = SDL_IGNORE;
	}

	SDL_EventState(SDL_MOUSEMOTION, iState);
	SDL_EventState(SDL_MOUSEBUTTONDOWN, iState);
	SDL_EventState(SDL_MOUSEBUTTONUP, iState);
}

// does any shutting down necessary
// 1 = success, 0 = failure
int SDL_input_shutdown(void)
{
	SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
	if ((g_game->get_mouse_enabled()) && (g_mouse_mode == MANY_MOUSE)) ManyMouse_Quit();
	return(1);
}

// checks to see if there is incoming input, and acts on it
void SDL_check_input()
{

	SDL_Event event;

	while ((SDL_PollEvent (&event)) && (!get_quitflag()))
	{
		// if they press the tilda key to bring down the console
		// this is somewhat of a hacked if statement but I can't see
		// a better way based on the SDL_Console API ...
		if ((event.type == SDL_KEYDOWN) && (event.key.keysym.sym == SDLK_BACKQUOTE))
		{
			// we must not bring down the console if blitting is not allowed
			if (g_ldp->is_blitting_allowed())
			{
#ifdef CPU_DEBUG
				toggle_console();
#endif
			}
		}
		// if we've got the console down, process events
		else if (g_consoledown)
		{
			ConsoleEvents(&event);
		}
		// else handle events normally
		else
		{
			process_event(&event);
		}
	}

	if (g_game->get_mouse_enabled() && (g_mouse_mode == MANY_MOUSE)) ManyMouse_Update_Mice();

	check_console_refresh();

	// added by JFA for -idleexit
	if (get_idleexit() > 0 && elapsed_ms_time(idle_timer) > get_idleexit()) set_quitflag();

	// if the coin queue has something entered into it
	if (!g_coin_queue.empty())
	{
		struct coin_input coin = g_coin_queue.front();	// examine the next element in the queue to be considered

		// NOTE : when cpu timers are flushed, the coin queue is automatically "reshuffled"
		// so it is safe not to check to see whether the cpu timers were flushed here

		// if it's safe to activate the coin
		//if (get_total_cycles_executed(0) > coin.cycles_when_to_enable)
		{
			// if we're supposed to enable this coin
			if (coin.coin_enabled)
			{
				g_game->input_enable(coin.coin_val, NOMOUSE);
			}
			// else we are supposed to disable this coin
			else
			{
				g_game->input_disable(coin.coin_val, NOMOUSE);
			}
			g_coin_queue.pop();	// remove coin entry from queue
		}
		// else it's not safe to activate the coin, so we just wait
	}
	// else the coin queue is empty, so we needn't do anything ...

}

#ifdef CPU_DEBUG
void toggle_console()
{
	if (get_console_initialized())
	{
		// if console is down, get rid of it
		if (g_consoledown)
		{
			g_consoledown = false;
			SDL_EnableUNICODE(0);
			display_repaint();
		}
		// if console is not down, display it
		else
		{
			g_consoledown = true;
			SDL_EnableUNICODE(1);
		}
	}
}
#endif

// processes incoming input
void process_event(SDL_Event *event)
{
	unsigned int i = 0;
	
	// by RDG2010
	// make things easier to read...
	SDLKey keyPressed = event->key.keysym.sym;

	switch (event->type)
	{
		case SDL_KEYDOWN:
			reset_idle(); // added by JFA for -idleexit
			
			
			// by RDG2010
			// Get SINGE full access to keyboard input (like Thayers)

				
				{
					singe *l_singe = dynamic_cast<singe *>(g_game);
					if (l_singe) l_singe->process_keydown(keyPressed, g_key_defs);
				} 
				 

			
			break;
		case SDL_KEYUP:
			// MPO : since con_getkey doesn't catch the key up event, we shouldn't call reset_idle here.
			//reset_idle(); // added by JFA for -idleexit

			// by RDG2010
			// Get SINGE full access to keyboard input (like Thayers)

				if (keyPressed == SDLK_g)
				{
					if ((g_game->get_mouse_enabled()) && (g_mouse_mode == MANY_MOUSE)) 
					{
						bolGrab = !bolGrab;						
						if (bolGrab)
						{
							SDL_WM_GrabInput(SDL_GRAB_ON);
							ManyMouse_Quit();
							ManyMouse_Init_Mice();
						}
						else
							SDL_WM_GrabInput(SDL_GRAB_OFF);
					}
				}
				else
				{
					singe *l_singe = dynamic_cast<singe *>(g_game);
					if (l_singe) l_singe->process_keyup(keyPressed, g_key_defs);
					
				} 		
			
			break;
		case SDL_JOYAXISMOTION:
			//reset_idle(); // added by JFA for -idleexit
			// reset_idle removed here because the analog controls were registering
			// even when the joystick was not in use.  Joystick buttons still reset the idle.
			process_joystick_motion(event);
			break;
		case SDL_JOYHATMOTION:
			// only process events for the first hat
			if ( event->jhat.hat == 0 )
			{
				reset_idle();
				process_joystick_hat_motion(event);
			}
			break;
		case SDL_JOYBUTTONDOWN:
			reset_idle(); // added by JFA for -idleexit

			// added by Russ
			// loop through buttons and look for a press
			for (i = 0; i < (sizeof(joystick_buttons_map) / sizeof(int)); i++) {
				if (event->jbutton.button == i) {
					input_enable((Uint8) joystick_buttons_map[i], NOMOUSE);
					break;
				}
			}

			break;
		case SDL_JOYBUTTONUP:
			reset_idle(); // added by JFA for -idleexit

			// added by Russ
			for (i = 0; i < (sizeof(joystick_buttons_map) / sizeof(int)); i++) {
				if (event->jbutton.button == i) {
					input_disable((Uint8) joystick_buttons_map[i], NOMOUSE);
					break;
				}
			}

			break;
			/*
		case SDL_MOUSEBUTTONDOWN:
			// added by ScottD
			// loop through buttons and look for a press
			for (i = 0; i < (sizeof(mouse_buttons_map) / sizeof(int)); i++) {
				if (event->button.button == i) {
					input_enable((Uint8)mouse_buttons_map[i]);
					break;
				}
			}
			break;
		case SDL_MOUSEBUTTONUP:
			// added by ScottD
			for (i = 0; i < (sizeof(mouse_buttons_map) / sizeof(int)); i++) {
				if (event->button.button == i) {
					input_disable((Uint8)mouse_buttons_map[i]);
					break;
				}
			}
			break;
			
		case SDL_MOUSEMOTION:
			// added by ScottD
			g_game->OnMouseMotion(event->motion.x, event->motion.y, event->motion.xrel, event->motion.yrel);
			break;
			*/
		case SDL_QUIT:
			// if they are trying to close the window
			set_quitflag();
			break;
		default:
			break;
	}

	if (g_game->get_mouse_enabled() && (g_mouse_mode == SDL_MOUSE))
	{
		switch (event->type)
		{
		case SDL_MOUSEBUTTONDOWN:
			// added by ScottD
			// loop through buttons and look for a press
			for (i = 0; i < (sizeof(mouse_buttons_map) / sizeof(int)); i++) {
				if (event->button.button == i) {
					//input_enable((Uint8)mouse_buttons_map[i]);
					g_game->input_enable((Uint8)mouse_buttons_map[i], NOMOUSE);
					break;
				}
			}
			break;
		case SDL_MOUSEBUTTONUP:
			// added by ScottD
			for (i = 0; i < (sizeof(mouse_buttons_map) / sizeof(int)); i++) {
				if (event->button.button == i) {
					//input_disable((Uint8)mouse_buttons_map[i]);
					g_game->input_disable((Uint8)mouse_buttons_map[i], NOMOUSE);
					break;
				}
			}
			break;
			
		case SDL_MOUSEMOTION:
			// added by ScottD
			g_game->OnMouseMotion(event->motion.x, event->motion.y, event->motion.xrel, event->motion.yrel, NOMOUSE); // Last parameter is for ManyMouse use only.
			break;
		}
	}

	// added by JFA for -idleexit
	if (get_idleexit() > 0 && elapsed_ms_time(idle_timer) > get_idleexit()) set_quitflag();

}

// if a key is pressed, we go here
void process_keydown(SDLKey key)
{	
	// go through each key def (defined in enum in daphne.h) and check to see if the key entered matches
	// If we have a match, the switch to be used is the value of the index "move"
	for (Uint8 move = 0; move < SWITCH_COUNT; move++)
	{
		if ((key == g_key_defs[move][0]) || (key == g_key_defs[move][1]))
		{
			input_enable(move, NOMOUSE);
		}
	}
	
	// check for ALT-ENTER here
	if ((key == SDLK_LALT) || (key == SDLK_RALT))
	{
		g_alt_pressed = true;
	}
	else if ((key == SDLK_RETURN) && (g_alt_pressed))
	{
		vid_toggle_fullscreen();
	}
	// end ALT-ENTER check
}

// if a key is released, we go here
void process_keyup(SDLKey key)
{
	// go through each key def (defined in enum in daphne.h) and check to see if the key entered matches
	// If we have a match, the switch to be used is the value of the index "move"
	for (Uint8 move = 0; move < SWITCH_COUNT; move++)
	{
		if ((key == g_key_defs[move][0]) || (key == g_key_defs[move][1]))
		{
			input_disable(move, NOMOUSE);
		}
	}

	// if they are releasing an ALT key	
	if ((key == SDLK_LALT) || (key == SDLK_RALT))
	{
		g_alt_pressed = false;
	}
}

// processes movements of the joystick
void process_joystick_motion(SDL_Event *event)
{

	static int x_axis_in_use = 0;	// true if joystick is left or right
	static int y_axis_in_use = 0;	// true if joystick is up or down

	// if they are moving along the verticle axis
	if (event->jaxis.axis == 1)
	{
		// if they're moving up
		if (event->jaxis.value < -JOY_AXIS_MID)
		{
			input_enable(SWITCH_UP, NOMOUSE);
			y_axis_in_use = 1;
		}
		// if they're moving down
		else if (event->jaxis.value > JOY_AXIS_MID)
		{
			input_enable(SWITCH_DOWN, NOMOUSE);
			y_axis_in_use = 1;
		}

		// if they just barely stopped moving up or down
		else if (y_axis_in_use == 1)
		{
			input_disable(SWITCH_UP, NOMOUSE);
			input_disable(SWITCH_DOWN, NOMOUSE);
			y_axis_in_use = 0;
		}
	} // end verticle axis

	// horizontal axis
	else if (event->jaxis.axis == 0)
	{
		// if they're moving right
		if (event->jaxis.value > JOY_AXIS_MID)
		{
			input_enable(SWITCH_RIGHT, NOMOUSE);
			x_axis_in_use = 1;
		}
		// if they're moving left
		else if (event->jaxis.value < -JOY_AXIS_MID)
		{
			input_enable(SWITCH_LEFT, NOMOUSE);
			x_axis_in_use = 1;
		}
		// if they just barely stopped moving right or left
		else if (x_axis_in_use == 1)
		{
			input_disable(SWITCH_RIGHT, NOMOUSE);
			input_disable(SWITCH_LEFT, NOMOUSE);
			x_axis_in_use = 0;
		}
	} // end horizontal axis
}

// processes movement of the joystick hat
void process_joystick_hat_motion(SDL_Event *event)
{

	static Uint8 prev_hat_position = SDL_HAT_CENTERED; 

	if ( ( event->jhat.value & SDL_HAT_UP ) && !( prev_hat_position & SDL_HAT_UP ) )
	{
		// hat moved to the up position
		input_enable(SWITCH_UP, NOMOUSE);
	}
	else if ( !( event->jhat.value & SDL_HAT_UP ) && ( prev_hat_position & SDL_HAT_UP ) )
	{ 
		// up hat released
		input_disable(SWITCH_UP, NOMOUSE);
	}

	if ( ( event->jhat.value & SDL_HAT_RIGHT ) && !( prev_hat_position & SDL_HAT_RIGHT ) )
	{
		// hat moved to the right position
		input_enable(SWITCH_RIGHT, NOMOUSE);
	}
	else if ( !( event->jhat.value & SDL_HAT_RIGHT ) && ( prev_hat_position & SDL_HAT_RIGHT ) )
	{ 
		// right hat released
		input_disable(SWITCH_RIGHT, NOMOUSE);
	}

	if ( ( event->jhat.value & SDL_HAT_DOWN ) && !( prev_hat_position & SDL_HAT_DOWN ) )
	{
		// hat moved to the down position
		input_enable(SWITCH_DOWN, NOMOUSE);
	}
	else if ( !( event->jhat.value & SDL_HAT_DOWN ) && ( prev_hat_position & SDL_HAT_DOWN ) )
	{ 
		// down hat released
		input_disable(SWITCH_DOWN, NOMOUSE);
	}

	if ( ( event->jhat.value & SDL_HAT_LEFT ) && !( prev_hat_position & SDL_HAT_LEFT ) )
	{
		// hat moved to the left position
		input_enable(SWITCH_LEFT, NOMOUSE);
	}
	else if ( !( event->jhat.value & SDL_HAT_LEFT ) && ( prev_hat_position & SDL_HAT_LEFT ) )
	{ 
		// left hat released
		input_disable(SWITCH_LEFT, NOMOUSE);
	}

	prev_hat_position = event->jhat.value;
}

// functions to help us avoid 'extern' statements
bool get_consoledown()
{
	return (g_consoledown);
}

void set_consoledown (bool value)
{
	g_consoledown = value;
}

// draws console if it's down and if there's been enough of a delay
void check_console_refresh()
{

	static unsigned int console_refresh = 0;
	const unsigned int refresh_every = 125;	// refreshes console every (this many) ms

	if (g_consoledown)
	{
		if (elapsed_ms_time(console_refresh) > refresh_every)
		{
			DrawConsole();
			vid_blit(get_screen_blitter(), 0, 0);
			vid_flip();
			console_refresh = refresh_ms_time();
		}
	}
}

// if user has pressed a key/moved the joystick/pressed a button
void input_enable(Uint8 move, int mouseID)
{
	// first test universal input, then pass unknown input on to the game driver

	switch (move)
	{
	default:
		g_game->input_enable(move, mouseID);
		break;
	case SWITCH_RESET:
		g_game->reset();
		break;
	case SWITCH_SCREENSHOT:		
		g_ldp->request_screenshot();
		break;
	case SWITCH_PAUSE:
		if (g_game->get_pause_key_flag())  // rdg
			g_game->toggle_game_pause();
		break;
	case SWITCH_QUIT:
		set_quitflag();
		break;
	case SWITCH_COIN1:
	case SWITCH_COIN2:
		// coin inputs are buffered to ensure that they are not dropped while the cpu is busy (such as during a seek)
		// therefore if the input is coin1 or coin2 AND we are using a real cpu (and not a program such as seektest)
		//if (get_cpu_hz(0) > 0)
		{
			add_coin_to_queue(true, move);
		}
		break;
	case SWITCH_CONSOLE:
		// we must not bring down the console if blitting is not allowed
		if (g_ldp->is_blitting_allowed())
		{
#ifdef CPU_DEBUG
			toggle_console();
#endif
		}
		break;
	}
}

// if user has released a key/released a button/moved joystick back to center position
void input_disable(Uint8 move, int mouseID)
{
	// don't send reset or screenshots key-ups to the individual games because they will return warnings that will alarm users
	if ((move != SWITCH_RESET) && (move != SWITCH_SCREENSHOT) && (move != SWITCH_QUIT)
		&& (move != SWITCH_PAUSE))
	{
		// coin inputs are buffered to ensure that they are not dropped while the cpu is busy (such as during a seek)
		// therefore if the input is coin1 or coin2 AND we are using a real cpu (and not a program such as seektest)
		//if (((move == SWITCH_COIN1) || (move == SWITCH_COIN2)) && (get_cpu_hz(0) > 0))
		if ((move == SWITCH_COIN1) || (move == SWITCH_COIN2))
		{
			add_coin_to_queue(false, move);
		}
		else
		{
			g_game->input_disable(move, mouseID);
		}
	}
	// else do nothing
}

inline void add_coin_to_queue(bool enabled, Uint8 val)
{
	Uint64 total_cycles = 0;//get_total_cycles_executed(0);
	struct coin_input coin;
	coin.coin_enabled = enabled;
	coin.coin_val = val;

	// make sure that we are >= to the total cycles executed otherwise coin insertions will be really quick
	if (g_last_coin_cycle_used < total_cycles)
	{
		g_last_coin_cycle_used = total_cycles;
	}
	g_last_coin_cycle_used += g_sticky_coin_cycles;	// advance to the next safe slot
	coin.cycles_when_to_enable = g_last_coin_cycle_used;	// and assign this safe slot to this current coin
	g_coin_queue.push(coin);	// add the coin to the queue ...
}

// added by JFA for -idleexit
void reset_idle(void)
{
	static bool bSoundOn = false;

	// At this time, the only way the sound will be muted is if -startsilent was passed via command line.
	// So the first key press should always unmute the sound.
	if (!bSoundOn)
	{
		bSoundOn = true;
		set_sound_mute(false);
	}

	idle_timer = refresh_ms_time();
}
// end edit

// primarily to disable joystick use if user wishes not to use one
void set_use_joystick(bool val)
{
	g_use_joystick = val;
}

bool set_mouse_mode(int thisMode)
{
	bool result = false;

	if (g_game->get_mouse_enabled())
	{
		if (g_mouse_mode == MANY_MOUSE) ManyMouse_Quit();
		if (thisMode == MANY_MOUSE || thisMode == SDL_MOUSE)
		{			
			g_mouse_mode = thisMode;
			result = true;
			memset(mouse_buttons_map, 0, sizeof(mouse_buttons_map)); // Erase array
			if (thisMode == SDL_MOUSE) 
			{
				
				mouse_buttons_map[0] =	SWITCH_BUTTON1;  // 0 (Left Button)
				mouse_buttons_map[1] =	SWITCH_BUTTON3;  // 1 (Middle Button)
				mouse_buttons_map[2] =	SWITCH_BUTTON2;  // 2 (Right Button)
				mouse_buttons_map[3] =	SWITCH_BUTTON1;  // 3 (Wheel Up)
				mouse_buttons_map[4] =	SWITCH_BUTTON2;  // 4 (Wheel Down)		
				mouse_buttons_map[5] =	SWITCH_MOUSE_DISCONNECT;  // Dummy value so array matches in both branches.
			}
			else
			{
				mouse_buttons_map[0] =	SWITCH_BUTTON3;  // 0 (Left Button)
				mouse_buttons_map[1] =	SWITCH_BUTTON1;  // 1 (Middle Button)
				mouse_buttons_map[2] =	SWITCH_BUTTON2;  // 2 (Right Button)
				mouse_buttons_map[3] =	SWITCH_MOUSE_SCROLL_UP;  // 3 (Wheel Up)
				mouse_buttons_map[4] =	SWITCH_MOUSE_SCROLL_DOWN;  // 4 (Wheel Down)		
				mouse_buttons_map[5] =	SWITCH_MOUSE_DISCONNECT;  // 4 (Wheel Down)	
				
				ManyMouse_Init_Mice();

			} // end if

		} // end if

	} // end if

	return result;

}

// Allow us to specify an alternate keymap.ini file
void set_inputini_file(const char *inputFile)
{
	m_altInputFileSet = true;
	g_inputini_file = inputFile;
}

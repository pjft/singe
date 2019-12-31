/*
* singe.cpp
*
* Copyright (C) 2006 Scott C. Duensing
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

/*
* This is SINGE - the Somewhat Interactive Nostalgic Game Engine!
*/

#include <string.h>
#include "singe.h"
#include "singe/singe_interface.h"


// Win32 doesn't use strcasecmp, it uses stricmp (lame)
#ifdef WIN32
#define strcasecmp stricmp
#endif

////////////////////////////////////////////////////////////////////////////////

// For intercepting the VLDP MPEG data

extern       struct vldp_in_info   g_local_info;
extern const struct vldp_out_info *g_vldp_info;
extern SDL_Surface *g_screen_blitter;
//extern SDL_Surface *g_screen;
//extern SDL_Surface *get_screen_blitter();
extern void vid_blank();
extern void vid_flip();
extern void vid_blit(SDL_Surface *srf, int x, int y);

////////////////////////////////////////////////////////////////////////////////

typedef const struct singe_out_info *(*singeinitproc)(const struct singe_in_info *in_info);

// pointer to all info SINGE PROXY gives to us
const struct singe_out_info *g_pSingeOut = NULL;

// info that we provide to the SINGE PROXY DLL
struct singe_in_info g_SingeIn;

////////////////////////////////////////////////////////////////////////////////

// by RDG2010
const int singe::i_full_keybd_defs[] = {SDLK_BACKSPACE, SDLK_TAB, SDLK_RETURN, SDLK_PAUSE, 
										SDLK_SPACE, SDLK_QUOTE, SDLK_COMMA, SDLK_SEMICOLON, 
										SDLK_EQUALS, SDLK_LEFTBRACKET, SDLK_RIGHTBRACKET, 
										SDLK_BACKSLASH, SDLK_SLASH, SDLK_DELETE, SDLK_PERIOD };

#define KEYBD_ARRAY_SIZE 15

singe::singe()
{
	m_strGameScript = "";
	m_shortgamename = "singe";
	m_strScriptPath = "";
	m_strName = "[Undefined scripted game]";
	m_video_overlay_width = 320;	// sensible default
	m_video_overlay_height = 240;	// " " "
	m_palette_color_count = 256;
	m_overlay_size_is_dynamic = true;	// this 'game' does reallocate the size of its overlay
	m_bMouseEnabled = true;
	m_dll_instance = NULL;
	// by RDG2010
	m_game_type = GAME_SINGE;
	i_keyboard_mode = KEYBD_NORMAL;

}

bool singe::init()
{
	bool bSuccess = false;
	singeinitproc pSingeInit;	// pointer to the init proc ...

#ifndef STATIC_SINGE	// if we're loading SINGE dynamically
#ifndef DEBUG
	m_dll_instance = M_LOAD_LIB(singe);	// load SINGE.DLL
#else
	m_dll_instance = M_LOAD_LIB(singe_dbg);	// load SINGED.DLL (debug version)
#endif

	// If the handle is valid, try to get the function address. 
	if (m_dll_instance)
	{
		pSingeInit = (singeinitproc) M_GET_SYM(m_dll_instance, "singeproxy_init");

		// if init function was found
		if (pSingeInit)
		{
			bSuccess = true;
		}
		else
		{
			printerror("SINGE LOAD ERROR : singeproxy_init could not be loaded");
		}
	}
	else
	{
		printerror("ERROR: could not open the SINGE dynamic library (file not found maybe?)");
	}

#else // else if we're loading SINGE statically
	pSingeInit = singeproxy_init;
	bSuccess = true;
#endif // STATIC_SINGE

	// if pSingeInit is valid ...
	if (bSuccess)
	{
		// DLL basics can be confusing so I am writing this down to remind myself in the future.
		// These g_SingeIn declarations wires functions located on the daphne side
		// so they can be called on the DLL side of singe.
		// An alias is used by the DLL source code to refer to them. 
		//
		// Looking below:
		// The name on the right of the declaration is the actual daphne function or variable.
		// The name on the left of the declaration is the alias used by the DLL.

		// Before you can add a g_SingeIn declaration here, you must have 
		// the function properly declared in singe.h

        g_SingeIn.uVersion = SINGE_INTERFACE_API_VERSION;
		g_SingeIn.printline = printline;
		g_SingeIn.set_quitflag = set_quitflag;
		g_SingeIn.disable_audio1 = disable_audio1;
		g_SingeIn.disable_audio2 = disable_audio2;
		g_SingeIn.enable_audio1 = enable_audio1;
		g_SingeIn.enable_audio2 = enable_audio2;
		g_SingeIn.framenum_to_frame = framenum_to_frame;
		g_SingeIn.get_current_frame = get_current_frame;
		g_SingeIn.pre_change_speed = pre_change_speed;
		g_SingeIn.pre_pause = pre_pause;
		g_SingeIn.pre_play = pre_play;
		g_SingeIn.pre_search = pre_search;
		g_SingeIn.pre_skip_backward = pre_skip_backward;
		g_SingeIn.pre_skip_forward = pre_skip_forward;
		g_SingeIn.pre_step_backward = pre_step_backward;
		g_SingeIn.pre_step_forward = pre_step_forward;
		g_SingeIn.pre_stop = pre_stop;
		g_SingeIn.set_search_blanking = set_search_blanking;
		g_SingeIn.set_skip_blanking = set_skip_blanking;
		g_SingeIn.g_local_info = &g_local_info;
		g_SingeIn.g_vldp_info = g_vldp_info;
		g_SingeIn.get_video_height = get_video_height;
		g_SingeIn.get_video_width = get_video_width;
		g_SingeIn.draw_string = draw_string;
		g_SingeIn.samples_play_sample = samples_play_sample;
		g_SingeIn.set_last_error = set_last_error;
		
		// by RDG2010
		g_SingeIn.get_vldp_status = get_vldp_status;
		g_SingeIn.get_singe_version = get_singe_version;
		g_SingeIn.set_ldp_verbose = set_ldp_verbose;
		g_SingeIn.samples_set_state = samples_set_state;
		g_SingeIn.samples_is_sample_playing = samples_is_sample_playing;
		g_SingeIn.samples_end_early = samples_end_early;		
		g_SingeIn.set_soundchip_nonvldp_volume = set_soundchip_nonvldp_volume;
		g_SingeIn.get_soundchip_nonvldp_volume = get_soundchip_nonvldp_volume;
		g_SingeIn.samples_flush_queue = samples_flush_queue;
		//g_SingeIn.g_screen_blitter = g_screen_blitter;

		// Special cases where it is needed to access or change values 
		// inside the singe class require special wrapper functions.
		// These functions allow the DLL side of SINGE call the 
		// functions set_keyboard_mode and get_keyboard_mode inside this very class.
		// Take a look a singe.h for the full declarations of these functions:		
		
		g_SingeIn.dll_side_set_keyboard_mode  = daphne_side_set_keyboard_mode;
		g_SingeIn.dll_side_get_keyboard_mode  = daphne_side_get_keyboard_mode;
		g_SingeIn.dll_side_get_script_path    = daphne_side_get_script_path;
		g_SingeIn.dll_side_set_caption        = daphne_side_set_caption;		
		g_SingeIn.dll_side_mouse_enable       = daphne_side_mouse_enable;
		g_SingeIn.dll_side_mouse_disable      = daphne_side_mouse_disable;
		g_SingeIn.dll_side_mouse_get_how_many = daphne_side_mouse_get_how_many;
		g_SingeIn.dll_side_set_mouse_mode     = daphne_side_set_mouse_mode;		
		g_SingeIn.dll_side_pause_enable       = daphne_side_pause_enable;
		g_SingeIn.dll_side_pause_disable      = daphne_side_pause_disable;

		/*
		Why a wrapper?

		A function on the DLL side can't change or refer to a value
		inside the singe class because the pointer types don't match.
		Go around this by creating a special pointer to this class
		and also declaring a function between the Singe method inside 
		the class and the DLL function that needs access to it.	

		Pointer declaration below...
		*/

		g_SingeIn.pSingeInstance = this;

		// establish link betwixt singe proxy and us
		g_pSingeOut = pSingeInit(&g_SingeIn);	

#ifdef WIN32
		// do something here....
		
#else
		// version check
		if (g_pSingeOut->uVersion != SINGE_INTERFACE_API_VERSION)
		{
			printline("Singe API version mismatch!  Something needs to be recompiled...");
			bSuccess = false;
		}
#endif

	}

	// if we're not using VLDP, then singe will segfault, so abort ...
	if (g_vldp_info == NULL)
	{
		printerror("You must use VLDP when using Singe.");
		bSuccess = false;	
	}
	if (!bSuccess)
	{
#ifndef STATIC_SINGE
		M_FREE_LIB(m_dll_instance);
#endif // STATIC_SINGE


		
	}

	return bSuccess;
}

void singe::start()
{
	int intReturn = 0;
	char s1[100];
	sprintf(s1,"Starting Singe version %.2f",get_singe_version());
	printline(s1);
	// comment these two lines for official releases -- rdg
	//sprintf(s1,"Singe v%.2f BETA", get_singe_version());
	SDL_WM_SetCaption(s1, "Singe");
	// BETA
	
	g_pSingeOut->sep_set_surface(m_video_overlay_width, m_video_overlay_height);
	g_pSingeOut->sep_set_static_pointers(&m_disc_fps, &m_uDiscFPKS);
	g_pSingeOut->sep_startup(m_strGameScript.c_str());

	// if singe didn't get an error during startup...
	if (!get_quitflag())
	{
		while (!get_quitflag())
		{
			g_pSingeOut->sep_call_lua("onOverlayUpdate", ">i", &intReturn);
			if (intReturn == 1)
			{
				m_video_overlay_needs_update = true;
			}
			video_blit();
			SDL_check_input();
			samples_do_queued_callbacks();	// hack to ensure sound callbacks are called at a time when lua can accept them without crashing
			g_ldp->think_delay(10);	// don't hog cpu, and advance timer
		}

		g_pSingeOut->sep_call_lua("onShutdown", "");
	} // end if there was no startup error

	// always call sep_shutdown just to make sure everything gets cleaned up
	g_pSingeOut->sep_shutdown();
}

void singe::shutdown()
{
#ifndef STATIC_SINGE
	// if a DLL is loaded, then free it
	if (m_dll_instance)
	{
		M_FREE_LIB(m_dll_instance);
		m_dll_instance = NULL;
	}
	// else do nothing ...
#endif // STATIC_SINGE
}

void singe::input_enable(Uint8 input, int mouseID)
{
	if (g_pSingeOut) // by RDG2010
		g_pSingeOut->sep_call_lua("onInputPressed", "ii", input, mouseID);
}

void singe::input_disable(Uint8 input, int mouseID)
{
	if (g_pSingeOut) // by RDG2010
		g_pSingeOut->sep_call_lua("onInputReleased", "ii", input, mouseID);
}

void singe::OnMouseMotion(Uint16 x, Uint16 y, Sint16 xrel, Sint16 yrel, Uint16 mouseID)
{
	if (g_pSingeOut)
	{
		g_pSingeOut->sep_do_mouse_move(x, y, xrel, yrel, mouseID);
	}
}

// game-specific command line arguments handled here
bool singe::handle_cmdline_arg(const char *s)
{
	
	bool bResult = false;
	static bool scriptLoaded = false;

		if (mpo_file_exists(s))
		{
			if (!scriptLoaded)
			{
				bResult = scriptLoaded = true;
				m_strGameScript = s;
			}
			else
			{
				printline("Only one game script may be loaded at a time!");				
			}
		}
		else
		{
			string strErrMsg = "Script ";
			strErrMsg += s;
			strErrMsg += " does not exist.";
			printline(strErrMsg.c_str());
		}

	/*
	char s[81] = { 0 };

	if (strcasecmp(arg, "-script") == 0)
	{
		get_next_word(s, sizeof(s));

		if (mpo_file_exists(s))
		{
			if (!scriptLoaded)
			{
				bResult = scriptLoaded = true;
				m_strGameScript = s;
			}
			else
			{
				printline("Only one game script may be loaded at a time!");
				bResult = false;
			}
		}
		else
		{
			string strErrMsg = "Script ";
			strErrMsg += s;
			strErrMsg += " does not exist.";
			printline(strErrMsg.c_str());
		}

	}
	*/

	return bResult;

}


void singe::palette_calculate()
{
	SDL_Color temp_color;

	temp_color.unused = 0; // Eliminates a warning.

	// go through all colors and compute the palette
	// (start at 2 because 0 and 1 are a special case)
	for (unsigned int i = 2; i < 256; i++)
	{
		temp_color.r = i & 0xE0;					// Top 3 bits for red
		temp_color.g = (i << 3) & 0xC0;		// Middle 2 bits for green
		temp_color.b = (i << 5) & 0xE0;		// Bottom 3 bits for blue
		palette_set_color(i, temp_color);
	}

	// special case: 00 is reserved for transparency, so 01 becomes fully black
	temp_color.r = temp_color.g = temp_color.b = 0;
	palette_set_color(1, temp_color);

	// safety : 00 should never be visible so we'll make it a bright color to help us
	//  catch errors
	temp_color.r = temp_color.g = temp_color.b = 0xFF;
	palette_set_color(0, temp_color);
}


// redraws video
void singe::video_repaint()
{
	Uint32 cur_w = g_ldp->get_discvideo_width() >> 1;	// width overlay should be
	Uint32 cur_h = g_ldp->get_discvideo_height() >> 1;	// height overlay should be

	// if the width or height of the mpeg video has changed since we last were here (ie, opening a new mpeg)
	// then reallocate the video overlay buffer
	if ((cur_w != m_video_overlay_width) || (cur_h != m_video_overlay_height))
	{
		if (g_ldp->lock_overlay(1000))
		{
			m_video_overlay_width = cur_w;
			m_video_overlay_height = cur_h;

			g_pSingeOut->sep_set_surface(m_video_overlay_width, m_video_overlay_height);

			video_shutdown();
			if (!video_init())
			{
				printline("Fatal Error, trying to re-create the surface failed!");
				set_quitflag();
			}
			g_ldp->unlock_overlay(1000);	// unblock game video overlay
		}
		else
		{
			g_pSingeOut->sep_print("Timed out trying to get a lock on the yuv overlay");
			return;
		}
	} // end if dimensions are incorrect

	g_pSingeOut->sep_do_blit(m_video_overlay[m_active_video_overlay]);	
	
	//g_pSingeOut->sep_do_blit(g_screen_blitter);	

	//vid_blank();	
	//vid_blit(g_screen_blitter, 0, 0);
	//SDL_BlitSurface(g_screen_blitter, NULL, g_screen, NULL);
	//vid_flip();
	
}

void singe::set_last_error(const char *cpszErrMsg)
{
	// TODO : figure out reliable way to call printerror (maybe there isn't one?)
	printline(cpszErrMsg);
}

// by RDG2010
void singe::set_keyboard_mode(int thisVal)
{
	if (thisVal != KEYBD_NORMAL && thisVal != KEYBD_FULL) 
	{
		//printline("Singe tried to se an invalid keyboard mode. Defaulting to normal.");
		i_keyboard_mode = KEYBD_NORMAL;
	}
	else
		i_keyboard_mode = thisVal;
	
}

int singe::get_keyboard_mode()
{
	return i_keyboard_mode;
}

double singe::get_singe_version()
{
	double thisVersion = SINGE_VERSION;
	return thisVersion;
}

// Have SINGE deal directly with SDL input
// This handles when a key is pressed down
void singe::process_keydown(SDLKey key, int keydefs[][2])
{
	/* Normal Daphne use has the program look for a set of default keys.
	 * These are read from daphne.ini (or if daphne.ini is absent, then set
	 * a default configuration). The rest of the keyboard is ignored.
	 * This is the default mode that works for most gamees.
	 *
	 * The alternate mode is to scan for all keys.
	 * In this mode daphne.ini settings are ignored.
	 * The ESCAPE key is hardwired to quit.
	 * 
	 * i_keyboard_mode stores the scanning mode.
	 * It is set by default to KEYBD_NORMAL.
	 * A singe script can change this to KEYBD_FULL
	 * typing "keyboardSetMode" command in the lua
	 * scripting side.
	 *
	 * RDG2010					 
	 * 
	 * */

		if (i_keyboard_mode == KEYBD_NORMAL)  // Using normal keyboard mappings
		{	// traverse the keydef array for mapped keys.
			for (Uint8 move = 0; move < SWITCH_COUNT; move++)
			{
				if ((key == keydefs[move][0]) || (key == keydefs[move][1]))
				{
					if (move != SWITCH_PAUSE) input_enable(move, NOMOUSE);
				}
			}

		} else { // Using full keyboard access....
			
			if (key >= SDLK_a && key <= SDLK_z)	
				input_enable(key, NOMOUSE);	
			// check to see if key is a number on the top row of the keyboard (not keypad)
			else if (key >= SDLK_MINUS && key <= SDLK_9) 
				input_enable(key, NOMOUSE);	
			// numeric keypad keys
			else if (key >= SDLK_KP0 && key <= SDLK_KP_EQUALS) 
				input_enable(key, NOMOUSE);
			// arrow keys and insert, delete, home, end, pgup, pgdown
			else if (key >= SDLK_UP && key <= SDLK_PAGEDOWN) 
				input_enable(key, NOMOUSE);	
			// function keys
			else if (key >= SDLK_F1 && key <= SDLK_F15) 
				input_enable(key, NOMOUSE);	
			// Key state modifier keys (left and right ctrls, alts)
			else if (key >= SDLK_NUMLOCK && key <= SDLK_LMETA) 
				input_enable(key, NOMOUSE);	
			// International keys
			else if (key >= SDLK_WORLD_0 && key <= SDLK_WORLD_95) 
				input_enable(key, NOMOUSE);				
			else
			{

				/*
				* SDLK_BACKSPACE, SDLK_TAB, SDLK_RETURN, SDLK_PAUSE, 
				* SDLK_SPACE, SDLK_QUOTE, SDLK_COMMA, SDLK_SEMICOLON, 
				* SDLK_EQUALS, SDLK_LEFTBRACKET, SDLK_RIGHTBRACKET, 
				* SDLK_BACKSLASH, SDLK_SLASH, SDLK_DELETE, SDLK_PERIOD };
				*/

				for (int k=0;k<KEYBD_ARRAY_SIZE;k++)
				{
					if (key == i_full_keybd_defs[k])
					{
						input_enable(key, NOMOUSE);
						break;
					} // end if

				} // end for

			} // endif		

		} // endif

	
}

// this is called when the user releases a key
void singe::process_keyup(SDLKey key, int keydefs[][2])
{
	/* Process_keyup is handled differently in SINGE.
	 * 
	 * Array key_defs has the key mappings set in daphne.ini (or defaults).
	 * 
	 * RDG2010					 
	 * 
	 * */

	{	// check keyboard mode
		if (i_keyboard_mode == KEYBD_NORMAL)  // Using normal keyboard mappings
		{	// traverse the keydef array for mapped keys.

			// Handle pause and quit keypresses first.
			if (key == keydefs[SWITCH_PAUSE][0] || key == keydefs[SWITCH_PAUSE][1]) 
			{
				if (g_game->get_pause_key_flag())  // rdg
				{
					g_game->toggle_game_pause();				
					//input_disable(SWITCH_PAUSE, NOMOUSE);  // -1 is for no mouse.
					
				}
				input_disable(SWITCH_PAUSE, NOMOUSE);  // -1 is for no mouse.
				
			} else if (key == keydefs[SWITCH_QUIT][0] || key == keydefs[SWITCH_QUIT][1]) {
				
				set_quitflag();	
							
			} else if (key == keydefs[SWITCH_SCREENSHOT][0]) {

				printline("Screenshot requested!");
				g_ldp->request_screenshot();

			} else {

				for (Uint8 move = 0; move < SWITCH_COUNT; move++)
				{
					if ((key == keydefs[move][0]) || (key == keydefs[move][1]))
					{
						if (move != SWITCH_PAUSE) input_disable(move, NOMOUSE);
					}

				} // end for

			} // endif

		} else { // Using full keyboard access....

			// Hardwire ESCAPE key to quit
			if (key == SDLK_ESCAPE)	
				set_quitflag();	
			// letter keys
			else if (key >= SDLK_a && key <= SDLK_z)	
				input_disable(key, NOMOUSE);	
			// check to see if key is a number on the top row of the keyboard (not keypad)
			else if (key >= SDLK_MINUS && key <= SDLK_9)
				input_disable(key, NOMOUSE);			
			// numeric keypad keys
			else if (key >= SDLK_KP0 && key <= SDLK_KP_EQUALS) 
				input_disable(key, NOMOUSE);	
			// arrow keys and insert, delete, home, end, pgup, pgdown
			else if (key >= SDLK_UP && key <= SDLK_PAGEDOWN) 
				input_disable(key, NOMOUSE);	
			// function keys
			else if (key >= SDLK_F1 && key <= SDLK_F15) 
				input_disable(key, NOMOUSE);	
			// Key state modifier keys (left and right ctrls, alts)
			else if (key >= SDLK_NUMLOCK && key <= SDLK_LMETA) 
				input_disable(key, NOMOUSE);	
			// International keys
			else if (key >= SDLK_WORLD_0 && key <= SDLK_WORLD_95) 
				input_disable(key, NOMOUSE);				
			else
			{
				/*
				* SDLK_BACKSPACE, SDLK_TAB, SDLK_RETURN, SDLK_PAUSE, 
				* SDLK_SPACE, SDLK_QUOTE, SDLK_COMMA, SDLK_SEMICOLON, 
				* SDLK_EQUALS, SDLK_LEFTBRACKET, SDLK_RIGHTBRACKET, 
				* SDLK_BACKSLASH, SDLK_SLASH, SDLK_DELETE, SDLK_PERIOD };
				*/

				for (int k=0;k<KEYBD_ARRAY_SIZE;k++)
				{
					if (key == i_full_keybd_defs[k])
					{
						input_disable(key, NOMOUSE);
						break;
					} // end if

				} // end for

			} // endif	

		} // endif

	} // endif	

}

void singe::set_game_name(char *thisName) // Sets value of m_strName;
{// basic_string& assign ( const charT* s, size_type n );
	m_strName.clear();
	m_strName.assign(thisName,MAX_TITLE_LENGTH);	
}

void singe::change_caption(char *thisName)
{
	char s1[100];
	set_game_name(thisName);
	sprintf(s1,"Singe v%.2f - %s", get_singe_version(), m_strName.c_str());
	//sprintf(s1,"%s", m_strName.c_str());
	SDL_WM_SetCaption(s1, "Singe");
}

void singe::set_script_path(const char *thisPath)
{
	string s1(thisPath);
	string s2(thisPath);
	size_t found = 9999;
	found=s1.find_first_of("\\");
	while (found <= 9999)
	{		
		if (found != 9999)
		{
			s2.erase();
			s2.append(s1.substr(found+1,s1.length()));
			s1 = s1.substr(0,found);
			s1.append("/");
			s1.append(s2.c_str());
			found=s1.find_first_of("\\");
		}
	}
	found=s1.find_last_of("/");
	m_strScriptPath.erase();	
	m_strScriptPath.append(s1.substr(0,found+1));

}

void singe::get_script_path(char *thisPath)
{
	sprintf(thisPath,"%.255s", m_strScriptPath.c_str());
}

/*
void singe::set_script_path(const char *thisPath)
{
	string s1(thisPath);
	string s2(thisPath);
	size_t found = 9999;
	found=s1.find_first_of("/");
	while (found <= 9999)
	{		
		if (found != 9999)
		{
			s2.erase();
			s2.append(s1.substr(found+1,s1.length()));
			s1 = s1.substr(0,found);
			s1.append("\\");
			s1.append(s2.c_str());
			found=s1.find_first_of("/");
		}
	}
	m_strScriptPath.erase();	
	m_strScriptPath.append(s1.c_str());

}
*/

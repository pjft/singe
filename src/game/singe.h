/*
 * singe.h
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

#include "../daphne.h"           // for get_quitflag
#include "game.h"
#include "../io/conout.h"
#include "../io/mpo_fileio.h"
#include "../io/error.h"
#include "../io/dll.h"
#include "../io/cmdline.h"  // for get_next_word  (rdg2010)
#include "../io/input.h"    // for set_mouse_mode rdg
#include "../ldp-out/ldp.h"
#include "../sound/samples.h"
#include "../video/palette.h"
#include "../video/video.h"
#include "../vldp2/vldp/vldp.h"  // to get the vldp structs

#include <string>

using namespace std;

// by rdg2010
// INCREASE THIS NUMBER EVERY TIME YOU CHANGE SOMETHING IN SINGE!!!
#define SINGE_VERSION 1.18
#define NOMOUSE -1
enum { KEYBD_NORMAL, KEYBD_FULL };

////////////////////////////////////////////////////////////////////////////////

class singe : public game
{
public:
	singe();
	bool init();
	void start();
	void shutdown();
	void input_enable(Uint8, int);
	void input_disable(Uint8, int);
	void OnMouseMotion(Uint16 x, Uint16 y, Sint16 xrel, Sint16 yrel, Uint16 mouseID);
	bool handle_cmdline_arg(const char *arg);
	void palette_calculate();
	void video_repaint();

	// g_ldp function wrappers (to make function pointers out of them)
	static void enable_audio1() { g_ldp->enable_audio1(); }
	static void enable_audio2() { g_ldp->enable_audio2(); }
	static void disable_audio1() { g_ldp->disable_audio1(); }
	static void disable_audio2() { g_ldp->disable_audio2(); }
	static void request_screenshot() { g_ldp->request_screenshot(); }
	static void set_search_blanking(bool enabled) { g_ldp->set_search_blanking(enabled); }
	static void set_skip_blanking(bool enabled) { g_ldp->set_skip_blanking(enabled); }
	static bool pre_change_speed(unsigned int uNumerator, unsigned int uDenominator)
	{
		return g_ldp->pre_change_speed(uNumerator, uDenominator);
	}
	static unsigned int get_current_frame() { return g_ldp->get_current_frame(); }
	static void pre_play() { g_ldp->pre_play(); }
	static void pre_pause() { g_ldp->pre_pause(); }
	static void pre_stop() { g_ldp->pre_stop(); }
	static bool pre_search(const char *cpszFrame, bool block_until_search_finished)
	{
		return g_ldp->pre_search(cpszFrame, block_until_search_finished);
	}
	static void framenum_to_frame(Uint32 u16Frame, char *pszFrame) { g_ldp->framenum_to_frame(u16Frame, pszFrame); }
	static bool pre_skip_forward(Uint32 u16Frames) { return g_ldp->pre_skip_forward(u16Frames); }
	static bool pre_skip_backward(Uint32 u16Frames) { return g_ldp->pre_skip_backward(u16Frames); }
	static void pre_step_forward() { g_ldp->pre_step_forward(); }
	static void pre_step_backward() { g_ldp->pre_step_backward(); }
	
	// by RDG2010
	// Sometimes it's useful to know the status of the vldp.	
	// Lets give Singe the ability to query for this.
	static int get_vldp_status() { return g_ldp->get_status(); }
	static double get_singe_version();  // Returns version of the Singe engine	
	// Controls VLDP message displays on daphne_log.txt
	static void set_ldp_verbose(bool thisBol) { g_ldp->setVerbose(thisBol); }
	
	// These wrapper functions make the functions set_keyboard_mode and get_keyboard_mode 
	// available for the DLL/so library side of SINGE. 
	// Take a look at the comments in singe::init on singe.cpp for more info.
	// 	
	static void daphne_side_set_keyboard_mode(void *pInstance, int thisVal)
	{
		singe *pSingeInstance = (singe *) pInstance; 
		pSingeInstance->set_keyboard_mode(thisVal);
	}

	static int daphne_side_get_keyboard_mode(void *pInstance)
	{
		singe *pSingeInstance = (singe *) pInstance; 
		return pSingeInstance->get_keyboard_mode();
	}

	static void daphne_side_get_script_path(void *pInstance, char *thisVal)
	{
		singe *pSingeInstance = (singe *) pInstance; 
		pSingeInstance->get_script_path(thisVal);
	}

	static void daphne_side_mouse_enable(void *pInstance)
	{ 
		singe *pSingeInstance = (singe *) pInstance; 
		pSingeInstance->m_bMouseEnabled = true;
	}
	static void daphne_side_mouse_disable(void *pInstance)
	{
		singe *pSingeInstance = (singe *) pInstance; 
		pSingeInstance->m_bMouseEnabled = false;
	}
	static int daphne_side_mouse_get_how_many(void *pInstance)
	{
		singe *pSingeInstance = (singe *) pInstance; 
		return pSingeInstance->get_mice_detected();
	}
	static bool daphne_side_set_mouse_mode(int thisVal)
	{
		//singe *pSingeInstance = (singe *) pInstance; 
		return set_mouse_mode(thisVal);
	}

	static void daphne_side_pause_enable(void *pInstance)
	{ 
		singe *pSingeInstance = (singe *) pInstance; 
		pSingeInstance->m_bPauseKeyEnabled = true;
	}
	static void daphne_side_pause_disable(void *pInstance)
	{
		singe *pSingeInstance = (singe *) pInstance; 
		pSingeInstance->m_bPauseKeyEnabled = false;
		
	}

	void set_keyboard_mode(int); // Sets value of private member i_keyboard_mode
	int get_keyboard_mode();     // Retrieves the value of i_keyboard_mode	

	// By RDG2010
	// Copying expanded keyboard handling technique from Thayer's driver.
	void process_keydown(SDLKey, int [][2]);
	void process_keyup  (SDLKey, int [][2]);

	void set_game_name(char *); // Changes the value of private member m_strName;
	void change_caption(char *thisName); // Adds the name of the singe game to the window's title bar.
	static void daphne_side_set_caption(void *pInstance, char *thisName) // Sets value of m_strName
	{
		singe *pSingeInstance = (singe *) pInstance;
		pSingeInstance->change_caption(thisName);

	}	

	void set_script_path(const char *);	
	void get_script_path(char *);

private:
	// callback function for singe to pass error messages to us
	static void set_last_error(const char *cpszErrMsg);

	string m_strName;	// name of the game	
	string m_strGameScript;	// script name for the game
	string m_strScriptPath;	// script name for the game

	DLL_INSTANCE m_dll_instance;	// pointer to DLL we load (if we aren't statically linked)
	
	// by RDG2010
	
	int i_keyboard_mode; // Stores the keyboard access mode. Valid values are:
						 // KEYBD_NORMAL - Tells SINGE to use daphne.ini keys.
						 // KEYBD_FULL   - Tells SINGE to scan the full keyboard (a la Thayers).	
	static const int i_full_keybd_defs[]; // Array with discrete SDLKey values. Used in process_keyup/keydown.
	
};

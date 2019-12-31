/*
 * cmdline.cpp
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

// Parses our command line and sets our variables accordingly
// by Matt Ownby

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmdline.h"
#include "conout.h"
//#include "network.h"
#include "numstr.h"
#include "homedir.h"
#include "input.h"	// to disable joystick use
#include "../io/numstr.h"
#include "../video/video.h"
//#include "../video/led.h"
#include "../daphne.h"
//#include "../cpu/cpu-debug.h"	// for set_cpu_trace
//#include "../game/lair.h"
//#include "../game/cliff.h"
#include "../game/game.h"
//#include "../game/superd.h"
//#include "../game/thayers.h"
//#include "../game/speedtest.h"
//#include "../game/seektest.h"
//#include "../game/releasetest.h"
//#include "../game/cputest.h"
//#include "../game/multicputest.h"
//#include "../game/firefox.h"
//#include "../game/ffr.h"
//#include "../game/astron.h"
//#include "../game/esh.h"
//#include "../game/laireuro.h"
//#include "../game/badlands.h"
//#include "../game/starrider.h"
//#include "../game/bega.h"
//#include "../game/cobraconv.h"
//#include "../game/gpworld.h"
//#include "../game/interstellar.h"
//#include "../game/benchmark.h"
//#include "../game/lair2.h"
//#include "../game/mach3.h"
//#include "../game/lgp.h"
//#include "../game/timetrav.h"

#include "../game/singe.h"

//#include "../game/test_sb.h"
#include "../ldp-out/ldp.h"
//#include "../ldp-out/sony.h"
//#include "../ldp-out/pioneer.h"
//#include "../ldp-out/ld-v6000.h"
//#include "../ldp-out/hitachi.h"
//#include "../ldp-out/philips.h"
//#include "../ldp-out/ldp-combo.h"
#include "../ldp-out/ldp-vldp.h"
#include "../ldp-out/framemod.h"

#ifdef UNIX
#include <unistd.h>     // for unlink
#endif

int g_argc = 0;
char **g_argv = NULL;
int g_arg_index = 0;

// Win32 doesn't use strcasecmp, it uses stricmp (lame)
#ifdef WIN32
#define strcasecmp stricmp
#endif


// parses the command line looking for the -homedir switch, returns true if found and valid (or not found)
// (this must be done first because the the game and ldp classes may rely on the homedir being set to its proper place)
bool parse_homedir()
{
	bool result = true;
	char s[81] = { 0 };
	bool bHomeDirSet = false;	// whether set_homedir was called
	
	for (;;)
	{
		get_next_word(s, sizeof(s));
		// if there is nothing left for us to parse, break out of the while loop
		if (s[0] == 0)
		{
			break;
		}
	
		// if they are defining an alternate 'home' directory.
		// Primary used for OSX/linux to keep roms, framefiles and saverams in the user-writable space.
		else if (strcasecmp(s, "-homedir")==0)
		{
			//Ignore this one, already handled
			get_next_word(s, sizeof(s));
			if (s[0] == 0)
			{
				printline("Homedir switch used but no homedir specified!");
				result = false;
				break;
			}
			else
			{
				g_homedir.set_homedir(s);
				bHomeDirSet = true;
				printline("Setting alternate home dir:");
				printline(s);
				break;
			}
		}
	}			

	// if user didn't specify a homedir, then we have a default one.
	// (the reason this isn't in the homedir constructor is that we don't want to create
	//  default directories in a default location if the user specifies his/her own homedir)
	if (!bHomeDirSet)
	{

#ifdef UNIX
#ifndef MAC_OSX	// Zaph doesn't seem to like this behavior and he is maintaining the Mac build, so...
		// on unix, if the user doesn't specify a homedir, then we want to default to ~/.daphne
		//  to follow standard unix convention.
#ifndef GP2X	// gp2x has no homedir as far as I can tell, easier to put everything in current dir
		const char *homeenv = getenv("HOME");

		// this is always expected to be non-NULL
		if (homeenv != NULL)
		{
			string strHomeDir = homeenv;
			strHomeDir += "/.daphne";
			g_homedir.set_homedir(strHomeDir);
		}
		// else we couldn't set the homedir, so just leave it as default
#endif // not GP2X
#endif // not MAC_OSX
#else // WIN32
		g_homedir.set_homedir(".");	// for win32 set it to the current directory
		// "." is already the default,
		//  so the the main purpose for this is to ensure that the ram and
		//  framefile directories are created
		
#endif
	} // end if homedir was not set

	//Reset arg index and return
	g_arg_index = 1;
	return result;
}

// parses the game type from the command line and allocates g_game
// returns true if successful or false of failed
bool parse_game_type()
{
	bool result = true;
	int iLen,k;
	char s[81] = { 0 };

	// first thing we need to get from the command line is the game type
	get_next_word(s, sizeof(s));

	iLen = strlen(s);

	if (iLen < 6) 
	{
		result = false;
		printline("Command line error: first parameter is too short to be a script.");
		printline("Make sure your script file is the first parameter in your command line.");
	}
	
	if (result)
	{
		for(k=0;k<iLen;k++)
			s[k] = tolower(s[k]);

		string s1(s);
		string s2(s1.substr(s1.length()-6));	

		if (s2.compare(".singe") != 0)
		{
			result = false;
			printline("Command line error: No singe script found.");
			printline("Make sure your script file ends with a '.singe' extension");
		}
		else
		{
			get_framefile_from_name(s1.c_str());
			printline(s1.c_str());
			singe *cur_singe = dynamic_cast<singe *>(g_game);
			result = cur_singe->handle_cmdline_arg(s1.c_str());
			if (result) cur_singe->set_script_path(s1.c_str());
		}

	}

	return(result);
}

bool get_framefile_from_name(const char *s)
{
	bool result = true;
	string s1(s);
	string sFilename;

	size_t found;    
    found=s1.find_last_of("/\\");		
	sFilename = s1;
	found=sFilename.find_last_of(".");
	sFilename.erase(found+1,5);
	sFilename.append("txt");

	ldp_vldp *cur_ldp = dynamic_cast<ldp_vldp *>(g_ldp);	// see if the currently selected LDP is VLDP	
	// if it is a vldp, then this option has meaning and we can use it
	if (cur_ldp)
	{
		cur_ldp->set_framefile(sFilename.c_str());		
	}

	return(result);

}

// parses the LDP type from the command line and allocates g_ldp
// returns true if successful or false if failed
bool parse_ldp_type()
{
	bool result = true;
	char s[81] = { 0 };

	get_next_word(s, sizeof(s));

//	net_set_ldpname(s);	// report to server which ldp we are using

	if (strcasecmp(s, "fast_noldp")==0)
	{
		g_ldp = new fast_noldp();	// 'no seek-delay' version of noldp
	}	
	else if (strcasecmp(s, "noldp")==0)
	{
		g_ldp = new ldp();	// generic interface
	}	
	else if (strcasecmp(s, "vldp")==0)
	{
		g_ldp = new ldp_vldp();
	}
	else
	{
		printline("ERROR: Unknown laserdisc player type specified");
		result = false;
	}

	// safety check
	if (!g_ldp)
	{
		result = false;
	}

	return(result);
}

// parses command line
// Returns 1 on success or 0 on failure
bool parse_cmd_line(int argc, char **argv)
{
	bool result = true;
	char s[320] = { 0 };	// in case they pass in a huge directory as part of the framefile
	int i = 0;
	bool log_was_disabled = false;	// if we actually get "-nolog" while going through arguments

	//////////////////////////////////////////////////////////////////////////////////////

	g_argc = argc;
	g_argv = argv;
	g_arg_index = 1;	// skip name of executable from command line
	g_game = new singe();
	g_ldp = new ldp_vldp();

	// if game and ldp types are correct
	//if (parse_homedir() && parse_game_type() && parse_ldp_type())
	if (parse_homedir() && parse_game_type())
	{		 
  	  // while we have stuff left in the command line to parse
	  for (;;)
	  {
		get_next_word(s, sizeof(s));

		// if there is nothing left for us to parse, break out of the while loop
		if (s[0] == 0)
		{
			break;
		}
		
		// if they are defining an alternate 'home' directory.
		// Primary used for OSX/linux to keep roms, framefiles and saverams in the user-writable space.
		else if (strcasecmp(s, "-homedir")==0)
		{
			//Ignore this one, already handled
			get_next_word(s, sizeof(s));
		}

		// If they are defining an alternate 'data' directory, where all other files aside from the executable live.
		// Primary used for linux to separate binary file (eg. daphne.bin) and other datafiles . 
		else if (strcasecmp(s, "-datadir")==0) 
		{ 
			get_next_word(s, sizeof(s)); 
			change_dir(s);
		}

		// if user wants laserdisc player to blank video while searching (VLDP only)
		/*
		else if (strcasecmp(s, "-blank_searches")==0)
		{
			g_ldp->set_search_blanking(true);
		}
		*/
		// if user wants to laserdisc player to blank video while skipping (VLDP only)
		else if (strcasecmp(s, "-blank_skips")==0)
		{
			g_ldp->set_skip_blanking(true);
		}

		// if they are pointing to a framefile to be used by VLDP
		else if (strcasecmp(s, "-framefile")==0)
		{
			ldp_vldp *cur_ldp = dynamic_cast<ldp_vldp *>(g_ldp);	// see if the currently selected LDP is VLDP
			get_next_word(s, sizeof(s));
			// if it is a vldp, then this option has meaning and we can use it
			if (cur_ldp)
			{
				cur_ldp->set_framefile(s);
			}
			else
			{
				printline("You can only set a framefile using VLDP as your laserdisc player!");
				result = false;
			}
		}

		// specify an alternate singeinput.ini file (located in home or app directory)
		else if (strcasecmp(s, "-keymapfile")==0)
		{

			bool loadini = true;
			int iLen, k;

			get_next_word(s, sizeof(s));
			iLen = strlen(s);

			if (iLen < 5)
			{
				printline("WARNING : specified keymap file too short to be an '.ini' file. Ignoring.");
				loadini = false;
			}

			if (loadini)
			{

				for(k=0;k<iLen;k++)
					s[k] = tolower(s[k]);

				string s1(s);
				string s2(s1.substr(s1.length()-4));

				if (s2.compare(".ini") != 0)
				{
					printline("WARNING : specified keymap file does not end with a '.ini' extension. Ignoring.");
					loadini = false;
				}

				if (loadini)
				{
					string s3 = s1.substr(0, s1.size()-4);
					for (int i = 0; s3[i] != '\0'; i++)
					{
						if (!isalnum(s3[i]))
						{
							loadini = false;
						}
					}

					if (loadini)
					{
						set_inputini_file(s);

					} else {
						printline("WARNING : specified keymap file has invalid characters in filename. Ignoring.");
					}

				}
			}
		}
		
		// if they are defining an alternate soundtrack to be used by VLDP
		else if (strcasecmp(s, "-altaudio")==0)
		{
			ldp_vldp *cur_ldp = dynamic_cast<ldp_vldp *>(g_ldp);	// see if the currently selected LDP is VLDP
			get_next_word(s, sizeof(s));
			// if it is a vldp, then this option has meaning and we can use it
			if (cur_ldp)
			{
				cur_ldp->set_altaudio(s);
			}
			else
			{
				printline("You can only set an alternate soundtrack when using VLDP as your laserdisc player!");
				result = false;
			}
		}

		// The # of frames that we can seek per millisecond (to simulate seek delay)
		// Typical values for real laserdisc players are about 30.0 for 29.97fps discs
		//  and 20.0 for 23.976fps discs (dragon's lair and space ace)
		// FLOATING POINT VALUES ARE ALLOWED HERE
		// Minimum value is 12.0 (5 seconds for 60,000 frames), maximum value is
		//  600.0 (100 milliseconds for 60,000 frames).  If you want a value higher than
		//  the max, you should just use 0 (as fast as possible).
		// 0 = disabled (seek as fast as possible)
		//
		// Not for SINGE use
		/*
		else if (strcasecmp(s, "-seek_frames_per_ms")==0)
		{
			get_next_word(s, sizeof(s));
			double d = numstr::ToDouble(s);

			// WARNING : VLDP will timeout if it hasn't received a response after
			// a certain period of time (7.5 seconds at this moment) so you will
			// cause problems by reducing the minimum value much below 12.0
			if ((d > 12.0) && (d < 600.0)) g_ldp->set_seek_frames_per_ms(d);
			else printline("NOTE : Max seek delay disabled");
		}
		*/
		// the minimum amount of milliseconds to force a seek to take (artificial delay)
		// 0 = disabled
		// Not for SINGE use
		/*
		else if (strcasecmp(s, "-min_seek_delay")==0)
		{
			get_next_word(s, sizeof(s));
			i = atoi(s);

			if ((i > 0) && (i < 5000)) g_ldp->set_min_seek_delay((unsigned int) i);
			else printline("NOTE : Min seek delay disabled");
		}
		*/
		// if the user wants the searching to be the old blocking style instead of non-blocking
		else if (strcasecmp(s, "-blocking")==0)
		{
			g_ldp->set_use_nonblocking_searching(false);
		}

		// to disable any existing joysticks that may be plugged in that may interfere with input
		else if ((strcasecmp(s, "-nojoystick")==0) || (strcasecmp(s, "-nojoy")==0))
		{
			set_use_joystick(false);
		}
		else if ((strcasecmp(s,"-nomouse")==0) || (strcasecmp(s,"-disablemouse")==0))
		{
			g_game->set_mouse_enabled(false);
			printline("Not tracking mouse input for this game session.");
		}
		
		// if they are paranoid and don't want data sent to the server
		// (don't be paranoid, read the source code, nothing bad is going on)
		/*
		else if (strcasecmp(s, "-noserversend")==0)
		{
			//net_no_server_send();
		}
		*/
		else if ((strcasecmp(s, "-nosound")==0) || (strcasecmp(s, "-mutesound")==0))
		{
			set_sound_enabled_status(false);
			printline("Disabling sound...");
		}
		else if (strcasecmp(s, "-sound_buffer")==0)
		{
			get_next_word(s, sizeof(s));
			Uint16 sbsize = (Uint16) atoi(s);
			set_soundbuf_size(sbsize);
			sprintf(s, "Setting sound buffer size to %d", sbsize);
			printline(s);
		}
		else if (strcasecmp(s, "-volume_vldp")==0)
		{
			get_next_word(s, sizeof(s));
			unsigned int uVolume = atoi(s);
			set_soundchip_vldp_volume(uVolume);
		}
		else if (strcasecmp(s, "-volume_nonvldp")==0)
		{
			get_next_word(s, sizeof(s));
			unsigned int uVolume = atoi(s);
			set_soundchip_nonvldp_volume(uVolume);
		}
		else if (strcasecmp(s, "-nocrc")==0)
		{
			g_game->disable_crc();
			printline("Disabling ROM CRC check...");
		}
		// Not for SINGE use
		/*
		else if (strcasecmp(s, "-scoreboard")==0)
		{
			set_scoreboard(1);
			printline("Enabling external scoreboard...");
		}
		else if (strcasecmp(s, "-scoreport")==0)
		{
			get_next_word(s, sizeof(s));
			set_scoreboard_port((unsigned int)numstr::ToUint32(s, 16));
			sprintf(s, "Setting scoreboard port to %d", i);
			printline(s);
		}
		else if (strcasecmp(s, "-port")==0)
		{
			get_next_word(s, sizeof(s));
			i = atoi(s);
			set_serial_port((unsigned char) i);
			sprintf(s, "Setting serial port to %d", i);
			printline(s);
		}
		else if (strcasecmp(s, "-baud")==0)
		{
			get_next_word(s, sizeof(s));
			i = atoi(s);
			set_baud_rate(i);
			sprintf(s, "Setting baud rate to %d", i);
			printline(s);
		}

		// used to modify the dip switch settings of the game in question
		else if (strcasecmp(s, "-bank")==0)
		{
			get_next_word(s, sizeof(s));
			i = s[0] - '0';	// bank to modify.  We convert to a number this way to catch more errors
			
			get_next_word(s, sizeof(s));
			unsigned char value = (unsigned char) (strtol(s, NULL, 2));	// value to be set is in base 2 (binary)

			result = g_game->set_bank((unsigned char) i, (unsigned char) value);
		}

		else if (strcasecmp(s, "-latency")==0)
		{
			get_next_word(s, sizeof(s));
			i = atoi(s);

			// safety check
			if (i >= 0)
			{
				g_ldp->set_search_latency(i);
				sprintf(s, "Setting Search Latency to %d milliseconds", i);
				printline(s);
			}
			else
			{
				printline("Search Latency value cannot be negative!");
				result = false;
			}
		}
		else if (strcasecmp(s, "-cheat")==0)
		{
			g_game->enable_cheat();	// enable any cheat we have available :)
		}

		// enable keyboard LEDs for use with games such as Space Ace
		else if (strcasecmp(s, "-enable_leds")==0)
		{
			enable_leds(true);
		}
		// hacks roms for certain games such as DL, SA, and CH to bypass slow boot process
		else if (strcasecmp(s, "-fastboot")==0)
		{
			g_game->set_fastboot(true);
		}
		*/
		// stretch video vertically by x amount (a value of 24 removes letterboxing effect in Cliffhanger)
		else if (strcasecmp(s, "-vertical_stretch")==0)
		{
			ldp_vldp *the_ldp = dynamic_cast<ldp_vldp *>(g_ldp);

			get_next_word(s, sizeof(s));
			i = atoi(s);
			if (i > 50) {
				i=50;
				printline("WARNING: vertical stretch restricted to 50 max.");
			}
			if (i < 0) {
				i=0;
				printline("WARNING: vertical stretch cannot be negative. zeroed.");
			}
			if (the_ldp != NULL)
			{
				the_ldp->set_vertical_stretch(i);
			}
			else
			{
				printline("Vertical stretch only works with VLDP.");
				result = false;
			}
		}

		// don't force 4:3 aspect ratio regardless of window size
		else if (strcasecmp(s, "-ignore_aspect_ratio")==0)
		{
			set_force_aspect_ratio(false);
		}

		// run daphne in fullscreen mode
		else if (strcasecmp(s, "-fullscreen")==0)
		{
			set_fullscreen(true);
		}

		// disable log file
		else if (strcasecmp(s, "-nolog")==0)
		{
			log_was_disabled = true;
		}

		// disable YUV hardware acceleration
		else if (strcasecmp(s, "-nohwaccel")==0)
		{
			set_yuv_hwaccel(false);
		}

		// by RDG2010
		// Preliminary light gun support.
		// Creates a borderless, no title bar window the size of the desktop
		// and positions it accordingly so that it covers the whole screen.
		else if (strcasecmp(s, "-fullscreen_window")==0)
		{			
			set_fakefullscreen(true);
					
		}

		// by RDG2010
		// Scales video image to something smaller than the window size.
		// Helpful for users with overscan issues on arcade monitors or CRT TVs.
		// Valid values are 50-100, where 50 means half the size of the 
		// window, 100 means the same size.
		else if (strcasecmp(s, "-scalefactor")==0)
		{
			get_next_word(s, sizeof(s));
			i = atoi(s);
			sprintf(s, "Scaling image by %d%%", i);
			printline(s);
			set_scalefactor((Uint16)i);
		}

#ifdef USE_OPENGL
		else if (strcasecmp(s, "-opengl")==0)
		{
			set_use_opengl(true);			
			
		}
#endif // USE_OPENGL
		
		else if (strcasecmp(s, "-preset")==0)
		{
			get_next_word(s, sizeof(s));
			i = atoi(s);
			g_game->set_preset(i);
		}
		else if (strcasecmp(s, "-rotate")==0)
		{
			get_next_word(s, sizeof(s));
			float f = (float) numstr::ToDouble(s);
			set_rotate_degrees(f);
		}

		// continuously updates SRAM (after every seek)  
		// ensures sram is saved even if Daphne is terminated improperly
		// Not for SINGE use
		/*
		else if (strcasecmp(s, "-sram_continuous_update")==0)
		{
			g_ldp->set_sram_continuous_update(true);
		}

		else if (strcasecmp(s, "-version")==0)
		{
			get_next_word(s, sizeof(s));
			i = atoi(s);
			g_game->set_version(i);
		}
		*/
		else if (strcasecmp(s, "-x")==0)
		{
			get_next_word(s, sizeof(s));
			i = atoi(s);
			set_video_width((Uint16)i);
			sprintf(s, "Setting screen width to %d", i);
			printline(s);
		}
		else if (strcasecmp(s, "-y")==0)
		{
			get_next_word(s, sizeof(s));
			i = atoi(s);
			set_video_height((Uint16)i);
			sprintf(s, "Setting screen height to %d", i);
			printline(s);
		}
		// Not for SINGE use
		/*
		else if (strcasecmp(s, "-trace")==0)
		{
#ifdef CPU_DEBUG
//			printline("CPU tracing enabled");
//			set_cpu_trace(1);
#else
			printline("DAPHNE needs to be compiled in debug mode for this to work");
			result = false;
#endif
		}
		*/
		// added by JFA for -idleexit
		else if (strcasecmp(s, "-idleexit")==0)
		{
			get_next_word(s, sizeof(s));
			i = atoi(s);
			set_idleexit(i*1000);
			sprintf(s, "Setting idleexit to %d ", i);
			printline(s);
		}
		// end edit

		// added by JFA for -startsilent
		else if (strcasecmp(s, "-startsilent")==0)
		{
			set_startsilent(1);
			printline("Starting silent...");
		}
		// end edit

		// if they are requesting to stop the laserdisc when the program exits
		else if (strcasecmp(s, "-stoponquit")==0)
		{
			g_ldp->set_stop_on_quit(true);
		}                
        else if (strcasecmp(s, "-prefer_samples")==0)
        {
			// If a game doesn't support "prefer samples" then it is not an error.
			// That's why it is _prefer_ samples instead of _require_ samples :)
			g_game->set_prefer_samples(true);
        }

		else if  (strcasecmp(s, "-noissues")==0)
		{
			g_game->set_issues(NULL);
		}

		// Scale the game overlay graphics to the virtual screen dimension
        // this is needed when Daphne is used for overlaying game graphics over the real 
        // laserdisc movie (using a video genlock), and the screen dimension is different
        // from the dimensions of the game.
		else if (strcasecmp(s, "-fullscale")==0)
		{
			ldp_vldp *cur_ldp = dynamic_cast<ldp_vldp *>(g_ldp);	// see if the currently selected LDP is VLDP
			// if it is a vldp, then this option is not supported
			if (cur_ldp)
			{
				printline("Full Scale mode only works with NOLDP.");
				result = false;
			}
			else
			{
				g_game->SetFullScale(true);
			}
		}

		// check for any game-specific arguments ...
		else if (g_game->handle_cmdline_arg(s))
		{
			// don't do anything in here, it has already been handled by the game class ...
		}
		// check for any ldp-specific arguments ...
		else if (g_ldp->handle_cmdline_arg(s))
		{
			// don't do anything in here, it has already been handled by the ldp class ...
		}
		else
		{
			printline("Unknown command line parameter or parameter value:");
			printline(s);
			result = false;
		}
	  } // end for
	} // end if we know our game type
	
	// if game or ldp was unknown
	else
	{
		result = false;
	}

	// if we didn't receive "-nolog" while parsing, then it's ok to enable the log file now.
	if (!log_was_disabled)
	{
		// first we need to delete the old logfile so we can start fresh
		// (this is the best place to do it since up until this point we don't know where our homedir is)
		string logfile = g_homedir.get_homedir();
		logfile += "/";
		logfile += LOGNAME;
		unlink(logfile.c_str());	// delete logfile if possible, because we're starting over
	
		set_log_enabled(true);
	}

	return(result);
}


// copies a single word from the command line into result (a word is any group of characters separate on both sides by whitespace)
// if there are no more words to be copied, result will contain a null string
void get_next_word(char *result, int result_size)
{
	// make sure we still have command line left to parse
	if (g_arg_index < g_argc)
	{
		strncpy(result, g_argv[g_arg_index], result_size);
		result[result_size-1] = 0;	// terminate end of string just in case we hit the limit
		g_arg_index++;

		
	}

	// if we have no command line left to parse ...
	else
	{
		result[0] = 0;	// return a null string
	}
}

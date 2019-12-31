=========================================
Singe Changelog
=========================================

v1.18 - Mice Update: Added support for multiple mice. Singe can now track 
		up to ^_^ mice (but why would you want that many?). Keep in mind there 
		is a performance hit when tracking multiple cursors.
		For the programming oriented, function OnMouseMoved gains 
		an additional parameter, like this:
		
		OnMouseMoved(intX, intY, relX, relY, mouseID)
		
		Singe will return an integer value called mouseID. Use this
		to keep track of your mice. 		
		
		
	-	Added parameter "-nomouse". Disables mouse from the command line
		
	-	Alpha blending issue fixed.
	
	-	Fixed bug crash related to sounds still playing during shutdown.

	- New Singe Functions

		// mouseSetMode() Enables/Disable multiple mice.
		// e.g. lua code,
		// 
		// 	mouseSetMode(MANY_MOUSE)
		//      mouseSetMode(SINGLE_MOUSE)
		//
		// Where those constant values are as follows:
		//
		// 	SINGLE_MOUSE = 100
		//	MANY_MOUSE   = 200
		//
		// Note: SINGLE_MOUSE is the default mode.
		// Only call this function to read multiple mice.

		// mouseDisable() Disable mouse from within lua code.
		// e.g. lua code, 
		//       
		//      mouseDisable()
		//
		// 

		// mouseEnable() Enables mouse from within lua code.
		// e.g. lua code, 
		//       
		//      mouseEnable()
		//
		//

		// mouseHowMany() Returns the number of mice attached to
		// the computer. For use with multiple mice situations only.
		// NOTES: If "-single_mouse" command line switch
		// was used to launch Singe, then return value will be -1.
		//
		// e.g. lua code, 
		//       
		//      local intMiceDetected = mouseHowMany()
		//
		//

		// discPauseAtFrame() - Skips video to a given frame and stays there (pauses playback).
		// e.g. lua code,
		//
		//	discPauseAtFrame(3494)

		// singeDisablePauseKey() - Disables the pause key (usually letter "P" on the keyboard.)
		// e.g. lua code,
		//
		//	singeDisablePauseKey()
		//
		// Use this when you don't want players pausing your game.
		// Specially useful for situations where letting the player pause the game
		// may potentially mess up your singe program, a pause menu, a time sensitive range, etc.
		//
		
v1.17 - Large File Support - Singe can now read mpegs over 2GBs in size.

		+ Matt's YUV mod applied to version 1.2.14 of the SDL library.

v1.16 - Sound support update: New functions for more robust sound handling.
		
		soundPause()
		soundResume()
		soundIsPlaying()
		soundGetVolume/soundSetVolume()
		soundFullStop()
	
		// soundPause() Pauses a sample currently playing
		// in the sound queue.		
		// User must feed the sound handle on the lua side.
		// e.g. lua code, 
		//       
		//      thisHandle = soundPlay(thisSound)
		//      soundPause(thisHandle)
		//
		// Function returns an integer value ranging from 0 to 63.

		// soundResume() Instructs Daphne to resume playback 
		// of a previously paused sound.
		// User must feed the sound handle on the lua side.
		// e.g. lua code, 
		//
		//		thisHandle = soundPlay(mySound)
		//		soundPause(thisHandle)
		//		soundResume(thisHandle)
		//
		// Function returns true if sample was unpaused, false otherwise.

		// soundIsPlaying() Checks to see if a certain sound has finished playing.
		// User must feed the sound handle on the lua side.
		// e.g. lua code, 
		//
		//		thisHandle = soundPlay(mySound)
		//		if (soundIsPlaying(thisSound)) then do something ... end
		//
		// Function returns true if sample is still playing, false otherwise.
		//

		// soundGetVolume/soundSetVolume() Allows manipulation of sample volume.
		// Valid values range from 0 to 63
		// e.g. lua code, 
		//
		//       soundSetVolume(32)
		//
		// Function returns nothing.
		//

		// soundFullStop() Clears the audio queue of any samples actively playing.
		// No parameters needed. Function returns nothing.
		// e.g. lua code, 
		//
		//		soundFullStop()
		//	

v1.15 - As of version 1.15 Singe is officially a fork of Daphne. It only runs singe games.

		+ Command Line has been greatly simplified. To run a Singe game type:
		
			singe.exe c:\path.to.my.script\somegame.singe
			
			Singe will assume the framefile will be "somegame.txt". If your framefile has 
			a different name, then use the -framefile parameter as you would in Daphne.

		+ Additional functions:
		
		singeGetScriptPath()		
		singeSetGameName()
		
		// singeGetScriptPath() Returns the path to the singe script.
		// e.g. lua code, 
		//			
		//		sGameDirectory = singeGetScriptPath()
		//		
			
		// singeSetGameName() Adds the name of the singe game to the window's title bar.
		// Valid value is a string no longer than 25 characters.
		// e.g. lua code, 
		//
		//       singeSetGameName("My FMV game")
		//
		// Function returns nothing.


#ifndef SINGE_INTERFACE_H
#define SINGE_INTERFACE_H

// increase this number every time you change something in this file!!!
#define SINGE_INTERFACE_API_VERSION 7
#define MAX_TITLE_LENGTH 40 //27

// info provided to Daphne to Singe
struct singe_in_info
{
	// the API version (THIS MUST COME FIRST!)
	unsigned int uVersion;

	// FUNCTIONS:

	// shuts down daphne
	void (*set_quitflag)();

	// print a line to the debug console, and the log file (and to stdout on some platforms)
	void (*printline)(const char *);
	
	// notifies daphne of the last error that singe got (so daphne can display it)
	void (*set_last_error)(const char *);

	// From video/video.h
	Uint16 (*get_video_width)();
	Uint16 (*get_video_height)();
	void (*draw_string)(const char*, int, int, SDL_Surface*);
	
	// From sound/samples.h
	int (*samples_play_sample)(Uint8 *pu8Buf, unsigned int uLength, unsigned int uChannels, int iSlot, void (*finishedCallback)(Uint8 *pu8Buf, unsigned int uSlot));
	bool (*samples_set_state)  (unsigned int, bool); // by rdg -- give Singe the ability to pause/resume samples
	bool (*samples_is_sample_playing) (unsigned int); // Find out is a sample is still in the stream queue
	bool (*samples_end_early) (unsigned int); // Terminate a sample from playing
	void (*set_soundchip_nonvldp_volume)(unsigned int); // Control sound sample volume 
	void (*samples_flush_queue)();
	unsigned int (*get_soundchip_nonvldp_volume)();

	// Laserdisc Control Functions
	void (*enable_audio1)();
	void (*enable_audio2)();
	void (*disable_audio1)();
	void (*disable_audio2)();
	void (*request_screenshot)();
	void (*set_search_blanking)(bool enabled);
	void (*set_skip_blanking)(bool enabled);
	bool (*pre_change_speed)(unsigned int uNumerator, unsigned int uDenominator);
	unsigned int (*get_current_frame)();
	void (*pre_play)();
	void (*pre_pause)();
	void (*pre_stop)();
	bool (*pre_search)(const char *, bool block_until_search_finished);
	void (*framenum_to_frame)(Uint32, char *);
	bool (*pre_skip_forward)(Uint32);
	bool (*pre_skip_backward)(Uint32);
	void (*pre_step_forward)();
	void (*pre_step_backward)();

	//by RDG2010
	void *pSingeInstance;
	void (*dll_side_set_keyboard_mode)(void *, int);
	int  (*dll_side_get_keyboard_mode)(void *);	
	int  (*get_vldp_status)();
	double (*get_singe_version)(void);
	void (*set_ldp_verbose)(bool);	
	void (*dll_side_set_caption)(void *, char *);
	void (*dll_side_get_script_path)(void *, char *);
	void (*dll_side_mouse_enable)(void *);
	void (*dll_side_mouse_disable)(void *);
	bool (*dll_side_set_mouse_mode)(int);
	int (*dll_side_mouse_get_how_many)(void *);
	void (*dll_side_pause_enable)(void *);
	void (*dll_side_pause_disable)(void *);

	// VARIABLES:
	
	// VLDP Interface
	struct vldp_in_info        *g_local_info;
	const struct vldp_out_info *g_vldp_info;
	//SDL_Surface *g_screen_blitter;
	
};

// info provided from Singe to Daphne
struct singe_out_info
{
	// the API version (THIS MUST COME FIRST!)
	unsigned int uVersion;

	// FUNCTIONS:
	void (*sep_call_lua)(const char *func, const char *sig, ...);
	void (*sep_do_blit)(SDL_Surface *srfDest);
	void (*sep_do_mouse_move)(Uint16 x, Uint16 y, Sint16 xrel, Sint16 yrel, Uint16 mouseID);
	void (*sep_error)(const char *fmt, ...);
	void (*sep_print)(const char *fmt, ...);
	void (*sep_set_static_pointers)(double *m_disc_fps, unsigned int *m_uDiscFPKS);
	void (*sep_set_surface)(int width, int height);
	void (*sep_shutdown)(void);
	void (*sep_startup)(const char *script);
	
	////////////////////////////////////////////////////////////
};

// if you want to build singe as a DLL, then define SINGE_DLL in your DLL project's preprocessor defs
#ifdef SINGE_DLL
#define SINGE_EXPORT __declspec(dllexport)
#else
// otherwise SINGE_EXPORT is blank
#define SINGE_EXPORT
#endif

extern "C"
{
SINGE_EXPORT const struct singe_out_info *singeproxy_init(const struct singe_in_info *in_info);
}

#endif // SINGE_INTERFACE_H

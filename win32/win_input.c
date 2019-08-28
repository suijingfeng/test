/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// win_input.c -- win32 mouse and joystick code
// 02/21/97 JCB Added extended DirectInput code to support external controllers.

#include "win_public.h"
#include "../client/client.h"
#include "win_input.h"
#include "win_event.h"


extern WinVars_t g_wv;

typedef struct {
	int	oldButtonState;
	int	window_center_x;
	int	window_center_y;

	qboolean	mouseActive;
	qboolean	mouseInitialized;
	qboolean	isAppActive;
} WinMouseVars_t;

static WinMouseVars_t s_wmv;

cvar_t	*in_mouse;


/*
============================================================

WIN32 MOUSE CONTROL

============================================================
*/


static void IN_InitWin32Mouse( void ) 
{
}


static void IN_ShutdownWin32Mouse( void )
{
}


static void IN_ActivateWin32Mouse( void )
{

	int width = GetSystemMetrics (SM_CXSCREEN);
	int height = GetSystemMetrics (SM_CYSCREEN);

	RECT window_rect;
	GetWindowRect ( g_wv.hWnd, &window_rect);
	if (window_rect.left < 0)
		window_rect.left = 0;
	if (window_rect.top < 0)
		window_rect.top = 0;
	if (window_rect.right >= width)
		window_rect.right = width-1;
	if (window_rect.bottom >= height-1)
		window_rect.bottom = height-1;

	s_wmv.window_center_x = (window_rect.right + window_rect.left)/2;
	s_wmv.window_center_y = (window_rect.top + window_rect.bottom)/2;

	SetCursorPos (s_wmv.window_center_x, s_wmv.window_center_y);

	SetCapture ( g_wv.hWnd );
	ClipCursor (&window_rect);
	while (ShowCursor (FALSE) >= 0)
		;
}


static void IN_DeactivateWin32Mouse( void ) 
{
	ClipCursor (NULL);
	ReleaseCapture ();
	while (ShowCursor (TRUE) < 0)
		;
}


static void IN_Win32Mouse( int * const mx, int * const my )
{
	POINT current_pos;

	// find mouse movement
	GetCursorPos (&current_pos);

	// force the mouse to the center, so there's room to move
	SetCursorPos (s_wmv.window_center_x, s_wmv.window_center_y);

	*mx = current_pos.x - s_wmv.window_center_x;
	*my = current_pos.y - s_wmv.window_center_y;
}


/*
===========
IN_ActivateMouse

Called when the window gains focus or changes in some way
===========
*/
static void IN_ActivateMouse( void ) 
{
	if (!s_wmv.mouseInitialized )
	{
		return;
	}
	if ( !in_mouse->integer ) 
	{
		s_wmv.mouseActive = qfalse;
		return;
	}
	if ( s_wmv.mouseActive ) 
	{
		return;
	}

	s_wmv.mouseActive = qtrue;

	IN_ActivateWin32Mouse();
}


/*
===========
IN_DeactivateMouse

Called when the window loses focus
===========
*/
static void IN_DeactivateMouse( void )
{
	if (!s_wmv.mouseInitialized ) {
		return;
	}
	if (!s_wmv.mouseActive ) {
		return;
	}
	s_wmv.mouseActive = qfalse;

	IN_DeactivateWin32Mouse();
}


static void IN_StartupMouse(void)
{
	s_wmv.mouseInitialized = qfalse;

	if (in_mouse->integer == 0) {
		Com_Printf("Mouse control not active.\n");
		return;
	}

	s_wmv.mouseInitialized = qtrue;
	IN_InitWin32Mouse();
	Com_Printf("Win32 mouse input initialized.\n");
}


// ====================================================
// 
// ====================================================

void IN_Init( void )
{
    in_mouse = Cvar_Get ("in_mouse", "1", CVAR_ARCHIVE|CVAR_LATCH);

	Com_Printf("\n------- Input Initialization -------\n");
	IN_StartupMouse();
	Com_Printf("------------------------------------\n");

	in_mouse->modified = qfalse;
}

void IN_Shutdown(void)
{
	IN_DeactivateMouse();
}


/*
===========
IN_Activate

Called when the main window gains or loses focus.
The window may have been destroyed and recreated
between a deactivate and an activate.
===========
*/
void IN_Activate (qboolean active)
{
	s_wmv.isAppActive = active;

	if ( !active )
	{
		IN_DeactivateMouse();
	}
}


/*
==================
IN_Frame

Called every frame, even if not generating commands
==================
*/
void IN_Frame (void)
{
	if ( !s_wmv.mouseInitialized ) {
		return;
	}

	if ( Key_GetCatcher() & KEYCATCH_CONSOLE )
	{
		// temporarily deactivate if not in the game and
		// running on the desktop
		// voodoo always counts as full screen
		if (g_wv.isFullScreen == 0 )
		{
			IN_DeactivateMouse ();
			return;
		}
	}

	if ( !s_wmv.isAppActive)
	{
		IN_DeactivateMouse ();
		return;
	}

	IN_ActivateMouse();

	// post events to the system que
	// IN_MouseMove();

	int	mx, my;

	IN_Win32Mouse(&mx, &my);

	if (mx || my)
	{
		Sys_QueEvent(0, SE_MOUSE, mx, my, 0, NULL);
	}
}



void IN_MouseEvent(int mstate)
{
	if (!s_wmv.mouseInitialized)
		return;

	// perform button actions
	for (i = 0; i < 3; ++i)
	{
		if ((mstate & (1 << i)) &&
			!(s_wmv.oldButtonState & (1 << i)))
		{
			Sys_QueEvent(g_wv.sysMsgTime, SE_KEY, K_MOUSE1 + i, qtrue, 0, NULL);
		}

		if (!(mstate & (1 << i)) &&
			(s_wmv.oldButtonState & (1 << i)))
		{
			Sys_QueEvent(g_wv.sysMsgTime, SE_KEY, K_MOUSE1 + i, qfalse, 0, NULL);
		}
	}

	s_wmv.oldButtonState = mstate;
}

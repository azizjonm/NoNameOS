/*
 *     AAA    M M    OOO    SSSS
 *    A   A  M M M  O   O  S 
 *    AAAAA  M M M  O   O   SSS
 *    A   A  M   M  O   O      S
 *    A   A  M   M   OOO   SSSS 
 *
 *    Author:  Stephen Fewer
 *    Contact: steve [AT] harmonysecurity [DOT] com
 *    Web:     http://amos.harmonysecurity.com/
 *    License: GNU General Public License (GPL)
 */

#include <kernel/io/dev/keyboard.h>
#include <kernel/interrupt.h>
#include <kernel/kernel.h>
#include <kernel/mm/mm.h>
#include <kernel/io/dev/console.h>
#include <kernel/io/io.h>
#include <kernel/fs/vfs.h>

struct VFS_HANDLE * keyboard_output;

BYTE keyboard_shift;
BYTE keyboard_caps;
BYTE keyboard_num;
BYTE keyboard_scroll;

BYTE keyboard_upperMap[256] =
{
      0,  27, '!', '"',  '�',  '$', '%', '^',  '&', '*', '(', ')', '_', '+', '\b', '\t', 'Q', 'W',  'E', 'R',
	'T', 'Y', 'U', 'I',  'O',  'P', '{', '}', '\n',   0, 'A', 'S', 'D', 'F',  'G',  'H', 'J', 'K',  'L', ':',
	 '�', '~',  0, '|',  'Z',  'X', 'C', 'V',  'B', 'N', 'M', '<', '>', '?',   0,   '*',   0, ' ',    0,   0,
      0,   0,   0,   0,   0,     0,   0,   0,    0,   0,   0,   0,   0,   0, '-',     0,   0,   0,  '+',   0,
	  0,   0,   0,   0,   0,     0,   0,   0,    0,   0,   0,   0,   0,   0,   0,     0,   0,   0,    0,   0,
	  0,   0,   0,   0,   0,     0,   0,   0,    0,   0,   0,   0,   0,   0,   0,     0,   0,   0,    0,   0,
      0,   0,   0,   0,   0,     0,   0,   0,    0,   0,   0,   0,   0,   0,   0,     0,   0,   0,    0,   0,
      0,   0,   0,   0,   0,     0,   0,   0,    0,   0,   0,   0,   0,   0,   0,     0,   0,   0,    0,   0,
      0,   0,   0,   0,   0,     0,   0,   0,    0,   0,   0,   0,   0,   0,   0,     0,   0,   0,    0,   0,
      0,   0,   0,   0,   0,     0,   0,   0,    0,   0,   0,   0,   0,   0,   0,     0,   0,   0,    0,   0,
      0,   0,   0,   0,   0,     0,   0,   0,    0,   0,   0,   0,   0,   0,   0,     0,   0,   0,    0,   0,
      0,   0,   0,   0,   0,     0,   0,   0,    0,   0,   0,   0,   0,   0,   0,     0,   0,   0,    0,   0,
      0,   0,   0,   0,   0,     0,   0,   0,    0,   0,   0,   0,   0,   0,   0,     0
};

BYTE keyboard_lowerMap[256] =
{
      0,  27, '1', '2',  '3',  '4', '5', '6',  '7', '8', '9', '0', '-', '=', '\b', '\t', 'q', 'w',  'e', 'r',
   	't', 'y', 'u', 'i',  'o',  'p', '[', ']', '\n',   0, 'a', 's', 'd', 'f',  'g',  'h', 'j', 'k',  'l', ';',
   '\'', '`',  0, '\\',  'z',  'x', 'c', 'v',  'b', 'n', 'm', ',', '.', '/',   0,   '*',   0, ' ',    0,   0,
      0,   0,   0,   0,   0,     0,   0,   0,    0,   0,   0,   0,   0,   0, '-',     0,   0,   0,  '+',   0,
	  0,   0,   0,   0,   0,     0,   0,   0,    0,   0,   0,   0,   0,   0,   0,     0,   0,   0,    0,   0,
	  0,   0,   0,   0,   0,     0,   0,   0,    0,   0,   0,   0,   0,   0,   0,     0,   0,   0,    0,   0,
      0,   0,   0,   0,   0,     0,   0,   0,    0,   0,   0,   0,   0,   0,   0,     0,   0,   0,    0,   0,
      0,   0,   0,   0,   0,     0,   0,   0,    0,   0,   0,   0,   0,   0,   0,     0,   0,   0,    0,   0,
      0,   0,   0,   0,   0,     0,   0,   0,    0,   0,   0,   0,   0,   0,   0,     0,   0,   0,    0,   0,
      0,   0,   0,   0,   0,     0,   0,   0,    0,   0,   0,   0,   0,   0,   0,     0,   0,   0,    0,   0,
      0,   0,   0,   0,   0,     0,   0,   0,    0,   0,   0,   0,   0,   0,   0,     0,   0,   0,    0,   0,
      0,   0,   0,   0,   0,     0,   0,   0,    0,   0,   0,   0,   0,   0,   0,     0,   0,   0,    0,   0,
      0,   0,   0,   0,   0,     0,   0,   0,    0,   0,   0,   0,   0,   0,   0,     0
};

struct IO_HANDLE * keyboard_open( struct IO_HANDLE * handle, char * filename )
{
	return handle;
}

int keyboard_close( struct IO_HANDLE * handle )
{
	return SUCCESS;
}
/*
int keyboard_wait( void )
{
	while( TRUE )
	{
		if( inportb( KEYBOARD_DATAREG ) == KEYBOARD_ACK )
			break;
		process_yield();
	}
	return SUCCESS;
}

int keyboard_setLED( BYTE led )
{
	outportb( KEYBOARD_DATAREG, 0xED );
	
	keyboard_wait();
	
	outportb( KEYBOARD_DATAREG, led );
}
*/
struct PROCESS_INFO * keyboard_handler( struct PROCESS_INFO * process )
{
	BYTE scancode;
	
	scancode = inportb( KEYBOARD_DATAREG );
	
	switch( scancode )
	{
		case KEYBORAD_KEY_LSHIFT:
			keyboard_shift = TRUE;
			break;
		case KEYBORAD_KEY_RSHIFT:
			keyboard_shift = TRUE;
			break;
		case KEYBORAD_KEY_CAPS:
			if( keyboard_caps )
				keyboard_caps = FALSE;
			else
				keyboard_caps = TRUE;
			//keyboard_setLED( KEYBOARD_LED_CAPS );
			break;
		case KEYBORAD_KEY_NUM:
			if( keyboard_num )
				keyboard_num = FALSE;
			else
				keyboard_num = TRUE;
			//keyboard_setLED( KEYBOARD_LED_NUM );
			break;
		case KEYBORAD_KEY_SCROLL:
			if( keyboard_scroll )
				keyboard_scroll = FALSE;
			else
				keyboard_scroll = TRUE;
			//keyboard_setLED( KEYBOARD_LED_SCROLL );
			break;
		case KEYBORAD_KEY_F1:
			vfs_control( keyboard_output, CONSOLE_SETACTIVE, CONSOLE_1 );
			break;
		case KEYBORAD_KEY_F2:
			vfs_control( keyboard_output, CONSOLE_SETACTIVE, CONSOLE_2 );
			break;
		case KEYBORAD_KEY_F3:
			vfs_control( keyboard_output, CONSOLE_SETACTIVE, CONSOLE_3 );
			break;
		case KEYBORAD_KEY_F4:
			vfs_control( keyboard_output, CONSOLE_SETACTIVE, CONSOLE_4 );
			break;
		case 0xAA:
			keyboard_shift = FALSE;
			break;
		default:
			if( !(scancode & 0x80) )
			{
				BYTE b;
			
				b = (keyboard_shift||keyboard_caps ? keyboard_upperMap : keyboard_lowerMap )[scancode];
					
				vfs_control( keyboard_output, CONSOLE_SENDCHAR, b );
			}
			break;
	}

	return process;
}

int keyboard_init( void )
{
	struct IO_CALLTABLE * calltable;
	calltable = (struct IO_CALLTABLE *)mm_malloc( sizeof(struct IO_CALLTABLE) );
	calltable->open    = keyboard_open;
	calltable->close   = keyboard_close;
	calltable->read    = NULL;
	calltable->write   = NULL;
	calltable->seek    = NULL;
	calltable->control = NULL;
	
	keyboard_shift  = FALSE;
	keyboard_caps   = FALSE;
	keyboard_num    = FALSE;
	keyboard_scroll = FALSE;
	//keyboard_setLED( 0 );
	
	keyboard_output = vfs_open( "/device/console0", VFS_MODE_WRITE );
	if( keyboard_output == NULL )
		return FAIL;
	// setup the keyboard handler
	interrupt_enable( IRQ1, keyboard_handler, SUPERVISOR );
	// add the keyboard device
	io_add( "keyboard1", calltable, IO_CHAR );
	// return success
	return SUCCESS;
}

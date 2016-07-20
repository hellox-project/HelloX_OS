//***********************************************************************/
//    Author                    : tywind
//    Original Date             : 6 JUNE,2016
//    Module Name               : ssh_terminal.c
//    Module Funciton           : 
//    Description               : Implementation code of terminal application.
//    Last modified Author      :
//    Last modified Date        : 
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//    Extra comment             : 
//***********************************************************************/

#ifndef __STDAFX_H__
#include <StdAfx.h>
#endif

#include "stdlib.h"
#include "ssh/ssh.h"


#define CLAMP(arg, lim)		((arg) = ((arg) > (lim)) ? (lim) : (arg))
#define LATTR_NORM			0x00000000UL
#define LATTR_WIDE			0x00000001UL
#define LATTR_TOP			0x00000002UL
#define LATTR_BOT			0x00000003UL
#define LATTR_MODE			0x00000003UL
#define LATTR_WRAPPED		0x00000010UL     /* this line wraps to next */
#define LATTR_WRAPPED2		0x00000020UL    


#define ANSI(x,y) ((x)+((y)<<8))
#define ANSI_QUE(x) ANSI(x,TRUE)
#define CSET_ASCII 0x0000D800UL 


#define SWP_SAVE_SCREEN        1
#define SWP_RESTORE_SCREEN     2

#define ARGS_MAX 32 

enum terminal_state
{
	SEEN_CHAR,
	SEEN_ESC,
	SEEN_CSI,
	SEEN_OSC,
	SEEN_OSC_W,
	DO_CTRLS,
	SEEN_OSC_P,
	OSC_STRING, 	
} ;

typedef struct
{
	enum terminal_state state;
	int esc_args[ARGS_MAX];
	char*        screen_buf           ;  
	int          screen_len           ;
	int          alt_witch            ;
	int          cmd_line             ;
	int          esc_query            ;
	int          esc_nargs            ;
	int          margin_t             ;
	int          margin_b             ;

	int         row;
	int         col;
}terminal;

static terminal*  term = NULL;

VOID  VGA_EnabledScroll(BOOL bAllowScroll);

int t_IsDigit(unsigned long c)
{
	if(c >= 0x30 && c <= 0x39)
	{
		return 1;
	}
	else 
	{
		return 0;
	}
}

void put_char(char ch)
{
	CD_PrintChar(ch);
}

void get_cursor_pos(int* x, int* y)
{
	CD_GetCursorPos((WORD*)x,(WORD*)y);
}

void set_cursor_pos(int x, int y)
{
	if(x < 0) x = 0;
	if(y < 0) y = 0;

	CD_SetCursorPos(x,y);
}


void swap_screen(int witch)
{
	int  clear = 0;

	if(witch)
	{
		clear = 1;
	}

	if(witch != term->alt_witch)
	{
		char* temp = 0;
		int  i  = 0;

		term->alt_witch = witch;

		temp = (char*)_hx_malloc(term->screen_len);
		memset(temp,0,term->screen_len);
		memcpy(temp,term->screen_buf,term->screen_len);

		set_cursor_pos(0,0);
		memset(term->screen_buf,0,term->screen_len);
		CD_GetScreen(term->screen_buf,term->screen_len);

		
		set_cursor_pos(0,0);
		for(i=0;i<term->screen_len;i++)
		{
			put_char(temp[i]);			
		}		
		_hx_free(temp);
		/*
		ttr = term->alt_screen;
		term->alt_screen = term->screen;
		term->screen = ttr;
		*/
	}
	
	if(clear)
	{		
		CD_Clear();
		set_cursor_pos(0,0);
	}

	//screen_switch
	/*if(mode == SWP_SAVE_SCREEN)
	{
		//memset(term->screen_buf,0,term->screen_len);
		
	}
	else
	{
		int  i=0;
		CD_SetCursorPos(0,0);

		for(i=0;i<term->screen_len;i++)
		{
			CD_PrintChar(term->screen_buf[i]);			
		}
	}*/
}

void eraseline(int curline,int from,int end)
{	
	int i,j;
	
	
	if(curline == 1)
	{
		int x,y;
		
		get_cursor_pos(&x,&y);		
		for(i=0;i<term->col-x-1;i++)
		{
			put_char(' ');
		}
		set_cursor_pos(x,y);
	}
	else
	{	
		set_cursor_pos(0,0);

		for(j=0;j<term->row;j++)
			for(i=0;i<term->col;i++)
			{
				put_char(' ');
			}
		set_cursor_pos(0,0);
	}	
}


void toggle_mode(int mode, int query, int state)
{
	if (query == 0)  return;

	switch (mode) 
	{
		case 1:		       /* DECCKM: application cursor keys */
			break;
		case 2:		       /* DECANM: VT52 mode */
			break;
		case 3:		       /* DECCOLM: 80/132 columns */	   
			break;
		case 5:		       /* DECSCNM: reverse video */	  
			break;
		case 6:		       /* DECOM: DEC origin mode */	    
			break;
		case 7:		       /* DECAWM: auto wrap */	    
			break;
		case 8:		       /* DECARM: auto key repeat */
			break;
		case 10:		   /* DECEDM: set local edit mode */	    
			break;
		case 25:		   /* DECTCEM: enable/disable cursor */	    

			break;
		case 47:		     /* alternate screen */	   
			break;
		case 1000:		     /* xterm mouse 1 (normal) */
			break;
		case 1002:		    /* xterm mouse 2 (inc. button drags) */

			break;
		case 1006:		    /* xterm extended mouse */
			break;
		case 1015:		    /* urxvt extended mouse */	    
			break;
		case 1047:                   /* alternate screen */	   
			break;
		case 1048:                   /* save/restore cursor */

			break;
		case 1049:                   /* cursor & alternate screen */	
			{			

				swap_screen(state);
				/*if(term->cmd_line == 1)
				{
					swap_screen(SWP_SAVE_SCREEN);
					ClearScreen();
					term->cmd_line = 0;
				}
				else
				{
					ClearScreen();
					swap_screen(SWP_RESTORE_SCREEN);
					term->cmd_line = 1;
				}*/
			}
			break;
		case 2004:		       /* xterm bracketed paste */	    
			break;
	}
}


void terminal_ESC(int c)
{
	
	if (c >= ' ' && c <= '/') 
	{
		if (term->esc_query)
			term->esc_query = -1;
		else
			term->esc_query = c;

		return;
	}

	term->state = SEEN_CHAR;
	switch (ANSI(c, term->esc_query)) 
	{
		case '[':		/* enter CSI mode */
			{
			term->state       = SEEN_CSI;
			term->esc_nargs   = 1;		    
			term->esc_args[0] = 0;
			term->esc_args[1] = 0;
			term->esc_query   = FALSE;
			}
			break;
		case ']':		/* OSC: xterm escape sequences */
			{
			term->state = SEEN_OSC;
			term->esc_nargs   = 1;				
			term->esc_args[0] = 0;			
			term->esc_args[1] = 0;
			}
			break;
		case '7':		/* DECSC: save cursor */
			break;
		case '8':	 	/* DECRC: restore cursor */
			break;
		case '=':		/* DECKPAM: Keypad application mode */
			break;
		case '>':		/* DECKPNM: Keypad numeric mode */
			break;
		case 'D':	       /* IND: exactly equivalent to LF */
			break;
		case 'E':	       /* NEL: exactly equivalent to CR-LF */
			break;
		case 'M':	       /* RI: reverse index - backwards LF */		    
			break;
		case 'Z':	       /* DECID: terminal type query */
			break;
		case 'H':	       /* HTS: set a tab */
			break;
		case ANSI('8', '#'):	/* DECALN: fills screen with Es :-) */
			break;
		case ANSI('3', '#'):
		case ANSI('4', '#'):
		case ANSI('5', '#'):
		case ANSI('6', '#'):		
			{
				int nlattr;
				switch (ANSI(c,term->esc_query)) 
				{
				case ANSI('3', '#'): /* DECDHL: 2*height, top */
					nlattr = LATTR_TOP;
					break;
				case ANSI('4', '#'): /* DECDHL: 2*height, bottom */
					nlattr = LATTR_BOT;
					break;
				case ANSI('5', '#'): /* DECSWL: normal */
					nlattr = LATTR_NORM;
					break;
				default: /* case ANSI('6', '#'): DECDWL: 2*width */
					nlattr = LATTR_WIDE;
					break;
				}
			}
			break;
			/* GZD4: G0 designate 94-set */
		case ANSI('A', '('):
			break;
		case ANSI('B', '('):
			break;
		case ANSI('0', '('):
			break;
		case ANSI('U', '('): 		    
			break;
			/* G1D4: G1-designate 94-set */
		case ANSI('A', ')'):
			break;
		case ANSI('B', ')'):
			break;
		case ANSI('0', ')'):
			break;
		case ANSI('U', ')'): 
			break;
			/* DOCS: Designate other coding system */
		case ANSI('8', '%'):	/* Old Linux code */
		case ANSI('G', '%'):
			break;
		case ANSI('@', '%'):
			break;
	}
	
	return;
}

void terminal_SGR()
{
	static BYTE  szAttrMap[8] = {0,4,2,6,1,5,3,7};
	static BYTE  bCurrAttr    = 0x07;
	int    i;

	for(i=0;i<term->esc_nargs;i++)
	{	
		unsigned int val = term->esc_args[i];

		switch(term->esc_args[i])
		{
			case 0: //default
				{
				bCurrAttr = 0x07;
				CD_SetCharAttr(bCurrAttr);
				}
				break;
			case 30:
			case 31:
			case 32:
			case 33:
			case 34:
			case 35:
			case 36:
			case 37:
				{				
				
				bCurrAttr = (bCurrAttr&0xF8)|(szAttrMap[val-30])|0x08;
				CD_SetCharAttr(bCurrAttr);
				}
				break;
			case 40:
			case 41:
			case 42:
			case 43:
			case 44:
			case 45:
			case 46:
			case 47:
				{
				BYTE  attr = szAttrMap[val-40];

				bCurrAttr = (bCurrAttr&0x8F)|(attr << 4)|0x08;
				CD_SetCharAttr(bCurrAttr);
				}
				break;
		}
	}
}

void terminal_CSI(int c)
{
	term->state = SEEN_CHAR;  /* default */
	
	if (t_IsDigit(c)) 
	{
		//reocrd params
		unsigned int *p  = &term->esc_args[term->esc_nargs - 1];

		*p = 10 * (*p) +	c - '0';

		term->state = SEEN_CSI;
	} 
	else if (c == ';') 
	{
		//add params
		term->esc_args[term->esc_nargs++] = 0;
		term->state = SEEN_CSI;
	} 
	else if (c < '@') 
	{
		if (term->esc_query)
		{
			term->esc_query = -1;
		}
		else if (c == '?')
		{
			term->esc_query = TRUE;
		}
		else term->esc_query = c;
		{
			term->state = SEEN_CSI;
		}
	}
	else
	{	
		switch (ANSI(c, term->esc_query)) 
		{
		case 'A':       /* CUU: move up N lines */
			{
			int x,y;

			get_cursor_pos(&x,&y);

			y -= term->esc_args[0]?term->esc_args[0]:1;				
			set_cursor_pos(x,y);				
			}
			break;
		case 'e':		/* VPR: move down N lines */
			/* FALLTHROUGH */
		case 'B':		/* CUD: Cursor down */

			break;
		case ANSI('c', '>'):	/* DA: report xterm version */
			break;
		case 'a':		/* HPR: move right N cols */
			/* FALLTHROUGH */
		case 'C':		/* CUF: Cursor right */ 
			{
			int  row,col;
			int  movecol;

			get_cursor_pos(&col,&row);
			movecol = term->esc_args[0]?term->esc_args[0]:1;

			set_cursor_pos(col+movecol,row);
			}
			break;
		case 'D':       /* CUB: move left N cols */
			{			
			int  row,col;
			int  movecol;

			get_cursor_pos(&col,&row);
			movecol = term->esc_args[0]?term->esc_args[0]:1;

			set_cursor_pos(col-movecol,row);
			}
			break;
		case 'E':       /* CNL: move down N lines and CR */
			{
			int  row,col;
			int  moveline;
			
			get_cursor_pos(&col,&row);
			moveline = term->esc_args[0]?term->esc_args[0]:1;

			set_cursor_pos(0,row+moveline);
			}

			break;
		case 'F':       /* CPL: move up N lines and CR */
			{
			int  row,col;
			int  moveline;

			get_cursor_pos(&col,&row);
			moveline = term->esc_args[0]?term->esc_args[0]:1;

			set_cursor_pos(0,row-moveline);

			}
			break;
		case 'G':	      /* CHA */
		case '`':       /* HPA: set horizontal posn */		
			{
			int  row,col;

			get_cursor_pos(&col,&row);
			col = term->esc_args[0]?term->esc_args[0]:1;
			set_cursor_pos(col-1,row);
			}
			break;
		case 'd':       /* VPA: set vertical posn */
			{
			int  row,col;

			get_cursor_pos(&col,&row);
			row = term->esc_args[0]?term->esc_args[0]:1;
			set_cursor_pos(col,row-1);
			}
			break;
		case 'H':	     /* CUP */
		case 'f':      /* HVP: set horz and vert posns at once */			
			{
			int  row,col;

			row = term->esc_args[0]?term->esc_args[0]:1;
			col = term->esc_args[1]?term->esc_args[1]:1;
	
			set_cursor_pos(col-1,row-1);
			}
			break;
		case 'J':       /* ED: erase screen or parts of it */
			{			
			eraseline(0,0,0);
			}
			break;
		case 'K':       /* EL: erase line or parts of it */
			{
			eraseline(1,0,0);
			}
			break;
		case 'L':       /* IL: insert lines */
			break;
		case 'M':       /* DL: delete lines */
			break;
		case '@':       /* ICH: insert chars */
			break;
		case 'P':       /* DCH: delete chars */
			break;
		case 'c':       /* DA: terminal type query */		
			break;
		case 'n':       /* DSR: cursor position query */
			break;
		case 'h':       /* SM: toggle modes to high */
		case ANSI_QUE('h'):
			{
				int i;
				for (i = 0; i < term->esc_nargs; i++)
				{
					toggle_mode(term->esc_args[i],term->esc_query, TRUE);
				}						
			}
			break;
		case 'i':		/* MC: Media copy */
		case ANSI_QUE('i'):			
			break;			
		case 'l':       /* RM: toggle modes to low */
		case ANSI_QUE('l'):
			{
				int i;
				for (i = 0; i < term->esc_nargs; i++)
				{
					toggle_mode(term->esc_args[i],term->esc_query, FALSE);
				}
			}
			break;
		case 'g':       /* TBC: clear tabs */
			break;
		case 'r':       /* DECSTBM: set scroll margins */
			{
			term->margin_t = term->esc_args[0]-1;
			term->margin_b = term->esc_args[1]-1;
			}
			break;
		case 'm':       /* SGR: set graphics rendition */
			{		
			terminal_SGR();

			}
			break;
		case 's':       /* save cursor */

			break;
		case 'u':       /* restore cursor */

			break;
		case 't': /* DECSLPP: set page size - ie window height */
			/*
			* VT340/VT420 sequence DECSLPP, DEC only allows values
			*  24/25/36/48/72/144 other emulators (eg dtterm) use
			* illegal values (eg first arg 1..9) for window changing 
			* and reports.
			*/

			break;
		case 'S':		/* SU: Scroll up */

			break;
		case 'T':		/* SD: Scroll down */

			break;
		case ANSI('|', '*'): /* DECSNLS */

			break;
		case ANSI('|', '$'): /* DECSCPP */

			break;
		case 'X':     /* ECH: write N spaces w/o moving cursor */
			/* XXX VTTEST says this is vt220, vt510 manual
			* says vt100 */

			break;
		case 'x':       /* DECREQTPARM: report terminal characteristics */

			break;
		case 'Z':		/* CBT */

			break;
		case ANSI('c', '='):      /* Hide or Show Cursor */

			break;
		case ANSI('C', '='):
			/*
			* set cursor start on scanline esc_args[0] and
			* end on scanline esc_args[1].If you set
			* the bottom scan line to a value less than
			* the top scan line, the cursor will disappear.
			*/

			break;
		case ANSI('D', '='):

			break;
		case ANSI('E', '='):

			break;
		case ANSI('F', '='):      /* set normal foreground */

			break;
		case ANSI('G', '='):      /* set normal background */

			break;
		case ANSI('L', '='):

			break;
		case ANSI('p', '"'): /* DECSCL: set compat level */

			break;
		}		

	}	
}

void terminal_OSC(int c)
{
	switch (c) 
	{
		case 'P':	       /* Linux palette sequence */
			term->state = SEEN_OSC_P;

			break;
		case 'R':	       /* Linux palette reset */
			term->state = SEEN_CHAR;
			break;
		case 'W':	       /* word-set */
			term->state = SEEN_OSC_W;

			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':

			break;
		case 'L':

			/* else fall through */
		default:
			term->state = OSC_STRING;
	}
}


void terminal_OSC_STRING(int c)
{
	
	if (c == '\012' || c == '\015') 
	{
		term->state = SEEN_CHAR;
	} 
	else if (c == 0234 || c == '\007') 
	{
		/*
		* These characters terminate the string; ST and BEL
		* terminate the sequence and trigger instant
		* processing of it, whereas ESC goes back to SEEN_ESC
		* mode unless it is followed by \, in which case it is
		* synonymous with ST in the first place.
		*/

		term->state = SEEN_CHAR;
	} else if (c == '\033')
	{
		//t_state = OSC_MAYBE_ST;
	}


}

void terminal_OSC_P(int c)
{
	term->state = SEEN_CHAR;
}

void terminal_OSC_W(int c)
{
	switch (c) 
	{
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':

		break;
	default:
		term->state = OSC_STRING;
	}
}

void terminal_CHAR(int ch)
{
	int x,y;		

	get_cursor_pos(&x,&y);

	if(ch == VK_BACKSPACE)
	{				
		if(x > 0)
		{
			set_cursor_pos(x-1,y);
		}
		else if(y > 0)
		{
			set_cursor_pos(term->col-1,y-1);
		}
		CD_DelChar(DISPLAY_DELCHAR_CURR);		
	}
	else if(ch >= 0x20 && ch <= 0x7E)
	{
		//disabled auto change line,so got last col and old row
		if(x == term->col-1)
		{
			set_cursor_pos(term->col-2,y);
		}

		put_char(ch);		
	} 
	else if(ch == 0x0D) //CR
	{
		set_cursor_pos(0,y);

	}else if(ch == 0x0A) //CL
	{
		set_cursor_pos(x,y+1);
	}
}

void terminal_init()
{
	if(term == NULL)
	{
		WORD  row,col;

		term = (terminal*)_hx_malloc(sizeof(terminal));
		term->cmd_line = 1;

		CD_GetDisPlayRang(&row,&col);
		term->screen_len = row*col;
		term->screen_buf = (char*)_hx_malloc(term->screen_len);
		memset(term->screen_buf,0,term->screen_len);
	
		term->state         = SEEN_CHAR;
		term->col           = col;
		term->row           = row;

		term->esc_args[0]   = 0;
		term->esc_query     = 0;
		term->esc_nargs     = 0;
	}
	
}

void terminal_uninit()
{
	if(term)
	{
		_hx_free(term->screen_buf);
		_hx_free(term);		
		term = NULL;
	}
}

int  terminal_cmdline()
{
	return term->cmd_line;
}

void terminal_analyze(char* srcstr,int len)
{	
	char*        strpos   = srcstr;	
	int          nchars   = len;
	int          c        = 0;
		
	while (nchars > 0) 
	{
		c = *strpos++; nchars--;

		if(c >= 0x7F) 
		{
			continue;
		}

		if (c == 0x1B && term->state < DO_CTRLS) 
		{
			/* ESC: Escape */			
			term->state     = SEEN_ESC;
			term->esc_query = FALSE;	
		}
		else
		{
			switch(term->state)
			{
				case SEEN_CHAR:
					terminal_CHAR(c);
					break;
				case SEEN_ESC:
					terminal_ESC(c);
					break;
				case SEEN_CSI:	
					terminal_CSI(c);
					break;
				case SEEN_OSC:
					terminal_OSC(c);
					break;
				case SEEN_OSC_P:
					terminal_OSC_P(c);
					break;
				case SEEN_OSC_W:
					terminal_OSC_W(c);
					break;
				case OSC_STRING:
					terminal_OSC_STRING(c);
					break;
			}
		}
	}

}
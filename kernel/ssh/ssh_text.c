


#include "ssh_def.h"
//#include "ssh/ssh.h"

#define CLAMP(arg, lim) ((arg) = ((arg) > (lim)) ? (lim) : (arg))
#define LATTR_NORM   0x00000000UL
#define LATTR_WIDE   0x00000001UL
#define LATTR_TOP    0x00000002UL
#define LATTR_BOT    0x00000003UL
#define LATTR_MODE   0x00000003UL
#define LATTR_WRAPPED 0x00000010UL     /* this line wraps to next */
#define LATTR_WRAPPED2 0x00000020UL    


#define ANSI(x,y) ((x)+((y)<<8))
#define ANSI_QUE(x) ANSI(x,TRUE)
#define CSET_ASCII 0x0000D800UL 

int IsDigit(unsigned long c)
{
	if(c >= 0x30 && c <= 0x39)
		 return 1;
	else return 0;
}

void analyze_str(const char* src,int len,char* dst)
{

	enum {
		TOPLEVEL,
		SEEN_ESC,
		SEEN_CSI,
		SEEN_OSC,
		SEEN_OSC_W,
		DO_CTRLS,
		SEEN_OSC_P,
		OSC_STRING, OSC_MAYBE_ST,
		VT52_ESC,
		VT52_Y1,
		VT52_Y2,
		VT52_FG,
		VT52_BG
	} tstate ;

	unsigned long c;
	int    unget;	
	uint8 *chars    = (uint8*)src;
	char  *dstpos   = dst;
	int   nchars    = len;
	int   esc_query = 0;

	unget = -1;
			 
	tstate = TOPLEVEL;
	while (nchars > 0 || unget != -1 ) 
	{
		c = *chars++; nchars--;
		
		if(c >= 0x7F) continue;
				
		/*
		* How about C1 controls? 
		* Explicitly ignore SCI (0x9a), which we don't translate to DECID.
		*/
		if ((c & -32) == 0x80 && tstate < DO_CTRLS ) 
		{
			if (c == 0x9a)
			{
				c = 0;
			}
			else 
			{
				tstate = SEEN_ESC;
				esc_query = FALSE;
				c = '@' + (c & 0x1F);
			}
		}
		
		if (c == 0x1B && tstate < DO_CTRLS) 
		{
			 /* ESC: Escape */			
			tstate    = SEEN_ESC;
			esc_query = FALSE;	
			
		}
		else
			switch (tstate) 
		{
			case TOPLEVEL:		
				{
					*dstpos =  (char)c;
					dstpos ++;
				}
				break;

			case OSC_MAYBE_ST:

				if (c == '\\') 
				{
					tstate = TOPLEVEL;
					break;
				}
				/* else fall through */
			case SEEN_ESC:
				if (c >= ' ' && c <= '/') 
				{
					if (esc_query)
						esc_query = -1;
					else
						esc_query = c;
					break;
				}
				tstate = TOPLEVEL;
				switch (ANSI(c, esc_query)) 
				{
				case '[':		/* enter CSI mode */
					tstate = SEEN_CSI;
					//esc_nargs = 1;		    
					esc_query = FALSE;
					break;
				case ']':		/* OSC: xterm escape sequences */

					tstate = SEEN_OSC;
					//term->esc_args[0] = 0;
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
					//compatibility(VT100);
					{
						int nlattr;
						switch (ANSI(c,esc_query)) 
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
				break;
			case SEEN_CSI:
				tstate = TOPLEVEL;  /* default */
				if (IsDigit(c)) 
				{
					tstate = SEEN_CSI;
				} else if (c == ';') 
				{

					tstate = SEEN_CSI;
				} else if (c < '@') 
				{
					if (esc_query) 	esc_query = -1;
					else if (c == '?')
						esc_query = TRUE;
					else esc_query = c;
					tstate = SEEN_CSI;
				} else

					switch (ANSI(c, esc_query)) 
				{
					case 'A':       /* CUU: move up N lines */
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

						break;
					case 'D':       /* CUB: move left N cols */

						break;
					case 'E':       /* CNL: move down N lines and CR */

						break;
					case 'F':       /* CPL: move up N lines and CR */

						break;
					case 'G':	      /* CHA */
					case '`':       /* HPA: set horizontal posn */		
						break;
					case 'd':       /* VPA: set vertical posn */

						break;
					case 'H':	     /* CUP */
					case 'f':      /* HVP: set horz and vert posns at once */

						break;
					case 'J':       /* ED: erase screen or parts of it */

						break;
					case 'K':       /* EL: erase line or parts of it */
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

						break;
					case 'i':		/* MC: Media copy */
					case ANSI_QUE('i'):			
						break;			
					case 'l':       /* RM: toggle modes to low */
					case ANSI_QUE('l'):

						break;
					case 'g':       /* TBC: clear tabs */

						break;
					case 'r':       /* DECSTBM: set scroll margins */

						break;
					case 'm':       /* SGR: set graphics rendition */
						{
							/* 
							* A VT100 without the AVO only had one
							* attribute, either underline or
							* reverse video depending on the
							* cursor type, this was selected by
							* CSI 7m.
							*
							* case 2:
							*  This is sometimes DIM, eg on the
							*  GIGI and Linux
							* case 8:
							*  This is sometimes INVIS various ANSI.
							* case 21:
							*  This like 22 disables BOLD, DIM and INVIS
							*
							* The ANSI colours appear on any
							* terminal that has colour (obviously)
							* but the interaction between sgr0 and
							* the colours varies but is usually
							* related to the background colour
							* erase item. The interaction between
							* colour attributes and the mono ones
							* is also very implementation
							* dependent.
							*
							* The 39 and 49 attributes are likely
							* to be unimplemented.
							*/

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
				break;
			case SEEN_OSC:			
				switch (c) {
				case 'P':	       /* Linux palette sequence */
					tstate = SEEN_OSC_P;

					break;
				case 'R':	       /* Linux palette reset */
					tstate = TOPLEVEL;
					break;
				case 'W':	       /* word-set */
					tstate = SEEN_OSC_W;

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
					tstate = OSC_STRING;

				}
				break;
			case OSC_STRING:
				/*
				* This OSC stuff is EVIL. It takes just one character to get into
				* sysline mode and it's not initially obvious how to get out.
				* So I've added CR and LF as string aborts.
				* This shouldn't effect compatibility as I believe embedded 
				* control characters are supposed to be interpreted (maybe?) 
				* and they don't display anything useful anyway.
				*
				* -- RDB
				*/
				if (c == '\012' || c == '\015') 
				{
					tstate = TOPLEVEL;
				} else if (c == 0234 || c == '\007') 
				{
					/*
					* These characters terminate the string; ST and BEL
					* terminate the sequence and trigger instant
					* processing of it, whereas ESC goes back to SEEN_ESC
					* mode unless it is followed by \, in which case it is
					* synonymous with ST in the first place.
					*/

					tstate = TOPLEVEL;
				} else if (c == '\033')
					tstate = OSC_MAYBE_ST;

				break;
			case SEEN_OSC_P:
				{
					tstate = TOPLEVEL;
				}
				break;
			case SEEN_OSC_W:
				switch (c) {
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
					tstate = OSC_STRING;

				}
				break;
			

			default: break;	       /* placate gcc warning about enum use */
		}

	}


}
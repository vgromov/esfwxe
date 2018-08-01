// our own string utiliities implementation
//
// maximum local buffer length for string conversion
#define ES_STRFMT_LOCALBUF_LEN 	(sizeof(long) * 8 + 1)
#define FETCH_BYTE(pch)					(*((esU8*)(pch)))
#define mkhex(b)								c_hexChars[(b) & 0x0F]

static ES_ASCII_CSTR c_nullFmt = "(null)\n";

// 0-terminated string length
int utilsStrLenGet(ES_ASCII_CSTR str)
{
	const ES_ASCII_CHAR* pos = str;
	while(*pos)
		++pos;

	return pos-str;
}

/*
 * Put a NUL-terminated ASCII number (base <= 16) in a buffer in reverse
 * order; return an optional length and a pointer to the last character
 * written in the buffer (i.e., the first character of the string).
 * The buffer pointed to by `nbuf' must have length >= MAXNBUF.
 */
static esU8* es_sprintn(esU8 *nbuf, esU32 ul, esU8 base, int width, esU8 *lenp)
{
	esU8* p = nbuf;
	*p = 0;
	for (;;) {
					*++p = mkhex(ul % base);
					ul /= base;
					if (--width > 0)
									continue;
					if (! ul)
									break;
	}
	if (lenp)
					*lenp = p - nbuf;
	return (p);
}

// abstract string formatter and streamer
int utilsVstrFmtStream(
	utilsPfnChStreamFn pfn,	// streaming function
	void* target,					// target stream
	ES_ASCII_CSTR fmt,
	va_list lst)
{
#define ES_PUTC(c)	if( utilsStreamOk == pfn(target, (esU8)(c)) ) ++retval; else return utilsStreamEnd;

	esU8 nbuf[ES_STRFMT_LOCALBUF_LEN], padding, *q;
	const esU8* s;
	esU8 c, base, lflag, ladjust, sharpflag, neg, dot, size;
	int n, width, dwidth, retval, uppercase, extrazeros, sign;
	esU32 ul;

	if(!target)
		return 0;

	if(!fmt)
		fmt = c_nullFmt;

	retval = 0;
	for (;;) {
	while((c = FETCH_BYTE(fmt++)) != '%') 
	{
		if(! c)
			return retval;
		ES_PUTC(c);
	}
	padding = ' ';
	width = 0; extrazeros = 0;
	lflag = 0; ladjust = 0; sharpflag = 0; neg = 0;
	sign = 0; dot = 0; uppercase = 0; dwidth = -1;
  
reswitch:			 
	switch(c = FETCH_BYTE(fmt++)) 
	{
		case '.':
			dot = 1;
			padding = ' ';
			dwidth = 0;
			goto reswitch;

		case '#':
			sharpflag = 1;
			goto reswitch;

		case '+':
			sign = -1;
			goto reswitch;

		case '-':
			ladjust = 1;
			goto reswitch;

		case '%':
			ES_PUTC(c);
			break;

		case '*':
			if (! dot) {
				width = va_arg (lst, int);
				if (width < 0) {
					ladjust = !ladjust;
					width = -width;
				}
			} else {
				dwidth = va_arg (lst, int);
			}
			goto reswitch;

		case '0':
			if (! dot) {
				padding = '0';
				goto reswitch;
			}
		case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			for (n=0; ; ++fmt) {
				n = n * 10 + c - '0';
				c = FETCH_BYTE (fmt);
				if (c < '0' || c > '9')
					break;
			}
			if (dot)
				dwidth = n;
			else
				width = n;
			goto reswitch;

		case 'b':
			ul = va_arg (lst, int);
			s = va_arg (lst, const unsigned char*);
			q = es_sprintn(nbuf, ul, *s++, -1, 0);
			while (*q)
				ES_PUTC (*q--);

			if (! ul)
				break;
			size = 0;
			while (*s) {
				n = *s++;
				if ((char) (ul >> (n-1)) & 1) {
					ES_PUTC (size ? ',' : '<');
					for (; (n = *s) > ' '; ++s)
						ES_PUTC (n);
					size = 1;
				} else
					while (*s > ' ')
						++s;
			}
			if (size)
				ES_PUTC ('>');
			break;

		case 'c':
			if (! ladjust && width > 0)
				while (width--)
					ES_PUTC (' ');

			ES_PUTC (va_arg (lst, int));

			if (ladjust && width > 0)
				while (width--)
					ES_PUTC (' ');
			break;

		case 'D':
			s = va_arg (lst, const unsigned char*);
			if (! width)
				width = 16;
			if (sharpflag)
				padding = ':';
			while (width--) {
				c = *s++;
				ES_PUTC (mkhex (c >> 4));
				ES_PUTC (mkhex (c));
				if (width)
					ES_PUTC (padding);
			}
			break;

		case 'd':
			ul = lflag ? va_arg (lst, long) : va_arg (lst, int);
			if (! sign) sign = 1;
			base = 10;
			goto number;

		case 'l':
			lflag = 1;
			goto reswitch;

		case 'o':
			ul = lflag ? va_arg (lst, unsigned long) :
				va_arg (lst, unsigned int);
			base = 8;
			goto nosign;

		case 'p':
			ul = (size_t) va_arg (lst, void*);
			if (! ul) {
				s = (const unsigned char*) "(nil)";
				goto const_string;
			}
			base = 16;
			sharpflag = (width == 0);
			goto nosign;

		case 'n':
			ul = lflag ? va_arg (lst, unsigned long) :
				sign ? (unsigned long) va_arg (lst, int) :
				va_arg (lst, unsigned int);
			base = 10;
			goto number;

		case 'S':
		case 's':
			s = va_arg (lst, unsigned char*);
			if (! s)
				s = (const unsigned char*) "(null)";
	const_string:
			if (! dot)
				n = utilsStrLenGet((ES_ASCII_CSTR)s);
			else
				for (n=0; n<dwidth && s[n]; n++)
					continue;

			width -= n;

			if (! ladjust && width > 0)
				while (width--)
					ES_PUTC (' ');
			while (n--)
				ES_PUTC (*s++);
			if (ladjust && width > 0)
				while (width--)
					ES_PUTC (' ');
			break;

		case 'r':
			/* Saturated counters. */
			base = 10;
			if (lflag) {
				ul = va_arg (lst, unsigned long);
				if(ul == (esU32)-1) 
				{
cnt_unknown:														
					if (ladjust)
									ES_PUTC ('-');
								while (--width > 0)
									ES_PUTC (' ');
								if (! ladjust)
									ES_PUTC ('-');
								break;
				}
				if (ul >= (esU32)-2) 
				{
					ul = (esU32)-3;
					neg = '>';
					goto nosign;
				}
			} 
			else 
			{
				ul = va_arg (lst, unsigned int);
				if (ul == (unsigned short) -1)
					goto cnt_unknown;
				if (ul >= (unsigned short) -2) {
					ul = (unsigned short) -3;
					neg = '>';
					goto nosign;
				}
			}
			goto nosign;

		case 'u':
			ul = lflag ? va_arg (lst, unsigned long) :
				va_arg (lst, unsigned int);
			base = 10;
			goto nosign;

		case 'x':
		case 'X':
			ul = lflag ? va_arg (lst, unsigned long) :
				va_arg (lst, unsigned int);
			base = 16;
			uppercase = (c == 'X');
			goto nosign;
		case 'z':
		case 'Z':
			ul = lflag ? va_arg (lst, unsigned long) :
				sign ? (unsigned long) va_arg (lst, int) :
				va_arg (lst, unsigned int);
			base = 16;
			uppercase = (c == 'Z');
			goto number;

nosign:								 
		sign = 0;
number:				 
		if (sign && ((long) ul != 0L)) {
			if ((long) ul < 0L) {
				neg = '-';
				ul = -(long) ul;
			} else if (sign < 0)
				neg = '+';
	 }
	 if (dwidth >= (int) sizeof(nbuf)) {
		 extrazeros = dwidth - sizeof(nbuf) + 1;
		 dwidth = sizeof(nbuf) - 1;
	 }
	 s = es_sprintn (nbuf, ul, base, dwidth, &size);
	 if (sharpflag && ul != 0) {
		 if (base == 8)
			 size++;
		 else if (base == 16)
			 size += 2;
	 }
	 if (neg)
		 size++;

	 if (! ladjust && width && padding == ' ' &&
		 (width -= size) > 0)
		 do {
			 ES_PUTC (' ');
		 } while (--width > 0);

	 if (neg)
		 ES_PUTC (neg);

	 if (sharpflag && ul != 0) {
		 if (base == 8) {
			 ES_PUTC ('0');
		 } else if (base == 16) {
			 ES_PUTC ('0');
			 ES_PUTC (uppercase ? 'X' : 'x');
		 }
	 }

	 if (extrazeros)
		 do {
			 ES_PUTC ('0');
		 } while (--extrazeros > 0);

	 if (! ladjust && width && (width -= size) > 0)
		 do {
			 ES_PUTC (padding);
		 } while (--width > 0);

	 for (; *s; --s) {
		 if (uppercase && *s>='a' && *s<='z') {
			 ES_PUTC (*s + 'A' - 'a');
		 } else {
			 ES_PUTC (*s);
		 }
	 }

	 if (ladjust && width && (width -= size) > 0)
		 do {
			 ES_PUTC (' ');
		 } while (--width > 0);
	 break;
	#if ARCH_HAVE_FPU
		case 'e':
		case 'E':
		case 'f':
		case 'F':
		case 'g':
		case 'G': {
			double d = va_arg (lst, double);
			/*
			* don't do unrealistic precision; just pad it with
			* zeroes later, so buffer size stays rational.
			*/
			if (dwidth > DBL_DIG) {
				if ((c != 'g' && c != 'G') || sharpflag)
					extrazeros = dwidth - DBL_DIG;
				dwidth = DBL_DIG;
			} else if (dwidth == -1) {
				dwidth = (lflag ? DBL_DIG : FLT_DIG);
			}
			/*
			* softsign avoids negative 0 if d is < 0 and
			* no significant digits will be shown
			*/
			if (d < 0) {
				neg = 1;
				d = -d;
			}
			/*
			* cvt may have to round up past the "start" of the
			* buffer, i.e. ``intf("%.2f", (double)9.999);'';
			* if the first char isn't NULL, it did.
			*/
			if (isnan (d) || isinf (d)) {
				strcpy_flash (nbuf, isnan (d) ? "NaN" : "Inf");
				size = 3;
				extrazeros = 0;
				s = nbuf;
			} else {
				*nbuf = 0;
				size = cvt (d, dwidth, sharpflag, &neg, c,
					nbuf, nbuf + sizeof(nbuf) - 1);
				if (*nbuf) {
					s = nbuf;
					nbuf [size] = 0;
				} else {
					s = nbuf + 1;
					nbuf [size + 1] = 0;
				}
			}
			if (neg)
				size++;
			if (! ladjust && width && padding == ' ' &&
				(width -= size) > 0)
				do {
					ES_PUTC (' ');
				} while (--width > 0);

			if (neg)
				ES_PUTC ('-');

			if (! ladjust && width && (width -= size) > 0)
				do {
					ES_PUTC (padding);
				} while (--width > 0);

			for (; *s; ++s) {
				if (extrazeros && (*s == 'e' || *s == 'E'))
					do {
						ES_PUTC ('0');
					} while (--extrazeros > 0);

				ES_PUTC (*s);
			}
			if (extrazeros)
				do {
					ES_PUTC ('0');
				} while (--extrazeros > 0);

			if (ladjust && width && (width -= size) > 0)
				do {
					ES_PUTC (' ');
				} while (--width > 0);
			break;
							}
	#endif
		default:
			ES_PUTC ('%');
			if (lflag)
				ES_PUTC ('l');
			ES_PUTC (c);
			break;
		}
	}
}

// buffered string fomratters
//
typedef struct {
	esU8* pos;
	esU8* end;

} UtilsMemTarget;

static int utilsPutcMem(void* target, esU8 c)
{
	UtilsMemTarget* mtarget = (UtilsMemTarget*)target;
	*mtarget->pos = c;	// assign char
	if( !mtarget->end || 
			mtarget->pos < mtarget->end )
	{
		++mtarget->pos;	// increment target pointer
		return utilsStreamOk;
	}

	return utilsStreamEnd;
}

int utilsSprintf(ES_ASCII_STR target, ES_ASCII_CSTR fmt, ...)
{
	int result = utilsStreamEnd;
	if(target && fmt)
	{
		UtilsMemTarget mtarget;
		va_list arp;

		mtarget.pos = (esU8*)target;
		mtarget.end = 0;

		va_start(arp, fmt);
		result = utilsVstrFmtStream(utilsPutcMem, &mtarget, fmt, arp);
		va_end(arp);
	}

	return result;
}

int utilsSnprintf(ES_ASCII_STR target, esU32 maxTargetLen, ES_ASCII_CSTR fmt, ...)
{
	int result = utilsStreamEnd;
	if(target && fmt)
	{
		UtilsMemTarget mtarget;
		va_list arp;

		va_start(arp, fmt);
		mtarget.pos = (esU8*)target;
		mtarget.end = (esU8*)target+maxTargetLen;

		va_start(arp, fmt);
		result = utilsVstrFmtStream(utilsPutcMem, &mtarget, fmt, arp);
		va_end(arp);
	}

	return result;
}

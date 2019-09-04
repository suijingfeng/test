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
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// q_shared.c -- stateless support routines that are included in each code dll
#include "q_shared.h"

// q_math.c -- stateless support routines that are included in each code module

// Some of the vector functions are static inline in q_shared.h. q3asm
// doesn't understand static functions though, so we only want them in
// one file. That's what this is about.
const vec3_t vec3_origin = {0,0,0};
const vec3_t axisDefault[3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };

ID_INLINE void ClearBounds( vec3_t mins, vec3_t maxs )
{
	mins[0] = mins[1] = mins[2] = 99999;
	maxs[0] = maxs[1] = maxs[2] = -99999;
}

ID_INLINE void AddPointToBounds( const vec3_t v, vec3_t mins, vec3_t maxs )
{
	if ( v[0] < mins[0] )
		mins[0] = v[0];

	if ( v[0] > maxs[0])
		maxs[0] = v[0];


	if ( v[1] < mins[1] )
		mins[1] = v[1];

	if ( v[1] > maxs[1])
		maxs[1] = v[1];


	if ( v[2] < mins[2] )
		mins[2] = v[2];

	if ( v[2] > maxs[2])
		maxs[2] = v[2];
}



/*
=================
SetPlaneSignbits
=================
*/
ID_INLINE void SetPlaneSignbits (cplane_t *out)
{
	int	bits = 0, j = 0;

	// for fast box on planeside test
	for (j=0 ; j<3 ; j++)
    {
		if (out->normal[j] < 0)
        {
			bits |= 1<<j;
		}
	}
	out->signbits = bits;
}

/*
==================
BoxOnPlaneSide

Returns 1, 2, or 1 + 2
==================
*/
int BoxOnPlaneSide(vec3_t emins, vec3_t emaxs, struct cplane_s *p)
{
	float	dist[2];
	int		sides, b, i;

	// fast axial cases
	if (p->type < 3)
	{
		if (p->dist <= emins[p->type])
			return 1;
		if (p->dist >= emaxs[p->type])
			return 2;
		return 3;
	}

	// general case
	dist[0] = dist[1] = 0;
	if (p->signbits < 8) // >= 8: default case is original code (dist[0]=dist[1]=0)
	{
		for (i=0 ; i<3 ; i++)
		{
			b = (p->signbits >> i) & 1;
			dist[ b] += p->normal[i]*emaxs[i];
			dist[!b] += p->normal[i]*emins[i];
		}
	}

	sides = 0;
	if (dist[0] >= p->dist)
		sides = 1;
	if (dist[1] < p->dist)
		sides |= 2;

	return sides;
}

/*
=================
RadiusFromBounds
=================
*/
ID_INLINE float RadiusFromBounds( const vec3_t mins, const vec3_t maxs )
{
	int		i;
	vec3_t	v;

	for (i=0 ; i<3 ; i++)
    {
		float a = fabs( mins[i] );
		float b = fabs( maxs[i] );
		v[i] = a > b ? a : b;
	}

	return sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}


void AngleVectors( const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up)
{
	static float sr, sp, sy, cr, cp, cy;
	// static to help MS compiler fp bugs

	float angle = angles[YAW] * (M_PI / 180);
	sy = sin(angle);
	cy = cos(angle);

	angle = angles[PITCH] * (M_PI / 180);
	sp = sin(angle);
	cp = cos(angle);
	
    angle = angles[ROLL] * (M_PI / 180);
	sr = sin(angle);
	cr = cos(angle);

	if (forward)
	{
		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;
	}
	if (right)
	{
		right[0] = (-1*sr*sp*cy+-1*cr*-sy);
		right[1] = (-1*sr*sp*sy+-1*cr*cy);
		right[2] = -1*sr*cp;
	}
	if (up)
	{
		up[0] = (cr*sp*cy+-sr*-sy);
		up[1] = (cr*sp*sy+-sr*cy);
		up[2] = cr*cp;
	}
}


const char *COM_GetExtension( const char *name )
{
	const char* dot = strrchr(name, '.');
    const char* slash;

	if (dot && (!(slash = strrchr(name, '/')) || slash < dot))
		return dot + 1;
	else
		return "";
}



/*
============
COM_CompareExtension

string compare the end of the strings and return qtrue if strings match
============
*/
qboolean COM_CompareExtension(const char *in, const char *ext)
{
	int inlen = strlen(in);
	int extlen = strlen(ext);
	
	if(extlen <= inlen)
	{
		in += inlen - extlen;
		
		if(!Q_stricmp(in, ext))
			return qtrue;
	}
	
	return qfalse;
}


// never goes past bounds or leaves without a terminating 0
void Q_strcat( char *dest, int size, const char *src )
{
	int	l1 = strlen( dest );
	if( l1 >= size )
		fprintf( stderr, "Q_strcat: already overflowed" );

	Q_strncpyz( dest + l1, src, size - l1 );
}

/*
==================
COM_DefaultExtension

if path doesn't have an extension, then append
 the specified one (which should include the .)
==================
*/
void COM_DefaultExtension( char *path, int maxSize, const char *extension )
{
	const char *dot = strrchr(path, '.'), *slash;
	if (dot && (!(slash = strrchr(path, '/')) || slash < dot))
		return;
	else
		Q_strcat(path, maxSize, extension);
}


ID_INLINE void CopyShortSwap(void *dest, void *src)
{
	unsigned char *to = dest, *from = src;

	to[0] = from[1];
	to[1] = from[0];
}


ID_INLINE void CopyLongSwap(void *dest, void *src)
{
	unsigned char *to = dest, *from = src;

	to[0] = from[3];
	to[1] = from[2];
	to[2] = from[1];
	to[3] = from[0];
}

ID_INLINE short ShortSwap(short int l)
{
	unsigned char b1 = l & 0xFF;
    unsigned char b2 = (l>>8) & 0xFF;

	return (b1<<8) + b2;
}


ID_INLINE short ShortNoSwap(short l)
{
	return l;
}


ID_INLINE int LongSwap(int l)
{
	unsigned char b1,b2,b3,b4;

	b1 = l & 0xff;
	b2 = (l>>8) & 0xff;
	b3 = (l>>16) & 0xff;
	b4 = (l>>24) & 0xff;

	return ((b1<<24) | (b2<<16) | (b3<<8) | b4 );
}


ID_INLINE int LongNoSwap (int l)
{
	return l;
}


qint64 Long64Swap(qint64 ll)
{
	qint64	result;

	result.b0 = ll.b7;
	result.b1 = ll.b6;
	result.b2 = ll.b5;
	result.b3 = ll.b4;
	result.b4 = ll.b3;
	result.b5 = ll.b2;
	result.b6 = ll.b1;
	result.b7 = ll.b0;

	return result;
}


qint64 Long64NoSwap (qint64 ll)
{
	return ll;
}


float FloatSwap (const float *f)
{
	floatint_t out;

	out.f = *f;
	out.ui = LongSwap(out.ui);

	return out.f;
}


float FloatNoSwap (const float *f)
{
	return *f;
}


/*
============================================================================

PARSING

============================================================================
*/

static	char	com_token[MAX_TOKEN_CHARS];
static	char	com_parsename[MAX_TOKEN_CHARS];
static	int		com_lines;
static	int		com_tokenline;

void COM_BeginParseSession( const char *name )
{
	com_lines = 1;
	com_tokenline = 0;
	Com_sprintf(com_parsename, sizeof(com_parsename), "%s", name);
}

int COM_GetCurrentParseLine( void )
{
	if ( com_tokenline )
	{
		return com_tokenline;
	}

	return com_lines;
}



/*
void COM_ParseError( char *format, ... )
{
	va_list argptr;
	static char string[4096];

	va_start (argptr, format);
	Q_vsnprintf (string, sizeof(string), format, argptr);
	va_end (argptr);

	Com_Printf("ERROR: %s, line %d: %s\n", com_parsename, COM_GetCurrentParseLine(), string);
}


void COM_ParseWarning( char *format, ... )
{
	va_list argptr;
	static char string[4096];

	va_start (argptr, format);
	Q_vsnprintf (string, sizeof(string), format, argptr);
	va_end (argptr);

	Com_Printf("WARNING: %s, line %d: %s\n", com_parsename, COM_GetCurrentParseLine(), string);
}
*/

static char* SkipWhitespace(char* data, qboolean *hasNewLines)
{
	unsigned char c;

	while( (c = *data) <= ' ')
    {
		if( !c ) {
			return NULL;
		}
		if( c == '\n' )
        {
			com_lines++;
			*hasNewLines = qtrue;
		}
		data++;
	}

	return data;
}

int COM_Compress( char *data_p )
{
	int c;
	qboolean newline = qfalse, whitespace = qfalse;

	char* in = data_p;
    char* out = data_p;

	if (in)
    {
		while ((c = *in) != 0) {

			// skip double slash comments
			if ( c == '/' && in[1] == '/' ) {
				while (*in && *in != '\n') {
					in++;
				}
			// skip /* */ comments
			} else if ( c == '/' && in[1] == '*' ) {
				while ( *in && ( *in != '*' || in[1] != '/' ) ) 
					in++;
				if ( *in ) 
					in += 2;
				// record when we hit a newline
			} else if ( c == '\n' || c == '\r' ) {
				newline = qtrue;
				in++;
				// record when we hit whitespace
			} else if ( c == ' ' || c == '\t') {
				whitespace = qtrue;
				in++;
				// an actual token
			} else {
				// if we have a pending newline, emit it (and it counts as whitespace)
				if (newline) {
					*out++ = '\n';
					newline = qfalse;
					whitespace = qfalse;
				} if (whitespace) {
					*out++ = ' ';
					whitespace = qfalse;
				}

				// copy quoted strings unmolested
				if (c == '"') {
					*out++ = c;
					in++;
					while (1) {
						c = *in;
						if (c && c != '"') {
							*out++ = c;
							in++;
						} else {
							break;
						}
					}
					if (c == '"') {
						*out++ = c;
						in++;
					}
				} else {
					*out = c;
					out++;
					in++;
				}
			}
		}

		*out = 0;
	}
	return out - data_p;
}


/*
==============
COM_Parse

Parse a token out of a string
Will never return NULL, just empty strings

If "allowLineBreaks" is qtrue then an empty
string will be returned if the next token is
a newline.
==============
*/

char *COM_ParseExt( char **data_p, qboolean allowLineBreaks )
{
	int c = 0, len = 0;
	qboolean hasNewLines = qfalse;
	char *data = *data_p;
	
	com_token[0] = 0;
	com_tokenline = 0;

	// make sure incoming data is valid
	if ( *data_p == NULL)
		return com_token;


	while ( 1 )
	{
		// skip whitespace
		data = SkipWhitespace( data, &hasNewLines );
		if ( !data )
		{
			*data_p = NULL;
			return com_token;
		}
		if ( hasNewLines && !allowLineBreaks )
		{
			*data_p = data;
			return com_token;
		}

		c = *data;

		// skip double slash comments
		if ( c == '/' && data[1] == '/' )
		{
			data += 2;
			while (*data && *data != '\n') {
				data++;
			}
		}
		// skip /* */ comments
		else if ( c=='/' && data[1] == '*' ) 
		{
			data += 2;
			while ( *data && ( *data != '*' || data[1] != '/' ) ) 
			{
				if ( *data == '\n' )
				{
					com_lines++;
				}
				data++;
			}
			if ( *data ) 
			{
				data += 2;
			}
		}
		else
		{
			break;
		}
	}

	// token starts on this line
	com_tokenline = com_lines;

	// handle quoted strings
	if (c == '\"')
	{
		data++;
		while (1)
		{
			c = *data++;
			if (c=='\"' || !c)
			{
				com_token[len] = 0;
				*data_p = ( char * ) data;
				return com_token;
			}
			if ( c == '\n' )
			{
				com_lines++;
			}
			if (len < MAX_TOKEN_CHARS - 1)
			{
				com_token[len] = c;
				len++;
			}
		}
	}

	// parse a regular word
	do
	{
		if (len < MAX_TOKEN_CHARS - 1)
		{
			com_token[len] = c;
			len++;
		}
		data++;
		c = *data;
	} while (c>32);

	com_token[len] = 0;

	*data_p = ( char * ) data;
	return com_token;
}






/*
=================
SkipBracedSection

The next token should be an open brace or set depth to 1 if already parsed it.
Skips until a matching close brace is found.
Internal brace depths are properly skipped.
=================
*/
qboolean SkipBracedSection (char **program, int depth)
{
	do
    {
		char* token = COM_ParseExt( program, qtrue );
		if( token[1] == 0 )
        {
			if( token[0] == '{' )
            {
				depth++;
			}
			else if( token[0] == '}' )
            {
				depth--;
			}
		}
	} while( depth && *program );

	return ( depth == 0 );
}

/*
=================
SkipRestOfLine
=================
*/
void SkipRestOfLine ( char **data )
{
	char* p = *data;
	int	c;

	if ( !*p )
		return;

	while ( (c = *p++) != 0 )
    {
		if ( c == '\n' )
        {
			com_lines++;
			break;
		}
	}

	*data = p;
}

/*
==================
COM_MatchToken
==================
*/
void COM_MatchToken( char **buf_p, char *match )
{
	char* token = COM_ParseExt( buf_p, qtrue );
	if ( strcmp( token, match ) )
		fprintf( stderr, "MatchToken: %s != %s", token, match );
}

void Parse1DMatrix (char **buf_p, int x, float *m)
{
	int		i;

	COM_MatchToken( buf_p, "(" );

	for (i = 0 ; i < x ; i++)
    {
		char* token = COM_ParseExt(buf_p, qtrue);
		m[i] = atof(token);
	}

	COM_MatchToken( buf_p, ")" );
}

void Parse2DMatrix (char **buf_p, int y, int x, float *m) {
	int		i;

	COM_MatchToken( buf_p, "(" );

	for (i = 0 ; i < y ; i++) {
		Parse1DMatrix (buf_p, x, m + i * x);
	}

	COM_MatchToken( buf_p, ")" );
}

void Parse3DMatrix (char **buf_p, int z, int y, int x, float *m) {
	int		i;

	COM_MatchToken( buf_p, "(" );

	for (i = 0 ; i < z ; i++) {
		Parse2DMatrix (buf_p, y, x, m + i * x*y);
	}

	COM_MatchToken( buf_p, ")" );
}

/*
===================
Com_HexStrToInt
===================
*/
int Com_HexStrToInt( const char *str )
{
	if ( !str || !str[ 0 ] )
		return -1;

	// check for hex code
	if( str[ 0 ] == '0' && str[ 1 ] == 'x' )
	{
		int i, n = 0;

		for( i = 2; i < strlen( str ); i++ )
		{
			char digit;

			n *= 16;

			digit = tolower( str[ i ] );

			if( digit >= '0' && digit <= '9' )
				digit -= '0';
			else if( digit >= 'a' && digit <= 'f' )
				digit = digit - 'a' + 10;
			else
				return -1;

			n += digit;
		}

		return n;
	}

	return -1;
}


/*
============================================================================

					LIBRARY REPLACEMENT FUNCTIONS

============================================================================
*/

int Q_isprint( int c )
{
	if ( c >= 0x20 && c <= 0x7E )
		return ( 1 );
	return ( 0 );
}

int Q_islower( int c )
{
	if (c >= 'a' && c <= 'z')
		return ( 1 );
	return ( 0 );
}

int Q_isupper( int c )
{
	if (c >= 'A' && c <= 'Z')
		return ( 1 );
	return ( 0 );
}

int Q_isalpha( int c )
{
	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
		return ( 1 );
	return ( 0 );
}

qboolean Q_isanumber( const char *s )
{
	char *p;
	double UNUSED_VAR d;

	if( *s == '\0' )
		return qfalse;

	d = strtod( s, &p );

	return *p == '\0';
}

qboolean Q_isintegral( float f )
{
	return (int)f == f;
}

#ifdef _MSC_VER
/*
=============
Q_vsnprintf
 
Special wrapper function for Microsoft's broken _vsnprintf() function.
MinGW comes with its own snprintf() which is not broken.
=============
*/

int Q_vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
	int retval = _vsnprintf(str, size, format, ap);

	if(retval < 0 || retval == size)
	{
		// Microsoft doesn't adhere to the C99 standard of vsnprintf,
		// which states that the return value must be the number of bytes written if the output string had sufficient length.
		// Obviously we cannot determine that value from Microsoft's implementation, so we have no choice but to return size.
		
		str[size - 1] = '\0';
		return size;
	}
	
	return retval;
}
#endif

/*
 * Safe strncpy that ensures a trailing zero
 */
ID_INLINE void Q_strncpyz( char *dest, const char *src, int destsize )
{
/*    
    if( !dest )
        Com_Error( ERR_FATAL, "Q_strncpyz: NULL dest" );

	if( !src )
		Com_Error( ERR_FATAL, "Q_strncpyz: NULL src" );

	if ( destsize < 1 )
		Com_Error(ERR_FATAL,"Q_strncpyz: destsize < 1" ); 
*/
	strncpy(dest, src, destsize-1);
    dest[destsize-1] = 0;
}


int Q_stricmpn(const char *s1, const char *s2, int n)
{
	int c1, c2;

    if( s1 == NULL )
    {
        if( s2 == NULL )
             return 0;
        else
             return -1;
    }
    else if ( s2 == NULL )
	{
        return 1;
	}

	do
	{
		c1 = *s1++;
		c2 = *s2++;

		if (!n--)
			return 0;		// strings are equal until end point

		if(c1 >= 'a' && c1 <= 'z')
			c1 -= ('a' - 'A');
				
		if(c2 >= 'a' && c2 <= 'z')
			c2 -= ('a' - 'A');

		if(c1 != c2) 
			return c1 < c2 ? -1 : 1;
   } while (c1);
	
	return 0;		// strings are equal
}

/*
static int Q_strncmp(const char *s1, const char *s2, int n)
{
	int	c1, c2;
	
	do {
		c1 = *s1++;
		c2 = *s2++;

		if (!n--) {
			return 0;		// strings are equal until end point
		}
		
		if (c1 != c2) {
			return c1 < c2 ? -1 : 1;
		}
	} while (c1);
	
	return 0;		// strings are equal
}
*/

ID_INLINE int Q_stricmp(const char *s1, const char *s2)
{
	//return (s1 && s2) ? Q_stricmpn (s1, s2, 99999) : -1;
    #ifdef _MSC_VER
        return stricmp(s1, s2);
    #else
        return strcasecmp(s1, s2);
    #endif
}


char *Q_strlwr( char *s1 )
{
    char *s = s1;
	while ( *s ) {
		*s = tolower(*s);
		s++;
	}
    return s1;
}

char *Q_strupr( char *s1 ) {
    char* s = s1;
	while( *s )
    {
		*s = toupper(*s);
		s++;
	}
    return s1;
}




/*
 * Find the first occurrence of find in s.
 */
const char *Q_stristr( const char *s, const char *find)
{
  char c, sc;
  size_t len;

  if ((c = *find++) != 0)
  {
    if (c >= 'a' && c <= 'z')
    {
      c -= ('a' - 'A');
    }
    len = strlen(find);
    do
    {
      do
      {
        if ((sc = *s++) == 0)
          return NULL;
        if (sc >= 'a' && sc <= 'z')
        {
          sc -= ('a' - 'A');
        }
      } while (sc != c);
    } while (Q_stricmpn(s, find, len) != 0);
    s--;
  }
  return s;
}


int Q_PrintStrlen( const char *string )
{
	int	len = 0;
	const char *p = string;

	if( !p )
		return 0;

	while( *p )
    {
		if( Q_IsColorString( p ) )
        {
			p += 2;
			continue;
		}
		p++;
		len++;
	}

	return len;
}


char *Q_CleanStr( char *string )
{
	char* d = string;
	char* s = string;
	int	c;

	while ((c = *s) != 0 )
    {
		if ( Q_IsColorString( s ) )
        {
			s++;
		}		
		else if ( c >= 0x20 && c <= 0x7E )
        {
			*d++ = c;
		}
		s++;
	}
	*d = '\0';

	return string;
}

int Q_CountChar(const char *string, char tocount)
{
	int count = 0;
	
	for( ; *string; string++)
	{
		if(*string == tocount)
			count++;
	}
	
	return count;
}


int QDECL Com_sprintf(char *dest, int size, const char *fmt, ...)
{
	va_list	argptr;

	va_start (argptr, fmt);
	int len = Q_vsnprintf(dest, size, fmt, argptr);
	va_end (argptr);

	if(len >= size)
		fprintf(stderr, "Com_sprintf: Output length %d too short, require %d bytes.\n", size, len + 1);
	
	return len;
}

/*
 * does a varargs printf into a temp buffer, so I don't need to have varargs versions of all text functions.
 */
char* QDECL va(char *format, ... )
{
	va_list		argptr;
	static char string[2][32000]; // in case va is called by nested functions
	static int	index = 0;
    
	char *buf = string[index & 1];
	index++;

	va_start(argptr, format);
	Q_vsnprintf(buf, sizeof(*string), format, argptr);
	va_end(argptr);

	return buf;
}

/*
============
Com_TruncateLongString

Assumes buffer is atleast TRUNCATE_LENGTH big
============
*/
void Com_TruncateLongString( char *buffer, const char *s )
{
	int length = strlen( s );

	if( length <= TRUNCATE_LENGTH )
		Q_strncpyz( buffer, s, TRUNCATE_LENGTH );
	else
	{
		Q_strncpyz( buffer, s, ( TRUNCATE_LENGTH / 2 ) - 3 );
		Q_strcat( buffer, TRUNCATE_LENGTH, " ... " );
		Q_strcat( buffer, TRUNCATE_LENGTH, s + length - ( TRUNCATE_LENGTH / 2 ) + 3 );
	}
}



/*
==================
Info_Validate

Some characters are illegal in info strings because they can mess up the server's parsing
==================

qboolean Info_Validate( const char *s )
{
	if ( strchr( s, '\"' ) )
		return qfalse;

	if ( strchr( s, ';' ) )
		return qfalse;

	return qtrue;
}
*/







//====================================================================

/*
==================
Com_CharIsOneOfCharset
==================
*/
static qboolean Com_CharIsOneOfCharset( char c, char *set )
{
	int i;

	for( i = 0; i < strlen( set ); i++ )
	{
		if( set[ i ] == c )
			return qtrue;
	}

	return qfalse;
}

/*
==================
Com_SkipCharset
==================
*/
char *Com_SkipCharset( char *s, char *sep )
{
	char	*p = s;

	while( p )
	{
		if( Com_CharIsOneOfCharset( *p, sep ) )
			p++;
		else
			break;
	}

	return p;
}


char *Com_SkipTokens( char *s, int numTokens, char *sep )
{
	int		sepCount = 0;
	char	*p = s;

	while( sepCount < numTokens )
	{
		if( Com_CharIsOneOfCharset( *p++, sep ) )
		{
			sepCount++;
			while( Com_CharIsOneOfCharset( *p, sep ) )
				p++;
		}
		else if( *p == '\0' )
			break;
	}

	if( sepCount == numTokens )
		return p;
	else
		return s;
}

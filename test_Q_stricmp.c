/*
 * =====================================================================================
 *
 *       Filename:  test_Q_stricmp.c
 *
 *    Description:  
 * =====================================================================================
 */
#include <stdio.h>

int Q_stricmpn(const char *s1, const char *s2, int n)
{
	int	c1, c2;

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




int Q_stricmp(const char *s1, const char *s2)
{
	return (s1 && s2) ? Q_stricmpn (s1, s2, 99999) : -1;
/*
    #ifdef _MSC_VER
        return stricmp(s1, s2);
    #else
        return strcasecmp(s1, s2);
    #endif
*/	
}

// retunrn s1 == s2 ? 1 : 0

int isCaseStringEqual(const char * s1, const char * s2)
{
    if( s1 == NULL || s2 == NULL)
    {
        // should not compare null, this is not meaningful ...
        return 0;
    }
    else
    {
        while( (*s1 != 0) || (*s2 != 0) )
        {
            if(*s1++ == *s2++)
            {
                continue;
            }
            else
            {
                // string are not equal
                return 0;
            }
        }
    }
    return 1;
}

int main()
{
    char * pToken1 = "autosprite";
    char * pToken2 = "autosprite2";

/*
	if ( !Q_stricmp( pToken1, "autosprite" ) ) {
		printf( "%s === DEFORM_AUTOSPRITE \n", pToken1);
	}

	if ( !Q_stricmp( pToken1, "autosprite2" ) ) {
		printf( "%s === DEFORM_AUTOSPRITE2 \n", pToken1);
	}

	if ( !Q_stricmp( pToken2, "autosprite" ) ) {
		printf( "%s === DEFORM_AUTOSPRITE \n", pToken2);
	}

	if ( !Q_stricmp( pToken2, "autosprite2" ) ) {
		printf( "%s === DEFORM_AUTOSPRITE2 \n", pToken2);
	}
*/

	if ( isCaseStringEqual( pToken1, "autosprite" ) ) {
		printf( "%s === DEFORM_AUTOSPRITE \n", pToken1);
	}

	if ( isCaseStringEqual( pToken1, "autosprite2" ) ) {
		printf( "%s === DEFORM_AUTOSPRITE2 \n", pToken1);
	}

	if ( isCaseStringEqual( pToken2, "autosprite" ) ) {
		printf( "%s === DEFORM_AUTOSPRITE \n", pToken2);
	}

	if ( isCaseStringEqual( pToken2, "autosprite2" ) ) {
		printf( "%s === DEFORM_AUTOSPRITE2 \n", pToken2);
	}

    return 0;
}

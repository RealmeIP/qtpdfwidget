
#include <stdlib.h>
#include <string.h>
//vishwajeet #include <unistd.h>
#include <stdio.h>
//vishwajeet #include <dirent.h>
#include <fcntl.h>

//#ifndef EKA2
//#include <io.h>
//#endif
#if defined(WIN32)
#include <io.h>
#endif


#include <gmem.h>

#include <gfile.h>
#include <GString.h>

#include <qdir.h>


// replacement stubs to simplify (and speed up) operations

void *gmalloc ( int size )                   { return malloc ( size ); }
void *grealloc ( void *p, int size )         { return realloc ( p, size ); }
void gfree ( void *p )                       { free ( p ); }
char *copyString ( char *str )               { return strdup ( str ); }
char *getLine(char *buf, int size, FILE *f)  { return fgets ( buf, size, f ); }

// TODO VCD "IS THIS SHOULD BE HERE ???????"
GString *getHomeDir ( )                      { return new GString ( QDir::home ( ).absolutePath( ).toLocal8Bit( )); }
GString *appendToPath ( GString *path, char *fileName )   { return new GString ( QDir ( path-> getCString ( )). absoluteFilePath( fileName ).toLocal8Bit ( )); }

// mostly taken from XPDF, but simplified...

GBool openTempFile ( GString **name, FILE **f, char *mode, char *ext )
{
	char *s, *p;
	int fd;
	
	if ( !ext ) 
		ext = ".tmp";
	
	if (!( s = tmpnam ( 0 ))) 
		return gFalse;
			
	*name = new GString ( s );

	s = (*name)-> getCString ( );
	if (( p = strrchr ( s, '.' ))) 
		(*name)-> del ( p - s, (*name)-> getLength ( ) - ( p - s ));
	
	(*name)-> append ( "_qpdf" );
	(*name)-> append ( ext );
	
	fd = open ((*name)-> getCString ( ), O_WRONLY | O_CREAT | O_EXCL, 0600 );
	
	if ( fd < 0 || !( *f = fdopen ( fd, mode ))) {
		delete *name;
		return gFalse;
	}
	
	return gTrue;	
}

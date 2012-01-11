//========================================================================
//
// UTF8.h
//
// Copyright 2001 Derek B. Noonburg
// Modified for QPE by Robert Griebl
//
//========================================================================

 #if QT_VERSION >= 0x040100
 #warning "Vishwajeet : Fixed Me"
     #include <QString>
     #include <QByteArray>	
#define QCString QByteArray

 #else
     #include <qstring.h>
#endif 

#include <string.h>

static int mapUTF8 ( Unicode u, char *buf, int bufSize ) 
{
 #if QT_VERSION >= 0x040100
// #warning "Vishwajeet : Fixed Me"
	QByteArray utf = QString ( QChar ( u )). toUtf8 ( );
#else
	QCString utf = QString ( QChar ( u )). utf8 ( );
#endif
	int len = utf. length ( );
	
	if ( len <= bufSize ) {
		::memcpy ( buf, utf. data ( ), len );
		return len;
	}
	else
		return 0;
}

static int mapUCS2 ( Unicode u, char *buf, int bufSize) 
{
	if (u <= 0xffff) {
		if (bufSize < 2)
			return 0;

		buf[0] = (char)((u >> 8) & 0xff);
		buf[1] = (char)(u & 0xff);
		return 2;
	} 
	else 
		return 0;
}

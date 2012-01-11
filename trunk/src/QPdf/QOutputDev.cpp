///========================================================================
//
// QOutputDev.cc
//
// Copyright 1996 Derek B. Noonburg
// CopyRight 2002 Robert Griebl
//
//========================================================================

#ifdef __GNUC__
#pragma implementation
#endif

#include <aconf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
//vishwajeet #include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#ifndef EKA2
#include <iostream>
#endif

#include "GString.h"
#include "Object.h"
#include "Stream.h"
#include "Link.h"
#include "GfxState.h"
#include "GfxFont.h"
#include "UnicodeMap.h"
#include "CharCodeToUnicode.h"
#include "FontFile.h"
#include "Error.h"
#include "TextOutputDev.h"
#include "QOutputDev.h"



#if QT_VERSION >= 0x040100
//#warning "Vishwajeet : Fix Me" 

#include <QPixmap>
#include <QImage>
#include <QPainter>
#include <QHash>
#include <QTimer>
#include <QApplication>
#include <QClipboard>
#include <QtGlobal>
#include <QFontDatabase>

#define QDict QHash
#define white Qt::white
#define blue Qt::blue
#define FlatCap Qt::FlatCap
#define RoundCap Qt::RoundCap
#define SquareCap Qt::SquareCap
#define MiterJoin Qt::MiterJoin
#define RoundJoin Qt::RoundJoin
#define BevelJoin Qt::BevelJoin
#define DashLine Qt::DashLine
#define SolidLine Qt::SolidLine
#define NoPen Qt::NoPen

// By VCD RISK {
#define QT_NO_TRANSFORMATIONS 1
// }

//by vishwajeet
#ifdef EKA2
#define QT_NO_TRANSFORMATIONS 0
#endif

#define QMAX qMax

#define printf qDebug
#else

#include <qpixmap.h>
#include <qimage.h>
#include <qpainter.h>
#include <qdict.h>
#include <qtimer.h>
#include <qapplication.h>
#include <qclipboard.h>


#endif


//#define QPDFDBG(x) x		// special debug mode
#define QPDFDBG(x)   		// normal compilation


//------------------------------------------------------------------------
// Constants and macros
//------------------------------------------------------------------------


static inline QColor q_col ( const GfxRGB &rgb )
{
	return QColor ( lrint ( rgb. r * 255 ), lrint ( rgb. g * 255 ), lrint ( rgb. b * 255 ));
}


//------------------------------------------------------------------------
// Font substitutions
//------------------------------------------------------------------------

struct QOutFontSubst {
	char * m_name;
	char * m_sname;
	bool   m_bold;
	bool   m_italic;
	QFont::StyleHint m_hint;
};

static QOutFontSubst qStdFonts [] = {
	{ "Helvetica",             "Helvetica", false, false, QFont::Helvetica },
	{ "Helvetica-Oblique",     "Helvetica", false, true,  QFont::Helvetica },
	{ "Helvetica-Bold",        "Helvetica", true,  false, QFont::Helvetica },
	{ "Helvetica-BoldOblique", "Helvetica", true,  true,  QFont::Helvetica },
	{ "Times-Roman",           "Times",     false, false, QFont::Times },
	{ "Times-Italic",          "Times",     false, true,  QFont::Times },
	{ "Times-Bold",            "Times",     true,  false, QFont::Times },
	{ "Times-BoldItalic",      "Times",     true,  true,  QFont::Times },
	{ "Courier",               "Courier",   false, false, QFont::Courier },
	{ "Courier-Oblique",       "Courier",   false, true,  QFont::Courier },
	{ "Courier-Bold",          "Courier",   true,  false, QFont::Courier },
	{ "Courier-BoldOblique",   "Courier",   true,  true,  QFont::Courier },

	{ "Symbol",                0,           false, false, QFont::AnyStyle },
	{ "Zapf-Dingbats",         0,           false, false, QFont::AnyStyle },

	{ 0,                       0,           false, false, QFont::AnyStyle }
};







QFont QOutputDev::matchFont ( GfxFont *gfxFont, fp_t m11, fp_t m12, fp_t m21, fp_t m22 )
{

	/*
	int qpdf_debug_level = 10;
	static QDict<QString,QOutFontSubst *> stdfonts;

	fouble fsize = sqrt ( m21 * m21 + m22 * m22 );
	
	// this function needs to be rewritten:
	// use cache of fontDB stuff
	// is the avg_size stuff required?

	if (fsize <= 0.0) {
	  m21 *= fouble(1.0/36.0);
	  m22 *= fouble(1.0/36.0);
	  fsize = fouble(6.0) * sqrt( m21 * m21 + m22 * m22 );
	  if (fsize <= 0.0) {
	    printf("Bad font size %f %f = %f\n", (float)m21, (float)m22, (float)fsize);
	    fsize = 50.0;
	  }
	}

	if (qpdf_debug_level > 5 || fsize < 0.01) {
	  printf("matchFont matrix %f %f %f %f\n",
		 (float) m11, (float) m12,(float) m21, (float) m22);
	}

	m_good_font_match = true;

	// build dict for std. fonts on first invocation	
	if ( stdfonts. isEmpty ( )) {
		for ( QOutFontSubst *ptr = qStdFonts; ptr-> m_name; ptr++ ) {
			stdfonts[ QString ( ptr-> m_name )] = ptr ;
		}	
	}

	int avg_size = 0;

	if (! gfxFont->isCIDFont()) {
	  avg_size = lrint((fouble)fsize * 
			   ( ((Gfx8BitFont*)gfxFont)->getWidth('e')
			     + ((Gfx8BitFont*)gfxFont)->getWidth('t')
			     + ((Gfx8BitFont*)gfxFont)->getWidth('a')
			     + ((Gfx8BitFont*)gfxFont)->getWidth('o')
			     + ((Gfx8BitFont*)gfxFont)->getWidth('i')
			     + ((Gfx8BitFont*)gfxFont)->getWidth('r')
			     + ((Gfx8BitFont*)gfxFont)->getWidth('n') ) );
	  if (qpdf_debug_level > 4) {
	    printf("8BitFont fsize=%f avg_size=%f\n", (float)fsize, (float)avg_size);
	  }
	} else {
	  // 0.55 too big, 0.50 too big, 0.45 sort of works
	  avg_size = lrint((fouble)fsize*7.0*0.35);
	  if (qpdf_debug_level > 0) {
	    printf("CIDFont fsize=%f avg_size=%f\n", (float)fsize, (float)avg_size);
	  }
	}

	QString fname ((gfxFont->getName() && gfxFont->getName()->getCString()) ? 
		       gfxFont-> getName ( )-> getCString ( ) : "<n/a>" );
	QFont f;
	// fast lookup for std. fonts
	QOutFontSubst *subst = stdfonts [fname];

	if ( subst ) {
	  if ( subst-> m_sname )
	    f. setFamily ( subst-> m_sname );
	  f. setStyleHint ( subst-> m_hint, (QFont::StyleStrategy) ( QFont::PreferOutline | QFont::PreferQuality ));	
	  f. setBold ( subst-> m_bold );
	  f. setItalic ( subst-> m_italic );
	} else {
	  QFont::StyleHint sty;
	  
	  if ( gfxFont-> isSerif ( ))
	    sty = QFont::Serif;
	  else if ( gfxFont-> isFixedWidth ( ))
	    sty = QFont::TypeWriter;
	  else
	    sty = QFont::Helvetica;
	  
	  f. setStyleHint ( sty, (QFont::StyleStrategy) ( QFont::PreferOutline | QFont::PreferQuality ));	
	  f. setBold ( gfxFont-> isBold ( ) > 0 );
	  f. setItalic ( gfxFont-> isItalic ( ) > 0 );
	  f. setFixedPitch ( gfxFont-> isFixedWidth ( ) > 0 );
	  
	  // common specifiers in font names 
	  if ( fname. contains ( "Oblique" ) || fname. contains ( "Italic" ))
	    f. setItalic ( true ); 
	  if ( fname. contains ( "Bold" ))
	    f. setWeight ( QFont::Bold );
	  if ( fname. contains ( "Demi" ))
	    f. setWeight ( QFont::DemiBold );
	  if ( fname. contains ( "Light" ))
	    f. setWeight ( QFont::Light );
	  if ( fname. contains ( "Black" ))
	    f. setWeight ( QFont::Black );
	}		
	// Treat x-sheared fonts as italic
	if (( m12 > -0.1 ) && ( m12 < 0.1 ) && ((( m21 > -5.0 ) && ( m21 < -0.1 )) || (( m21 > 0.1 ) && ( m21 < 5.0 )))) {
	  f. setItalic ( true ); 
	}	
	
	if (fsize < 2) {
	  if (qpdf_debug_level > 0) {
	    printf("Font size = %f (%f %f %f %f)\n", 
		   (float)fsize, (float)m11, (float)m12,
		   (float)m21, (float)m22);
	  }
	  fsize = 2; // type3 fonts misbehave sometimes  (???)
	}

	if (!m_reproportion_text) {
	  if (fsize > 128) {  // something in Qt seems to break
	    fsize = 128;
	  }
	  f.setPixelSize ( lrint(fsize) );
	  m_painter-> setFont ( f );

	} else {


	  // increase app startup speed, but pay the price one first adjust spacing
	   // Commented by Vishwajeet	  	
	  if (m_sys_font_database.count() == 0) {
	   cacheSysFontDatabase();
	   }

	  // this section needs to be rewritten
	  // it should use a cached array of font data from QFontDatabase

	  int best_avg_size;
	  int best_fit;
	  int best_font_index = -1;

	  best_avg_size = 100000;
	  best_fit = 100000;

	  // some fonts just have no char widths
	  if (avg_size < 1.0) {
	    // 0.6 was too big
	    // was 0.45 - worked, 0.45, 0.48 & 0.55 were a tiny bit too big
	    if (qpdf_debug_level > 0) {
	      printf("font avg_size = %f, fsize=%f, new avg_size=", (float)avg_size,
		     (float)fsize);
	    }
	    avg_size = 7.0 * 0.43 * fsize;
	    if (qpdf_debug_level > 0) {
	      printf("%f\n", (float)avg_size);
	    }
	  }

	  for(int retry=0; retry<=1; retry++) {
	    for(int font_index=0; font_index<m_sys_font_database.count(); font_index++) {
	      SysFontCacheEntry *font_entry;
	      
	      font_entry = m_sys_font_database[font_index];
	      if (retry == 0
		  && (font_entry->font.bold() != f.bold()
		      || font_entry->font.italic() != f.italic())) {
		continue;
	      }
	      
	      if (
		  // find closest match smaller than target = avg_size
		  (font_entry->avg_size <= avg_size
		   && qAbs(font_entry->avg_size - avg_size) < best_fit)
		  ||
		  // if no smaller match, look for smallest larger match
		  (best_avg_size > avg_size
		   && font_entry->avg_size < best_avg_size)
		  ) {
		best_font_index = font_index;
		best_avg_size = font_entry->avg_size;
		best_fit = qAbs(font_entry->avg_size - avg_size);
	      }
#if 0
	      printf("try %d avg_size %d font_index %d avg_size %d best_index %d best_avg_size %d best_fit %d\n",
		     retry, avg_size, font_index, font_entry->avg_size,
		     best_font_index, best_avg_size, best_fit);
#endif
	    }

	    if (best_avg_size <= avg_size) {
	      break;
	    }
	  }
	  if (best_avg_size > avg_size + 1) {
	    m_good_font_match = false;
	  }
	  //	  assert(best_font_index >= 0);
	  m_painter-> setFont ( m_sys_font_database[best_font_index]->font );
	}
	
	if (qpdf_debug_level > 0) {
	  //	  QFontMetrics fm(m_painter->fontMetrics());
	  printf("Font ");
//	  printFontDescription(gfxFont, fsize);
	  printf ( "size %.2f got size %.2f %s%d/%d%s%s%s%s\n",
		   (float)(avg_size/7.0),
		   (float)(m_painter->fontMetrics().width("etaoinr")/7.0),
		   m_painter->font().family().toAscii(), 
		   m_painter->font().pointSize(), 
		   m_painter->font().weight(), 
		   m_painter->font().bold()?" bold ":"", 
		   m_painter->font().italic()? " ital ":"", 
		   m_painter->font().underline()? " under ":"",
		   m_painter->font().strikeOut()? " strikeout ":"");
	}
*/
	

	static QDict<QString,QOutFontSubst *> stdfonts;

	// build dict for std. fonts on first invocation
	if ( stdfonts. isEmpty ( )) {
		for ( QOutFontSubst *ptr = qStdFonts; ptr-> m_name; ptr++ ) {
			stdfonts[QString ( ptr-> m_name )] =  ptr ;
		}
	}

	// compute size and normalized transform matrix
	int size = lrint ( sqrt ( m21 * m21 + m22 * m22 ));

	QPDFDBG( printf ( "SET FONT: Name=%s, Size=%d, Bold=%d, Italic=%d, Mono=%d, Serif=%d, Symbol=%d, CID=%d, EmbFN=%s, M=(%f,%f,%f,%f)\n",
	         (( gfxFont-> getName ( )) ? gfxFont-> getName ( )-> getCString ( ) : "<n/a>" ),
	         size,
	         gfxFont-> isBold ( ),
	         gfxFont-> isItalic ( ),
	         gfxFont-> isFixedWidth ( ),
	         gfxFont-> isSerif ( ),
	         gfxFont-> isSymbolic ( ),
	         gfxFont-> isCIDFont ( ),
	         ( gfxFont-> getEmbeddedFontName ( ) ? gfxFont-> getEmbeddedFontName()-> getCString ( ) : "<n/a>" ),
                          (double) m11, (double) m12, (double) m21, (double) m22 ) );


	/*	qDebug ( "SET FONT: Name=%s, Size=%d, Bold=%d, Italic=%d, Mono=%d, Serif=%d, Symbol=%d, CID=%d, EmbFN=%s, M=(%f,%f,%f,%f)\n",
	         (( gfxFont-> getName ( )) ? gfxFont-> getName ( )-> getCString ( ) : "<n/a>" ),
	         size,
	         gfxFont-> isBold ( ),
	         gfxFont-> isItalic ( ),
	         gfxFont-> isFixedWidth ( ),
	         gfxFont-> isSerif ( ),
	         gfxFont-> isSymbolic ( ),
	         gfxFont-> isCIDFont ( ),
	         ( gfxFont-> getEmbeddedFontName ( ) ? gfxFont-> getEmbeddedFontName()-> getCString ( ) : "<n/a>" ),
                          (double) m11, (double) m12, (double) m21, (double) m22 ) ;
*/

	QString fontName(gfxFont-> getName ( )->getCString());

	QString fname (( gfxFont-> getName ( )) ? gfxFont-> getName ( )-> getCString ( ) : "<n/a>" );

	QFont f;
	f. setPixelSize ( size > 0 ? size : 8 ); // type3 fonts misbehave sometimes

	f.setFamily("Times");

	

	// fast lookup for std. fonts
	QOutFontSubst *subst = stdfonts [fname];

	if ( subst ) {
		if ( subst-> m_sname )
			f. setFamily ( subst-> m_sname );
		f. setStyleHint ( subst-> m_hint, (QFont::StyleStrategy) ( QFont::PreferOutline | QFont::PreferQuality ));
		f. setBold ( subst-> m_bold );
		f. setItalic ( subst-> m_italic );
	}
	else {
		QFont::StyleHint sty;

		if ( gfxFont-> isSerif ( ))
			sty = QFont::Serif;
		else if ( gfxFont-> isFixedWidth ( ))
			sty = QFont::TypeWriter;
		else
			sty = QFont::Helvetica;

		f. setStyleHint ( sty, (QFont::StyleStrategy) ( QFont::PreferOutline | QFont::PreferQuality ));
		f. setBold ( gfxFont-> isBold ( ) > 0 );
		f. setItalic ( gfxFont-> isItalic ( ) > 0 );
		f. setFixedPitch ( gfxFont-> isFixedWidth ( ) > 0 );

		// common specifiers in font names
		if ( fname. contains ( "Oblique" ) || fname. contains ( "Italic" ))
			f. setItalic ( true );
		if ( fname. contains ( "Bold" ))
			f. setWeight ( QFont::Bold );
		if ( fname. contains ( "Demi" ))
			f. setWeight ( QFont::DemiBold );
		if ( fname. contains ( "Light" ))
			f. setWeight ( QFont::Light );
		if ( fname. contains ( "Black" ))
			f. setWeight ( QFont::Black );
	}
	// Treat x-sheared fonts as italic
	if (( m12 > -0.1 ) && ( m12 < 0.1 ) && ((( m21 > -5.0 ) && ( m21 < -0.1 )) || (( m21 > 0.1 ) && ( m21 < 5.0 )))) {
		f. setItalic ( true );
	}



//	qDebug("FONT :%s",f.toString().toAscii());
	return f;
}

void
QOutputDev::cacheSysFontDatabase()
{
  // this code causes repeated access to all the font files
  // which is not slow
  QFontDatabase fontDB;
  QStringList family_names;
  SysFontCacheEntry *font_entry;
  QFont f;

  family_names = fontDB.families( QFontDatabase::Any );
  for(int family_index = 0; family_index < family_names.count(); family_index++) {

//    printf("FAMILY %s STYLES = %s  \n",
//		family_names[family_index].toLatin1(),
//	   fontDB.styles(family_names[family_index]).join(":").toLatin1());

    QStringList family_styles = fontDB.styles(family_names[family_index]);
    for(int style_index = 0; style_index < family_styles.count(); style_index++) {
      QList<int> family_sizes = fontDB.pointSizes( family_names[family_index],
							family_styles[style_index]);
      if (family_sizes.count() > 0) {
	for(int size_index = 0; size_index < family_sizes.count(); size_index++) { 
	  int size;
	  size = family_sizes[size_index];
#ifndef __i386__
	  // a hack, sizes on Z seem to be points*10
	  size /= 10;
#endif
	  f = QFont(family_names[family_index], size, 
		    fontDB.weight(family_names[family_index], family_styles[style_index]),
		    fontDB.italic(family_names[family_index], family_styles[style_index]));
#if 1
	  QFontMetrics fm(f);
#else
	  QPainter painter;
	  painter.setFont(f);
	  QFontMetrics fm = painter.fontMetrics();
#endif
	  
	  font_entry = new SysFontCacheEntry();
	  //	if (fm.width("l") == fm.width("m")) {
	  //	  // don't trust f.fixedPitch() or f.exactMatch(), they lie!!
	  //	  font_entry->fixed = true;
	  //	}
	  font_entry->font = f;
	  //	font_entry->name = family_names[family_index];
	  //	font_entry->size = family_sizes[size_index];
	  //	font_entry->pixel_size = f.pixelSize();
	  font_entry->avg_size = fm.width("etaoinr");
//	  printf("FONT %s[%d] pixel size %d avg = %f bold %d italic %d\n",
//		 family_names[family_index].toLatin1(),
//		 family_sizes[size_index],
//		 f.pixelSize(),
//		 (float)font_entry->avg_size/ 7.0, f.bold(), f.italic());
	  m_sys_font_database.append(font_entry);
	}
      }
    }
  }
}


//------------------------------------------------------------------------
// QOutputDev
//------------------------------------------------------------------------

#if QT_VERSION >= 0x040100
//#warning "Vishwajeet : Fix Me" 
QOutputDev::QOutputDev ( QWidget *parent, const char *name, Qt::WindowFlags flags ) : QScrollView ( parent)
#else
QOutputDev::QOutputDev ( QWidget *parent, const char *name, int flags ) : QScrollView ( parent, name, WRepaintNoErase | WResizeNoErase | flags )
#endif
{


	m_pixmap = 0;
	m_painter = 0;

	#if QSCROLLAREA_SUPPORT
		setViewport(0);
	#endif
	setWindowFlags(flags );

	// create text object
	m_text = new TextPage ( gFalse );
}

QOutputDev::~QOutputDev ( )
{
	delete m_painter;
	delete m_pixmap;
	delete m_text;
}


void QOutputDev::startPage ( int /*pageNum*/, GfxState *state )
{
	// Added by VCD {
	 delete m_painter;
	 delete m_pixmap;
	//QPainter *deleteLaterPainter = m_painter;
	//QPixmap *deleteLaterPixmap = m_pixmap;
	// }

	m_pixmap = new QPixmap ( lrint ( state-> getPageWidth ( )), lrint ( state-> getPageHeight ( )));
	m_painter = new QPainter ( m_pixmap );

	QPDFDBG( printf ( "NEW PIXMAP (%ld x %ld)\n", lrint ( state-> getPageWidth ( )),  lrint ( state-> getPageHeight ( ))));

	#if QT_VERSION >= 0x040100
	resize(m_pixmap-> width ( ), m_pixmap-> height ( ));
	//viewport()->setContentsMargins(0,0,0,0);

	#else
	resizeContents ( m_pixmap-> width ( ), m_pixmap-> height ( ));
	setContentsPos ( 0, 0 );
	#endif
	
	
	m_pixmap-> fill ( white ); // clear window
	m_text-> clear ( ); // cleat text object

	#if QSCROLLAREA_SUPPORT
		viewport ( )-> repaint( );
	#endif

	// Added by VCD {
	//delete deleteLaterPainter;
	//delete deleteLaterPixmap;
	// }
}

void QOutputDev::endPage ( )
{
        QPDFDBG( printf("End page\n") );
	m_text-> coalesce ( );

        /*
         * I get stupid crashes after endPage is called and then we do clipping
         * and other stuff.....
         */
#if 0
	delete m_painter;
	m_painter = 0;
#endif

	#if QT_VERSION >= 0x040100
	update();
	#else
	updateContents ( 0, 0, contentsWidth ( ), contentsHeight ( ));
	#endif
}

void QOutputDev::drawLink ( Link *link, Catalog */*catalog*/ )
{
	fp_t x1, y1, x2, y2, w;

	link-> getBorder ( &x1, &y1, &x2, &y2, &w );

	if ( w > 0 ) {
		int x, y, dx, dy;

		cvtUserToDev ( x1, y1, &x,  &y );
		cvtUserToDev ( x2, y2, &dx, &dy );

		QPen oldpen = m_painter-> pen ( );
		m_painter-> setPen ( blue );
		m_painter-> drawRect ( x, y, dx, dy );
		m_painter-> setPen ( oldpen );
	}
}

void QOutputDev::saveState ( GfxState */*state*/ )
{
        if ( ! m_painter )
            return;

	QPDFDBG( printf ( "SAVE (CLIP=%d/%d)\n",  m_painter-> hasClipping ( ), !m_painter-> clipRegion ( ). isEmpty ( )));

        m_painter-> save ( );
}

void QOutputDev::restoreState ( GfxState */*state*/ )
{
	if( ! m_painter )
	    return;

	m_painter-> restore ( );

//	m_painter-> setClipRegion ( QRect ( 0, 0, m_pixmap-> width ( ), m_pixmap-> height ( )));
//	m_painter-> setClipping ( false );
	QPDFDBG ( printf ( "RESTORE (CLIP=%d/%d)\n", m_painter-> hasClipping ( ), !m_painter-> clipRegion ( ). isEmpty ( )));
}

void QOutputDev::updateAll ( GfxState *state )
{
	updateLineAttrs ( state, gTrue );
//	updateFlatness ( state );
//	updateMiterLimit ( state );
	updateFillColor ( state );
	updateStrokeColor ( state );
	updateFont ( state );
}

void QOutputDev::updateCTM ( GfxState *state, fp_t /*m11*/, fp_t /*m12*/, fp_t /*m21*/, fp_t /*m22*/, fp_t /*m31*/, fp_t /*m32*/ )
{
	updateLineAttrs ( state, gTrue );
}

void QOutputDev::updateLineDash ( GfxState *state )
{
	updateLineAttrs ( state, gTrue );
}

void QOutputDev::updateFlatness ( GfxState */*state*/ )
{
	// not supported
	QPDFDBG( printf ( "updateFlatness not supported !\n" ));
}

void QOutputDev::updateLineJoin ( GfxState *state )
{
	updateLineAttrs ( state, gFalse );
}

void QOutputDev::updateLineCap ( GfxState *state )
{
	updateLineAttrs ( state, gFalse );
}

// unimplemented
void QOutputDev::updateMiterLimit ( GfxState */*state*/ )
{
	QPDFDBG( printf ( "updateMiterLimit not supported !\n" ));
}

void QOutputDev::updateLineWidth ( GfxState *state )
{
	updateLineAttrs ( state, gFalse );
}

void QOutputDev::updateLineAttrs ( GfxState *state, GBool updateDash )
{
	fp_t *dashPattern;
	int dashLength;
	fp_t dashStart;

	Qt::PenCapStyle  cap;
	Qt::PenJoinStyle join;
	int width;

	width = lrint ( state-> getTransformedLineWidth ( ));

	switch ( state-> getLineCap ( )) {
		case 0: cap = FlatCap; break;
		case 1: cap = RoundCap; break;
		case 2: cap = SquareCap; break;
		default:
			qWarning ( "Bad line cap style (%d)\n", state-> getLineCap ( ));
			cap = FlatCap;
			break;
	}

	switch (state->getLineJoin()) {
		case 0: join = MiterJoin; break;
		case 1: join = RoundJoin; break;
		case 2: join = BevelJoin; break;
		default:
			qWarning ( "Bad line join style (%d)\n", state->getLineJoin ( ));
			join = MiterJoin;
			break;
	}

	state-> getLineDash ( &dashPattern, &dashLength, &dashStart );

	QColor oldcol = m_painter-> pen ( ). color ( );
	GfxRGB rgb;

	state-> getStrokeRGB ( &rgb );
	oldcol = q_col ( rgb );

	m_painter-> setPen ( QPen ( oldcol, width, dashLength > 0 ? DashLine : SolidLine, cap, join ));

	if ( updateDash && ( dashLength > 0 )) {
		// Not supported by QT
/*
		char dashList[20];
		if (dashLength > 20)
			dashLength = 20;
		for ( int i = 0; i < dashLength; ++i ) {
			dashList[i] = xoutRound(state->transformWidth(dashPattern[i]));
			if (dashList[i] == 0)
				dashList[i] = 1;
		}
		XSetDashes(display, strokeGC, xoutRound(dashStart), dashList, dashLength);
*/
	}
}

void QOutputDev::updateFillColor ( GfxState *state )
{
	GfxRGB rgb;
	state-> getFillRGB ( &rgb );

	m_painter-> setBrush ( q_col ( rgb ));
}

void QOutputDev::updateStrokeColor ( GfxState *state )
{
	GfxRGB rgb;
	state-> getStrokeRGB ( &rgb );

	QPen pen = m_painter-> pen ( );
	pen. setColor ( q_col ( rgb ));
	m_painter-> setPen ( pen );
}

void QOutputDev::updateFont ( GfxState *state )
{
	fp_t m11, m12, m21, m22;
	GfxFont *gfxFont = state-> getFont ( );

	if ( !gfxFont )
		return;

	state-> getFontTransMat ( &m11, &m12, &m21, &m22 );
	m11 *= state-> getHorizScaling ( );
	m12 *= state-> getHorizScaling ( );

	QFont font = matchFont ( gfxFont, m11, m12, m21, m22 );

	m_painter-> setFont ( font );
	m_text-> updateFont ( state );
}

void QOutputDev::stroke ( GfxState *state )
{
	QPointArray points;
	QArray<int> lengths;

	// transform points
	int n = convertPath ( state, points, lengths );

	QPDFDBG( printf ( "DRAWING: %d POLYS\n", n ));

	// draw each subpath
	int j = 0;
	for ( int i = 0; i < n; i++ ) {
		int len = lengths [i];

		if ( len >= 2 ) {
			QPDFDBG( printf ( " - POLY %d: ", i ));
			QPDFDBG( for ( int ii = 0; ii < len; ii++ ))
				QPDFDBG( printf ( "(%d/%d) ", points [j+ii]. x ( ), points [j+ii]. y ( )));
			QPDFDBG( printf ( "\n" ));
		// By VCD RISK{ 
		//m_painter-> drawPolyline ( points, j, len );
		//}
		m_painter-> drawPolyline ( points);
		}
		j += len;
	}
	qApp-> processEvents ( );
}

void QOutputDev::fill ( GfxState *state )
{
	doFill ( state, true );
}

void QOutputDev::eoFill ( GfxState *state )
{
	doFill ( state, false );
}

//
//  X doesn't color the pixels on the right-most and bottom-most
//  borders of a polygon.  This means that one-pixel-thick polygons
//  are not colored at all.  I think this is supposed to be a
//  feature, but I can't figure out why.  So after it fills a
//  polygon, it also draws lines around the border.  This is done
//  only for single-component polygons, since it's not very
//  compatible with the compound polygon kludge (see convertPath()).
//
void QOutputDev::doFill ( GfxState *state, bool winding )
{
	QPointArray points;
	QArray<int> lengths;

	// transform points
	int n = convertPath ( state, points, lengths );

	QPDFDBG( printf ( "FILLING: %d POLYS\n", n ));

	QPen oldpen = m_painter-> pen ( );
	m_painter-> setPen ( QPen ( NoPen ));

	// draw each subpath
	int j = 0;
	for ( int i = 0; i < n; i++ ) {
		int len = lengths [i];

		if ( len >= 3 ) {
			QPDFDBG( printf ( " - POLY %d: ", i ));
			QPDFDBG( for ( int ii = 0; ii < len; ii++ ))
				QPDFDBG( printf ( "(%d/%d) ", points [j+ii]. x ( ), points [j+ii]. y ( )));
			QPDFDBG( printf ( "\n" ));
			// By VCD RISK{	
			//m_painter-> drawPolygon ( points, winding, j, len );
			// }
			m_painter-> drawPolygon ( points);

		}
		j += len;
	}
	m_painter-> setPen ( oldpen );

	qApp-> processEvents ( );
}

void QOutputDev::clip ( GfxState *state )
{
	doClip ( state, true );
}

void QOutputDev::eoClip ( GfxState *state )
{
	doClip ( state, false );
}

void QOutputDev::doClip ( GfxState *state, bool winding )
{
	QPointArray points;
	QArray<int> lengths;

	// transform points
	int n = convertPath ( state, points, lengths );

	QRegion region;

	QPDFDBG( printf ( "CLIPPING: %d POLYS\n", n ));

	// draw each subpath
	int j = 0;
	for ( int i = 0; i < n; i++ ) {
		int len = lengths [i];

		if ( len >= 3 ) {
			QPointArray dummy;

#if QT_VERSION >= 0x040100
//#warning "Vishwajeet : Fix Me" 			
			dummy = points;
#else
			dummy. setRawData ( points. data ( ) + j, len );
#endif

			QPDFDBG( printf ( " - POLY %d: ", i ));
			QPDFDBG( for ( int ii = 0; ii < len; ii++ ) printf ( "(%d/%d) ", points [j+ii]. x ( ), points [j+ii]. y ( )));
			QPDFDBG( printf ( "\n" ));
			// By VCD RISK{
			//region |= QRegion ( dummy, winding );
			// }	

			region |= QRegion ( dummy);

#if QT_VERSION >= 0x040100
//#warning "Vishwajeet : Fix Me" 
			dummy.clear();
#else
			dummy. resetRawData ( points. data ( ) + j, len );
#endif

		}
		j += len;
	}

	if ( m_painter && m_painter-> hasClipping ( ))
		region &= m_painter-> clipRegion ( );

//	m_painter-> setClipRegion ( region );
//	m_painter-> setClipping ( true );

//	m_painter-> fillRect ( 0, 0, m_pixmap-> width ( ), m_pixmap-> height ( ), red );
//	m_painter-> drawText ( points [0]. x ( ) + 10, points [0]. y ( ) + 10, "Bla bla" );
	qApp-> processEvents ( );
}

//
// Transform points in the path and convert curves to line segments.
// Builds a set of subpaths and returns the number of subpaths.
// If <fillHack> is set, close any unclosed subpaths and activate a
// kludge for polygon fills:  First, it divides up the subpaths into
// non-overlapping polygons by simply comparing bounding rectangles.
// Then it connects subaths within a single compound polygon to a single
// point so that X can fill the polygon (sort of).
//
int QOutputDev::convertPath ( GfxState *state, QPointArray &points, QArray<int> &lengths )
{
	GfxPath *path = state-> getPath ( );
	int n = path-> getNumSubpaths ( );

	lengths. resize ( n );

	// do each subpath
	for ( int i = 0; i < n; i++ ) {
		// transform the points
		lengths [i] = convertSubpath ( state, path-> getSubpath ( i ), points );
	}

	return n;
}

//
// Transform points in a single subpath and convert curves to line
// segments.
//
int QOutputDev::convertSubpath ( GfxState *state, GfxSubpath *subpath, QPointArray &points )
{
	int oldcnt = points. count ( );

	fp_t x0, y0, x1, y1, x2, y2, x3, y3;

	int m = subpath-> getNumPoints ( );
	int i = 0;

	while ( i < m ) {
		if ( i >= 1 && subpath-> getCurve ( i )) {
			state-> transform ( subpath-> getX ( i - 1 ), subpath-> getY ( i - 1 ), &x0, &y0 );
			state-> transform ( subpath-> getX ( i ),     subpath-> getY ( i ),     &x1, &y1 );
			state-> transform ( subpath-> getX ( i + 1 ), subpath-> getY ( i + 1 ), &x2, &y2 );
			state-> transform ( subpath-> getX ( i + 2 ), subpath-> getY ( i + 2 ), &x3, &y3 );

			QPointArray tmp;
			tmp. setPoints ( 4, lrint ( x0 ), lrint ( y0 ), lrint ( x1 ), lrint ( y1 ),
			                    lrint ( x2 ), lrint ( y2 ), lrint ( x3 ), lrint ( y3 ));

#if QT_VERSION < 0x030000
			tmp = tmp. quadBezier ( );

			for ( uint loop = 0; loop < tmp. count ( ); loop++ ) {
				QPoint p = tmp. point ( loop );
				points. putPoints ( points. count ( ), 1, p. x ( ), p. y ( ));
			}
#else
			// By VCD HIGH RISK {
			//tmp = tmp. cubicBezier ( );
			//}
			points. putPoints ( points. count ( ), tmp. count ( ), tmp );
#endif
	
			i += 3;
		}
		else {
			state-> transform ( subpath-> getX ( i ), subpath-> getY ( i ), &x1, &y1 );

			points. putPoints ( points. count ( ), 1, lrint ( x1 ), lrint ( y1 ));
			++i;
		}
	}
	return points. count ( ) - oldcnt;
}


void QOutputDev::beginString ( GfxState *state, GString */*s*/ )
{
	m_text-> beginString ( state );
}

void QOutputDev::endString ( GfxState */*state*/ )
{
	m_text-> endString ( );
}

void QOutputDev::drawChar ( GfxState *state, fp_t x, fp_t y,
                           fp_t dx, fp_t dy, fp_t originX, fp_t originY,
                           CharCode code, Unicode *u, int uLen )
{
	fp_t x1, y1, dx1, dy1;

	if ( uLen > 0 )
		m_text-> addChar ( state, x, y, dx, dy, u, uLen );

	// check for invisible text -- this is used by Acrobat Capture
	if (( state-> getRender ( ) & 3 ) == 3 ) {
		return;
	}

	x -= originX;
	y -= originY;
	state-> transform      ( x,  y,  &x1,  &y1 );
	state-> transformDelta ( dx, dy, &dx1, &dy1 );


	if ( uLen > 0 ) {
		QString str;
		QFontMetrics fm = m_painter-> fontMetrics ( );

		for ( int i = 0; i < uLen; i++ ) {
			QChar c = QChar ( u [i] );

			if ( fm. inFont ( c )) {
				str [i] = QChar ( u [i] );
			}
			else {
				str [i] = ' ';
				QPDFDBG( printf ( "CHARACTER NOT IN FONT: %hx\n", c. unicode ( )));
			}
		}

		if (( uLen == 1 ) && ( str [0] == ' ' ))
			return;


		fp_t m11, m12, m21, m22;

		state-> getFontTransMat ( &m11, &m12, &m21, &m22 );
		m11 *= state-> getHorizScaling ( );
		m12 *= state-> getHorizScaling ( );

		fp_t fsize = m_painter-> font ( ). pixelSize ( );

#ifndef QT_NO_TRANSFORMATIONS
		QWMatrix oldmat;

		bool dorot = (( m12 < -0.1 ) || ( m12 > 0.1 )) && (( m21 < -0.1 ) || ( m21 > 0.1 ));

		if ( dorot ) {
			oldmat = m_painter-> worldMatrix ( );

#if QT_VERSION >= 0x040100
//#warning "Vishwajeet : Fix Me" 

			//qDebug() << "ROTATED: " << m11 << ", " << m12 << ", " << m21 << ", " << m22 << " / SIZE: " << fsize << " / TEXT: " << str. local8Bit ( );

#else
			std::cerr << std::endl << "ROTATED: " << m11 << ", " << m12 << ", " << m21 << ", " << m22 << " / SIZE: " << fsize << " / TEXT: " << str. local8Bit ( ) << endl << endl;
#endif



			QWMatrix mat ( lrint ( m11 / fsize ), lrint ( m12 / fsize ), -lrint ( m21 / fsize ), -lrint ( m22 / fsize ), lrint ( x1 ), lrint ( y1 ));

			m_painter-> setWorldMatrix ( mat );

			x1 = 0;
			y1 = 0;
		}
#endif

		QPen oldpen = m_painter-> pen ( );

		if (!( state-> getRender ( ) & 1 )) {
			QPen fillpen = oldpen;

			fillpen. setColor ( m_painter-> brush ( ). color ( ));
			m_painter-> setPen ( fillpen );
		}

		if ( fsize > 5 )
			m_painter-> drawText ( lrint ( x1), lrint ( y1 ), str );
			
		else
			m_painter-> fillRect ( lrint ( x1 ), lrint ( y1 ), lrint ( QMAX( fp_t(1), dx1 )), lrint ( QMAX( fsize, dy1 )), m_painter-> pen ( ). color ( ));

		m_painter-> setPen ( oldpen );

#ifndef QT_NO_TRANSFORMATIONS
		if ( dorot )
			m_painter-> setWorldMatrix ( oldmat );
#endif

		QPDFDBG( printf ( "DRAW TEXT: \"%s\" at (%ld/%ld)\n", str. local8Bit ( ). data ( ), lrint ( x1 ), lrint ( y1 )));
	}
	else if ( code != 0 ) {
		// some PDF files use CID 0, which is .notdef, so just ignore it
		qWarning ( "Unknown character (CID=%d Unicode=%hx)\n", code, (unsigned short) ( uLen > 0 ? u [0] : (Unicode) 0 ));
	}
	qApp-> processEvents ( );
}



void QOutputDev::drawImageMask ( GfxState *state, Object */*ref*/, Stream *str, int width, int height, GBool invert, GBool inlineImg )
{
	// get CTM, check for singular matrix
	fp_t *ctm = state-> getCTM ( );

	if ( fabs ( ctm [0] * ctm [3] - ctm [1] * ctm [2] ) < 0.000001 ) {
		qWarning ( "Singular CTM in drawImage\n" );

		if ( inlineImg ) {
			str-> reset ( );
			int j = height * (( width + 7 ) / 8 );
			for ( int i = 0; i < j; i++ )
				str->getChar();

			str->close();
		}
		return;
	}

	GfxRGB rgb;
	state-> getFillRGB ( &rgb );
	uint val = ( lrint ( rgb. r * 255 ) & 0xff ) << 16 | ( lrint ( rgb. g * 255 ) & 0xff ) << 8 | ( lrint ( rgb. b * 255 ) & 0xff );


	QImage img ( width, height,QImage::Format_ARGB32);
	// By VCD RISK {
	//img. setAlphaBuffer ( true );
	//}

	QPDFDBG( printf ( "IMAGE MASK (%dx%d)\n", width, height ));

	// initialize the image stream
	ImageStream *imgStr = new ImageStream ( str, width, 1, 1 );
	imgStr-> reset ( );
  // By VCD RISK {
 //	uchar **scanlines = img. jumpTable ( );
	uchar **scanlines = new uchar * [height];
	uchar **refscanlines = scanlines;
	for(int h=0;h<height;++h)
	{
		scanlines[h] = img. scanLine(h);
	}
	//}

	if ( ctm [3] > 0 )
		scanlines += ( height - 1 );

	for ( int y = 0; y < height; y++ ) {
		QRgb *scanline = (QRgb *) *scanlines;

		if ( ctm [0] < 0 )
			scanline += ( width - 1 );

		for ( int x = 0; x < width; x++ ) {
			Guchar alpha;

			imgStr-> getPixel ( &alpha );

			if ( invert )
				alpha ^= 1;

			*scanline = ( alpha == 0 ) ? 0xff000000 | val : val;

			ctm [0] < 0 ? scanline-- : scanline++;
		}
		ctm [3] > 0 ? scanlines-- : scanlines++;

		qApp-> processEvents ( );
	}

#ifndef QT_NO_TRANSFORMATIONS
	QWMatrix mat ( ctm [0] / width, ctm [1], ctm [2], ctm [3] / height, ctm [4], ctm [5] );

	std::cerr << "MATRIX T=" << mat. dx ( ) << "/" << mat. dy ( ) << std::endl
	         << " - M=" << mat. m11 ( ) << "/" << mat. m12 ( ) << "/" << mat. m21 ( ) << "/" << mat. m22 ( ) << std::endl;

	QWMatrix oldmat = m_painter-> worldMatrix ( );
	m_painter-> setWorldMatrix ( mat, true );

#ifdef QWS
	QPixmap pm;
	pm. convertFromImage ( img );
	m_painter-> drawPixmap ( 0, 0, pm );
#else
	m_painter-> drawImage ( QPoint ( 0, 0 ), img );
#endif

	m_painter-> setWorldMatrix ( oldmat );

#else
	if (( ctm [1] < -0.1 ) || ( ctm [1] > 0.1 ) || ( ctm [2] < -0.1 ) || ( ctm [2] > 0.1 )) {
		QPDFDBG( printf (  "### ROTATED / SHEARED / ETC -- CANNOT DISPLAY THIS IMAGE\n" ));
	}
	else {
		int x = lrint ( ctm [4] );
		int y = lrint ( ctm [5] );

		int w = lrint ( ctm [0] );
		int h = lrint ( ctm [3] );

		if ( w < 0 ) {
			x += w;
			w = -w;
		}
		if ( h < 0 ) {
			y += h;
			h = -h;
		}

		QPDFDBG( printf ( "DRAWING IMAGE MASKED: %d/%d - %dx%d\n", x, y, w, h ));

		img = img. scaled ( w, h );
		qApp-> processEvents ( );
		m_painter-> drawImage ( x, y, img );
	}

#endif
	//delete []scanlines;
	delete imgStr;
	delete refscanlines;
	qApp-> processEvents ( );
}


void QOutputDev::drawImage(GfxState *state, Object */*ref*/, Stream *str, int width, int height, GfxImageColorMap *colorMap, int *maskColors, GBool inlineImg )
{
	int nComps, nVals, nBits;

	// image parameters
	nComps = colorMap->getNumPixelComps ( );
	nVals = width * nComps;
	nBits = colorMap-> getBits ( );

	// get CTM, check for singular matrix
	fp_t *ctm = state-> getCTM ( );

	if ( fabs ( ctm [0] * ctm [3] - ctm [1] * ctm [2] ) < 0.000001 ) {
		qWarning ( "Singular CTM in drawImage\n" );

		if ( inlineImg ) {
			str-> reset ( );
			int j = height * (( nVals * nBits + 7 ) / 8 );
			for ( int i = 0; i < j; i++ )
				str->getChar();

			str->close();
		}
		return;
	}

	QImage img ( width, height,QImage::Format_ARGB32 );

		
	// By VCD RISK {
	//if ( maskColors )
		//	img. setAlphaBuffer ( true );
		//}

	QPDFDBG( printf ( "IMAGE (%dx%d)\n", width, height ));

	// initialize the image stream
	ImageStream *imgStr = new ImageStream ( str, width, nComps, nBits );
	imgStr-> reset ( );

	Guchar pixBuf [gfxColorMaxComps];
	GfxRGB rgb;

	// By VCD HIGH RISK, PROGRAM WILL CRASH IF IMAGES ARE THERE IN THE PDF 
	//TODO: FIX IT IMPORTANT
	uchar **scanlines = new uchar * [height];
	uchar **refscanlines = scanlines;
	for(int h=0;h<height;++h)
	{
		//if(img. scanLine(h) != NULL)
		scanlines[h] = img. scanLine(h);
	}


	if ( ctm [3] > 0 )
		scanlines += ( height - 1 );

	for ( int y = 0; y < height; y++ ) {
		QRgb *scanline = (QRgb *) *scanlines;

		if ( ctm [0] < 0 )
			scanline += ( width - 1 );

		for ( int x = 0; x < width; x++ ) {
			imgStr-> getPixel ( pixBuf );
			colorMap-> getRGB ( pixBuf, &rgb );

			uint val = ( lrint ( rgb. r * 255 ) & 0xff ) << 16 | ( lrint ( rgb. g * 255 ) & 0xff ) << 8 | ( lrint ( rgb. b * 255 ) & 0xff );

			if ( maskColors ) {
				for ( int k = 0; k < nComps; ++k ) {
					if (( pixBuf [k] < maskColors [2 * k] ) || ( pixBuf [k] > maskColors [2 * k] )) {
						val |= 0xff000000;
						break;
					}
				}
			}
			*scanline = val;

			ctm [0] < 0 ? scanline-- : scanline++;
		}
		ctm [3] > 0 ? scanlines-- : scanlines++;

		qApp-> processEvents ( );
	}


#ifndef QT_NO_TRANSFORMATIONS
		QWMatrix mat ( ctm [0] / width, ctm [1], ctm [2], ctm [3] / height, ctm [4], ctm [5] );

	std::cerr << "MATRIX T=" << mat. dx ( ) << "/" << mat. dy ( ) << std::endl
	         << " - M=" << mat. m11 ( ) << "/" << mat. m12 ( ) << "/" << mat. m21 ( ) << "/" << mat. m22 ( ) << std::endl;

	QWMatrix oldmat = m_painter-> worldMatrix ( );
	m_painter-> setWorldMatrix ( mat, true );

#ifdef QWS
	QPixmap pm;
	pm. convertFromImage ( img );
	m_painter-> drawPixmap ( 0, 0, pm );
#else
	m_painter-> drawImage ( QPoint ( 0, 0 ), img );


	
#endif

	m_painter-> setWorldMatrix ( oldmat );

#else // QT_NO_TRANSFORMATIONS

	if (( ctm [1] < -0.1 ) || ( ctm [1] > 0.1 ) || ( ctm [2] < -0.1 ) || ( ctm [2] > 0.1 )) {
		QPDFDBG( printf ( "### ROTATED / SHEARED / ETC -- CANNOT DISPLAY THIS IMAGE\n" ));
	}
	else {
		int x = lrint ( ctm [4] );
		int y = lrint ( ctm [5] );

		int w = lrint ( ctm [0] );
		int h = lrint ( ctm [3] );

		if ( w < 0 ) {
			x += w;
			w = -w;
		}
		if ( h < 0 ) {
			y += h;
			h = -h;
		}

		QPDFDBG( printf ( "DRAWING IMAGE: %d/%d - %dx%d\n", x, y, w, h ));

		img = img. scaled ( w, h );
		qApp-> processEvents ( );
		m_painter-> drawImage ( x, y, img );
	}

#endif

//	delete []scanlines;
	delete imgStr;
	delete refscanlines;
	qApp-> processEvents ( );
}



bool QOutputDev::findText ( const QString &str, QRect &r, bool top, bool bottom )
{
	int l, t, w, h;
	r. getRect ( &l, &t, &w, &h );

	bool res = findText ( str, l, t, w, h, top, bottom );

	r. setRect ( l, t, w, h );
	return res;
}

bool QOutputDev::findText ( const QString &str, int &l, int &t, int &w, int &h, bool top, bool bottom )
{
	bool found = false;
	uint len = str. length ( );
	Unicode *s = new Unicode [len];

	for ( uint i = 0; i < len; i++ )
		s [i] = str [i]. unicode ( );

	fp_t x1 = (fp_t) l;
	fp_t y1 = (fp_t) t;
	fp_t x2 = (fp_t) l + w - 1;
	fp_t y2 = (fp_t) t + h - 1;

	if ( m_text-> findText ( s, len, top, bottom, &x1, &y1, &x2, &y2 )) {
		l = lrint ( x1 );
		t = lrint ( y1 );
		w = lrint ( x2 ) - l + 1;
		h = lrint ( y2 ) - t + 1;
		found = true;
	}
	delete [] s;
	
	return found;
}

GBool QOutputDev::findText ( Unicode *s, int len, GBool top, GBool bottom, int *xMin, int *yMin, int *xMax, int *yMax )
{
	bool found = false;
	fp_t xMin1 = (double) *xMin;
	fp_t yMin1 = (double) *yMin;
	fp_t xMax1 = (double) *xMax;
	fp_t yMax1 = (double) *yMax;

	if ( m_text-> findText ( s, len, top, bottom, &xMin1, &yMin1, &xMax1, &yMax1 )) {
		*xMin = lrint ( xMin1 );
		*xMax = lrint ( xMax1 );
		*yMin = lrint ( yMin1 );
		*yMax = lrint ( yMax1 );
		found = true;
	}
	return found;
}

QString QOutputDev::getText ( int l, int t, int w, int h )
{
	GString *gstr = m_text-> getText ( l, t, l + w - 1, t + h - 1 );
	QString str = gstr-> getCString ( );
	delete gstr;
	return str;
}

QString QOutputDev::getText ( const QRect &r )
{
	return getText ( r. left ( ), r. top ( ), r. width ( ), r. height ( ));
}

void QOutputDev::paintEvent ( QPaintEvent * event )
{
	drawContents(m_painter,0,0,289,65);
}

void QOutputDev::drawContents ( QPainter *p, int clipx, int clipy, int clipw, int cliph )
{
	if ( m_pixmap )
		//p-> drawPixmap ( clipx, clipy, *m_pixmap, clipx, clipy, clipw, cliph );
		p-> drawPixmap ( clipx, clipy, clipw, cliph,*m_pixmap,clipx, clipy, clipw, cliph );
	else
		p-> fillRect ( clipx, clipy, clipw, cliph, white );
}

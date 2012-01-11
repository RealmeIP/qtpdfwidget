
//========================================================================
//
// XOutputDev.h
//
// Copyright 1996 Derek B. Noonburg
//
//========================================================================

#ifndef QOUTPUTDEV_H
#define QOUTPUTDEV_H

#ifdef __GNUC__
#pragma interface
#endif

#include "aconf.h"
#include <stddef.h>

//#if QT_VERSION >= 0x040100
//#warning "Vishwajeet : Fix Me,QT_VERSION >= 0x040100 not working" 
      #include <QPolygon>
      #include <QScrollArea>
#include <QList>

#define VCD_SCROLLVIEW 0
	
#define QScrollView QWidget
#define QArray QVector
#define QPointArray QPolygon
//#else
//      #include <qscrollview.h>
//#endif

class Object;

#include "config.h"
#include "CharTypes.h"
#include "GlobalParams.h"
#include "OutputDev.h"

class GString;
class GList;
struct GfxRGB;
class GfxFont;
class GfxSubpath;
class TextPage;
class XOutputFontCache;
class Link;
class Catalog;
class DisplayFontParam;
class UnicodeMap;
class CharCodeToUnicode;


class QPainter;
class QPixmap;
class QPointArray;


typedef fouble fp_t;

//------------------------------------------------------------------------
// Constants
//------------------------------------------------------------------------


//------------------------------------------------------------------------
// Misc types
//------------------------------------------------------------------------


//------------------------------------------------------------------------
// XOutputDev
//------------------------------------------------------------------------

class QOutputDev : public QScrollView, public OutputDev {
	Q_OBJECT

public:

	// Constructor.
	QOutputDev( QWidget *parent = 0, const char *name = 0, Qt::WindowFlags flags = 0 );

	// Destructor.
	virtual ~QOutputDev();

	//---- get info about output device

	// Does this device use upside-down coordinates?
	// (Upside-down means (0,0) is the top left corner of the page.)
	virtual GBool upsideDown() { return gTrue; }

	// Does this device use drawChar() or drawString()?
	virtual GBool useDrawChar() { return gTrue; }

	// Does this device use beginType3Char/endType3Char?  Otherwise,
	// text in Type 3 fonts will be drawn with drawChar/drawString.
	virtual GBool interpretType3Chars() { return gFalse; }
       
	// Does this device need non-text content?
	virtual GBool needNonText() { return gFalse; }

	//----- initialization and control

	// Start a page.
	virtual void startPage(int pageNum, GfxState *state);

	// End a page.
	virtual void endPage();

	//----- link borders
	virtual void drawLink(Link *link, Catalog *catalog);

	//----- save/restore graphics state
	virtual void saveState(GfxState *state);
	virtual void restoreState(GfxState *state);

	//----- update graphics state
	virtual void updateAll(GfxState *state);
	virtual void updateCTM(GfxState *state, fp_t m11, fp_t m12,
			 fp_t m21, fp_t m22, fp_t m31, fp_t m32);
	virtual void updateLineDash(GfxState *state);
	virtual void updateFlatness(GfxState *state);
	virtual void updateLineJoin(GfxState *state);
	virtual void updateLineCap(GfxState *state);
	virtual void updateMiterLimit(GfxState *state);
	virtual void updateLineWidth(GfxState *state);
	virtual void updateFillColor(GfxState *state);
	virtual void updateStrokeColor(GfxState *state);

	//----- update text state
	virtual void updateFont(GfxState *state);

	//----- path painting
	virtual void stroke(GfxState *state);
	virtual void fill(GfxState *state);
	virtual void eoFill(GfxState *state);

	//----- path clipping
	virtual void clip(GfxState *state);
	virtual void eoClip(GfxState *state);

	//----- text drawing
	virtual void beginString(GfxState *state, GString *s);
	virtual void endString(GfxState *state);
	virtual void drawChar(GfxState *state, fp_t x, fp_t y,
	                      fp_t dx, fp_t dy,
	                      fp_t originX, fp_t originY,
	                      CharCode code, Unicode *u, int uLen);

	//----- image drawing
	virtual void drawImageMask(GfxState *state, Object *ref, Stream *str,
	                          int width, int height, GBool invert,
	                          GBool inlineImg);
	virtual void drawImage(GfxState *state, Object *ref, Stream *str,
	                       int width, int height, GfxImageColorMap *colorMap,
	                       int *maskColors, GBool inlineImg);


	// Find a string.  If <top> is true, starts looking at <l>,<t>;
	// otherwise starts looking at top of page.  If <bottom> is true,
	// stops looking at <l+w-1>,<t+h-1>; otherwise stops looking at bottom
	// of page.  If found, sets the text bounding rectange and returns
	// true; otherwise returns false.
	GBool findText ( Unicode *s, int len, GBool top, GBool bottom, int *xMin, int *yMin, int *xMax, int *yMax );
	
	//----- special QT access

	bool findText ( const QString &str, int &l, int &t, int &w, int &h, bool top = 0, bool bottom = 0 );
	bool findText ( const QString &str, QRect &r, bool top = 0, bool bottom = 0 );

	// Get the text which is inside the specified rectangle.
	QString getText ( int left, int top, int width, int height );
	QString getText ( const QRect &r );

protected:
	virtual void paintEvent ( QPaintEvent * event );
	virtual void drawContents ( QPainter *p, int, int, int, int );

protected:
	QPainter *m_painter;
	QPixmap *m_pixmap;   		// pixmap to draw into
private:

	TextPage *m_text;		// text from the current page

	bool m_good_font_match;
	bool m_reproportion_text;
	
	struct SysFontCacheEntry {
	  QFont font;
	  int avg_size;
	};
	QList<SysFontCacheEntry*> m_sys_font_database;

private:	
	QFont matchFont ( GfxFont *, fp_t m11, fp_t m12, fp_t m21, fp_t m22 );

	void updateLineAttrs ( GfxState *state, GBool updateDash );
	void doFill ( GfxState *state, bool winding );
	void doClip ( GfxState *state, bool winding );
	int convertPath ( GfxState *state, QPointArray &points, QArray<int> &lengths );
	int convertSubpath ( GfxState *state, GfxSubpath *subpath, QPointArray &points );
	void cacheSysFontDatabase();

};

#endif

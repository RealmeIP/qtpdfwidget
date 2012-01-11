#ifndef __QPE_OUTPUTDEV_H__
#define __QPE_OUTPUTDEV_H__


#include "QOutputDev.h"

class QLabel;
class QBusyBar;

class QPEOutputDev : public QOutputDev {
	Q_OBJECT

public:
	QPEOutputDev ( QWidget *parent = 0, const char *name = 0 );
	virtual ~QPEOutputDev ();

	void setPageCount ( int actp, int maxp );

	virtual void setBusy ( bool b = true );
	virtual bool isBusy ( ) const;

	QRect selection ( ) const;
	void setSelection ( const QRect &r, bool scrollto = false );

	virtual void startPage ( int pn, GfxState *gs );

signals:
	void selectionChanged ( const QRect &sel );

protected:
	virtual void setHBarGeometry ( QScrollBar &hbar, int x, int y, int w, int h );
	
	virtual void keyPressEvent ( QKeyEvent *e );
	virtual void viewportMousePressEvent ( QMouseEvent *e );
	virtual void viewportMouseMoveEvent ( QMouseEvent *e ); 
	virtual void viewportMouseReleaseEvent ( QMouseEvent *e );
	
	virtual void paintEvent ( QPaintEvent * event );
	virtual void drawContents ( QPainter *p, int, int, int, int );

private:


	bool m_isbusy;

	QRect   m_selection;
	QPoint  m_selectionstart;
	QPoint  m_selectioncursor;
	bool    m_selectiondrag;


};

#endif
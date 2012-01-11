#include <aconf.h>

#include "QPEOutputDev.h"

#if QT_VERSION >= 0x040100
//#warning "Vishwajeet : Fix Me" 

#include <QKeyEvent>
#include <QApplication>
#include <QLabel>	
#include <QPainter>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QVBoxLayout>


#define AlignCenter Qt::AlignCenter
#define SingleLine Qt::SingleLine
#define WheelFocus Qt::WheelFocus

#define Key_Left Qt::Key_Left
#define Key_Right Qt::Key_Right
#define Key_Up Qt::Key_Up
#define Key_Down Qt::Key_Down 

#define LeftButton Qt::LeftButton


#else
 	#include <qapplication.h>
	#include <qlabel.h>

#endif

#include "qbusybar.h"


QPEOutputDev::QPEOutputDev ( QWidget *parent, const char *name ) : QOutputDev ( parent, name )
{

#if QSCROLLAREA_SUPPORT
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
#endif
	

	m_isbusy = false;
	
	m_selectiondrag = false;
	
	setFocusPolicy ( WheelFocus );
}

QPEOutputDev::~QPEOutputDev ()
{

}

void QPEOutputDev::startPage ( int pn, GfxState *st )
{
	m_selection = QRect ( );
	m_selectiondrag = false;
	
	QOutputDev::startPage ( pn, st );
}


void QPEOutputDev::setPageCount ( int actp, int maxp )
{
	//m_counter-> setText ( QString ( "%1 / %2" ). arg ( actp ). arg ( maxp ));
}

void QPEOutputDev::setBusy ( bool b )
{
	if ( b != m_isbusy ) {
		if ( b ) {
#if NO_OTHER_WIDGET_BY_VISHWAJEET
			m_busybar-> beginBusy ( );
			m_busybar-> show ( );
			m_counter-> hide ( );
		}
		else {
			m_counter-> show ( );	
			m_busybar-> hide ( );
			m_busybar-> endBusy ( );
#endif
		}		
		m_isbusy = b;
	}
}

bool QPEOutputDev::isBusy ( ) const
{
#if NO_OTHER_WIDGET_BY_VISHWAJEET
	return m_isbusy;
#else
	return false;
#endif

}

void QPEOutputDev::setHBarGeometry ( QScrollBar &hbar, int x, int y, int w, int h )
{
	int delta = w * 3 / 10;
#if NO_OTHER_WIDGET_BY_VISHWAJEET	
	m_counter-> setGeometry ( x, y, delta, h );
	m_busybar-> setGeometry ( x, y, delta, h );
#endif

	hbar. setGeometry ( x + delta, y, w - delta, h );
}


void QPEOutputDev::keyPressEvent ( QKeyEvent *e )
{
	switch ( e-> key ( )) {

	#if QT_VERSION >= 0x040100
		case Key_Left:
			scroll ( -10, 0 );
			break;
		case Key_Right:
			scroll ( 10, 0 );
			break;
		case Key_Up:
			scroll ( 0, -10 );
			break;
		case Key_Down:
			scroll ( 0, 10 );
			break;
	#else
		case Key_Left:
			scrollBy ( -10, 0 );
			break;
		case Key_Right:
			scrollBy ( 10, 0 );
			break;
		case Key_Up:
			scrollBy ( 0, -10 );
			break;
		case Key_Down:
			scrollBy ( 0, 10 );
			break;
	#endif
	
		default:
			QOutputDev::keyPressEvent ( e );
	}
}

void QPEOutputDev::paintEvent ( QPaintEvent * event )
{
	/*
	QPainter painter(this);
	 painter.setPen(QColor(0,255,0));
     painter.setFont(QFont("Arial", 50));
     painter.drawText(20,20 , "Vishwajeet Dusane");
	*/

//	QPainter painter(viewport());
//	painter.setPen(QColor(0,255,0));
 //    painter.setFont(QFont("Arial", 50));
 //    painter.drawText(20,20 , "Vishwajeet Dusane");
	
	if(m_pixmap != NULL )
	{
		QPainter painter(this);
		drawContents(&painter,0,0,m_pixmap->width(),m_pixmap->height());
	}
	//m_counter->setPixmap(*m_pixmap);
}

void QPEOutputDev::drawContents ( QPainter *p, int clipx, int clipy, int clipw, int cliph )
{

	//qDebug("DrawContents QPEOutputDev::drawContents");
	QOutputDev::drawContents ( p, clipx, clipy, clipw, cliph );


	if ( m_selection. isValid ( )) {
		QRect clip ( clipx, clipy, clipw, cliph );
			
		if ( m_selection. intersects ( clip )) {
			
			#if QT_VERSION >= 0x040100
//			#warning "Vishwajeet : Fix Me" 
			//TODO Vishwajeet : Implement this , Refer http://lists.trolltech.com/qt-interest/2007-05/thread00975-0.html
				
				QPainter::CompositionMode rop = p->compositionMode(); 
				//RasterOp rop = p-> rasterOp ( );
				//p-> setRasterOp ( XorROP );
				p->setCompositionMode(rop);	
				QBrush brush;
				brush.setStyle(Qt::Dense4Pattern);
				brush.setColor(Qt::cyan);
				p->setPen(Qt::black);
				//p->setRenderHint(QPainter::TextAntialiasing);
				p-> fillRect ( m_selection & clip, brush );
				//p-> setRasterOp ( rop );
				p->setCompositionMode(rop);

			#else
				RasterOp rop = p-> rasterOp ( );
				p-> setRasterOp ( XorROP );
				p-> fillRect ( m_selection & clip, white );
				p-> setRasterOp ( rop );
			#endif
		}
	}
}


QRect QPEOutputDev::selection ( ) const
{
	return m_selection;
}


void QPEOutputDev::setSelection ( const QRect &r, bool scrollto )
{
	QRect oldsel = m_selection;
	m_selection = r;

	QArray<QRect> urects = ( QRegion ( oldsel ) ^ QRegion ( m_selection )). rects ( );

	for ( uint i = 0; i < urects. count ( ); i++ )
		#if QT_VERSION >= 0x040100
		update(urects [i]);
		#else
		repaintContents ( urects [i] );
		#endif

	
	if ( scrollto ) {
		QPoint c = r. center ( );
		
#if QSCROLLAREA_SUPPORT	
		ensureVisible ( c. x ( ), c. y ( ), r. width ( ) / 2 + 5, r. height ( ) / 2 + 5 );
#endif
		 
	}
	
	if ( !m_selectiondrag )
		emit selectionChanged ( m_selection );
}


void QPEOutputDev::viewportMousePressEvent ( QMouseEvent *e )
{
	if ( e-> button ( ) == LeftButton ) {
	#if QT_VERSION >= 0x040100
		#if	QSCROLLAREA_SUPPORT
			m_selectionstart =  e-> pos ( ) + QPoint ( this->viewport()->x( ), this->viewport()->y( ));
		#endif
	#else
		m_selectionstart =  e-> pos ( ) + QPoint ( contentsX ( ), contentsY ( ));
	#endif

		m_selectioncursor = m_selectionstart;
		m_selectiondrag = true;
		
		setSelection ( QRect ( m_selectionstart, QSize ( 0, 0 )), true );
    }
}

void QPEOutputDev::viewportMouseMoveEvent ( QMouseEvent *e )
{
	if ( e-> button ()& LeftButton ) {
		if ( m_selectiondrag ) {
		#if QT_VERSION >= 0x040100
			#if QSCROLLAREA_SUPPORT
				QPoint to ( e-> pos ( ) + QPoint ( this->viewport()->x( ), this->viewport()->y( )));
			#else
				QPoint to ( e-> pos ( ) + QPoint ( this->x( ), this->y( )));
			#endif
		#else
			QPoint to ( e-> pos ( ) + QPoint ( contentsX ( ), contentsY ( )));
		#endif

			if ( to != m_selectioncursor ) {
				setSelection ( QRect ( m_selectionstart, to ). normalized ( ), false );
				m_selectioncursor = to;
			}
			#if QSCROLLAREA_SUPPORT
				ensureVisible ( m_selectioncursor. x ( ), m_selectioncursor. y ( ), 5, 5 );
			#endif	
		}
	}
}


void QPEOutputDev::viewportMouseReleaseEvent ( QMouseEvent *e )
{
	if ( e-> button ( ) == LeftButton ) {
		if ( m_selectiondrag ) {
			m_selectiondrag = false;
			
			setSelection ( selection ( ), false ); // emit signal
		} 
		else {
			setSelection ( QRect ( 0, 0, 0, 0 ), false );
		}		
	}
}


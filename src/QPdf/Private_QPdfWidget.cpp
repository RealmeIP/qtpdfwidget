
#include "Private_QPdfWidget.h"


#include <QLabel>
#include <QPalette>

//////////////////////////////////
// Qt includes
//////////////////////////////////
#include <QTimer>
#include <QFileInfo>
#include <QMessageBox>
#include <QPainter>
#include <QWidget>
#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>
//////////////////////////////////
// xpdf includes
//////////////////////////////////
#include "GString.h"
#include "PDFDoc.h"
#include "QOutputDev.h"
#include "QPEOutputDev.h"
#include "TextOutputDev.h"




CPrivate_QPdfWidget::CPrivate_QPdfWidget( QWidget * parent , Qt::WindowFlags f ):QWidget(parent,f)
{
	m_doc= 0;
	m_busy = false;
	m_pages = 0;
	m_zoom = 72;
	m_currentpage = 0;	

	m_renderok = false;

	m_outdev = new QPEOutputDev ();
	
	scrollArea = new QScrollArea();

    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setAlignment(Qt::AlignCenter);
    scrollArea->setWidget(m_outdev);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	QVBoxLayout *mainLayout = new QVBoxLayout(this);

	mainLayout->addWidget(scrollArea);

	mainLayout->setContentsMargins(0,0,0,0);

	setLayout(mainLayout);


	//this->openFile("D:\\Projects\\QPdf_Viewer\\PDF_ Files\\Untitled1.pdf");
}


CPrivate_QPdfWidget::~CPrivate_QPdfWidget(void)
{
	if(m_doc)
		{
		delete m_doc;
		m_doc = NULL;
		}
	if(m_outdev)
		{
		delete m_outdev;
		m_outdev = NULL;
		}
	
	delete scrollArea;
}

void CPrivate_QPdfWidget::closepdf()
	{
		if(m_outdev)
			{
			delete m_outdev;
			m_outdev = new QPEOutputDev ();
			scrollArea->setWidget(m_outdev);
			m_pages = 0;
			m_currentpage = 0;
			emit fileclosed();
			}
	}

void CPrivate_QPdfWidget::scroll(const int &x,const int &y)
{
	int horizontalPageStep = x * (scrollArea->horizontalScrollBar()->value() + /*scrollArea->horizontalScrollBar()->pageStep()*/30);

	if(x < 0)
	{
		if(-horizontalPageStep < scrollArea->horizontalScrollBar()->minimum())
		{
			scrollArea->horizontalScrollBar()->setValue(scrollArea->horizontalScrollBar()->minimum());
		}else
		{
			scrollArea->horizontalScrollBar()->setValue(/*horizontalPageStep*/scrollArea->horizontalScrollBar()->value()- 30);
		}
	}else if(x > 0)
	{
		if(horizontalPageStep > scrollArea->horizontalScrollBar()->maximum())
		{
			scrollArea->horizontalScrollBar()->setValue(scrollArea->horizontalScrollBar()->maximum());
		}else
		{
			scrollArea->horizontalScrollBar()->setValue(horizontalPageStep);
		}
	}

	int verticalPageStep = y * (scrollArea->verticalScrollBar()->value() +/*scrollArea->verticalScrollBar()->pageStep()*/30);
	
	
	if((scrollArea->verticalScrollBar()->minimum() == scrollArea->verticalScrollBar()->value()) && (y < 0))
	{
		prevPage();
		scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->minimum());
		return ;
	}

	if((scrollArea->verticalScrollBar()->maximum() == scrollArea->verticalScrollBar()->value()) && (y > 0))
	{
		nextPage();
		scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->minimum());
		return ;
	}

	if(y < 0)
	{
		if(- verticalPageStep < scrollArea->verticalScrollBar()->minimum())
		{
			scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->minimum());
		}else
		{
			scrollArea->verticalScrollBar()->setValue(/*verticalPageStep*/scrollArea->verticalScrollBar()->value()-30);
		}
	}else if(y > 0)
	{
		if(verticalPageStep > scrollArea->verticalScrollBar()->maximum())
		{
			scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->maximum());
		}else
		{
			scrollArea->verticalScrollBar()->setValue(/*verticalPageStep*/scrollArea->verticalScrollBar()->value()+30);
		}
	}

}

void CPrivate_QPdfWidget::firstPage ( )
{
	gotoPage ( 1 );
}

void CPrivate_QPdfWidget::prevPage ( )
{
	gotoPage ( m_currentpage - 1 );
}

void CPrivate_QPdfWidget::nextPage ( )
{
	gotoPage ( m_currentpage + 1 );
}

void CPrivate_QPdfWidget::lastPage ( )
{
	gotoPage ( m_pages );
}

void CPrivate_QPdfWidget::gotoPage ( int n )
{
	emit setbusy();
	emit emitpage(n);
	if ( n < 1 )
		n = 1;
	if ( n > m_pages )
		n = m_pages;
		
	if ( n != m_currentpage ) {
		m_currentpage = n;	
		
		renderPage ( );
		scrollArea->horizontalScrollBar()->setValue(scrollArea->horizontalScrollBar()->minimum());
		scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->minimum());
	}
	emit setfree();
}

void CPrivate_QPdfWidget::setZoom ( const int &id )
{
	emit setbusy();
	int dpi = 0;

	switch ( id ) {
	case 1:
		if ( m_doc && m_doc-> isOk ( ))
			#if QT_VERSION >= 0x040100
				dpi = m_outdev-> width ( ) *72 / m_doc-> getPageWidth ( m_currentpage );	
			#else
				dpi = m_outdev-> visibleWidth ( ) * 72 / m_doc-> getPageWidth ( m_currentpage );
			#endif
		break;
	
	case 2:
		if ( m_doc && m_doc-> isOk ( )) 
			#if QT_VERSION >= 0x040100
				dpi = qMin( m_outdev-> width ( ) * 72 / m_doc-> getPageWidth ( m_currentpage ),m_outdev-> height ( ) * 72 / m_doc-> getPageHeight ( m_currentpage ));
			#else
				dpi = QMIN( m_outdev-> visibleWidth ( ) * 72 / m_doc-> getPageWidth ( m_currentpage ), \
			        	    m_outdev-> visibleHeight ( ) * 72 / m_doc-> getPageHeight ( m_currentpage ));
			#endif
		break;
		
	default:
		dpi = (id * 72) / 100;
		break;
	}
	
	if ( dpi < 18 )
		dpi = 18;
	if ( dpi > 216 )
		dpi = 216;

	/* TODO VCD "NOT SURE WEATHER IN QT4 THIS IS REQUIRED OR NOT"
	for ( uint i = 0; i < m_pm_zoom-> count ( ); i++ ) {
		int xid = m_pm_zoom-> idAt ( i );
		m_pm_zoom-> setItemChecked ( xid, xid == id );
	}
	*/

	if ( dpi != m_zoom ) { 
		m_zoom = dpi;

			renderPage ( );

	}
	emit setfree();
}
int CPrivate_QPdfWidget::getCurrentZoomInLevel() const
{
	return this->m_zoom;
}

void CPrivate_QPdfWidget::findText ( const QString &searchString)
{
		if ( !m_doc || !m_doc-> isOk ( ) || searchString. isEmpty ( )) 
		{
			return;
		}

	TextOutputDev *textOut = 0;
	int pg = 0;

	QPalette palette;


	//TODO: Vishwajeet busy status is not there, add this feature and uncomment below line
	//setBusy ( true );
		
	int len = searchString. length ( );
	Unicode *u = new Unicode [len];
	for ( int i = 0; i < len; i++ )
		u [i] = searchString[i]. unicode ( );
		
	int xMin = 0, yMin = 0, xMax = 0, yMax = 0;
	QRect selr = m_outdev-> selection ( );
	bool fromtop = true; 
	
	if ( selr. isValid ( )) {
		xMin = selr. right ( );
		yMin = selr. top ( ) + selr. height ( ) / 2;
		fromtop = false;
	}

	if ( m_outdev-> findText ( u, len, fromtop, true, &xMin, &yMin, &xMax, &yMax )) 
		goto found;

		
	// search following pages
	textOut = new TextOutputDev ( 0, gFalse, gFalse );
	if ( !textOut-> isOk ( )) 
		goto done;
	
	
	for ( pg = ( m_currentpage % m_pages ) + 1; pg != m_currentpage; pg = ( pg % m_pages ) + 1 ) {
		m_doc-> displayPage ( textOut, pg, 72, 0, gFalse );
		
		fp_t xMin1, yMin1, xMax1, yMax1;
		
	
		if ( textOut-> findText ( u, len, gTrue, gTrue, &xMin1, &yMin1, &xMax1, &yMax1 ))
			goto foundPage;
			
		
	}

	// search current page ending at current selection
	if ( selr. isValid ( )) {
		xMax = selr. left ( );
		yMax = selr. top ( ) + selr. height ( ) / 2;

		if ( m_outdev-> findText ( u, len, gTrue, gFalse, &xMin, &yMin, &xMax, &yMax )) 
			goto found;
	}
	
	// not found
	QMessageBox::information ( this, tr( "Find..." ), tr( "'%1' could not be found." ). arg ( searchString));
	goto done;

foundPage:
	
	gotoPage ( pg );
	
	if ( !m_outdev-> findText ( u, len, gTrue, gTrue, &xMin, &yMin, &xMax, &yMax )) {
		// this can happen if coalescing is bad
		goto done;
	}

found:
	selr. setCoords ( xMin, yMin, xMax, yMax );
	m_outdev-> setSelection ( selr, true ); // this will emit QPEOutputDev::selectionChanged ( ) -> copyToClipboard ( )

done:			

	delete [] u; 
	delete textOut;

	//TODO: Vishwajeet busy status is not there, add this feature and uncomment below line
	//setBusy ( false );
}

void CPrivate_QPdfWidget::openFile ( const QString &fileName )
{
	emit setbusy();
	QString fn = fileName;
	QFileInfo fi ( fn );

	if ( fi. exists ( )) {
		delete m_doc;

		m_doc = new PDFDoc ( new GString ( fn.toLocal8Bit( )), 0, 0 );

		if ( m_doc-> isOk ( )) {
			m_currentdoc = fi. baseName ( );
			int sep = m_currentdoc. lastIndexOf( '/' );
			if ( sep > 0 )
				m_currentdoc = m_currentdoc. mid ( sep + 1 );

			m_pages = m_doc-> getNumPages ( );
			m_currentpage = 0;
		
			QTimer::singleShot ( 0, this, SLOT( delayedInit()));
			emit fileexists();
		}
		else {
			delete m_doc;
			m_doc = 0;
			
			m_currentdoc = QString::null;
		}
	
		
	}
	else
	{
		QMessageBox::warning ( this, tr( "Error" ), tr( "File does not exist!" ));

	}

	emit setfree();

}

void CPrivate_QPdfWidget::delayedInit ( )
{
	
	m_currentpage = 1;
	m_zoom = 0;
	m_renderok = false;
	
	setZoom ( 60 );
	gotoPage ( 1 );
	
	m_renderok = true;
	
	renderPage ( );
	
	m_outdev-> setFocus ( );

}

void CPrivate_QPdfWidget::renderPage ( )
{

		if ( m_renderok && m_doc && m_doc-> isOk ( )) {
		m_renderok = false;
	
//		setBusy ( true );

		if(m_outdev == NULL)
		{
			qDebug("m_outdev");
		}

		if(m_doc == NULL)
		{
			qDebug("m_doc");
		}
				

		qDebug("%d",m_currentpage);
		m_doc-> displayPage ( m_outdev, m_currentpage, m_zoom, 0, true ); 
		//setBusy ( false );
		
		m_outdev-> setPageCount ( m_currentpage, m_pages );
		
		m_renderok = true;
	}

}
	
void CPrivate_QPdfWidget::resizeEvent ( QResizeEvent *e )
{
	
}

void CPrivate_QPdfWidget::focusInEvent ( QFocusEvent *e )
{
	
}

	
void CPrivate_QPdfWidget::paintEvent ( QPaintEvent * event )
{

	//QPainter paint(this);
	//if(m_pixmap != NULL)
	//paint.drawPixmap(0,0,*m_pixmap);
}

int CPrivate_QPdfWidget::getCurrentPageNumber() const
{
	return this->m_currentpage;
}

int CPrivate_QPdfWidget::getTotalPages() const
{
	return this->m_pages;
}

int CPrivate_QPdfWidget::getZoomLevel() const
{
	return this->m_zoom;
}

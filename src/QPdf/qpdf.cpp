//========================================================================
//
// qpdf.cc
//
// Copyright 2001 Robert Griebl
//
//========================================================================

#include "aconf.h"
#include "GString.h"
#include "PDFDoc.h"
#include "TextOutputDev.h"

#include "QPEOutputDev.h"

#include <QtGlobal>
#include <QInputDialog>
#include <QPixmap>
#include <QClipboard>
#include <QToolBar>
#include <QToolButton>
#include <QMenuBar>
#include <QTimer>
#include <QFileInfo>
#include <QString>
#include <QLineEdit>
#include <QSpinBox>
#include <QLayout>
#include <QDialog>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include "QPdfWidget.h"

#define QCString QByteArray
#define WStyle_Customize Qt::WStyle_Customize
#define WStyle_NoBorder Qt::WStyle_NoBorder
#define QSCROLLAREA_SUPPORT 0
#define NO_QPDFWIDGET 1


#include "qpdf.h"
#include <QKeyEvent>



int main ( int argc, char **argv ) 
{
#if defined(BAS_USE_QPE)		//BAS
	QPEApplication app ( argc, argv );
#else
	QApplication app ( argc, argv );	//BAS
#endif	//#if defined(BAS_USE_QPE)	//BAS

	//Q_INIT_RESOURCE("");
		// read config file
	globalParams = new GlobalParams ( "" );
	globalParams-> setErrQuiet ( true );

#if NO_QPDFWIDGET
	QPdfDlg *dlg = new QPdfDlg ( );				
#else
	QPdfWidget pdfWidget;
	
#endif

	
#if defined(BAS_USE_QPE)
	//BAS
	app. showMainDocumentWidget ( dlg );
#else
	
	#if NO_QPDFWIDGET

			#ifdef EKA2
			dlg->showFullScreen();
		#else
			dlg->show();
		#endif
	#else
		pdfWidget.show();
	#endif

	//qDebug("By Vishwajeet");
#endif	//#if defined(BAS_USE_QPE)	//BAS

	if (( app. argc ( ) == 3 ) && ( app. argv ( ) [1] == QCString ( "-f" )))
	{
		qDebug ( app. argv ( ) [2] );
		#if NO_QPDFWIDGET
			dlg-> openFile ( app. argv ( ) [2] );
		#endif
	}

	return app. exec ( );
}


QPdfDlg::QPdfDlg ( ) : QWidget ( )
{
	setupUi(this);
	//setupUi(this);

	this->setWindowTitle( tr( "QPdf" ));

	this->setWindowIcon( QIcon(QPixmap( "./qpdf_icon.png" )));
	isFileOpen = false;
	m_pMainMenuItems = new CMainMenuItems(this) ;
	m_pMainMenuItems->hide();
	m_pMainMenuItems->setGeometry(m_pMainMenuItems->x(),m_pMainMenuItems->height() * -1,m_pMainMenuItems->width(),m_pMainMenuItems->height());
	m_busy = false;
	m_docexists = false;

	m_pBusyBar = new QLabel(this);
	m_pBusyBar->hide();
	m_pBusyBar->setText("Loading ...");
	m_pBusyBar->setBackgroundRole(QPalette::Base);
	//m_pBusyBar-> setParameters ( 12, 8, 200 );

	m_fullscreen = false;
	m_renderok = false;

	QVBoxLayout *layout = new QVBoxLayout(this);
	m_pPdfWidget = new QPdfWidget (m_pPdfDocumentWindowRegion );
	
	timeLine = new QTimeLine(1000,this);
	timeLine->setFrameRange(m_pMainMenuItems->height() * -1,0);
    timeLine->setCurveShape(QTimeLine::EaseInCurve); 
	timeLine->setDirection(QTimeLine::Backward);
	//layout->addWidget(m_pMainMenuItems);	
	layout->addWidget(m_pPdfWidget);
	layout->setContentsMargins(0,0,0,0);
	m_pPdfDocumentWindowRegion ->setLayout(layout);
		
	//connect(m_pMenuButton,SIGNAL(clicked()),this,SLOT(show_UI_Input_For_User_And_Accept_Input_From_Him()));

	connect(this,SIGNAL(menuButtonPressed()),this,SLOT(show_UI_Input_For_User_And_Accept_Input_From_Him()));


	connect(this,SIGNAL(leftArrowPressed()),this,SLOT(scrollLeft()));
	connect(this,SIGNAL(rightArrowPressed()),this,SLOT(scrollRight()));
	connect(this,SIGNAL(bottomArrowPressed()),this,SLOT(scrollDown()));
	connect(this,SIGNAL(topArrowPressed()),this,SLOT(scrollUp()));
  
	connect(m_pPdfWidget,SIGNAL(setitsbusy()),this,SLOT(setbusyshow()));
	connect(m_pPdfWidget,SIGNAL(setitsfree()),this,SLOT(setbusyhide()));

	connect(m_pPdfWidget,SIGNAL(itsfileexists()),this,SLOT(enablemenuitems()));
	connect(m_pPdfWidget,SIGNAL(emittedpage(int)),this,SLOT(enablemenuitems(int)));
	connect(m_pPdfWidget,SIGNAL(itsfileclosed()),this,SLOT(setpagesnull()));
	connect(m_pPdfWidget,SIGNAL(itsfileclosed()),this,SLOT(enablemenuitems()));
	

	connect(m_pMainMenuItems,SIGNAL(itemSelected(QString)),this,SLOT(parseResponceOnSelectedMenuItem(QString)));
	connect(timeLine,SIGNAL(frameChanged(int)),this,SLOT(animatePopUp(int)));
	connect(timeLine,SIGNAL(finished()),this,SLOT(takeAction()));
	
  createMenuItems();
	zoomLevels << 50 << 75 << 100 << 125 << 150 << 200;
	

	//openFile("D:\\Projects\\QPdf_Viewer\\PDF_ Files\\accessbooklet.pdf");

	m_pMainMenuItems->disableMenuItem(QStringList() << "nextPage"<<"prevPage"<<"firstPage"<<"lastPage"<<"gotoPage"<<"zoomIn"<<"zoomOut"<<"close");

	m_pPdfDocument_Page_Status->setText(" ");

	this->grabKeyboard();
}


QPdfDlg::~QPdfDlg ( )
{
	delete m_pPdfWidget;
	delete m_pBusyBar;
	delete m_pMainMenuItems;
	delete timeLine;
}



// ^^ Fullscreen handling (for broken QT-lib)

void QPdfDlg::setBusy ( bool b )
{
//TODO @vishwajeet use busyBar class and add those functionality here 
#if 0
qDebug ("QPdfDlg::setBusy ");
	if ( b != m_busy ) {
		m_busy = b;
		
		m_outdev-> setBusy ( m_busy );
		setEnabled ( !m_busy );
	}
#endif

}


bool QPdfDlg::busy ( ) const
{
	//TODO @vishwajeet use busyBar class and add those functionality here 
#if 0
qDebug ("QPdfDlg::busy ");
	return m_busy;
#else
	return false;
#endif

}


void QPdfDlg::updateCaption ( )
{

	QString cap = "";
	
	if ( !m_currentdoc. isEmpty ( )) 
		cap = QString ( "%1 - " ). arg ( m_currentdoc );
	cap += "QPdf";
	
	// TODO VCD:add appropriate place to show status
	this->setWindowTitle ( cap );
}


void QPdfDlg::zoomIn()
{
	if(!isFileOpen)
	{
		return;
	}
		
	int index = zoomLevels.indexOf(currentZoomLevel);

	if(index < (zoomLevels.count() -1) )
	{
		this->setZoom( zoomLevels.at(index + 1));
		currentZoomLevel = zoomLevels.at(index + 1);
	}
	if(currentZoomLevel == 200)
		{
		//m_pMenuAction->actions().at(8)->setDisabled(true);
		}
	else
		{
		//m_pMenuAction->actions().at(8)->setEnabled(true);
		}
	//m_pMenuAction->actions().at(9)->setEnabled(true);
}

void QPdfDlg::zoomOut()
{
	if(!isFileOpen)
	{
		return;
	}

	int index = zoomLevels.indexOf(currentZoomLevel);

	if(index > 0 )
	{
		this->setZoom( zoomLevels.at(index - 1));
		currentZoomLevel = zoomLevels.at(index - 1);
	}
	if(currentZoomLevel == 50)
		{
		//m_pMenuAction->actions().at(9)->setDisabled(true);
		}
	else
		{
		//m_pMenuAction->actions().at(9)->setEnabled(true);
		}
	//m_pMenuAction->actions().at(8)->setEnabled(true);
}



void QPdfDlg::setZoom ( const int &z)
{
//TODO @vishwajeet use busyBar class and add those functionality here 

	m_pPdfWidget->setZoom(z);
}


void QPdfDlg::gotoPageDialog ( )
{
	if(!isFileOpen)
	{
		return;
	}

    bool ok;

	this->releaseKeyboard();
    int i = QInputDialog::getInteger(this, tr("Enter Page Number"),
		tr("Page Number:"), m_pPdfWidget->getCurrentPageNumber(), 1, m_pPdfWidget->getTotalPages(), 1, &ok);

	if(ok)
	{
		gotoPage(i);
	}
	this->grabKeyboard();
}

void QPdfDlg::toggleFindBar ( )
{

}

void QPdfDlg::findText ( const QString &str )
{
	//TODO @vishwajeet use busyBar class and add those functionality here 

	m_pPdfWidget->findText(str);
}
		

void QPdfDlg::findText ( )
{
	//findText ( m_findedit-> text ( ));
}


void QPdfDlg::firstPage ( )
{
	if(!isFileOpen)
	{
		return;
	}
	gotoPage ( 1 );
	m_pMainMenuItems->activateMenuItem(QStringList() << "nextPage"<<"prevPage"<<"lastPage"<<"gotoPage"<<"zoomIn"<<"zoomOut"<<"firstPage"<<"close");
	m_pMainMenuItems->disableMenuItem(QStringList() << "firstPage"<<"prevPage");
}

void QPdfDlg::prevPage ( )
{
	if(!isFileOpen)
	{
		return;
	}
	gotoPage ( m_pPdfWidget->getCurrentPageNumber() - 1 );

}

void QPdfDlg::nextPage ( )
{
	if(!isFileOpen)
	{
		return;
	}
	gotoPage ( m_pPdfWidget->getCurrentPageNumber() + 1 );
	
}

void QPdfDlg::lastPage ( )
{
	if(!isFileOpen)
	{
		return;
	}
	gotoPage ( m_pPdfWidget->getTotalPages() );
	m_pMainMenuItems->activateMenuItem(QStringList() << "nextPage"<<"prevPage"<<"lastPage"<<"gotoPage"<<"zoomIn"<<"zoomOut"<<"firstPage"<<"close");
	m_pMainMenuItems->disableMenuItem(QStringList() << "lastPage"<< "nextPage");
}

void QPdfDlg::gotoPage ( int n )
{
	if(!isFileOpen)
	{
		return;
	}
	m_pPdfWidget->gotoPage(n);
	m_pPdfDocument_Page_Status->setText(QString().setNum(m_pPdfWidget->getCurrentPageNumber()) + "/" + QString().setNum(m_pPdfWidget->getTotalPages()));
}


void QPdfDlg::openFile ( )
{
	this->releaseKeyboard();
	QString fileName = QFileDialog::getOpenFileName(0, tr("Open PDF File"),
#ifdef EKA2
			"E:\\data",
#else
                                                 "/home",
#endif
                                                 tr("PDF Files (*.pdf)"));

	openFile(fileName);
	if(m_pPdfWidget->getTotalPages() > 0)
	{
		m_pPdfDocument_Page_Status->setText(QString().setNum(m_pPdfWidget->getCurrentPageNumber() + 1) + "/" + QString().setNum(m_pPdfWidget->getTotalPages()));
		m_pMainMenuItems->activateMenuItem(QStringList() << "nextPage"<<"lastPage"<<"gotoPage"<<"zoomIn"<<"zoomOut"<<"close");
		isFileOpen = true;
	}
	currentZoomLevel = 100;
	this->grabKeyboard();
}

void QPdfDlg::scrollDown()
{
	if(m_pMainMenuItems->isVisible())
	{
		m_pMainMenuItems->selectBelowItem();
		return;
	}
	m_pPdfWidget->scroll(0,1);

}

void QPdfDlg::scrollUp()
{
	if(m_pMainMenuItems->isVisible())
	{
		m_pMainMenuItems->selectAboveItem();
		return;
	}

		m_pPdfWidget->scroll(0,-1);
}


void QPdfDlg::closefile()
{
	isFileOpen = false;
	m_pPdfWidget->closeitspdf();
	m_pMainMenuItems->disableMenuItem(QStringList() << "nextPage"<<"prevPage"<<"firstPage"<<"lastPage"<<"gotoPage"<<"zoomIn"<<"zoomOut"<<"close");
	
}

void QPdfDlg::setpagesnull()
{
    //TODO: vishwajeetkumar remove this function later since similar functionality is available in private_widget class
	m_pPdfDocument_Page_Status->setText(" ");
}
void QPdfDlg::scroll(QAction *currentSelectedAction,QPdfDlg::scrollMenuDirection direction )
{
}

void QPdfDlg::parseResponceOnSelectedMenuItem(QString item)
{
	timeLine->setDirection(QTimeLine::Backward);
	timeLine->start();
	m_pMainMenuItems->hide();

	if(item == "nextPage")
	{
		this->nextPage ( );

	}else if(item == "prevPage")
	{
		this->prevPage ( );
	}else if(item == "firstPage")
	{
		this->firstPage ( );
	}else if(item == "lastPage")
	{
		this->lastPage ( );
	}else if(item == "gotoPage")
	{
		this-> gotoPageDialog ( );
	}else if(item == "zoomIn")
	{
		this->zoomIn();
	}else if(item == "zoomOut")
	{
		this->zoomOut();
	}else if(item == "open")
	{
		this->openFile ( );
	}else if(item == "close")
	{
		this->closefile();
	}else if(item == "exit")
	{	
		this->close();
	}

}

void QPdfDlg::scrollLeft()
{
	if(m_pMainMenuItems->isVisible())
	{
		m_pMainMenuItems->selectLeftItem();
		return;
	}
	m_pPdfWidget->scroll(-1,0);
}

void QPdfDlg::scrollRight()
{
	if(m_pMainMenuItems->isVisible())
	{
		m_pMainMenuItems->selectRightItem();
		return;
	}
	m_pPdfWidget->scroll(1,0);
}


void QPdfDlg::createMenuItems()
{
}

void QPdfDlg::takeAction()
{
	if(timeLine->direction() == QTimeLine::Backward)
	{
		m_pMainMenuItems->hide();
	}
}

void QPdfDlg::animatePopUp(int value)
{
	if(timeLine->direction() == QTimeLine::Forward)
	{
		m_pMainMenuItems->setGeometry(m_pMainMenuItems->x(),value,m_pMainMenuItems->width(),m_pMainMenuItems->height());
	}else
	{
		m_pMainMenuItems->setGeometry(m_pMainMenuItems->x(),value,m_pMainMenuItems->width(),m_pMainMenuItems->height());	
	}

}

bool QPdfDlg::event ( QEvent * event )
{
	/*if(!this->isEnabled())
	{
		return false;
	}*/

	if(event->type() == QEvent::Paint )
	{
		if(m_pPdfWidget->getTotalPages() > 0)
		{
			m_pPdfDocument_Page_Status->setText(QString().setNum(m_pPdfWidget->getCurrentPageNumber() ) + "/" + QString().setNum(m_pPdfWidget->getTotalPages()));
		}else
		{
			m_pPdfDocument_Page_Status->setText(" ");
		}	
	}

	if(event->type() == QEvent::KeyPress)
	{
		QKeyEvent *keyboardEvent = (QKeyEvent *)event;
		switch (keyboardEvent->key())
		{

		//Select Key 
		case 16842752:
			if(m_pMainMenuItems->isVisible())
			{
				parseResponceOnSelectedMenuItem(m_pMainMenuItems->getSelectedItem());
				takeAction();
			}
			break;
		case 35 :
		case 42 :
		case 48 :	
			//Menu Key
		case 16777217:
			emit menuButtonPressed();
			break;
			// Up Key
		case 16777235:
			emit topArrowPressed();
			break;

			// Left key
		case 16777234:
			emit leftArrowPressed();
			break;

			// Right key
		case 16777236:
			emit rightArrowPressed();
			break;

			// Down key
		case 16777237:
			emit bottomArrowPressed();
			break;
		
			// 1 key
		case 49:
			parseResponceOnSelectedMenuItem("open");
			break;
			// 2 key
		case 50:
			parseResponceOnSelectedMenuItem("firstPage");
			break;
			// 3 key
		case 51:
			parseResponceOnSelectedMenuItem("close");
			break;
			
			// 4 key
		case 52:
			parseResponceOnSelectedMenuItem("prevPage");
			break;
			
			// 5 key
		case 53:
			parseResponceOnSelectedMenuItem("gotoPage");
			break;
			
			// 6 key
		case 54:
			parseResponceOnSelectedMenuItem("nextPage");
			break;
			
			// 7 key
		case 55:
			parseResponceOnSelectedMenuItem("zoomIn");
			break;
			
			// 8 key
		case 56:
			parseResponceOnSelectedMenuItem("lastPage");
			break;

			// 9 key
		case 57:
			parseResponceOnSelectedMenuItem("zoomOut");
			break;
			
		default:
			int key = keyboardEvent->key();
			// Select key
		//case 0:
		//	emit menuButtonPressed();
		//	break;
		}

	}
	/*else if(event->type() == QEvent::WindowDeactivate)
		{
			QKeyEvent *keyboardEvent = (QKeyEvent *)event;
			return true;
		}*/

	
	return QWidget::event(event);

}

void QPdfDlg::show_UI_Input_For_User_And_Accept_Input_From_Him()
{
	//m_pMainMenuItems->setGeometry(0,0,m_pMainMenuItems->width(),m_pMainMenuItems->height());
	m_pMainMenuItems->setFixedWidth(this->geometry().width());

	timeLine->toggleDirection();
	m_pMainMenuItems->show();
	timeLine->start();
}

void QPdfDlg::openFile ( const QString &f )
{
	m_pPdfWidget->openFile(f);
	m_docexists = true;
	//enablemenuitems(m_docexists);
//	pageStatus->setText(QString().setNum(m_pPdfWidget->getCurrentPageNumber()+1) + "/" + QString().setNum(m_pPdfWidget->getTotalPages()));
}

void QPdfDlg::setbusyshow()
{
	m_pBusyBar->setFixedSize(this->width() * 30 / 100 ,this->height()* 30 / 100);
	m_pBusyBar->setGeometry(this->x() + (this->width()/2) - (m_pBusyBar->width()/2),this->y() + (this->height()/2) - (m_pBusyBar->height()/2),m_pBusyBar->width(),m_pBusyBar->height());

//	m_pBusyBar-> beginBusy ( );
	m_pBusyBar-> show ( );
//	m_pMenuButton->setDisabled(true);
//	m_pBackButton->setDisabled(true);
	//this->setDisabled(true);
}
void QPdfDlg::setbusyhide()
{
	m_pBusyBar-> hide( );
//	m_pBusyBar-> endBusy ( );
//	m_pMenuButton->setDisabled(false);
//	m_pBackButton->setDisabled(false);
	//this->setDisabled(false);
}
void QPdfDlg::enablemenuitems(int n)
{
	if(n >= m_pPdfWidget->getTotalPages())
	{
		m_pMainMenuItems->activateMenuItem(QStringList() << "nextPage"<<"prevPage"<<"lastPage"<<"gotoPage"<<"zoomIn"<<"zoomOut"<<"firstPage"<<"close");
		m_pMainMenuItems->disableMenuItem(QStringList() << "lastPage"<< "nextPage");
	}else if(n <= 1)
	{
		m_pMainMenuItems->activateMenuItem(QStringList() << "nextPage"<<"prevPage"<<"lastPage"<<"gotoPage"<<"zoomIn"<<"zoomOut"<<"firstPage"<<"close");
		m_pMainMenuItems->disableMenuItem(QStringList() << "firstPage"<< "prevPage");	
	}else
	{
		m_pMainMenuItems->activateMenuItem(QStringList() << "nextPage"<<"prevPage"<<"lastPage"<<"gotoPage"<<"zoomIn"<<"zoomOut"<<"firstPage"<<"close");
	}
}


//TODO Added By VCD 
// NOTE : This is wrong place to add function defination for xpdf library 
// This is temporary setting 
GString *grabPath(char *fileName)
{
#ifdef VMS
  //---------- VMS ----------
  char *p;

  if ((p = strrchr(fileName, ']')))
    return new GString(fileName, p + 1 - fileName);
  if ((p = strrchr(fileName, ':')))
    return new GString(fileName, p + 1 - fileName);
  return new GString();

#elif defined(__EMX__) || defined(WIN32)
  //---------- OS/2+EMX and Win32 ----------
  char *p;

  if ((p = strrchr(fileName, '/')))
    return new GString(fileName, p - fileName);
  if ((p = strrchr(fileName, '\\')))
    return new GString(fileName, p - fileName);
  if ((p = strrchr(fileName, ':')))
    return new GString(fileName, p + 1 - fileName);
  return new GString();

#elif defined(ACORN)
  //---------- RISCOS ----------
  char *p;

  if ((p = strrchr(fileName, '.')))
    return new GString(fileName, p - fileName);
  return new GString();

#elif defined(MACOS)
  //---------- MacOS ----------
  char *p;

  if ((p = strrchr(fileName, ':')))
    return new GString(fileName, p - fileName);
  return new GString();

#else
  //---------- Unix ----------
  char *p;

  if ((p = strrchr(fileName, '/')))
    return new GString(fileName, p - fileName);
  return new GString();
#endif
}

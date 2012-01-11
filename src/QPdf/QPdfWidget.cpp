#include "QPdfWidget.h"

#include "Private_QPdfWidget.h"
#include <QHBoxLayout>

QPdfWidget::QPdfWidget( QWidget * parent , Qt::WindowFlags f ):QWidget(parent,f)
{
	QHBoxLayout *layout = new QHBoxLayout(this);

	m_pCPrivate_QPdfWidgetInstance = new CPrivate_QPdfWidget ();

	layout->addWidget(m_pCPrivate_QPdfWidgetInstance);
	
	layout->setContentsMargins(0,0,0,0);
	QObject::connect(m_pCPrivate_QPdfWidgetInstance,SIGNAL(setbusy()),this,SIGNAL(setitsbusy()));
	QObject::connect(m_pCPrivate_QPdfWidgetInstance,SIGNAL(setfree()),this,SIGNAL(setitsfree()));
	QObject::connect(m_pCPrivate_QPdfWidgetInstance,SIGNAL(fileexists()),this,SIGNAL(itsfileexists()));
	QObject::connect(m_pCPrivate_QPdfWidgetInstance,SIGNAL(emitpage(int)),this,SIGNAL(emittedpage(int)));
	QObject::connect(m_pCPrivate_QPdfWidgetInstance,SIGNAL(fileclosed()),this,SIGNAL(itsfileclosed()));
	setLayout(layout);
}

QPdfWidget::~QPdfWidget(void)
{
	delete m_pCPrivate_QPdfWidgetInstance;
}

void QPdfWidget::scroll(const int &x,const int &y)
{
	m_pCPrivate_QPdfWidgetInstance ->scroll(x,y);
}

void QPdfWidget::firstPage ( )
{
	m_pCPrivate_QPdfWidgetInstance ->firstPage();
}

void QPdfWidget::prevPage ( )
{
	m_pCPrivate_QPdfWidgetInstance->prevPage();
}

void QPdfWidget::nextPage ( )
{
	m_pCPrivate_QPdfWidgetInstance->nextPage();
}

void QPdfWidget::lastPage ( )
{
	m_pCPrivate_QPdfWidgetInstance->lastPage();
}

void QPdfWidget::gotoPage ( const int &n )
{
	m_pCPrivate_QPdfWidgetInstance->gotoPage(n);
}

void QPdfWidget::setZoom ( const int &z )
{
	m_pCPrivate_QPdfWidgetInstance->setZoom(z);
}
void QPdfWidget::closeitspdf()
	{
	m_pCPrivate_QPdfWidgetInstance->closepdf();
	}
void QPdfWidget::findText ( const QString &searchString )
{
	m_pCPrivate_QPdfWidgetInstance->findText(searchString);
}

void QPdfWidget::openFile ( const QString &fileName )
{
	m_pCPrivate_QPdfWidgetInstance->openFile(fileName);
}

int QPdfWidget::getCurrentPageNumber() const
{
	return m_pCPrivate_QPdfWidgetInstance->getCurrentPageNumber();
}

int QPdfWidget::getTotalPages() const
{
	return m_pCPrivate_QPdfWidgetInstance->getTotalPages();
}

int QPdfWidget::getZoomLevel() const
{
	return m_pCPrivate_QPdfWidgetInstance->getZoomLevel();
}

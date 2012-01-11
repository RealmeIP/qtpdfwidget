#ifndef CPRIVATE_QPDFWIDGET_H
#define CPRIVATE_QPDFWIDGET_H

#include <QWidget>
#include "aconf.h"

class PDFDoc;
class QScrollArea;
class QOutputDev;
class QPEOutputDev;

class CPrivate_QPdfWidget :	public QWidget
{
	Q_OBJECT

public:
	CPrivate_QPdfWidget( QWidget * parent = 0, Qt::WindowFlags f = 0 );
	virtual ~CPrivate_QPdfWidget(void);

	void firstPage ( );
	void prevPage ( );
	void nextPage ( );
	void lastPage ( );
	void gotoPage ( int n );	
	void setZoom ( const int &id );
	void findText ( const QString &searchString = QString() );	
	
	void openFile ( const QString &fileName );

	int getCurrentPageNumber() const;
	int getTotalPages() const;
	int getZoomLevel() const;
	void scroll(const int &x,const int &y);
	int getCurrentZoomInLevel() const;
	void closepdf();
	
	signals:
	void setbusy();
	void setfree();
	void fileexists();
	void emitpage(int i);
	void fileclosed();
private slots:

	void delayedInit ( );

protected:
	
	void renderPage ( );
	
	virtual void resizeEvent ( QResizeEvent *e );
	virtual void focusInEvent ( QFocusEvent *e );
	virtual void paintEvent ( QPaintEvent * event );

private:

	bool m_busy;
	bool m_renderok;
	
	int m_currentpage;
	int m_pages;
	int m_zoom;
	QString m_currentdoc;
	PDFDoc *m_doc;
	QScrollArea *scrollArea;
	QPEOutputDev *m_outdev;	

};

#endif 



#ifndef QPDFWIDGET_H_
#define QPDFWIDGET_H_
#include <QWidget>

class CPrivate_QPdfWidget;

/**

*/
class QPdfWidget :	public QWidget
{
	Q_OBJECT 

public:
	QPdfWidget( QWidget * parent = 0, Qt::WindowFlags f = 0 );
	virtual ~QPdfWidget(void);

	int getCurrentPageNumber() const;
	int getTotalPages() const;
	int getZoomLevel() const;
	void closeitspdf();
public slots:
	void firstPage ( );
	void prevPage ( );
	void nextPage ( );
	void lastPage ( );
	void gotoPage ( const int &n );	
	void setZoom ( const int &z );
	void findText ( const QString &searchString = QString() );	
	
	void openFile ( const QString &fileName );
	void scroll(const int &x,const int &y);

signals:
	void setitsbusy();
	void setitsfree();
	void itsfileexists();
	void emittedpage(int i);
	void itsfileclosed();
protected:


private:

	CPrivate_QPdfWidget *m_pCPrivate_QPdfWidgetInstance;

};
#endif 

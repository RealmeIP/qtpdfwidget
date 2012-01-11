#ifndef __QPDF_H__
#define __QPDF_H__

#include "aconf.h"


//#define QPDF_QPE_ONLY 1
#include <QToolButton>

#include <QMainWindow>
#include <QMenu>
#include <QStackedWidget>
#include <QTimeLine>

#include "ui_PdfApplication_UI_For_N97.h"
#include "qbusybar.h"
#include "MainMenuItems.h"

#define QPopupMenu QMenu
#define QWidgetStack QStackedWidget


class QPopupMenu;

class QPEOutputDev;
class PDFDoc;
class QLineEdit;
class QPdfWidget;
class QLabel;

class QPdfDlg : public QWidget,Ui::PdfUIWindow /*, public Ui::PdfDialog*/{
	Q_OBJECT

public:
	QPdfDlg ( );
	virtual ~QPdfDlg ( );

public slots:
	void firstPage ( );
	void prevPage ( );
	void nextPage ( );
	void lastPage ( );
	
	void gotoPage ( int n );
	
	void setZoom ( const int &z );

	void zoomIn();
	void zoomOut();
	
	void gotoPageDialog ( );


	void toggleFindBar ( );	

	void findText ( const QString & );	
	void findText ( );
	
	void openFile ( );
	void openFile ( const QString & );
	void enablemenuitems(int n=0);
	void closefile();
signals:
	void menuButtonPressed();
	void leftArrowPressed();
	void rightArrowPressed();
	void topArrowPressed();
	void bottomArrowPressed();
	void softKeyPressed(char key);
	
private slots:	
	
	// QPdfWidget method
	void updateCaption ( );
	void show_UI_Input_For_User_And_Accept_Input_From_Him();
	void scrollDown();
	void scrollUp();
	void scrollLeft();
	void scrollRight();
	void parseResponceOnSelectedMenuItem(QString);
	void animatePopUp(int value);
	void takeAction();
	
    //vishwajeet
	void setbusyshow();
	void setbusyhide();
	void setpagesnull();
protected:
	// QPdfWidget method
	void setBusy ( bool b = true );
	// QPdfWidget method
	bool busy ( ) const;

	virtual bool event ( QEvent * event );

	void createMenuItems();

private:
	enum scrollMenuDirection
	{
		Up,
		Down
	};

	void scroll(QAction *currentSelectedAction,QPdfDlg::scrollMenuDirection direction);
	

	
private:
	QPdfWidget *m_pPdfWidget;
	
	bool m_fullscreen;
	bool isFileOpen;
	bool m_busy;
	bool m_renderok;
	QString m_currentdoc;
	QList<int> zoomLevels;
	int currentZoomLevel;
	bool m_docexists;
	QLabel *m_pBusyBar;
	CMainMenuItems *m_pMainMenuItems;
	QTimeLine *timeLine;
};


#endif

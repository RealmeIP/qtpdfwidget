#include "MainMenuItems.h"

#include <QBitmap>
#include <QScrollBar>
#include <QGridLayout>

CMainMenuItems::CMainMenuItems(QWidget *parent):QDialog(parent)
{
	setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);
	setBackgroundRole(QPalette::Base);
selectedItem = 0;	

	setupMenuOption();

	
	
//	QPalette palette;

//	palette.setBrush(QPalette::Background,Qt::transparent);

	
}

QString CMainMenuItems::getSelectedItem()
{
	return menuList.at(selectedItem)->identity();
}

void CMainMenuItems::setupMenuOption()
{
	QGridLayout *layout = new QGridLayout(this);

	layout->setContentsMargins(24,0,0,0);
	layout->setSpacing(0);

	//-------------------------------------------------------------
	// Add Open file 
	//-------------------------------------------------------------
			CustomLabel *pCustomLabel = new CustomLabel(this);
			pCustomLabel->setIdentity("open");
			pCustomLabel->setToolTip("Select to open file dialog");
			menuList.append(pCustomLabel);
			pCustomLabel->setPixmap(QPixmap(":images/fileopenSmall.png"),QPixmap(":images/fileopenLarge.png")); 
			layout->addWidget(pCustomLabel,0,0);
			pCustomLabel->setSelected(true);
			selectedItem=0;
			connect(pCustomLabel,SIGNAL(mousePressed(QString)),this,SIGNAL(itemSelected(QString)));
		
	//-------------------------------------------------------------
	// Add First Page
	//-------------------------------------------------------------
			pCustomLabel = new CustomLabel(this);
			pCustomLabel->setIdentity("firstPage");
			pCustomLabel->setToolTip("Go to first page");
			menuList.append(pCustomLabel);
			pCustomLabel->setPixmap(QPixmap(":images/prevSmall.png"),QPixmap(":images/prevLarge.png")); 
			layout->addWidget(pCustomLabel,0,1);
			connect(pCustomLabel,SIGNAL(mousePressed(QString)),this,SIGNAL(itemSelected(QString)));

	//-------------------------------------------------------------
	// Add Close file
	//-------------------------------------------------------------
			pCustomLabel = new CustomLabel(this);
			pCustomLabel->setIdentity("close");
			pCustomLabel->setToolTip("Close Pdf document");
			menuList.append(pCustomLabel);
			pCustomLabel->setPixmap(QPixmap(":images/filecloseSmall.png"),QPixmap(":images/filecloseLarge.png")); 
			layout->addWidget(pCustomLabel,0,2);
			connect(pCustomLabel,SIGNAL(mousePressed(QString)),this,SIGNAL(itemSelected(QString)));

	//-------------------------------------------------------------
	// Add prev page
	//-------------------------------------------------------------
			pCustomLabel = new CustomLabel(this);
			pCustomLabel->setIdentity("prevPage");
			pCustomLabel->setToolTip("Go to previous page");
			menuList.append(pCustomLabel);
			pCustomLabel->setPixmap(QPixmap(":images/leftSmall.png"),QPixmap(":images/leftLarge.png")); 
			layout->addWidget(pCustomLabel,1,0);
			connect(pCustomLabel,SIGNAL(mousePressed(QString)),this,SIGNAL(itemSelected(QString)));

	//-------------------------------------------------------------
	// Add Goto page
	//-------------------------------------------------------------
			pCustomLabel = new CustomLabel(this);
			pCustomLabel->setIdentity("gotoPage");
			pCustomLabel->setToolTip("Go to page number");
			menuList.append(pCustomLabel);
			pCustomLabel->setPixmap(QPixmap(":images/gotoSmall.png"),QPixmap(":images/gotoLarge.png")); 
			layout->addWidget(pCustomLabel,1,1);
	
			connect(pCustomLabel,SIGNAL(mousePressed(QString)),this,SIGNAL(itemSelected(QString)));

	//-------------------------------------------------------------
	// Add right page
	//-------------------------------------------------------------
			pCustomLabel = new CustomLabel(this);
			pCustomLabel->setIdentity("nextPage");
			pCustomLabel->setToolTip("Go to next page");
			menuList.append(pCustomLabel);
			pCustomLabel->setPixmap(QPixmap(":images/nextSmall.png"),QPixmap(":images/nextLarge.png")); 
			layout->addWidget(pCustomLabel,1,2);
			connect(pCustomLabel,SIGNAL(mousePressed(QString)),this,SIGNAL(itemSelected(QString)));

	//-------------------------------------------------------------
	// Add Zoom in
	//-------------------------------------------------------------
			pCustomLabel = new CustomLabel(this);
			pCustomLabel->setIdentity("zoomIn");
			pCustomLabel->setToolTip("Zoom In");
			menuList.append(pCustomLabel);
			pCustomLabel->setPixmap(QPixmap(":images/zoominSmall.png"),QPixmap(":images/zoominLarge.png")); 
			layout->addWidget(pCustomLabel,2,0);
			connect(pCustomLabel,SIGNAL(mousePressed(QString)),this,SIGNAL(itemSelected(QString)));

	//-------------------------------------------------------------
	// Add last page
	//-------------------------------------------------------------
			pCustomLabel = new CustomLabel(this);
			pCustomLabel->setIdentity("lastPage");
			pCustomLabel->setToolTip("Go to last page");
			menuList.append(pCustomLabel);
			pCustomLabel->setPixmap(QPixmap(":images/lastSmall.png"),QPixmap(":images/lastLarge.png")); 
			layout->addWidget(pCustomLabel,2,1);
			connect(pCustomLabel,SIGNAL(mousePressed(QString)),this,SIGNAL(itemSelected(QString)));

	//-------------------------------------------------------------
	// Add Zoom out
	//-------------------------------------------------------------
			pCustomLabel = new CustomLabel(this);
			pCustomLabel->setIdentity("zoomOut");
			pCustomLabel->setToolTip("Zoom Out");
			menuList.append(pCustomLabel);
			pCustomLabel->setPixmap(QPixmap(":images/zoomoutSmall.png"),QPixmap(":images/zoomoutLarge.png")); 
			layout->addWidget(pCustomLabel,2,2);
			connect(pCustomLabel,SIGNAL(mousePressed(QString)),this,SIGNAL(itemSelected(QString)));


	//-------------------------------------------------------------
	// Add Exit 
	//-------------------------------------------------------------
			pCustomLabel = new CustomLabel(this);
			pCustomLabel->setIdentity("exit");
			pCustomLabel->setToolTip("Exit from the application");
			menuList.append(pCustomLabel);
			pCustomLabel->setPixmap(QPixmap(":images/appexitSmall.png"),QPixmap(":images/appexitLarge.png")); 
			layout->addWidget(pCustomLabel,3,0);
			connect(pCustomLabel,SIGNAL(mousePressed(QString)),this,SIGNAL(itemSelected(QString)));



	setLayout(layout);
}

void CMainMenuItems::activateMenuItem(const QStringList &itemList)
{
	for(int i=0; i<itemList.count();++i)
	{
		for(int j=0;j<menuList.count();++j)
		{
			if(itemList.at(i) == menuList.at(j)->identity())
			{
				menuList.at(j)->setDisabled(false);
			}
		}
	}
}

void CMainMenuItems::disableMenuItem(const QStringList &itemList)
{
	for(int i=0; i<itemList.count();++i)
	{
		for(int j=0;j<menuList.count();++j)
		{
			if(itemList.at(i) == menuList.at(j)->identity())
			{
				menuList.at(j)->setDisabled(true);
			}
		}
	}
}

CMainMenuItems::~CMainMenuItems(void)
{

}

void CMainMenuItems::selectRightItem()
{
	if(selectedItem > (menuList.count()-2))	
	{
		return;
	}
	menuList.at(selectedItem)->setSelected(false);
	selectedItem ++;

	if(menuList.at(selectedItem)->isEnabled())
	{
		menuList.at(selectedItem)->setSelected(true);
	}else
	{
		selectRightItem();
	}
}

void CMainMenuItems::selectLeftItem()
{
	if(selectedItem < 1)	
	{
		return;
	}

	menuList.at(selectedItem)->setSelected(false);
	selectedItem --;

	if(menuList.at(selectedItem)->isEnabled())
	{
		menuList.at(selectedItem)->setSelected(true);
	}else
	{
		selectLeftItem();
	}

}

void CMainMenuItems::selectAboveItem()
{
	int jumpTo = selectedItem - 3;

	if(jumpTo < 0)	
	{
		return;
	}

	menuList.at(selectedItem)->setSelected(false);
	selectedItem = jumpTo;

	if(menuList.at(selectedItem)->isEnabled())
	{
		menuList.at(selectedItem)->setSelected(true);
	}else
	{
		selectAboveItem();	
	}

}

void CMainMenuItems::selectBelowItem()
{
	int jumpTo = selectedItem + 3;

	if(jumpTo > (menuList.count()-1))	
	{
		return;
	}

	menuList.at(selectedItem)->setSelected(false);
	selectedItem = jumpTo;

	if(menuList.at(selectedItem)->isEnabled())
	{
		menuList.at(selectedItem)->setSelected(true);
	}else
	{
		selectBelowItem();
	}

}


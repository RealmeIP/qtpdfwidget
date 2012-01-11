#ifndef MAINMENUITEM_H
#define MAINMENUITEM_H
#include <QDialog>
#include "ui_PdfMenuItem.h"
#include "CustomLabel.h"

#include <QList>
#include <QScrollArea>

class CMainMenuItems :	public QDialog, public Ui::MainMenuItems
{
	Q_OBJECT
public:
	CMainMenuItems(QWidget *parent = 0);
	virtual ~CMainMenuItems(void);

	void disableMenuItem(const QStringList &itemList);
	void activateMenuItem(const QStringList &itemList);
	QString getSelectedItem();
signals:
	void itemSelected(QString);

public slots:
	void selectRightItem();
	void selectLeftItem();
	void selectAboveItem();
	void selectBelowItem();

private:
	void setupMenuOption();

private:
	QList<CustomLabel *> menuList;
	int selectedItem;

};
#endif

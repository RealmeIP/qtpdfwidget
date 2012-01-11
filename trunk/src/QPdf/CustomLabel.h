#ifndef CUSTOMLABEL_H
#define CUSTOMLABEL_H

#include <QLabel>

class CustomLabel :	public QLabel
{
	Q_OBJECT
public:
	CustomLabel(QWidget * parent = 0, Qt::WindowFlags f = 0);
	virtual ~CustomLabel(void);

	void setSelected(bool flag);
	void setPixmap(const QPixmap &small,const QPixmap &large);
	void setIdentity(const QString &identity);
	QString identity() const;

signals:
	void mousePressed(QString);

protected:
#ifndef EKA2
	void mousePressEvent ( QMouseEvent * event );
#endif	

private:
	QPixmap m_SmallPixmap;
	QPixmap m_LargePixmap;
	QString m_identity;
};
#endif


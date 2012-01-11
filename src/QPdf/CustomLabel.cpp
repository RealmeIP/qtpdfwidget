#include "CustomLabel.h"
#include <QToolTip>

CustomLabel::CustomLabel(QWidget * parent , Qt::WindowFlags f ):QLabel(parent,f)
{

}

CustomLabel::~CustomLabel(void)
{

}
#ifndef EKA2
void CustomLabel::mousePressEvent ( QMouseEvent * event )
{
	emit mousePressed(identity());
}
#endif

void CustomLabel::setPixmap(const QPixmap &small,const QPixmap &large)
{
	m_SmallPixmap = small;
	m_LargePixmap = large;
	QLabel::setPixmap(m_SmallPixmap);
}

void CustomLabel::setIdentity(const QString &identity)
{
	m_identity = identity;
}

QString CustomLabel::identity() const
{
	return m_identity;
}


void CustomLabel::setSelected(bool flag)
{
	if(flag)
	{
		QLabel::setPixmap(m_LargePixmap);
		QToolTip::showText(QPoint(this->frameGeometry().x(),this->frameGeometry().y()),this->toolTip(),this);
	}else
	{
		QLabel::setPixmap(m_SmallPixmap);
	}
}


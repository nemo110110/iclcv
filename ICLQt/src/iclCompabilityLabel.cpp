#include "iclCompabilityLabel.h"
#include <QPainter>

namespace icl{

  CompabilityLabel::CompabilityLabel(const QString &text, QWidget *parent):
    QLabel("placeholder",parent),m_sText(text){
    
  }
  
  
  void CompabilityLabel::paintEvent(QPaintEvent *evt){
    m_oMutex.lock();
    QString t = m_sText;
    m_oMutex.unlock();

    //    QWidget::paintEvent(evt); do i need this explicitly ??
    QPainter p(this);
    p.setPen(QColor(0,0,0));
    p.drawText(QRect(0,0,width(),height()),Qt::AlignCenter,t);
  }
  void CompabilityLabel::setNum(int i){
    m_oMutex.lock();
    m_sText = QString::number(i);
    m_oMutex.unlock();
  }
  void CompabilityLabel::setNum(float f){
    m_oMutex.lock();
    m_sText = QString::number(f);
    m_oMutex.unlock();
  }
  void CompabilityLabel::setText(const QString &text){
    m_oMutex.lock();
    m_sText = text;
    m_oMutex.unlock();
  }

  QString CompabilityLabel::text() const{
    QString retVal;
    const_cast<CompabilityLabel*>(this)->m_oMutex.lock();
    retVal = m_sText;
    const_cast<CompabilityLabel*>(this)->m_oMutex.unlock();
    return retVal;
  }

}



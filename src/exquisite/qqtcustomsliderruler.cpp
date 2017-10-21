﻿#include "qqtcustomsliderruler.h"
#include "qpainter.h"
#include "qevent.h"
#include "qtimer.h"
#include "qdebug.h"

QQtCustomSliderRuler::QQtCustomSliderRuler(QWidget* parent) :
    QWidget(parent)
{
    value = 0.0;
    minValue = 0.0;
    maxValue = 100.0;

    precision = 0;
    longStep = 10;
    shortStep = 1;
    space = 20;

    bgColorStart = QColor(100, 100, 100);
    bgColorEnd = QColor(60, 60, 60);
    lineColor = QColor(255, 255, 255);

    sliderColorTop = QColor(100, 184, 255);
    sliderColorBottom = QColor(235, 235, 235);

    tipBgColor = QColor(255, 255, 255);
    tipTextColor = QColor(50, 50, 50);

    pressed = false;
    currentValue = 0;
    sliderLastPot = QPointF(space, longLineHeight / 2);
}

void QQtCustomSliderRuler::resizeEvent(QResizeEvent*)
{
    resetVariables();
    setValue(currentValue);
}

void QQtCustomSliderRuler::wheelEvent(QWheelEvent* e)
{
    /*滚动的角度,*8就是鼠标滚动的距离*/
    int degrees = e->delta() / 8;

    /*滚动的步数,*15就是鼠标滚动的角度*/
    int steps = degrees / 15;

    /*如果是正数代表为左边移动,负数代表为右边移动*/
    if (e->orientation() == Qt::Vertical)
    {
        double value = currentValue - steps;

        if (steps > 0)
        {
            if (value > minValue)
            {
                setValue(value);
            }
            else
            {
                setValue(minValue);
            }
        }
        else
        {
            if (value < maxValue)
            {
                setValue(value);
            }
            else
            {
                setValue(maxValue);
            }
        }
    }
}

void QQtCustomSliderRuler::mousePressEvent(QMouseEvent* e)
{
    if (e->button() & Qt::LeftButton)
    {
        if (sliderRect.contains(e->pos()))
        {
            pressed = true;
            update();
        }
    }
}

void QQtCustomSliderRuler::mouseReleaseEvent(QMouseEvent* e)
{
    pressed = false;
    update();
}

void QQtCustomSliderRuler::mouseMoveEvent(QMouseEvent* e)
{
    if (pressed)
    {
        if (e->pos().x() >= lineLeftPot.x() && e->pos().x() <= lineRightPot.x())
        {
            double totalLineWidth = lineRightPot.x() - lineLeftPot.x();
            double dx = e->pos().x() - lineLeftPot.x();
            double ratio = (double)dx / totalLineWidth;
            sliderLastPot = QPointF(e->pos().x(), sliderTopPot.y());

            currentValue = (maxValue - minValue) * ratio + minValue;
            this->value = currentValue;
            emit valueChanged(currentValue);
            update();
        }
    }
}

void QQtCustomSliderRuler::paintEvent(QPaintEvent*)
{
    /*绘制准备工作,启用反锯齿*/
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    /*绘制背景*/
    drawBg(&painter);
    /*绘制标尺*/
    drawRule(&painter);
    /*绘制滑块*/
    drawSlider(&painter);
    /*绘制当前值的提示*/
    drawTip(&painter);
}

void QQtCustomSliderRuler::drawBg(QPainter* painter)
{
    painter->save();
    painter->setPen(Qt::NoPen);
    QLinearGradient bgGradient(0, 0, 0, height());
    bgGradient.setColorAt(0.0, bgColorStart);
    bgGradient.setColorAt(1.0, bgColorEnd);
    painter->setBrush(bgGradient);
    painter->drawRect(rect());
    painter->restore();
}

void QQtCustomSliderRuler::drawRule(QPainter* painter)
{
    painter->save();
    painter->setPen(lineColor);

    /*绘制横向标尺底部线,居中*/
    double initX = space;
    double initY = (double)height() / 2;
    lineLeftPot = QPointF(initX, initY);
    lineRightPot = QPointF(width() - initX, initY);
    painter->drawLine(lineLeftPot, lineRightPot);

    /*绘制纵向标尺刻度  */
    double length = width() - 2 * space;
    /*计算每一格移动多少*/
    double increment = length / (maxValue - minValue);

    /*根据范围值绘制刻度值及刻度值*/
    for (int i = minValue; i <= maxValue; i = i + shortStep)
    {
        if (i % longStep == 0)
        {
            QPointF topPot(initX, initY - longLineHeight);
            QPointF bottomPot(initX, initY);
            painter->drawLine(topPot, bottomPot);

            QString strValue = QString("%1").arg((double)i, 0, 'f', precision);
            double textWidth = fontMetrics().width(strValue);
            double textHeight = fontMetrics().height();
            QPointF textPot(initX - textWidth / 2, initY + textHeight);
            painter->drawText(textPot, strValue);
        }
        else
        {
            QPointF topPot(initX, initY - shortLineHeight);
            QPointF bottomPot(initX, initY);
            painter->drawLine(topPot, bottomPot);
        }

        initX += increment * shortStep;
    }

    painter->restore();
}

void QQtCustomSliderRuler::drawSlider(QPainter* painter)
{
    painter->save();

    /*绘制滑块上部分三角形*/
    sliderTopPot = QPointF(sliderLastPot.x(), lineLeftPot.y() - longLineHeight / 4);
    sliderLeftPot = QPointF(sliderTopPot.x() - width() / 100, sliderTopPot.y() + sliderTopHeight);
    sliderRightPot = QPointF(sliderTopPot.x() + width() / 100, sliderTopPot.y() + sliderTopHeight);

    painter->setPen(sliderColorTop);
    painter->setBrush(sliderColorTop);

    QVector<QPointF> potVec;
    potVec.append(sliderTopPot);
    potVec.append(sliderLeftPot);
    potVec.append(sliderRightPot);
    painter->drawPolygon(potVec);

    /*绘制滑块下部分矩形*/
    double indicatorLength = sliderRightPot.x() - sliderLeftPot.x();

    QPointF handleBottomRightPot(sliderLeftPot.x() + indicatorLength, sliderLeftPot.y() + sliderBottomHeight);
    sliderRect = QRectF(sliderLeftPot, handleBottomRightPot);

    QPointF tipRectTopLeftPot(sliderRect.topRight().x() + 2, sliderRect.topRight().y());
    QString strValue = QString("%1").arg(currentValue, 0, 'f', precision);

    double textLength = fontMetrics().width(strValue);
    double textHeight = fontMetrics().height();
    QPointF tipRectBottomRightPot(tipRectTopLeftPot.x() + textLength + 10, tipRectTopLeftPot.y() + textHeight + 5);
    tipRect = QRectF(tipRectTopLeftPot, tipRectBottomRightPot);

    painter->setPen(sliderColorBottom);
    painter->setBrush(sliderColorBottom);
    painter->drawRect(sliderRect);
    painter->restore();
}

void QQtCustomSliderRuler::drawTip(QPainter* painter)
{
    if (!pressed)
    {
        return;
    }

    painter->save();
    painter->setPen(tipTextColor);
    painter->setBrush(tipBgColor);
    painter->drawRect(tipRect);
    QString strValue = QString("%1").arg(currentValue, 0, 'f', precision);
    painter->drawText(tipRect, Qt::AlignCenter, strValue);
    painter->restore();
}

void QQtCustomSliderRuler::resetVariables()
{
    longLineHeight = height() / 5;
    shortLineHeight = height() / 7;
    sliderTopHeight = height() / 7;
    sliderBottomHeight = height() / 6;
}

void QQtCustomSliderRuler::setRange(double minValue, double maxValue)
{
    /*如果最小值大于或者等于最大值以及最小值小于0则不设置*/
    if (minValue >= maxValue || minValue < 0)
    {
        return;
    }

    this->minValue = minValue;
    this->maxValue = maxValue;
    setValue(minValue);
}

void QQtCustomSliderRuler::setRange(int minValue, int maxValue)
{
    setRange((double)minValue, (double)maxValue);
}

void QQtCustomSliderRuler::setValue(double value)
{
    /*值小于最小值或者值大于最大值则无需处理*/
    if (value < minValue || value > maxValue)
    {
        return;
    }

    double lineWidth = width() - 2 * space;
    double ratio = (double)value / (maxValue - minValue);
    double x = lineWidth * ratio;
    double newX = x + space;
    double y = space + longLineHeight - 10;
    sliderLastPot = QPointF(newX, y);

    this->value = value;
    this->currentValue = value;
    emit valueChanged(value);
    update();
}

void QQtCustomSliderRuler::setValue(int value)
{
    setValue((double)value);
}

void QQtCustomSliderRuler::setPrecision(int precision)
{
    this->precision = precision;
    update();
}

void QQtCustomSliderRuler::setStep(int longStep, int shortStep)
{
    /*短步长不能超过长步长*/
    if (longStep < shortStep)
    {
        return;
    }

    this->longStep = longStep;
    this->shortStep = shortStep;
    update();
}

void QQtCustomSliderRuler::setSpace(int space)
{
    this->space = space;
    update();
}

void QQtCustomSliderRuler::setBgColor(QColor bgColorStart, QColor bgColorEnd)
{
    this->bgColorStart = bgColorStart;
    this->bgColorEnd = bgColorEnd;
    update();
}

void QQtCustomSliderRuler::setLineColor(QColor lineColor)
{
    this->lineColor = lineColor;
    update();
}

void QQtCustomSliderRuler::setSliderColor(QColor sliderColorTop, QColor sliderColorBottom)
{
    this->sliderColorTop = sliderColorTop;
    this->sliderColorBottom = sliderColorBottom;
    update();
}

void QQtCustomSliderRuler::setTipBgColor(QColor tipBgColor, QColor tipTextColor)
{
    this->tipBgColor = tipBgColor;
    this->tipTextColor = tipTextColor;
    update();
}
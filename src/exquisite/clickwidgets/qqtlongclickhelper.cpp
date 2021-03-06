#include "qqtlongclickhelper.h"

QQtLongClickHelper::QQtLongClickHelper ( QObject* parent ) : QQtClickHelper ( parent )
{
    mLongClickInterval = longClickInterval;

    now_press = QTime::currentTime().addMSecs ( 1 );
    now_release = QTime::currentTime().addMSecs ( 2 );

    nLongClickNum = 0;
    nLongClickNumWithCancel = 0;
}

QQtLongClickHelper::~QQtLongClickHelper()
{

}

void QQtLongClickHelper::mousePressEvent ( QMouseEvent* event, QWidget* userWidget )
{
    p2debug() << "press" << event->pos() << userWidget;
    mPoint = event->pos();
    mClickType = QQtClick;
    now_press = QTime::currentTime();
}

void QQtLongClickHelper::mouseReleaseEvent ( QMouseEvent* event, QWidget* userWidget )
{
    p2debug() << "release" << event->pos() << userWidget;
    //这一次 current click
    QTime t2_release_preview = now_release;
    now_release = QTime::currentTime();
    //这里加了个判断,其实肯定>=0
    if ( now_press.msecsTo ( now_release ) >= 0 && now_press.msecsTo ( now_release ) <= mLongClickInterval )
    {
        //单击发生
        mClickType = QQtClick;
    }
    else if ( now_press.msecsTo ( now_release ) >= 0 && now_press.msecsTo ( now_release ) > mLongClickInterval )
    {
        //长击发生
        mClickType = QQtLongClick;

        //这次点击,
        //上次的release竟然比press晚,说明press没发生,置位click
        //第一次click, 上一次release=press也在这里被置位click
        if ( t2_release_preview.msecsTo ( now_release ) >= 0
             && t2_release_preview.msecsTo ( now_press ) <= 0 )
        {
            mClickType = QQtClick;
        }

    }

    if ( mClickType == QQtLongClick )
    {
        //计算点击数目
        checkClickNumWithCancel();

        //修改状态
        mClickType = QQtNullClick;

        //如果手移开了,那么不发送.
        //条件是用户传入了这个Widget为真.
        if ( userWidget )
        {
            p2debug() << userWidget <<  userWidget->rect() << event->pos() << userWidget->rect().contains ( event->pos() );
            if ( !userWidget->rect().contains ( event->pos() ) )
            {
                p2debug() << "send long click canceled." ;
                return;
            }
        }

        checkClickNum ( QQtLongClick );

        //发送长信号
        p2debug() << "send long click " ;
        emit longClick();
        emit longClickWithPoint ( event->pos() );
        emit longClickWithPointF ( event->localPos() );

#if 0
        emit longClick ( userWidget );
        emit longClickWithPoint ( event->pos(), userWidget );
        emit longClickWithPointF ( event->localPos(), userWidget );
#endif
        return;
    }
    else if ( mClickType == QQtClick )
    {
        //计算点击数目
        checkClickNumWithCancel();

        //修改状态
        mClickType = QQtNullClick;

        //如果手移开了,那么不发送.
        //条件是用户传入了这个Widget为真.
        if ( userWidget )
        {
            p2debug() << userWidget <<  userWidget->rect() << event->pos() << userWidget->rect().contains ( event->pos() );
            if ( !userWidget->rect().contains ( event->pos() ) )
            {
                p2debug() << "send click canceled." ;
                return;
            }
        }

        checkClickNum ( QQtClick );

        //发送单击信号
        p2debug() << "send click." ;
        emit click();
        emit clickWithPoint ( event->pos() );
        emit clickWithPointF ( event->localPos() );

#if 0
        emit click ( userWidget );
        emit clickWithPoint ( event->pos(), userWidget );
        emit clickWithPointF ( event->localPos(), userWidget );
#endif

        return;
    }

    //修改状态
    mClickType = QQtNullClick;

    //保存位置
    mPoint = event->pos();
    return;

}

void QQtLongClickHelper::mouseDoubleClickEvent ( QMouseEvent* event, QWidget* userWidget )
{
    p2debug() << "double click" << event->pos() << userWidget;
    mPoint = event->pos();
}

void QQtLongClickHelper::setLongClickInterval ( int millSecond )
{
    mLongClickInterval = millSecond;
}

int QQtLongClickHelper::getLongClickInterval() const
{
    return mLongClickInterval;
}


void QQtLongClickHelper::checkClickNumWithCancel()
{
    QQtClickHelper::checkClickNumWithCancel();
    switch ( mClickType )
    {
        case QQtLongClick:
        {
            nLongClickNumWithCancel++;
            if ( nLongClickNumWithCancel >= 0xFFFFFFFF )
            {
                p2debug() << "out......";
                nLongClickNumWithCancel = 0;
            }
        }
        break;
        default:
            break;
    }

    nTotalClickNumWithCancel = nClickNumWithCancel + nLongClickNumWithCancel;
    if ( nTotalClickNumWithCancel >= 0xFFFFFFFFFFFFFFFF )
    {
        p2debug() << "out......";
        nTotalClickNumWithCancel = 0;
    }
}

void QQtLongClickHelper::checkClickNum ( QQtLongClickHelper::QQtClickType type )
{
    QQtClickHelper::checkClickNum ( type );
    switch ( type )
    {
        case QQtLongClick:
        {
            nLongClickNum++;
            if ( nLongClickNum >= 0xFFFFFFFF )
            {
                p2debug() << "out......";
                nLongClickNum = 0;
            }
        }
        break;
        default:
            break;
    }

    nTotalClickNum = nClickNum + nLongClickNum;
    if ( nTotalClickNum >= 0xFFFFFFFFFFFFFFFF )
    {
        p2debug() << "out......";
        nTotalClickNum = 0;
    }
}

﻿#ifndef QQTPROTOCOLMANAGER_H
#define QQTPROTOCOLMANAGER_H

#include <QObject>
#include "qqtmessage.h"
#include "qqtprotocol.h"
#include <qqtobjectmanager.h>

#include "qqtcore.h"
#include <qqt-local.h>

/**
 * @brief The QQtProtocolManager class
 * very good ideology and impliment
 * 用于管理多个Protocol实例
 * 用户实现的协议注册到这里，当协议当中有消息需要告知业务层，从这里告知。
 *
 * 注释：
 * 讲解notify过程，
 * QQtSocketServer，管理QQtSocketClient句柄
 * QQtProtocolManager，管理QQtProtocol句柄
 * cli -> proto, dispatcher, recvFunc, notifyToProtocolManager,
 * protoManager -> businessLevel, 完全信号传递。
 *
 * 关于识别Protocl，
 * 用户的客户端可以发个人信息上来，Protocol保存在句柄内部。
 *
 * 定位：
 * 对同种类型的协议句柄，可以管理1024个。
 */
class QQTSHARED_EXPORT QQtProtocolManager : public QObject
{
    Q_OBJECT
public:
    explicit QQtProtocolManager ( QObject* parent = 0 ) : QObject ( parent ) {
    }
    virtual ~QQtProtocolManager() {

    }

    //获取Protocol列表
    //这里列举的函数是给BusinessLevel用的，Protocol里面不要用
    //findProtocolInstanceByXXX(...);
    //findClientInfoByProtocolInstance(Protocol);
    //sendMessageToProtocolInstance(Protocol, Message);
    //sendMessageToAllProtocolInstance(...);

signals:
    /**
     * @brief notifyToUser
     * 这个信号是给用户的，用户可以接收下来，转换为自己的协议使用。注释：如果必要。
     * 用户使用这个ProtocolManager，Protocol需要提交给用户处理的命令，提交到这里，在这里提交给用户。
     * ProtocolManager -> BusinessLevel
     * @param protocol
     * @param message
     */
    void notifyToBusinessLevel ( const QQtProtocol* protocol, const QQtMessage* message );

public:
    /**
     * 注册用户协议类型
     * 模板函数
     * 用于ProtocolManager内部生成用户协议实例。
     * 这个用户在生成ProtocolManager对象的时候，需要注册一下自己的协议类，需要调用一次。
     * 注意：
     * registerProtocol<QQtXXXProtocol>();
     */
    template <typename T>
    bool registerProtocol () {
        for ( int i = 0; i < 1024; i++ ) {
            QQtProtocol* p0 = new T ( this );
            m_protocol_list.push_back ( p0 );
        }
        return true;
    }

    /**
     * 以下和用户无关
     */
public:
    /**
     * @brief createProtocol
     * 这个函数给QQtSocketServer用的，不是给用户用的。
     * 用于生成用户协议对象实例。
     * @param protocolTypeName
     * @return
     */
    QQtProtocol* createProtocol () {
        if ( m_protocol_list.isEmpty() )
            return NULL;

        //无论如何，使用对象工厂也一样，都不能正确生成。对象工厂崩溃退出。
        //QQtProtocol* p0 = ( QQtProtocol* ) mProtocol->metaObject()->newInstance ( Q_ARG(QQtProtocolManager*, this) );
        QQtProtocol* p0 = findDetachedInstance();
        if ( p0 == 0 )
            return NULL;
        pmeta ( p0 ) << p0;

        //帮助Protocol给用户发数据。
        connect ( p0, SIGNAL ( notifyToProtocolManager ( const QQtProtocol*, const QQtMessage* ) ),
                  this, SIGNAL ( notifyToBusinessLevel ( const QQtProtocol*, const QQtMessage* ) ) );
        return p0;
    }

protected:
    QQtProtocol* findDetachedInstance() {
        QListIterator<QQtProtocol*> itor ( m_protocol_list );
        while ( itor.hasNext() ) {
            QQtProtocol* p = itor.next();
            if ( p->detached() )
                return p;
        }
        return NULL;
    }
private:
    QList<QQtProtocol*> m_protocol_list;
};

#endif // QQTPROTOCOLMANAGER_H
/*
    Copyright (C) 2011  Francesco Cecconi <francesco.cecconi@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "maindiscover.h"
#include "memorytools.h"

namespace discoverLayer {
    QStringList m_ipSospended;
    QObject *m_parent;
    int ScanCounter;
    int threadLimit;
    QTimer *timer;
    bool connectState;
    QStringList parameters_;
}

pingInterface::mainDiscover::mainDiscover(int uid) 
    : ipState(false),
      uid_(uid)
{
    discoverLayer::timer = new QTimer(this);
    discoverLayer::connectState = false;
    discoverLayer::ScanCounter = 0;
    discoverLayer::threadLimit = 20;
}

pingInterface::mainDiscover::~mainDiscover()
{
    delete discoverLayer::timer;
    discoverLayer::m_ipSospended.clear();
    discoverLayer::parameters_.clear();
    memory::freelist<QProcessThread*>::itemDeleteAllWithWait(_threadList);
}

QList<QNetworkInterface> pingInterface::mainDiscover::getAllInterfaces() const
{
    /*
     * return all loca interfaces
     */
    return QNetworkInterface::allInterfaces();
}

QList<QNetworkAddressEntry> pingInterface::mainDiscover::getAddressEntries(QNetworkInterface interface) const
{
    /*
     * return all entries for a single QNeworkInterface
     */
    return interface.addressEntries();
}

QList<QNetworkAddressEntry> pingInterface::mainDiscover::getAddressEntries(const QString interfaceName) const
{
    /*
     * return all entries for a single interface name
     */
    QNetworkInterface interface_ = QNetworkInterface::interfaceFromName(interfaceName);
    
    QList<QNetworkAddressEntry> entryList_;
    
    if (interface_.isValid()) 
    {
        return interface_.addressEntries();
    } 
    else 
    {
        return entryList_;
    }
}

void pingInterface::mainDiscover::isUp(const QStringList networkIpList, QObject *parent, QStringList parameters) 
{
    foreach (const QString& host, networkIpList)
    {
        isUp(host, parent, parameters);
    }
}

void pingInterface::mainDiscover::isUp(const QString networkIp, QObject *parent, QStringList parameters) 
{
    /*
     * start thread for discover ip state
     */
    QByteArray pingBuffer;
    QByteArray bufferError;
    discoverLayer::m_parent = parent;
    discoverLayer::parameters_ = parameters;
    // Create parameters list for npig
    parameters.append("-c 1");
    parameters.append("-v4");
    parameters.append(networkIp);
    
    if (discoverLayer::threadLimit) 
    {
        // acquire one element from thread counter
        discoverLayer::threadLimit--;
        discoverLayer::ScanCounter++;
        QPointer<QProcessThread> pingTh = new QProcessThread("nping", pingBuffer, bufferError, parameters);
        _threadList.push_back(pingTh);
        pingTh->start();
        connect(pingTh, SIGNAL(threadEnd(QStringList,QByteArray,QByteArray)),
            this, SLOT(threadReturn(QStringList,QByteArray,QByteArray)));
    } 
    else 
    {
        qDebug() << "DEBUG:: thread suspended:: " << networkIp;
        // create a QStringlist with address suspended
        discoverLayer::m_ipSospended.append(networkIp);
    }
    
    if (!discoverLayer::connectState) 
    {
        connect(parent, SIGNAL(killDiscover()), this, SLOT(stopDiscover()));
    }
    
    // check suspended discover ip
    if (discoverLayer::m_ipSospended.size() && !discoverLayer::connectState) 
    {
        discoverLayer::connectState = true;
        connect(discoverLayer::timer, SIGNAL(timeout()), this, SLOT(repeatScanner()));
        if (!discoverLayer::timer->isActive()) 
        {
            discoverLayer::timer->start(5000);
        }
    }
}

void pingInterface::mainDiscover::threadReturn(QStringList ipAddr, QByteArray ipBuffer, QByteArray bufferError)
{
    Q_UNUSED(bufferError);
    /*
     * Signal return, send data to discoverCalls
     */
    // increment thread limit, new ip discover is possible
    discoverLayer::threadLimit++;
    // remove ip from counter
    discoverLayer::ScanCounter--;
    QString buffString(ipBuffer);
    QTextStream buffStream(&buffString);
    QString buffLine;
    
    while(!buffStream.atEnd()) 
    {
        buffLine = buffStream.readLine();
        if (buffLine.startsWith(QLatin1String("RCVD")) || buffLine.startsWith(QLatin1String("RECV"))) 
        {
            emit endPing(ipAddr, true, ipBuffer);
            return;
        }
    }
    emit endPing(ipAddr, false, ipBuffer);
}

void pingInterface::mainDiscover::repeatScanner()
{
    /*
     * Recall discover for ip suspended 
     */
    qDebug() << "DEBUG:: scan Counter timer:: " << discoverLayer::ScanCounter;
    if (!discoverLayer::ScanCounter) 
    {
        disconnect(this, SLOT(repeatScanner()));        
        discoverLayer::connectState = false;
        discoverLayer::timer->stop();
        int lengthMin_ = discoverLayer::threadLimit;
    
        if (lengthMin_ > discoverLayer::m_ipSospended.size()) 
        {
            lengthMin_ = discoverLayer::m_ipSospended.size();
            for (int index = 0; index < lengthMin_; index++) 
            {
                isUp(discoverLayer::m_ipSospended.takeFirst(), discoverLayer::m_parent, discoverLayer::parameters_);
            }
        } 
        else 
        {
            for (int index = 0; index < lengthMin_; index++) 
            {
                isUp(discoverLayer::m_ipSospended.takeFirst(), discoverLayer::m_parent, discoverLayer::parameters_);
            }
        }
    }
}

void pingInterface::mainDiscover::stopDiscover()
{
    /*
     * disconnect timer slot and stop it
     */
    disconnect(this, SLOT(repeatScanner()));
    discoverLayer::timer->stop();
}
/*
Copyright 2007-2012  Francesco Cecconi <francesco.cecconi@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "config-nmapsi4.h"

#include "ui_scanwidget.h"

// Qt5 include
#include <QtCore/QHash>
#include <QtCore/QWeakPointer>
#include <QtCore/QTimer>
#include <QtCore/QList>
#include <QtCore/QFile>
#include <QMainWindow>
#include <QMessageBox>
#include <QSplitter>
#include <QCompleter>
#include <QStringListModel>
#include <QCloseEvent>
#include <QClipboard>
#include <QtQuick/QQuickView>
#include <QtQml/QQmlContext>

// local include
#include "preferencesdialog.h"
#include "debug.h"
#include "addparameterstobookmark.h"
#include "memorytools.h"
#include "monitor.h"
#include "utilities.h"
#include "hostutilities.h"
#include "vulnerability.h"
#include "discovermanager.h"
#include "parsermanager.h"
#include "profilermanager.h"
#include "bookmarkmanager.h"
#include "actionmanager.h"
#include "notify.h"
#include "profilehandler.h"
#include "mouseeventfilter.h"
#include "about.h"
#include "qmlwelcome.h"

// system
#if !defined(Q_OS_WIN32)
#include <unistd.h>
#endif

class ScanWidget : public QWidget, public Ui::ScanWidgetForm
{
   Q_OBJECT
public:
    explicit ScanWidget(QWidget* parent = 0);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();
    void updateComboBook();
    void updateCompleter();
    void buildScanProfileList();

    Vulnerability* m_vulnerability;
    DiscoverManager* m_discoverManager;
    Monitor* m_monitor;
    Utilities* m_utilities;
    ParserManager* m_parser;
    BookmarkManager* m_bookmark;
    ActionManager* m_collections;
    ProfileHandler* m_profileHandler;
    int m_hostCache;
    ScanWidget* m_scanWidget;
    QQuickView* m_welcomeQmlView;
    QTabWidget* m_mainTabWidget;
    MouseEventFilter* m_mouseFilter;

private:
    void addHostToMonitor(const QString hostname);
    void restoreSettings();
    void setDefaultSplitter();
    void updateQmlScanHistory();
    void copyToClipboard(const QString& text);

    QSplitter *m_mainHorizontalSplitter;
    QSplitter *m_mainVerticalSplitter;
    QCompleter* m_completer;
    QStringListModel* m_hostModel;
    int m_userId;
    int m_lookupType;
    int m_savedProfileIndex;
    QByteArray m_scanListWidgetSize;
    QByteArray m_detailsWidgetSize;
    QmlWelcome* m_qmlWelcome;
    QWidget* qmlWelcomeWidget;

protected:
    virtual void closeEvent(QCloseEvent * event);

public slots:
    void startScan();
    void updateComboHostnameProperties();
    void comboParametersSelectedEvent();
    void takeHostFromBookmark();
    void copyTextFromHostInfoTree();
    void copyTextFromScanPortsTree();
    void copyTextFromScanFullOutputTree();
    void updateScanSection();
    void updateVulnerabilitySection();
    void updateDiscoverSection();
    void clearAll();
    void newProfile();
    void editProfile();
    void setFullScreen();
    void updateMenuBar();
    void updateWelcomeSection();
    void resizeScanListWidgetEvent();
    void resizeHostDetailsWidgetEvent();
    void startPreferencesDialog();

private slots:
    void initObject();
    void updateScanCounter(int hostNumber);
    void syncSettings();
    void linkCompleterToHostname();
    void quickAddressSelectionEvent();
    void saveSettings();
    void clearHostnameCombo();
    void resizeVerticalSplitterEvent();
    void resizeHorizontalSplitterEvent();
    void resetComboParameters();
    void loadTargetListFromFile();

};

#endif

#include "AdBlockManager.h"
#include "BrowserApplication.h"
#include "BrowserTabWidget.h"
#include "BrowserTabBar.h"
#include "FaviconStore.h"
#include "MainWindow.h"
#include "WebPage.h"
#include "WebView.h"

#include <algorithm>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QMenu>
#include <QTimer>

BrowserTabWidget::BrowserTabWidget(ViperServiceLocator &serviceLocator, bool privateMode, QWidget *parent) :
    QTabWidget(parent),
    m_settings(serviceLocator.getServiceAs<Settings>("Settings")),
    m_serviceLocator(serviceLocator),
    m_faviconStore(serviceLocator.getServiceAs<FaviconStore>("FaviconStore")),
    m_privateBrowsing(privateMode),
    m_activeView(nullptr),
    m_tabBar(new BrowserTabBar(this)),
    m_backMenu(nullptr),
    m_forwardMenu(nullptr),
    m_lastTabIndex(0),
    m_currentTabIndex(0),
    m_nextTabIndex(1),
    m_mainWindow(qobject_cast<MainWindow*>(parent)),
    m_closedTabs()
{
    setObjectName(QLatin1String("tabWidget"));

    // Set tab bar
    setTabBar(m_tabBar);

    // Set tab widget UI properties
    setDocumentMode(true);
    setElideMode(Qt::ElideRight);
    //setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(this, &BrowserTabWidget::tabCloseRequested, this, &BrowserTabWidget::closeTab);
    connect(this, &BrowserTabWidget::currentChanged, this, &BrowserTabWidget::onCurrentChanged);

    connect(m_tabBar, &BrowserTabBar::duplicateTabRequest, this, &BrowserTabWidget::duplicateTab);
    connect(m_tabBar, &BrowserTabBar::newTabRequest, [=](){ newBackgroundTab(); });
    connect(m_tabBar, &BrowserTabBar::reloadTabRequest, [=](int index){
        if (WebWidget *view = getWebWidget(index))
            view->reload();
    });

    QCoreApplication::instance()->installEventFilter(this);
}

WebWidget *BrowserTabWidget::currentWebWidget() const
{
    return qobject_cast<WebWidget*>(currentWidget());
}

WebWidget *BrowserTabWidget::getWebWidget(int tabIndex) const
{
    if (QWidget *item = widget(tabIndex))
        return qobject_cast<WebWidget*>(item);
    return nullptr;
}

bool BrowserTabWidget::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type())
    {
        case QEvent::MouseMove:
        {
            if (m_mainWindow && m_mainWindow->isFullScreen())
                m_mainWindow->onMouseMoveFullscreen(static_cast<QMouseEvent*>(event)->y());
            break;
        }
        case QEvent::KeyPress:
        {
            if (m_mainWindow && m_mainWindow->isFullScreen())
            {
                QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
                if (keyEvent->key() == Qt::Key_F11)
                    m_mainWindow->onToggleFullScreen(false);
            }
            else if (sBrowserApplication->activeWindow() == m_mainWindow)
            {
                QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
                if (keyEvent->key() == Qt::Key_Tab && keyEvent->modifiers() == Qt::ControlModifier)
                {
                    // switch to the next tab
                    setCurrentIndex((currentIndex() + 1) % count());
                    return true;
                }
            }
            break;
        }
        default:
            break;
    }
    return QObject::eventFilter(watched, event);
}

void BrowserTabWidget::setTabPinned(int index, bool value)
{
    m_tabBar->setTabPinned(index, value);
}

bool BrowserTabWidget::canReopenClosedTab() const
{
    return !m_closedTabs.empty();
}

bool BrowserTabWidget::isTabPinned(int tabIndex) const
{
    return m_tabBar->isTabPinned(tabIndex);
}

void BrowserTabWidget::reopenLastTab()
{
    if (m_closedTabs.empty())
        return;

    auto &tabInfo = m_closedTabs.front();

    WebWidget *ww = newBackgroundTabAtIndex(tabInfo.index);
    m_tabBar->setTabPinned(tabInfo.index, tabInfo.isPinned);
    setTabText(tabInfo.index, tabInfo.title);
    setTabToolTip(tabInfo.index, tabInfo.title);
    setTabIcon(tabInfo.index, tabInfo.icon);
    ww->setWebState(tabInfo);

    m_closedTabs.pop_front();
}

void BrowserTabWidget::saveTab(int index)
{
    WebWidget *ww = getWebWidget(index);
    if (!ww)
        return;

    WebState tabState(ww, this);
    m_closedTabs.push_front(tabState);

    while (m_closedTabs.size() > 30)
        m_closedTabs.pop_back();
}

void BrowserTabWidget::closeTab(int index)
{
    const int numTabs = count();
    if (index < 0 || numTabs == 1 || index >= numTabs)
        return;

    saveTab(index);
    WebWidget *ww = getWebWidget(index);
    ww->stop();
    emit tabClosing(ww);

    // If closed tab was the active tab, set current to opposite direction of the last active tab (if possible)
    if (index == m_currentTabIndex)
    {
        if (index == 0)
            setCurrentIndex(1);
        else if (index == numTabs - 1)
            setCurrentIndex(numTabs - 2);
        else
        {
            int nextIndex = (m_lastTabIndex > index ? index - 1 : index + 1);
            setCurrentIndex(nextIndex);
            if (nextIndex == index + 1)
            {
                m_lastTabIndex = index;
                m_currentTabIndex = index;
                m_nextTabIndex = nextIndex;
            }
        }
    }

    removeTab(index);

    ww->deleteLater();
}

void BrowserTabWidget::closeCurrentTab()
{
    closeTab(currentIndex());
}

void BrowserTabWidget::duplicateTab(int index)
{
    if (WebWidget *ww = getWebWidget(index))
        openLinkInNewBackgroundTab(ww->url());
}

WebWidget *BrowserTabWidget::createWebWidget()
{
    WebWidget *ww = new WebWidget(m_serviceLocator, m_privateBrowsing, this);
    if (m_mainWindow)
    {
        ww->setMaximumWidth(m_mainWindow->maximumWidth());
        ww->view()->setMaximumWidth(m_mainWindow->maximumWidth());
    }

    // Connect web view signals to functionalty
    connect(ww, &WebWidget::iconChanged,            this, &BrowserTabWidget::onIconChanged);
    connect(ww, &WebWidget::loadFinished,           this, &BrowserTabWidget::onLoadFinished);
    connect(ww, &WebWidget::loadProgress,           this, &BrowserTabWidget::onLoadProgress);
    connect(ww, &WebWidget::openRequest,            this, &BrowserTabWidget::loadUrl);
    connect(ww, &WebWidget::openInNewTab,           this, &BrowserTabWidget::openLinkInNewTab);
    connect(ww, &WebWidget::openInNewBackgroundTab, this, &BrowserTabWidget::openLinkInNewBackgroundTab);
    connect(ww, &WebWidget::openInNewWindowRequest, this, &BrowserTabWidget::openLinkInNewWindow);
    connect(ww, &WebWidget::titleChanged,           this, &BrowserTabWidget::onTitleChanged);
    connect(ww, &WebWidget::urlChanged,             this, &BrowserTabWidget::onUrlChanged);
    connect(ww, &WebWidget::closeRequest,           this, &BrowserTabWidget::onViewCloseRequested);

    connect(ww, &WebWidget::openHttpRequestInBackgroundTab, this, &BrowserTabWidget::openHttpRequestInBackgroundTab);

	connect(ww, &WebWidget::aboutToHibernate, [=]() {
		if (currentWebWidget() == ww) {
            emit aboutToHibernate();
		}
	});
    connect(ww, &WebWidget::aboutToWake, [=]() {
        if (currentWebWidget() == ww) {
            emit aboutToWake();
        }
    });

    if (!m_privateBrowsing)
    {
        connect(ww, &WebWidget::iconUrlChanged, [=](const QUrl &url) {
            m_faviconStore->updateIcon(url.toString(QUrl::FullyEncoded), ww->url(), ww->getIcon());
        });
    }

    auto newTabPage = static_cast<NewTabType>(m_settings->getValue(BrowserSetting::NewTabPage).toInt());
    switch (newTabPage)
    {
        case NewTabType::HomePage:
            ww->load(QUrl::fromUserInput(m_settings->getValue(BrowserSetting::HomePage).toString()));
            break;
        case NewTabType::BlankPage:
            ww->loadBlankPage();
            break;
        case NewTabType::FavoritesPage:
            ww->load(QUrl(QLatin1String("viper://newtab")));
            break;
    }

    return ww;
}

WebWidget *BrowserTabWidget::newTab()
{
    return newTabAtIndex(m_nextTabIndex);
}

WebWidget *BrowserTabWidget::newTabAtIndex(int index)
{
    WebWidget *ww = createWebWidget();

    if (index >= 0)
    {
        if (index > count())
            index = count();

        index = insertTab(index, ww, QLatin1String("New Tab"));
        if (index <= m_nextTabIndex)
            ++m_nextTabIndex;
    }
    else
        m_nextTabIndex = insertTab(m_nextTabIndex, ww, QLatin1String("New Tab")) + 1;

    m_activeView = ww;
    setCurrentWidget(ww);

    if (count() == 1)
        emit currentChanged(currentIndex());

    emit newTabCreated(ww);
    return ww;
}

WebWidget *BrowserTabWidget::newBackgroundTab()
{
    return newBackgroundTabAtIndex(m_nextTabIndex);
}

WebWidget *BrowserTabWidget::newBackgroundTabAtIndex(int index)
{
    WebWidget *ww = createWebWidget();

    if (index >= 0)
    {
        if (index > count())
            index = count();

        index = insertTab(index, ww, QLatin1String("New Tab"));
        if (index <= m_nextTabIndex)
            ++m_nextTabIndex;
    }
    else
        m_nextTabIndex = insertTab(m_nextTabIndex, ww, QLatin1String("New Tab")) + 1;

    ww->resize(currentWidget()->size());
    ww->view()->resize(ww->size());
    ww->show();

    emit newTabCreated(ww);
    return ww;
}

void BrowserTabWidget::onIconChanged(const QIcon &icon)
{
    WebWidget *ww = qobject_cast<WebWidget*>(sender());
    int tabIndex = indexOf(ww);
    if (tabIndex < 0 || !ww)
        return;

    if (!icon.isNull())
        setTabIcon(tabIndex, icon);
    else
        setTabIcon(tabIndex, m_faviconStore->getFavicon(ww->url(), true));
}


void BrowserTabWidget::openLinkInNewTab(const QUrl &url)
{
    WebWidget *ww = newTab();
    ww->load(url);
}

void BrowserTabWidget::openLinkInNewBackgroundTab(const QUrl &url)
{
    WebWidget *ww = newBackgroundTab();
    ww->load(url);
}

void BrowserTabWidget::openHttpRequestInBackgroundTab(const HttpRequest &request)
{
    WebWidget *ww = newBackgroundTab();
    ww->load(request);
}

void BrowserTabWidget::openLinkInNewWindow(const QUrl &url, bool privateWindow)
{
    MainWindow *newWin = privateWindow ? sBrowserApplication->getNewPrivateWindow() : sBrowserApplication->getNewWindow();
    if (!newWin)
        return;
    newWin->loadUrl(url);
}

void BrowserTabWidget::loadUrl(const QUrl &url)
{
    if (m_activeView)
    {
        m_activeView->load(url);
        m_activeView->setFocus();
    }
}

void BrowserTabWidget::resetZoomCurrentView()
{
    if (WebWidget *ww = currentWebWidget())
        ww->view()->resetZoom();
}

void BrowserTabWidget::zoomInCurrentView()
{
    if (WebWidget *ww = currentWebWidget())
        ww->view()->zoomIn();
}

void BrowserTabWidget::zoomOutCurrentView()
{
    if (WebWidget *ww = currentWebWidget())
        ww->view()->zoomOut();
}

void BrowserTabWidget::onCurrentChanged(int index)
{
    WebWidget *ww = getWebWidget(index);
    if (!ww)
        return;

    ww->show();
    m_activeView = ww;

    m_lastTabIndex = m_currentTabIndex;
    m_currentTabIndex = index;
    m_nextTabIndex = index + 1;

    m_tabBar->updateGeometry();

    emit loadProgress(ww->getProgress());
    emit viewChanged(index);
}

void BrowserTabWidget::onLoadFinished(bool ok)
{
    WebWidget *ww = qobject_cast<WebWidget*>(sender());
    if (!ww)
        return;

    const int tabIndex = indexOf(ww);
    if (tabIndex < 0)
        return;

    const QString pageTitle = ww->getTitle();
    QIcon icon = ww->getIcon();
    if (icon.isNull())
        icon = m_faviconStore->getFavicon(ww->url(), true);

    setTabIcon(tabIndex, icon);
    setTabText(tabIndex, pageTitle);
    setTabToolTip(tabIndex, pageTitle);

    if (ok && ww == m_activeView)
        emit loadFinished();
}

void BrowserTabWidget::onLoadProgress(int progress)
{
    WebWidget *ww = qobject_cast<WebWidget*>(sender());
    if (ww == m_activeView)
        emit loadProgress(progress);
}

void BrowserTabWidget::onTitleChanged(const QString &title)
{
    WebWidget *ww = qobject_cast<WebWidget*>(sender());
    const int viewTabIndex = indexOf(ww);
    if (viewTabIndex >= 0)
    {
        setTabText(viewTabIndex, title);
        setTabToolTip(viewTabIndex, title);

        if (ww == m_activeView)
            emit titleChanged(title);
    }
}

void BrowserTabWidget::onUrlChanged(const QUrl &url)
{
    WebWidget *ww = qobject_cast<WebWidget*>(sender());
    if (ww == m_activeView && !url.isEmpty())
        emit urlChanged(url);
}

void BrowserTabWidget::onViewCloseRequested()
{
    WebWidget *ww = qobject_cast<WebWidget*>(sender());
    closeTab(indexOf(ww));
}

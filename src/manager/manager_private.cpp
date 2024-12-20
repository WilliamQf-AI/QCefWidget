#include "QWebView/ManagerPrivate.h"
#include "top_level_window_util.h"
#include <QTimer>

void QWebViewManagerPrivate::setCefCanUnInitialize() {
  cefCanUnInitialize_ = true;

  checkAllWebViewsClosed();
}

void QWebViewManagerPrivate::add(QWebView* webview) {
  QWidget* topLevel = GetTopLevelWindow(webview);
  Q_ASSERT(topLevel);
  Q_ASSERT(!topLevel->testAttribute(Qt::WA_DeleteOnClose));

  if (webview->browserEngine() == QWebView::BrowserEngine::CEF) {
    cefCanUnInitialize_ = false;
  }

  WebViewData wvd;
  wvd.WebView = webview;

  if (topLevelDataMap_.contains(topLevel)) {
    topLevelDataMap_[topLevel].WebviewColl.push_back(wvd);
  }
  else {
    TopLevelWndData data;
    data.WebviewColl.push_back(wvd);

    topLevelDataMap_[topLevel] = data;
  }
}

QWebViewManagerPrivate::QWebViewManagerPrivate() {
}

void QWebViewManagerPrivate::remove(QWebView* webview) {
  QWidget* topLevel = GetTopLevelWindow(webview);
  Q_ASSERT(topLevel);
  Q_ASSERT(topLevelDataMap_.contains(topLevel));

  if (topLevelDataMap_.contains(topLevel)) {
    auto& coll = topLevelDataMap_[topLevel].WebviewColl;
    for (QVector<WebViewData>::iterator it = coll.begin(); it != coll.end(); ++it) {
      if (it->WebView == webview) {
        coll.erase(it);
        break;
      }
    }
  }

  checkAllWebViewsClosed();
}

void QWebViewManagerPrivate::prepareToCloseTopLevelWindow(QWidget* topLevel) {
  qDebug() << ">>>> QWebViewManagerPrivate::prepareToCloseTopLevelWindow" << topLevel;

  if (topLevelDataMap_.contains(topLevel)) {
    if (topLevelDataMap_[topLevel].CloseState == QWebViewManager::TopLevelWndCloseState::NotStart) {
      topLevelDataMap_[topLevel].CloseState = QWebViewManager::TopLevelWndCloseState::BrowsersClosing;

      auto& coll = topLevelDataMap_[topLevel].WebviewColl;
      for (QVector<WebViewData>::iterator it = coll.begin(); it != coll.end(); ++it) {
        if (!it->BrowserClosed) {
          Q_ASSERT(it->WebView);
          if (it->WebView->browserEngine() == QWebView::BrowserEngine::CEF) {
            qDebug() << ">>>>     Call QWidget:close()" << it->WebView;
            QWidget* w = it->WebView;
            QTimer::singleShot(1, [w]() {
              w->close();
            });
          }
          else if (it->WebView->browserEngine() == QWebView::BrowserEngine::WebView2) {
            QWebView* webview = it->WebView;
            webview->close(); // 在CloseEvent事件中做一些清理操作
            it->BrowserClosed = true;

            // WebView2 不会给顶级窗口发送 WM_CLOSE 消息，为了保持和 CEF 流程一直，模拟发送 CloseEvent 到顶级窗口
            QTimer::singleShot(1, [topLevel, webview, this]() {
              topLevel->close();
              setWebViewClosed(topLevel, webview);
              remove(webview);
            });
          }
        }
      }
    }

    checkBrowsersCloseState(topLevel);
  }
}

void QWebViewManagerPrivate::setWebViewClosed(QWidget* topLevel, QWebView* webview) {
  qDebug() << ">>>> QWebViewManagerPrivate::setCefWebViewClosed" << webview;

  if (!topLevel)
    topLevel = GetTopLevelWindow(webview);
  Q_ASSERT(topLevel);

  // CEF的Popup窗口不在其中
  if (topLevelDataMap_.contains(topLevel)) {
    auto& coll = topLevelDataMap_[topLevel].WebviewColl;
    for (QVector<WebViewData>::iterator it = coll.begin(); it != coll.end(); ++it) {
      if (it->WebView == webview) {
        it->BrowserClosed = true;
        break;
      }
    }

    checkBrowsersCloseState(topLevel);
  }
}

void QWebViewManagerPrivate::sendCloseEventToTopLevel(QWebView* webview) {
  QWidget* topLevel = GetTopLevelWindow(webview);
  QTimer::singleShot(1, [topLevel]() {
    if (topLevel)
      topLevel->close();
  });
}

void QWebViewManagerPrivate::checkBrowsersCloseState(QWidget* topLevel) {
  qDebug() << ">>>> QWebViewManager::checkBrowsersCloseState" << topLevel;

  if (topLevelDataMap_.contains(topLevel)) {
    auto& coll = topLevelDataMap_[topLevel].WebviewColl;

    bool isAllClosed = true;
    for (QVector<WebViewData>::iterator it = coll.begin(); it != coll.end(); ++it) {
      if (!it->BrowserClosed) {
        isAllClosed = false;
        break;
      }
    }

    if (isAllClosed) {
      qDebug() << ">>>>    All Browsers Closed";
      topLevelDataMap_[topLevel].CloseState = QWebViewManager::TopLevelWndCloseState::BrowsersClosed;
    }
  }
}

void QWebViewManagerPrivate::checkAllWebViewsClosed() {
  if (!cefCanUnInitialize_)
    return;

  bool allEmpty = true;
  for (auto it = topLevelDataMap_.cbegin(); it != topLevelDataMap_.cend(); ++it) {
    if (!it.value().WebviewColl.isEmpty()) {
      allEmpty = false;
      break;
    }
  }

  if (!allEmpty)
    return;

  emit QWebViewManager::Get() -> allWebViewsClosed();
}
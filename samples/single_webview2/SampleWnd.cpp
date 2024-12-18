#include "SampleWnd.h"
#include "QWebView/Creator.h"
#include "QWebView/Manager.h"

SampleWnd::SampleWnd(QWidget* parent /*= nullptr*/) :
    QWidget(parent) {
  webview_ = CreateWebView2();
  webview_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  connect(webview_, &QWebView::titleChanged, this, [this](QString title) {
    setWindowTitle(title);
  });

  QPushButton* btnClose = new QPushButton("����QWidget::close()");
  connect(btnClose, &QPushButton::clicked, this, [this]() {
    this->close();
  });

  QPushButton* btnQuit = new QPushButton("����QCoreApplication::quit()");
  connect(btnQuit, &QPushButton::clicked, this, [this]() {
    QMessageBox::warning(this, "ʹ�ô���", "����ֱ�ӵ��� QCoreApplication::quit() ���˳�Ӧ�ó���");
  });

  QHBoxLayout* lTop = new QHBoxLayout();
  lTop->addWidget(webview_);

  QHBoxLayout* lBottom = new QHBoxLayout();
  lBottom->addWidget(btnClose);
  lBottom->addWidget(btnQuit);

  QVBoxLayout* v = new QVBoxLayout();
  v->addLayout(lTop);
  v->addLayout(lBottom);

  setLayout(v);

  webview_->navigate("https://www.baidu.com");
}

void SampleWnd::closeEvent(QCloseEvent* e) {
  QWebViewManager::TopLevelWndCloseState state = QWebViewManager::Get()->topLevelWinCloseState(this);
  qDebug() << ">>>> SampleWnd closeEvent" << state;

  if (state == QWebViewManager::TopLevelWndCloseState::NotStart) {
    if (QMessageBox::Yes == QMessageBox::question(this, "����", "ȷ��Ҫ�˳�ʾ��������")) {
      QWebViewManager::Get()->prepareToCloseTopLevelWindow(this);
    }
    qDebug() << ">>>> SampleWnd closeEvent: ignore";
    e->ignore();
  }
  else if (state == QWebViewManager::TopLevelWndCloseState::BrowsersClosing) {
    qDebug() << ">>>> SampleWnd closeEvent: ignore";
    e->ignore();
  }
  else if (state == QWebViewManager::TopLevelWndCloseState::BrowsersClosed) {
    qDebug() << ">>>> SampleWnd closeEvent: accept";
    e->accept();
  }
  else if (state == QWebViewManager::TopLevelWndCloseState::Closed) {
    Q_UNREACHABLE();
  }
}

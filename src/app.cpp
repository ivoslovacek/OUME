#include "app.hpp"

#include <qaction.h>
#include <qmainwindow.h>
#include <qnamespace.h>
#include <qpushbutton.h>
#include <qwidget.h>

#include <memory>

#include "events.hpp"
#include "menubar/menubar.hpp"

namespace OUMP {
AppWindow::AppWindow(QWidget *t_parent)
    : QMainWindow(t_parent),
      m_events(std::make_shared<EventsHub>()),
      m_main_widget(new QWidget(this)),
      m_player(new PlayerLayout(this->m_events)),
      m_menubar(new MenuBar(this->m_events)) {
    this->setCentralWidget(this->m_main_widget);
    this->m_main_widget->setLayout(this->m_player);
    this->setMenuBar(this->m_menubar);
    QMainWindow::show();
}
}  // namespace OUMP

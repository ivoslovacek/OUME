#pragma once

#include <qmainwindow.h>
#include <qobject.h>
#include <qpushbutton.h>
#include <qwidget.h>

#include <memory>

#include "events.hpp"
#include "menubar/menubar.hpp"
#include "player/layout.hpp"

namespace OUMP {
class AppWindow : public QMainWindow {
   private:
    std::shared_ptr<EventsHub> m_events;
    QWidget *m_main_widget;
    PlayerLayout *m_player;
    MenuBar *m_menubar;

    std::string m_current_file;

   public:
    AppWindow(QWidget *t_parent = nullptr);
    ~AppWindow() = default;
};
}  // namespace OUMP

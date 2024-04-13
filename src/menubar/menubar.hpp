#pragma once

#include <qmainwindow.h>
#include <qmenubar.h>

#include "events.hpp"
#include "menubar/file.hpp"

namespace OUMP {
class MenuBar : public QMenuBar {
   private:
    std::shared_ptr<EventsHub> m_events;
    FileMenu* m_file_menu;

   public:
    MenuBar(std::shared_ptr<EventsHub> t_events);
    ~MenuBar() = default;
};
};  // namespace OUMP

#include "menubar/menubar.hpp"

#include <memory>

#include "events.hpp"
#include "menubar/file.hpp"

namespace OUMP {
MenuBar::MenuBar(std::shared_ptr<EventsHub> t_events)
    : QMenuBar(), m_events(t_events), m_file_menu(new FileMenu(t_events)) {
    this->addMenu(this->m_file_menu);
}
}  // namespace OUMP

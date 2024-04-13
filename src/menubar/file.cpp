#include "menubar/file.hpp"

#include <qaction.h>
#include <qfiledialog.h>
#include <qmenu.h>

#include "events.hpp"

namespace OUMP {
FileMenu::FileMenu(std::shared_ptr<EventsHub> t_events)
    : QMenu("File"), m_events(t_events), m_open_file(new OpenFile()) {
    this->addAction(this->m_open_file);

    connect(this->m_open_file, &OpenFile::triggered, this,
            &FileMenu::handleOpenFile);

    connect(this, &FileMenu::onNewFile, [this](QString t_filename) {
        this->m_events->changeFileName(t_filename);
    });
}

void FileMenu::handleOpenFile() {
    this->m_filename = QFileDialog::getOpenFileName(
        this, "Open file", QDir::currentPath(), "All files (*.*)");

    emit this->onNewFile(this->m_filename);
}

OpenFile::OpenFile() : QAction("Open file") {}
}  // namespace OUMP

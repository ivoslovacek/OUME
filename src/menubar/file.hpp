#pragma once

#include <qaction.h>
#include <qmenu.h>
#include <qtmetamacros.h>

#include "events.hpp"

namespace OUMP {
class OpenFile;

/**
 * @brief Dropdown "file" menu.
 */
class FileMenu : public QMenu {
    Q_OBJECT
   signals:
    void onNewFile(QString);

   private slots:
    void handleOpenFile();

   private:
    std::shared_ptr<EventsHub> m_events;
    OpenFile* m_open_file;
    QString m_filename;

   public:
    FileMenu(std::shared_ptr<EventsHub> t_events);
    ~FileMenu() = default;
};

/**
 * @brief QAction that opens a file.
 */
class OpenFile : public QAction {
   private:
   public:
    OpenFile();
    ~OpenFile() = default;
};
}  // namespace OUMP

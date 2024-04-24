#pragma once

#include <qobject.h>
#include <qtmetamacros.h>

namespace OUMP {
/**
 * @brief A class used for handling events.
 */
class EventsHub : public QObject {
    Q_OBJECT
   signals:
    /**
     * @brief Qt slot distributing a new file path.
     */
    void changedFileName(QString);

   private:
    QString m_filename;

   public:
    EventsHub();
    ~EventsHub() = default;

    /**
     * @brief An Qt slot that should be called upon a obtaining a new file path.
     *
     * This Qt slot dispatches the new file path to all listeners.
     *
     * @param t_filename A QString containing the new path to the file.
     */
   public slots:
    void changeFileName(QString);
};
}  // namespace OUMP

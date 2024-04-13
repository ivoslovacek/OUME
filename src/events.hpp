#pragma once

#include <qobject.h>
#include <qtmetamacros.h>

namespace OUMP {
class EventsHub : public QObject {
    Q_OBJECT
   signals:
    void changedFileName(QString);

   private:
    QString m_filename;

   public:
    EventsHub();
    ~EventsHub() = default;

   public slots:
    void changeFileName(QString);
};
}  // namespace OUMP

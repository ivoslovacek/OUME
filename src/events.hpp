#pragma once

#include <qobject.h>
#include <qtmetamacros.h>

#include <cstdint>

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
    void changedPlayingState(bool);
    void changedVolume(int);
    void changedMediaEndTimepoint(int64_t);
    void changedCurrentMediaTimepoint(int64_t);
    void changedCurrentSliderTimepoint(int64_t);

   private:
    QString m_filename;

   public:
    EventsHub();
    ~EventsHub() = default;

   public slots:
    /**
     * @brief An Qt slot that should be called upon a obtaining a new file path.
     *
     * This Qt slot dispatches the new file path to all listeners.
     *
     * @param t_filename A QString containing the new path to the file.
     */
    void changeFileName(QString);
    void changePlayingState(bool);
    void changeVolume(int);
    void changeMediaEndTimepoint(int64_t);
    void changeCurrentMediaTimepoint(int64_t);
    void changeCurrentSliderTimepoint(int64_t);
};
}  // namespace OUMP

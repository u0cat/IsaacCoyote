//
// Created by TsCat on 2026/7/27.
//

#ifndef ISAACCOYOTE_GAME_OVER_SOURCE_H
#define ISAACCOYOTE_GAME_OVER_SOURCE_H
#include "source_type.h"

namespace app::event
{
    class EventEngine;

    namespace sources
    {
        class GameOverSource : public IEventSource {
        public:
            explicit GameOverSource(EventEngine& engine);

            void tick() override;
            void reset() override;

        private:

            bool game_over_flag_ = false;
        };
    }
}

#endif //ISAACCOYOTE_GAME_OVER_SOURCE_H

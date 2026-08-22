//
// Created by TsCat on 2026/7/7.
//

#ifndef ISAACCOYOTE_APP_OVERLAY_TYPES_H
#define ISAACCOYOTE_APP_OVERLAY_TYPES_H
#include <string>

namespace app::overlay
{
    class ITab
    {
    public:
        virtual ~ITab() = default;

        virtual void render() = 0;

        virtual std::string get_id() = 0;
        virtual std::string get_display_name() = 0;
    };
}
#endif // ISAACCOYOTE_APP_OVERLAY_TYPES_H

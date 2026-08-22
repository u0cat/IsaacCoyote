//
// Created by TsCat on 2026/8/8.
//

#ifndef ISAACCOYOTE_ENTITY_REF_H
#define ISAACCOYOTE_ENTITY_REF_H
#include <cstdint>

#include "entity.h"

namespace isaac_spy::isaac
{

    class EntityRef {
    public:
        EntityRef(uintptr_t entity_ref_ptr);

        int get_type() const;
        int get_variant() const;
        int get_subtype() const;
        Entity get_entity() const;

    private:
        uintptr_t this_ptr_;
        mutable uintptr_t entity_ptr_ = 0;
    };
}
#endif //ISAACCOYOTE_ENTITY_REF_H

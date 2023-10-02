#include <loader/allocator.h>

enum AllocatorType {
    ALLOCATOR_TYPE_STATIC,
    ALLOCATOR_TYPE_DYNAMIC,
};

struct allocator {
    paddr_t from;
    paddr_t free;
    paddr_t to;
    enum AllocatorType type;
    size_t alignment;
};

void allocator_initialize(struct allocator *self, paddr_t from, paddr_t to, enum AllocatorType type, size_t alignment)
{
    self->from = from;
    self->free = from;
    self->to = to;
    self->type = type;
    self->alignment = alignment;
}

paddr_t allocate_transient(struct allocator *self, size_t size, size_t align)
{
    paddr_t ret = self->to - size;

    if (align != 0)
        ret &= ~(align - 1);

    if (ret < self->free)
        return 0;

    self->to = ret;
    return ret;
}

paddr_t allocate_permanent(struct allocator *self, size_t size, size_t align)
{
    paddr_t up, ret = self->free;
    size_t mask;

    mask = align - 1;
    if ((align != 0) && ((ret & mask) != 0)) {
        ret &= ~mask;
        ret += align;
    }

    up = ret + size;

    if (up > self->to)
        return 0;

    self->free = up;
    return ret;
}

void reserve_zone(struct allocator *self, paddr_t from, paddr_t to)
{
    size_t up = 0, down = 0;

    if (from >= self->to)
        return;
    if (to <= self->from)
        return;

    if (from > self->free)
        down = from - self->free;
    if (to < self->to)
        up = self->to - to;

    if (down >= up) {
        self->to = from;
    } else {
        self->from = to;
        if (self->from > self->free)
            self->free = self->from;
    }
}

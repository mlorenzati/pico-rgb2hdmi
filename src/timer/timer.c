#include "timer.h"
#include "pico/util/pheap.h"
#include "pico/sync.h"

#if PICO_TIME_DEFAULT_ALARM_POOL_DISABLED
typedef struct alarm_pool_entry {
    absolute_time_t target;
    alarm_callback_t callback;
    void *user_data;
} alarm_pool_entry_t;

struct alarm_pool {
    pheap_t *heap;
    spin_lock_t *lock;
    alarm_pool_entry_t *entries;
    // one byte per entry, used to provide more longevity to public IDs than heap node ids do
    // (this is increment every time the heap node id is re-used)
    uint8_t *entry_ids_high;
    alarm_id_t alarm_in_progress; // this is set during a callback from the IRQ handler... it can be cleared by alarm_cancel to prevent repeats
    uint8_t hardware_alarm_num;
    uint8_t core_num;
};

// To avoid bringing in calloc, we statically allocate the arrays and the heap
PHEAP_DEFINE_STATIC(default_alarm_pool_heap, PICO_TIME_DEFAULT_ALARM_POOL_MAX_TIMERS);
static alarm_pool_entry_t default_alarm_pool_entries[PICO_TIME_DEFAULT_ALARM_POOL_MAX_TIMERS];
static uint8_t default_alarm_pool_entry_ids_high[PICO_TIME_DEFAULT_ALARM_POOL_MAX_TIMERS];
static lock_core_t sleep_notifier;

static alarm_pool_t default_alarm_pool = {
        .heap = &default_alarm_pool_heap,
        .entries = default_alarm_pool_entries,
        .entry_ids_high = default_alarm_pool_entry_ids_high,
};

static alarm_pool_t *pools[NUM_TIMERS];

static inline bool default_alarm_pool_initialized(void) {
    return default_alarm_pool.lock != NULL;
}

static inline alarm_pool_entry_t *get_entry(alarm_pool_t *pool, pheap_node_id_t id) {
    assert(id && id <= pool->heap->max_nodes);
    return pool->entries + id - 1;
}

static inline alarm_id_t make_public_id(uint8_t id_high, pheap_node_id_t id) {
    return (alarm_id_t)(((uint)id_high << 8u * sizeof(id)) | id);
}

alarm_pool_t *alarm_pool_get_default() {
    assert(default_alarm_pool_initialized());
    return &default_alarm_pool;
}

bool timer_pool_entry_comparator(void *user_data, pheap_node_id_t a, pheap_node_id_t b);

static inline uint8_t *get_entry_id_high(alarm_pool_t *pool, pheap_node_id_t id) {
    assert(id && id <= pool->heap->max_nodes);
    return pool->entry_ids_high + id - 1;
}

static pheap_node_id_t add_alarm_under_lock(alarm_pool_t *pool, absolute_time_t time, alarm_callback_t callback,
                                       void *user_data, pheap_node_id_t reuse_id, bool create_if_past, bool *missed) {
    pheap_node_id_t id;
    if (reuse_id) {
        assert(!ph_contains_node(pool->heap, reuse_id));
        id = reuse_id;
    } else {
        id = ph_new_node(pool->heap);
    }
    if (id) {
        alarm_pool_entry_t *entry = get_entry(pool, id);
        entry->target = time;
        entry->callback = callback;
        entry->user_data = user_data;
        if (id == ph_insert_node(pool->heap, id)) {
            bool is_missed = hardware_alarm_set_target(pool->hardware_alarm_num, time);
            if (is_missed && !create_if_past) {
                ph_remove_and_free_node(pool->heap, id);
            }
            if (missed) *missed = is_missed;
        }
    }
    return id;
}

static void alarm_pool_alarm_callback(uint alarm_num) {
    // note this is called from timer IRQ handler
    alarm_pool_t *pool = pools[alarm_num];
    bool again;
    do {
        absolute_time_t now = get_absolute_time();
        alarm_callback_t callback = NULL;
        absolute_time_t target = nil_time;
        void *user_data = NULL;
        uint8_t id_high;
        again = false;
        uint32_t save = spin_lock_blocking(pool->lock);
        pheap_node_id_t next_id = ph_peek_head(pool->heap);
        if (next_id) {
            alarm_pool_entry_t *entry = get_entry(pool, next_id);
            if (absolute_time_diff_us(now, entry->target) <= 0) {
                // we don't free the id in case we need to re-add the timer
                pheap_node_id_t __unused removed_id = ph_remove_head(pool->heap, false);
                assert(removed_id == next_id); // will be true under lock
                target = entry->target;
                callback = entry->callback;
                user_data = entry->user_data;
                assert(callback);
                id_high = *get_entry_id_high(pool, next_id);
                pool->alarm_in_progress = make_public_id(id_high, removed_id);
            } else {
                if (hardware_alarm_set_target(alarm_num, entry->target)) {
                    again = true;
                }
            }
        }
        spin_unlock(pool->lock, save);
        if (callback) {
            int64_t repeat = callback(make_public_id(id_high, next_id), user_data);
            save = spin_lock_blocking(pool->lock);
            // todo think more about whether we want to keep calling
            if (repeat < 0 && pool->alarm_in_progress) {
                assert(pool->alarm_in_progress == make_public_id(id_high, next_id));
                add_alarm_under_lock(pool, delayed_by_us(target, (uint64_t)-repeat), callback, user_data, next_id, true, NULL);
            } else if (repeat > 0 && pool->alarm_in_progress) {
                assert(pool->alarm_in_progress == make_public_id(id_high, next_id));
                add_alarm_under_lock(pool, delayed_by_us(get_absolute_time(), (uint64_t)repeat), callback, user_data, next_id,
                                     true, NULL);
            } else {
                // need to return the id to the heap
                ph_free_node(pool->heap, next_id);
                (*get_entry_id_high(pool, next_id))++; // we bump it for next use of id
            }
            pool->alarm_in_progress = 0;
            spin_unlock(pool->lock, save);
            again = true;
        }
    } while (again);
}

void alarm_pool_post_alloc_init(alarm_pool_t *pool, uint hardware_alarm_num) {
    hardware_alarm_cancel(hardware_alarm_num);
    hardware_alarm_set_callback(hardware_alarm_num, alarm_pool_alarm_callback);
    pool->lock = spin_lock_instance(next_striped_spin_lock_num());
    pool->hardware_alarm_num = (uint8_t) hardware_alarm_num;
    pool->core_num = (uint8_t) get_core_num();
    pools[hardware_alarm_num] = pool;
}

#endif

// This initialization must be called from the CORE that will manage the IRQs
bool timer_alarm_pool_init_default() {
#if PICO_TIME_DEFAULT_ALARM_POOL_DISABLED   
    // allow multiple calls for ease of use from host tests
    if (!default_alarm_pool_initialized()) {
        ph_post_alloc_init(default_alarm_pool.heap, PICO_TIME_DEFAULT_ALARM_POOL_MAX_TIMERS,
                           timer_pool_entry_comparator, &default_alarm_pool);
        hardware_alarm_claim(PICO_TIME_DEFAULT_ALARM_POOL_HARDWARE_ALARM_NUM);
        alarm_pool_post_alloc_init(&default_alarm_pool,
                                   PICO_TIME_DEFAULT_ALARM_POOL_HARDWARE_ALARM_NUM);
    }
    lock_init(&sleep_notifier, PICO_SPINLOCK_ID_TIMER);
    return true;
#else
    return false;
#endif
}

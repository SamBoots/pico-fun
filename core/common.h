#ifndef COMMON_H
#define COMMON_H

typedef struct delay_bool_t
{
    bool value;
    u32 last_change_ms;
    u32 ms_switch;
} delay_bool_t;

delay_bool_t delay_bool_init(u32 a_switch_ms, bool a_default);
bool delay_bool_update(delay_bool_t* a_bool, u32 a_now_ms);

#endif // COMMON_H

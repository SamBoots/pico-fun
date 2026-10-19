#include "types.h"
#include "common.h"

delay_bool_t delay_bool_init(u32 a_switch_ms, bool a_default)
{
    delay_bool_t val;
    val.value = a_default;
    val.last_change_ms = 0;
    val.ms_switch = a_switch_ms;
    return val;
}

bool delay_bool_update(delay_bool_t* a_bool, u32 a_now_ms)
{
    if ((u32)(a_now_ms - a_bool->last_change_ms) > a_bool->ms_switch)
    {
        a_bool->last_change_ms = a_now_ms;
        a_bool->value = !a_bool->value;
    }
    return a_bool->value;
}

//
// Created by Szymon on 21.10.2026.
//

#ifndef BATTERY_H
#define BATTERY_H

#include <stdint.h>

enum {
    BATT_ALK_1V5 = 0,
    BATT_NIMH_1V2,
    BATT_ALK_9V,
    BATT_NIMH_8V4,
    BATT_ALK_12V,
    BATT_LI_3V6,
    BATT_MODE_COUNT
};

uint8_t battery_percent(uint8_t mode, uint16_t pack_mv);
uint16_t read_pack_mv(void);

#endif //BATTERY_H

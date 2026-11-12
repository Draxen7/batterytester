//
// Created by Szymon on 21.10.2026.
//

#include "battery.h"
#include "adc.h"

static const uint16_t alkaline[][2] = {
    { 900, 0 },
    { 1000, 5 },
    { 1050, 12 },
    { 1100, 20 },
    { 1150, 30 },
    { 1200, 40 },
    { 1250, 50 },
    { 1300, 60 },
    { 1350, 68 },
    { 1400, 76 },
    { 1450, 84 },
    { 1500, 90 },
    { 1550, 96 },
    { 1600, 100 }
};
static const uint16_t nimh[][2] = {
    { 1000, 0 },
    { 1100, 5 },
    { 1150, 12 },
    { 1200, 25 },
    { 1220, 40 },
    { 1250, 55 },
    { 1280, 70 },
    { 1320, 84 },
    { 1360, 94 },
    { 1400, 100 }
};
static const uint16_t liion[][2] = {
    { 3000, 0 },
    { 3300, 3 },
    { 3450, 8 },
    { 3550, 15 },
    { 3650, 25 },
    { 3720, 40 },
    { 3780, 50 },
    { 3850, 62 },
    { 3920, 75 },
    { 4000, 85 },
    { 4100, 93 },
    { 4200, 100 }
};
static const uint16_t (*curves[])[2] = { alkaline, nimh, liion };
static const uint8_t curve_len[] = {
    sizeof alkaline / sizeof alkaline[0],
    sizeof nimh / sizeof nimh[0],
    sizeof liion / sizeof liion[0]
};
//                        1v5,    1v2,    9v,    8v4,    12v,    3v6
static const uint8_t mode_curve[] = { 0, 1, 0, 1, 0, 2 };
static const uint8_t mode_cells[] = { 1, 1, 6, 7, 8, 1 };

static uint8_t lookup(const uint16_t curve[][2], const uint8_t n, const uint16_t mv) {
    if (mv <= curve[0][0]) return 0;
    if (mv >= curve[n - 1][0]) return 100;
    uint8_t i = 0;
    while (i + 1 < n && mv > curve[i + 1][0]) i++;
    const uint16_t mv0 = curve[i][0], mv1 = curve[i + 1][0];
    const uint16_t p0 = curve[i][1], p1 = curve[i + 1][1];
    return p0 + (uint32_t)(p1 - p0) * (mv - mv0) / (mv1 - mv0);
}

uint8_t battery_percent(const uint8_t mode, const uint16_t pack_mv) {
    const uint8_t curve = mode_curve[mode];
    const uint16_t cell_mv = pack_mv / mode_cells[mode];
    return lookup(curves[curve], curve_len[curve], cell_mv);
}

uint16_t read_pack_mv(void) {
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    const uint32_t count = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    const uint32_t vref = HAL_ADC_GetValue(&hadc1), vdda_mv = 1210UL * 4095UL / vref;
    return count * vdda_mv / 4095UL * 49UL / 10UL;
}

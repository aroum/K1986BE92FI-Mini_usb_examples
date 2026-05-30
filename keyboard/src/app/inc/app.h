#ifndef _APP_H_
#define _APP_H_

// Из комплекта поставки
#include "MDR32FxQI_rst_clk.h"
#include "MDR32FxQI_bkp.h"
#include "MDR32FxQI_eeprom.h"
#include "K1986VE9xI_IT.h"

// Сишные библиотеки
// #include <stdio.h> // раскомментировать syscalls.c в common/Drivers/CMakeLists.txt

// Написанное мною
#include "clk.h"


// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"


#endif /*_APP_H_*/
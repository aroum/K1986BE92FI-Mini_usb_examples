# Примеры USB для K1986BE92FI-Mini

1986ВЕ9х семейство:
К1986ВЕ92FI  ( MDR1211FI )
К1986ВЕ92F1I ( MDR1211F1I )
К1986ВЕ94GI  ( MDR1209GI )
К1986ВЕ92QI  ( MDR32F9Q2I ) снятый с производтсва

Шесть USB-прошивок для мк К1986ВЕ92F1I (MDR1211F1I) на **FreeRTOS** с общей конфигурацией платы и единым скриптом сборки.

## Подготовка

Установите в системе (FreeRTOS уже лежит в репозитории, отдельно ставить не нужно):

| Инструмент          | Зачем                                     |
| ------------------- | ----------------------------------------- |
| `arm-none-eabi-gcc` | Кросс-компилятор GCC                      |
| `cmake`             | Генерация Makefile                        |
| `make`              | Сборка                                    |
| `openocd`           | Прошивка через J-Link (`-f`)              |
| `bc`                | Расчет процентов занимаемой памяти (`-s`) |

## Проблема с резисторами CC

![schematic](schematic.png)

Если планируется подключение кабелем USB-C — USB-C, проверьте номиналы R21 and R22. В схеме/на плате ошибочно стоят 510 кОм, их необходимо заменить на 5.1 кОм (SMD 0603). Без этой замены контроллер порта не перейдет в активный режим.

## Конфигурация платы

Файл [`config.h`](config.h) в корне `usb_examples`:

- `HSE_Value` — частота внешнего кварца (по умолчанию 8 МГц)
- `BOARD_BUTTON`, `BOARD_LED` — пины в формате `PB6` (порт B, вывод 6)
- `BOARD_DAC` — вывод ЦАП (по умолчанию `PE0` = DAC2_OUT)

Макросы `MDR_PORTB`, `PORT_Pin_6` и т.п. задаются в [`board_pins.h`](board_pins.h).

## Сборка и прошивка

Из корня `usb_examples`:

```bash
chmod +x build_all.sh
./build_all.sh vcom            # собрать vcom
./build_all.sh vcom_eeprom -sf # размер + прошивка vcom_eeprom
./build_all.sh keyboard -c     # чистая сборка keyboard
./build_all.sh midi -sf        # размер + прошивка midi
./build_all.sh synt -sf        # размер + прошивка synt
./build_all.sh combo -sf       # размер + прошивка combo
```

Из каталога проекта можно вызвать обёртку (делегирует в общий скрипт):

```bash
./vcom/build_all.sh -csf
```

Флаги: `-c`/`--clean` чистая сборка, `-s`/`--size` отчёт по памяти, `-f`/`--flash` прошивка через J-Link, `-h`/`--help` справка.

## Примеры

| Пример        | Описание                                                      |
| ------------- | ------------------------------------------------------------- |
| `vcom`        | USB CDC echo                                                  |
| `vcom_eeprom` | USB CDC echo + Запись/Чтение EEPROM в фоновой задаче FreeRTOS |
| `keyboard`    | USR Кнопка → HID key `F`, LED                                 |
| `midi`        | USR Кнопка → MIDI нота A4, LED                                |
| `synt`        | USB MIDI Note On/Off → синус на ЦАП, LED                      |
| `combo`       | USR при включении: VCOM EEPROM (можно задать букву которая будет печататься в режиме клавиатуры a-z); иначе HID клавиатура из ПЗУ    |

Общий код: `common/` (SPL/CMSIS, startup, тактирование, GPIO, USB), `freertos/` (ядро FreeRTOS).

В каждом примере в `src/sdk/` остаются только настройки SPL под конкретный USB-класс: `MDR32FxQI_config.h` and `MDR32FxQI_usb_handlers.h`.

## Другие проекты

- [Примеры для платы MILUINO](https://github.com/arty1223/MILUINO)
- [Примеры работы с микроконтроллером MDR32F9Q2I (K1986BE92QI)](https://github.com/mdr32/examples/tree/main)
- [Пример проекта для K1986BE92FI (MDR1211FI)](https://github.com/MikhaelKaa/K1986BE92FI_Example)
- [Пример проекта для K1986BE92FI (MDR1211FI) в vs code](https://github.com/Ayrat-Kh/milandr-k1986be92QI)
- [Пример проекта для K1986BE92FI (MDR1211FI) с использованием i2c дисплея 1602](https://github.com/arty1223/MDR1211_lcd1602_i2c)

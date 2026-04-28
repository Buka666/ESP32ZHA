# ESP32-C6 SuperMini Zigbee RGB лампа для ZHA

Минимальный шаблон Zigbee устройства типа **Color Dimmable Light** для **ESP32-C6 SuperMini** на базе `esp-zigbee-sdk`, который определяется в **Home Assistant ZHA** как RGB-лампа с регулировкой яркости.

## Что реализовано

- Zigbee End Device (ZED).
- Профиль Home Automation (HA), устройство `Color Dimmable Light`.
- Endpoint `10`.
- Кластеры:
  - `Basic` (с `ManufacturerName`/`ModelIdentifier`),
  - `Identify`,
  - `On/Off`,
  - `Level Control`,
  - `Color Control` (XY).
- Обработка атрибутов от ZHA: включение, яркость, цвет (XY).
- Управление **встроенным RGB LED (WS2812)** на ESP32-C6 SuperMini через `led_strip`.

## Встроенный RGB LED ESP32-C6 SuperMini

В примере используется встроенный адресный светодиод:

- `DATA` → `GPIO8`
- Тип светодиода: `WS2812`
- Количество: `1`

Если у вашей ревизии платы другой пин встроенного светодиода, поменяйте `RGB_LED_GPIO` в `main/zigbee_light.c`.

## Привязка кнопкой на плате

- Для запуска привязки добавлена кнопка на `GPIO9` (кнопка `BOOT` на большинстве ESP32-C6 SuperMini).
- Кнопка активна по уровню `LOW` и опрашивается с антидребезгом.
- **Одинарное нажатие**: простой Binding (`network steering`) для подключения устройства к сети.
- **Двойное нажатие**: Direct Binding (`finding & binding`).

## Структура

- `main/zigbee_light.c` — логика Zigbee RGB лампы.
- `main/CMakeLists.txt` — регистрация компонента.
- `CMakeLists.txt` — корневой CMake проекта.
- `sdkconfig.defaults` — базовые дефолты для Zigbee на ESP32-C6.

## Сборка

Локально:

```bash
idf.py set-target esp32c6
idf.py -D CMAKE_BUILD_TYPE=Release build
idf.py flash monitor
```

Для отладочной сборки:

```bash
idf.py -D CMAKE_BUILD_TYPE=Debug build
```

## CI-сборки и релизы

В репозиторий добавлены GitHub Actions workflow:

- `.github/workflows/build.yml`:
  - запускается на `push`, `pull_request` и вручную (`workflow_dispatch`);
  - собирает `Release` и `Debug` для `esp32c6`;
  - публикует артефакты прошивки (`.bin`, `.elf`, `.map`).
- `.github/workflows/release.yml`:
  - запускается при пуше тега формата `v*` (например, `v1.0.0`) или вручную;
  - собирает релизную прошивку;
  - публикует GitHub Release c файлами прошивки (`.bin`, `.elf`, `.map`, `flasher_args.json`).


## Как сформировать релиз

1. Обновите `CHANGELOG.md` и укажите новую версию в заголовке (например, `## [0.1.1] - 2026-04-28`).
2. Закоммитьте изменения в `main`/`master`.
3. Сформируйте тег **строго по версии из `CHANGELOG.md`** и отправьте его:

```bash
VERSION=$(sed -nE 's/^## \[(.+)\] - .*/\1/p' CHANGELOG.md | head -n1)
git tag -a "v${VERSION}" -m "Release v${VERSION}"
git push origin "v${VERSION}"
```

4. После пуша тега workflow `.github/workflows/release.yml` автоматически соберёт прошивку и создаст GitHub Release с артефактами (`.bin`, `.elf`, `.map`, `flasher_args.json`).

Альтернатива: запустить workflow **Release firmware** вручную через `workflow_dispatch`, передав тег в том же формате (`v<версия>`).

## Подключение к ZHA

1. В Home Assistant: **Settings → Devices & Services → Zigbee Home Automation → Add Device**.
2. Нажмите кнопку `BOOT` на плате, чтобы запустить привязку.
3. После интервью устройство появится как RGB лампа.
4. Проверка: On/Off, Brightness и Color должны управляться из интерфейса ZHA и отображаться на встроенном RGB LED.

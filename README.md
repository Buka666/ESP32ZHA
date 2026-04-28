# ESP32-C6 Zigbee лампа с совместимостью ZHA

Минимальный шаблон Zigbee устройства типа **On/Off Light** для ESP32-C6 на базе `esp-zigbee-sdk` (в составе ESP-IDF компонентов), который корректно определяется в **Home Assistant ZHA** как лампа.

## Что реализовано

- Zigbee End Device (можно переключить на Router через `sdkconfig`).
- Профиль Home Automation (HA), устройство On/Off Light.
- Endpoint `10`.
- Кластер `Basic` с `ManufacturerName` и `ModelIdentifier` (важно для ZHA).
- Кластер `Identify`.
- Кластер `On/Off` с обработкой команд от координатора (ZHA).

## Структура

- `main/zigbee_light.c` — основная логика Zigbee устройства.
- `main/CMakeLists.txt` — регистрация компонента.
- `CMakeLists.txt` — корневой CMake проекта.
- `sdkconfig.defaults` — базовые дефолты для Zigbee на ESP32-C6.

## Сборка

> Требуется ESP-IDF с компонентом Zigbee (`esp-zigbee-sdk`) и целевой чип `esp32c6`.

```bash
idf.py set-target esp32c6
idf.py build
idf.py flash monitor
```

## Подключение к ZHA

1. Переведите устройство в режим Commissioning (в этом примере запускается автоматически после старта).
2. В Home Assistant откройте: **Settings → Devices & Services → Zigbee Home Automation → Add Device**.
3. После интервью устройство появится как лампа (On/Off).

## Важные заметки для совместимости ZHA

- Указывайте осмысленные `ManufacturerName` и `ModelIdentifier`.
- Используйте стандартные HA кластеры без vendor-specific расширений в минимальном MVP.
- Для стабильного повторного подключения храните NVRAM (по умолчанию включено в IDF).


# Changelog

Все заметные изменения в этом проекте фиксируются в этом файле.

Формат основан на [Keep a Changelog](https://keepachangelog.com/ru/1.1.0/),
а версияция следует [Semantic Versioning](https://semver.org/lang/ru/).

## [0.1.0] - 2026-04-28

### Added
- Базовый Zigbee шаблон лампы `Color Dimmable Light` для ESP32-C6 SuperMini.
- Поддержка кластеров `On/Off`, `Level Control`, `Color Control` (XY).
- Управление встроенным RGB LED (WS2812) и кнопкой `BOOT` для привязки.
- CI-сборки `Release`/`Debug` и автоматический GitHub Release по тегам `v*`.

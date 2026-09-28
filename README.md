# NUNU ESP32 Lab

Private monorepo for NUNU's ESP32 DIY experiments and products: Wi-Fi/BLE, chatbots, sensors, displays, automation, robotics, and AI-assisted hardware.

## Repository layout

```text
nunu-esp32-lab/
├── projects/
│   ├── esp32-wifi-test/
│   ├── esp32-chatbot/
│   ├── esp32-sensors/
│   ├── esp32-display/
│   └── esp32-robot/
├── shared/
│   ├── libraries/
│   └── components/
├── docs/
│   ├── wiring/
│   ├── notes/
│   └── experiments/
└── tools/
```

## Conventions

- Each project is self-contained and may have its own `platformio.ini`.
- Reusable code goes in `shared/`.
- Wiring diagrams, experiment logs, and notes go in `docs/`.
- Scripts and helper utilities go in `tools/`.
- Never commit Wi-Fi passwords, API keys, tokens, certificates, or other secrets.
- Keep secrets in ignored local files such as `include/secrets.h` or environment files.

## First project

The existing local `ESP32_WiFi_Test` project will be imported into:

```text
projects/esp32-wifi-test/
```

Target board currently used by NUNU: ESP32-C3 SuperMini / PlatformIO.

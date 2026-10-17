# ESP32 wiring check (robot1)

Diagnostic firmware for the Pocket ESP AI wiring on an ESP32-C3 SuperMini. It keeps an open
Wi-Fi AP (`NUNU-WIRECHECK`) running while it exercises the OLED, the INMP441 microphone and
the MAX98357A amplifier, and reports shorts, bridges, I2C devices, mic level and reset reasons.

```bash
pio run -t upload
pio device monitor   # keys: r=report b=beep t=toggle tx power w=wifi scan
```

Step-by-step procedure (Vietnamese): [docs/wiring/robot1-wiring-check.md](../../docs/wiring/robot1-wiring-check.md)

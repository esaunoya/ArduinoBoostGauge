# Arduino Boost Gauge

A DIY digital turbo/boost gauge for Arduino. It reads manifold pressure from an
I²C pressure sensor and shows the **live** and **peak** boost (in psi) on a
128×64 SH1106 OLED — large numeric readout, a horizontal bar gauge with per-psi
tick marks, and a peak-hold line.

## Features

- Live boost pressure as a large, right-aligned numeric readout (psi)
- Horizontal bar gauge (0–20 psi) with tick marks every psi and a taller mark every 5 psi
- **Peak-hold line** that tracks the highest boost seen since power-on
- Animated startup sweep that fills and empties the gauge
- `SENSOR ERROR` message if the sensor stops responding
- ~30 fps refresh

## Hardware

| Component | Details |
| --- | --- |
| Microcontroller | Any I²C-capable Arduino (e.g. Uno / Nano) |
| Display | 128×64 **SH1106** OLED, I²C address `0x3C` |
| Pressure sensor | I²C pressure transducer at address `0x28` with the Honeywell-style 10–90% transfer function (e.g. Honeywell ABP/HSC/SSC series), full-scale ≈ 145 psi (10 bar) |

> The sensor part number and exact full-scale range are inferred from the code's
> calibration constants — adjust `MAX_SENSOR_PRESSURE` if your sensor differs.

### Wiring

Both devices share the same I²C bus. Connect each to the Arduino's I²C pins:

| Device pin | Arduino (Uno/Nano) |
| --- | --- |
| VCC | 5V (or 3.3V per your sensor/display) |
| GND | GND |
| SDA | A4 |
| SCL | A5 |

On other boards use that board's SDA/SCL pins. Plumb the pressure sensor's port
to the intake manifold / charge pipe you want to measure.

## Libraries

Install these via the Arduino IDE Library Manager (**Sketch → Include Library →
Manage Libraries…**):

- [`Adafruit GFX Library`](https://github.com/adafruit/Adafruit-GFX-Library)
- [`Adafruit SH110X`](https://github.com/adafruit/Adafruit_SH110X)

`Wire` ships with the Arduino core.

## Build & upload

1. Open `BoostGauge.ino` in the Arduino IDE.
2. Install the libraries above.
3. Select your board and port, then **Upload**.
4. (Optional) Open the Serial Monitor at **9600 baud**.

## Configuration

The behavior is set by `#define`s at the top of `BoostGauge.ino`:

| Define | Default | Meaning |
| --- | --- | --- |
| `DISPLAY_ADDRESS` | `0x3C` | I²C address of the OLED |
| `SENSOR_ADDRESS` | `0x28` | I²C address of the pressure sensor |
| `MIN_SENSOR_PRESSURE` | `0.0` | Sensor minimum pressure (psi) |
| `MAX_SENSOR_PRESSURE` | `145.03773773` | Sensor maximum pressure (psi) |
| `MAX_GAUGE_PRESSURE` | `20` | Top of the displayed gauge scale (psi) |

## How it works

The sensor reports a 14-bit count over I²C. The sketch converts it to psi using
the standard Honeywell transfer function, where counts run from 10% to 90% of
the 14-bit range across the sensor's pressure span:

```
pressure = (raw - 1638) / 13107 * (MAX_SENSOR_PRESSURE - MIN_SENSOR_PRESSURE) + MIN_SENSOR_PRESSURE
```

`1638` ≈ 10% and `1638 + 13107 = 14745` ≈ 90% of 2¹⁴. The result is drawn as the
numeric readout and the bar gauge, and compared against the running peak to drive
the peak-hold line.

## License

Released under the [GNU GPL v3.0](LICENSE).

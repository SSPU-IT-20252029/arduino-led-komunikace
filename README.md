# 💡 Optický textový komunikátor (Arduino)

> Obousměrný (half-duplex) optický přenos ASCII textu mezi dvěma mikrokontroléry pomocí viditelného světla s adaptivní kalibrací okolního osvětlení a detekcí šumu.

[![Platform](https://img.shields.io/badge/Platform-Arduino%20(Nano%20%2F%20Uno)-00979D?style=for-the-badge&logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![Language](https://img.shields.io/badge/Language-C%2B%2B%20%2F%20Wiring-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)](https://en.wikipedia.org/wiki/C%2B%2B)
[![Protocol](https://img.shields.io/badge/Protocol-Custom%20Optical%20Serial-orange?style=for-the-badge)](main.cpp)
[![License](https://img.shields.io/badge/License-MIT-blue?style=for-the-badge)](LICENSE)

---

## 📌 Princip fungování

Vysílačem je standardní bílá LED, přijímačem je červená LED zapojená v závěrném směru na analogový vstup (využívající fotoelektrický jev). Kód disponuje **dynamickým prahováním**: při startu provede kalibraci okolního světla a během provozu automaticky filtruje drift okolního osvětlení, čímž eliminuje falešné detekce i zablokování při oslnění.

---

## 🔌 Hardware a zapojení

| Komponenta | Pin | Režim | Poznámka |
| :--- | :--- | :--- | :--- |
| **Bílá LED** (vysílač) | `D4` | `OUTPUT` | Přes ochranný rezistor (např. 220–330 Ω) na GND |
| **Červená LED** (senzor) | `A7` | `INPUT` (analog) | Katoda na `A7`, anoda na GND (měření fotoelektrického napětí) |

> [!NOTE]
> Pin `A7` je na deskách Arduino Nano / Pro Mini vyhrazený analogový vstup. Pokud používáte Arduino Uno, zapojte senzor na `A0`–`A5` a upravte `#define PIN_SENSOR` v [main.cpp](main.cpp).

---

## 📡 Komunikační protokol

Přenos probíhá sériově po jednotlivých znacích od nejnižšího bitu (LSB first):

1. **Start bit:** Světelný puls o délce `90 ms` (`CAS_START`), následovaný mezerou `30 ms` (`CAS_BITU`). Přijímač validuje puls v toleranci `75–200 ms`.
2. **Synchronizace čtení:** Po skončení startovacího pulsu přijímač vyčká `45 ms` (`1,5 × CAS_BITU`), čímž se vzorkování posune přesně do středu prvního datového bitu.
3. **Datové bity (8 bitů):** Každý bit trvá `30 ms`.
   - `HIGH` (světlo `> dynamický práh`) = logická `1`
   - `LOW` (světlo `<= dynamický práh`) = logická `0`
4. **Stop mezera:** Zhasnutí LED na `60 ms` (`2 × CAS_BITU`) před dalším znakem.
5. **Konec zprávy:** Po odeslání celého řetězce se automaticky odešle znak nového řádku `\n` (ASCII `10`).

### Přenosová rychlost
- **Doba přenosu 1 znaku:** `90 + 30 + (8 × 30) + 60 = 420 ms` (~2,38 znaku/s)

---

## ⚙️ Adaptivní prahování & Ochrana proti oslepení

- **Automatická kalibrace při startu:** Funkce `kalibrujOkolniSvetlo()` zprůměruje 20 vzorků z ADC a nastaví detekční hranici na `klidová hladina + 35`.
- **Adaptivní klidový drift:** Pomalé změny denního světla v místnosti jsou průběžně vyhlazovány exponenciálním průměrováním bez ovlivnění rychlých datových pulsů.
- **Detekce oslepení:** Pokud na senzor dopadne trvalé světlo (delší než 200 ms), mikrokontrolér se nezablokuje v nekonečné smyčce, ale ohlásí varování a provede rekalibraci na novou úroveň okolního osvětlení.

---

## 🚀 Jak spustit

1. Otevřete [main.cpp](main.cpp) v Arduino IDE nebo VS Code (rozšíření PlatformIO).
2. Nahrajte kód do obou Arduino desek.
3. Otevřete **Serial Monitor** na rychlosti **9600 baud**.
4. Napište text do horního řádku a stiskněte **Enter**.

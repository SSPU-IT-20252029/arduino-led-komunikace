# Optický textový komunikátor (Arduino)

Obousměrný (half-duplex) optický přenos ASCII textu mezi dvěma mikrokontroléry pomocí viditelného světla. Vysílačem je běžná bílá LED, přijímačem červená LED zapojená jako fotodioda na analogovém vstupu. Kód obsahuje integrovanou diagnostiku okolního světla a detekci šumu.

## Hardware a zapojení

| Komponenta | Pin | Režim | Poznámka |
| :--- | :--- | :--- | :--- |
| **Bílá LED** (vysílač) | `D4` | `OUTPUT` | Přes ochranný rezistor na GND |
| **Červená LED** (senzor) | `A7` | `INPUT` (analog) | Katoda na `A7`, anoda na GND (měření fotoelektrického napětí) |

> **Poznámka:** Pin `A7` je na deskách Arduino Nano / Pro Mini čistě analogový vstup. Na Arduino Uno použijte `A0`–`A5`.

## Komunikační protokol

Přenos probíhá sériově po jednotlivých znacích (LSB first – od nejnižšího bitu):

1. **Start bit:** Světelný puls o délce `90 ms` (`CAS_START`), následovaný mezerou `30 ms` (`CAS_BITU`). Přijímač akceptuje puls v délce `75–500 ms`.
2. **Synchronizace čtení:** Po skončení startovacího pulsu přijímač vyčká `45 ms` (`1,5 × CAS_BITU`), čímž se vzorkování nastaví přesně do středu prvního datového bitu.
3. **Datové bity (8 bitů):** Každý bit trvá `30 ms`.
   - `HIGH` (světlo `> 245`) = logická `1`
      - `LOW` (světlo `<= 245`) = logická `0`
      4. **Stop mezera:** Zhasnutí LED na `60 ms` (`2 × CAS_BITU`) před dalším znakem.
      5. **Konec zprávy:** Po odeslání celého řetězce se automaticky odešle znak `\n` (ASCII `10`).

      ### Rychlost přenosu
      - **Doba přenosu 1 znaku:** `90 + 30 + (8 × 30) + 60 = 420 ms` (~2,38 znaků/s)

      ## Konfigurace v kódu

      ```cpp
      const int HRANICE_SVETLA = 245; // Prahová hodnota ADC (0-1023) pro logickou 1
      const int CAS_BITU = 30;        // Délka jednoho bitu (ms)
      const int CAS_START = 90;       // Délka startovacího záblesku (ms)
      

# Optický textový komunikátor (Arduino)

Obousměrný (half-duplex) optický přenos ASCII textu mezi dvěma deskami Arduino pomocí viditelného světla. 

Vysílačem je běžná bílá LED, přijímačem červená LED zapojená v závěrném směru na analogový vstup (využití fotoelektrického napětí). Kód obsahuje automatickou kalibraci okolního osvětlení při startu, průběžné přizpůsobení driftu denního světla a ochranu proti zablokování při oslnění.

## Hardware a zapojení

| Komponenta | Pin | Režim | Zapojení |
| :--- | :--- | :--- | :--- |
| **Bílá LED** (vysílač) | `D4` | `OUTPUT` | Anoda na D4 přes 220–330 Ω rezistor, katoda na GND |
| **Červená LED** (senzor) | `A7` | `INPUT` (analog) | Katoda na A7, anoda na GND |

*Poznámka:* Pin `A7` je na deskách Arduino Nano / Pro Mini vyhrazený čistě analogový vstup. Na desce Arduino Uno použijte pin `A0`–`A5` a upravte `#define PIN_SENSOR` v `main.cpp`.

## Komunikační protokol

Přenos probíhá sériově po jednotlivých znacích od nejnižšího bitu (LSB first):

1. **Start puls:** Rozsvícení bílé LED na `90 ms` (`CAS_START`), následované mezerou `30 ms` (`CAS_BITU`). Přijímač validuje startovní puls v rozmezí `75–200 ms`.
2. **Synchronizace čtení:** Po skončení startovního pulsu přijímač vyčká `45 ms` (`1.5 × CAS_BITU`), čímž se vzorkování posune přesně doprostřed prvního datového bitu.
3. **Datové bity (8 bitů):** Každý bit trvá `30 ms`.
   - Úroveň nad prahem = logická `1`
   - Úroveň pod prahem = logická `0`
4. **Stop mezera:** Zhasnutí LED na `60 ms` (`2 × CAS_BITU`) před dalším znakem.
5. **Konec zprávy:** Po odeslání celého řetězce se odešle znak nového řádku `\n` (ASCII `10`).

Přenos jednoho znaku trvá přibližně `420 ms` (~2.4 znaku za sekundu).

## Adaptivní prahování a ochrana proti oslnění

- **Počáteční kalibrace:** Funkce `kalibrujOkolniSvetlo()` v `setup()` odebere 20 vzorků z ADC a nastaví rozhodovací práh na `klidová hladina + 35`.
- **Adaptivní klidový drift:** Pomalé změny ambientního světla v místnosti jsou průběžně vyhlazovány exponenciálním průměrem bez vlivu na rychlé datové impulsy.
- **Detekce oslnění:** Pokud na senzor dopadne trvalé světlo delší než 200 ms, mikrokontrolér nečeká v nekonečné smyčce, ale ohlásí varování na sériovou linku a rekalibruje se na novou úroveň osvětlení.

## Použití

1. Nahrajte `main.cpp` do obou Arduino desek.
2. Otevřete Serial Monitor na přenosové rychlosti **9600 baud**.
3. Napište text do vstupního řádku a stiskněte Enter.

## Licence

MIT

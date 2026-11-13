#define PIN_WHITE  4   // Vysílací bílá LED
#define PIN_SENSOR A7  // Přijímací červená LED

// Časové konstanty přenosu (v ms)
const int CAS_BITU = 30;       // Délka jednoho bitu v ms
const int CAS_START = 90;      // Délka startovacího záblesku (3x delší než bit)
const int PRAHOVY_OFFSET = 35; // Bezpečnostní rezerva nad klidovým okolním světlem

// Dynamické prahování a kalibrace
int klidovaUroven = 200;       // Průběžný klidový průměr okolního světla
int hraniceSvetla = 245;       // Aktuální dynamická prahová hodnota
unsigned long posledniDebug = 0; // Časovač pro výpis diagnostiky

// Funkce prototypy
void kalibrujOkolniSvetlo();
void odesliZnak(char znak);
char prectiZnak();

void kalibrujOkolniSvetlo() {
  long suma = 0;
  const int vzorku = 20;
  for (int i = 0; i < vzorku; i++) {
    suma += analogRead(PIN_SENSOR);
    delay(10);
  }
  klidovaUroven = suma / vzorku;
  hraniceSvetla = klidovaUroven + PRAHOVY_OFFSET;
  if (hraniceSvetla > 1000) hraniceSvetla = 1000;

  Serial.print("[KALIBRACE] Okolni svetlo: ");
  Serial.print(klidovaUroven);
  Serial.print(" | Nastaveny prah: ");
  Serial.println(hraniceSvetla);
}

void setup() {
  pinMode(PIN_WHITE, OUTPUT);
  digitalWrite(PIN_WHITE, LOW);

  Serial.begin(9600);
  Serial.println("=== OPTICKY TEXTOVY KOMUNIKATOR + ADAPTIVNI PRAHOVANI ===");
  kalibrujOkolniSvetlo();
  Serial.println("Napis zpravu nahore do radku a stiskni Enter:");
}

void loop() {
  int aktualniSvetlo = analogRead(PIN_SENSOR);

  // 0. ADAPTIVNÍ DRIFT: Pokud je klid, pomalu adaptujeme referenční hladinu
  if (aktualniSvetlo < hraniceSvetla) {
    klidovaUroven = (klidovaUroven * 15 + aktualniSvetlo) / 16;
    hraniceSvetla = klidovaUroven + PRAHOVY_OFFSET;
  }

  // 1. DIAGNOSTIKA KLIDOVÉHO SVĚTLA (každých 500 ms)
  if (millis() - posledniDebug > 500) {
    Serial.print("[DEBUG] Klidove svetlo: ");
    Serial.print(aktualniSvetlo);
    Serial.print(" / Prah: ");
    Serial.println(hraniceSvetla);
    posledniDebug = millis();
  }

  // 2. ODESÍLÁNÍ: Zpráva zadaná do Serial Monitoru
  if (Serial.available() > 0) {
    String zprava = Serial.readStringUntil('\n');
    zprava.trim();

    if (zprava.length() > 0) {
      Serial.print("Ja: ");
      Serial.println(zprava);
      
      for (size_t i = 0; i < zprava.length(); i++) {
        odesliZnak(zprava[i]);
      }
      odesliZnak('\n'); 
      posledniDebug = millis();
    }
  }

  // 3. PŘÍJEM: Detekce světelného pulzu nad dynamickým prahem
  if (aktualniSvetlo > hraniceSvetla) {
    unsigned long zacatekSvetla = millis();
    int maxSvetlo = aktualniSvetlo;

    // Čekáme na konec pulzu s timeoutem proti oslepení
    while (true) {
      int cteni = analogRead(PIN_SENSOR);
      if (cteni > maxSvetlo) maxSvetlo = cteni;
      if (cteni <= hraniceSvetla) break;
      if (millis() - zacatekSvetla > 400) break; // Ochrana proti zablokování při trvalém světle
    }

    unsigned long delkaSvetla = millis() - zacatekSvetla;

    // Validní startovací záblesk (musí mít alespoň CAS_START - 15 ms a méně než 200 ms)
    if (delkaSvetla >= (CAS_START - 15) && delkaSvetla <= 200) {
      Serial.print("[DEBUG START] Delka pulsu: ");
      Serial.print(delkaSvetla);
      Serial.print(" ms | Max svetlo: ");
      Serial.println(maxSvetlo);

      char prijatyZnak = prectiZnak();
      
      if (prijatyZnak == '\n') {
        Serial.println("\n--- Konec zpravy ---");
      } else if (prijatyZnak >= 32 && prijatyZnak <= 126) {
        Serial.print("Prijaty znak: ");
        Serial.println(prijatyZnak);
      } else {
        Serial.print("[DEBUG CHYBA] Neznamy znak (ASCII): ");
        Serial.println((int)prijatyZnak);
      }
    } else if (delkaSvetla > 200) {
      // Světlo svítilo příliš dlouho -> vnější osvětlení / oslepení senzoru
      Serial.print("[VAROVANI] Trvale svetlo/oslepeni (");
      Serial.print(delkaSvetla);
      Serial.println(" ms), rekalibruji...");
      kalibrujOkolniSvetlo();
    } else if (delkaSvetla > 5) {
      // Ignoruje extrémně krátké záblesky (1-5 ms), ale vypíše delší šum
      Serial.print("[DEBUG SUM] Kratky zablesk (");
      Serial.print(delkaSvetla);
      Serial.print(" ms), max: ");
      Serial.println(maxSvetlo);
    }
    
    posledniDebug = millis();
  }
}

void odesliZnak(char znak) {
  digitalWrite(PIN_WHITE, HIGH);
  delay(CAS_START);
  digitalWrite(PIN_WHITE, LOW);
  delay(CAS_BITU);

  for (int bit = 0; bit < 8; bit++) {
    int hodnotaBitu = bitRead(znak, bit);
    digitalWrite(PIN_WHITE, hodnotaBitu == 1 ? HIGH : LOW);
    delay(CAS_BITU);
  }

  digitalWrite(PIN_WHITE, LOW);
  delay(CAS_BITU * 2);
}

char prectiZnak() {
  char vyslednyZnak = 0;

  delay(CAS_BITU + (CAS_BITU / 2));

  Serial.print("  Bity (uroven svetla): ");
  for (int bit = 0; bit < 8; bit++) {
    int svetlo = analogRead(PIN_SENSOR);
    Serial.print(svetlo);
    Serial.print(" ");
    
    if (svetlo > hraniceSvetla) {
      bitWrite(vyslednyZnak, bit, 1);
    } else {
      bitWrite(vyslednyZnak, bit, 0);
    }
    
    delay(CAS_BITU);
  }
  Serial.println();

  return vyslednyZnak;
}

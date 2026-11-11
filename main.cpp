#define PIN_WHITE  4   // Vysílací bílá LED
#define PIN_SENSOR A7  // Přijímací červená LED

const int HRANICE_SVETLA = 245; // Prahová hodnota pro rozpoznání světla
const int CAS_BITU = 30;       // Délka jednoho bitu v ms
const int CAS_START = 90;      // Délka startovacího záblesku (3x delší než bit)

unsigned long posledniDebug = 0; // Časovač pro výpis klidového světla

void setup() {
  pinMode(PIN_WHITE, OUTPUT);

  Serial.begin(9600);
  Serial.println("=== OPTICKY TEXTOVY KOMUNIKATOR + DEBUG ===");
  Serial.println("Napis zpravu nahore do radku a stiskni Enter:");
}

void loop() {
  int aktualniSvetlo = analogRead(PIN_SENSOR);

  // 0. DEBUG KLIDOVÉHO SVĚTLA: Vypíše úroveň každých 500 ms (neblokuje příjem)
  if (millis() - posledniDebug > 500) {
    Serial.print("[DEBUG] Klidove svetlo: ");
    Serial.print(aktualniSvetlo);
    Serial.print(" / Limit: ");
    Serial.println(HRANICE_SVETLA);
    posledniDebug = millis();
  }

  // 1. ODESÍLÁNÍ: Pokud jsi napsal text do Serial Monitoru
  if (Serial.available() > 0) {
    String zprava = Serial.readStringUntil('\n');
    zprava.trim();

    if (zprava.length() > 0) {
      Serial.print("Ja: ");
      Serial.println(zprava);
      
      for (int i = 0; i < zprava.length(); i++) {
        odesliZnak(zprava[i]);
      }
      odesliZnak('\n'); 
      posledniDebug = millis(); // Reset debug časovače po odeslání
    }
  }

  // 2. PŘÍJEM: Červená LED zachytila světlo z druhého zařízení
  if (aktualniSvetlo > HRANICE_SVETLA) {
    unsigned long zacatekSvetla = millis();
    int maxSvetlo = aktualniSvetlo;

    while (true) {
      int cteni = analogRead(PIN_SENSOR);
      if (cteni > maxSvetlo) maxSvetlo = cteni; // Zaznamená špičku signálu
      if (cteni <= HRANICE_SVETLA) break;
      if (millis() - zacatekSvetla > 500) break;
    }

    unsigned long delkaSvetla = millis() - zacatekSvetla;

    if (delkaSvetla >= (CAS_START - 15)) {
      Serial.print("[DEBUG START] Delka pulsu: ");
      Serial.print(delkaSvetla);
      Serial.print(" ms | Max uroven svetla: ");
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
    } else if (delkaSvetla > 5) {
      // Ignoruje extrémně krátké záblesky (1-5 ms), ale vypíše delší rušení
      Serial.print("[DEBUG SUM] Kratky zablesk (");
      Serial.print(delkaSvetla);
      Serial.print(" ms), max svetlo: ");
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
    
    if (hodnotaBitu == 1) {
      digitalWrite(PIN_WHITE, HIGH);
    } else {
      digitalWrite(PIN_WHITE, LOW);
    }
    
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
    
    if (svetlo > HRANICE_SVETLA) {
      bitWrite(vyslednyZnak, bit, 1);
    } else {
      bitWrite(vyslednyZnak, bit, 0);
    }
    
    delay(CAS_BITU);
  }
  Serial.println();

  return vyslednyZnak;
}

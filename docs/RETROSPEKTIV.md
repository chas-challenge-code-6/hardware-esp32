# 📝 **Retrospektiv – Sentinel-projektet**

## 🔁 1. Projektmål och sammanfattning  
Syftet med projektet var att ta fram ett bärbart hälsoövervakningssystem – *Sentinel* – som kombinerar flera sensorer för att mäta puls, temperatur, luftkvalitet, fall och rörelse. Systemet består av en ESP32-baserad enhet som kommunicerar med ett externt pulsband (Polar HRT H7), samlar in data och skickar det till ett backend-system via LTE/mobilnät.

Vi lyckades få alla centrala funktioner på plats, inklusive datainsamling, automatisk BLE-parning, lagring och visualisering i både serial monitor och i en applikation. Även om vissa delar tog längre tid än planerat och vissa delar behövde läggas i papperskorgen o därefter tänkas om, blev projektet i sin helhet funktionellt och stabilt.

---

## 💪 2. Det som gick bra  
- **BLE-parning med HRT-bandet** fungerade automatiskt och stabilt tack vare god förberedelse och förståelse för BLE-protokollet.  
- **Teamet hade god kommunikation**, särskilt under felsökning.  
- **ESP32-plattformen** visade sig vara tillräckligt kraftfull och flexibel för våra behov.  
- Dataöverföring till backend via LTE fungerade som planerat efter mycket testning och felaktigheter.

---

## ⚠️ 3. Det som var utmanande  
- Det tog tid att få alla sensorer att fungera tillsammans över **I2C**, då vissa adresser krockade. Löste sig genom att ändra prioriteringar i xTaskCreate. 
- **GPS och LTE** tog oväntat lång tid att initialisera, vilket gjorde realtidsfunktioner tröga.  
- **Debugging av hårdvara** (kablar, lödningar, kontakter) var tidskrävande och ibland frustrerande.
- **Wasp-OS och PineTime** var problematisk att få tillgång till datan. Mycket tid behövde läggas på detta, tillslut stannade den i Bootläge och kunde inte längre användas med tanke på tidsbrist.

---

## 🔧 4. Tekniska lärdomar  
- Vi lärde oss mycket om **BLE och hur man handskas med anslutningar i realtid**.  
- Ökad förståelse för **FreeRTOS och multitasking**, vilket hade kunnat utnyttjas mer.  
- Vikten av **att testa alla komponenter tillsammans**, inte bara separat.  
- Hur **API-design** påverkar både frontend och backend-prestanda.

---

## 🤝 5. Samarbete och arbetsprocess  
- Vi använde **Github-projects**, vilket hjälpte oss hålla koll på arbetsfördelning.  
- Veckovis kort avstämning bidrog till snabbare problemlösning.  
- Dokumentationen försöktes utvecklas löpande men kan förbättras i tydlighet och struktur.  
- Vi använde GitHub effektivt för versionshantering och pull requests.

---

## 📈 6. Vad kan förbättras till nästa gång?  
- Börja med **prototyper snabbare** – även enkla versioner hjälper till att hitta fel i tid.   
- Fokusera på **tidig integration av alla delar**: hårdvara, backend och frontend.
- Bättre sammanarbete mellan alla parter för att utöka våra kunskaper och hjälpa de som fastnat.

---

## 🛠️ 7. Övrigt  
- Arbetet med hårdvaran var både utmanande och väldigt roligt – känslan när LED-lampan lyste för första gången var magisk eftersom vi visste att den kunde användas utan problem.
- Vi hade många kreativa idéer under testningen, bl.a. om framtida funktioner som realtidsvarningar och möjligheten att kunna ta fram ett case för produkten..  

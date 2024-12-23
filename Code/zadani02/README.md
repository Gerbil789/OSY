# Chat

Upravte program soketového serveru tak, aby pro každého nově připojeného klienta vytvořil samostatné vlákno.

Každou zprávu kterou vlákno příjme, přepošle všem připojeným klientům, ne sobě.

Připojení klienti budou evidováni v poli pevné délky. Omezení počtu klientů bude globální konstantou.

Činnost serveru:
- server příjme nové spojení a vytvoří pro každého klienta vlákno.
  - pro klienta se najde volné místo v seznamu.
  - není-li volné místo, klient je o tom informován a spojení končí.
  - opakuje se příjem zpráv od klienta a rozesílání všem připojeným klientům, né sobě.
  - rozesílaná zpráva musí obsahovat informaci o tom, který klient zprávu rozesílá.
  - při ukončení spojení se v seznamu klientů označí jeho místo jako volné.
  - zavření soketu, ukončení vlákna.

Práci se seznamem klientů je nutno považovat za kritickou sekci. Proto je potřeba přidání klienta, odebrání klienta a rozesílání zpráv vhodně ošetřit pomocí semaforu.

---

Pokud bude první slovo zprávy od klienta LOCK/UNLOCK, tak klient zamkne/odemkne rozesílání zpráv ostatním klientům. Tento zámek však bude automaticky po 5 zprávách zrušen.
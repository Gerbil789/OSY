
# HTTP server pro úroveň nabití baterie.
Upravte soketový server tak, aby poždavek GET z prohlížeče interpretoval jako úroveň nabití baterie v rozsahu 0 až 100%.

Odpověd serveru bude jeden z předpřipravených obrazků "battery-000/020/040/060/080/100.png"

Po zadání požadavku např. "http://localhost:3333/level-68/300x300" obdrží server request např.:

    GET /level-68/300x300 HTTP/1.1
    Host: localhost:3333
    User-Agent: Mozilla/5.0
    Accept: text/html,application/xhtml+xml,application/xml;
    ...
Za úroveň nabití bude považován text "level-NNN" mezi "GET" a "HTTP".

Server pro každého nového klienta provede následující kroky:
- pro nového klienta vytvoří nový proces, kód procesu bude v samostatné funkci
	- příjme request a vybere z jeho začátku úroven nabití (a rozlišení)
	- pokud nenajde "GET" a "HTTP", zavře jeho spojení a končí.
	- odešle hlavičku, viz níže
	- vytvoří proces pro příkaz
	- provede přesměrování STDOUT do sokuetu a provede příkaz:
		- "convert battery-080.png -", nebo
		- "convert -resample 300x300 battery-080.png -",
- počká na dokončení příkazu
- končí

Ošetřete pomocí semaforu, aby se nikdy neprováděly dva příkazy současně

HEAD:

    "HTTP/1.1 200 OK\n"
    "Server: OSY/1.1.1 (Ubuntu)\n"
    "Accept-Ranges: bytes\n"
    "Vary: Accept-Encoding\n"
    "Content-Type: image/png\n\n"


---
#### moje poznamky:
- **convert** nemusí být na pc nainstalovaný.
- Kontrola: `convert --version`
- Instalace: `sudo apt-get install imagemagick`


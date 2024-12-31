Soketový server provádí příkazy zadané z internetového prohlížeče.

Upravte soketový server tak, aby požadavek GET z prohlížeče interpretoval jako systémový příkaz s parametry, provedený na výstup do soketu.
Po zadání např. "http://localhost:3333/ls" obdrží server request např.:

GET /ls HTTP/1.1
Host: localhost:3333
User-Agent: Mozilla/5.0
Accept: text/html,application/xhtml+xml,application/xml;

Za příkaz bude považován text mezi "GET" a "HTTP".

Server pro každého nového klienta provede následující kroky:

- pro nového klienta vytvoří nový proces, kde postupně dostane funkci,
- extrahuje textový příkaz z počátku příkazu,
- pokud nenajde "GET" a "HTTP", zavře spojení a končí,
- odešle hlavičku, viz níže,
- vytvoří proces pro příkaz,
- provede přesměrování výstupu a příkaz,
- počká na dokončení příkazu,
- odešle patičku,
- končí.

Ošetřete pomocí semaforu, aby se nikdy neprováděly dva příkazy současně.

HEAD:
"HTTP/1.1 200 OK\n"
"Server: OSY/1.1.1 (Ubuntu)\n"
"Accept-Ranges: bytes\n"
"Vary: Accept-Encoding\n"
"Content-Type: text/html\n"
"<!DOCTYPE html>\n"
"<html>\n"
"<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /></head>\n"
"<body><h2><pre>"

FOOTER:
"</pre></h2></body></html>\n"

Předejte příkazu i parametry z requestu. Např. požadavek na "ls -la /etc" bude v prohlížeči zadán jako "http://localhost:3333/ls-la/-la/etc", kde je "/" zadán místo mezery. To je mnohem snadnější použít. Tento požadavek se do requestu promítne následující:

GET /ls-la/-la/etc HTTP/1.1


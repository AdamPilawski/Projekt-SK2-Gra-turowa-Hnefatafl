# Gra turowa Hnefatafl
## Konfiguracja
- Kompilacja Serwera
```
gcc -Wall hnefataflServer.c -o hnefataflServer.o
```
- Kompilacja Klienta
```
gcc -Wall hnefataflClient.c -o hnefataflClient.o
```
- Uruchomienie Serwera
```
./hnefataflServer.o <port>
```
Parametr `<port>` jest opcjonalny, domyślnie przyjmuje 1111.

- Uruchomienie Serwera
```
./hnefataflClient.o <adres> <port>
```
Parametr `<adres>` jest wymagany, `<port>` jest opcjonalny, domyślnie przyjmuje 1111.

## Rozgrywka
- Aby wykonać ruch pionkiem należy wpisać dane z uwzględnieniem spacji w postaci:
```
<obecna_kolumna> <obecny_wiersz> <nowa_kolumna> <nowy_wiersz>
```
- Kolumny należy wpisywać jako duże lub małe litery z zakresu A-I/a-i.
- Wiersze należy wpisywać jako liczby z zakresu 1-9.

## Zasady gry
* Rozgrywka odbywa się na planszy 9x9.
*	Obrońca ma króla i 8 pionków, atakujący otacza go 16 pionkami.
* Król zajmuje centralne pole na planszy, tzw. tron. Tylko król może tam przebywać, inne pionki mogą jedynie przez niego przechodzić, król może wrócić na tron.
*	Grę rozpoczyna atakujący.
*	Każdy gracz musi poruszyć jeden pionek w swojej rundzie, nie można pominąć ruchu ani poruszać więcej pionków niż jeden.
*	Pionki poruszają się tylko w linii prostej o dowolną liczbę pól, nie można jednak przeskakiwać przez inne pionki.
* Pionki bije się otaczając je z dwóch stron.
*	Pionek może wejść pomiędzy dwóch przeciwników w swoim ruchu i nie jest wtedy bity (tzw. szarża).
*	Król bierze udział w biciu pionków na normalnych zasadach.
*	Król jest bity jeżeli:
    *	jest na tronie i zostanie otoczony z czterech stron.
    *	jest tuż obok tronu i zostaje otoczony z trzech stron, z czwartej zaś strony przez tron,
    *	na planszy tak samo jak inne pionki, otoczony z dwóch stron,
*	Obrońca wygrywa jeśli doprowadzi króla do jednego z czterech pól w rogach planszy.
*	Pola w rogach planszy i tron działają jak przeciwnicy dla obu stron, tzn. pionek umieszczony tuż obok rogu może zostać zbity jeśli po drugiej stronie znajdzie się przeciwnik.


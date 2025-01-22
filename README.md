# Projekt-SK2-Gra-turowa-Hnefatafl
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
Parametr <port> jest opcjonalny, domyślnie przyjmuje 1111.

- Uruchomienie Serwera
```
./hnefataflClient.o <adres> <port>
```
Parametr <adres> jest wymagany, <port> jest opcjonalny, domyślnie przyjmuje 1111.

## Rozgrywka
- Aby wykonać ruch pionkiem należy wpisać dane z uwzględnieniem spacji w postaci:
```
<obecna_kolumna> <obecny_wiersz> <nowa_kolumna> <nowy_wiersz>
```
- Kolumny należy wpisywać jako duże lub małe litery z zakresu A-I/a-i.
- Wiersze należy wpisywać jako liczby z zakresu 1-9.

# GoPoznań - Projekt Zaliczeniowy JPO
Autor: Stas

## Opis Projektu
GoPoznań to inteligentna wyszukiwarka połączeń komunikacji miejskiej ZTM Poznań.
Aplikacja została napisana w C++ (Qt6 Widgets) z użyciem wielowątkowości. Pobiera otwarte dane GTFS,
przetwarza je skryptem w Pythonie (wyszukiwanie trasy), a następnie komunikuje się z
lokalnym dużym modelem językowym (LLM Ollama - qwen2.5:3b) poprzez REST API, aby w przyjazny sposób
opisać znalezione połączenie. LLM dodatkowo generuje kod Pythona do wyrysowania wykresu,
który aplikacja automatycznie uruchamia i wyświetla w oknie.

Aplikacja jest odporna na brak połączenia sieciowego i błędy środowiskowe (informuje o problemach i nie zawiesza GUI).

## Wymagania systemowe
1. Środowisko C++ z zainstalowaną biblioteką Qt 6 (moduły: Widgets, Network, Concurrent, Test).
2. Zainstalowany i uruchomiony lokalnie serwer Ollama z pobranym modelem `qwen2.5:3b`
   (Można uruchomić poprzez komendę w terminalu: `ollama run qwen2.5:3b`).
3. Środowisko Python (w wersji 3.x) dodane do zmiennej środowiskowej PATH.

## Instalacja bibliotek Pythona
Przed pierwszym uruchomieniem aplikacji należy zainstalować wymagane biblioteki:
```bash
pip install -r requirements.txt
```

## Uruchomienie projektu (Aplikacja główna)
1. Otwórz projekt (plik `CMakeLists.txt`) w Qt Creator.
2. Z lewej strony w zakładce "Projekty" upewnij się, że wybrany jest "Zbuduj (Build)" i profil kompilatora (np. MinGW).
3. Na samym dole po lewej kliknij w ikonę komputera (Target) i wybierz docelowy program: `zal_jpo`.
4. Skompiluj projekt (Ctrl+B) i uruchom (Ctrl+R lub zielony trójkąt).
5. Przy pierwszym uruchomieniu aplikacja automatycznie pobierze plik GTFS z serwerów ZTM Poznań.

## Uruchomienie Testów Jednostkowych (Opcja na 5.0)
Aby uruchomić pełen zestaw testów weryfikujących logikę:
1. Podobnie jak wyżej, w Qt Creator kliknij na dole po lewej w ikonę komputera (Wybór docelowego profilu / Target).
2. Zamiast `zal_jpo` z listy rozwijanej wybierz **`GoPoznanTests`**.
3. Uruchom projekt (Ctrl+R lub zielony trójkąt).
4. W oknie komunikacyjnym na dole Qt Creatora (Application Output / Wyjście aplikacji) pojawi się raport z testów, z wypisanymi wynikami (PASS/FAIL) dla każdego sprawdzenia.

## Generowanie Dokumentacji (Doxygen)
Aby wygenerować dokumentację kodu:
1. Zainstaluj program Doxygen.
2. Będąc w głównym katalogu projektu uruchom komendę:
   `doxygen Doxyfile`
3. Dokumentacja HTML pojawi się w folderze `docs/html`. Otwórz plik `docs/html/index.html`.

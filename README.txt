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

## Uruchomienie projektu
1. Otwórz projekt (plik `CMakeLists.txt`) w Qt Creator.
2. Skonfiguruj projekt za pomocą CMake.
3. Skompiluj projekt (Ctrl+B lub zielony młotek).
4. Uruchom projekt (Ctrl+R lub zielony trójkąt).
5. Przy pierwszym uruchomieniu aplikacja automatycznie pobierze plik GTFS z serwerów ZTM Poznań.

## Generowanie Dokumentacji (Doxygen)
Aby wygenerować dokumentację kodu:
1. Zainstaluj program Doxygen.
2. Będąc w głównym katalogu projektu uruchom komendę:
   `doxygen Doxyfile`
3. Dokumentacja HTML pojawi się w folderze `html`. Otwórz plik `html/index.html`.

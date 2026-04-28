"""
Plik: draw_chart.py
Opis: Skrypt generujący wykres z danymi odjazdów za pomocą biblioteki matplotlib.
      Wykres ten jest podpinany do pliku PDF na koniec działania aplikacji.
      Stanowi to realizację wymogu 5.0 (wykonanie polecenia generującego wykres).
"""
import sys
import matplotlib.pyplot as plt

def main():
    # Oczekiwany format: python draw_chart.py <hour1> <count1> <hour2> <count2> ...
    args = sys.argv[1:]
    
    if len(args) < 2:
        print("Nie podano wystarczających danych do wykresu.")
        sys.exit(1)

    labels = []
    counts = []

    for i in range(0, len(args) - 1, 2):
        hour = str(args[i])
        labels.append(f"{hour}:00")
        try:
            counts.append(int(args[i+1]))
        except ValueError:
            counts.append(0)

    # Rysujemy wykres
    plt.figure(figsize=(9, 5))
    bars = plt.bar(labels, counts, color='#2196F3')
    plt.title('Liczba odjazdów z przystanku w poszczególnych godzinach (0:00 - 12:00)', fontsize=14)
    plt.xlabel('Godzina', fontsize=12)
    plt.ylabel('Suma odjazdów', fontsize=12)
    
    # Etykiety na słupkach
    for bar in bars:
        yval = bar.get_height()
        if yval > 0:
            plt.text(bar.get_x() + bar.get_width()/2, yval + 0.1, f'{int(yval)}', ha='center', va='bottom', fontsize=10)

    plt.tight_layout()
    plt.savefig('chart.png')

if __name__ == "__main__":
    main()

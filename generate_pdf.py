"""
Plik: generate_pdf.py
Opis: Skrypt do tworzenia plików PDF (raportów) przy użyciu biblioteki fpdf.
      Wczytuje on wynik działania modelu językowego (z report_text.txt) i opcjonalny wykres (chart.png).
"""
import sys
import os
from fpdf import FPDF

def generate_pdf():
    pdf = FPDF()
    pdf.add_page()
    
    # Używamy standardowej czcionki Arial lub domyślnej
    pdf.set_font("Helvetica", 'B', 16)
    pdf.cell(200, 10, txt="Raport Podrozy - GoPoznan", ln=True, align='C')
    pdf.ln(10)

    pdf.set_font("Helvetica", size=12)
    
    # Wczytanie tekstu z AI
    try:
        with open('report_text.txt', 'r', encoding='utf-8') as f:
            lines = f.readlines()
            for line in lines:
                # Zamiana polskich znaków jeśli czcionka Helvetica by ich nie łykała,
                # FPDF ma problemy z UTF-8 na domyślnych czcionkach.
                # W prostym projekcie użyjemy łacinki lub podmienimy znaki:
                line = line.replace('ą', 'a').replace('ć', 'c').replace('ę', 'e')\
                           .replace('ł', 'l').replace('ń', 'n').replace('ó', 'o')\
                           .replace('ś', 's').replace('ź', 'z').replace('ż', 'z')\
                           .replace('Ą', 'A').replace('Ć', 'C').replace('Ę', 'E')\
                           .replace('Ł', 'L').replace('Ń', 'N').replace('Ó', 'O')\
                           .replace('Ś', 'S').replace('Ź', 'Z').replace('Ż', 'Z')
                pdf.multi_cell(0, 10, txt=line.strip())
    except FileNotFoundError:
        pdf.cell(200, 10, txt="Brak opisu trasy.", ln=True)

    pdf.ln(10)
    
    # Dodanie wykresu jeśli istnieje
    if os.path.exists('chart.png'):
        pdf.cell(200, 10, txt="Analiza zatloczenia:", ln=True)
        pdf.image('chart.png', x=10, w=180)

    pdf.output("Raport_GoPoznan.pdf")
    
    # Otwórz automatycznie wygenerowany plik na Windowsie
    if sys.platform == "win32":
        os.startfile("Raport_GoPoznan.pdf")

if __name__ == "__main__":
    generate_pdf()

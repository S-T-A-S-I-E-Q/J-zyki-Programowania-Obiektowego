"""
Plik: gtfs_processor.py
Opis: Skrypt pobierający najnowsze dane GTFS (rozkłady jazdy) ze strony ZTM Poznań
      oraz wyciągający najszybsze połączenia pomiędzy stacjami dla autobusów i tramwajów.
      
      UWAGA: NAJWAŻNIEJSZE MIEJSCE 3 - IMPLEMENTACJA POBIERANIA DANYCH Z ZTM.
      Ten plik służy do komunikacji ze źródłami zewnętrznymi oraz zaawansowanej logiki wyszukiwania.
"""
import sys
import os
import zipfile
import urllib.request
import json
import pandas as pd
from collections import defaultdict

GTFS_URL = "https://www.ztm.poznan.pl/pl/dla-deweloperow/getGTFSFile"
ZIP_FILE = "ZTMPoznanGTFS.zip"
GTFS_DIR = "gtfs_data"

def download_and_extract():
    if not os.path.exists(GTFS_DIR):
        if not os.path.exists(ZIP_FILE):
            req = urllib.request.Request(GTFS_URL, headers={
                'Accept': 'application/octet-stream',
                'Content-Type': 'application/x-www-form-urlencoded'
            })
            with urllib.request.urlopen(req) as response, open(ZIP_FILE, 'wb') as out_file:
                out_file.write(response.read())
        
        with zipfile.ZipFile(ZIP_FILE, 'r') as zip_ref:
            zip_ref.extractall(GTFS_DIR)

def get_all_stops():
    try:
        stops_df = pd.read_csv(os.path.join(GTFS_DIR, "stops.txt"))
        # Get unique stop names (some stops have multiple platforms)
        stop_names = sorted(stops_df['stop_name'].dropna().unique().tolist())
        return {"status": "ok", "stops": stop_names}
    except Exception as e:
        return {"status": "error", "message": str(e)}

def time_diff(start_t, end_t):
    def to_min(t):
        h, m, s = map(int, t.split(':'))
        return h * 60 + m
    diff = to_min(end_t) - to_min(start_t)
    return f"{diff} min"

def find_route(start_name, end_name):
    try:
        stops_df = pd.read_csv(os.path.join(GTFS_DIR, "stops.txt"))
        routes_df = pd.read_csv(os.path.join(GTFS_DIR, "routes.txt"))
        trips_df = pd.read_csv(os.path.join(GTFS_DIR, "trips.txt"))
        stop_times_df = pd.read_csv(os.path.join(GTFS_DIR, "stop_times.txt"))

        start_ids = stops_df[stops_df['stop_name'].str.lower() == start_name.lower()]['stop_id'].tolist()
        end_ids = stops_df[stops_df['stop_name'].str.lower() == end_name.lower()]['stop_id'].tolist()

        if not start_ids:
            return {"status": "error", "message": f"Nie znaleziono przystanku: {start_name}"}
        if not end_ids:
            return {"status": "error", "message": f"Nie znaleziono przystanku: {end_name}"}

        start_times = stop_times_df[stop_times_df['stop_id'].isin(start_ids)]
        end_times = stop_times_df[stop_times_df['stop_id'].isin(end_ids)]

        merged = pd.merge(start_times, end_times, on='trip_id', suffixes=('_start', '_end'))
        valid_trips = merged[merged['stop_sequence_start'] < merged['stop_sequence_end']]

        from datetime import datetime
        now = datetime.now()
        # W GTFS czasy po północy to często 24:00:00, 25:00:00 itp.
        current_minutes = now.hour * 60 + now.minute
        if now.hour < 4:
            current_minutes += 24 * 60

        def to_min(t):
            h, m, s = map(int, t.split(':'))
            return h * 60 + m

        # Filtruj wycieczki, by pokazać tylko te z przyszłości
        valid_trips = valid_trips[valid_trips['departure_time_start'].apply(to_min) >= current_minutes]

        if valid_trips.empty:
            return {"status": "error", "message": "Nie znaleziono bezpośredniego połączenia na resztę dnia."}

        valid_trips = pd.merge(valid_trips, trips_df[['trip_id', 'route_id']], on='trip_id')
        valid_trips = pd.merge(valid_trips, routes_df[['route_id', 'route_short_name', 'route_type']], on='route_id')

        # ZTM Poznan: Tram = 0 (czasem 900), Bus = 3 (czasem 700)
        trams_df = valid_trips[valid_trips['route_type'].isin([0, 900])].sort_values('departure_time_start')
        buses_df = valid_trips[valid_trips['route_type'].isin([3, 700])].sort_values('departure_time_start')
        
        # Filtrujemy duplikaty z tego samego czasu (żeby nie wyświetlać kursów z innych dni np. weekendowych)
        trams_df = trams_df.drop_duplicates(subset=['departure_time_start'])
        buses_df = buses_df.drop_duplicates(subset=['departure_time_start'])

        # Ogranicz do 5 unikalnych odjazdów dla każdej kategorii
        trams = []
        for _, row in trams_df.head(15).iterrows():
            if len(trams) >= 5: break
            duration = time_diff(row['departure_time_start'], row['arrival_time_end'])
            trams.append({
                "line": str(row['route_short_name']),
                "departure": str(row['departure_time_start'])[:5], # np 14:30
                "duration": duration
            })

        buses = []
        for _, row in buses_df.head(15).iterrows():
            if len(buses) >= 5: break
            duration = time_diff(row['departure_time_start'], row['arrival_time_end'])
            buses.append({
                "line": str(row['route_short_name']),
                "departure": str(row['departure_time_start'])[:5],
                "duration": duration
            })

        # Zbieranie statystyk odjazdów do wykresu (wszystkie linie z przystanku początkowego)
        all_start_times = stop_times_df[stop_times_df['stop_id'].isin(start_ids)].copy()
        
        def get_hour(t):
            h = int(str(t).split(':')[0])
            return h % 24
            
        all_start_times['hour'] = all_start_times['departure_time'].apply(get_hour)
        # Bierzemy tylko godziny od 0 do 12 (łącznie 13 godzin)
        filtered_times = all_start_times[all_start_times['hour'] < 13]
        
        # Odrzucamy duplikaty na podstawie trip_id (jeśli są)
        filtered_times = filtered_times.drop_duplicates(subset=['trip_id'])
        
        counts = filtered_times['hour'].value_counts().to_dict()
        chart_data = []
        for h in range(13):
            c = counts.get(h, 0)
            if c > 0:
                chart_data.append({"hour": str(h), "count": c})

        return {
            "status": "ok",
            "trams": trams,
            "buses": buses,
            "chart_data": chart_data,
            "start_stop": start_name,
            "end_stop": end_name
        }
    except Exception as e:
        return {"status": "error", "message": str(e)}

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(json.dumps({"status": "error", "message": "Za malo argumentow"}))
        sys.exit(1)

    command = sys.argv[1]

    try:
        download_and_extract()
    except Exception as e:
        print(json.dumps({"status": "error", "message": f"Blad pobierania GTFS: {e}"}))
        sys.exit(1)

    if command == "get_stops":
        print(json.dumps(get_all_stops()))
    elif command == "find_route" and len(sys.argv) == 4:
        start = sys.argv[2]
        end = sys.argv[3]
        print(json.dumps(find_route(start, end)))
    else:
        print(json.dumps({"status": "error", "message": "Nieznana komenda"}))

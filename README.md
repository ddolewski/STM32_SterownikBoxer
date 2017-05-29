# Eclipse ARM GCC 
Monitor uprawy roślin STM32F051


Zmiany:
- dorobić do programatora lampy sodowej opcję godziny początkowej dla danego stanu (18/6, stan początkowy włącz, o godzinie: xx:xx)
- dodać obsługę błędów czujników na LCD (w miejscu wartości pomiarowej dać napis Error)
- uruchomić HSE+PLL na 48MHz
- dorobić CRC do ramek uart/wifi
- uruchomić i nawadnianie (obsługa pompki, dolewania wody, sterowanie z aplikacji, pomiar przepływu wody)
- dorobić funkcję sprawdzania różnicy czasu ostatniego zapisu z aktualnym i kalibracja licznika lampy
- poprawić błąd pomiaru lub wyświetlania natężenia światła (zatrzymuje się na wartości 2)
- dorobić info o nawadnianiu na LCD

WYKONANE:

- dodano strone kalibracji sond na LCD  ------> OK

- delay_us postawiony na timerze (licznik 1us)  ------> OK

- poprawiony algorytm sterowania wentylatorami (PWM, timingi, zakresy, soft start)  ------> OK

- zmiana czasów wykonywania pomiarów  ------> OK 

- poprawki wizualne  ------> OK

- poprawiona dwukrotna początkowa synchronizacja czasu ------> OK

- poprawki wyświetlania strony pH ------> OK

- pierwsza synchronizacja czasu po włączeniu urządzenia po 10s potem co godzinę ------> OK

- dodać manualne sterowanie wentylatorami z C# ------> OK

- zmienić sposób pomiaru wilgotności gleby (oksydacja sondy -> podawać napięcie tylko na czas pomiaru a potem 0V) ------> OK

- funkcja ustawiania czasu przez atnel wifi (wbudowane ntp?) ------> OK

- przerobić usart tx na przerwaniach i fifo + poprawione ramki ------> OK 

- dodanie przywracania ustawień domyślnych we flash ------> OK

- zmienić zapis liczników oświetlenia z pamięci RAM PCF8583 do pamięci FLASH ------> OK

- zmienić RTC z PCF8583 na PCF8563 ------> OK

- dorobić czas letni/zimowy dla Polski (biblioteka time.h ze sterownika bojlera) ------> OK

- zmniejszono częstotliwość PWM'a sterującego wentylatorami (z 12kHz na 30Hz) w 
  celu zmniejszenia szpilek napięciowych i zakłóceń (spadek napięcia), poprawiono automatykę obrotów ------> OK
  
- zrobić nowe ramki transmisji (osobne ramki od każdej komendy, ramki w ASCII) ------> OK

- sprawdzanie wilgotności gleby (ADC) ------> OK

- ustawienia flash w jednej strukturze a w niej struktury danych funkcji + flagi poprawności ------> OK

- dobrać elementy bufora w zależności od pomiaru (mapowanie kanałów ADC do tablicy) ------> OK

- napis kalibracja na LCD podczas kalibracji sondy ------> OK

- dodano zapis liczników lampy do pamięci RAM PCF8583 (adresy od 0x08 do 0x0C) ------> OK

- sprawdzić i poprawić dlaczego współczynniki równania nie zawsze są 0 przed kalibracją ? ------> OK

- napis kalibracja na LCD (kasowanie) (wysylanie potwierdzenia z procka do C# ze kalibracja wykonana) ------> OK

- dorobiono kalkulację CRC dla pomiarów z DS18B20 + odrzucanie błędnych ramek ------> OK

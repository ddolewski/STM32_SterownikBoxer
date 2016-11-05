# Eclipse ARM GCC 
Monitor uprawy roślin STM32F051


Zmiany:

- zrobić sprzętowe opóźnienie delay_us (1us) -> Timer?
- zrobić algorytm wyszukiwania 1Wire
- uruchomić HSE+PLL na 48MHz
- dorobić info o nawadnianiu na LCD





- zmniejszono częstotliwość PWM'a sterującego wentylatorami (z 12kHz na 30Hz) w 
  celu zmniejszenia szpilek napięciowych i zakłóceń (spadek napięcia), poprawiono automatykę obrotów ------> OK
  
- zrobić nowe ramki transmisji (osobne ramki od każdej komendy, ramki w ASCII) ------> OK

- sprawdzanie wilgotności gleby (ADC) ------> OK

- ustawienia flash w jednej strukturze a w niej struktury danych funkcji + flagi poprawności ------> OK

- dobrać elementy bufora w zależności od pomiaru ([1]-> PA4 czyli coś tam, itd.) ------> OK

- napis kalibracja na LCD podczas kalibracji sondy ------> OK

- dodano zapis liczników lampy do pamięci RAM PCF8583 (adresy od 0x08 do 0x0C) ------> OK

- sprawdzić i poprawić dlaczego współczynniki równania nie zawsze są 0 przed kalibracją ? ------> OK

- napis kalibracja na LCD (kasowanie) (wysylanie potwierdzenia z procka do C# ze kalibracja wykonana) ---> PRZETESTOWAĆ!! ------> OK

- dorobiono kalkulację CRC dla pomiarów z DS18B20 + odrzucanie błędnych ramek ------> OK


#include <DHT.h>
#include <SPI.h>
#include <SD.h>
#include <iarduino_RTC.h>    
#include <Wire.h>

#define RAINPIN A1 //пин датчика
#define RELAY_W 2 // реле полива теплицы 
#define RELAY_BARREL 3 // реле пополнения бочки 
#define DHTPIN 4 //датчик температуры и влажности 
#define MOSFET 5 // транзистор на датчик влажности почвы
#define GERKON 9

#define DHTTYPE DHT11  


iarduino_RTC watch(RTC_DS1302,8,6,7);
File dataFile;
File dayFile;
DHT dht(DHTPIN, DHTTYPE);


const int chipSelect = 10;

int watrDay = 3;  // переменная частоты полива в днях Э каждые watrDay дня Э

int wDayNow = 0; // день до полива // фактический счетсчик дней 
bool flag1 = true, flag2 = true, flag3 = true, flag4 = true, flag5 = true, flag6 = true;
bool test = true;
bool timer1 = false;  // таймер полива теплицы 
bool barrel = false;  // если был полив теплицы переменная тру, нужно пополнить бочку. 
bool timer2 = false;  // таймер сигнализирующий о том что бочка пополняется 
bool gerkon = false;  // переменная датчика уровня воды в бочке
bool powerFlag = true;

bool WATERING = false; // переменная сигнализирующая, что в фактическое время полива, питание на МК было снято и он небыл осуществлен
bool WATERINGtimer = false; // переменная для таймера по WATERING 
int WATERINGmin = 0, WATERINGhrs = 0, WATERINGsec = 0; // в переменную фиксируется текщее время срабатывания флага WATERING + Продолжительность полива с последующей корректировкой вых за границы.

int bMin = 0, bHrs = 0, bSec = 0; // переменные для корректировки времени таймера наполнения бочки 
int timeoutM = 13, timeoutH = 0, timeoutS = 0; // переменная в минутах. таймаут пополнение бочки. на случай выхода из строя геркона. МОЖНО КОРРЕКТИРОВАТЬ ФАКТИЧЕСКОЕ ВРЕМЯ 10М21С

int wHrs = 18, wMin = 0, wSec = 0; // тут записано время полива МОЖНО КОРРЕКТИРОВАТЬ 
int WDHrs = 0, WDMin = 30, WDSec = 4;  //продолжительность полива МОЖНО КОРРЕКТИРОВАТЬ

int WEHrs = 0, WEMin = 0, WESec = 0; //конечное время полива, высчитывается в блоке // текущее время + продолжительность полива 
int i = 0;
 
uint32_t StartB = 0, StartW = 0; //переменные для храннения времени начала полива или пополнения в МС на случай отказа RTC.  
uint32_t FinishB = 4000 , FinishW = 4000; // переменные время в МС по истечению которого отключается насаос МОЖНО КОРРЕКТИРОВАТЬ. // 60 000 = 1 минуте // 300 000 = 5 минутам // 600 000 = 10мин

void setup() {
  watch.begin();
 // watch.settime(40,53,13,11,7,21,7);
  Serial.begin(9600);
  SD.begin(chipSelect);
  dht.begin();
   
   pinMode(GERKON, INPUT); // вывод геркона как вход 
  digitalWrite(GERKON, HIGH); // активируем внутрений подтягивающий резистор вывода 
    
    pinMode(RELAY_W, OUTPUT); // Объявляем пин реле как выход
  digitalWrite(RELAY_W, LOW); // Выключаем реле - посылаем LOW сигнал

    pinMode(RELAY_BARREL, OUTPUT); // Объявляем пин реле как выход
  digitalWrite(RELAY_BARREL, LOW);  //Выключаем реле - посылаем LOW сигнал
    
    pinMode(MOSFET, OUTPUT); // Объявляем пин реле как выход
  digitalWrite(MOSFET, LOW); // Выключаем транзистор низкий сигнал  


//**************************проверка на отключение питания и восстановление текущего дня полива.

 dayFile = SD.open("daylog.txt");
 while (dayFile.available()) {
      dayFile.read(); 
      i++;
    }
    // close the file:
    dayFile.close();
   
    

  i = i / 3;   // число = 1 байт , /n = 2 делим на 3 и узнаем фактическое количество чисел 
  i = i - 1; // поправка 
  
  for(int j = 0; j <= i; j++){

 if(wDayNow > watrDay){
  wDayNow = 0;
  }
  wDayNow++;
  }
wDayNow = wDayNow - 1;
watch.gettime();
int hrs = watch.Hours;
int m = watch.minutes;
if (wDayNow >= watrDay && hrs >= wHrs && m >= wMin ){
  WATERING = true; 
  }
  Serial.print(wDayNow);
  if (wDayNow == -1){
    wDayNow = 0;
    }
}


void loop() {
watch.gettime();
int hrs = watch.Hours;
int m = watch.minutes;
int s = watch.seconds;
int i = 0;
 

 
if(hrs == 0 && m == 0 && s == 0 && flag2 == true){ // счетчик нового дня 
  flag2 = false; 
  wDayNow++;
  File dayFile = SD.open("daylog.txt", FILE_WRITE);
  dayFile.println(wDayNow);
  dayFile.close();
  
  
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  dataFile.println();
  dataFile.println();
  dataFile.print(watch.gettime("d-m-Y, H:i:s, D"));
      dataFile.close();
  
  }
  
if (hrs >= 8 && hrs <= 20 && hrs % 1 == 0 && m == 0 && flag3 == true){ // измерение в дневное время 1 раз в час 
   if(s >= 1){
   
    int t =  dht.readTemperature();
     
     if(s >= 2){
 int h = dht.readHumidity();
 flag3 = false;

 File dataFile = SD.open("datalog.txt", FILE_WRITE);
     
    
    dataFile.println();
    dataFile.print(hrs);   dataFile.print(":");
    dataFile.print(m);     dataFile.print(" ta ");
    dataFile.print(t);     dataFile.print("C ");    dataFile.print("th ");
    dataFile.print(h);     dataFile.print("% ");

    dataFile.close();
     }
   }
  }  
if (hrs < 8 && hrs % 2 == 0 && m == 0 &&  flag3 == true || hrs > 20 && hrs % 2 == 0 && m == 0 && flag3 == true ){ // измерение в ночное время 1 раз в 2 часа
  
      if(hrs != 0){
      if(s >= 1){
   
    int t =  dht.readTemperature();
     
     if(s >= 2){
      
 int h = dht.readHumidity();
 flag3 = false;

 File dataFile = SD.open("datalog.txt", FILE_WRITE);
     
    
    dataFile.println();
    dataFile.print(hrs);   dataFile.print(":");
    dataFile.print(m);     dataFile.print(" ta ");
    dataFile.print(t);     dataFile.print("C ");    dataFile.print("th ");
    dataFile.print(h);     dataFile.print("% ");

    dataFile.close();
     }
   }
 }
}



///////////////////////////////////////////////////////////////////////полив теплицы///////////////////////////////////////////////////////////////////////
 
 
 if (wDayNow == watrDay && hrs == wHrs && m == wMin && s == wSec && flag4 == true || WATERING == true && flag4 == true){ // условия для начала полива теплицы 
   
    if(WATERING == true){
      WATERING = false;
      flag4 = false; 
      WATERINGtimer = true;
      WATERINGhrs = hrs + WDHrs;//WATERING фиксация времени начала полива и прибавление времени полива с учетом минут часов секунд 
      WATERINGmin = m + WDMin;
      WATERINGsec = s + WDSec;
      
      for(int i = 0; i <= 2; i++){
      if (WATERINGmin > 60){   //поправка при выходе минут за границы 60мин
          WATERINGmin = WATERINGmin - 60;
          WATERINGhrs++;
          }
          if(WATERINGmin == 60){
            WATERINGmin = 0;
            WATERINGhrs++;
            }   
              if(WATERINGhrs > 24){  // поправка при выходи часов за границы 24ч 
                WATERINGhrs = WATERINGhrs - 24;  // день прибавится сам блок 00:00 
               }
                  if(WATERINGhrs == 24){
                    WATERINGhrs = 0;
                    } 
                     if(WATERINGsec > 60){  //поправка при выходе секунд за границы 60сек
                      WATERINGsec = WATERINGsec - 60;
                      WATERINGmin++;
                      }
                        if(WATERINGsec == 60){
                          WATERINGsec = 0;
                           WATERINGmin++;
                          }
                                  }
                                } else {  
                             timer1 = true; // запуск таймера 
                                WEHrs = hrs + WDHrs; //фиксация времени начала полива и прибавление времени полива с учетом минут часов секунд 
                                WEMin = m + WDMin;
                                WESec = s + WDSec;
                            
                                      for(int i = 0; i <= 2; i++){
                                  if (WEMin > 60){   //поправка при выходе минут за границы 60мин
                                      WEMin = WEMin - 60;
                                      WEHrs++;
                                      }
                                      if(WEMin == 60){
                                        WEMin = 0;
                                        WEHrs++;
                                        }   
                                          if(WEHrs > 24){  // поправка при выходи часов за границы 24ч 
                                            WEHrs = WEHrs - 24;  // день прибавится сам блок 00:00 
                                           }
                                              if(WEHrs == 24){
                                                WEHrs = 0;
                                                } 
                                                 if(WESec > 60){  //поправка при выходе секунд за границы 60сек
                                                  WESec = WESec - 60;
                                                  WEMin++;
                                                  }
                                                    if(WESec == 60){
                                                      WESec = 0;
                                                       WEMin++;
                                                      }
                                  }
                                }
   StartW = millis();  // старт таймера для подстраховки RTC
   wDayNow = 0;    
   flag4 = false;

   File dayFile = SD.open("daylog.txt", FILE_WRITE);  // запись нулевого дня для калибровки в случе отклоючения питания 
    dayFile.println(wDayNow);
   dayFile.close();
    
  digitalWrite(RELAY_W, HIGH);  // watering on 
 
   File dataFile = SD.open("datalog.txt", FILE_WRITE);
     
      dataFile.print(" w+ ");
        dataFile.print(hrs);
        dataFile.print(":");
        dataFile.print(m);
    
      dataFile.close();
   
}


if (timer1 == true && hrs == WEHrs && m == WEMin && s == WESec || WATERINGtimer == true && WATERINGhrs == hrs && WATERINGmin == m || millis() - StartW >= FinishW && timer1 == true || millis() - StartW >= FinishW && WATERINGtimer == true){ //условия для окончания полива теплицы 
timer1 = false; 
barrel = true; 
WATERINGtimer == false;
wDayNow = 0; 
  digitalWrite(RELAY_W, LOW); //watering off

 File dataFile = SD.open("datalog.txt", FILE_WRITE);
      dataFile.print(" w- ");
        dataFile.print(hrs);
        dataFile.print(":");
        dataFile.print(m);

      dataFile.close();
             // отключить полив  
}










///////////////////////////////////////////////////////////////////////пополнение бочки///////////////////////////////////////////////////////////////////////



if (barrel == true){ // полив теплицы прошел! пора пополнить бочку. 
  barrel = false; 
  timer2 = true;
  StartB = millis();  // старт таймера для подстраховки RTC
  bHrs = hrs + timeoutH;
  bMin = m + timeoutM;
  bSec = s + timeoutS; // время с учетом таймера
  
        for(int i = 0; i <= 2; i++){
      if (bMin > 60){   //поправка при выходе минут за границы 60мин
          bMin = bMin - 60;
          bHrs++;
          }
          if(bMin == 60){
            bMin = 0;
            bHrs++;
            }   
              if(bHrs > 24){  // поправка при выходи часов за границы 24ч 
                bHrs = bHrs - 24;  // день прибавится сам блок 00:00 
               }
                  if(bHrs == 24){
                    bHrs = 0;
                    } 
                     if(bSec > 60){  //поправка при выходе секунд за границы 60сек
                      bSec = bSec - 60;
                      bMin++;
                      }
                        if(bSec == 60){
                          bSec = 0;
                           bMin++;
                          }
      }

   digitalWrite(RELAY_BARREL, HIGH); // включение колодесного насоса на пополнение бочки 
    File dataFile = SD.open("datalog.txt", FILE_WRITE);
      dataFile.print(" b+ ");
        dataFile.print(hrs);
        dataFile.print(":");
        dataFile.print(m);

      dataFile.close(); 
           
  }




 if (timer2 == true && gerkon == false){  // логический блок для чтения данных с датчика уровня воды в бочке
  // считывай данные с геркона и опрееляй когда вода заполнится, как только геркон сработал gerkon = true;
  int g = digitalRead(GERKON); // если 0 вода наполнилась, 1 бочка еще не наполнена 
    if(g == 0){
      gerkon = true;
      }
   
  }
 if (timer2 == true && gerkon == true || timer2 == true && hrs == bHrs && m == bMin && s == bSec || millis() - StartB >= FinishB && timer2 == true){  // условия прекращения пополнения бочки датчик или таймаут 
// так же добавить отключение по таймауту. 
      
    digitalWrite(RELAY_BARREL, LOW); // выключение колодесного насоса
       File dataFile = SD.open("datalog.txt", FILE_WRITE);
      dataFile.print(" b- ");
        dataFile.print(hrs);
        dataFile.print(":");
        dataFile.print(m);
 
      dataFile.close(); 
      timer2 = false;
}
///////////////////////////////////////////////////////////////////////*считывание почвы*///////////////////////////////////////////////////////////////////////

if (hrs == 8 && m == 0 && flag5 == true || hrs == 20 && m == 0 && flag5 == true){ // считывание почвы 
     digitalWrite(MOSFET, HIGH);
      
      if (s >= 1){
        int sensor = analogRead(RAINPIN);
          
          if (s >= 2){
          flag5 = false;
         File dataFile = SD.open("datalog.txt", FILE_WRITE);
        
         dataFile.print("he% ");
         dataFile.print(sensor);
         dataFile.close();
         digitalWrite(MOSFET, LOW);
       }
    }
  }
///////////////////////////////////////////////////////////////////////*сброс флагов инициализация флешки*///////////////////////////////////////////////////////////////////////
  if(m == 10 && s == 0 || m == 40 && s == 0) {  // сброс флагов раз в пол часа
    flag1 = true; flag2 = true; flag3 = true; flag4 = true; flag5 = true; flag6 = true;
    }
     
     if(m == 9 && s == 0 && flag6 == true|| m == 39 && s == 0 && flag6 == true){  // инициализация флешки 2 раза в час
        SD.begin(chipSelect);
      flag6 = false; 
     
      //if(day == 0){             // БЛОК ОПРЕДЕЛЕНИЯ ТЕКУЩЕГО ДНЯ В СЛУЧЕ ОТКЛЮЧЕНИЯ ПИТАНИЯ (ПРОТЕСТИРОВАТЬ)!!!
        
       //  File dayFile = SD.open("daylog.txt", FILE_WRITE);
      // unsigned long PrePos = dayFile.position() - 1; 
      //  dayFile.seek(PrePos);
      //  int vaule = dayFile.read();
       //   if(vaule != watrDay){
       //     day == vaule;
           // }
        }
      }

    //задачи! Отработать и подкоректировать код парсинга daylog! 
    //задачи! Проверить таймаут на основе millis!
    //задачи! Протестировать код с новыми переменными полива! 
    //Задачи! Протестировать систему с насосами начиная с сегоднешней ночи 07.07.2021
    //Задачи! Провести Оптимизацию кода путем сокрощению СТРОК для освобождении памяти!!!!!!!

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

int g = 1;
const int chipSelect = 10;
int day = 0;
int watrDay = 0;  // переменная частоты полива в днях Э каждые watrDay дня Э
int timeout = 5; // переменная в минутах. таймаут пополнение бочки. на случай выхода из строя геркона.
bool flag1 = true, flag2 = true, flag3 = true, flag4 = true, flag5 = true, flag6 = true;
bool test = true;
bool timer1 = false;  // таймер полива теплицы 
bool barrel = false;  // если был полив теплицы переменная тру, нужно пополнить бочку. 
bool timer2 = false;  // таймер сигнализирующий о том что бочка пополняется 
bool gerkon = false;  // переменная датчика уровня воды в бочке
int bMin = 0, bHrs = 0; // переменные для корректировки времени таймеранаполнения бочки 
bool schet = true;

void setup() {
  watch.begin();
  watch.settime(0 ,0,0,16,6,21,3);
  Serial.begin(9600);
  SD.begin(chipSelect);
  dht.begin();
   
   pinMode(GERKON, INPUT); // вывод геркона как вход 
  digitalWrite(GERKON, HIGH); // активируем внутрений подтягивающий резистор вывода 
    
    pinMode(RELAY_W, OUTPUT); // Объявляем пин реле как выход
  digitalWrite(RELAY_W, LOW); // Выключаем реле - посылаем LOW сигнал

    pinMode(RELAY_BARREL, OUTPUT); // Объявляем пин реле как выход
  digitalWrite(RELAY_BARREL, HIGH); // Выключаем реле - посылаем высокий сигнал
    
    pinMode(MOSFET, OUTPUT); // Объявляем пин реле как выход
  digitalWrite(MOSFET, LOW); // Выключаем транзистор низкий сигнал  
}


void loop() {
watch.gettime();
int hrs = watch.Hours;
int m = watch.minutes;
int s = watch.seconds;
int i = 0;
 

 


  /////////////////****************************************TEST************************************////////////////////////


 if (gerkon == false){  // логический блок для чтения данных с датчика уровня воды в бочке
  // считывай данные с геркона и опрееляй когда вода заполнится, как только геркон сработал gerkon = true;
  bool go = true;
  while(!go){ 
  int g = digitalRead(GERKON); // если 0 вода наполнилась, 1 бочка еще не наполнена 
    if(g == 0){ // если геркон нажат
      gerkon = true;
      while(g != 1){ //ждем пока геркон опустится 
      g = digitalRead(GERKON);
        if (g == 0){ // как только опустился заканчиваем цикл идем в блок остановки полива
          go = false;
          }
        }
      }
    }
  }

  
 if (gerkon == true){   
     
    digitalWrite(RELAY_BARREL, HIGH); // включение колодесного насоса тестим
       File dataFile = SD.open("datalog.txt", FILE_WRITE);
      dataFile.print(" Pomp in the barrel ON ");
        dataFile.print(hrs);
        dataFile.print(":");
        dataFile.print(m);
        dataFile.println();
      dataFile.close();
   
   while (g != 0){
    g = digitalRead(GERKON);
    }
   
    if(g == 0){
      gerkon = true;
      
      while(g != 1){
      g = digitalRead(GERKON);
      }
      }
}

}

  /////////////////****************************************TEST************************************////////////////////////



 
 

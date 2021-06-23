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

#define DHTTYPE DHT11 /123


iarduino_RTC watch(RTC_DS1302,8,6,7);
File dataFile;
File dayFile;
DHT dht(DHTPIN, DHTTYPE);


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

void setup() {
  watch.begin();
  watch.settime(40,59,17,16,6,21,3);
  Serial.begin(9600);
  SD.begin(chipSelect);
  dht.begin();
   
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
 

 
if(hrs == 0 && m == 0 && s == 0 && flag2 == true){ // счетчик нового дня 
  flag2 = false; 
  day++;
  
  File dayFile = SD.open("daylog.txt", FILE_WRITE);
  dayFile.println(day);
  dayFile.close();
  
  timer1 = true;
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
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
 
 
 if (day == watrDay && hrs == 18 && m == 0 && s == 0 && flag4 == true){ // время полива 
    
 timer1 = true; // запуск таймера 
 flag4 = false;
  
    day = 0;
   File dayFile = SD.open("daylog.txt", FILE_WRITE);  // запись нулевого дня для калибровки в случе отклоючения питания 
    dayFile.println(day);
   dayFile.close();
   
  digitalWrite(RELAY_W, HIGH);  // watering on 
 
   File dataFile = SD.open("datalog.txt", FILE_WRITE);
     
      dataFile.print(" watering ON ");
        dataFile.print(hrs);
        dataFile.print(":");
        dataFile.print(m);
        dataFile.println();
      dataFile.close();
   
}


if (timer1 == true && hrs == 18 && m == 10 && s == 0){ //проверка таймера на полив теплицы 
timer1 = false; 
barrel = true; 


  digitalWrite(RELAY_W, LOW); //watering off

 File dataFile = SD.open("datalog.txt", FILE_WRITE);
      dataFile.print(" watering OFF ");
        dataFile.print(hrs);
        dataFile.print(":");
        dataFile.print(m);
        dataFile.println();
      dataFile.close();
             // отключить полив  
}

if (barrel == true){ // полив теплицы прошел! пора пополнить бочку. 
  barrel = false; 
  timer2 = true;
  bMin = m;
  bHrs = hrs;
  bMin = bMin + timeout; // время с учетом таймера
  if (bMin > 60){   //поправка при выходе минут за границы 60мин
    bMin = bMin - 60;
    bHrs++;
  if(bHrs > 24){  // поправка при выходи часов за границы 24ч 
    bHrs = bHrs - 24; 
    } 
   }

   digitalWrite(RELAY_BARREL, LOW); // включение колодесного насоса на пополнение бочки 
    File dataFile = SD.open("datalog.txt", FILE_WRITE);
      dataFile.print(" Pomp in the barrel ON ");
        dataFile.print(hrs);
        dataFile.print(":");
        dataFile.print(m);
        dataFile.println();
      dataFile.close(); 
           // включить насос колодца
  }

 if (timer2 == true && gerkon == false){  // логический блок для чтения данных с датчика уровня воды в бочке
  // считывай данные с геркона и опрееляй когда вода заполнится, как только геркон сработал gerkon = true;
  
  }
 if (timer2 == true && gerkon == true || timer2 == true && hrs == bHrs && m == bMin){  // условия прекращения пополнения бочки датчик или таймаут 
// написать условия для геркона поместить в булевую переменную bool gerkon и отключать полив если он сработал 
// так же добавить отключение по таймауту. 
      
    digitalWrite(RELAY_BARREL, HIGH); // выключение колодесного насоса
       File dataFile = SD.open("datalog.txt", FILE_WRITE);
      dataFile.print(" Pomp in the barrel OFF ");
        dataFile.print(hrs);
        dataFile.print(":");
        dataFile.print(m);
        dataFile.println();
      dataFile.close(); 
      timer2 = false;
}


if (hrs == 18 && m == 16 && flag5 == true || hrs == 20 && m == 0 && flag5 == true){ // считывание почвы 
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
    

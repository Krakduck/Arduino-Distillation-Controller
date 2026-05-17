#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <Keypad.h>

#define TFT_CS   -1
#define TFT_DC   9
#define TFT_RST  8

Adafruit_ILI9341 display(TFT_CS, TFT_DC, TFT_RST);

const byte ROWS = 4; //число строк у нашей клавиатуры
const byte COLS = 4; //число столбцов у нашей клавиатуры
char hexaKeys[ROWS][COLS] = {// здесь мы располагаем названия наших клавиш, как на клавиатуре,для удобства пользования
  {'D', '#', '0', '*'},
  {'C', '9', '8', '7'},
  {'B', '6', '5', '4'},
  {'A', '3', '2', '1'}
};

byte colPins[ROWS] = {22,23,25,27}; //к каким выводам подключаем управление строками    35, 33, 31, 29    
byte rowPins[COLS] = {29,31,33,35}; //к каким выводам подключаем управление столбцами   27, 25, 23, 22

//передаем все эти данные библиотеке:
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

bool sospechka = false;
bool otchet = false;
float glavtemp = 10000;
float glavtempsravnenie =10000;
bool ledsos = false;
bool durationactive = false;
unsigned long time;
unsigned long ledstart = 0;  // Добавляем инициализацию
bool allowNewCycle = true; // Разрешение на новый цикл
int maxzmeitemp = 200;
int vvod;
int maxgradusnik = 200;
bool nastroiactive = false;
int position = 0;
float celsiusgradusnik;
float celsiuscube;
float celsiusverh;
int maxcube =200;
float lastCelsius = -1000, lastCelsius1 = -1000, lastCelsius2 = -1000, lastCelsius3 = -1000;
bool lastSospechka = false;
int lastPosition = -1;
int procentotbor = 200; 
bool voda = true;
float deltat=0;
int timeotbora =0;
int timestabilization = 0;
unsigned long timerotbortela =0;
unsigned long startTimeotboratela = 0;
unsigned long timerotborgol = 0;
unsigned long startTimeotboragol = 0;
bool vodaVkluchena = false; // глобальная переменная-флаг
int countpechka = 0;
bool flagotkl = false;

void setup() {
  display.begin();
  display.setRotation(0);  // Ориентация экрана
  display.fillScreen(ILI9341_BLACK);
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(45, INPUT);
  pinMode(3,INPUT);
  Serial.begin(9600);
  pinMode(7,OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(5,OUTPUT);
}



void otrisovkavoda() {
    // Если вода ВКЛЮЧЕНА и температура превысила максимум - ВЫКЛЮЧАЕМ
    if (vodaVkluchena && celsiusverh > maxzmeitemp || !voda) {
        // Выключаем воду
        display.drawLine(125, 30, 111, 50, ILI9341_WHITE);//змеевик
        display.drawLine(125, 35, 115, 50, ILI9341_WHITE);
        display.drawLine(111, 50, 125, 75, ILI9341_WHITE);
        display.drawLine(115, 50, 125, 70, ILI9341_WHITE);
        display.drawRect(125, 30, 10, 5, ILI9341_WHITE);
        display.drawRect(125, 70, 10, 5, ILI9341_WHITE);
        digitalWrite(6, HIGH);
        
        vodaVkluchena = false; // сбрасываем флаг
    }
    // Если вода ВЫКЛЮЧЕНА и условия для включения выполнены - ВКЛЮЧАЕМ
    else if (!vodaVkluchena && celsiuscube >= maxcube && celsiusverh < 35 && voda) {
        // Включаем воду
        display.drawLine(125, 30, 111, 50, ILI9341_BLUE);//змеевик
        display.drawLine(125, 35, 115, 50, ILI9341_BLUE);
        display.drawLine(111, 50, 125, 75, ILI9341_BLUE);
        display.drawLine(115, 50, 125, 70, ILI9341_BLUE);
        display.drawRect(125, 30, 10, 5, ILI9341_BLUE);
        display.drawRect(125, 70, 10, 5, ILI9341_BLUE);
        digitalWrite(6, LOW);
        
        vodaVkluchena = true; // устанавливаем флаг
    }
    // Если вода УЖЕ ВКЛЮЧЕНА - просто поддерживаем состояние
    else if (vodaVkluchena) {
        display.drawLine(125, 30, 111, 50, ILI9341_BLUE);//змеевик
        display.drawLine(125, 35, 115, 50, ILI9341_BLUE);
        display.drawLine(111, 50, 125, 75, ILI9341_BLUE);
        display.drawLine(115, 50, 125, 70, ILI9341_BLUE);
        display.drawRect(125, 30, 10, 5, ILI9341_BLUE);
        display.drawRect(125, 70, 10, 5, ILI9341_BLUE);
        digitalWrite(6, LOW);
    }
    // Во всех остальных случаях - вода выключена
    else {
        display.drawLine(125, 30, 111, 50, ILI9341_WHITE);//змеевик
        display.drawLine(125, 35, 115, 50, ILI9341_WHITE);
        display.drawLine(111, 50, 125, 75, ILI9341_WHITE);
        display.drawLine(115, 50, 125, 70, ILI9341_WHITE);
        display.drawRect(125, 30, 10, 5, ILI9341_WHITE);
        display.drawRect(125, 70, 10, 5, ILI9341_WHITE);
        digitalWrite(6, HIGH);
    }
}
void gradusnik() {
  display.fillRect(0, 121, 70, 15, ILI9341_BLACK);
  display.setCursor(0, 121);
  display.setTextColor(ILI9341_GREEN);
  display.setTextSize(2);
  display.println(celsiusgradusnik);
}

void cube() {
  display.fillRect(90, 278, 69, 15, ILI9341_BLACK);
  display.setCursor(90, 278);
  display.setTextSize(2);
  display.println(celsiuscube);
  if (celsiuscube > 75) {
    display.setTextColor(ILI9341_RED);
  }
  else {
    display.setTextColor(ILI9341_GREEN);
  }
}

void vodaverh() {
  display.fillRect(155, 30, 70, 15, ILI9341_BLACK);
  display.setCursor(155, 30);
  display.setTextColor(ILI9341_GREEN);
  display.setTextSize(2);
  display.println(celsiusverh);
}

void miganie() {
  // Логика активации мигания
  if (celsiusgradusnik >= maxgradusnik && !otchet && !durationactive && allowNewCycle) { //70 vmesto 35
    otchet = true;
    time = millis();
    Serial.println("Начало ожидания 20м"); 
    display.fillRect(0,47,91,30,ILI9341_BLACK);
    display.setCursor(0,47);
    display.setTextColor(ILI9341_GREEN);
    display.setTextSize(2);
    display.print("stab:");
    display.println((millis() - time)/60000);//////////////////////////
  }
  // Активация через 500мс
  if (otchet && (millis() - time >= timestabilization*60000) && !durationactive) {  //таймер на 20 минут (1200000)
    otchet = false;
    durationactive = true;
    ledsos = true;
    ledstart = millis();
    digitalWrite(5, LOW);
    allowNewCycle = false;
    Serial.println("Начало мигания");  
    //display.fillRect(0,62,91,30,ILI9341_BLACK);
    //display.setCursor(0,62);
    //display.setTextColor(ILI9341_GREEN);
    //display.setTextSize(2);
    //display.print("otb%:");
    //display.println(procentotbor/30);////////////////////////////////////////////////
  }
  // Логика мигания
  if (durationactive) {
    if (millis() - ledstart >= 172800000) { // 2 дня
        durationactive = false;
        digitalWrite(5, HIGH);
        Serial.println("Конец мигания");
    } 
    else {
        static bool processPaused = false;
        static unsigned long pauseStartTime = 0;
        static unsigned long tempOkTime = 0;
        static bool waitingForStableTemp = false;
        
        if (processPaused) {
            // Процесс на паузе - мигаем с фиксированной частотой
            unsigned long cycleTime = (millis() - pauseStartTime) % 1000;
            digitalWrite(5, (cycleTime < 0) ? LOW : HIGH);
            
            // Проверяем 20-минутный таймер паузы
            if (millis() - pauseStartTime >= 1200000) { // 20 минут
                flagotkl = true;
                processPaused = false;
                waitingForStableTemp = false;
                Serial.println("Пауза завершена по времени (20 минут)");
            }
            
            // Проверяем восстановление температуры
            if (celsiusgradusnik <= glavtemp) {
                if (!waitingForStableTemp) {
                    // Первый раз температура упала - запускаем 5-секундный таймер
                    tempOkTime = millis();
                    waitingForStableTemp = true;
                    Serial.println("Температура восстановлена, ждем 5 секунд...");
                }
                else if (millis() - tempOkTime >= 5000) { // 5 секунд стабильной температуры
                    // Температура стабильно низкая 5 секунд - снимаем паузу
                    processPaused = false;
                    waitingForStableTemp = false;
                    Serial.println("Пауза снята - температура стабилизировалась");
                }
            }
            else {
                // Температура снова поднялась - сбрасываем 5-секундный таймер
                waitingForStableTemp = false;
            }
        }
        else if (millis() - ledstart <= (timeotbora * 60000)) { // время 95% отбора
            procentotbor = 150;//250
            unsigned long cycleTime = (millis() - ledstart) % 3000;//5000
            digitalWrite(5, (cycleTime < procentotbor) ? LOW : HIGH);
            Serial.print("отбор:");
            Serial.print(procentotbor);
            Serial.println("/5000");
            vivodpokazanigolov();
            glavtemp = celsiusgradusnik + deltat;
            procentotbor = 1500;//2500
        }
        else {
            vivodpokazanitela();
            unsigned long cycleTime = (millis() - ledstart) % 3000;//5000
            digitalWrite(5, (cycleTime < procentotbor) ? LOW : HIGH);
            Serial.print("отбор:");
            Serial.print(procentotbor);
            Serial.println("/3000");//5000
            
            if (celsiusgradusnik > glavtemp && !processPaused) {
                // Активируем паузу процесса
                pauseStartTime = millis();
                procentotbor = procentotbor - 150;//250
                processPaused = true;
                waitingForStableTemp = false;
                Serial.println("ПАУЗА: температура превышена!");
                Serial.println(glavtemp);
            }
        }
    }
}
}

void vivodpokazanigolov(){
  display.fillRect(0,31,91,30,ILI9341_BLACK);
  if(!nastroiactive){
    if (startTimeotboragol == 0) {
    startTimeotboragol = millis();
    }
    timerotborgol = millis() - startTimeotboragol;
    display.setCursor(0,31);
    display.setTextColor(ILI9341_GREEN);
    display.setTextSize(2);
    display.print("goL ");
    display.print(timerotborgol/3600000);
    display.print(":");
    display.println(timerotborgol/60000);
    display.fillRect(0,62,91,30,ILI9341_BLACK);
    display.setCursor(0,62);
    display.setTextColor(ILI9341_GREEN);
    display.setTextSize(2);
    display.print("otb%:");
    display.println(procentotbor/30);
  }
}

void vivodpokazanitela(){
  display.fillRect(0,0,91,30,ILI9341_BLACK);
  if(!nastroiactive){
    if (startTimeotboratela == 0) {
    startTimeotboratela = millis();
    }
    timerotbortela = millis() - startTimeotboratela;
    display.setCursor(0,0);
    display.setTextColor(ILI9341_GREEN);
    display.setTextSize(2);
    display.println(glavtemp);
    display.print("teL ");
    display.print(timerotbortela/3600000); //3600000
    display.print(":");
    display.println(timerotbortela/60000);//60000
    display.fillRect(0,62,91,30,ILI9341_BLACK);
    display.setCursor(0,62);
    display.setTextColor(ILI9341_GREEN);
    display.setTextSize(2);
    display.print("otb%:");
    display.println(procentotbor/30);
  }
}
  
void pechka() {
    if (sospechka) {
      display.drawRect(84, 300, 70, 20, ILI9341_RED);
      digitalWrite(7,LOW);
    } else {
      display.drawRect(84, 300, 70, 20, ILI9341_WHITE);
      digitalWrite(7,HIGH);
    }
}

void vvodogranichenia(char key) {
  if (key >= '0' && key <= '9') {
    vvod = vvod * 10 + (key - '0');
  }
  else if (key == '*') { // Сброс ввода
    vvod = 0;
  }
  else if (key == '#') {
    if (position == 0) {
      maxzmeitemp = vvod;
    }                   // Подтверждение числа
    if (position == 1) {
      maxgradusnik = vvod;
    }
    if(position ==2){
      maxcube = vvod;
    }
    if(position ==3){
      deltat = vvod/10.0;
    }
    if(position ==4){
      timeotbora = vvod;
    }
    if(position ==5){
      timestabilization = vvod;
    }
    vvod = 0;
  }
}

void nastroiky(int pozicia) {
  if (pozicia != lastPosition ) {
    display.fillScreen(ILI9341_BLACK);
    if (pozicia == 0) {
      display.fillRect(0, 0, 240, 53, ILI9341_WHITE);
      display.setTextColor(ILI9341_BLACK);
      display.setCursor(5, 0);
      display.setTextSize(2);
      display.print("t ohlajdenie:");
      display.println(maxzmeitemp);
    } else {
      display.fillRect(0, 0, 240, 53, ILI9341_BLACK);
      display.setTextColor(ILI9341_WHITE);
      display.setCursor(5, 0);
      display.print("t ohlajdenie:");
      display.println(maxzmeitemp);
    }

    if (pozicia == 1) {
      display.fillRect(0, 53, 240, 106, ILI9341_WHITE);
      display.setTextColor(ILI9341_BLACK);
      display.setCursor(5, 53);
      display.setTextSize(2);
      display.print("t otbora:");
      display.println(maxgradusnik);
    } else {
      display.fillRect(0, 53, 240, 106, ILI9341_BLACK);
      display.setTextColor(ILI9341_WHITE);
      display.setCursor(5, 53);
      display.setTextSize(2);
      display.print("t otbora:");
      display.println(maxgradusnik);
    }

    if (pozicia == 2) {
      display.fillRect(0, 106, 240, 159, ILI9341_WHITE);
      display.setTextColor(ILI9341_BLACK);
      display.setCursor(5, 106);
      display.setTextSize(2);
      display.print("t cube:");
      display.println(maxcube);
    } else {
      display.fillRect(0, 106, 240, 159, ILI9341_BLACK);
      display.setTextColor(ILI9341_WHITE);
      display.setCursor(5, 106);
      display.setTextSize(2);
      display.print("t cube:");
      display.println(maxcube);    
      }

    if (pozicia == 3) {
      display.fillRect(0, 159, 240, 212, ILI9341_WHITE);
      display.setTextColor(ILI9341_BLACK);
      display.setCursor(5, 159);
      display.setTextSize(2);
      display.print("delta t:");
      display.println(deltat);
    } else {
      display.fillRect(0, 159, 240, 212, ILI9341_BLACK);
      display.setTextColor(ILI9341_WHITE);
      display.setCursor(5, 159);
      display.setTextSize(2);
      display.print("delta t:");
      display.println(deltat);
    }

    if (pozicia == 4) {
      display.fillRect(0, 212, 240, 265, ILI9341_WHITE);
      display.setTextColor(ILI9341_BLACK);
      display.setCursor(5, 212);
      display.setTextSize(2);
      display.print("time otbora G:");
      display.println(timeotbora);
    } else {
      display.fillRect(0, 212, 240, 265, ILI9341_BLACK);
      display.setTextColor(ILI9341_WHITE);
      display.setCursor(5, 212);
      display.setTextSize(2);
      display.print("time otbora G:");
      display.println(timeotbora);
    }

    if (pozicia == 5) {
      display.fillRect(0, 265, 240, 320, ILI9341_WHITE);
      display.setTextColor(ILI9341_BLACK);
      display.setCursor(5, 265);
      display.setTextSize(2);
      display.print("time stabil:");
      display.println(timestabilization);
    } else {
      display.fillRect(0, 265, 240, 320, ILI9341_BLACK);
      display.setTextColor(ILI9341_WHITE);
      display.setCursor(5, 265);
      display.setTextSize(2);
      display.print("time stabil:");
      display.println(timestabilization);
    }
    lastPosition = pozicia;
  }
}

void drawStaticElements() {
  display.drawRect(85, 250, 70, 45, ILI9341_WHITE);
  display.drawRect(84, 249, 71, 45, ILI9341_WHITE);
  display.drawRect(111, 10, 15, 240, ILI9341_WHITE);
  display.drawRect(86, 125, 25, 5, ILI9341_WHITE);
  display.drawRect(76, 124, 10, 7, ILI9341_WHITE); 
}

void drawOtbor(){
  if (procentotbor > 0 && durationactive){
    display.drawRect(126, 110, 12, 5, ILI9341_RED);
    display.drawRect(138, 110, 5, 40, ILI9341_RED);
  }
  else{
    display.drawRect(126, 110, 12, 5, ILI9341_WHITE);
    display.drawRect(138, 110, 5, 40, ILI9341_WHITE);
  }
}

void loop() {
  char customKey = customKeypad.getKey();
  if (customKey) {
    int number = customKey - '0';
    Serial.println(customKey);
    vvodogranichenia(customKey);
    if (number == 17) {
      position --;
    }
    if (number == 18) {
      position++;
    }
    if (number == 20) {
      display.fillScreen(ILI9341_BLACK);
      nastroiactive = !nastroiactive;
    }
    if (number == 19) {
      sospechka = !sospechka; 
      countpechka++;////////////
    }
  }

  int sum = 0;
  int sum1 = 0;
  int sum2 = 0;

  for(int i = 0; i < 50; i++) {
    sum += analogRead(A0);
    sum1 += analogRead(A1);
    sum2+= analogRead(A2);
  }

  float noroundcelsiusgradusnik = 1.0/(1.0/298.15 + 1.0/3950.0 * log((100000.0 * (sum / 50) / (1023.0 - (sum / 50)))/100000.0)) - 273.50; //268.50
  celsiusgradusnik = round(noroundcelsiusgradusnik*10)/10.0;

  float noroundcelsiuscube = 1.0/(1.0/298.15 + 1.0/3950.0 * log((100000.0 * (sum1 / 50) / (1023.0 - (sum1 / 50)))/100000.0)) - 272.50;
  celsiuscube = round(noroundcelsiuscube*10)/10.0;

  float noroundcelsiusverh = 1.0/(1.0/298.15 + 1.0/3950.0 * log((100000.0 * (sum2 / 50) / (1023.0 - (sum2 / 50)))/100000.0)) - 272.50;
  celsiusverh = round(noroundcelsiusverh*10)/10.0;

  int knopka = digitalRead(45);

  //if (celsiusverh > maxzmeitemp ) {//maxzmeitemp
    //sospechka = false; //если температура воды вверху больше сорока, то отключить печку
  //}

  // Сброс разрешения на новый цикл для отсчета при падении температуры ниже 70
  if (celsiusgradusnik < maxgradusnik) {
    allowNewCycle = true;
  }

  //digitalWrite(5,HIGH);

  if (position > 5) {
    position = 0;
  }

  if (position < 0) {
    position = 5;
  }

  if (glavtemp > glavtempsravnenie){
    Serial.println("отбор выключен");
  }

  if(procentotbor == 0 || celsiuscube == 100 || celsiusverh > maxzmeitemp||flagotkl){
    sospechka = false;
    voda = false;
    durationactive = false;
  }

  //Serial.println(countpechka%2);
  //Serial.println(countpechka);
  miganie();
  if (!nastroiactive) {
    gradusnik();
    cube();
    vodaverh();
    drawStaticElements();
    pechka();
    otrisovkavoda();
    drawOtbor();
  }
  else {
    nastroiky(position);
  }
} 

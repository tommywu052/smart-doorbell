///////////////////
//Servo Pin set
//黃 <--->(signal)
//紅<--->(voltage)
//棕 <--->(ground)
/////////////////////
#include <Wire.h>
#include "DHT.h"
#include "FastLED.h"
#include <Servo.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#include "rgb_lcd.h"

rgb_lcd lcd;

const int colorR = 255;
const int colorG = 255;
const int colorB = 255;

#define Do 262
#define Re 294
#define Mi 330 
#define Fa 349
#define So 392
// notes in the melody:
int melody[] = {
  Do, Re, Mi, Fa,So};
int duration = 50;  // 500 miliseconds


//溫度sensor設定
//#define DHTPIN 2
//#define DHTTYPE DHT11
//DHT dht(DHTPIN, DHTTYPE);

#include <Bounce.h>

Bounce bouncer = Bounce(5, 100);
static int ledStatus = LOW;

//LCD
#define ROW_NUM 2                             
#define COL_NUM 16                            
//#liquidCrystal_I2C lcd(0x27,COL_NUM,ROW_NUM);  
bool thunder_flag = 0;
int thunder_count = 0;

#define DATA_PIN 2
#define MAX_BRIGHTNESS 250
#define BRIGHT_INTERVAL MAX_BRIGHTNESS/NUM_METEORS
#define NUM_LEDS 8
#define NUM_METEORS 3
#define MAX_VOL 160

#define RELAY_PIN 4 

Servo myservo;

CRGB leds[NUM_LEDS];
int meteors[NUM_METEORS][3];

void MeteorLight();
void BlinkLight();
String readLine();
void CloseDoor();
void OpenDoor();

const size_t bufferSize = JSON_OBJECT_SIZE(5) + 40;
DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(8));
//StaticJsonBuffer<200> jsonBuffer;
//JsonObject& Jsonroot = readBuffer.createObject();

int leds_start,leds_end,start,num;
int bright = 50;

String Name = "";
bool Lock = false;
int val_R = 120;
int val_G = 120;
int val_B = 120;

void setup() {
   // Serial1.begin(9600);
    //Serial1.setTimeout(50);
    Serial.begin(9600);
      pinMode(5, INPUT);
    FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);
    //dht.begin();
//    lcd.begin();                      // initialize the lcd 
//    lcd.backlight();
//    lcd.clear(); 
//    lcd.setCursor(0, 0);
//    lcd.print("Who is coming?");
      lcd.begin(16, 2);    
      lcd.setRGB(colorR, colorG, colorB);    
      // Print a message to the LCD.
      lcd.print("Who Are You!");

    myservo.attach(9); 
    myservo.write(10);
    digitalWrite(RELAY_PIN,LOW);
    //設定Metrolight的顏色
    bright = 250;
    for(int i = 0; i < NUM_METEORS  ; i++ ){
        meteors[i][0] = 0;
        meteors[i][1] = 200;
        meteors[i][2] = bright;
        bright = bright + BRIGHT_INTERVAL;
    }

    //設定初始顏色
    for(int i = 0; i <= NUM_LEDS  ; i++ ){
        leds[i].setHSV(0,0,0);
        //leds[i] = CRGB::Red;
        //leds[i].setRGB(0,255,255);
    }
    //leds[0] = CRGB::Red;
    FastLED.show();

    Serial.println("start!");
    delay(1000);
    //rainbowWithGlitter();
    //FastLED.show();
}

String msgJSON;
char C_msgJSON[] = "";
bool DoorStatus = false; //false = close , true = open
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
void loop() { 
      
       if(bouncer.update() == true && bouncer.read() == HIGH) {
          ledStatus = ledStatus == HIGH ? LOW : HIGH;
          Serial.println(ledStatus);
          if(ledStatus==HIGH)
            OpenDoor();
          else
            CloseDoor();
          //digitalWrite(13, ledStatus);
        }
    while(Serial.available()){
    
        msgJSON = Serial.readString();
//        msgJSON.toCharArray(C_msgJSON,msgJSON.length()+1);
//        delay(1000);
//        Serial.print("C:");Serial.println(C_msgJSON);
        Serial.print("S:");Serial.println(msgJSON);
        JsonObject& root = jsonBuffer.parseObject(msgJSON);
        if (!root.success()) {
            Serial.println("parseObject() failed");
            return;
        }

        String man = root["Name"];
        Name = man;
        Lock = root["Lock"];
        val_R = root["R"];
        val_G = root["G"];
        val_B = root["B"];

        Serial.print("Name:");Serial.println(Name);
        Serial.print("Lock:");Serial.println(Lock);
        Serial.print("val_R:");Serial.println(val_R);
        Serial.print("val_G:");Serial.println(val_G);
        Serial.print("val_B:");Serial.println(val_B);
        
        lcd.setCursor(0, 1);
        lcd.print("        ");
        lcd.setCursor(0, 1);
        lcd.print(Name);
        
        if(DoorStatus != Lock){
            if(Lock == 1){
                OpenDoor();
                lcd.setCursor(11, 1);
                lcd.print("      ");
                lcd.setCursor(11, 1);
                lcd.print("OPEN");  
                DoorStatus = Lock;   
            }
            else{
                CloseDoor(); 
                lcd.setCursor(11, 1);
                lcd.print("      ");
                lcd.setCursor(11, 1);
                lcd.print("CLOSE"); 
                DoorStatus = Lock;      
            }    
        }
          EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow

// = 0; i < NUM_METEORS  ; i++ ){
//            meteors[i][0] = val_R;
//            met
//        for(int ieors[i][1] = val_G;
//            meteors[i][2] = val_B;
//        }
//        for(int i = 0; i <= NUM_LEDS  ; i++ ){
//        //leds[i].setHSV(0,0,0);
//        leds[i].setRGB(val_R,val_G,val_B);
//        }
//        man = "";  
    }
    //Serial.println("...");
    //delay(10);
    //BlinkLight();
    jsonBuffer.clear();
    msgJSON = ""; 
    Name = "";
     ThunderLight(val_R,val_G,val_B);
    //MeteorLight();
    //BlinkLight();
    

}
//    lock = true;
//    ControlDoor(lock);
//    delay(5000); 
//    lock = false;
//    ControlDoor(lock);
//    delay(5000); 

    //BlinkLight();
    //MeteorLight();  

//    if(Serial1.available()){
//        //Serial.print("read:");
//        cmdJson = Serial.readString();  
//        Serial.println(cmdJson);
//        cmdJson = "";
//        }
//    String str = '';
//    while (Serial.available()) {
//        char cmd = Serial.read();
//        if(c!='\n'){
//            str += cmd;
//        }
//        delay(5);    // 沒有延遲的話 UART 串口速度會跟不上Arduino的速度，會導致資料不完整
//    }

    
    
   
//    float Humidity = dht.readHumidity();
//    float Temperature = dht.readTemperature();
//
//    if (isnan(Humidity) || isnan(Temperature)) {
//        Serial.println("Failed to read from DHT sensor!");
//        return;
//    }
//
//    float HeatIndex = dht.computeHeatIndex(Temperature, Humidity, false);
//
//    Jsonroot["Humidity"] = Humidity;
//    Jsonroot["Temperature"] = Temperature;
//    Jsonroot["HeatIndex"] = HeatIndex;
//
//    Jsonroot.printTo(Serial);
//    Serial.println();
//    Serial1.flush();
//    delay(2000);


//String readLine(){
//    String s = "";
//    char c;
//    while(Serial1.available()){
//        c = Serial1.read();
//        s += c;
//        delay(50);
//        }
//    return s;
//}
//

void MeteorLight(){
    for(int i=0 ; i < NUM_LEDS+NUM_METEORS ; i++){
    for(int j = 0; j <= NUM_LEDS  ; j++ ){
    leds[j].setHSV(255,0,0);
    }
    
    if(i < NUM_METEORS){
        leds_start = 0;
        start = NUM_METEORS - i;
        num = i;
    }
    else if(i > NUM_LEDS){
        leds_start = i-NUM_METEORS;
        start = 0;
        num = (NUM_LEDS+NUM_METEORS) - i;
    }
    else{
        leds_start = i-NUM_METEORS;
        start = 0;
        num = NUM_METEORS;
      }

    for(int j = 0 ; j < num ; j++){
        leds[leds_start].setHSV(meteors[start][0],meteors[start][1],meteors[start][2]); 
        leds_start ++;
        start ++ ;
      }
      
    FastLED.show();
    delay(50);
    }
}


void BlinkLight(){
  for(int i=0; i<=MAX_BRIGHTNESS;i=i+5){
    for(int j=0; j<NUM_LEDS ; j++){
      leds[j].setHSV(0,100,i);   
      }
    FastLED.show();
    delay(10);
    }
  for(int i=MAX_BRIGHTNESS ; i>=0 ; i=i-5){
    for(int j=0; j<NUM_LEDS ; j++){
      leds[j].setHSV(0,100,i);   
      }
    FastLED.show();
    delay(10);
    }
}

void ThunderLight(int R, int G, int B){ 
  //Turn off all light
  for(int j = 0; j <= NUM_LEDS  ; j++ ){
    leds[j].setHSV(0,0,0); 
    }
    
  if(thunder_flag == 0){
    if(thunder_count < NUM_LEDS){
      
      leds[thunder_count].setHSV(R,G,B); //0,255,255
      if(thunder_count!=0)
        leds[thunder_count-1].setHSV(0,0,0);
      FastLED.show();
      delay(50);
      
      if(thunder_count == NUM_LEDS -1){
//        leds[thunder_count].setHSV(20,100,100);
//        FastLED.show();
 //       delay(10000);
        thunder_flag = 1;
        thunder_count = NUM_LEDS-2;
        }
      else
        thunder_count++;  
      }
  }
  if(thunder_flag == 1){
    if(thunder_count > 0){
      leds[thunder_count].setHSV(R,G,B);
      //leds[thunder_count]=CRGB::Purple;
      if(thunder_count!=NUM_LEDS-1)
        leds[thunder_count+1].setHSV(0,0,0);
        FastLED.show();
        delay(50);
        thunder_count--;
        if(thunder_count == 0){
          thunder_flag = 0;
          thunder_count = 0;
        }
      }
  }
  }

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}
void OpenDoor(){
  for (int thisNote = 0; thisNote < 8; thisNote++) {
    // 在 pin8 上輸出聲音，每個音階響 0.5 秒
    tone(3, melody[thisNote], duration);
    
    // 間隔一段時間後再播放下一個音階
    delay(100);
  }
    for(int i = 00; i <= 90; i+=2){
        myservo.write(i); // 使用write，傳入角度，從0度轉到180度
        delay(5);    
    }
}
void CloseDoor(){
    for(int i = 90; i >= 0; i-=2){
        myservo.write(i);// 使用write，傳入角度，從180度轉到0度
        delay(5);   
    }
}

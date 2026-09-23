#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "cyd_pins.h"

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(VSPI);
XPT2046_Touchscreen touch(PIN_TOUCH_CS, PIN_TOUCH_IRQ);

enum Page : uint8_t { HOME, WATER, TEMP, MANUAL, SETTINGS };
Page page = HOME;

struct State {
  float temp = 25.4;
  float targetTemp = 25.0;
  int tankLiters = 400;
  int changePercent = 20;
  bool autoChange = true;
  bool heater = false;
  bool chiller = false;
  bool inlet = false;
  bool drain = false;
  bool filter = true;
  bool levelOK = true;
} st;

uint32_t lastDemo = 0;

uint16_t C_BG, C_CARD, C_CARD2, C_TEXT, C_MUTED, C_ACCENT, C_OK, C_WARN, C_DANGER, C_LINE;

uint16_t rgb(uint8_t r,uint8_t g,uint8_t b){ return tft.color565(r,g,b); }

void theme() {
  C_BG=rgb(8,14,24); C_CARD=rgb(17,27,42); C_CARD2=rgb(23,36,55);
  C_TEXT=rgb(235,242,250); C_MUTED=rgb(137,155,177); C_ACCENT=rgb(39,166,255);
  C_OK=rgb(43,201,132); C_WARN=rgb(255,181,71); C_DANGER=rgb(255,91,91); C_LINE=rgb(45,62,82);
}
void txt(const String&s,int x,int y,int size=1,uint16_t c=0){
  tft.setTextColor(c?c:C_TEXT,C_BG); tft.setTextSize(size); tft.setCursor(x,y); tft.print(s);
}
void fillRound(int x,int y,int w,int h,uint16_t c){ tft.fillRoundRect(x,y,w,h,7,c); }
void header(const String& title){
  tft.fillScreen(C_BG);
  txt(title,10,8,2);
  tft.fillCircle(294,14,4,C_OK);
  txt("LIVE",269,9,1,C_MUTED);
  tft.drawFastHLine(8,31,304,C_LINE);
}
void badge(const String&s,int x,int y,uint16_t c){
  int w=s.length()*6+12; fillRound(x,y,w,18,c); tft.setTextColor(C_BG,c); tft.setTextSize(1); tft.setCursor(x+6,y+5); tft.print(s);
}
void card(int x,int y,int w,int h,const String& label){
  fillRound(x,y,w,h,C_CARD);
  tft.setTextColor(C_MUTED,C_CARD); tft.setTextSize(1); tft.setCursor(x+9,y+8); tft.print(label);
}
void bottomNav(){
  const char* names[]={"HOME","WATER","TEMP","MAN","SET"};
  int xs[]={3,67,131,195,259};
  for(int i=0;i<5;i++){
    uint16_t c=((int)page==i)?C_ACCENT:C_CARD2;
    fillRound(xs[i],211,58,25,c);
    tft.setTextColor(((int)page==i)?C_BG:C_MUTED,c);
    tft.setTextSize(1); tft.setCursor(xs[i]+8,220); tft.print(names[i]);
  }
}
void home(){
  header("SMART AQUARIUM");
  card(8,39,148,72,"WATER TEMPERATURE");
  tft.setTextColor(C_TEXT,C_CARD); tft.setTextSize(3); tft.setCursor(17,60); tft.print(st.temp,1);
  tft.setTextSize(1); tft.print(" C"); badge("NORMAL",89,83,C_OK);

  card(164,39,148,72,"WEEKLY WATER CHANGE");
  tft.setTextColor(C_TEXT,C_CARD); tft.setTextSize(3); tft.setCursor(174,60); tft.print(st.changePercent); tft.print("%");
  tft.setTextSize(1); tft.setCursor(174,89); tft.print(String(st.tankLiters*st.changePercent/100)+" L / Sunday");

  card(8,119,96,82,"LEVEL"); badge(st.levelOK?"OK":"ALARM",17,145,st.levelOK?C_OK:C_DANGER);
  card(112,119,96,82,"FILTER"); badge(st.filter?"ON":"OFF",121,145,st.filter?C_OK:C_MUTED);
  card(216,119,96,82,"CLIMATE");
  badge(st.heater?"HEAT":st.chiller?"COOL":"IDLE",225,145,(st.heater||st.chiller)?C_WARN:C_OK);
  bottomNav();
}
void water(){
  header("AUTO WATER CHANGE");
  card(8,39,304,49,"SCHEDULE");
  txt("Sunday  09:00",18,59,2,C_TEXT); badge(st.autoChange?"AUTO ON":"AUTO OFF",225,55,st.autoChange?C_OK:C_MUTED);

  card(8,96,304,68,"REPLACEMENT VOLUME");
  txt(String(st.changePercent)+"%",18,116,3,C_TEXT);
  txt(String(st.tankLiters*st.changePercent/100)+" L of "+String(st.tankLiters)+" L",90,122,1,C_MUTED);
  fillRound(18,146,276,6,C_CARD2);
  tft.fillRoundRect(18,146,(276*st.changePercent)/50,6,3,C_ACCENT);

  fillRound(8,172,94,31,C_CARD2); txt("-5%",36,183,1,C_TEXT);
  fillRound(113,172,94,31,C_ACCENT); tft.setTextColor(C_BG,C_ACCENT);tft.setCursor(132,183);tft.print("START");
  fillRound(218,172,94,31,C_CARD2); txt("+5%",246,183,1,C_TEXT);
  bottomNav();
}
void tempPage(){
  header("TEMPERATURE");
  card(8,39,304,73,"CURRENT");
  tft.setTextColor(C_TEXT,C_CARD);tft.setTextSize(4);tft.setCursor(18,62);tft.print(st.temp,1);tft.setTextSize(1);tft.print(" C");
  card(8,120,148,81,"TARGET");
  txt(String(st.targetTemp,1)+" C",18,145,2,C_TEXT);
  fillRound(18,174,55,20,C_CARD2);txt("-",42,180,1,C_TEXT);
  fillRound(91,174,55,20,C_CARD2);txt("+",115,180,1,C_TEXT);
  card(164,120,148,81,"CONTROL");
  badge(st.heater?"HEATER ON":"HEATER OFF",174,145,st.heater?C_WARN:C_MUTED);
  badge(st.chiller?"CHILLER ON":"CHILLER OFF",174,174,st.chiller?C_ACCENT:C_MUTED);
  bottomNav();
}
void manualPage(){
  header("MANUAL CONTROL");
  const char* labels[]={"INLET VALVE","DRAIN VALVE","HEATER","CHILLER"};
  bool vals[]={st.inlet,st.drain,st.heater,st.chiller};
  int x[]={8,164,8,164}, y[]={42,42,122,122};
  for(int i=0;i<4;i++){
    card(x[i],y[i],148,70,labels[i]);
    badge(vals[i]?"ON":"OFF",x[i]+10,y[i]+37,vals[i]?C_OK:C_MUTED);
  }
  txt("Preview mode - outputs are not connected",16,198,1,C_WARN);
  bottomNav();
}
void settings(){
  header("SETTINGS");
  card(8,39,304,45,"AQUARIUM VOLUME"); txt(String(st.tankLiters)+" L",220,55,2,C_TEXT);
  card(8,92,304,45,"I/O EXPANSION"); txt("MCP23017 planned",170,108,1,C_MUTED);
  card(8,145,304,56,"SAFETY");
  txt("LOW / HIGH / EMERGENCY / LEAK / FLOW",18,166,1,C_TEXT);
  txt("Hardware outputs disabled in preview",18,184,1,C_WARN);
  bottomNav();
}
void draw(){ switch(page){case HOME:home();break;case WATER:water();break;case TEMP:tempPage();break;case MANUAL:manualPage();break;case SETTINGS:settings();break;} }

bool readTouch(int &x,int &y){
  if(!touch.touched()) return false;
  TS_Point p=touch.getPoint();
  // Common CYD calibration; fine-tune later from real panel values.
  x=map(p.x,200,3800,0,320);
  y=map(p.y,240,3800,0,240);
  x=constrain(x,0,319); y=constrain(y,0,239);
  return true;
}
void handleTouch(){
  int x,y; if(!readTouch(x,y)) return;
  if(y>=207){
    int idx=constrain(x/64,0,4); page=(Page)idx; draw(); delay(220); return;
  }
  if(page==WATER){
    if(y>=168 && y<=207 && x<105){ st.changePercent=max(5,st.changePercent-5);draw();delay(220); }
    else if(y>=168 && y<=207 && x>215){ st.changePercent=min(50,st.changePercent+5);draw();delay(220); }
    else if(y>=50 && y<=90 && x>210){st.autoChange=!st.autoChange;draw();delay(220);}
  } else if(page==TEMP){
    if(y>=168&&y<=200&&x<80){st.targetTemp-=0.5;draw();delay(220);}
    else if(y>=168&&y<=200&&x>80&&x<160){st.targetTemp+=0.5;draw();delay(220);}
  } else if(page==MANUAL){
    if(y>=40&&y<115&&x<160) st.inlet=!st.inlet;
    else if(y>=40&&y<115) st.drain=!st.drain;
    else if(y>=118&&y<195&&x<160) st.heater=!st.heater;
    else if(y>=118&&y<195) st.chiller=!st.chiller;
    draw(); delay(220);
  }
}
void setup(){
  Serial.begin(115200);
  pinMode(PIN_TFT_BL,OUTPUT); digitalWrite(PIN_TFT_BL,HIGH);
  tft.init(); tft.setRotation(1); tft.setTextWrap(false);
  theme();
  touchSPI.begin(PIN_TOUCH_CLK,PIN_TOUCH_MISO,PIN_TOUCH_MOSI,PIN_TOUCH_CS);
  // This library uses the global SPI object internally on some versions.
  // The common CYD touch controller is still initialized safely for UI preview.
  touch.begin();
  touch.setRotation(1);
  draw();
}
void loop(){
  handleTouch();
  if(millis()-lastDemo>5000){
    lastDemo=millis();
    st.temp += 0.1; if(st.temp>26.0) st.temp=25.2;
    if(page==HOME || page==TEMP) draw();
  }
  delay(10);
}

#include <U8g2lib.h>
#include <LM75A.h>
#include <EEPROM.h>

U8G2_PCD8544_84X48_1_4W_SW_SPI u8g2(U8G2_R2, /* clock=*/2, /* data=*/ 3, /* cs=*/ 5, /* dc=*/ 4, /* reset=*/ 6);  // Nokia 5110 Display
LM75A lm75a_sensor(false,false,false);

float jilu[84] = {};
short jiluc=84;
int zhenjiao = 11;
int settem;
float nowtem;
short zhuangtai = 1; //1加热中0未加热
boolean as0 = false, as1 = false, as2 = false;
float tem0 = 2.0; //未加热时低于设定多少开始加热
float tem1 = 2.0; //加热时高于设定多少停止加热
unsigned long shangci;
short xiaozhouqi = 20; //小循环键盘扫描时间
int dazhouqi = 200; //大循环逻辑判断与测温显示时间
int jiluzhouqi = 0; //初始化时计算
unsigned long shangcijilu;
short xianshiz = 0;
float jtem;
short switcha;
boolean start = false;

void setup() {
  jiluzhouqi=300000/jiluc; //记录周期300秒
  pinMode(zhenjiao,OUTPUT); 
  pinMode(8,INPUT_PULLUP); 
  pinMode(9,INPUT_PULLUP); 
  pinMode(7,INPUT_PULLUP); 
  EEPROM.get(16,settem);
  u8g2.begin();
}

void loop() {
  if(millis()-shangci>=dazhouqi)
  {
    shangci=millis();
    xunhuan();
  }
  if(millis()-shangcijilu>=jiluzhouqi)
  {
    shangcijilu=millis();
    jiluwendu();
  }
  boolean ts0 = false, ts1 = false, ts2 = false;
  boolean gengxin = false;
  if(digitalRead(8)==LOW)
  {
    ts0=true;
  }
  if(digitalRead(9)==LOW)
  {
    ts1=true;
  }
  if(digitalRead(7)==LOW)
  {
    ts2=true;
  }
  if(ts0==true && as0==false)
  {
    settem--;
    gengxin=true;
    tone(10,1500,50);
  }
  if(ts1==true && as1==false)
  {
    settem++;
    gengxin=true;
    tone(10,3000,50);
  }
  if(ts2==true && as2==false)
  {
    xianshiz++;
    if(xianshiz>1)
    {
      xianshiz=0;
    }
    tone(10,2250,10);
    gengxin=true;
  }
  as0=ts0;
  as1=ts1;
  as2=ts2;
  if(gengxin==true)
  {
    EEPROM.put(16,settem);
    xianshi();
  }
  delay(xiaozhouqi);
}

void jiluwendu() {
  for (int i=0; i<jiluc-1 ;i++)
  {
    jilu[i]=jilu[i+1];
  }
  jilu[jiluc-1]=nowtem;
}

void xianshiquxian() {
  float maxtem = 0;
  float smalltem = 100;
  for (int i=0; i<jiluc ;i++)
  {
    if(jilu[i]>maxtem)
    {
      maxtem=jilu[i];
    }
    if(jilu[i]<smalltem)
    {
      smalltem=jilu[i];
    }
  }
  if(settem>maxtem)
  {
    maxtem=settem;
  }
  if(settem<smalltem)
  {
    smalltem=settem;
  }
  double beishu = (maxtem-smalltem)/48.0;
  short jilua[84] = {};
  for (int i=0; i<jiluc ;i++)
  {
    jilua[i]=pxy(jilu[i],smalltem,beishu);
  }
  short setp = pxy(settem,smalltem,beishu);
  boolean xvxian = true;
  u8g2.firstPage();
  u8g2.setFont(u8g2_font_blipfest_07_tr);
  do {
    for (int i=0; i<jiluc ;i++)
    {
      if(i!=0)
      {
        u8g2.drawLine(i-1,jilua[i-1],i,jilua[i]);
      }
      //u8g2.drawPixel(i,jilua[i]);
      if(xvxian)
      {
        xvxian=false;
        u8g2.drawPixel(i,setp);
      }
      else
      {
        xvxian=true;
      }
    }
    u8g2.setCursor(0, 6);
    u8g2.print(round1(maxtem,-1),1);
    u8g2.setCursor(0, 47);
    u8g2.print(round1(smalltem,-1),1);
    if(setp>6)
    {
      u8g2.setCursor(77, setp);
    }
    else
    {
      u8g2.setCursor(77, setp+6);
    }
    u8g2.print(settem);
  } while ( u8g2.nextPage() );
}

short pxy(float sj,float sm,double bs){
  float yp = (sj-sm)/bs;
  int ypi = 47-round1(yp,0);
  if(ypi<0)
  {
    ypi=0;
  }
  if(ypi>47)
  {
    ypi=47;
  }
  return ypi;
}

void xunhuan() {
  nowtem=lm75a_sensor.getTemperatureInDegrees();
  if(!start)
  {
    start=true;
    for (int i=0; i<jiluc ;i++)
    {
      jilu[i]=nowtem;
    }
  }
  if(zhuangtai==0 && switcha==0)
  {
    nowtem=nowtem-jtem;
  }
  if(zhuangtai==0)
  {
    digitalWrite(zhenjiao, HIGH);
    if(settem-nowtem>=tem0)
    {
      zhuangtai=1;
      digitalWrite(zhenjiao, LOW);
      delay(200);
    }
    if(switcha==1)
    {
      switcha=0;
      delay(200);
      jtem=lm75a_sensor.getTemperatureInDegrees()-nowtem;
    }
  }
  else if(zhuangtai==1)
  {
    if(nowtem-settem>=tem1)
    {
      zhuangtai=0;
      switcha=1;
    }
    digitalWrite(zhenjiao, LOW);
  }
  xianshi();
}

void xianshi() {
  if(xianshiz==0)
  {
    xinxi();
  }
  else if(xianshiz==1)
  {
    xianshiquxian();
  }
}

void xinxi() {
  long ts = millis()/1000;
  long tm = 0;
  int th = 0;
  while(ts>=60)
  {
    ts=ts-60;
    tm++;
  }
  while(tm>=60)
  {
    tm=tm-60;
    th++;
  }
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_fub14_tf);
    u8g2.setCursor(0, 15);
    u8g2.print("Set:");
    u8g2.print(settem);
    u8g2.print(" C");
    u8g2.setCursor(0, 31);
    u8g2.print("Now");
    u8g2.print(round1(nowtem,-1),1);
    u8g2.setCursor(0, 47);
    u8g2.print("");
    if(th<10)
    {
      u8g2.print("0");
    }
    u8g2.print(th);
    u8g2.print(":");
    if(tm<10)
    {
      u8g2.print("0");
    }
    u8g2.print(tm);
    u8g2.print(":");
    if(ts<10)
    {
      u8g2.print("0");
    }
    u8g2.println(ts);
  } while ( u8g2.nextPage() );
}

float round1(float d, int n){ //这个子程序代码忘了是哪里找的，反正下面这个不是我做的，四舍五入用的
  double dst;
  d = d * pow(10, -n );     /*四捨五入して残す桁を1の位にする*/
  if(d>=0){
   dst = (double)(int)(d + 0.5);
  }else{    // マイナスの時は絶対値にマイナスを付ける
      dst = (double)(int)(d - 0.5);
  }
  return  dst * pow(10, n );    /*処理を行った桁を元に戻す*/
}

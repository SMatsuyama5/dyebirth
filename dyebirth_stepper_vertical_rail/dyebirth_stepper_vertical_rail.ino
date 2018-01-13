#include <OSCBoards copy.h>
#include <OSCBoards.h>
#include <OSCBundle.h>
#include <OSCData.h>
#include <OSCMatch.h>
#include <OSCMessage.h>
#include <OSCTiming.h>
#include <SLIPEncodedSerial.h>
#include <SLIPEncodedUSBSerial.h>


#include <SPI.h>
#include <Ethernet2.h>
#include <EthernetUdp2.h>
//////////////////////OSC用のインクルード/////////////////////

//OSC用設定
byte mac[] = {0x90, 0xA2, 0xDA, 0x10, 0x51, 0x1E}; //シールドのアドレス Arduino1 縦レール

IPAddress ip(192, 168, 11, 6);//環境に合わせて設定を Arduino2

IPAddress gateway(192, 168, 11, 5);   //環境に合わせて設定を
IPAddress subnet(255, 255, 255, 0);    //環境に合わせて設定を
unsigned int localPort = 8888;
byte destIP[] = {192, 168, 11, 5}; //送信相手のIPアドレス
int destPort = 9001;
EthernetUDP Udp;
EthernetServer server(23);

//////////////////////////////////////////////////

int PUL=7; //define Pulse pin
int DIR=6; //define Direction pin
int ENA=5; //define Enable Pin


int speedval;

void setup() {

//エンドセンサー用
 pinMode(2,INPUT) ;    //スイッチに接続ピンをデジタル入力に設定
 pinMode(3,INPUT) ;    //スイッチに接続ピンをデジタル入力に設定

 // UDPなどの初期設定
  Ethernet.begin(mac, ip, gateway, subnet);
  Udp.begin(localPort);
  Serial.println(Ethernet.localIP());//IP取れてるか確認用

  
  pinMode (PUL, OUTPUT);
  pinMode (DIR, OUTPUT);
  pinMode (ENA, OUTPUT);

//スピード初期値
  speedval =70;

}


void loop() {
EthernetClient client = server.available();

  OSCMessage msg;
  OSCBundle bndl;
   int size;
   
    //receive a bundle
   if( (size = Udp.parsePacket())>0)
   {
   
    while(size--)
           bndl.fill(Udp.read());  
           msg.fill(Udp.read()); 
         
   }


   if(!bndl.hasError())
        {
               //DO SOME WORK HERE-------------------
              bndl.route("/step", rail_position1);   
              bndl.route("/speed2", set_speed);  ////C
              bndl.route("/Calibration", Calibration);
               
              //and echo it back
             if(bndl.size() > 0)
             {   
                       
           
             }
        }        
}

void rail_position1(OSCMessage &msg){
        int inValue = msg.getInt(0); 
        String DIR;

            OSCBundle bndlOUT;
            bndlOUT.add("/from_rail2").add(inValue);
            Udp.beginPacket(Udp.remoteIP(), destPort);
            bndlOUT.send(Udp);
            Udp.endPacket();

        
        if(inValue >=0)   {
          DIR ="front";        
        }
         else if(inValue <0){
         DIR ="back";
         inValue= inValue*-1;    
        }
        
        front_step(inValue,speedval,DIR);


         OSCBundle bndlOUT2;
         bndlOUT2.add("/from_rail2_end").add(inValue);
         Udp.beginPacket(Udp.remoteIP(), destPort);
         bndlOUT2.send(Udp);
         Udp.endPacket();

}


void set_speed(OSCMessage &msg){
      int inValue = msg.getInt(0); 
          speedval = inValue;

          OSCBundle bndlOUT;
            bndlOUT.add("/set_speed2").add(speedval);   ////C
            Udp.beginPacket(Udp.remoteIP(), destPort);
            bndlOUT.send(Udp);
            Udp.endPacket(); 
}

void Calibration(OSCMessage &msg){
        int inValue = msg.getInt(0); 
             
            OSCBundle bndlOUT;
            bndlOUT.add("/Calibration").add(inValue);
            Udp.beginPacket(Udp.remoteIP(), destPort);
            bndlOUT.send(Udp);
            Udp.endPacket();

        if(inValue == 1 ){
          
             for (int i=0; i<30000; i++){

              if (digitalRead(3) == LOW){  
                delay(500);
                for (int i=0; i<1000; i++)   //Backward 5000 steps
                 {
     
                     digitalWrite(DIR,LOW);
                     digitalWrite(ENA,HIGH);
                     digitalWrite(PUL,HIGH);
                     delayMicroseconds(100);
                     digitalWrite(PUL,LOW);
                     delayMicroseconds(100); 
                   }
                

                OSCBundle bndlOUT;
                bndlOUT.add("/Calibration_com").add(inValue);
                Udp.beginPacket(Udp.remoteIP(), destPort);
                bndlOUT.send(Udp);
                Udp.endPacket();
              
              break;
              }  
             
                back(90);          
          }
  
        }
  }

//指定ステップ、指定したスピードで回転
void front_step(int step, int Speed, String DIR){
 for (int i=0; i<step; i++)
  { 

   if(DIR == "front"){
if (digitalRead(2) == LOW) {     //スイッチの状態を調べる
    sensor1();
    break;
    }  
  }    
  
  if(DIR == "back"){
   if (digitalRead(3) == LOW) {     //スイッチの状態を調べる
    sensor2();
    break;
    }  
  } 
  
    int speed1 =Speed;
    if (i < 1500){
      speed1 = 150-i/15;     
    }
    if (step-i<1500){
      speed1 = 150 - ((step-i)/15);
    }

  if(DIR == "front"){
   front(speed1); 
    }

  if(DIR == "back"){
    back(speed1); 
    }   
  } 
}


//前進する関数
void front(int Speed){
   for (int i=0; i<3; i++){
    digitalWrite(DIR,LOW);
    digitalWrite(ENA,HIGH);
    digitalWrite(PUL,HIGH);
    delayMicroseconds(Speed);
    digitalWrite(PUL,LOW);
    delayMicroseconds(Speed);
   }
   
}

//バックする関数
void back(int Speed){
  for (int i=0; i<3; i++){
    digitalWrite(DIR,HIGH);
    digitalWrite(ENA,HIGH);
    digitalWrite(PUL,HIGH);
    delayMicroseconds(Speed);
    digitalWrite(PUL,LOW);
    delayMicroseconds(Speed);
     }
}



//ぶつかったときにちょっと戻す関数
void sensor1(){
delay(500);
for (int i=0; i<500; i++)   //Backward 5000 steps
  {
     
    digitalWrite(DIR,HIGH);
    digitalWrite(ENA,HIGH);
    digitalWrite(PUL,HIGH);
    delayMicroseconds(100);
    digitalWrite(PUL,LOW);
    delayMicroseconds(100); 
  }
                OSCBundle bndlOUT;
                bndlOUT.add("/sensor2_1").add(1);
                Udp.beginPacket(Udp.remoteIP(), destPort);
                bndlOUT.send(Udp);
                Udp.endPacket();
  
}

void sensor2(){
delay(500);
for (int i=0; i<500; i++)   //Backward 5000 steps
  {
    
    digitalWrite(DIR,LOW);
    digitalWrite(ENA,HIGH);
    digitalWrite(PUL,HIGH);
    delayMicroseconds(100);
    digitalWrite(PUL,LOW);
    delayMicroseconds(100); 
  }

                OSCBundle bndlOUT;
                bndlOUT.add("/sensor2_2").add(1);
                Udp.beginPacket(Udp.remoteIP(), destPort);
                bndlOUT.send(Udp);
                Udp.endPacket();
  
}



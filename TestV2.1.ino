#include<SoftwareSerial.h>
#define OK 0
#define NOTOK 1
//핀번호 정의
int PIR[2]={2,3};
int ECHO[4]={6,13,4,10};
int TRIG[4]={7,12,5,11};
int BT_RXD = 8;
int BT_TXD = 9;
SoftwareSerial bluetooth(BT_TXD, BT_RXD);
/* 
arrayEnd: 배열의 크기 지정
start: 거리가 변하는 수를 체크할 때 시작할 위치
standard: 배열상 거리 변화한 수가 일정 수 이상일 경우 앉았다고 체크할 기준
distancePersesnt: 초음파의 오차 범위(%)
shitcount: 사람이 없다고 판단할 경우 1씩 증가, 이 수가 20이상일 경우 비었다고 판단 
*/
//////////////////////////////////////////////////
int arrayEnd=51;
int start=arrayEnd/2;
int standard=9; //거리변화의 수 > 카운트 앉아있음
//int standUpPersent=10; //센서의 오차 
int distancePersent=1; //높이면 오차범위 증가 
//bool shitDown=false;
//int shitDownCount=0;
unsigned long myTime=millis();
//unsigned long time_2=millis();

float distance[51]={0,}, avg=0;
int count=0;
bool tmp[4]={false, false, false, false};
int shitcount[4]={0,};
bool isShit[4]={false, false, false, false};
int wentCount=20;
//////////////////////////////////////////////////////

//거리 리턴해줌
float returnDistance(int ECHO, int TRIG) {
  digitalWrite(TRIG,HIGH);
  delay(10);
  digitalWrite(TRIG,LOW);
  
  unsigned long int duration = pulseIn(ECHO, HIGH);

  return ((340*duration)/10000.0)/2.0;
}

//배열에 거리 50개 넣어서 변화를 체크한다음 자리 있음, 비었음 체크
int checkPerson(int ECHO, int TRIG) {
  int i=0;
  distance[0]=returnDistance(ECHO,TRIG);

    avg=0;
    count=0;
    /*if(digitalRead(PIR)==HIGH || time+60000<millis()){
        if(time+300000<millis()){
          time=millis();
        }*/
        for(i=1; i<arrayEnd; i++){
          float tmp=returnDistance(ECHO, TRIG);
            /*if(tmp>300){
              distance[i]=distance[i-1];
            }*/
            /*else{*/
              distance[i]=tmp;
            //}
            avg+=distance[i];
            Serial.print(i);
            Serial.print("번 ");
            Serial.print(distance[i]);
            Serial.print("[cm]");
            delay(100);
        }
        avg/=arrayEnd-1;
        Serial.print("평균: ");
        Serial.println(avg);

        //배열상에서 거리가 변하는 수를 체크
        /*distance[i-1]*(1-distancePersent*0.01)<distance[i] && distance[i-1]*(1+distancePersent*0.01)>distance[i]*/
        for(i=start; i<arrayEnd; i++){
          if(abs(distance[i-1]-distance[i])>distance[i-1]*distancePersent*0.01) {
            count++;
          }
        }
        
        Serial.print("count= ");
        Serial.println(count);

        //배열의 거리 변화 수가 일정 수(standard)이하면 누가 치고 갔거나 
        //모션인식이 멀리 있는 사람을 인식한 경우
        if(count<standard){
            Serial.println("no person");
            //shitDown=false;
            //shitDownCount++;
            return NOTOK;
            //break;
        }
        //배열의 거리 변화 수가 일정 수(standard)이상이면 자리에 앉는 경우
        else {
            //사람이 있다.
            Serial.println("yes person");
            //shitDown=true;
            //shitDownCount=0;
            return OK;
        }

        /*if(shitDownCount>20) {
              Serial.println("went out");
        }*/
}

void sendInfo(char shit, char table, char seat) {
    bluetooth.write(shit);
    bluetooth.write(table);
    bluetooth.write(seat);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  int i=0;
  for(i=0; i<4; i++) {
    pinMode(ECHO[i],INPUT);
    pinMode(TRIG[i],OUTPUT);
    pinMode(ECHO[i],INPUT);
    pinMode(TRIG[i],OUTPUT);
  }
  /*
  pinMode(ECHO_1,INPUT);
  pinMode(TRIG_1,OUTPUT);
  pinMode(ECHO_2,INPUT);
  pinMode(TRIG_2,OUTPUT);
  */
  bluetooth.begin(9600);
  Serial.print("set up\n");
}
/*
 검사할 좌석을 선택하는 기능
 자리 비는 시간 기준(count)을 외부에서 설정 가능하게
 시간 체크를 위한 자리 비움 체크, 모션인식 따로 검사하는 기능
 키오스크에서 주문 정보를 받아오면 실시간은 불가능이어도 
 어느정도 정확한 아웃시간 확인 가능(회전률, 평균 이용시간 확인 가능)
 */
 void checkShit(int i) {
    tmp[i]=checkPerson(ECHO[i], TRIG[i]);
    Serial.print(i+1);
    Serial.println("번자리");
    if(tmp[i]==OK) {
      if(isShit[i]==false) {
        isShit[i]=true;
        char iStr=i+49;
        sendInfo('1','1',iStr);
        Serial.print(iStr);
        Serial.println("번자리 in");
      }
      shitcount[i]=0;
    }
 }
void loop() {
  
  if(digitalRead(PIR[0])==HIGH) {
    //초음파 센서로 검사
    checkShit(0);
    checkShit(1);
  }
  if(digitalRead(PIR[1])==HIGH) {
    //초음파 센서로 검사
    checkShit(2);
    checkShit(3);
  }
  
  if(myTime+60000<millis()) {
    myTime=millis();
    //모든 좌석 검사
    int i=0;
    for(i=0; i<4; i++){
      if(checkPerson(ECHO[i], TRIG[i])==NOTOK) {
        shitcount[i]++;
      }
      if(shitcount[i]==wentCount) {
        char iStr=i+49;
        Serial.print(iStr);
        Serial.println("번 자리 out");
        
        sendInfo('0','1',iStr);
      }
    }
  }
} 

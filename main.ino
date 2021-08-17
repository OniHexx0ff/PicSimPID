
#include <LiquidCrystal.h>

#define SENSOR A0
#define POT1   A2
#define HEATER 10
#define COOLER 9
#define BTN_CHANGE 2
LiquidCrystal lcd (12, 11, 5, 4, 3, 7);


  double 
    error;
    
  double 
    kP = 25.0,
    kI =  6.5,
    kD = 1.0 ;      
  double P, I, D;
  double pid;
  
  double setPoint =42.0;
  

int 
  i,
  value,
  cont,
  displayCont=0,
  timer1_counter,
  val =0,
  state=1;

double
  pwmControl =0,
  tension =0,
  temperature =0,
  temperatureSumm =0,
  lastTemp =0;

float 
  deltaTime=0,
  const_temp = 0.01,
  dt=0;
  

long lastDebounceTime = 0;  
long debounceDelay = 20;   



void lerEntrada(int sample){
  
  for(i=0; i < sample ; i++){
      value = analogRead(SENSOR);
      tension = value*5 /1023.0;
      temperatureSumm+= tension/0.010;  
    }
    
    temperature = temperatureSumm/sample;  
    
    temperatureSumm = 0; 
    
}


void outputManager(){

  
   if(pwmControl < 0)
  {    pwmControl = 0;    }
  if(pwmControl > 255)  
  {    pwmControl = 255;  }
  
  analogWrite(HEATER,pwmControl);
  if(displayCont >=30 ){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print ("Temp:");
      lcd.setCursor(6,0);
      if(state==1){
        lcd.print (String(temperature)+" "+((char) 223)+"C");
      //  lcd.print (String(pwmControl));
      }
      else if(state==2){
        lcd.print (String((temperature*1.8+32))+" "+((char) 223)+"F");
      }
      else if(state==3){
        lcd.print (String((temperature+273.15))+" "+"K");
      }
      lcd.setCursor(0,1);
      lcd.print ("Duty:   "+String(pwmControl));
      displayCont =0;
  }
}


void disturbio(){
  // Liga o coller para fazer um disturbio no ambiente
  dt = map(analogRead(POT1),0,1023,0,50);
  
  analogWrite(COOLER, dt);
   
}

void mudaEscala(){
  
  if((millis() - lastDebounceTime) > debounceDelay){
    if(state<3){
      state++;
    }
    else{
      state=1;
   }
  }
  lastDebounceTime = millis();
}




void setup () {
  
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  timer1_counter = 64911;   // preload timer 65536-16MHz/256/100Hz
  TCNT1 = timer1_counter;   // preload timer
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts
  

  pinMode(HEATER, OUTPUT);
  pinMode(COOLER, OUTPUT);
  pinMode(POT1, INPUT);
 attachInterrupt(digitalPinToInterrupt(BTN_CHANGE), mudaEscala, RISING);
  
  lcd.begin (16, 2);
  lcd.setCursor(0,0);
  lcd.print ("   Iniciando:");
  lcd.setCursor(0,1);
  lcd.print ("    00000000     ");

}

void loop(){
  
  lerEntrada(150);
  
  deltaTime = cont* const_temp;
  cont = 0;

  error = setPoint - temperature;
  
  P = error * kP;
  
  //I
  I = I + (error * kI) * deltaTime;
  
  //D
  D = (lastTemp - temperature) * kD / deltaTime;
  lastTemp= temperature;
   
  
  // Soma tudo
  pid = P + I + D;
  
  pwmControl=(pid+10 );
 
  outputManager();
  disturbio();

}


ISR(TIMER1_OVF_vect){    //interrupção a cada 10ms
  TCNT1 = timer1_counter;
  cont++;
  displayCont++;
  
}

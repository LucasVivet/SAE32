#include <RH_RF95.h>
#include <M5Stack.h>

RH_RF95 rf95(5,36);                   // Une instance de la couche radio

uint8_t state;                // état courant
uint8_t txbuf[RH_RF95_MAX_MESSAGE_LEN] ;    // buffer de trame en émission
uint8_t txbuflen = RH_RF95_MAX_MESSAGE_LEN;   // taille de la trame à émettre
uint16_t S, SP;
char temp[255];

// déclaration des noms et numéros de chaque état
#define EMISSION 0
#define DELAI  1

#define canal 0        // déclaration du canal

int i;

void setup ()
{
  M5.begin() ;  // Initialisation port console pour debug
  M5.Power.begin();
  termInit();  

  if (!rf95.init())
    M5.lcd.println("RF95 init erreur") ;
  else
    M5.lcd.println("RF95 init OK");

  // configuration de la fréquence radio
  rf95.setTxPower(10);
  rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128 );
  rf95.setFrequency(867.4);

  state = EMISSION;     // état de départ

  delay(3000);      // délai au début pour laisser le temps de lancer le terminal 
  M5.lcd.println("boucle principale");

}

void loop() {
  switch ( state ) {
  case EMISSION:          // code source de 1’état EMISSION
    delay(3000);
    printString("--------------------------------------------\r");
    printString("EMISSION\r");
    S=0; SP=0;
    for (i=0; i<20; i++) {
      txbuf[i] = 255; S = S + txbuf[i]; SP = SP + txbuf[i]*(i+1);
      sprintf(temp,"%02X",txbuf[i]);
      printString("|");
      printString(temp); 
      }
    txbuf[20] = S & 0x00FF; txbuf[21] = (S & 0xFF00) >> 8;
    sprintf(temp,"%02X",txbuf[20]);
      printString("|");
      printString(temp);
      sprintf(temp,"%02X",txbuf[21]);
      printString("|");
      printString(temp); 
    
    txbuf[22] = SP & 0x00FF; txbuf[23] = (SP & 0xFF00) >> 8;
    sprintf(temp,"%02X",txbuf[22]);
      printString("|");
      printString(temp);
      sprintf(temp,"%02X",txbuf[23]);
      printString("|");
      printString(temp); 
    
    rf95.send(txbuf, 24);     //émission
    rf95.waitPacketSent() ;   // attente fin d’émission

    digitalWrite(13, LOW);    // pour aider Aa débugger

    state = DELAI;        // transition vers état suivant
    printString("\r");
    break;

  case DELAI: // code source de l’état DELAI
    printString("DELAI\r") ;
    delay (1000) ;        // 1s avant d’émettre la trame suivante
    state = EMISSION;       // transition vers état suivant
    break;

  default:
    break;
  }
}

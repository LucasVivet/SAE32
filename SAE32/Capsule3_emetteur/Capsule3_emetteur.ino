#include <RH_RF95.h>
#include <M5Stack.h>

#define RFM95_CS 5 // M5-Stack LoRa
#define RFM95_DIO0 36
RH_RF95 rf95 (RFM95_CS, RFM95_DIO0); //si nouvelle librairie RadioHead

uint8_t rxbuf[RH_RF95_MAX_MESSAGE_LEN], rxbuflen = RH_RF95_MAX_MESSAGE_LEN;   // buffer de trame en réception
uint8_t txbuf[RH_RF95_MAX_MESSAGE_LEN], txbuflen = RH_RF95_MAX_MESSAGE_LEN; ;     // buffer de trame en émission
uint8_t rxlen = RH_RF95_MAX_MESSAGE_LEN,state,RxSeq,TxSeq, credit;
uint32_t attente;
uint8_t FCS; //champ de contrôle d'un octet
int i; //index
char temp[255];

// déclaration des noms et numéros de chaque état
#define E0 0
#define E1 1
#define E2 2
#define E3 3
#define E4 4
#define E5 5

#define canal 1 //déclaration du canal de transmission
#define  TYPE_DATA 1
#define TYPE_ACK 2
#define TIMEOUT_ACK 40

void setup() {
 
  M5.begin() ;  // Initialisation port console pour debug
  M5.Power.begin();
  termInit();

  if (!rf95.init())
    M5.lcd.println("RF95 init erreur") ;
  else {
    M5.lcd.println("RF95 init OK");
  }
   state = E0;
   delay(3000);
   M5.lcd.println("Boucle principale");
   TxSeq = 0;
   credit = 5;
   // configuration de la fréquence radio
  rf95.setTxPower(10);
  rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128 );
  rf95.setFrequency(867.4);
}

void loop() {
  switch (state) {
    case E0:
      printString("--------------------------------------------\r");
      sprintf(temp,"%d",TxSeq);
      printString("Test EMISSION  numero : ");
      printString(temp);
      printString("\r");
      txbuf[0] = TYPE_DATA; txbuf[1] = TxSeq;
      for (i = 2;i<20;i++){
        txbuf[i] = 255; //18 octets à 255 de payload (max)
      }
      //calcul du FCS : XOR
      FCS =0; //champ de contrôle d'un octet
      for (i = 0;i<21;i++){
      sprintf(temp,"%02X",txbuf[i]);
      printString("|");
      printString(temp);
      }
      printString("|");
      rf95.send(txbuf, 21); //emission
      rf95.waitPacketSent();
      credit--; //on vient d'émettre une fois de + la trame
      delay(3000);
      state = E1;
      break;

    case E1:
      attente = millis()+ TIMEOUT_ACK; //armement du chien de garde
      state = E2;
      break;

    case E2:
      rf95.setModeRx(); //mettre la radio en mode réception pour l'ACK
      state = E3;
      break;

    case E3:
      if (millis() > attente)
        state = E5;
      else {
        if (rf95.recv(rxbuf, &rxlen)) { //si la trame reçue est ACK et même numéro que DATA émise et trame juste
          state = E4;
        }
        else state = E2; //sinon on retourne à l'état E2
        if ((rxbuf[0]==TYPE_ACK)&&(rxbuf[1]==TxSeq)&&((rxbuf[0]^rxbuf[1])!=rxbuf[2])) { //si la trame reçue est de type ACK et même numéro mais trame erronée
          state = E5; //si oui on affiche échec
        }
      } break;
    case E4:
      printString("ACK_RECU");
      state = E0; TxSeq++; credit = 5; //trame suivante
      break;

    case E5: //si le watchdog expire sans réception d'ACK, ECHEC si crédit épuisé
      if (credit ==0) { //5 tentatives passées
        printString("\r");
        printString("ECHEC\r");
        state = E0;
        credit = 5; TxSeq++; //trame suivante
        break;
      }
      else {
        sprintf(temp,"%d",5-credit);
        printString("Nouvelle tentative numero ");
        printString(temp);
        printString("\r");
        state = E0;
        break;
      }

     defaut:
      state = E0;
      break;
  }
}

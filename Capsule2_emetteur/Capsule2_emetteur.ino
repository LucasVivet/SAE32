#include <RH_RF95.h>
#include <M5Stack.h>

#define RFM95_CS 5 // M5-Stack LoRa
#define RFM95_DIO0 36
RH_RF95 rf95 (RFM95_CS, RFM95_DIO0); //si nouvelle librairie RadioHead

uint8_t rxbuf[RH_RF95_MAX_MESSAGE_LEN], rxbuflen = RH_RF95_MAX_MESSAGE_LEN;   // buffer de trame en réception
uint8_t txbuf[RH_RF95_MAX_MESSAGE_LEN], txbuflen = RH_RF95_MAX_MESSAGE_LEN; ;     // buffer de trame en émission
uint8_t rxlen = RH_RF95_MAX_MESSAGE_LEN;
uint8_t state,RxSeq, TxSeq, credit;
uint32_t attente;
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
#define TIMEOUT_ACK 5

void setup(){
  M5.begin() ;  // Initialisation port console pour debug
  M5.Power.begin();

  termInit();

  if (!rf95.init())
    M5.lcd.println("RF95 init erreur") ;
  else {
    M5.lcd.println("RF95 initialisation OK");
  }

    // configuration de la fréquence radio
  rf95.setTxPower(10);
  rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128 );
  rf95.setFrequency(867.4);

  state = E0;
  delay(3000);

  TxSeq = 1;
  credit = 5;

}

void loop(){
  switch (state){
    case E0:
      printString("--------------------------------------------\r");
      sprintf(temp,"%d",TxSeq);
      printString("Test EMISSION  numero : ");
      printString(temp);
      printString("\r");
      txbuf[0] = TYPE_DATA; //type de message
      txbuf[1] = TxSeq; //numéro de trame DATA émise
      txbuf[2] = 0x0AA; //infos
      txbuf[3] = 0x55; //..
      rf95.send(txbuf, 4); //envoie de données (une trame de 4 octets)
      rf95.waitPacketSent();

      credit--; //on vient d'émettre une fois de + la trame

      state = E1;
      break;
      
     case E1:
      attente = millis()+TIMEOUT_ACK; //armement du chien de garde
      state = E2;
      break;

     case E2:
      rf95.setModeRx(); //mettre la radio en mode réception pour l'ACK
      state = E3;
      break;

     case E3: //attente d'ACK non bloquante (car test trame recue durant le CdG)
      if (millis() > attente) { //vérification du le WatchDog est expiré
        state = E5;
      }
      else {
        if (rf95.recv(rxbuf, &rxlen)) { //vérifier si une trame est disponible
          if ((rxbuf[0] == TYPE_ACK) && (rxbuf[1]) == TxSeq) { //si la trame recue est de type ACK et même numéro que DATA émise
            state = E4; //si oui on affiche la trame reçue
          }
          else state = E2; //sinon on retourne à l'état E2
        }
      }
      break;

      case E4:
        printString("Réponse(ACK_RECU)\r");
        state = E0;
        TxSeq++; 
        credit = 5; //trame suivante
        break;

      case E5: //si le CdG expire sans réception d'ACK, ECHEC si crédit épuisé
        if (credit == 0) { //5 tentatives passées
          printString("ECHEC de la réception\r");
          credit = 5; 
          TxSeq++; //trame suivante
          state = E0;
          break;
        }
        else {
          sprintf(temp,"%d",5-credit);
          printString("Nouvelle tentative  numero : ");
          printString(temp);
          printString("\r");
          state = E0;
          delay(3000);
          break;
        }
        
        default:
          state = E0;
          break;
  }
}

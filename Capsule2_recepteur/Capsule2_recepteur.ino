#include <RH_RF95.h>
#include <M5Stack.h>

#define RFM95_CS 5 // M5-Stack LoRa
#define RFM95_DIO0 36
RH_RF95 rf95 (RFM95_CS, RFM95_DIO0); //si nouvelle librairie RadioHead

uint8_t rxbuf[RH_RF95_MAX_MESSAGE_LEN], rxbuflen = RH_RF95_MAX_MESSAGE_LEN;   // buffer de trame en réception
uint8_t txbuf[RH_RF95_MAX_MESSAGE_LEN], txbuflen = RH_RF95_MAX_MESSAGE_LEN; ;     // buffer de trame en émission
uint8_t rxlen = RH_RF95_MAX_MESSAGE_LEN;
uint8_t RxSeq, state;
uint32_t attente;
char temp[255];

// déclaration des noms et numéros de chaque état
#define E0 0
#define E1 1
#define E2 2
#define E3 3

#define canal 1 //déclaration du canal de transmission
#define  TYPE_DATA 1
#define TYPE_ACK 2


void setup() {
  
  M5.begin() ;  // Initialisation port console pour debug
  M5.Power.begin();
  termInit();

  if (!rf95.init())
    M5.lcd.println("RF95 init erreur") ;
  else {
    M5.lcd.println("RF95 init OK");
  }

    // configuration de la fréquence radio
  rf95.setTxPower(10);
  rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128 );
  rf95.setFrequency(867.4);

  state = E0;
  delay(3000);
   M5.lcd.println("Boucle principale");
   RxSeq = 255; //pas encore recu la 1ere trame n°0

}

void loop() {
  int j; //:index pour parcourir les octets de la trame recue à afficher
  switch (state){
    case E0 : //mettre en réception des données
      printString("--------------------------------------------\r");
      printString("ECOUTE\r");
      rf95.setModeRx();
      state = E1;
      break;

     case E1: //vérifier si un message recu est disponible
      rxlen = RH_RF95_MAX_MESSAGE_LEN; //taille max en réception
      if (rf95.recv(rxbuf, &rxlen)) {
        if (rxbuf[0]==TYPE_DATA) { //si le premier octet du message est de type DATA
          state = E2;
        }
        else {
          state = E0;
        }
      }
      break;

      case E2: // afficher la trame
        if (RxSeq != rxbuf[1]){ //si 1ere fois que l'on recoit cette trame
          RxSeq = rxbuf[1];

          sprintf(temp,"[%d]",RxSeq);
          printString(temp);
          printString("\r");
          sprintf(temp,"%d",rxlen);
          printString("DATA de ");
          printString(temp);
          printString("octets : \r");
          for (j=0; j<rxlen; j++) {
            sprintf(temp,"%02x",rxbuf[j]);
            printString(temp);
          }
          printString("\r");
          }
         else { //si on à déjà reçu cette trame
           printString("Duplication\r");
         }
         state = E3;
         break;

       case E3: //envoyer l'ACK 
         printString("Envoie de l'ACK\r");
         txbuf[0] = TYPE_ACK; //octet indiquant le type message ACK
         txbuf[1] = rxbuf[1]; //ACK de meme numéro que DATA
         rf95.send(txbuf, 2);
         rf95.waitPacketSent();

         state = E0;
         break; 
  }

}

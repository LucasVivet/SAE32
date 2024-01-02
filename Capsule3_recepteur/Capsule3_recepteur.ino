#include <RH_RF95.h>
#include <M5Stack.h>

#define RFM95_CS 5 // M5-Stack LoRa
#define RFM95_DIO0 36
RH_RF95 rf95 (RFM95_CS, RFM95_DIO0); //si nouvelle librairie RadioHead

uint8_t rxbuf[RH_RF95_MAX_MESSAGE_LEN], rxbuflen = RH_RF95_MAX_MESSAGE_LEN;   // buffer de trame en réception
uint8_t txbuf[RH_RF95_MAX_MESSAGE_LEN], txbuflen = RH_RF95_MAX_MESSAGE_LEN; ;     // buffer de trame en émission
uint8_t rxlen = RH_RF95_MAX_MESSAGE_LEN;
uint8_t RxSeq, state, credit;
uint32_t attente;
uint8_t FCSc, FCSr; //champ de controle d'un octet calculé et reçu
int i,j;
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
   state = E0;
   delay(3000);
   M5.lcd.println("Boucle principale");
   RxSeq = 255; //pas encore reçu 1ere trame n°0
   credit = 5;
  
   // configuration de la fréquence radio
  rf95.setTxPower(10);
  rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128 );
  rf95.setFrequency(867.4);
}

void loop() {
  switch (state) {
    case E0: //mettre en réception des données
      rf95.setModeRx();
      state = E1;
      break;

   case E1: //vérifier si un message reçu est disponible
   delay(3000);

    rxlen = RH_RF95_MAX_MESSAGE_LEN; //taille max en réception
    if (rf95.recv(rxbuf, &rxlen)){
      if (rxbuf[0]==TYPE_DATA) { //si le premier octete du message est de type DATA
        //test si FCS est juste
        FCSr = rxbuf[20]; //champ de controle reçu
        FCSc = 0; //calcul FCS
        for (i = 0; i<20;i++) FCSc=FCSc ^ rxbuf[i]; //code=XOR des 20 octets de payload
         if (FCSc == FCSr){ //même FCS calculé et reçu: trame juste
          state = E2;
         }
         else {
          state = E0; //trame fausse!
         }
      }
      else {
        state = E0;
      }
    }
    break;

    case E2: //afficher la trame
      if (RxSeq != rxbuf[1]){  //si 1ere fois que l'on recoit cette trame
       RxSeq = rxbuf[1];

       printString("--------------------------------------------\r");
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
      }
      else { //si on a déjà reçue cette même trame
        printString("\rDuplication");
      }
      state = E3;
      break;

    case E3: //envoyer ACK
      txbuf[0] = TYPE_ACK; //octet indiquant le type_message ACK
      txbuf[1] = rxbuf[1]; //ACK de même numéro que DATA
      txbuf[2] = txbuf[0] ^ txbuf[1]; //XOR
      rf95.send(txbuf, 3);
      rf95.waitPacketSent();

      state = E0;
      break;

   default:
    state = E0;
    break;
  }
}

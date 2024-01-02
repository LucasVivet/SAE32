#include <M5Stack.h>
#include <RH_RF95.h>

RH_RF95 rf95;

uint8_t rxbuf [RH_RF95_MAX_MESSAGE_LEN];
uint8_t rxlen = RH_RF95_MAX_MESSAGE_LEN;
uint8_t data[RH_RF95_MAX_MESSAGE_LEN];
uint8_t state;
int packetLen = 0;

#define RECEPTION 0
#define EMISSION 1

void setup ()
  {
    M5.begin() ;  // Initialisation port console pour debug
    M5.Power.begin();
    rf95.init();
    
    if (!rf95.init())
      M5.lcd.println("RF95 init erreur") ;
    else
      M5.lcd.println("RF95 init OK");
    
    // configuration de la fréquence radio
    rf95.setTxPower(10);
    rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128 );
    rf95.setFrequency(867.4);
    
    state = RECEPTION;   // état de départ
    
    delay(3000);
   }
    
void  loop() {
  switch (state) {

    case RECEPTION:
      rf95.setModeRx();
      if (rf95.available()){
        if (rf95.recv(rxbuf,&rxlen)){
          // Décodage de l'adresse multicast de 16 bits dans le paquet
          uint16_t multicastAddress;
          int i = 0;
          memcpy(&multicastAddress, &rxbuf[i], 2);
          i += 2;
          
          uint16_t nodesAddress = 0xFFFF;
          if (multicastAddress = nodesAddress){
            // Récupération des données restantes dans le paquet
            uint8_t payload[rxlen - i];
            memcpy(payload, &rxbuf[i], rxlen - i);
            M5.lcd.println("Voici les données recu : ");
            //M5.lcd.print(payload);
          }
        }
      }
      if (M5.BtnA.isPressed()){
        state = EMISSION;
      }
    break;
    
    
    case EMISSION:
    
    // MULTICAST_ADDRESS, sur 16 bits, est placé sur les deux premiers ([0]) octets du tableau data
    packetLen = 0;
    uint16_t MULTICAST_ADDRESS = 0xFFFF;
    memcpy(&data[packetLen], &MULTICAST_ADDRESS, 2);
    packetLen += 2;
    rf95.send(data, packetLen);
    rf95.waitPacketSent();
    M5.lcd.println("Envoi en cours...");
    delay(3000);
    state = RECEPTION;  // retour à l'état de réception
    break;
    }
}

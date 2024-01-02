#include <M5Stack.h>
#include <RH_RF95.h>

RH_RF95 rf95;

uint8_t rxbuf [RH_RF95_MAX_MESSAGE_LEN];
uint8_t rxlen = RH_RF95_MAX_MESSAGE_LEN;
uint8_t data[RH_RF95_MAX_MESSAGE_LEN];
uint8_t state;
int packetLen,ttl,num_seq = 0;
int ttl_2,num_seq_2;
char message[100];
uint8_t deja_routee[2];

uint16_t MULTICAST_ADDRESS,SOURCE_ADDRESS,routerAddress,multicastAddress,source_address;

#define TTL_MAX 7
#define RECEPTION 0
#define EMISSION 1
#define REEMISSION 2

void setup ()
  {
    //Démarrage du M5
    M5.begin() ;
    M5.Power.begin();
    rf95.init();

    //Test d'initialisation
    if (!rf95.init())
      M5.lcd.println("RF95 init erreur") ;
    else
      M5.lcd.println("RF95 init OK");
    
    // configuration de la fréquence radio
    rf95.setTxPower(10);
    rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128 );
    rf95.setFrequency(867.4);

    // état de départ
    state = RECEPTION;
    
    delay(3000);
   }
    
void  loop() {
  switch (state) {

    //Etat de réception
    case RECEPTION:
      while (true) {
      rf95.setModeRx(); //Initialisation en mode réception
      if (rf95.available()){ //Vérifiation de la disponibilité des données
        if (rf95.recv(rxbuf,&rxlen)){ //Lecture des données reçus
          
          // Décodage de l'adresse multicast de 16 bits dans le paquet
          multicastAddress;
          int i = 0;
          memcpy(&multicastAddress, &rxbuf[i], 2); //Enregistrement de l'adresse multicast dans les deux permiers octets
          i += 2;
          M5.lcd.println("Voici l'adresse multicast : ");
          //M5.lcd.print(multicastAddress);

          routerAddress = 0xFFFF; //Adresse du noeud
          if (multicastAddress = routerAddress){
            
            //Décrémentation du TTL 
            memcpy(&ttl_2, &rxbuf[i], 1); //Décodage du TTL
            i += 1;

            //Récupération de l'adresse source
            memcpy(&source_address, &rxbuf[i], 2);
            i += 2;
          
            //Récupération du numéro de séquence
            memcpy(&num_seq_2, &rxbuf[i], 1);
            i += 1;

            //Vérification du TTL, si < 7 alors réémission sinon retour en ecoute
            ttl_2+=1;
            if (ttl_2 < TTL_MAX) {
              if (source_address == deja_routee[0] && num_seq_2 == deja_routee[1]) { //Si le paquet a deja été routée on ne le réemet pas
                deja_routee[0] = source_address; //Si le paquet n'a pas deja été routée on enregistre l'adresse source
                deja_routee[1] = num_seq_2;
                state = REEMISSION;
              }
            }
          }
        }
      }
      if (M5.BtnA.isPressed()){ //Mise en état d'émission si le bouton A est pressé
        state = EMISSION;
      }
      delay(3000);
     }
    break;
    
    
    case EMISSION:

    // MULTICAST_ADDRESS, sur 16 bits, est placé sur les deux premiers ([0]) octets du tableau data
    MULTICAST_ADDRESS = 0xFFFF;
    packetLen = 0;
    memcpy(&data[packetLen], &MULTICAST_ADDRESS, 2);  //Copie les deux premiers octets dans la variable multicast address
    packetLen += 2;
    
    //Création du TTL
    memcpy(&data[packetLen], &ttl, 1);
    packetLen+= 1;

    //Création de l'adresse Source
    SOURCE_ADDRESS = 0x1234;
    memcpy(&data[packetLen], &SOURCE_ADDRESS, 2);
    packetLen+= 2;

    //Création du numéro de séquence
    num_seq+=1;
    memcpy(&data[packetLen], &num_seq, 1);
    packetLen+= 1;

    //Création du message à envoyer
    strcpy(message, "test");
    memcpy(&data[packetLen], &message, 6); //Enregistrement du message à envoyer dans le paquet
    packetLen+= 6;
    rf95.send(data, packetLen); //Envoie des données
    M5.lcd.println("Envoi en cours...");
    delay(3000);
    state = RECEPTION;  // retour à l'état de réception
    break;

    case REEMISSION:

     //Etat de réémission de la trame
     packetLen = 0;
     memcpy(&data[packetLen], &multicastAddress, 2);
     packetLen += 2;
     memcpy(&data[packetLen], &ttl_2, 1);
     packetLen+= 1;
     memcpy(&data[packetLen], &source_address, 2);
     packetLen += 2;
     memcpy(&data[packetLen], &num_seq, 1);
     packetLen+= 1;
     memcpy(&data[packetLen], &message, 6);
     packetLen+= 6;
     //Envoie de la trame
     rf95.send(data, packetLen); //Envoie des données
     M5.lcd.println("Envoi en cours...");
     delay(3000);
     state = RECEPTION;  // retour à l'état de réception
     break;
    }
}

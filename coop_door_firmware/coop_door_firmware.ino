/*
 *          
 * Les 4 entrées sont sur entrée Analogiques, afin de permettre de brancher des sondes
 *          les 2 premières entrées de la carte (connectées sur A0 et A1) possèdent une option pour permettre un pont diviseur avec la sonde :
 *          1ère entrée commandée par la sortie D3 : si niveau haut, alors la sonde branchée sur l'entrée est en "pull up"
 *          2eme entrée commandée par la sortie D8 : si niveau haut, alors la sonde branchée sur l'entrée est en "pull up"
 *          Ex : Sonde de température : a brancher en pull down et mettre une résistance de 10k en pull up                         
 *          Pour une utilisation en lecture numérique simple, prévoir d'activer la résistance de pull-up interne et passer la sortie pilote correspondante au niveua bas
 *          Les 2 autres entrées sont en lecture simple sur niveau bas en D8 et D9 (activer la résistance de pull-up)
 * 
 * Les 4 sorties sont commandées dans l'ordre par les sorties numérique D4, D5, D6 et D7
 * 
 * 
 * 
 */
// Pour que ce soit plus facilement identifiable dans le code, on va créer des
// alias pour les sorties du microcontrôleur qui commanderont les relais.
// on va déclarer les 3 relais de sortie :
// le premier sera la coupure électrique = ALIM
// Par convention (sans savoir si ce sera vrai), on dira que la fermeture = relais non alimentés
// et l'ouverture sera faite en alimentant les 2 relais.



#define ALIM 4      // Broche de commande de l'alimentation du moteur
#define RELAIS_1 5  // Broche de commande du relais 1 (sens du moteur)
#define RELAIS_2 6  // Broche de commande du relais 2 (sens opposé du moteur)

#define LDR A0                         // Broche analogique connectée à la photorésistance (capteur de lumière)
#define FDC A1                         // Fin de course (capteur indiquant que la porte est complètement fermée)
#define BUTTON_SEUIL A2                // Bouton permettant de mettre à jour le seuil de luminosité
#define BUTTON_OUVERTURE_FERMETURE A3  // Bouton d'ouverture/fermeture manuel


unsigned long previousMillis = 0;  // Variable utilisée pour mesurer des durées avec millis()
unsigned long currentMillis;       // Variable temporaire pour stocker le temps actuel

int NiveauLumiere;        // Valeur de luminosité de référence (seuil)
int TempsLevage = 8000;   // Durée d'activation du moteur (en ms) pour ouverture/fermeture
int TempsAttente = 10000;  // Pause après une action, pour éviter les répétitions immédiates

bool OUVERT = 1;  // État de la porte : 1 = ouverte, 0 = fermée


void setup() {
  // Configuration des broches en sortie
  pinMode(ALIM, OUTPUT);
  pinMode(RELAIS_1, OUTPUT);
  pinMode(RELAIS_2, OUTPUT);

  // Configuration des capteurs et boutons en entrée
  pinMode(LDR, INPUT);
  pinMode(FDC, INPUT);
  pinMode(BUTTON_SEUIL, INPUT);
  pinMode(BUTTON_OUVERTURE_FERMETURE, INPUT);

  // Lecture initiale de la luminosité pour définir un seuil
  NiveauLumiere = analogRead(LDR);
}


void loop() {

  if ((analogRead(LDR) > NiveauLumiere) && (OUVERT == 1)) {
    ferme();                    // Appelle la procédure de fermeture
    OUVERT = 0;                 // Met à jour l’état
    previousMillis = millis();  // Démarre la période d’attente

    while (millis() - previousMillis <= TempsAttente) {
      if (digitalRead(BUTTON_SEUIL) == LOW) {
        NiveauLumiere = analogRead(LDR);  // On peut mettre à jour le seuil pendant l’attente
      }
    }

  } else if ((analogRead(LDR) < NiveauLumiere - 10) && (OUVERT == 0)) {
    ouvre();
    OUVERT = 1;
    previousMillis = millis();
    while (millis() - previousMillis <= TempsAttente) {
      if (digitalRead(BUTTON_SEUIL) == LOW) {
        NiveauLumiere = analogRead(LDR);
      }
    }
  } else if ((digitalRead(BUTTON_OUVERTURE_FERMETURE) == LOW) && (OUVERT == 1)) {
    ferme();
    OUVERT = 0;
    previousMillis = millis();
    while (millis() - previousMillis <= TempsAttente) {
      if (digitalRead(BUTTON_SEUIL) == LOW) {
        NiveauLumiere = analogRead(LDR);
      }
    }
  } else if ((digitalRead(BUTTON_OUVERTURE_FERMETURE) == LOW) && (OUVERT == 0)) {
    ouvre();
    OUVERT = 1;
    previousMillis = millis();
    while (millis() - previousMillis <= TempsAttente) {
      if (digitalRead(BUTTON_SEUIL) == LOW) {
        NiveauLumiere = analogRead(LDR);
      }
    }
  }
}

// ----------------------------------------------------------
// PROCEDURE D'OUVERTURE DE LA PORTE
// void = ne renvoi aucune valeur (=procédure)
// les parenthèses vides indiquent qu'il n'y a pas de paramètre à donner à la procédure
// ----------------------------------------------------------

void ouvre() {
  digitalWrite(RELAIS_1, HIGH);  // Active les deux relais pour orienter le moteur dans le sens ouverture
  digitalWrite(RELAIS_2, HIGH);
  digitalWrite(ALIM, HIGH);  // Allume l’alimentation du moteur

  previousMillis = millis();  // Mémorise l’heure de départ

  // Maintient l’alim pendant TempsLevage ms
  while (millis() - previousMillis <= TempsLevage) {
    if (digitalRead(BUTTON_SEUIL) == LOW) {
      NiveauLumiere = analogRead(LDR);  // Permet de recalibrer pendant l’ouverture
    }
  }

  digitalWrite(ALIM, LOW);  // Coupe l’alimentation
}

void ferme() {
  digitalWrite(RELAIS_1, LOW);  // Inverse le sens du moteur
  digitalWrite(RELAIS_2, LOW);
  digitalWrite(ALIM, HIGH);  // Active le moteur

  previousMillis = millis();  // Mémorise le temps de départ

  // Boucle jusqu’à ce que le contact de fin de course soit détecté OU qu’un temps max soit dépassé
  while ((digitalRead(FDC) == HIGH) && (millis() - previousMillis <= TempsLevage + 30000)) {
    if (digitalRead(BUTTON_SEUIL) == LOW) {
      NiveauLumiere = analogRead(LDR);  // Mise à jour du seuil possible
    }
  }

  digitalWrite(ALIM, LOW);  // Coupe l’alimentation
}

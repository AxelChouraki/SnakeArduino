#include <Vector.h> // pour utiliser les vecteurs
#include <LiquidCrystal.h> //librairie pour utiliser un lcd
// include TFT and SPI libraries for the screen
#include <TFT.h>  
#include <SPI.h>

//Initialisation bluetooth
//bluetoothSerial


//pin de controle
int pinHaut = 9;
int pinDroite = 6;
int pinGauche = 7;
int pinBas = 8;

//variable de controle
int haut;
int droite;
int gauche;
int bas;

//pin de l'écran
#define cs   53
#define dc   49
#define rst  47
TFT TFTscreen = TFT(cs, dc, rst);  // création de l'instance de l'écran

const int hauteur = 42; // hauteur de l'écran
const int largeur = 53; // largeur de l'écran

//variable du jeu solo
int queue = 0;
char terrain[hauteur][largeur];
int posActuelle[2];
char queueHistorique[hauteur*largeur];
bool start = true;
bool gameOn = false;
bool initialisation = false;
bool death = false;


//variable du jeu multi
int queue1 = 0;
int posActuelle1[2];
char queueHistorique1[hauteur*largeur];
bool isHost;
bool launchBeginScreen = false;
bool solo = true;
char whoIsDead; //'h' pour host et 'c' pour client
bool hostPlay; //true pour que l'hote joue seul
bool hostUpdate; //true pour que l'hote update les données
bool updateFromHost;
char DernierDeplacement;

void setup() {
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps -->debug & control from computer

  randomSeed(analogRead(0));
  TFTscreen.begin();  // initialisation de la librairie
  TFTscreen.background(0,0,0);  // mets le fond en Noir (r,g,b)

  pinMode(pinHaut, INPUT);
  pinMode(pinGauche, INPUT);
  pinMode(pinDroite, INPUT);
  pinMode(pinBas, INPUT);
}

// TODO : MESURER LA DUREE DEXECUTION DE LA BOUCLE UNE FOIS LE CODE FINI(optimiser ou pas)
//TODO regarder décalage entre serveur et client
//Trouver meilleur lettre ou caractère pour l'affichage?
// ATTENTION : pour setCursor et begin cest colonne ligne et non ligne colonne

//##################################################################################################################################################################################
void loop() {
  if (start) {
      //écran d'accueil
      TFTscreen.background(0,0,0);
      TFTscreen.stroke(255, 255, 255);
      TFTscreen.setTextSize(2);  // taille de (int i) * 10 pixels
      TFTscreen.text("Air Snake", 25, 10);  // .text(text, xPos, yPos)
      //choix solo/duo
      TFTscreen.setTextSize(1);
      TFTscreen.text("Up to play solo", 6, 45);
      TFTscreen.text("Down to play duo", 6, 58);
      delay(2000);

      readSerial(&haut, &bas, &gauche, &droite);

    // SOLO GAME
      if(haut){
        initialisation = true;
        solo = true;
        gameOn = true;
        start = false;
      }

      else if(bas){
      solo = false;
      TFTscreen.background(0,0,0);
      TFTscreen.stroke(255, 255, 255);
      TFTscreen.setTextSize(2);
      TFTscreen.text("Air Snake", 25, 10);
      TFTscreen.setTextSize(1);
      TFTscreen.text("Up to host", 6, 45);
      TFTscreen.text("Down to join", 6, 58);

        if(haut){
          //Initialiser bluetooth module en master,avec le bon mode de connection et l'IP de la slave
          BluetoothWrite('c');
          isHost = true;
        }
        else if(bas){
          //Initialiser bluetooth module en slave
          isHost = false;
          if(BluetoothRead() == 'c'){
            launchBeginScreen = true;
          }
        }
      }

    if(launchBeginScreen){
      TFTscreen.background(0,0,0);
      if(isHost)
      {
        TFTscreen.text("Start : Leftside", 6, 5);
      }
      else{
      	TFTscreen.text("Start : Rightside", 6, 5);
      }
      delay(2000);
  
      TFTscreen.background(0,0,0);
      TFTscreen.text("Start in 3s", 6, 5);
      delay(1000);
  
      TFTscreen.background(0,0,0);
      TFTscreen.text("Start in 2s", 6, 5);
      delay(1000);
  
      TFTscreen.background(0,0,0);
      TFTscreen.text("Start in 1s", 6, 5);
      delay(1000);

      TFTscreen.background(0,0,0);
      TFTscreen.text("Start in 0s", 6, 5);
      delay(1000);
  
      initialisation = true;
      launchBeginScreen = false;
      gameOn = true;
    }
  }
  if(gameOn){ // boucle de 500ms
    hostPlay = true;
    Snake(solo, &queue, &queue1, terrain, posActuelle, queueHistorique, &start, &gameOn, &initialisation, &death);
    delay(125);
    hostPlay = false;

    hostUpdate = true;
    updateFromHost = true;
    Snake(solo, &queue, &queue1, terrain, posActuelle, queueHistorique, &start, &gameOn, &initialisation, &death);
    delay(125);
    hostUpdate = false;

    Snake(solo, &queue, &queue1, terrain, posActuelle, queueHistorique, &start, &gameOn, &initialisation, &death); //client joue car hostPlay and hostUpdate sont false
    delay(125);

    hostUpdate = true;
    updateFromHost = false;
    Snake(solo, &queue, &queue1, terrain, posActuelle, queueHistorique, &start, &gameOn, &initialisation, &death);
    delay(125);
    hostUpdate = false;
  }
  
} //fin void loop

//##################################################################################################################################################################################
void SetElement(char terrain[][53], int i, int j, char element, int r=255, int g=255, int b=255) {
  terrain[i][j] = element;
  if (element == ' ') {
    TFTscreen.stroke(0, 0, 0);
    TFTscreen.rect(3*j, 3*i, 3, 3);
    TFTscreen.point(3*j+1, 3*i+1);
  }
  else if (element == 'F') {
    TFTscreen.stroke(b, g, r);
    TFTscreen.circle(3*j+1, 3*i+1, 1);
  }
  else {
    TFTscreen.stroke(b, g, r);
    TFTscreen.rect(3*j, 3*i, 3, 3);
  }
}

//##################################################################################################################################################################################
void Snake(bool solo, int* queue, int* queue1, char terrain[][largeur], int posActuelle[2], char queueHistorique,
            bool* start, bool* gameOn, bool* initialisation, bool* death)
{
  if(*initialisation)
  {
    // la fonction SetElement a besoin d'un tableau rempli
      
    //Si on ne veut pas de bordures
    for(int i = 0; i < hauteur; i++)
    {
      for(int j = 0; j < largeur; j++)
      {
        terrain[i][j] = ' ';
      }
    }
    
    //Si on veut des bordures
/*
    for (int i = 0 ; i < hauteur; i++) {
      for (int j = 0; j < largeur ; j++) {
        if (i == 0 or j == 0 or i == hauteur - 1 or j == largeur - 1)
        {
          SetElement(terrain, i, j, 'o');        }
        else {
          SetElement(terrain, i, j, ' ');
        }
      }
    }
*/
  
    //initialisation des variables
    TFTscreen.background(0, 0, 0);
    if(solo)
    {
      SetElement(terrain, hauteur/2, largeur/2, 'U');
      posActuelle[0] = hauteur / 2;
      posActuelle[1] = largeur / 2;
      *queue = 0;
      memset(queueHistorique, ' ', *queue);
      *start = false;
      *death = false;
      *initialisation=false;
      Serial.print("nbr_pixels:");
      GenerationFruit(terrain);
      delay(500);
    }
    else
    {
      //init j1
      SetElement(terrain, hauteur/2, largeur/4, 'U');
      posActuelle[0] = hauteur / 2;
      posActuelle[1] = largeur / 4;
      *queue = 0;
      memset(queueHistorique, ' ', *queue);
      
      //init j2
      SetElement(terrain, hauteur/2, largeur*3/4, 'U');
      posActuelle1[0] = hauteur / 2;
      posActuelle1[1] = largeur * 3 / 4;
      *queue1 = 0;
      memset(queueHistorique1, ' ', *queue1);

      //init gameInfo
      *start = false;
      *death = false;
      *initialisation=false;
      GenerationFruit(terrain);
      delay(500);
    }
  }


  // Obtention de la direction :
  
  // haut = digitalRead(pinHaut);
  // bas = digitalRead(pinBas);
  // droite = digitalRead(pinDroite);
  // gauche = digitalRead(pinGauche);

  // Depuis un ordinateur, avec zqsd :
  readSerial(&haut, &bas, &gauche, &droite);
  


  if (*death)
  {
    if(solo){
      TFTscreen.background(0,0,0);
      TFTscreen.text("Game Over", 6, 5);
      TFTscreen.text("UP : Recommencer", 6, 20);
  
      /* Si on a un plus grand écran
      SetElementLCD(terrain, 1, largeur / 3, "Game Over");
      SetElementLCD(terrain, 2, largeur / 2, "score = " + queue);
      SetElementLCD(terrain, 4, largeur / 3, "Press 'Up' to restart");
      SetElementLCD(terrain, 5, largeur / 3, "Press 'Down' to leave");
      delay(1000);
      */
    }
    
    else{
      TFTscreen.background(0,0,0);
      if(whoIsDead == 'h')
      {
        TFTscreen.text("Le client gagne", 6, 5);
      }
      else
      {
        TFTscreen.text("L'hôte gagne", 6, 5);
      }
      TFTscreen.text("UP : Recommencer", 6, 20);
    }

    if(haut)
    {
      *start = true;
      *death = false;
    }
  }

  if(solo or (isHost && hostPlay)){
    TourDeJeu(solo, isHost, haut, bas, gauche, droite, queue, terrain, posActuelle, queueHistorique);
  }
  
  else if(hostUpdate){
    if(updateFromHost) //il faut update le  client
    { 
      if(isHost == false)
      {
        char temp = BluetoothRead();

        if(temp == 'h'){TourDeJeu(solo, isHost, 1, 0, 0, 0, queue, terrain, posActuelle, queueHistorique);}
        else if(temp == 'b'){TourDeJeu(solo, isHost, 0, 1, 0, 0, queue, terrain, posActuelle, queueHistorique);}
        else if(temp == 'g'){TourDeJeu(solo, isHost, 0, 0, 1, 0, queue, terrain, posActuelle, queueHistorique);}
        else if(temp == 'd'){TourDeJeu(solo, isHost, 0, 0, 0, 1, queue, terrain, posActuelle, queueHistorique);}  
      }
      else //il faut update l'hote
      {
        char temp = BluetoothRead();
  
        if(temp == 'h'){TourDeJeu(solo, isHost, 1, 0, 0, 0, queue1, terrain, posActuelle1, queueHistorique1);}
        else if(temp == 'b'){TourDeJeu(solo, isHost, 0, 1, 0, 0, queue1, terrain, posActuelle1, queueHistorique1);}
        else if(temp == 'g'){TourDeJeu(solo, isHost, 0, 0, 1, 0, queue1, terrain, posActuelle1, queueHistorique1);}
        else if(temp == 'd'){TourDeJeu(solo, isHost, 0, 0, 0, 1, queue1, terrain, posActuelle1, queueHistorique1);}  
      }
    }  
  }

  else if(isHost == false && hostPlay)
  {
    return;
  } // l'hote ne fait rien si c'est au client de jouer

  else if(isHost ==false && hostPlay == false)
  {
    TourDeJeu(solo, isHost, haut, bas, gauche, droite, *queue1, terrain, posActuelle1, queueHistorique1);
  }
  
}//fin Snake

//##################################################################################################################################################################################
void TourDeJeu(bool solo, bool isHost, int haut, int bas, int gauche, int droite, int* queue, char terrain[][largeur], int posActuelle[], char queueHistorique[])
{
  //Serial.print(" queue historique: ");Serial.println(queueHistorique);
  //On assigne des priorités aux directions en cas de conflit càd si plusieurs boutons sont pressés en même temps : haut > droite > bas > gauche
  if (haut)
  {
    if ( terrain[posActuelle[0] - 1][posActuelle[1]] == 'u' or posActuelle[0] == 0)
    {
      death = true;
      if(isHost)
      {
        whoIsDead = 'h';
      }
      else
      {
        whoIsDead = 'c';
      }
    }
    else if (terrain[posActuelle[0] - 1][posActuelle[1]] == 'F')
    {
      Mange(terrain, posActuelle[0], posActuelle[1], posActuelle[0] - 1, posActuelle[1], queueHistorique, queue, 'h', posActuelle);
      queue++;
    }
    else
    {
      Deplacement(terrain, posActuelle[0], posActuelle[1], posActuelle[0] - 1, posActuelle[1], queueHistorique, queue, 'h', posActuelle);
    }
  
  }
  else if (droite)
  {
  
    if ( terrain[posActuelle[0]][posActuelle[1] + 1] == 'u' or posActuelle[1] + 1 == largeur)
    {
      death = true;
    }
    else if (terrain[posActuelle[0]][posActuelle[1] + 1] == 'F')
    {
      Mange(terrain, posActuelle[0], posActuelle[1], posActuelle[0], posActuelle[1] + 1, queueHistorique, queue, 'd', posActuelle);
      queue++;
    }
    else
    {
      Deplacement(terrain, posActuelle[0], posActuelle[1], posActuelle[0], posActuelle[1] + 1, queueHistorique, queue, 'd', posActuelle);
    }
  }
  else if (bas)
  {
  
  
    if ( terrain[posActuelle[0] + 1][posActuelle[1]] == 'u' or posActuelle[0] + 1 == hauteur)
    {
      death = true;
    }
    else if (terrain[posActuelle[0] + 1][posActuelle[1]] == 'F')
    {
      Mange(terrain, posActuelle[0], posActuelle[1], posActuelle[0] + 1, posActuelle[1], queueHistorique, queue, 'b', posActuelle);
      queue++;
    }
    else
    {
      Deplacement(terrain, posActuelle[0], posActuelle[1], posActuelle[0] + 1, posActuelle[1], queueHistorique, queue, 'b', posActuelle);
    }
  }
  else if (gauche)
  {
  
    if ( terrain[posActuelle[0]][posActuelle[1] - 1] == 'u' or posActuelle[1] == 0)
    {
      death = true;
    }
    else if (terrain[posActuelle[0]][posActuelle[1] - 1] == 'F')
    {
      Mange(terrain, posActuelle[0], posActuelle[1], posActuelle[0], posActuelle[1] - 1, queueHistorique, queue, 'g', posActuelle);
      queue++;
    }
    else
    {
      Deplacement(terrain, posActuelle[0], posActuelle[1], posActuelle[0], posActuelle[1] - 1, queueHistorique, queue, 'g', posActuelle);
    }
  }

  if(solo == false)
  {
    if (haut){DernierDeplacement = 'h';}
    if (bas){DernierDeplacement = 'b';}
    if (gauche){DernierDeplacement = 'g';}
    if (droite){DernierDeplacement = 'd';}
    
    BluetoothWrite(DernierDeplacement);
  }
}


//##################################################################################################################################################################################
void Deplacement(char terrainF[][53], int iAvant, int jAvant, int iApres, int jApres, char queueHistoriqueF[], int* queueF, char direction, int posActuelleF[]) {
  Serial.print("Deplacement :");Serial.print(direction);Serial.print(iAvant);Serial.print(jAvant);Serial.print(iApres);Serial.println(jApres);
  Serial.println(*queueF);
  //mise à jour de l'avant du serpent
  SetElement(terrain, iAvant, jAvant, 'u');
  SetElement(terrain, iApres, jApres, 'U');

  //mise à jour de l'arrière du serpent
  int iToDelete = iAvant;
  int jToDelete = jAvant;
  //Serial.print(queueF);

  if (*queueF > 0)
  {
    for (int i = 0; i < *queueF; i++)
    {
      //ATTENTION INVERSION CAR ON REMONTE A LENVERS (de la tete à la queue)
      switch (queueHistoriqueF[i]) {
        case 'h':
          iToDelete++;
          break;

        case 'b':
          iToDelete--;
          break;

        case 'd':
          jToDelete--;
          break;

        case 'g':
          jToDelete++;
          break;
      }
    }
  }

  SetElement(terrain, iToDelete, jToDelete, ' ');

  //Actualiser posActuelle + queueHistorique
  ActualiserPosition(posActuelleF, direction);
  ActualiserQueueHistorique(queueHistoriqueF, direction, false, queueF);
  
}

//##################################################################################################################################################################################
void Mange(char terrainF[][53], int iAvant, int jAvant, int iApres, int jApres, char queueHistoriqueF[], int* queueF, char direction, int posActuelleF[]) {
  SetElement(terrain, iAvant, jAvant, 'u');
  SetElement(terrain, iApres, jApres, 'U');
  ActualiserPosition(posActuelleF, direction);
  ActualiserQueueHistorique(queueHistoriqueF, direction, true, queueF);
  GenerationFruit(terrainF);
}

//##################################################################################################################################################################################
void ActualiserPosition(int posActuelleF[], char direction)
{
  switch (direction) {
    case 'h':
      posActuelleF[0]--;
      break;

    case 'b':
      posActuelleF[0]++;
      break;

    case 'd':
      posActuelleF[1]++;
      break;

    case 'g':
      posActuelleF[1]--;
      break;
  }
  
}

//##################################################################################################################################################################################
void ActualiserQueueHistorique(char queueHistoriqueF[], char direction, bool mange, int* queueF)
{
  if (mange)
  {
    *queueF +=1;
    for (int i = *queueF - 1; i > 0 ; i--)
    {
      queueHistoriqueF[i] = queueHistoriqueF[i-1];
    }
    queueHistoriqueF[0] = direction;
    Serial.print(" 0:");
    Serial.print(queueHistoriqueF[0]);
  }
  else
  {
    for (int i = *queueF - 1; i > 0 ; i--)
    {
      queueHistoriqueF[i] = queueHistoriqueF[i-1];
    }
    queueHistoriqueF[0] = direction;
  }
  Serial.print("Fin histo:");Serial.print(*queueF);Serial.print("-");
  Serial.println(queueHistoriqueF);
}

//##################################################################################################################################################################################
void GenerationFruit(char terrainF[][53])
{
  int i = random(hauteur);
  int j = random(largeur);
  while (terrainF[i][j] != ' ') {
    int i = random(hauteur);
    int j = random(largeur);
  }
  
  SetElement(terrain, i, j, 'F', 225, 0, 0);
  Serial.print("Fruit en :");Serial.print(i);Serial.print("x");Serial.println(j);

}

/*
void SetElementLCD(char terrainF[][53], int i1, int j2, char element) {
  terrain[i1][j2] = element;
  lcd.setCursor(j2, i1);
  lcd.print(terrainF[i1][j2]);
}
*/

/*
void SetElement(char terrain[][53], int i1, int j2, char element) {
  terrain[i1][j2] = element;
  
  int jlcd = j2;
  int ilcd = i1 / 2;
  lcd.setCursor(jlcd, ilcd);
  
  // On prend l'indice des deux lignes concernées par le changement d'affichage
  int iHaut;
  int iBas;
  
  if (ilcd == 0)
  {
    iHaut = 0;
    iBas = 1;
  }
  else
  {
    iHaut = 2;
    iBas = 3;
    }
    

  // On test toutes les combinaisons existantes et on écrit le cas correspondant
  if (terrain[iHaut][j2] == 'U' || terrain[iHaut][j2] == 'u')
  {
    if (terrain[iBas][j2] == 'u' || terrain[iBas][j2] == 'U') lcd.write(byte(0));
    else if (terrain[iBas][j2] == 'F') lcd.write(byte(1));
    else if (terrain[iBas][j2] == ' ') lcd.write(byte(2));
  }
  else if (terrain[iBas][j2] == 'U' || terrain[iBas][j2] == 'u')
  {
    if (terrain[iHaut][j2] == 'F') lcd.write(byte(3));
    else if (terrain[iHaut][j2] == ' ') lcd.write(byte(4));
  }
  else if (terrain[iHaut][j2] == 'F' && terrain[iBas][j2] == ' ') lcd.write(byte(5));
  else if (terrain[iHaut][j2] == ' ' && terrain[iBas][j2] == 'F') lcd.write(byte(6));
  else if (terrain[iHaut][j2] == ' ' && terrain[iBas][j2] == ' ') lcd.write(' ');
  else
  {
    lcd.write('x');
  }
}
*/


void BluetoothWrite(char Char)
{
  
}

char BluetoothRead()
{

  return ;
}

void readSerial(int* haut, int* bas, int* gauche, int* droite) {
  char c = 'x';
  if (Serial.available()) c = Serial.read();
  if (c =='z') {*haut = 1; *bas = 0; *gauche = 0; *droite = 0;}
  else if (c =='s') {*haut = 0; *bas = 1; *gauche = 0; *droite = 0;}
  else if (c =='q') {*haut = 0; *bas = 0; *gauche = 1; *droite = 0;}
  else if (c =='d') {*haut = 0; *bas = 0; *gauche = 0; *droite = 1;}
  else {*haut = 0; *bas = 0; *gauche = 0; *droite = 0;}
}

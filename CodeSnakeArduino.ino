// editeradczchdic
#include <Vector.h> // pour utiliser les vecteurs
#include <LiquidCrystal.h> //librairie pour utiliser un lcd

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

//pin du lcd
int pin1 = 5;
int pin2 = 4;
int pin3 = 3;
int pin4 = 2;
int rs = 12;
//int rw = 8;
int enable = 11;
LiquidCrystal lcd(rs, enable, pin1, pin2, pin3, pin4); // mettre tous les pins utilisés par le lcd

const int hauteur = 4; // hauteur de l'écran lcd
const int largeur = 16; // largeur de l'écran lcd

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

  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps -->debug
  lcd.begin(hauteur/2, largeur); //dimension du lcd 
  lcd.display(); // "allume" l'écran

  randomSeed(analogRead(0));

  pinMode(pinHaut, INPUT);
  pinMode(pinGauche, INPUT);
  pinMode(pinDroite, INPUT);
  pinMode(pinBas, INPUT);

  // Creation des figures avec liquidCrystal, appelable avec "byte(numeroDeFigure)", écrivable avec lcd.write seulement !!!
  // On code les 7 dispositions suivantes:
  // 0: 'u'  1: 'u'  2: 'u'  3:'F'  4:'_'  5:'F'  6:'_'
  //    'u'     'F'     '_'    'u'    'u'    '_'    'F'
  // Serpent : B01110, B01010, B01110, B00000
  // Fruit :   B10101, B01110, B01110, B00100
  // Vide :    B00000, B00000, B00000, B00000
  byte uu[8] = {B01110, B01010, B01110, B00000, B01110, B01010, B01110, B00000};
  lcd.createChar(0, uu); 
  byte uf[8] = {B01110, B01010, B01110, B00000, B10101, B01110, B01110, B00100};
  lcd.createChar(1, uf);
  byte u_[8] = {B01110, B01010, B01110, B00000, B00000, B00000, B00000, B00000};
  lcd.createChar(2, u_);
  byte fu[8] = {B10101, B01110, B01110, B00100, B01110, B01010, B01110, B00000};
  lcd.createChar(3, fu);
  byte _u[8] = {B00000, B00000, B00000, B00000, B01110, B01010, B01110, B00000};
  lcd.createChar(4, _u);
  byte f_[8] = {B10101, B01110, B01110, B00100, B00000, B00000, B00000, B00000};
  lcd.createChar(5, f_);
  byte _f[8] = {B00000, B00000, B00000, B00000, B10101, B01110, B01110, B00100};
  lcd.createChar(6, _f);
}

// TODO : MESURER LA DUREE DEXECUTION DE LA BOUCLE UNE FOIS LE CODE FINI(optimiser ou pas)
//TODO regarder décalage entre serveur et client
//Trouver meilleur lettre ou caractère pour l'affichage?
// ATTENTION : pour setCursor et begin cest colonne ligne et non ligne colonne

void loop() {
  
  if (start) {

      //écran d'accueil
      
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.write("Air Snake");
      delay(1000);

      //choix solo/duo

      lcd.clear();
      lcd.setCursor(0,0);
      lcd.write("Up to play solo");
      lcd.setCursor(1,0);
      lcd.write("Down to play duo");

    // SOLO GAME
      if(haut){
        initialisation = true;
        solo = true;
        gameOn = true;
      }

      else if(bas){
      solo = false;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.write("Up to host");
      lcd.setCursor(1,0);
      lcd.write("Down to join");

        if(haut){
          //Initialiser bluetooth module en master,avec le bon mode de connection et l'IP de la slave
          BluetoothWrite("connection");
          isHost = true;
        }
        else if(bas){
          //Initialiser bluetooth module en slave
          isHost = false;
          if(BluetoothRead() == "connection"){
            launchBeginScreen = true;
          }
        }
      }

    if(launchBeginScreen){
      lcd.clear();
      if(isHost)
      {
        lcd.setCursor(0,0);
        lcd.write("Start : Leftside");
      }
      else{
        lcd.setCursor(0,0);
        lcd.write("Start : Rightside");
      }
      delay(2000);
  
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.write("Start in 3s");
      delay(1000);
  
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.write("Start in 2s");
      delay(1000);
  
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.write("Start in 1s");
      delay(1000);
  
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.write("Start in 0s");
      delay(1000);
  
      initialisation = true;
      launchBeginScreen = false;
      gameOn = true;
    }
  }

  if(gameOn){ // boucle de 500ms
    hostPlay = true;
    Snake(solo);
    delay(125);
    hostPlay = false;

    hostUpdate = true;
    updateFromHost = true;
    Snake(solo);
    delay(125);
    hostUpdate = false;

    Snake(solo); //client joue car hostPlay and hostUpdate sont false
    delay(125);

    hostUpdate = true;
    updateFromHost = false;
    Snake(solo);
    delay(125);
    hostUpdate = false;
  }
  
} //fin void loop

void Snake(bool solo)
{
  if(initialisation)
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
    if(solo)
    {
      SetElement(terrain, hauteur/2, largeur/2, 'U');
      posActuelle[0] = hauteur / 2;
      posActuelle[1] = largeur / 2;
      queue = 0;
      memset(queueHistorique, ' ', queue);
      start = false;
      death = false;
      GenerationFruit(terrain);
      delay(500);  
    }
    else
    {
      //init j1
      SetElement(terrain, hauteur/2, largeur/4, 'U');
      posActuelle[0] = hauteur / 2;
      posActuelle[1] = largeur / 4;
      queue = 0;
      memset(queueHistorique, ' ', queue);
      
      //init j2
      SetElement(terrain, hauteur/2, largeur*3/4, 'U');
      posActuelle1[0] = hauteur / 2;
      posActuelle1[1] = largeur * 3 / 4;
      queue1 = 0;
      memset(queueHistorique1, ' ', queue1);

      //init gameInfo
      start = false;
      death = false;
      GenerationFruit(terrain);
      delay(500);
    }        
  }


  haut = digitalRead(pinHaut);
  bas = digitalRead(pinBas);
  droite = digitalRead(pinDroite);
  gauche = digitalRead(pinGauche);



  if (death)
  {
    if(solo){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Game Over");
      lcd.setCursor(0,1);
      lcd.print("UP : Recommencer");
  
      /* Si on a un plus grand écran
      SetElementLCD(terrain, 1, largeur / 3, "Game Over");
      SetElementLCD(terrain, 2, largeur / 2, "score = " + queue);
      SetElementLCD(terrain, 4, largeur / 3, "Press 'Up' to restart");
      SetElementLCD(terrain, 5, largeur / 3, "Press 'Down' to leave");
      delay(1000);
      */
    }
    
    else{
      lcd.clear();
      lcd.setCursor(0,0);
      if(whoIsDead == 'h')
      {
        lcd.print("Le client gagne");
      }
      else
      {
        lcd.print("L'hôte gagne");
      }
      lcd.setCursor(0,1);
      lcd.print("UP : Recommencer");
    }

    if(haut)
    {
      start = true;
      death = false;
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
    TourDeJeu(solo, isHost, haut, bas, gauche, droite, queue1, terrain, posActuelle1, queueHistorique1);
  }
  
}//fin Snake

void TourDeJeu(bool solo, bool isHost, int haut, int bas, int gauche, int droite, int queue, char terrain[][largeur], int posActuelle[], char queueHistorique[])
{
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


void Deplacement(char terrainF[][16], int iAvant, int jAvant, int iApres, int jApres, char queueHistoriqueF[], int queueF, char direction, int posActuelleF[]) {
  //mise à jour de l'avant du serpent
  SetElement(terrainF, iAvant, jAvant, 'u');
  SetElement(terrainF, iApres, jApres, 'U');

  //mise à jour de l'arrière du serpent
  int iToDelete = iAvant;
  int jToDelete = jAvant;
  Serial.print(queueF);

  if (queueF > 0)
  {
    for (int i = queueF - 1; i > -1; i--)
    {
      //ATTENTION INVERSION CAR ON REMONTE A LENVERS (de la tete à la queue)
      switch (queueHistoriqueF[i]) {
        case 'h':
          iToDelete--;
          break;

        case 'b':
          iToDelete++;
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
  ActualiserQueueHistorique(queueHistorique, direction, false, queueF);
  
}

void Mange(char terrainF[][16], int iAvant, int jAvant, int iApres, int jApres, char queueHistoriqueF[], int queueF, char direction, int posActuelleF[]) {
  SetElement(terrainF, iAvant, jAvant, 'u');
  SetElement(terrainF, iApres, jApres, 'U');
  ActualiserPosition(posActuelleF, direction);
  ActualiserQueueHistorique(queueHistorique, direction, true, queueF);
  GenerationFruit(terrainF);
}

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

void ActualiserQueueHistorique(char queueHistoriqueF[], char direction, bool mange, int queueF)
{
  if (mange)
  {
    queueHistoriqueF[queueF] = direction;
  }
  else
  {
    for (int i = 0; i < queue - 1; i++)
    {
      queueHistoriqueF[i+1] = queueHistoriqueF[i];
    }
    queueHistoriqueF[0] = direction;
  }
}

void GenerationFruit(char terrainF[][16])
{
  int caseLibreI[64];
  int count = 0;
  int caseLibreJ[64];
  

  for (int i = 0; i < hauteur; i++)
  {
    for (int j = 0; j < largeur; j++)
    {
      if (terrainF[i][j] == ' ')
      {
        caseLibreI[count] = i;
        caseLibreJ[count] = j;
        count++;
      }
    }
  }
  
  int r = random(count);

  SetElement(terrainF, caseLibreI[r], caseLibreJ[r], 'F');

}

void SetElementLCD(char terrainF[][16], int i1, int j2, char element) {
  terrain[i1][j2] = element;
  lcd.setCursor(j2, i1);
  lcd.print(terrainF[i1][j2]);
}

void SetElement(char terrain[][16], int i1, int j2, char element) {
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

void BluetoothWrite(char Char)
{
  
}

char BluetoothRead()
{

  return ;
}

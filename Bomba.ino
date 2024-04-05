//including libraries
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <arduino-timer.h>
//end of including

auto timer = timer_create_default(); //setting the timer

LiquidCrystal_I2C lcd(0x27, 2, 16); // settings of LCD

//Keypad settings
const byte row = 4;
const byte column = 4;

char key;
char allKeys[row][column] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte rowPins[row] = { 5, 6, 7, 8 };
byte columnPins[column] = { 9, 10, 11, 12 };

Keypad myKeypad = Keypad(makeKeymap(allKeys), rowPins, columnPins, row, column);
//End of keypad settings

//Definition of pins and buzzer tone
#define rLed 13
#define gLed 4
#define buzzerPin 3
#define ton 2050
//End of Definitions

//Definition of variables (Detailed information is given in Readme file)
bool bombSet = false, defused = false, isTimerStarted = false, exploded = false, isBuzzerActive = false;
int mins = 15, secs = 0, delayTime = 500, i = 0, page = 0, nextPage, previousPage;
char userMins[2], userSecs[2], pswd[8], trypswd[8];
//End of variable definitions

//Function of buzzer and red LED
void buzzer() {
  if (!isBuzzerActive) {
    tone(buzzerPin, ton);
    digitalWrite(rLed, HIGH);
    isBuzzerActive = true;
  } else {
    noTone(buzzerPin);
    digitalWrite(rLed, LOW);
    isBuzzerActive = false;
  }
} //End of the function

//Definition of timers (it's necessary for be able to cancel the timers)
auto taskBuzzer = timer.every(delayTime, buzzer);
auto bombTimer = timer.every(1000, bombUI);
//End of timer definitions

//Remaining time is been printing in bombUI function
bool bombUI() {
  if (secs == 0) {
    mins = (mins - 1) % 60; // 1 minute equals to 60 seconds so I've used % 60
    secs = 59;
  }
  secs = (secs - 1) % 60; //with usage of % 60, 'secs' has never been more than 60
  lcd.setCursor(0, 0);
  lcd.print("Patlamaya Kalan:");  //It means "Remaining Time to Explode"
  lcd.setCursor(0, 1);
  if (mins <= 10) {   //without this if condition, if mins < 10 it'll show on lcd like that 2:30
    lcd.print("0");   //But now, it's shown as 02:30
  }
  lcd.print(mins);
  lcd.print(":");
  if (secs <= 10) {   //It's used because of same description above.
    lcd.print("0");
  }
  lcd.print(secs);
  if (mins != 0 || secs != 0) {
    return true;    //It's used for timer function, if function returns true timer will call the function again, else it doesn't
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("BOMBA PATLADI!"); //It means "Bomb exploded"
    exploded = true;  //So 'exploded' variable has setted to 'true'
    return false;   //Also, function returns false, after that timer won't call it
  }
}

void setup() {
  lcd.begin();
  Serial.begin(9600);
  lcd.setCursor(0, 0);
  lcd.print("POROY"); 
  lcd.setCursor(8, 1);
  lcd.print("SOFTWARE");
  pinMode(rLed, OUTPUT);
  pinMode(gLed, OUTPUT);
  digitalWrite(rLed, LOW);
  digitalWrite(gLed, LOW);
  delay(500);
  timer.cancel(bombTimer);
  information();  //It's created for giving information to user. Detailed description is written to function.
}

void loop() {
  if ((bombSet && isTimerStarted) && (!exploded && !defused)) { //This condition statement is written for controlling the timers.
    delayTime = (mins * 60 + secs) * 10;   //Here 'delayTime' is calculated. It's using for setting frequency of the 'taskBuzzer' timer.
    if (delayTime >= 1000) {  //Also, here 'delayTime' variable is normalized. That is used for controlling the 'taskBuzzer' timer.
      delayTime = 1000;
    } else if (delayTime <= 10) { //If 'delayTime' is less than 10 milliseconds the buzzer sound will be just a noise. So, it's used for preventing that.
      delayTime = 10;
    }
    taskBuzzer = timer.every(delayTime, buzzer); //In every loop 'taskBuzzer' timer's 'delayTime' is updating because it's depended to remaining time to explode.
    key = myKeypad.getKey();  //'getKey()' function is using for reading the pressed key from keypad and, it's storing in 'key' variable.
    if (key && isDigit(key)) {  //If bomb set and a key is pressed, whether the key is a digit. This condition is activated. 
      trypswd[0] = key;   //If 'key' is a digit, pressed key will be stored in the 'trypswd' variable's 0. address.
      lcd.setCursor(0, 0);
      lcd.print(trypswd[0]);
      bombdefuse();   //Other keys storing and controlling processes are doing in 'bombdefuse()' function. Detailed descriptions are there is in the function.
    }
  } else if (exploded || defused) { //If the bomb's time is end or it's defused 'taskBuzzer' and 'bombTimer' timers are cancelled there.
    timer.cancel(taskBuzzer);
    timer.cancel(bombTimer);
  }
  timer.tick(); //For working correctly of the timers usage of this function is necessary.
}
void information() { //Information page for users.
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sonraki syfa '#'"); //It means 'for next page press '#''.
  lcd.setCursor(0, 1);
  lcd.print("Onceki syfa '*'"); //It means 'for previous page press '*''.
  lcd.setCursor(0, 0);
  delay(1500);  //This delay is used for users can easily read information.
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Onay/ileri '#'"); //It means 'for confirm/next press '#''.
  lcd.setCursor(0, 1);
  lcd.print("Iptal/geri '*'"); //It means 'for cancel/back press '*''.
  delay(1500);
  mainMenu();
}

void mainMenu() {
  digitalWrite(gLed, LOW); //After defusing the bomb green LED turns on. It's guaranteed the LED turned off.
  page = 0;   //This and next 2 lines are using for controlling the pages (for now they aren't necessary I've added them because I may need them in future)
  nextPage = 1;
  previousPage = 1;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Zaman ayari 'A'");   //This sentence means "For set the bomb timer press 'A'" It's default is 15 minutes.
  lcd.setCursor(0, 1);
  lcd.print("Bomba kurma 'B'");   //This sentence means "For set the bomb to exploding press 'B'"
  key = myKeypad.waitForKey();    //With 'waitForKey()' function, program waits for a key.
  switch (key) {
    case 'A':
      timeSet();  
      break;
    case 'B':
      setPswd();
      break;
    case 'C':
      printbombtime();
      break;
    case '#':
      if (nextPage == 1) {
        menuP2();
      } else {
        menuP2();
      }
      break;
    case '*':
      if (previousPage == 1) {
        menuP2();
      } else {
        menuP2();
      }
      break;
    default:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Hatali Tuslama...");   //It means "Wrong keystroke made"
      lcd.setCursor(0, 1);
      lcd.print("Menuye Donuluyor..."); //This means "Returning to main menu...."
      delay(2000);
      break;
  }
}

void menuP2() {
  page = 1;     //This and next 2 lines descriptions are same with at the 'mainMenu' function
  nextPage = 0;
  previousPage = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Kaytl Zmn 'C'"); //It means "For seeing the setted exploding time press 'C'".
  key = myKeypad.waitForKey();
  switch (key) {
    case 'A':
      timeSet();
      break;
    case 'B':
      setPswd();
      break;
    case 'C':
      printbombtime();
      break;
    case '#':
      if (nextPage == 0) {
        mainMenu();
      } else {
        mainMenu();
      }
      break;
    case '*':
      if (previousPage == 0) {
        mainMenu();
      } else {
        mainMenu();
      }
      break;
    default:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Hatali Tuslama...");
      lcd.setCursor(0, 1);
      lcd.print("Menuye Donuluyor...");
      delay(1000);
      break;
  } //Undescribed lines descriptions are same with in the 'mainMenu' function
}

void timeSet() {  //This function is using for getting and storing user's wanted to set exploding time.
  lcd.clear();
  lcd.home();
  lcd.print("Girisi onaylamak");  //This and 238. lines together means that "For confirming to input press '#'"
  lcd.setCursor(0, 1);
  lcd.print("Icin '#' basiniz"); //^^^
  delay(1500);
  lcd.clear();
  lcd.home();
  lcd.print("Iptal icin");  //This and 244. lines together means that "For cancelling press '*'"
  lcd.setCursor(0, 1);
  lcd.print("'*' basiniz");  //^^^
  delay(1500);
  lcd.clear();
  lcd.home();
  lcd.print("Dakika giriniz:"); //It means "please, input minute"
  lcd.setCursor(0, 1);
  for (i = 0; i <= strlen(userMins); i++) {
    userMins[i] = NULL;   //Here inside of the 'userMins' variable is set NULL
  }
  i = 0;
  while (i <= strlen(userMins)) {
    key = myKeypad.waitForKey();
    if (isDigit(key)) {   
      userMins[i] = key;    //If key is a digit it will be stored in 'userMins' variable as a char string
      lcd.print(userMins[i]);
      i++;
    } else if (key == '#' && atoi(userMins) != 0) {   //If last key is '#' and the 'userMins' value as an integer not zero
      mins = atoi(userMins);  //With the atoi function, datas inside the 'userMins' stored in 'mins' variable as integer. 
      break;
    } else if (key == '*') {  //If '*' pressed
      lcd.clear();
      lcd.home();
      lcd.print("Menuye Donuluyor");  //The 'Returning to main menu...' is printed on LCD
      delay(500);
      mainMenu();
      break;
    } else {
      lcd.clear();
      lcd.home();
      lcd.print("DAKIKA '0' OLAMAZ"); //If integer value of 'userMins' equals to zero the "Minute can't be 0" will be printed on LCD.
      delay(250);
      break;
    }
  }
  lcd.clear();
  lcd.home();
  lcd.print("Saniye giriniz:");   //Same processes for user seconds. It means "Please input seconds".
  lcd.setCursor(0, 1);
  for (i = 0; i <= strlen(userSecs); i++) {
    userSecs[i] = NULL;
  }
  i = 0;
  while (i <= strlen(userSecs)) {
    key = myKeypad.waitForKey();
    if (isDigit(key)) {
      userSecs[i] = key;
      lcd.print(userSecs[i]);
      i++;
    } else if (key == '#') {
      secs = atoi(userSecs);
      lcd.clear();
      lcd.home();
      lcd.print("Zaman kaydedildi");  //As a result of the process, "Time stored" is printed on the LCD
      delay(1000);
      mainMenu();
      break;
    }
  }
}

void setPswd() {  //It's used for setting the bomb's password
  lcd.clear();
  lcd.home();
  lcd.print("Sfre sadce rakam");  //This and 309. lines mean together "The password can only contain numbers and must be 8 characters"
  lcd.setCursor(0, 1);
  lcd.print("ve 8 krktr olmli");
  delay(2000);
  lcd.clear();
  lcd.home();
  lcd.print("Sifreyi giriniz:"); //It means "please enter the password"
  lcd.setCursor(0, 1);
  i = 0;
  while (i <= strlen(pswd)) {
    key = myKeypad.waitForKey();
    if (isDigit(key)) {
      pswd[i] = key;
      lcd.print(pswd[i]);
      i++;
    } else if (i == strlen(pswd)) {
      lcd.clear();
      lcd.home();
      lcd.print("Bomba kuruldu"); //This sentence means "Bomb is set"
      delay(750);
      bombSet = true;
      bombTimer = timer.every(1000, bombUI);
      bombUI();
      isTimerStarted = true;
      break;
    } else if (key == '*') {
      lcd.clear();
      lcd.home();
      lcd.print("Menuye donuluyor");
      delay(1000);
      for (i = 0; i <= strlen(pswd); i++) {
        pswd[i] = NULL;
      }
      mainMenu();
    } else {
      lcd.clear();
      lcd.home();
      lcd.print("Eksk/Hatali Sfre"); //Here writing "Missing/Incorrect password"
      lcd.setCursor(0, 1);
      lcd.print("Tekrar giriniz");  //It means "please, enter again".
      i = 0;
      delay(1000);
    }
  }
}

void printbombtime() { //It's using for showing the value of the bomb timer to user.
  lcd.clear();
  lcd.home();
  lcd.print("AYARLI ZAMAN:"); //This means "Stored time"
  lcd.setCursor(0, 1);
  if (mins <= 10) {
    lcd.print("0");
  }
  lcd.print(mins);
  lcd.print(":");
  if (secs <= 10) {
    lcd.print("0");
  }
  lcd.print(secs);
  myKeypad.waitForKey(); //before the returning to main menu a key waiting from user
  mainMenu();
}

void bombdefuse() { //It's continue of 'defusing the bomb' (the starting 'if statement' is in loop function)
  i = 1;  //The first char received and stored in loop, so here it's started from 1 and the second char
  while (i <= strlen(trypswd)) {
    key = myKeypad.getKey();
    if (isDigit(key)) {
      trypswd[i] = key;
      lcd.print(trypswd[i]);
      i++;
    }
  }
  if (strcmp(pswd, trypswd)) {  //Here, 'pswd'(it's received before the bomb set) and 'trypswd'(it's received after the bomb set) are comparing. If both are same bomb will be defused
    defused = true; 
    timer.cancel(taskBuzzer); 
    timer.cancel(bombTimer);
    lcd.clear();
    lcd.home();
    lcd.print("Bomba etkisiz"); //This and 389. lines mean together "Bomb defused"
    lcd.setCursor(0, 1);
    lcd.print("hale getirildi");
    digitalWrite(gLed, HIGH); //Between this line and 405. lines green led is blinking, at 406 it starts continuously on. 
    delay(10);
    digitalWrite(gLed, LOW);
    delay(200);
    digitalWrite(gLed, HIGH);
    delay(15);
    digitalWrite(gLed, LOW);
    delay(150);
    digitalWrite(gLed, HIGH);
    delay(25);
    digitalWrite(gLed, LOW);
    delay(100);
    digitalWrite(gLed, HIGH);
    delay(50);
    digitalWrite(gLed, LOW);
    delay(100);
    digitalWrite(gLed, HIGH);
    myKeypad.waitForKey();
    mainMenu();
  } else {
    lcd.clear();
    lcd.home();
    lcd.print("HATALI SIFRE");  //If "pswd" and "trypswd" not same "Incorrect password" is printed on LCD
  }
}
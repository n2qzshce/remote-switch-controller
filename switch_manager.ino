#define arr_len(x)(sizeof(x) / sizeof( * x))
const uint8_t SIGNAL_PINS[] = {A0, A1, A2, A3, A4, A5, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
bool is_receiver;
int NUM_PINS;
char startMarker = '<';
char endMarker = '>';

bool flashState;
char receivedChars[32];
int failedAttempts;
int bailThreshold = 10;

void setup() {
  NUM_PINS = arr_len(SIGNAL_PINS);
  failedAttempts = 0;
  flashState = true;
  
  pinMode(13, OUTPUT);
  pinMode(12, INPUT); 
  Serial.begin(9600);
}

void loop() {
  is_receiver = digitalRead(12);
  digitalWrite(13, is_receiver);

  if (is_receiver) {
    receiver_loop();
  } else {
    sender_loop();
  }
}  

void sender_loop() {
  
  for (int i = 0; i < NUM_PINS; i++) {
    pinMode(SIGNAL_PINS[i], OUTPUT);
    digitalWrite(SIGNAL_PINS[i], false);
  }

  delay(1);

  unsigned int pinValue;
  
  Serial.write(startMarker);
  for (int i = 0; i < NUM_PINS; i++) {
    pinMode(SIGNAL_PINS[i], INPUT);
    pinValue = digitalRead(SIGNAL_PINS[i]);
    Serial.write(pinValue);
  }
  Serial.write(endMarker);
}

void receiver_loop() {
  for (int i = 0; i < NUM_PINS; i++) {
    pinMode(SIGNAL_PINS[i], OUTPUT);
  }

  static boolean recvInProgress = false;
  static byte ndx = 0;
  char rc;
  unsigned int serialAvailable = Serial.available();  

  if(serialAvailable <= 0)
  {
    failedAttempts += 1;
    delay(10);
    if(failedAttempts > bailThreshold)
    {
      for (int i = 0; i < arr_len(SIGNAL_PINS); i++) {
        digitalWrite(SIGNAL_PINS[i], false);
      }

      digitalWrite(13, flashState);
      flashState = !flashState;
      delay(100);
    }
    return;
  }

  while (serialAvailable > 0) {
    failedAttempts = 0;
    rc = Serial.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= NUM_PINS) {
          ndx = NUM_PINS - 1;
        }
      } else {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
      }
    } else if (rc == startMarker) {
      recvInProgress = true;
      ndx = 0;
    }

    serialAvailable = Serial.available();
  }

  for (int i = 0; i < arr_len(SIGNAL_PINS); i++) {
    char x = receivedChars[i];
    bool pinValue = x;
    digitalWrite(SIGNAL_PINS[i], pinValue);
  }
}

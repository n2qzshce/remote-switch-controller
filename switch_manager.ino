#define arr_len(x)(sizeof(x) / sizeof( * x))
const uint8_t SIGNAL_PINS[] = {A0, A1, A2, A3, A4, A5, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
bool is_sender;
int NUM_PINS;
char startMarker = '<';
char endMarker = '>';

void setup() {
  NUM_PINS = arr_len(SIGNAL_PINS);
  
  pinMode(13, INPUT);
  pinMode(12, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  is_sender = !digitalRead(13);
  digitalWrite(12, is_sender);

  if (is_sender) {
    sender_loop();
  } else {
    receiver_loop();
  }
}

void sender_loop() {
  
  for (int i = 0; i < NUM_PINS; i++) {
    pinMode(SIGNAL_PINS[i], INPUT);
  }

  unsigned int pinValue;
  
  Serial.write(startMarker);
  for (int i = 0; i < NUM_PINS; i++) {
    pinValue = digitalRead(SIGNAL_PINS[i]);
    Serial.write(pinValue);
  }
  Serial.write(endMarker);
}

char receivedChars[32];

void receiver_loop() {
  for (int i = 0; i < NUM_PINS; i++) {
    pinMode(SIGNAL_PINS[i], OUTPUT);
  }

  static boolean recvInProgress = false;
  static byte ndx = 0;
  char rc;

  while (Serial.available() > 0) {
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
    }
  }

  for (int i = 0; i < arr_len(SIGNAL_PINS); i++) {
    char x = receivedChars[i];
    bool pinValue = x;
    digitalWrite(SIGNAL_PINS[i], pinValue);
  }
}

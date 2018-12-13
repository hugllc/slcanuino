#include <Arduino.h>
#include <mcp_can.h>

#define LED_OPEN 7
#define LED_ERR 8
#define CMD_LEN (sizeof("T12345678811223344556677881234\r")+1)
#define RTR_BIT 0x40000000
#define EXT_BIT 0x80000000


int g_can_speed = CAN_125KBPS; // default: 500k
int g_ts_en = 10;

MCP_CAN Canbus(g_ts_en);


// the setup function runs once when you press reset or power the board
void setup() {
  pinMode(LED_OPEN, OUTPUT);
  pinMode(LED_ERR, OUTPUT);
  Serial.begin(1000000); // select from 115200,500000,1000000
  if (Canbus.begin(MCP_ANY, g_can_speed, MCP_8MHZ) == CAN_OK) {
    digitalWrite(LED_ERR, LOW);
    Canbus.setMode(MCP_NORMAL);
  } else {
    digitalWrite(LED_ERR, HIGH);
  }
}

// transfer messages from CAN bus to host
void xfer_can2tty()
{
  uint8_t data[8];
  uint32_t id = 0;
  uint8_t length = 0;
  char buf[CMD_LEN];
  uint16_t i;
  static uint16_t ts = 0;
  char *p;

  while (Canbus.readMsgBuf(&id, &length, data) == CAN_OK) {
    p = buf;
    if ((id & EXT_BIT) == EXT_BIT) {
      if ((id & RTR_BIT) == RTR_BIT) {
        sprintf(p, "R%08lX%01d", id, length);
      } else {
        sprintf(p, "T%08lX%01d", id, length);
      }
      p += 10;
    } else {
      if ((id & RTR_BIT) == RTR_BIT) {
        sprintf(p, "r%03X%01X", (unsigned int)id, length);
      } else {
        sprintf(p, "t%03X%01X", (unsigned int)id, length);
      }
      p += 5;
    }
    for (i = 0; i < length; i++) {
      sprintf(p + (i * 2), "%02X", data[i]);
    }
    p += i * 2;

    // insert timestamp if needed
    if (g_ts_en) {
      sprintf(p, "%04X", ts++); // up to 60,000ms
      p += 4;
    }

    *(p++) = '\r';
    *(p++) = '\0';
    Serial.print(buf);
  }
}

void slcan_ack()
{
  Serial.write('\r'); // ACK
}

void slcan_nack()
{
  Serial.write('\a'); // NACK
}

void send_canmsg(char *buf)
{
  uint8_t data[8];
  uint32_t id;
  uint8_t length;
  uint16_t len = strlen(buf) - 1;
  uint32_t val;
  unsigned int val16;
  int is_eff = buf[0] & 0x20 ? 0 : 1;
  int is_rtr = buf[0] & 0x02 ? 1 : 0;

  if (!is_eff && len >= 4) { // SFF
    sscanf(&buf[1], "%03x", &val16);
    id = val16;
    if (is_rtr) {
      id |= RTR_BIT;
    }
    sscanf(&buf[4], "%01x", &val16);
    length = val16;
    if (len - 4 - 1 == length * 2) {
      for (uint8_t i = 0; (i < length) && (i < sizeof(data)); i++) {
        sscanf(&buf[5 + (i * 2)], "%02x", &val16);
        data[i] = val16;
      }
    }
    Canbus.sendMsgBuf(id, length, data);

  } else if (is_eff && len >= 9) { // EFF
    sscanf(&buf[1], "%08lx", &val);
    id = val | EXT_BIT;
    if (is_rtr) {
      id |= RTR_BIT;
    }
    sscanf(&buf[9], "%01x", &val16);
    length = val16;
    if (len - 9 - 1 == length * 2) {
      for (int i = 0; i < length; i++) {
        sscanf(&buf[10 + (i * 2)], "%02x", &val16);
        data[i] = val16;
      }
    }
    Canbus.sendMsgBuf(id, length, data);
  }
}

void pars_slcancmd(char *buf)
{
  switch (buf[0]) {
    // common commands
    case 'O': // open channel
      digitalWrite(LED_OPEN, HIGH);
      if (Canbus.begin(MCP_ANY, g_can_speed, MCP_8MHZ) == CAN_OK) {
        digitalWrite(LED_ERR, LOW);
        Canbus.setMode(MCP_NORMAL);
      } else {
        digitalWrite(LED_ERR, HIGH);
      }
      slcan_ack();
      break;
    case 'C': // close channel
      digitalWrite(LED_OPEN, LOW);
      digitalWrite(LED_ERR, LOW);
      slcan_ack();
      break;
    case 't': // SFF
    case 'T': // EFF
    case 'r': // RTR/SFF
    case 'R': // RTR/EFF
      send_canmsg(buf);
      slcan_ack();
      break;
    case 'Z': // turn timestamp on/off
      if (buf[1] == '0') {
        g_ts_en = 0;
      } else if (buf[1] == '1') {
        g_ts_en = 1;
      } else {
        slcan_nack();
      }
      slcan_ack();
      break;
    case 'M': // acceptance mask
      slcan_ack();
      break;
    case 'm': // acceptance value
      slcan_ack();
      break;

    // non-standard commands
    case 'S': // setup CAN bit-rates
      switch (buf[1]) {
        case '0': // 10k
        case '1': // 20k
        case '2': // 50k
          slcan_nack();
          break;
        case '3': // 100k
          g_can_speed = CAN_100KBPS;
          slcan_ack();
          break;
        case '4': // 125k
          g_can_speed = CAN_125KBPS;
          slcan_ack();
          break;
        case '5': // 250k
          g_can_speed = CAN_250KBPS;
          slcan_ack();
          break;
        case '6': // 500k
          g_can_speed = CAN_500KBPS;
          slcan_ack();
          break;
        case '7': // 800k
          slcan_nack();
          break;
        case '8': // 1000k
          g_can_speed = CAN_1000KBPS;
          slcan_ack();
          break;
        default:
          slcan_nack();
          break;
      }
      break;
    case 's': // directly set bitrate register of mcp2515
      slcan_nack();
      break;
    case 'F': // status flag
      Serial.print("F12");
      slcan_ack();
      break;
    case 'V': // hw/sw version
      Serial.print("V1234");
      slcan_ack();
      break;
    case 'N': // serial number
      Serial.print("N1234");
      slcan_ack();
      break;
    default: // unknown command
      slcan_nack();
      break;
  }
}

// transfer messages from host to CAN bus
void xfer_tty2can()
{
  int length;
  static char cmdbuf[CMD_LEN];
  static int cmdidx = 0;

  if ((length = Serial.available()) > 0) {
    for (int i = 0; i < length; i++) {
      char val = Serial.read();
      cmdbuf[cmdidx++] = val;

      if (cmdidx == CMD_LEN) { // command is too long
        slcan_nack();
        cmdidx = 0;
      } else if (val == '\r') { // end of command
        cmdbuf[cmdidx] = '\0';
        pars_slcancmd(cmdbuf);
        cmdidx = 0;
      }
    }
  }
}

// the loop function runs over and over again forever
void loop() {
  xfer_can2tty();
  xfer_tty2can();
}

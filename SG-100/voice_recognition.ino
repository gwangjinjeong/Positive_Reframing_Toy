#include <SoftwareSerial.h>
#include "VoiceRecognitionV3.h"
#include <Stepper.h>
#include <EEPROM.h>


VR myVR(2, 3);   // 2:RX 3:TX, you can choose your favourite pins.

/***************************************************************************/
/** declare print functions */
void printSeperator();
void printSignature(uint8_t *buf, int len);
void printVR(uint8_t *buf);
void printLoad(uint8_t *buf, uint8_t len);
void printTrain(uint8_t *buf, uint8_t len);
void printUserGroup(uint8_t *buf, int len);
void printCheckRecord(uint8_t *buf, int num);
void printCheckRecordAll(uint8_t *buf, int num);
void printSigTrain(uint8_t *buf, uint8_t len);
void printSystemSettings(uint8_t *buf, int len);
void printHelp(void);

/***************************************************************************/
// command analyze part
#define CMD_BUF_LEN      64+1
#define CMD_NUM     10
typedef int (*cmd_function_t)(int, int);
uint8_t cmd[CMD_BUF_LEN] = {108, 111, 97, 100, 32, 48, 32, 49, 32, 50, 32, 51, 32, 52, 32, 53, 32, 54, 10};
uint8_t cmd_cnt;
uint8_t *paraAddr;
int receiveCMD();
int checkCMD(int len);
int checkParaNum(int len);
int findPara(int len, int paraNum, uint8_t **addr);
int compareCMD(uint8_t *para1 , uint8_t *para2, int len);


int cmdLoad(int len, int paraNum);
int cmdSigTrain(int len, int paraNum);
int cmdHelp(int len, int paraNum);
int cmdClear(int len, int paraNum);
int eeprom_addr = 0;
/** cmdList, cmdLen, cmdFunction has correspondence */
const char cmdList[CMD_NUM][10] = {  // command list table

  {
    "load"
  }
  ,
  {
    "clear"
  }
  ,
  {
    "sigtrain"
  }

  ,
  {
    "help"
  }
  ,
};
const char cmdLen[CMD_NUM] = {   // command length
  4,  //  {"load"},
  5,  //  {"clear"},
  8,  //  {"sigtrain"},
  4,  //  {"help"}
};

cmd_function_t cmdFunction[CMD_NUM] = {    // command handle fuction(function pointer table)
  cmdLoad,
  cmdClear,
  cmdSigTrain,
  cmdHelp,
};

/***************************************************************************/
/** temprory data */
uint8_t buf[255];
uint8_t records[7]; // save record


// 2048:한바퀴(360도), 1024:반바퀴(180도)...
const int stepsPerRevolution = 1024;
// 모터 드라이브에 연결된 핀 IN4, IN2, IN3, IN1
Stepper myStepper(stepsPerRevolution, 11, 9, 10, 8);
int len, paraNum, paraLen, i, j;
int UP_count = EEPROM.read(0);

void setup(void)
{
  myVR.begin(9600);

  /** initialize */
  Serial.begin(115200);
  Serial.println(F("Elechouse Voice Recognition V3 Module \"train\" sample."));

  printSeperator();
  Serial.println(F("Usage:"));
  printSeperator();
  printHelp();
  printSeperator();
  cmd_cnt = 0;
  myStepper.setSpeed(19);

  for (int i = 0; i < 7; ++i)
  {
    cmd[0] = 108;
    cmd[1] = 111;
    cmd[2] = 97;
    cmd[3] = 100;
    cmd[4] = 32;
    cmd[5] = 48 + i;
    cmd[6] = 10;
    paraNum = checkParaNum(7);
    paraLen = findPara(7, 1, &paraAddr);
    cmdLoad(7, 2);
  }
  UP_count = 0;
  Serial.print("EEPROM : ");
  Serial.println(EEPROM.read(0));
}

void loop(void)
{

  int len, paraNum, paraLen, i, j;

  /** receive Serial command */
  len = receiveCMD();
  if (len > 0) {

    /** check if the received command is valid */
    if (!checkCMD(len)) {

      /** check parameter number of the received command  */
      paraNum = checkParaNum(len);

      /** display the receved command back */
      Serial.write(cmd, len);
      /** find the first parameter */

      paraLen = findPara(len, 1, &paraAddr);

      /** compare the received command with command in the list */
      for (i = 0; i < CMD_NUM; i++) {
        /** compare command length */
        if (paraLen == cmdLen[i]) {
          /** compare command content */
          if ( compareCMD(paraAddr, (uint8_t *)cmdList[i], paraLen) == 0 ) {
            /** call command function */
            //            Serial.print("cmdlen:");
            //            Serial.println(len);
            //            Serial.print("paraNum:");
            //            Serial.println(paraNum);
            for (j = 0; j < len; ++j)
            {
              Serial.println(cmd[j]);
            }
            if ( cmdFunction[i](len, paraNum) != 0) {
              printSeperator();
              Serial.println(F("Command Format Error!"));
              printSeperator();
            }
            break;
          }
        }
      }

      /** command is not supported*/
      if (i == CMD_NUM) {
        printSeperator();
        Serial.println(F("Unkonwn command"));
        printSeperator();
      }
    }
    else {
      /** received command is invalid */
      printSeperator();
      Serial.println(F("Command format error"));
      printSeperator();
    }
  }
  /** try to receive recognize result */
  int ret;
  String str = "";
  ret = myVR.recognize(buf, 50);
  if (ret > 0) {
    /** voice recognized, print result */
    for (int i = 0; i < buf[3]; ++i)
    {
      str += char(buf[4 + i]);
    }
    //    printVR(buf);
    //    Serial.print("STR : ");
    Serial.println(str);
    if (str.compareTo("UP") == 0)////////////////////////////////////한칸씩 위로
    {
      if (EEPROM.read(0) < 12)///////////////////////////////////////////////////////////////////////////////////////////////////여기가 최고
      {
        Serial.println("Motor UP");
        motorUP(1);
      }
    }
    else if (str.compareTo("DOWN") == 0)///////////////////////////////////////////////////////////여기가 끝까지 아래로
    {
      Serial.println("Motor DOWN");
      motorDOWN(UP_count);
    }
    else if (str.compareTo("DOWN1") == 0)
    {
      if (EEPROM.read(0) > 0)/////////////////////////////////////////////////////////////////////////////////////////////////////////////////여기가 한칸 아래로
      {
        Serial.println("Motor DOWN");
        motorDOWN1(UP_count);
      }
    }


  }
}

void motorUP(int count)
{


  for (int i = 0; i < count; ++i)
  {
    myStepper.step(-280);
    UP_count++;
    EEPROM.write(0, UP_count);
  }
  Serial.print("EEPROM : ");
  Serial.println(EEPROM.read(0));

}
void motorDOWN(int count)
{
  Serial.print("EEPROM : ");
  Serial.println(EEPROM.read(0));
  if (UP_count == 0)
  {
    myStepper.step(280);
  }
  for (int i = 0; i < EEPROM.read(0); ++i)
  {
    myStepper.step(280);
  }
  EEPROM.write(0, 0);
  UP_count = 0;

}

void motorDOWN1(int count)
{

  myStepper.step(280);
  int cnt = EEPROM.read(0);
  cnt--;
  EEPROM.write(0, cnt);
  Serial.print("EEPROM : ");
  Serial.println(EEPROM.read(0));

}

/**
   @brief   receive command from Serial.
   @param   NONE.
   @retval  command length, if no command receive return -1.
*/
int receiveCMD()
{
  int ret;
  int len;
  unsigned long start_millis;
  start_millis = millis();
  while (1) {
    ret = Serial.read();
    if (ret > 0) {
      start_millis = millis();
      cmd[cmd_cnt] = ret;
      if (cmd[cmd_cnt] == '\n') {
        len = cmd_cnt + 1;
        cmd_cnt = 0;
        return len;
      }
      cmd_cnt++;
      if (cmd_cnt == CMD_BUF_LEN) {
        cmd_cnt = 0;
        return -1;
      }
    }

    if (millis() - start_millis > 100) {
      cmd_cnt = 0;
      return -1;
    }
  }
}

/**
   @brief   compare two commands, case insensitive.
   @param   para1  -->  command buffer 1
   para2  -->  command buffer 2
   len    -->  buffer length
   @retval  0  --> equal
   -1  --> unequal
*/
int compareCMD(uint8_t *para1 , uint8_t *para2, int len)
{
  int i;
  uint8_t res;
  for (i = 0; i < len; i++) {
    res = para2[i] - para1[i];
    if (res != 0 && res != 0x20) {
      res = para1[i] - para2[i];
      if (res != 0 && res != 0x20) {
        return -1;
      }
    }
  }
  return 0;
}

/**
   @brief   Check command format.
   @param   len  -->  command length
   @retval  0  -->  command is valid
   -1  -->  command is invalid
*/
int checkCMD(int len)
{
  int i;
  for (i = 0; i < len; i++) {
    if (cmd[i] > 0x1F && cmd[i] < 0x7F) {

    }
    else if (cmd[i] == '\t' || cmd[i] == ' ' || cmd[i] == '\r' || cmd[i] == '\n') {

    }
    else {
      return -1;
    }
  }
  return 0;
}

/**
   @brief   Check the number of parameters in the command
   @param   len  -->  command length
   @retval  number of parameters
*/
int checkParaNum(int len)
{
  int cnt = 0, i;
  for (i = 0; i < len; ) {
    if (cmd[i] != '\t' && cmd[i] != ' ' && cmd[i] != '\r' && cmd[i] != '\n') {
      cnt++;
      while (cmd[i] != '\t' && cmd[i] != ' ' && cmd[i] != '\r' && cmd[i] != '\n') {
        i++;
      }
    }
    i++;
  }
  return cnt;
}

/**
   @brief   Find the specified parameter.
   @param   len       -->  command length
   paraIndex -->  parameter index
   addr      -->  return value. position of the parameter
   @retval  length of specified parameter
*/
int findPara(int len, int paraIndex, uint8_t **addr)
{
  int cnt = 0, i, paraLen;
  uint8_t dt;
  for (i = 0; i < len; ) {
    dt = cmd[i];
    if (dt != '\t' && dt != ' ') {
      cnt++;
      if (paraIndex == cnt) {
        *addr = cmd + i;
        paraLen = 0;
        while (cmd[i] != '\t' && cmd[i] != ' ' && cmd[i] != '\r' && cmd[i] != '\n') {
          i++;
          paraLen++;
        }
        return paraLen;
      }
      else {
        while (cmd[i] != '\t' && cmd[i] != ' ' && cmd[i] != '\r' && cmd[i] != '\n') {
          i++;
        }
      }
    }
    else {
      i++;
    }
  }
  return -1;
}

int cmdHelp(int len, int paraNum)
{
  if (paraNum != 1) {
    return -1;
  }
  printSeperator();
  printHelp();
  printSeperator();
  return 0;
}

/**
   @brief   Handle "load" command
   @param   len     --> command length
   paraNum --> number of parameters
   @retval  0 --> success
   -1 --> Command format error
*/
int cmdLoad(int len, int paraNum)
{
  int i, ret;
  if (paraNum < 2 || paraNum > 8 ) {
    return -1;
  }

  for (i = 2; i <= paraNum; i++) {
    findPara(len, i, &paraAddr);
    records[i - 2] = atoi((char *)paraAddr);
    if (records[i - 2] == 0 && *paraAddr != '0') {
      return -1;
    }
  }
  //  myVR.writehex(records, paraNum-1);
  ret = myVR.load(records, paraNum - 1, buf);
  printSeperator();
  if (ret >= 0) {
    printLoad(buf, ret);
  }
  else {
    Serial.println(F("Load failed or timeout."));
  }
  printSeperator();
  return 0;
}

/**
   @brief   Handle "clear" command
   @param   len     --> command length
   paraNum --> number of parameters
   @retval  0 --> success
   -1 --> Command format error
*/
int cmdClear(int len, int paraNum)
{
  if (paraNum != 1) {
    return -1;
  }
  if (myVR.clear() == 0) {
    printSeperator();
    Serial.println(F("Recognizer cleared."));
    printSeperator();
  }
  else {
    printSeperator();
    Serial.println(F("Clear recognizer failed or timeout."));
    printSeperator();
  }
  return 0;
}

/**
   @brief   Handle "sigtrain" command
   @param   len     --> command length
   paraNum --> number of parameters
   @retval  0 --> success
   -1 --> Command format error
*/
int cmdSigTrain(int len, int paraNum)
{
  int ret, sig_len;
  uint8_t *lastAddr;
  if (paraNum < 2) {
    return -1;
  }

  findPara(len, 2, &paraAddr);
  records[0] = atoi((char *)paraAddr);
  if (records[0] == 0 && *paraAddr != '0') {
    return -1;
  }

  findPara(len, 3, &paraAddr);
  sig_len = findPara(len, paraNum, &lastAddr);
  sig_len += ( (unsigned int)lastAddr - (unsigned int)paraAddr );

  printSeperator();
  ret = myVR.trainWithSignature(records[0], paraAddr, sig_len, buf);
  //  ret = myVR.trainWithSignature(records, paraNum-1);
  if (ret >= 0) {
    printSigTrain(buf, ret);
  }
  else {
    Serial.println(F("Train with signature failed or timeout."));
  }
  printSeperator();

  return 0;
}

/*****************************************************************************/
/**
   @brief   Print signature, if the character is invisible,
   print hexible value instead.
   @param   buf     --> command length
   len     --> number of parameters
*/
void printSignature(uint8_t *buf, int len)
{
  int i;
  for (i = 0; i < len; i++) {
    if (buf[i] > 0x19 && buf[i] < 0x7F) {
      Serial.write(buf[i]);
    }
    else {
      Serial.print(F("["));
      Serial.print(buf[i], HEX);
      Serial.print(F("]"));
    }
  }
}

/**
   @brief   Print signature, if the character is invisible,
   print hexible value instead.
   @param   buf  -->  VR module return value when voice is recognized.
   buf[0]  -->  Group mode(FF: None Group, 0x8n: User, 0x0n:System
   buf[1]  -->  number of record which is recognized.
   buf[2]  -->  Recognizer index(position) value of the recognized record.
   buf[3]  -->  Signature length
   buf[4]~buf[n] --> Signature
*/
void printVR(uint8_t *buf)
{
  Serial.println(F("VR Index\tGroup\tRecordNum\tSignature"));

  Serial.print(buf[2], DEC);
  Serial.print(F("\t\t"));

  if (buf[0] == 0xFF) {
    Serial.print(F("NONE"));
  }
  else if (buf[0] & 0x80) {
    Serial.print(F("UG "));
    Serial.print(buf[0] & (~0x80), DEC);
  }
  else {
    Serial.print(F("SG "));
    Serial.print(buf[0], DEC);
  }
  Serial.print(F("\t"));

  Serial.print(buf[1], DEC);
  Serial.print(F("\t\t"));
  if (buf[3] > 0) {
    printSignature(buf + 4, buf[3]);
  }
  else {
    Serial.print(F("NONE"));
  }
  Serial.println(F("\r\n"));
}

/**
   @brief   Print seperator. Print 80 '-'.
*/
void printSeperator()
{
  for (int i = 0; i < 80; i++) {
    Serial.write('-');
  }
  Serial.println();
}

/**
   @brief   Print "load" command return value.
   @param   buf  -->  "load" command return value
   buf[0]    -->  number of records which are load successfully.
   buf[2i+1]  -->  record number
   buf[2i+2]  -->  record load status.
   00 --> Loaded
   FC --> Record already in recognizer
   FD --> Recognizer full
   FE --> Record untrained
   FF --> Value out of range"
   (i = 0 ~ (len-1)/2 )
   len  -->  length of buf
*/
void printLoad(uint8_t *buf, uint8_t len)
{
  if (len == 0) {
    Serial.println(F("Load Successfully."));
    return;
  }
  else {
    Serial.print(F("Load success: "));
    Serial.println(buf[0], DEC);
  }
  for (int i = 0; i < len - 1; i += 2) {
    Serial.print(F("Record "));
    Serial.print(buf[i + 1], DEC);
    Serial.print(F("\t"));
    switch (buf[i + 2]) {
      case 0:
        Serial.println(F("Loaded"));
        break;
      case 0xFC:
        Serial.println(F("Record already in recognizer"));
        break;
      case 0xFD:
        Serial.println(F("Recognizer full"));
        break;
      case 0xFE:
        Serial.println(F("Record untrained"));
        break;
      case 0xFF:
        Serial.println(F("Value out of range"));
        break;
      default:
        Serial.println(F("Unknown status"));
        break;
    }
  }
}

/**
   @brief   Print "sigtrain" command return value.
   @param   buf  -->  "sigtrain" command return value
   buf[0]  -->  number of records which are trained successfully.
   buf[1]  -->  record number
   buf[2]  -->  record train status.
   00 --> Trained
   F0 --> Trained, signature truncate
   FE --> Train Time Out
   FF --> Value out of range"
   buf[3] ~ buf[len-1] --> Signature.
   len  -->  length of buf
*/
void printSigTrain(uint8_t *buf, uint8_t len)
{
  if (len == 0) {
    Serial.println(F("Train With Signature Finish."));
    return;
  }
  else {
    Serial.print(F("Success: "));
    Serial.println(buf[0], DEC);
  }
  Serial.print(F("Record "));
  Serial.print(buf[1], DEC);
  Serial.print(F("\t"));
  switch (buf[2]) {
    case 0:
      Serial.println(F("Trained"));
      break;
    case 0xF0:
      Serial.println(F("Trained, signature truncate"));
      break;
    case 0xFE:
      Serial.println(F("Train Time Out"));
      break;
    case 0xFF:
      Serial.println(F("Value out of range"));
      break;
    default:
      Serial.print(F("Unknown status "));
      Serial.println(buf[2], HEX);
      break;
  }
  Serial.print(F("SIG: "));
  Serial.write(buf + 3, len - 3);
  Serial.println();
}



void printHelp(void)
{
  Serial.println(F("COMMAND        FORMAT                        EXAMPLE                    Comment"));
  printSeperator();
  //  Serial.println(F("--------------------------------------------------------------------------------------------------------------"));
  Serial.println(F("load           load (r0) (r1) ...            load 0 51 2 3              Load records"));
  Serial.println(F("clear          clear                         clear                      remove all records in  Recognizer"));
  Serial.println(F("sigtrain       sigtrain (r) (sig)            sigtrain 0 ZERO            Train one record(r) with signature(sig)"));
  Serial.println(F("help           help                          help                       print this message"));
}


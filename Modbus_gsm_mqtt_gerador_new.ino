#include <ModbusMaster.h>
#include <ArduinoJson.h>
  
#define TINY_GSM_MODEM_SIM800
#define SerialMon Serial
#define SerialAT Serial1
#define TINY_GSM_DEBUG SerialMon
#define TINY_GSM_TEST_GSM_LOCATION false
#define TINY_GSM_POWERDOWN false


#define led 31
#define comap 26
#define deif  28

bool is_comap, is_deif;

String content = "";
char character;
String imei;
String gerador_id = "Gerador_";

#define GSM_PIN ""
// Your GPRS credentials, if any
const char apn_g[] = "generica.claro.com.br";
const char apn[] = "claro.com.br";
const char gprsUser[] = "claro";
const char gprsPass[] = "claro";

// MQTT details
const char* broker = "multiatomautomacao.ind.br";

char topicRec[40];
char topicP[40];
char topicStatus[40];

String topicRec_str = "g_mon_r/";
String topicP_str = "g_mon_p/";
String topicStatus_str = "g_mon_s/";


uint32_t result;

ModbusMaster node;

#include <TinyGsmClient.h>
#include <PubSubClient.h>


TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
PubSubClient mqtt(client);


uint32_t lastReconnectAttempt = 0;

void(* resetFunc) (void) = 0;//declare reset function at address 0

void mqttCallback(char* topic, byte* payload, unsigned int len) 
{
  SerialMon.print("Message arrived [");
  SerialMon.print(topic);
  SerialMon.print("]: ");
  SerialMon.write(payload, len);
  SerialMon.println();
  
  payload[len] = '\0';
  String strMensagem = String((char*)payload);
  
  if (String(topic) == topicRec)
  {
      if(strMensagem == "P1" )
      {
        StaticJsonBuffer<2000> jsonBuffer;
        JsonObject& root = jsonBuffer.createObject();
        JsonArray& Dados = root.createNestedArray("Dados_P1");
  
        Dados.add("Param1");  
        for (int i = 79; i < 209; i++)
        {
          Dados.add(le_h3(i));
        }
     
        char JSONmessageBuffer[2000];
  
        root.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  
        mqtt.publish(topicP, JSONmessageBuffer);
        
      }else if(strMensagem == "P2" )
      {
        StaticJsonBuffer<2000> jsonBuffer;
        JsonObject& root = jsonBuffer.createObject();
        JsonArray& Dados = root.createNestedArray("Dados_P2");
  
        Dados.add("Param2_3021_3120");  
        for (int i = 3021; i < 3121; i++)
        {
          Dados.add(le_h3(i));
        }
     
        char JSONmessageBuffer[2000];
  
        root.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  
        mqtt.publish(topicP, JSONmessageBuffer);
        
      }else if(strMensagem == "P3" )
      {
        StaticJsonBuffer<2000> jsonBuffer;
        JsonObject& root = jsonBuffer.createObject();
        JsonArray& Dados = root.createNestedArray("Dados_P3");
  
        Dados.add("Param3_3121_3239");  
        for (int i = 3121; i < 3239; i++)
        {
          Dados.add(le_h3(i));
        }
     
        char JSONmessageBuffer[2000];
  
        root.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  
        mqtt.publish(topicP, JSONmessageBuffer);
        
        
      }else
      {
        StaticJsonBuffer<200> jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(strMensagem);
        int modo = root["Modo"];
        int addr = root["Ende"];
        int data = root["Dados"];
        SerialMon.println(addr,DEC);
        SerialMon.println(data,DEC);

        // set word 0 of TX buffer to least-significant word of counter (bits 15..0)
        node.setTransmitBuffer(0, lowWord(data));
  
        // set word 1 of TX buffer to most-significant word of counter (bits 31..16)
        node.setTransmitBuffer(1, highWord(data));

        if(modo == 16)
        {
        // slave: write TX buffer to (2) 16-bit registers starting at register 0
        result = node.writeMultipleRegisters(addr, 2);
        }
        if(modo == 15)
        {
        // slave: write TX buffer to (2) 16-bit registers starting at register 0
        result = node.writeMultipleCoils(addr, 2);
        
        }
        if(modo == 6)
        {
        // slave: write TX buffer to (2) 16-bit registers starting at register 0
        result = node.writeSingleRegister(addr, 2);
        
        }
        if(modo == 5)
        {
        // slave: write TX buffer to (2) 16-bit registers starting at register 0
        result = node.writeSingleCoil(addr, 2);
        
        }
        if(modo == 3)
        {
            StaticJsonBuffer<1500> jsonBuffer;
            JsonObject& root = jsonBuffer.createObject();
            JsonArray& Dados = root.createNestedArray("Dados");
            Dados.add(addr);
            Dados.add(le_h3(addr));
            char JSONmessageBuffer[1500];
            root.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
            mqtt.publish(topicP, JSONmessageBuffer);
        
        }




        
      }
  }
  
  
}

boolean mqttConnect() {
  SerialMon.print("Connecting to ");
  SerialMon.print(broker);

  // Connect to MQTT Broker
  boolean status = mqtt.connect(gerador_id.c_str());
  // Or, if you want to authenticate MQTT:
  //boolean status = mqtt.connect("GsmClientName", "mqtt_user", "mqtt_pass");

  if (status == false) {
    SerialMon.println(" fail");
    return false;
  }
  SerialMon.println(" success");
  mqtt.subscribe(topicRec);
  return mqtt.connected();
}



void setup() {
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);

  pinMode(comap, INPUT_PULLUP);
  pinMode(deif, INPUT_PULLUP);

  is_comap = digitalRead(comap);
  is_deif = digitalRead(deif);

  if ((is_comap) && (is_deif))
  {
    digitalWrite(led, HIGH);
    delay(10000);
    resetFunc();
  }

  if (!is_comap)
  {
    digitalWrite(led, HIGH);
    delay(250);
    digitalWrite(led, LOW);
    delay(250);
    digitalWrite(led, HIGH);
    delay(250);
    digitalWrite(led, LOW);
    delay(250);    
  }
  if (!is_deif)
  {
    digitalWrite(led, HIGH);
    delay(250);
    digitalWrite(led, LOW);
    delay(250);
    digitalWrite(led, HIGH);
    delay(250);
    digitalWrite(led, LOW);
    delay(250);    
    digitalWrite(led, HIGH);
    delay(250);
    digitalWrite(led, LOW);
    delay(250);
  }
  
  Serial2.begin(9600);
  
  node.begin(1, Serial2);
  
  SerialMon.begin(115200);
  
  
  SerialMon.println("Wait...");
  
  SerialAT.begin(9600);
  delay(6000);

  SerialMon.println("Initializing modem...");
  modem.restart();
 
   
  String modemInfo = modem.getModemInfo();
  imei = modem.getIMEI();
  SerialMon.print("Modem Info: ");
  SerialMon.println(modemInfo);
  SerialMon.print("IMEI: ");
  SerialMon.println(imei);

  if ( GSM_PIN && modem.getSimStatus() != 3 ) {
    modem.simUnlock(GSM_PIN);
  }
  modem.gprsConnect(apn, gprsUser, gprsPass);

  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");

  if (modem.isNetworkConnected()) {
    SerialMon.println("Network connected");
  }
    SerialMon.print(F("Connecting to "));
    SerialMon.print(apn);
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
       SerialMon.print(apn_g);
      if (!modem.gprsConnect(apn_g, gprsUser, gprsPass)) {
      SerialMon.println(" fail");
      delay(10000);
      return;
      }
    }
    SerialMon.println(" success");

  if (modem.isGprsConnected()) {
    SerialMon.println("GPRS connected");
  }


  gerador_id = gerador_id + imei;
  SerialMon.println(gerador_id);
  
  topicP_str = topicP_str + imei +"/";
  topicRec_str = topicRec_str + imei +"/";
  topicStatus_str = topicStatus_str + imei +"/";

  SerialMon.println(topicP_str);
  SerialMon.println(topicRec_str);
  SerialMon.println(topicStatus_str);

  int topicP_len = topicP_str.length() + 1;
  int topicRec_len = topicRec_str.length() + 1;
  int topicStatus_len = topicStatus_str.length() + 1; 
  

  topicP_str.toCharArray(topicP, topicP_len);
  topicRec_str.toCharArray(topicRec, topicRec_len);
  topicStatus_str.toCharArray(topicStatus, topicStatus_len);

  // MQTT Broker setup
  mqtt.setServer(broker, 7083);
  mqtt.setCallback(mqttCallback);
   
}

float le_h2(int a)
{
  result = node.readDiscreteInputs(a, 1);
  if (result == node.ku8MBSuccess)
  {
    float aux;
    aux = node.getResponseBuffer(0);
    return aux;
  }
}

float le_h4(int a)
{
  result = node.readInputRegisters(a, 1);
  if (result == node.ku8MBSuccess)
  {
    float aux;
    aux = node.getResponseBuffer(0);
    return aux;
  }
}

float le_h4_double(int a)
{
  result = node.readInputRegisters(a, 2);
  if (result == node.ku8MBSuccess)
  {
    float aux;
    aux = ((node.getResponseBuffer(0)<<16) + node.getResponseBuffer(1));
    return aux;
  }
}

float le_h3(int a)
{
  result = node.readHoldingRegisters(a, 1);
  if (result == node.ku8MBSuccess)
  {
    float aux;
    aux = node.getResponseBuffer(0);
    return aux;
  }
}

float le_h3_double(int a)
{
  result = node.readHoldingRegisters(a, 2);
  if (result == node.ku8MBSuccess)
  {
    float aux;
    aux = ((node.getResponseBuffer(0)<<16) + node.getResponseBuffer(1));
    return aux;
  }
}

void envia_dados_deif()
{
  digitalWrite(led, HIGH);
  StaticJsonBuffer<1500> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonArray& Vendor = root.createNestedArray("Vendor");
  JsonArray& Gen_L1_L2 = root.createNestedArray("Gen_L1_L2");
  JsonArray& Gen_L2_L3 = root.createNestedArray("Gen_L2_L3");
  JsonArray& Gen_L3_L1 = root.createNestedArray("Gen_L3_L1");
  JsonArray& LoadA_L1 = root.createNestedArray("LoadA_L1");
  JsonArray& LoadA_L2 = root.createNestedArray("LoadA_L2");
  JsonArray& LoadA_L3 = root.createNestedArray("LoadA_L3");
  JsonArray& RPM = root.createNestedArray("RPM");
  JsonArray& Gen_Freq = root.createNestedArray("Gen_Freq");
  JsonArray& Load_kW = root.createNestedArray("Load_kW");
  JsonArray& Load_kW_L1 = root.createNestedArray("Load_kW_L1");
  JsonArray& Load_kW_L2 = root.createNestedArray("Load_kW_L2");
  JsonArray& Load_kW_L3 = root.createNestedArray("Load_kW_L3");
  JsonArray& Load_kVAr = root.createNestedArray("Load_kVAr");
  JsonArray& Load_kVAr_L1 = root.createNestedArray("Load_kVAr_L1");
  JsonArray& Load_kVAr_L2 = root.createNestedArray("Load_kVAr_L2");
  JsonArray& Load_kVAr_L3 = root.createNestedArray("Load_kVAr_L3");
  JsonArray& Load_PF = root.createNestedArray("Load_PF");
  JsonArray& Load_kVA = root.createNestedArray("Load_kVA");
  JsonArray& Load_kVA_L1 = root.createNestedArray("Load_kVA_L1");
  JsonArray& Load_kVA_L2 = root.createNestedArray("Load_kVA_L2");
  JsonArray& Load_kVA_L3 = root.createNestedArray("Load_kVA_L3");
  JsonArray& Mains_L1_L2 = root.createNestedArray("Mains_L1_L2");
  JsonArray& Mains_L2_L3 = root.createNestedArray("Mains_L2_L3");
  JsonArray& Mains_L3_L1 = root.createNestedArray("Mains_L3_L1");
  JsonArray& Mains_Freq = root.createNestedArray("Mains_Freq");
  JsonArray& Battery_V = root.createNestedArray("Battery_V");
  JsonArray& GBposition = root.createNestedArray("GBposition");
  JsonArray& MBposition = root.createNestedArray("MBposition");
  JsonArray& Running = root.createNestedArray("Running");
  JsonArray& Vgen_ok = root.createNestedArray("Vgen_ok");
  JsonArray& MainsFailure = root.createNestedArray("MainsFailure");
  JsonArray& OFF = root.createNestedArray("OFF");
  JsonArray& ManualMode = root.createNestedArray("ManualMode");
  JsonArray& AutoMode = root.createNestedArray("AutoMode");
  JsonArray& TestMode = root.createNestedArray("TestMode");
  JsonArray& LoadOver = root.createNestedArray("LoadOver");
  JsonArray& A1000 = root.createNestedArray("A1000");
  JsonArray& A1001 = root.createNestedArray("A1001");
  JsonArray& A1002 = root.createNestedArray("A1002");
  JsonArray& A1003 = root.createNestedArray("A1003");
  JsonArray& A1005 = root.createNestedArray("A1005");
  JsonArray& A1007 = root.createNestedArray("A1007");
  JsonArray& A1010 = root.createNestedArray("A1010");
  JsonArray& A1013 = root.createNestedArray("A1013");
  JsonArray& A1014 = root.createNestedArray("A1014");
  JsonArray& A1015 = root.createNestedArray("A1015");
  JsonArray& A1016 = root.createNestedArray("A1016");
  JsonArray& A1018 = root.createNestedArray("A1018");
  JsonArray& A1019 = root.createNestedArray("A1019");
  JsonArray& A1055 = root.createNestedArray("A1055");
  JsonArray& RunHours = root.createNestedArray("RunHours");
  JsonArray& NumStarts = root.createNestedArray("NumStarts");
  JsonArray& Alarms = root.createNestedArray("Alarms");
  JsonArray& AlarmsUnack = root.createNestedArray("AlarmsUnack");
  JsonArray& GsmSig = root.createNestedArray("GsmSig");
  JsonArray& IMEI = root.createNestedArray("IMEI");

  Vendor.add("DEIF");
  Gen_L1_L2.add(le_h4(501));
  Gen_L2_L3.add(le_h4(502));
  Gen_L3_L1.add(le_h4(503));
  LoadA_L1.add(le_h4(513));
  LoadA_L2.add(le_h4(514));
  LoadA_L3.add(le_h4(515));
  RPM.add(le_h4(576));
  Gen_Freq.add(le_h4(507));
  Load_kW.add(le_h4(519));
  Load_kW_L1.add(le_h4(516));
  Load_kW_L2.add(le_h4(517));
  Load_kW_L3.add(le_h4(518));
  Load_kVAr.add(le_h4(523));
  Load_kVAr_L1.add(le_h4(520));
  Load_kVAr_L2.add(le_h4(521));
  Load_kVAr_L3.add(le_h4(522));
  Load_PF.add(le_h4(538));
  Load_kVA.add(le_h4(527));
  Load_kVA_L1.add(le_h4(524));
  Load_kVA_L2.add(le_h4(525));
  Load_kVA_L3.add(le_h4(526));
  Mains_L1_L2.add(le_h4(539));
  Mains_L2_L3.add(le_h4(540));
  Mains_L3_L1.add(le_h4(541));
  Mains_Freq.add(le_h4(545));
  Battery_V.add(le_h4(567));
  A1000.add(le_h4(1000));
  A1001.add(le_h4(1001));
  A1002.add(le_h4(1002));
  A1003.add(le_h4(1003));
  A1005.add(le_h4(1005));
  A1007.add(le_h4(1007));
  A1010.add(le_h4(1010));
  A1013.add(le_h4(1013));
  A1014.add(le_h4(1014));
  A1015.add(le_h4(1015));
  A1016.add(le_h4(1016));
  A1018.add(le_h4(1018));
  A1019.add(le_h4(1019));
  A1055.add(le_h4(1055));
  GBposition.add(le_h2(0));
  MBposition.add(le_h2(1));
  Running.add(le_h2(3));
  Vgen_ok.add(le_h2(4));
  MainsFailure.add(le_h2(5));
  OFF.add(le_h2(6));
  ManualMode.add(le_h2(7));
  AutoMode.add(le_h2(9));
  TestMode.add(le_h2(10));
  LoadOver.add(le_h2(18));
  RunHours.add(le_h4(31));
  NumStarts.add(le_h4(29));
  Alarms.add(le_h4(27));
  AlarmsUnack.add(le_h4(28));
  GsmSig.add(modem.getSignalQuality());
  IMEI.add(imei);
  
  //root.printTo(SerialMon);
  char JSONmessageBuffer[1500];
  
  root.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  //SerialMon.print(JSONmessageBuffer);
  mqtt.publish(topicStatus, JSONmessageBuffer);
  //SerialMon.println();
  digitalWrite(led, LOW);
}




void envia_dados_comap()
{
  digitalWrite(led, HIGH);
  StaticJsonBuffer<2000> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonArray& Dados = root.createNestedArray("Dados");
  
  Dados.add("COMAP");  
  for (int i = 0; i < 80; i++)
  {
    Dados.add(le_h3(i));
  }
  
  Dados.add(le_h3_double(3000));
  Dados.add(le_h3(3003));
  Dados.add(le_h3(3004));
  Dados.add(le_h3_double(3005));
  Dados.add(le_h3_double(3007));
  Dados.add(le_h3_double(3009));
  Dados.add(le_h3_double(3011));
  
  Dados.add(modem.getSignalQuality());
  Dados.add(imei);

  
  
  
    
  char JSONmessageBuffer[2000];
  
  root.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  //SerialMon.print(JSONmessageBuffer);
  mqtt.publish(topicStatus, JSONmessageBuffer);
  //SerialMon.println();
  digitalWrite(led, LOW);
}



void loop() {
  if ((!modem.isNetworkConnected()) || (!modem.isGprsConnected())) {
    resetFunc();  //call reset
  }
  
  if (!mqtt.connected()) {
    SerialMon.println("=== MQTT NOT CONNECTED ===");
    // Reconnect every 10 seconds
    if ((!modem.isNetworkConnected()) || (!modem.isGprsConnected())) {
    resetFunc();  //call reset
  }
    uint32_t t = millis();
    if (t - lastReconnectAttempt > 10000) {
      lastReconnectAttempt = t;
      if (mqttConnect()) {
        lastReconnectAttempt = 0;
      }
    }
    delay(100);
    return;
  }
  
  mqtt.loop();
  if(!is_comap) envia_dados_comap();
  if(!is_deif) envia_dados_deif();
  delay(3000);
   
 

}

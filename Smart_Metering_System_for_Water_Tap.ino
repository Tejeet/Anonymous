      /***************************************************************************************************************
       *__________Smart Water Metering System for Tap______ Tejeet@outlook.com_________
       * 
       *      This is the Project made in the Hackthon Event "Smart City" Boot Camp on ( 8-10 Jully 2016) at Pune.
       * 
       * In This project we have Used the Water flow sensor to measure the flow rate of water, and by using ESP8266 WiFi
       * module , We sent the Data of Water usage on Datonis Cloud . and we have analysed it. and According to this data
       * we trigger the things like Sent the messege to user about its Over Usage of Water.....
       * 
       * 
       * ****************************************************************************************************************
       
      */
      
      
      
      #include <sha256.h>
      #include <ESP8266WiFi.h>
 
 
      #define SECRET_KEY "dd22fbee9d31c9e25bdatbf141725b3edc3f94ff"  // Secret Key given by Datonis
      #define HOST_URL "datonis.altizon.com"                         // Host URL Dentonis
      #define ACCESS_KEY "4a4et5d8f339a29cf79f7ctf5528t57628d97t56"  // Access Key given by Datonis
      #define SENSOR_KEY "73t2d783e1"                                // Sensor Key given by Datonis
      #define NO_OF_EVENTS 20
      #define NO_OF_HEARTBEATS 2

      int Epoc = 1468136996;           //Change this Epoch key from http://www.epochconverter.com/ Before Uploading 
                                       // the 
      int mepoc;
      const char* ssid = "TWEVENT";                                 // SSID of WiFi
      const char* password = "TWpune@1234";                         // WiFi Password
 
      volatile int NbTopsFan;   //measuring the rising edges of the signal 
      int sensor_data; 
      int totaldata=0;
      int hallsensor = 0;       //The pin location of the sensor
      WiFiClient client;        // starts a WiFi client.

 
      IPAddress server(54, 152, 59, 3);               // numeric IP for Datonis (no DNS)
      
      unsigned int localPort = 8888;                  // local port to listen for UDP packets
      
      char timeServer[] = "time.nist.gov";            // time.nist.gov NTP server
      
      const int NTP_PACKET_SIZE = 48;                 // NTP time stamp is in the first 48 bytes of the message
      
      byte packetBuffer[ NTP_PACKET_SIZE];            //buffer to hold incoming and outgoing packets
 
      
      
 
 
void rpm ()     //This is the function that the interupt calls  
    {
 
       NbTopsFan++;  //This function measures the rising and falling edge of the hall effect sensors signal   
        
    } 
  

void get_hash(char buf[], char json_buf[]);
void readResponse();
 
void setup() 
 
    {
 
           pinMode(hallsensor, INPUT);         //initializes digital pin 2 as an input i.e, Grooves Water Flow Sensor   
           Serial.begin(115200);               //This is the setup function where the serial port is initialised,   
           attachInterrupt(0, rpm, RISING);    //and the interrupt is attached 
 
           Serial.println();
           Serial.println();
           Serial.print("Connecting to ");
           Serial.println(ssid);
           WiFi.begin(ssid, password);         // starts WiFi & login
 
           while (WiFi.status() != WL_CONNECTED)
              { 
                delay(500);
                Serial.print(".");
              }
           Serial.println("");
           Serial.println("WiFi connected");
        
        char array[1024];
        memset(array, 0, 1024);
      
        char hash_buffer[65];
        
        mepoc=Epoc*1000;
        char *json ="{\"timestamp\":%d,\"name\":\"WaterMeter1\",\"description\":\"Water meter\",\"sensor_key\":\"73t2d783e1\",\"type\":\"WaterMeter\"}",mepoch;
      
        get_hash(hash_buffer, json);
        // To Register Things on Dantonis
        sprintf(array, "POST /api/v1/sensor/register.json HTTP/1.1\r\nHost: datonis.altizon.com\r\nAccept: */*\r\nX-Access-Key: %s\r\nX-Dtn-Signature: %s\r\nContent-Type: application/json\r\nContent-Length: %d\r\nCache-Control: no-cache\r\n\r\n%s", ACCESS_KEY, hash_buffer, strlen(json), json);  
        Serial.println("Printing register request\r\n");
        Serial.println(array);     
        if (client.connect(server, 80))
            {
              Serial.println("\n--Connected--\r\n");
              client.println(array);
              readResponse();
            }
 
    } 
 
void connectAndRead(unsigned long);
unsigned long get_epoch_time();
 
void loop()
  
  {  
        int i = 0;
        NbTopsFan = 0;   
        Serial.println("In main loop\r\n");
  
        unsigned long epoch_time = get_epoch_time();
      
        //Send  NO_OF_EVENTS  to Datonis
        while(true) 
              {
         
                cli();
                connectAndRead(epoch_time + i); //connect to the server and read the output
                readResponse();
                NbTopsFan = 0;
                sei();
                delay(1000);
                i++;
              }
        while (true); // Do nothing forever
      
  }
      
      
      /******************************* User Defined Functions ******************************/
      
void connectAndRead(unsigned long epoch_time) 
                {
                 Serial.println("In connect and read\r\n");
      
                 char *sensor_event;
                 char array[1024];
                 memset(array, 0, 1024);
      
                 char hash_buffer[65];
                 char json[512];
      
         
                 sensor_data = 0;
                 sensor_data = (NbTopsFan * 60/ 7.5); //(Pulse frequency x 60) / 7.5Q, = flow rate   
                 totaldata += sensor_data;
           
          
                //For Sensor Event
                 Serial.println("Sending Event\r\n");
      
                 sprintf(json, "{\"sensor_key\":\"%s\",\"timestamp\":%lu%s,\"data\":{\"flow\": %d,\"total\": %d}}", SENSOR_KEY, epoch_time, "000", sensor_data,totaldata);
                 get_hash(hash_buffer, json);
                 sprintf(array, "POST /api/v1/sensor/event.json HTTP/1.1\r\nHost: datonis.altizon.com\r\nAccept: */*\r\nX-Access-Key: %s\r\nX-Dtn-Signature: %s\r\nContent-Type: application/json\r\nContent-Length: %d\r\nCache-Control: no-cache\r\n\r\n%s", ACCESS_KEY, hash_buffer, strlen(json), json);
      
                 Serial.println(array);
      
                  // if you get a connection, report back via serial:
                   if (client.connect(server, 80)) 
                        {
                          Serial.println("\n--Connected--\r\n");
      
                         // Make a HTTP request:
                          client.println(array);
                        }
                  else 
                  {
                      // if you didn't get a connection to the server:
                       Serial.println("\nXX Connection failed XX");
                  }
        
      
                }
      
      
void readResponse() 
          {
      
        // if there are incoming bytes available
        // from the server, read them and print them:
      
            while (true) 
                {
                 if (client.available()) 
                    {
                      char c = client.read();
                      Serial.print(c);
                    } 
                else 
                    {
                     Serial.println("Disconnecting..................\r\n");
                     client.stop();
                     break;
                    }
 
          
                }
      
          }
      
      
void printHash(uint8_t* hash, char buf[]) 
       {
          int i;
          int j = 0;
          for (i = 0; i < 32; i++) 
              {
               buf[j++] = "0123456789abcdef"[hash[i] >> 4];
               buf[j++] = "0123456789abcdef"[hash[i] & 0xf];
              }
        Serial.println();
       }
      
      
unsigned long get_epoch_time() 
            {
              return Epoc;
            }
 
       
void get_hash(char buf[], char json_buf[]) 
            {
              uint8_t *hash;
             Sha256.initHmac((const unsigned char*)SECRET_KEY, strlen(SECRET_KEY)); // key, and length of key in bytes
      
             Sha256.print(json_buf);
      
             printHash(Sha256.resultHmac(), buf);
             buf[64] = '\0';
             return;
            }


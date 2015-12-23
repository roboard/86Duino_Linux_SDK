// Include the GSM library
#include <GSM.h>

#define PINNUMBER "0525"

// initialize the library instance
GSM gsmAccess;
GSM_SMS sms;
char* remoteNum = "+886911718168";
void setup()
{
  printf("SMS Messages Sender\n");

  // connection state
  boolean notConnected = true;

  // Start GSM shield
  // If your SIM has PIN, pass it as a parameter of begin() in quotes
  while(notConnected)
  {
    if(gsmAccess.begin(PINNUMBER)==GSM_READY)
      notConnected = false;
    else
    {
      printf("Not connected\n");
      delay(1000);
    }
  }
  
  printf("GSM initialized\n");
}

void loop()
{
  printf("Enter a mobile number: ");
  printf("%s", remoteNum);
    
  // sms text
  Serial.print("Now, enter SMS content: ");
  char* txtMsg = "Hello, 86Duino!";
  
  // send the message
  sms.beginSMS(remoteNum);
  sms.print(txtMsg);
  sms.endSMS(); 
  printf("\nCOMPLETE!\n");
}

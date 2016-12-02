// This #include statement was automatically added by the Particle IDE.
#include "MCP23008_I2CDIO8.h"

/* Includes ------------------------------------------------------------------*/
#include "NCD8Relay/NCD8Relay.h"
#include "spark_wiring_print.h"

SYSTEM_MODE(AUTOMATIC);
NCD8Relay relayController;
MCP23008 inputBoard;
int triggerRelay(String command);
int relayStatus = 256;
int inputStatus = 256;
//Global variables for timers.
unsigned long tripTime[8];

bool timmerRunning[8];

unsigned long runTime[8];

/* This function is called once at start up ----------------------------------*/
void setup()
{
	Serial.begin(115200);
	relayController.setAddress(0,0,0);
	inputBoard.setAddress(1,0,0);
	Particle.function("controlRelay", triggerRelay);
	Particle.variable("R_Status", relayStatus);
	Particle.variable("I_Status", inputStatus);
}

/* This function loops forever --------------------------------------------*/
void loop()
{
    relayStatus = relayController.readRelayBankStatus();
    
    inputStatus = inputBoard.readAllInputs();
    
    //Check our timers
    for(int i = 0; i < 8; i++){
        //See if a timer is running or not
        if(timmerRunning[i]){
            //Check to see if it is time to turn the relay off yet
            if(millis() >= tripTime[i] + runTime[i]){
                //Yep, turn off the relay
                relayController.turnOffRelay(i+1);
                timmerRunning[i] = false;
            }
        } 
    }
    
    delay(50);
}

int triggerRelay(String command){
	if(command.equalsIgnoreCase("turnonallrelays")){
		relayController.turnOnAllRelays();
		return 1;
	}
	if(command.equalsIgnoreCase("turnoffallrelays")){
		relayController.turnOffAllRelays();
		return 1;
	}
	if(command.startsWith("setBankStatus:")){
		int status = command.substring(14).toInt();
		if(status < 0 || status > 255){
			return 0;
		}
		Serial.print("Setting bank status to: ");
		Serial.println(status);
		relayController.setBankStatus(status);
		Serial.println("done");
		return 1;
	}
	
	    //New relay timer command
    //format of value passed to function is "timerRelayNumber,Duration in mS" Example to turn relay 1 on for 2 seconds: timer1,2000
    if(command.startsWith("timer")){
        int relayNumber = command.substring(5,6).toInt();
        int startIndex = command.indexOf(",");
        int tDuration = command.substring(startIndex+1, command.length()).toInt();
        int arrayIndex = relayNumber -1;
        relayController.turnOnRelay(relayNumber);
        timmerRunning[arrayIndex] = true;
        runTime[arrayIndex] = (unsigned long)tDuration;
        tripTime[arrayIndex] = millis();
        return 1;
    }
	
	//Relay Specific Command
	int relayNumber = command.substring(0,1).toInt();
	Serial.print("relayNumber: ");
	Serial.println(relayNumber);
	String relayCommand = command.substring(1);
	Serial.print("relayCommand:");
	Serial.print(relayCommand);
	Serial.println(".");
	if(relayCommand.equalsIgnoreCase("on")){
		Serial.println("Turning on relay");
		relayController.turnOnRelay(relayNumber);
		Serial.println("returning");
		return 1;
	}
	if(relayCommand.equalsIgnoreCase("off")){
		relayController.turnOffRelay(relayNumber);
		return 1;
	}
	if(relayCommand.equalsIgnoreCase("toggle")){
		relayController.toggleRelay(relayNumber);
		return 1;
	}
	if(relayCommand.equalsIgnoreCase("momentary")){
		relayController.turnOnRelay(relayNumber);
		delay(300);
		relayController.turnOffRelay(relayNumber);
		return 1;
	}
	return 0;
}

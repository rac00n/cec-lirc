#include <iostream>
#include <array>
#include <signal.h>
#include <chrono>
#include <thread>

#include "libcec/cec.h"
#include "libcec/cecloader.h"
#include "lirc_client.h"

using namespace std;
using namespace CEC;

// The main loop will just continue until a ctrl-C is received
static bool exit_now = false;
static int lircFd;

void handle_signal(int signal)
{
    exit_now = true;
}

void CECLogMessage(void *not_used, const cec_log_message* message)
{
	cout << "LOG" << message->level << " " << message->message << endl;
}

int send_packet(lirc_cmd_ctx* ctx, int fd)
{
        int r;
        do {
                r = lirc_command_run(ctx, fd);
                if (r != 0 && r != EAGAIN)
                        fprintf(stderr,
                                "Error running command: %s\n", strerror(r));
        } while (r == EAGAIN);
        return r == 0 ? 0 : -1;
}


void CECKeyPress(void *cbParam, const cec_keypress* key)
{
	static lirc_cmd_ctx ctx;

	cout << "CECKeyPress: key " << hex << unsigned(key->keycode) << " duration " << dec << unsigned(key->duration) << std::endl;

    switch( key->keycode )
     {
         case CEC_USER_CONTROL_CODE_VOLUME_UP:
        	 if (key->duration == 0) // key pressed
        	 {
        		 lirc_command_init(&ctx, "SEND_START Yamaha_RAV283 KEY_VOLUMEUP\n");
        	 }
        	 else
        	 {
        		 lirc_command_init(&ctx, "SEND_STOP Yamaha_RAV283 KEY_VOLUMEUP\n");
        	 }
    		 lirc_command_reply_to_stdout(&ctx);
    		 send_packet(&ctx, lircFd);
        	 break;
         case CEC_USER_CONTROL_CODE_VOLUME_DOWN:
        	 if (key->duration == 0) // key pressed
        	 {
        		 lirc_command_init(&ctx, "SEND_START Yamaha_RAV283 KEY_VOLUMEDOWN\n");
        	 }
        	 else
        	 {
        		 lirc_command_init(&ctx, "SEND_STOP Yamaha_RAV283 KEY_VOLUMEDOWN\n");
        	 }
    		 lirc_command_reply_to_stdout(&ctx);
    		 send_packet(&ctx, lircFd);
        	 break;
         case CEC_USER_CONTROL_CODE_MUTE:
        	 if (key->duration > 0) // key released
        	 {
        		 if (lirc_send_one(lircFd, "Yamaha_RAV283", "KEY_MUTE") == -1)
        		 {
        			 cout << "CECKeyPress: lirc_send_one KEY_MUTE failed" << endl;
        		 }
        	 }
        	 break;
         default:
        	 break;
     }

}

void CECCommand(void *cbParam, const cec_command* command)
{
    cout << "CECCommand: opcode " << hex << unsigned(command->opcode) << " " << unsigned(command->initiator) << " -> " <<
    		unsigned(command->destination) << endl;
    switch (command->opcode)
    {
    	case CEC_OPCODE_ACTIVE_SOURCE:
    		break;
    	case CEC_OPCODE_ROUTING_CHANGE:
    		break;
    	case CEC_OPCODE_REPORT_POWER_STATUS:
    		if (command->parameters.data[0] ==  CEC_POWER_STATUS_STANDBY)
    		{
    			if (lirc_send_one(lircFd, "Yamaha_RAV283", "KEY_SUSPEND") == -1)
    			{
    				cout << "CECKeyPress: lirc_send_one KEY_SUSPEND failed" << endl;
    			}
    		}
    		break;
    	default:
    		break;
    }
}

void CECAlert(void *cbParam, const libcec_alert type, const libcec_parameter param)
{
  switch (type)
  {
  case CEC_ALERT_CONNECTION_LOST:
  	  cout << "Connection lost" <<endl;
	  break;
  default:
    break;
  }
}


int main (int argc, char *argv[])
{
  ICECCallbacks         CECCallbacks;
  libcec_configuration  CECConfig;
  ICECAdapter*          CECAdapter;

  // Install the ctrl-C signal handler
  if( SIG_ERR == signal(SIGINT, handle_signal) )
  {
      cerr << "Failed to install the SIGINT signal handler\n";
      return 1;
  }

  CECConfig.Clear();
  CECCallbacks.Clear();
  snprintf(CECConfig.strDeviceName, LIBCEC_OSD_NAME_SIZE, "CECtoIR");
  CECConfig.clientVersion      = LIBCEC_VERSION_CURRENT;
  CECConfig.bActivateSource    = 0;
  CECCallbacks.logMessage      = &CECLogMessage;
  CECCallbacks.keyPress        = &CECKeyPress;
  CECCallbacks.commandReceived = &CECCommand;
  CECCallbacks.alert           = &CECAlert;
  CECConfig.callbacks          = &CECCallbacks;

  // CECConfig.deviceTypes.Add(CEC_DEVICE_TYPE_TUNER);
  CECConfig.deviceTypes.Add(CEC_DEVICE_TYPE_AUDIO_SYSTEM);

  if (! (CECAdapter = LibCecInitialise(&CECConfig)) )
  {
	  cerr << "LibCecInitialise failed"  << endl;
	  return 1;
  }

  array<cec_adapter_descriptor,10> devices;

  int8_t devices_found = CECAdapter->DetectAdapters(devices.data(), devices.size(), nullptr, false);
  if( devices_found <= 0)
   {
       cerr << "Could not automatically determine the cec adapter devices" << endl;
       UnloadLibCec(CECAdapter);
       return 1;
   }

  cerr << unsigned(devices_found) << " devices found" << endl;

  // Open a connection to the zeroth CEC device
   if( !CECAdapter->Open(devices[0].strComName) )
   {
       cerr << "Failed to open the CEC device on port " << devices[0].strComName << endl;
       UnloadLibCec(CECAdapter);
       return 1;
   }

   lircFd = lirc_get_local_socket("/var/run/lirc/lircd-tx", 0);
   if (lircFd < 0) {
       cerr << "Failed to get LIRC local socket" << endl;
       UnloadLibCec(CECAdapter);
       return 1;
   }

   cerr << "waiting for ctl-c" << endl;

  // Loop until ctrl-C occurs
  while( !exit_now )
  {
      // nothing to do.  All happens in the CEC callback on another thread
      this_thread::sleep_for( std::chrono::seconds(1) );
  }

  // Close down and cleanup
  cerr << "Close and cleanup" << endl;

  CECAdapter->Close();
  UnloadLibCec(CECAdapter);

  return 0;
}




#include <iostream>

#include "libcec/cec.h"
#include "libcec/cecloader.h"

using namespace std;
using namespace CEC;

int main (int argc, char *argv[])
{
  ICECCallbacks         CECCallbacks;
  libcec_configuration  CECConfig;
  ICECAdapter*          CECAdapter;

  CECConfig.Clear();
  CECCallbacks.Clear();
  snprintf(CECConfig.strDeviceName, LIBCEC_OSD_NAME_SIZE, "CECTester");
  CECConfig.clientVersion      = LIBCEC_VERSION_CURRENT;
  CECConfig.bActivateSource    = 0;
  // CECCallbacks.logMessage      = &CecLogMessage;
  // CECCallbacks.keyPress        = &CecKeyPress;
  // CECCallbacks.commandReceived = &CecCommand;
  // CECCallbacks.alert           = &CecAlert;
  // CECConfig.callbacks          = &CECCallbacks;

  CECConfig.deviceTypes.Add(CEC_DEVICE_TYPE_TUNER);
  CECConfig.deviceTypes.Add(CEC_DEVICE_TYPE_AUDIO_SYSTEM);

  CECAdapter = LibCecInitialise(&CECConfig);

  cout << "Hello World!" << endl;
  return 0;
}




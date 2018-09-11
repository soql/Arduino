#ifndef _SOQL_TOOLS_H
#define _SOQL_TOOLS_H
#include <WString.h>

struct wifi_struct {
    String ssid;
    String password;
};

int ConnectToAP(wifi_struct wifilist[], int wifisize);

#endif /*_SOQL_TOOLS_H */


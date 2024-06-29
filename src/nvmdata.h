#ifndef NVMData_h
#define NVMData_h

#include <Arduino.h>
#include <Preferences.h>

class NVMData
{
private:
    const char* prefDefaultValue = "none";

    const char* prefKeySSID = "ssid";
    const char* prefKeyPSK = "psk";
    const char* prefKeyNamespace = "battery";
    const char* prefKeyOperatingHourCounter = "OHC";
    const int prefKeyPowerSourceSerialDefault = 0;
    const uint32_t prefKeyOffsetDefault = 0;

    String NetName = "";
    bool NetNameChanged = false;
    bool NetNameValid = false;
    String NetPassword = "";
    bool NetPasswordChanged = false;
    bool NetPasswordValid = false;
    uint32_t OHC = 0;
    NVMData(/* args */);
public:
    static NVMData& get()
    {
        static NVMData nonVolatileData;
        return nonVolatileData;
    }
    void Init();
    void StoreNetData();
    void DeleteNetData();
    void SetNetData(String newNetName, String newNetPassword);
    void SetDisplayIP(String newDisplayIP);
    String GetNetName();
    String GetNetPassword();
    
    bool NetDataValid();
    void IncOHC();
    void StoreOHC();
    uint32_t getOHC();
    void NextDay();
};

void NVMData::Init() 
{
    Preferences preferences;
    preferences.begin(prefKeyNamespace, false);
    NetName = preferences.getString(prefKeySSID, prefDefaultValue);
    NetPassword = preferences.getString(prefKeyPSK, prefDefaultValue);
    
    OHC = preferences.getDouble(prefKeyOperatingHourCounter, 0);
    preferences.end();
    if (NetName != prefDefaultValue)
    {
        NetNameValid = true;
    }
    if (NetPassword != prefDefaultValue)
    {
        NetPasswordValid = true;
    }
}
bool NVMData::NetDataValid()
{
    bool retVal = true;
    if (NetNameValid == false)
    {
        retVal = false;
    }
    if (NetPasswordValid == false)
    {
        retVal = false;
    }
    return retVal;
}
void NVMData::SetNetData(String newNetName, String newNetPassword)
{
    if (NetName != newNetName)
    {
      NetName = newNetName;
      NetNameChanged = true;
    }
    if (NetPassword != newNetPassword)
    {
      NetPassword = newNetPassword;
      NetPasswordChanged = true;
    }
}
String NVMData::GetNetName()
{
    return NetName;
}
String NVMData::GetNetPassword()
{
    return NetPassword;
}
void NVMData::StoreNetData() {
    Preferences preferences;
    preferences.begin(prefKeyNamespace, false);
    if (NetNameChanged == true)
    {
        preferences.putString(prefKeySSID, NetName);
        NetNameChanged = false;
    }
    if (NetPasswordChanged == true)
    {
        preferences.putString(prefKeyPSK, NetPassword);
        NetPasswordChanged = false;
    }
    preferences.end();
}
void NVMData::DeleteNetData() {
    Preferences preferences;
    preferences.begin(prefKeyNamespace, false);
    preferences.remove(prefKeyPSK);
    preferences.remove(prefKeySSID);
    preferences.end();
    NetName = "";
    NetPassword ="";
}
void NVMData::IncOHC()
{
    OHC++;
}
void NVMData::StoreOHC()
{
    Preferences preferences;
    preferences.begin(prefKeyNamespace, false);
    preferences.putDouble(prefKeyOperatingHourCounter, OHC);
    preferences.end();
}
uint32_t NVMData::getOHC()
{
    return OHC;
}
void NVMData::NextDay()
{
    this->StoreOHC();
}
NVMData::NVMData(/* args */)
{
}

#endif
#include "ftdilib.h"

ftdilib::ftdilib()
{

}

/// \brief ftdilib::ftdi_open
/// \param ftHandle
/// \param ftStatus
/// \param [serial] if 0 then it returns first device's index(0)
/// \return index of ftdi device with the serial or -1 if not found!!!
int ftdilib::device_index(const char* serial, FT_STATUS* ftStatus)
{
    FT_DEVICE_LIST_INFO_NODE info;
    FT_HANDLE ftHandleTmp;
    DWORD numDevs;
    // create the device information list...
    *ftStatus = FT_CreateDeviceInfoList(&numDevs);
    if (*ftStatus != FT_OK || numDevs == 0) {
        return -1;
    }
    if (!serial) return 0; //first device
    for (DWORD i = 0; i < numDevs; i++) {
        *ftStatus = FT_GetDeviceInfoDetail(i, &info.Flags, &info.Type, &info.ID, &info.LocId, info.SerialNumber, info.Description, &ftHandleTmp);
        if (*ftStatus == FT_OK && strcmp(info.SerialNumber, serial) == 0) {
            return i;
        }
    }
    return -1; //not found
}

/// \brief ftdilib::ftdi_open
/// \param serial
/// \param ftStatus
/// \return
//DEPRECATED, use: FT_OpenEx()
FT_HANDLE ftdilib::open(const char* serial, FT_STATUS* ftStatus)
{
    FT_HANDLE handl;
    int idx = device_index(serial, ftStatus);
    if (idx >= 0) {
        *ftStatus = FT_Open(idx, &handl);
        if (*ftStatus == FT_OK) return handl;
    }
    return 0;
}


#include "BtInterface.h"

class WSDownloader
{
public:
    WSDownloader(CBtInterface *pBtInterface, LPBYTE pPatch, DWORD dwPatchSize, HWND hWnd);
    virtual ~WSDownloader();

    void Start();
    void ProcessEvent(BYTE Event);

    void TransferData();

    enum 
    {
        WS_UPGRADE_CONNECTED,
        WS_UPGRADE_RESPONSE_OK,
        WS_UPGRADE_START_VERIFICATION,
        WS_UPGRADE_RESPONSE_FAILED,
        WS_UPGRADE_ABORT,
    } m_event;

    enum 
    {
        WS_UPGRADE_STATE_IDLE,              
        WS_UPGRADE_STATE_WAIT_FOR_READY_FOR_DOWNLOAD,
        WS_UPGRADE_STATE_DATA_TRANSFER,
        WS_UPGRADE_STATE_VERIFICATION,  
        WS_UPGRADE_STATE_VERIFIED,     
        WS_UPGRADE_STATE_ABORTED,
    } m_state;

private:
    BOOL m_bConnected;

    HWND m_hWnd;
    DWORD m_offset;
    CBtInterface *m_btInterface;
    BYTE *m_Patch;
    DWORD m_PatchSize;
};

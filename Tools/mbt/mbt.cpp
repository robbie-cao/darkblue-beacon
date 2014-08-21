// mbt.cpp : Defines the entry point for the console application.
//
#include "tchar.h"
#include "mbt_com.h"

typedef unsigned char UINT8;

UINT8 in_buffer[1024];

//
// print hexadecimal digits of an array of bytes formatted as: 
// 0000 < 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F >
// 0010 < 10 11 12 13 14 15 16 1718 19 1A 1B 1C 1D 1E 1F >
//
void HexDump(LPBYTE p, DWORD dwLen)
{
    for (DWORD i = 0; i < dwLen; ++i)
    {
        if (i % 16 == 0)
            printf("%04X <", i);
        printf(" %02X", p[i]);
        if ((i + 1) % 16 == 0)
            printf(" >\n");
    }
    printf(" >\n");
}

static void print_usage_reset(bool full)
{
    printf ("Usage: mbt reset COMx\n");
}

static int execute_reset(char *szPort)
{
    ComHelper SerialPort;

    if (!SerialPort.OpenPort(szPort))
	{
        printf("Open %s port Failed\n", szPort);
		return 0;
	}

    UINT8 hci_reset[] = {0x01, 0x03, 0x0c, 0x00};
    UINT8 hci_reset_cmd_complete_event[] = {0x04, 0x0e, 0x04, 0x01, 0x03, 0x0c, 0x00};

    printf ("Sending HCI Command:\n");
    HexDump(hci_reset, sizeof(hci_reset));

    // write HCI reset
    SerialPort.Write(hci_reset, sizeof(hci_reset));

    // read HCI response header
    DWORD dwRead = SerialPort.Read((LPBYTE)&in_buffer[0], 3);

    // read HCI response payload
    if (dwRead == 3 && in_buffer[2] > 0)
        dwRead += SerialPort.Read((LPBYTE)&in_buffer[3], in_buffer[2]);

    printf ("Received HCI Event:\n");
    HexDump(in_buffer, dwRead);
    if (dwRead == sizeof(hci_reset_cmd_complete_event))
    {
        if (memcmp(in_buffer, hci_reset_cmd_complete_event, dwRead) == 0)
        {
            printf ("Success\n");
            return 1;
        }
    }
    return 0;
}

static void print_usage_le_receiver_test(bool full)
{
    printf ("Usage: mbt le_receiver_test COMx <rx_channel>\n");
    if (!full)
        return;
    printf ("                rx_channel = (F - 2402) / 2\n");
    printf ("                    Range: 0 - 39. Frequency Range : 2402 MHz to 2480 MHz\n");
}

static int execute_le_receiver_test(char *szPort, UINT8 chan_number)
{
    ComHelper SerialPort;

    if (!SerialPort.OpenPort(szPort))
	{
        printf("Open %s port Failed\n", szPort);
		return 0;
	}

    UINT8 hci_le_receiver_test[] = {0x01, 0x01D, 0x20, 0x01, 0x00};
    UINT8 hci_le_receiver_test_cmd_complete_event[] = {0x04, 0x0e, 0x04, 0x01, 0x01D, 0x20, 0x00};
    hci_le_receiver_test[4] = chan_number;

    printf ("Sending HCI Command:\n");
    HexDump(hci_le_receiver_test, sizeof(hci_le_receiver_test));

    // write HCI reset
    SerialPort.Write(hci_le_receiver_test, sizeof(hci_le_receiver_test));

    // read HCI response header
    DWORD dwRead = SerialPort.Read((LPBYTE)&in_buffer[0], 3);

    // read HCI response payload
    if (dwRead == 3 && in_buffer[2] > 0)
        dwRead += SerialPort.Read((LPBYTE)&in_buffer[3], in_buffer[2]);

    printf ("Received HCI Event:\n");
    HexDump(in_buffer, dwRead);
    if (dwRead == sizeof(hci_le_receiver_test_cmd_complete_event))
    {
        if (memcmp(in_buffer, hci_le_receiver_test_cmd_complete_event, dwRead) == 0)
        {
            printf ("Success\n");
			printf("LE Receiver Test running, to stop execute le_test_end\n");
			return 1;
        }
    }
    return FALSE;
}

static void print_usage_le_transmitter_test(bool full)
{
    printf ("Usage: mbt le_transmitter_test COMx <tx_channel> <data_length> <packet_payload>\n");
    if (!full)
        return;
    printf ("                tx_channel = (F - 2402) / 2\n");
    printf ("                    Range: 0 - 39. Frequency Range : 2402 MHz to 2480 MHz\n");
    printf ("                data_length: (0 - 37)\n");
    printf ("                data_pattern: (0 - 9)\n");
    printf ("                    0 - Pseudo-Random bit sequence 9\n");
    printf ("                    1 Pattern of alternating bits '11110000'\n");
    printf ("                    2 Pattern of alternating bits '10101010'\n");
    printf ("                    3 Pseudo-Random bit sequence 15\n");
    printf ("                    4 Pattern of All '1' bits\n");
    printf ("                    5 Pattern of All '0' bits\n");
    printf ("                    6 Pattern of alternating bits '00001111'\n");
    printf ("                    7 Pattern of alternating bits '0101'\n");
}

static int execute_le_transmitter_test(char *szPort, UINT8 chan_number, UINT8 length, UINT8 pattern)
{
    ComHelper SerialPort;

    if (!SerialPort.OpenPort(szPort))
	{
        printf("Open %s port Failed\n", szPort);
		return 0;
	}

    UINT8 hci_le_transmitter_test[] = {0x01, 0x01E, 0x20, 0x03, 0x00, 0x00, 0x00};
    UINT8 hci_le_transmitter_test_cmd_complete_event[] = {0x04, 0x0e, 0x04, 0x01, 0x01E, 0x20, 0x00};
    hci_le_transmitter_test[4] = chan_number;
    hci_le_transmitter_test[5] = length;
    hci_le_transmitter_test[6] = pattern;
    printf ("Sending HCI Command:\n");
    HexDump(hci_le_transmitter_test, sizeof(hci_le_transmitter_test));

    // write HCI reset
    SerialPort.Write(hci_le_transmitter_test, sizeof(hci_le_transmitter_test));

    // read HCI response header
    DWORD dwRead = SerialPort.Read((LPBYTE)&in_buffer[0], 3);

    // read HCI response payload
    if (dwRead == 3 && in_buffer[2] > 0)
        dwRead += SerialPort.Read((LPBYTE)&in_buffer[3], in_buffer[2]);

    printf ("Received HCI Event:\n");
    HexDump(in_buffer, dwRead);
    if (dwRead == sizeof(hci_le_transmitter_test_cmd_complete_event))
    {
        if (memcmp(in_buffer, hci_le_transmitter_test_cmd_complete_event, dwRead) == 0)
        {
            printf("Success\n");
			printf("LE Transmitter Test running, to stop execute le_test_end\n");
			return 1;
        }
    }
    return FALSE;
}

static void print_usage_le_test_end(bool full)
{
    printf ("Usage: mbt le_test_end COMx\n");
}

static int execute_le_test_end(char *szPort)
{
    ComHelper SerialPort;

    if (!SerialPort.OpenPort(szPort))
	{
        printf("Open %s port Failed\n", szPort);
		return 0;
	}

    UINT8 hci_le_test_end[] = {0x01, 0x1f, 0x20, 0x00};
    UINT8 hci_le_test_end_cmd_complete_event[] = {0x04, 0x0e, 0x06, 0x01, 0x1f, 0x20, 0x00};

    printf ("Sending HCI Command:\n");
    HexDump(hci_le_test_end, sizeof(hci_le_test_end));

    // write HCI reset
    SerialPort.Write(hci_le_test_end, sizeof(hci_le_test_end));

    // read HCI response header
    DWORD dwRead = SerialPort.Read((LPBYTE)&in_buffer[0], 3);

    // read HCI response payload
    if (dwRead == 3 && in_buffer[2] > 0)
        dwRead += SerialPort.Read((LPBYTE)&in_buffer[3], in_buffer[2]);

    printf ("Received HCI Event:\n");
    HexDump(in_buffer, dwRead);
    if ((dwRead > sizeof(hci_le_test_end_cmd_complete_event))
     && (memcmp(in_buffer, hci_le_test_end_cmd_complete_event, sizeof(hci_le_test_end_cmd_complete_event)) == 0))
    {
        printf("Success num_packets_received %d\n", in_buffer[7] + (in_buffer[8] << 8));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static void print_usage_set_tx_frequency_arm(bool full)
{
    printf ("Usage: mbt set_tx_frequency_arm COMx <carrier on/off> <tx_frequency> <tx_power>\n");
    if (!full)
        return;
    printf ("                carrier on/off: 1 - carrier on, 0 - carrier_off\n");
    printf ("                tx_frequency = (2402 – 2480) transmit frequency, in MHz\n");
    printf ("                tx_power = (-25 - +3) transmit power in dbm\n");
    
}

static int execute_set_tx_frequency_arm(char *szPort, UINT8 carrier_on, UINT16 tx_frequency, int tx_power)
{
    ComHelper SerialPort;

    if (!SerialPort.OpenPort(szPort))
    {
        printf("Open %s port Failed\n", szPort);
        return 0;
    }
    int chan_num = tx_frequency - 2400;
    UINT8 hci_set_tx_frequency_arm[] = {0x01, 0x014, 0xfc, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    UINT8 hci_set_tx_frequency_arm_cmd_complete_event[] = {0x04, 0x0e, 0x04, 0x01, 0x014, 0xfc, 0x00};
    hci_set_tx_frequency_arm[4] = (carrier_on == 0) ? 1 : 0;
    hci_set_tx_frequency_arm[5] = (carrier_on == 1) ? chan_num : 2;
    hci_set_tx_frequency_arm[6] = 0;                // unmodulated
    hci_set_tx_frequency_arm[7] = 0;                // modulation type  
    hci_set_tx_frequency_arm[8] = (carrier_on == 1) ? 8 : 0;
    hci_set_tx_frequency_arm[9] = tx_power;

    printf ("Sending HCI Command:\n");
    HexDump(hci_set_tx_frequency_arm, sizeof(hci_set_tx_frequency_arm));

    // write HCI reset
    SerialPort.Write(hci_set_tx_frequency_arm, sizeof(hci_set_tx_frequency_arm));

    // read HCI response header
    DWORD dwRead = SerialPort.Read((LPBYTE)&in_buffer[0], 3);

    // read HCI response payload
    if (dwRead == 3 && in_buffer[2] > 0)
        dwRead += SerialPort.Read((LPBYTE)&in_buffer[3], in_buffer[2]);

    printf ("Received HCI Event:\n");
    HexDump(in_buffer, dwRead);
    if (dwRead == sizeof(hci_set_tx_frequency_arm_cmd_complete_event))
    {
        if (memcmp(in_buffer, hci_set_tx_frequency_arm_cmd_complete_event, dwRead) == 0)
        {
            printf ("Success\n");
            return 1;
        }
    }
    return FALSE;
}

int _tmain(int argc, _TCHAR* argv[])
{
    int chan_num = 0;
    int pattern = 0;
    int length = 0;

    if ((argc >= 2) && (_stricmp(argv[1], "reset") == 0))
    {
        if (argc == 3)
        {
            return (execute_reset(argv[2]));
        }
        print_usage_reset(true);
        return 0;
    }
    else if ((argc >= 2) && (_stricmp(argv[1], "le_receiver_test") == 0))
    {
        if (argc == 4)
        {
            chan_num = atoi(argv[3]);
            if ((chan_num >= 0) && (chan_num <= 39))
            {
                return (execute_le_receiver_test(argv[2], chan_num));
            }
        }
        print_usage_le_receiver_test(true);
        return 0;
    }
    else if ((argc >= 2) && (_stricmp(argv[1], "le_test_end") == 0))
    {
        if (argc == 3)
        {
            return (execute_le_test_end(argv[2]));
        }
        print_usage_le_test_end(true);
        return 0;
    }
    else if ((argc >= 2) && (_stricmp(argv[1], "le_transmitter_test") == 0))
    {
        if (argc == 6)
        {
            chan_num = atoi(argv[3]);
            if ((chan_num >= 0) && (chan_num <= 39))
            {
                length = atoi(argv[4]);
                if ((length > 0) && (chan_num <= 255))
                {
                    pattern = atoi(argv[5]);
                    if ((pattern >= 0) && (pattern < 7))
                    {
                        return (execute_le_transmitter_test(argv[2], chan_num, length, pattern));
                    }
                }
            }
        }
        print_usage_le_transmitter_test(true);
        return 0;
    }
    else if ((argc >= 2) && (_stricmp(argv[1], "set_tx_frequency_arm") == 0))
    {
        if (argc >= 3)
        {
            UINT8 carrier_on = atoi(argv[3]);
            if ((carrier_on == 0) || (carrier_on == 1))
            {
                if (carrier_on == 0)
                {
                    return execute_set_tx_frequency_arm(argv[2], carrier_on, 2402, 0);
                }
                else if (argc == 6)
                {
                    int tx_frequency = atoi(argv[4]);
                    if ((tx_frequency >= 2402) && (chan_num <= 2480))
                    {
                        int tx_power = atoi(argv[5]);
                        if ((tx_power >= -25) && (tx_power <= 3))
                        {
                            return execute_set_tx_frequency_arm(argv[2], carrier_on, tx_frequency, tx_power);
                        }
                    }
                }
            }
        }
        print_usage_set_tx_frequency_arm(true);
        return 0;
    }
    else
    {
        printf("Usage: mbt help\n");
        print_usage_reset(false);
        print_usage_le_receiver_test(false);
        print_usage_le_transmitter_test(false);
        print_usage_le_test_end(false);
        print_usage_set_tx_frequency_arm(false);
        printf("Check Bluetooth Core 4.1 spec vol. 2 Sections 7.8.28-7.2.30\nfor details of LE Transmitter and Receiver tests");
    }
	return 0;
}


// location_and_navigation.h

#define LOCATION_AND_NAVIGATION_CP_CLIENT_CHAR_CONF_IMPROPERLY   0xfd
#define LOCATION_AND_NAVIGATION_CP_PROCEDURE_ALREADY_IN_PROGRESS 0xfe

enum ln_ln_feature_bits
{
    LN_INSTANTANEOUS_SPEED_SUPPORTED                     = 0x00000001,
    LN_TOTAL_DISTANCE_SUPPORTED                          = 0x00000002,
    LN_LOCATION_SUPPORTED_SUPPORTED                      = 0x00000004,
    LN_ELEVATION_SUPPORTED                               = 0x00000008,
    LN_HEADING_SUPPORTED                                 = 0x00000010,
    LN_ROLLING_TIME_SUPPORTED                            = 0x00000020,
    LN_UTC_TIME_SUPPORTED                                = 0x00000040,
    LN_REMAINING_DISTANCE_SUPPORTED                      = 0x00000080,
    LN_REMAINING_VERTICAL_DISTANCE_SUPPORTED             = 0x00000100,
    LN_ESTIMATED_TIME_OF_ARRIVAL_SUPPORTED               = 0x00000200,
    LN_NUMBER_OF_BEACONS_IN_SOLUTION_SUPPORTED           = 0x00000400,
    LN_NUMBER_OF_BEACONS_IN_VIEW_SUPPORTED               = 0x00000800,
    LN_TIME_TO_FIRST_FIX_SUPPORTED                       = 0x00001000,
    LN_ESTIMATED_HORIZONTAL_POSITION_ERROR_SUPPORTED     = 0x00002000,
    LN_ESTIMATED_VERTICAL_POSITION_ERROR_SUPPORTED       = 0x00004000,
    LN_HORIZONTAL_DILUTION_OF_PRECISION_SUPPORTED        = 0x00008000,
    LN_VERTICAL_DILUTION_OF_PRECISION_SUPPORTED          = 0x00010000,
    LN_LOCATION_AND_SPEED_CHAR_CONTENT_MASKING_SUPPORTED = 0x00020000,
    LN_FIX_RATE_SETTING_SUPPORTED                        = 0x00040000,
    LN_ELEVATION_SETTING_SUPPORTED                       = 0x00080000,
    LN_POSITION_STATUS_SUPPORTED                         = 0x00100000,
};

enum ln_las_flags
{
    LN_LAS_INSTANTANEOUS_SPEED_PRESENT = 0x0001,
    LN_LAS_TOTAL_DISTANCE_PRESENT      = 0x0002,
    LN_LAS_LOCATION_PRESENT            = 0x0004,
    LN_LAS_EVEVATION_PRESENT           = 0x0008,
    LN_LAS_HEADING_PRESENT             = 0x0010,
    LN_LAS_ROLLING_TIME_PRESENT        = 0x0020,
    LN_LAS_UTC_TIME_PRESENT            = 0x0040,
    LN_LAS_POSITION_STATUS             = 0x0180,
    LN_LAS_SPEED_AND_DISTANCE_FORMAT   = 0x0200,
    LN_LAS_ELEVATION_SOURCE            = 0x0c00,
    LN_LAS_HEADING_SOURCE              = 0x1000,
};

enum ln_position_status
{
    LN_POSITION_STATUS_NO_POSITION         = 0,
    LN_POSITION_STATUS_POSITION_OK         = 1,
    LN_POSITION_STATUS_ESTIMATED_POSITION  = 2,
    LN_POSITION_STATUS_LAST_KNOWN_POSITION = 3,
};

enum ln_speed_and_distance_format
{
    LN_SPEED_AND_DISTANCE_FORMAT_2D = 0,
    LN_SPEED_AND_DISTANCE_FORMAT_3D = 1,
};

enum ln_elevation_source
{
    LN_ELEVATION_SOURCE_POSITIONING_SYSTEM      = 0,
    LN_ELEVATION_SOURCE_BAROMETRIC_AIR_PRESSURE = 1,
    LN_ELEVATION_SOURCE_DATABASE_SERVICE        = 2,
    LN_ELEVATION_SOURCE_OTHER                   = 3,
};

enum ln_heading_source
{
    LN_HEADING_SOURCE_HEADING_BASED_ON_MOVEMENT         = 0,
    LN_HEADING_SOURCE_HEADING_BASED_ON_MAGNETIC_COMPASS = 1,
};

enum ln_pq_flags
{
    LN_PQ_NUMBER_OF_BEACONS_IN_SOLUTION_PRESENT = 0x0001,
    LN_PQ_NUMBER_OF_BEACONS_IN_VIEW_PRESENT     = 0x0002,
    LN_PQ_TIME_TO_FIRST_FIX_PRESENT             = 0x0004,
    LN_PQ_EHPE_PRESENT                          = 0x0008,
    LN_PQ_EVPE_PRESENT                          = 0x0010,
    LN_PQ_HDOP_PRESENT                          = 0x0020,
    LN_PQ_VDOP_PRESENT                          = 0x0040,
    LN_PQ_POSITION_STATUS                       = 0x0180,
};

typedef UINT8 UINT24[3];

#pragma pack(1)
typedef PACKED struct
{
    UINT16 year;
    UINT8  month;
    UINT8  day;
    UINT8  hours;
    UINT8  minutes;
    UINT8  seconds;
} LN_UTC_TIME;

typedef PACKED struct
{
    UINT16 flags;
    UINT16 instantaneous_speed;
    UINT24 total_distance;
    UINT32 latitude;
    UINT32 longitude;
    UINT24 elevation;
    UINT16 heading;
    //UINT8  rolling_time;
    //LN_UTC_TIME utc_time;
}  LN_LAS_DATA;

typedef PACKED struct
{
    UINT16 flags;
    UINT8  number_of_beacons_in_solution;
    UINT8  number_of_beacons_in_view;
    //UINT16 time_to_first_fix;
    //UINT32 ehpe;
    //UINT32 evpe;
    //UINT8  hdop;
    //UINT8  vdop;
}  LN_PQ_DATA;
#pragma pack()

//CP Op Code
enum ln_cp_opcode
{
    LN_CP_SET_CUMULATIVE_VALUE                 = 0x01,
    LN_CP_MASK_LOCATION_AND_SPEED_CHAR_CONTENT = 0x02,
    LN_CP_NAVIGATION_CONTROL                   = 0x03,
    LN_CP_REQUEST_NUMBER_OF_ROUTES             = 0x04,
    LN_CP_REQUEST_NAME_OF_ROUTE                = 0x05,
    LN_CP_SELECT_ROUTE                         = 0x06,
    LN_CP_SET_FIX_RATE                         = 0x07,
    LN_CP_SET_ELEVATION                        = 0x08,
    LN_CP_RESPONSE_CODE                        = 0x20,
};

// CP Response Values
enum ln_cp_responsevalue
{
    LN_CP_SUCCESS              = 0x01,
    LN_CP_OPCODE_NOT_SUPPORTED = 0x02,
    LN_CP_INVALID_PARAMETER    = 0x03,
    LN_CP_OPERATION_FAILED     = 0x04,
};

#pragma pack(1)
typedef PACKED struct
{
    UINT8 opcode;
    UINT8 parameter[18];
} LN_CP_HDR;

typedef PACKED struct
{
    UINT8 opcode;
    UINT8 request_opcode;
    UINT8 response_value;
    UINT8 response_parameter[16];
} LN_CP_RSP_HDR;
#pragma pack()

enum ln_navi_flags
{
    LN_NAVI_REMAINING_DISTANCE_PRESENT          = 0x0001,
    LN_NAVI_REMAINING_VERTICAL_DISTANCE_PRESENT = 0x0002,
    LN_NAVI_ESTIMATED_TIME_OF_ARRIVAL_PRESENT   = 0x0004,
    LN_NAVI_POSITION_STATUS                     = 0x0018,
    LN_NAVI_HEADING_SOURCE                      = 0x0020,
    LN_NAVI_NAVIGATION_INDICATOR_TYPE           = 0x0040,
    LN_NAVI_WAYPOINT_REACHED                    = 0x0080,
    LN_NAVI_DESTINATION_REACHED                 = 0x0100,
};

#pragma pack(1)
typedef PACKED struct
{
    UINT16 flags;
    UINT16 bearing;
    UINT16 heading;
    UINT24 remaining_distance;
    UINT24 remaining_vertical_distance;
    //LN_UTC_TIME estimated_time_of_arrival;
}  LN_NAVI_DATA;
#pragma pack()

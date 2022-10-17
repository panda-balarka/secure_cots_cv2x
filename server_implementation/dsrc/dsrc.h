#ifndef __TEMP_H
#define __TEMP_H

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int int16_t;
typedef unsigned short int uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t; 
typedef signed long int int64_t;
typedef unsigned long int uint64_t;

#define NULL_PTR NULL

// ASN1 TAG DEFINES
/*  every tag has a length byte. Since current data is mostly limited to size, 0x00-0xFF the length byte
    byte is always 1 (hence it is hardcoded) */
#define DSRC_BER_LENGTH_TAG_LENGTH  0x01
#define DSRC_BER_HEAD_TAG_LENGTH    0x01
#define DSRC_BER_SEQ_TAG_LENGTH     0x01
#define DSRC_BER_ELEM_TAG_LENGTH    0x01

#define DSRC_BER_ELEMENT_INCR_VAL   0x01

#define DSRC_BER_HEAD_TAG           0x30
#define DSRC_BER_SEQ_TAG            0xAD 
#define DSRC_ELEMENT_START_TAG      0x80

#define DSRC_BER_PAYLOAD_LENGTH_POS 0x01

enum BER_Extract_enum{
    // generateBSM_data actions
    BER_HEAD,
    BER_SEQUENCE,
    BER_ELEMENT,
    BER_LENGTH_UPDATE,

    //getBET_elementData actions
    BER_VALIDATE_LOAD_ELEMENT,
    BER_VALIDATE_LOAD_SEQUENCE
};

enum DSRC_MessageType_enum{
    RESERVED_DSRC,
    ALACARTEMESSAGE_DSRC,
    BSM_DSRC,
    BSM_VERBOSE_DSRC,
    CSM_DSRC,
    EVA_DSRC,
    ICA_DSRC,
    MAPDATA_DSRC,
    NMEACORRECTIONS_DSRC,
    PDM_DSRC,
    PVD_DSRC,
    RSA_DSRC,
    RTCMCORRECTIONS_DSRC,
    SPTM_DSRC,
    SRM_DSRC,
    SSM_DSRC,
    TRAVELERINFORMATION_DSRC
};

typedef struct __attribute__((packed)){
    uint8_t width;
    uint8_t length;
}bsmVehicleSize_St;

typedef struct __attribute__((aligned(4))){
    uint8_t  dsrc_msgtype;  // 80
    uint8_t  bsm_msgcnt;    // 81
    uint32_t bsm_id;        // 82
    uint8_t  bsm_secMark;   // 83
    uint32_t bsm_lat;       // 84   
    uint32_t bsm_long;      // 85
    uint16_t bsm_elev;      // 86
    uint32_t bsm_positionalAccuracy;    // 87
    uint16_t bsm_speed;     // 88
    uint16_t bsm_heading;   // 89
    uint8_t  bsm_steeringWheelAngle;    // 8A
    uint8_t  bsm_accelSet[7];   // 8B
    uint16_t bsm_brakes;        // 8C
    bsmVehicleSize_St bsm_size; // 8D
}basicSafetyMessageVerbose_st;

typedef union{
    // reserved_st;
    // bsm_st;
    basicSafetyMessageVerbose_st bsmv_st;
    // add other dsrc_message structures as required
}dsrcMessage_u;

int TA_ASN1_ENCODER(uint8_t msgType_u8,basicSafetyMessageVerbose_st *bsm_tst, uint8_t *asnBuff_au8, uint16_t *asn_len);
int generateBSM_data_verbose_DSRC(uint8_t *asnBuff, uint16_t *buffsize, 
                                    uint32_t msgcnt_e, uint32_t lat_e, uint32_t long_e, uint16_t elev_e);

#endif 
#include <stdio.h>
#include <stdlib.h>
#include "dsrc.h"

void memcpy(void *dest_p,void *src_p, size_t n)
{
    // Typecast src and dest addresses to (char *)
    uint8_t *csrc = (char *)src_p;
    uint8_t *cdest = (char *)dest_p;
  
    // Copy contents of src[] to dest[]
    for (uint8_t i=0; i<n; i++)
       cdest[i] = csrc[i];
}

void memset(void *dest_p,uint8_t byte_u8, size_t n)
{
    // Typecast dest addresses to (char *)
    uint8_t *cdest = (char *)dest_p;
  
    // Copy contents of src[] to dest[]
    for (uint8_t i=0; i<n; i++)
       cdest[i] = byte_u8;
}

// Temp API: Replace with FX that gets value from GPS module and 
int generateBSM_data_verbose_DSRC(uint8_t *asnBuff_au8, uint16_t *buffsize, 
                                    uint32_t msgcnt_e, uint32_t lat_e, uint32_t long_e, uint16_t elev_e)
{

    uint8_t tempArray[7] = {0x01,0x23,0x45,0x60,0x12,0x34,0x56};
    basicSafetyMessageVerbose_st *bsm_tst = (basicSafetyMessageVerbose_st*) malloc(sizeof(basicSafetyMessageVerbose_st));

    bsm_tst->dsrc_msgtype = BSM_VERBOSE_DSRC;
    bsm_tst->bsm_msgcnt = msgcnt_e;
    bsm_tst->bsm_id = 0x65123456;
    bsm_tst->bsm_secMark = 1;
    bsm_tst->bsm_lat = lat_e;
    bsm_tst->bsm_long = long_e;
    bsm_tst->bsm_elev = elev_e;
    bsm_tst->bsm_positionalAccuracy = 0xFE01FFFF;
    bsm_tst->bsm_speed = 0xFE10;
    bsm_tst->bsm_heading = 0x0040;
    bsm_tst->bsm_steeringWheelAngle = 0x01;
    memcpy(bsm_tst->bsm_accelSet,tempArray,7);
    bsm_tst->bsm_brakes = 0xFFFF;
    bsm_tst->bsm_size.width = 0x01;
    bsm_tst->bsm_size.length = 0x05;

    TA_ASN1_ENCODER(BSM_VERBOSE_DSRC, bsm_tst, asnBuff_au8, buffsize);
    free(bsm_tst);

    // remove structure used and clear space from the heap
    //for (int i = 0; i < *buffsize; i++)
        //printf("%02x ",asnBuff_au8[i]);    

    return 0;
}

// Temp API: Replace with FX that receives ASN.1 data from the HMAC verification APi 
// after data is received from the baseband modem
int getBSM_data(uint8_t *asnBuff_au8,uint8_t bufferSize_u8)
{
    // Handle buffer size mismatches later while copying
    uint8_t tempArray_au8[100] = {
                                  0x30, 0x40, 0x80, 0x01, 0x03, 0x81, 0x01, 0x01, 0x82, 0x04, 0x56,
    0x34, 0x12, 0x65, 0x83, 0x01, 0x01, 0x84, 0x02, 0x5a, 0x23, 0x85, 0x02, 0x64, 0x12, 0x86, 0x02,
    0x00, 0xF0, 0x87, 0x04, 0xff, 0xff, 0x01, 0xfe, 0x88, 0x02, 0x10, 0xfe, 0x89, 0x01, 0x40, 0x8a, 
    0x01, 0x01, 0x8b, 0x07, 0x01, 0x23, 0x45, 0x60, 0x12, 0x34, 0x56, 0x8c, 0x02, 0xff, 0xff, 0xad, 
    0x06, 0x80, 0x01, 0x01, 0x81, 0x01, 0x05
    };
    memcpy(asnBuff_au8,tempArray_au8,sizeof(tempArray_au8));
}

int updateASNBuffer(uint8_t *data_au8, uint16_t mxDataSize_u16, 
            uint8_t *asnBuffer_au8, uint16_t *idx_u16,uint8_t updateType, uint8_t *elem_tag_u8)
{

    uint16_t reqDataSize_u16 = mxDataSize_u16;

    if (updateType == BER_HEAD)
    {
        memcpy(&asnBuffer_au8[*idx_u16],data_au8,reqDataSize_u16);
        *idx_u16 += reqDataSize_u16 + DSRC_BER_LENGTH_TAG_LENGTH;
    }
    // If we have a new sequence within given sequence, reset Element tag to 0x80
    else if (updateType == BER_SEQUENCE)
    {
        *elem_tag_u8 = DSRC_ELEMENT_START_TAG;
        memcpy(&asnBuffer_au8[*idx_u16],data_au8,reqDataSize_u16);
        *idx_u16 += reqDataSize_u16 + DSRC_BER_LENGTH_TAG_LENGTH;
    }
    else if(updateType == BER_ELEMENT)
    {
        // use a flag to handle the 0x00 after non-zero bytes arising from endianess issue 
        uint8_t found_u8 = 0;
        asnBuffer_au8[*idx_u16] = *elem_tag_u8;
        *idx_u16 += DSRC_BER_ELEM_TAG_LENGTH;
        for (int ref=0; ref<mxDataSize_u16; ref++)
        {
            if (data_au8[ref] == 0x00 && found_u8)
            {
                reqDataSize_u16 -= 1;   
            }
            if (data_au8[ref] != 0x00 && !found_u8)
                found_u8 = 1;
        }
        asnBuffer_au8[*idx_u16] = reqDataSize_u16;
        *idx_u16 += DSRC_BER_LENGTH_TAG_LENGTH;
        *elem_tag_u8 += DSRC_BER_ELEMENT_INCR_VAL;
        memcpy(&asnBuffer_au8[*idx_u16],data_au8,reqDataSize_u16);
        *idx_u16 += reqDataSize_u16;        
    }
    else if(updateType == BER_LENGTH_UPDATE){
        asnBuffer_au8[mxDataSize_u16] = 0;
    }

    return 0;
}

/* API to perfrom BER encoding as per ASN1 schema */
int TA_ASN1_ENCODER(uint8_t msgType_u8,basicSafetyMessageVerbose_st *bsm_tst, uint8_t *asnBuff_au8, uint16_t *asn_len)
{
    // Main index to iterate over data
    uint16_t idx_u16 = 0;
    // Temporary buffer to load non-array values to use common API to update ASN1 buffer
    uint8_t temp_au8[10] =  {0};
    if (msgType_u8 == BSM_VERBOSE_DSRC)
    {
        uint8_t elem_tag_u8 = DSRC_ELEMENT_START_TAG;

        // Update BER packet head
        temp_au8[0] = DSRC_BER_HEAD_TAG;
        updateASNBuffer(temp_au8, DSRC_BER_HEAD_TAG_LENGTH, asnBuff_au8, &idx_u16, BER_HEAD, &elem_tag_u8);
        // Update the totalPacket size at the end based on other data as this varies based on data
        updateASNBuffer((uint8_t*)&(bsm_tst->dsrc_msgtype), sizeof(bsm_tst->dsrc_msgtype), asnBuff_au8, &idx_u16, BER_ELEMENT, &elem_tag_u8);
        updateASNBuffer((uint8_t*)&(bsm_tst->bsm_msgcnt), sizeof(bsm_tst->bsm_msgcnt), asnBuff_au8, &idx_u16, BER_ELEMENT, &elem_tag_u8);
        updateASNBuffer((uint8_t*)&(bsm_tst->bsm_id), sizeof(bsm_tst->bsm_id), asnBuff_au8, &idx_u16, BER_ELEMENT, &elem_tag_u8);
        updateASNBuffer((uint8_t*)&(bsm_tst->bsm_secMark), sizeof(bsm_tst->bsm_secMark), asnBuff_au8, &idx_u16, BER_ELEMENT, &elem_tag_u8);
        updateASNBuffer((uint8_t*)&(bsm_tst->bsm_lat), sizeof(bsm_tst->bsm_lat), asnBuff_au8, &idx_u16, BER_ELEMENT, &elem_tag_u8);
        updateASNBuffer((uint8_t*)&(bsm_tst->bsm_long), sizeof(bsm_tst->bsm_long), asnBuff_au8, &idx_u16, BER_ELEMENT, &elem_tag_u8);
        updateASNBuffer((uint8_t*)&(bsm_tst->bsm_elev), sizeof(bsm_tst->bsm_elev), asnBuff_au8, &idx_u16, BER_ELEMENT, &elem_tag_u8);
        updateASNBuffer((uint8_t*)&(bsm_tst->bsm_positionalAccuracy), sizeof(bsm_tst->bsm_positionalAccuracy), asnBuff_au8, &idx_u16, BER_ELEMENT, &elem_tag_u8);
        updateASNBuffer((uint8_t*)&(bsm_tst->bsm_speed), sizeof(bsm_tst->bsm_speed), asnBuff_au8, &idx_u16, BER_ELEMENT, &elem_tag_u8);
        updateASNBuffer((uint8_t*)&(bsm_tst->bsm_heading), sizeof(bsm_tst->bsm_heading), asnBuff_au8, &idx_u16, BER_ELEMENT, &elem_tag_u8);
        updateASNBuffer((uint8_t*)&(bsm_tst->bsm_steeringWheelAngle), sizeof(bsm_tst->bsm_steeringWheelAngle), asnBuff_au8, &idx_u16, BER_ELEMENT, &elem_tag_u8);
        updateASNBuffer((uint8_t*)&(bsm_tst->bsm_accelSet), sizeof(bsm_tst->bsm_accelSet), asnBuff_au8, &idx_u16, BER_ELEMENT, &elem_tag_u8);
        updateASNBuffer((uint8_t*)&(bsm_tst->bsm_brakes), sizeof(bsm_tst->bsm_brakes), asnBuff_au8, &idx_u16, BER_ELEMENT, &elem_tag_u8);
        // Create subseqeunce for vehicle size and take reference to update length byte
        temp_au8[0] =  DSRC_BER_SEQ_TAG;
        updateASNBuffer(temp_au8,DSRC_BER_SEQ_TAG_LENGTH,asnBuff_au8,&idx_u16,BER_SEQUENCE,&elem_tag_u8);
        uint8_t idx_ref_subseq1_u16 = idx_u16;
        updateASNBuffer((uint8_t*)&(bsm_tst->bsm_size.width), sizeof(bsm_tst->bsm_size.width), asnBuff_au8, &idx_u16, BER_ELEMENT, &elem_tag_u8);
        updateASNBuffer((uint8_t*)&(bsm_tst->bsm_size.length), sizeof(bsm_tst->bsm_size.length), asnBuff_au8, &idx_u16, BER_ELEMENT, &elem_tag_u8);
        // update subsequence length byte
        asnBuff_au8[idx_ref_subseq1_u16 - 1] = idx_u16 - idx_ref_subseq1_u16;

        // all subsequences and elements done, update packet body length byte
        asnBuff_au8[DSRC_BER_PAYLOAD_LENGTH_POS] = idx_u16-DSRC_BER_HEAD_TAG_LENGTH-DSRC_BER_SEQ_TAG_LENGTH;
        *asn_len = idx_u16;
        
    }
    return 0;
}


int getBER_elementData(uint8_t *dest_au8,uint8_t *asnBuffer_au8,uint8_t *idx_u8,
    uint8_t *elem_tag_u8,uint8_t loadType)
{
    if(loadType == BER_VALIDATE_LOAD_SEQUENCE)
    {
        if (asnBuffer_au8[*idx_u8]!=DSRC_BER_SEQ_TAG)
        {
            printf("Error!, Mismatch in sequence tag\n");
            return 0;
        }
        *idx_u8 +=DSRC_BER_SEQ_TAG_LENGTH+DSRC_BER_LENGTH_TAG_LENGTH;
        // reset subsequence tag
        *elem_tag_u8 = DSRC_ELEMENT_START_TAG;
    }
    else if(loadType == BER_VALIDATE_LOAD_ELEMENT)
    {
        if (asnBuffer_au8[*idx_u8]!=*elem_tag_u8)
        {
            printf("Error!, Mismatch in element sequence\n");
            return 0;
        }

        *idx_u8 += DSRC_BER_ELEM_TAG_LENGTH;
        uint8_t dataSize_u8 = asnBuffer_au8[*idx_u8];
        *idx_u8 += DSRC_BER_LENGTH_TAG_LENGTH;
        memcpy(dest_au8,&asnBuffer_au8[*idx_u8],dataSize_u8);
        *elem_tag_u8 += DSRC_BER_ELEMENT_INCR_VAL;
        *idx_u8 += dataSize_u8;
        return 0;        
    }


}

int TA_ASN1_DECODER(uint8_t msgType_u8)
{   
    uint8_t asnBuff_au8[100] = {0};
    uint8_t bufferSize_u8 = sizeof(asnBuff_au8);
    if(msgType_u8 == BSM_VERBOSE_DSRC)
    {
        uint8_t payload_len_u8;
        basicSafetyMessageVerbose_st *bsm_tst = (basicSafetyMessageVerbose_st*) malloc(sizeof(basicSafetyMessageVerbose_st));
        getBSM_data(asnBuff_au8,bufferSize_u8);

        if (asnBuff_au8[0] != DSRC_BER_HEAD_TAG)
        {
            printf("Error: Data is not BER encoded. Found Unknown BER_HEAD_TAG");
            return 0;
        }
        
        payload_len_u8 = asnBuff_au8[DSRC_BER_PAYLOAD_LENGTH_POS];
        uint8_t elem_tag_u8 = DSRC_ELEMENT_START_TAG;
        uint8_t tempBuff_u8[10] = {0};
        // the first two bytes have already been used, so we start after the payload length byte
        uint8_t idx_u16 = DSRC_BER_HEAD_TAG_LENGTH + DSRC_BER_LENGTH_TAG_LENGTH;
        // load the element details to the structure
        getBER_elementData((uint8_t*)&(bsm_tst->dsrc_msgtype),asnBuff_au8,&idx_u16,&elem_tag_u8,BER_VALIDATE_LOAD_ELEMENT);
        getBER_elementData((uint8_t*)&(bsm_tst->bsm_msgcnt),asnBuff_au8,&idx_u16,&elem_tag_u8,BER_VALIDATE_LOAD_ELEMENT);
        getBER_elementData((uint8_t*)&(bsm_tst->bsm_id),asnBuff_au8,&idx_u16,&elem_tag_u8,BER_VALIDATE_LOAD_ELEMENT);
        getBER_elementData((uint8_t*)&(bsm_tst->bsm_secMark),asnBuff_au8,&idx_u16,&elem_tag_u8,BER_VALIDATE_LOAD_ELEMENT);
        getBER_elementData((uint8_t*)&(bsm_tst->bsm_lat),asnBuff_au8,&idx_u16,&elem_tag_u8,BER_VALIDATE_LOAD_ELEMENT);
        getBER_elementData((uint8_t*)&(bsm_tst->bsm_long),asnBuff_au8,&idx_u16,&elem_tag_u8,BER_VALIDATE_LOAD_ELEMENT);
        getBER_elementData((uint8_t*)&(bsm_tst->bsm_elev),asnBuff_au8,&idx_u16,&elem_tag_u8,BER_VALIDATE_LOAD_ELEMENT);
        getBER_elementData((uint8_t*)&(bsm_tst->bsm_positionalAccuracy),asnBuff_au8,&idx_u16,&elem_tag_u8,BER_VALIDATE_LOAD_ELEMENT);
        getBER_elementData((uint8_t*)&(bsm_tst->bsm_speed),asnBuff_au8,&idx_u16,&elem_tag_u8,BER_VALIDATE_LOAD_ELEMENT);
        getBER_elementData((uint8_t*)&(bsm_tst->bsm_heading),asnBuff_au8,&idx_u16,&elem_tag_u8,BER_VALIDATE_LOAD_ELEMENT);
        getBER_elementData((uint8_t*)&(bsm_tst->bsm_steeringWheelAngle),asnBuff_au8,&idx_u16,&elem_tag_u8,BER_VALIDATE_LOAD_ELEMENT);        
        getBER_elementData((uint8_t*)&(bsm_tst->bsm_accelSet),asnBuff_au8,&idx_u16,&elem_tag_u8,BER_VALIDATE_LOAD_ELEMENT);
        getBER_elementData((uint8_t*)&(bsm_tst->bsm_brakes),asnBuff_au8,&idx_u16,&elem_tag_u8,BER_VALIDATE_LOAD_ELEMENT);
        getBER_elementData(NULL_PTR,asnBuff_au8,&idx_u16,&elem_tag_u8,BER_VALIDATE_LOAD_SEQUENCE);
        getBER_elementData((uint8_t*)&(bsm_tst->bsm_size.width),asnBuff_au8,&idx_u16,&elem_tag_u8,BER_VALIDATE_LOAD_ELEMENT);
        getBER_elementData((uint8_t*)&(bsm_tst->bsm_size.length),asnBuff_au8,&idx_u16,&elem_tag_u8,BER_VALIDATE_LOAD_ELEMENT);                

        free(bsm_tst);
    }
}

/*
int main()
{
    TA_ASN1_ENCODER(BSM_VERBOSE_DSRC);
    printf("\n");
    TA_ASN1_DECODER(BSM_VERBOSE_DSRC);
    printf("\n");
    return 0;
}*/

/*
int main()
{
    generateBSM_data_verbose_DSRC(0x01, 0x235A, 0x4664, 0xF000);
}*/

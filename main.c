/*******************************************************************************
 *         Copyright (c), NXP Semiconductors Gratkorn / Austria
 *
 *                     (C)NXP Semiconductors
 *       All rights are reserved. Reproduction in whole or in part is
 *      prohibited without the written consent of the copyright owner.
 *  NXP reserves the right to make changes without notice at any time.
 * NXP makes no warranty, expressed, implied or statutory, including but
 * not limited to any implied warranty of merchantability or fitness for any
 *particular purpose, or that the use will not infringe any third party patent,
 * copyright or trademark. NXP must not be liable for any loss or damage
 *                          arising from its use.
 ********************************************************************************
 *
 * Filename:          main.c
 * Processor family:  LPC122x, LPC17xx
 *
 * Description: This file contains main entry.
 *
 *******************************************************************************/


#include <stdio.h>

#define DEBUG
/**
 * Header for hardware configuration: bus interface, reset of attached reader ID, onboard LED handling etc.
 * */
/* Configuration of hardware platform */
#include <phhwConfig.h>

/**
 * Reader Library Headers
 */
/* Status code definitions */
#include <ph_Status.h>
/* Generic ISO14443-3A Component of
 * Reader Library Framework */
#include <phpalI14443p3a.h>
/* Generic ISO14443-3B Component of
 * Reader Library Framework */
#include <phpalI14443p3b.h>
/* Generic ISO14443-4 Component of
 * Reader Library Framework */
#include <phpalI14443p4.h>
#include <phpalI14443p4a.h>
/* Generic Felica Component of
 * Reader Library Framework */
#include <phpalFelica.h>
/* Generic ISO18092 passive initiator
 * mode Component of Reader Library Framework. */
#include <phpalI18092mPI.h>
/* Generic Discovery Loop Activities
 * Component of Reader Library Framework */
#include <phacDiscLoop.h>
/* Generic BAL Component of
 * Reader Library Framework */
#include <phbalReg.h>
/* Generic OSAL Component of
 * Reader Library Framework */
#include <phOsal.h>
/* Generic LLCP Link layer Component of
 * Reader Library Framework */
#include <phlnLlcp.h>
/* Generic Tag Operation Application Layer
 * Component of Reader Library Framework. */
#include <phalTop.h>
/* Generic MIFARE(R) Component of
 * Reader Library Framework. */
#include <phpalMifare.h>
/* Generic MIFARE(R) Ultralight Application
 * Component of Reader Library Framework. */
#include <phalMful.h>
/* Generic MIFARE DESFire(R) EV1 Application
 * Component of Reader Library Framework.*/
#include <phalMfdf.h>
/* Generic Felica Application
 * Component of Reader Library Framework.*/
#include <phalFelica.h>
/* Generic MIFARE(R) Application
 * Component of Reader Library Framework.*/
#include <phalMfc.h>
/* Generic KeyStore Component of
 * Reader Library Framework.*/
#include <phKeyStore.h>

/* Printf macro */
#define  DEBUG_PRINTF(...) printf(__VA_ARGS__)
#define   DEBUG_FLUSH(x)      {printf("%c", x);}

#define PRETTY_PRINTING              /**< Enable pretty printing */
#define DISCOVERY_MODE     PHAC_DISCLOOP_SET_POLL_MODE | PHAC_DISCLOOP_SET_PAUSE_MODE     /**< Enable Poll and Pause mode  */
#define POLL_TYPE          PHAC_DISCLOOP_CON_POLL_F | PHAC_DISCLOOP_CON_POLL_A | PHAC_DISCLOOP_CON_POLL_B   /**< Enable Technology type */

#define NUMBER_OF_KEYENTRIES        2
#define NUMBER_OF_KEYVERSIONPAIRS   2
#define NUMBER_OF_KUCENTRIES        1

#define DATA_BUFFER_LEN             20

/* prints if error is detected */
#define CHECK_STATUS(x)                                      \
        if ((x) != PH_ERR_SUCCESS)                               \
        {                                                            \
            DEBUG_PRINTF("Line: %d   Error - (0x%04X) has occurred : 0xCCEE CC-Component ID, EE-Error code. Refer-ph_Status.h\n", __LINE__, (x));    \
        }

/* Returns if error is detected */
#define CHECK_SUCCESS(x)              \
        if ((x) != PH_ERR_SUCCESS)        \
        {                                     \
            DEBUG_PRINTF("\nLine: %d   Error - (0x%04X) has occurred : 0xCCEE CC-Component ID, EE-Error code. Refer-ph_Status.h\n ", __LINE__, (x)); \
            return x;                         \
        }

uint8_t                            bHalBufferTx[256];         /* HAL  TX buffer. Size 256 - Based on maximum FSL */
uint8_t                            bHalBufferRx[256];         /* HAL  RX buffer. Size 256 - Based on maximum FSL */
uint8_t                            bAtr_Res[30];              /* ATR  response holder */
uint8_t                            bAtr_ResLength;            /* ATR  response length */
uint8_t                            bSak;                      /* SAK  card type information */
uint16_t                           wAtqa;                     /* ATQA card type information */
#if defined NXPBUILD__PHHAL_HW_RC523
phhalHw_Rc523_DataParams_t         halReader;                 /* HAL  component holder */
#endif
#if defined NXPBUILD__PHHAL_HW_RC663
phhalHw_Rc663_DataParams_t         halReader;                 /* HAL  component holder */
#endif
phpalI14443p3a_Sw_DataParams_t     palI14443p3a;              /* PAL  I14443-A component */
phpalI14443p4a_Sw_DataParams_t     palI14443p4a;              /* PAL  I14443-4A component */
phpalI14443p3b_Sw_DataParams_t     palI14443p3b;              /* PAL  I14443-B component */
phpalI14443p4_Sw_DataParams_t      palI14443p4;               /* PAL  I14443-4 component */
phpalFelica_Sw_DataParams_t        palFelica;                 /* PAL  Felica component */
phpalI18092mPI_Sw_DataParams_t     palI18092mPI;              /* PAL  MPI component */
phpalMifare_Sw_DataParams_t        palMifare;                 /* PAL  Mifare component */
phalMful_Sw_DataParams_t           alMful;                    /* AL   Mifare UltraLite component */
phalMfdf_Sw_DataParams_t           alMfdf;                    /* AL   Mifare Desfire component */
phalFelica_Sw_DataParams_t         alFelica;                  /* AL   Felica component */
phOsal_Stub_DataParams_t           osal;                      /* OSAL component holder */
phacDiscLoop_Sw_DataParams_t       discLoop;                  /* Discovery loop component */
phbalReg_Stub_DataParams_t         balReader;                 /**< BAL component holder */
void                              *pHal;                      /* HAL pointer */
uint8_t                            bDataBuffer[DATA_BUFFER_LEN];  /* universal data buffer */
phalMfc_Sw_DataParams_t            alMfc;
phKeyStore_Sw_DataParams_t         SwkeyStore;
phKeyStore_Sw_KeyEntry_t           pKeyEntries[NUMBER_OF_KEYENTRIES];
phKeyStore_Sw_KeyVersionPair_t     pKeyVersionPairs[NUMBER_OF_KEYVERSIONPAIRS * NUMBER_OF_KEYENTRIES];
phKeyStore_Sw_KUCEntry_t           pKUCEntries[NUMBER_OF_KUCENTRIES];


/***********************************************************************************************
 * \brief     Print a given array of integers on the console
 *
 **********************************************************************************************/
#ifdef   DEBUG
static void PRINT_BUFF(uint8_t *hex, uint8_t num)
    {
    uint32_t   i;

    for(i = 0; i < num; i++)
        {
        DEBUG_PRINTF(" %02X",hex[i]);
        }
    }
#else
#define  PRINT_BUFF(x, y)
#endif /* DEBUG */


#define NUMBER_OF_KEYENTRIES        2
#define NUMBER_OF_KEYVERSIONPAIRS   2
#define NUMBER_OF_KUCENTRIES        1

// Forward declarations
static void Fill_Block (uint8_t *pBlock, uint8_t MaxNr);

/* Set the key for the Mifare (R) Classic cards. */
static /* const */ uint8_t Key[6] = {0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU};

// Don't change the following line
static /* const */ uint8_t Original_Key[6] = {0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU};

/***********************************************************************************************
 * \brief     Prints the UID.
 *
 * \param     uid     Array of UID data to be printed in hex
 * \param     uidLen  Length of the UID array
 *
 **********************************************************************************************/
void printUid( uint8_t   *uid,
        uint8_t    uidLen)
    {
    uint8_t i;

    for (i=0; i<uidLen; i++)
        {
        DEBUG_PRINTF("%02X", uid[i]);
        }
    DEBUG_PRINTF(" ");
    }

/***********************************************************************************************
 * \brief      This function is just used to generate some example data for write operations
 *             on the card.
 *
 **********************************************************************************************/
static void Fill_Block(uint8_t *pBlock, uint8_t MaxNr)
    {
    uint8_t i;

    for (i = 0; i <= MaxNr; i++)
        {
        *pBlock++ = i + 0x20;
        }
    }

/***********************************************************************************************
 * \brief      Initializes the discover loop component
 *             DataParams: representing the discovery loop
 *
 **********************************************************************************************/
phStatus_t DiscLoopInit( phacDiscLoop_Sw_DataParams_t   *pDataParams )
    {
    phStatus_t status;

    /* Set for poll and listen mode */
    status = phacDiscLoop_SetConfig( pDataParams,
            PHAC_DISCLOOP_CONFIG_MODE,
            DISCOVERY_MODE );
    CHECK_SUCCESS(status);

    /* Set for detection of TypeA, TypeB and Type F tags */
    status = phacDiscLoop_SetConfig(
            pDataParams,
            PHAC_DISCLOOP_CONFIG_DETECT_TAGS,
            POLL_TYPE
    );
    CHECK_SUCCESS(status);

    status = phacDiscLoop_SetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_PAUSE_PERIOD_MS, 1000);
    CHECK_SUCCESS(status);

    /* Set number of polling loops to 5 */
    status = phacDiscLoop_SetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_NUM_POLL_LOOPS, 5);
    CHECK_SUCCESS(status);

    /* Configure felica discovery */
    /* Set the system code to 0xffff */
    status = phacDiscLoop_SetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_TYPEF_SYSTEM_CODE, 0xffff);
    CHECK_SUCCESS(status);
    /* Set the maximum number of Type F tags to be detected to 3 */
    status = phacDiscLoop_SetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_TYPEF_DEVICE_LIMIT, 3);
    CHECK_SUCCESS(status);
    /* Set the polling limit for Type F tags to 5 */
    status = phacDiscLoop_SetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_TYPEF_POLL_LIMIT, 5);
    CHECK_SUCCESS(status);

    /* Assign ATR response */
    discLoop.sTypeFTargetInfo.sTypeF_P2P.pAtrRes = bAtr_Res;
    /* Set ATR response length */
    status = phacDiscLoop_SetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_TYPEF_P2P_ATR_RES_LEN, bAtr_ResLength);

    /* Configure Type B tag discovery */
    /* Set slot coding number to 0 */
    status = phacDiscLoop_SetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_TYPEB_NCODING_SLOT, 0);
    CHECK_SUCCESS(status);
    /* Set AFI to 0, Let all TypeB tags in field respond */
    status = phacDiscLoop_SetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_TYPEB_AFI_REQ, 0);
    CHECK_SUCCESS(status);
    /* Disable extended ATQB response */
    status = phacDiscLoop_SetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_TYPEB_EXTATQB, 0);
    CHECK_SUCCESS(status);
    /* Set poll limit for Type B tags to 5 */
    status = phacDiscLoop_SetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_TYPEB_POLL_LIMIT, 5);
    CHECK_SUCCESS(status);

    /* Set LRI to 3 */
    status = phacDiscLoop_SetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_TYPEA_P2P_LRI, 3);
    CHECK_SUCCESS(status);
    /* Set DID to 3 */
    status = phacDiscLoop_SetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_TYPEA_P2P_DID, 0);
    CHECK_SUCCESS(status);
    /* Disable NAD */
    status = phacDiscLoop_SetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_TYPEA_P2P_NAD_ENABLE, PH_OFF);
    CHECK_SUCCESS(status);
    /* Clear NAD info */
    status = phacDiscLoop_SetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_TYPEA_P2P_NAD, 0);
    CHECK_SUCCESS(status);
    /* Assign ATR response */
    discLoop.sTypeATargetInfo.sTypeA_P2P.pAtrRes = bAtr_Res;
    /* Set ATR response length */
    status = phacDiscLoop_SetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_TYPEA_P2P_ATR_RES_LEN, bAtr_ResLength);

    /* Bail out on Type A detect */
    discLoop.bBailOut = PHAC_DISCLOOP_CON_BAIL_OUT_A;
    return PH_ERR_SUCCESS;
    }


/***********************************************************************************************
 * This function demonstrates the usage of discovery loop
 * It detects and displays the type of card and UID
 * In case of P2P device, it enables LLCP and transmits the message
 *
 * \param   pDataParams      The discovery loop data parameters
 *
 * \note   This function will never return
 *
 **********************************************************************************************/
phStatus_t DiscLoopDemo( phacDiscLoop_Sw_DataParams_t  *pDataParams )
    {
    phStatus_t    status=0;
    uint16_t      wTagsDetected=0;
    uint32_t      loop = 0;
    uint16_t      wNumberOfTags=0;
    uint8_t       bGtLen=0;    /* Gt length */
    uint8_t       bUid[PHAC_DISCLOOP_I3P3A_MAX_UID_LENGTH];
    uint8_t       bUidSize;

    PH_UNUSED_VARIABLE(bGtLen);

    while(1)
        {
        DEBUG_PRINTF("Ready to detect.");
        DEBUG_FLUSH('\n');

        status = phhalHw_FieldOff(pHal);
        status = phacDiscLoop_Start(pDataParams);

        if ((status & PH_ERR_MASK) == PH_ERR_SUCCESS)
            {
            /* Get the tag types detected info */
            status = phacDiscLoop_GetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_TAGS_DETECTED, &wTagsDetected);

            if (((status & PH_ERR_MASK) != PH_ERR_SUCCESS) || wTagsDetected == PHAC_DISCLOOP_NO_TAGS_FOUND)
                {
                phhalHw_FieldReset(&halReader);
                continue;
                }
            DEBUG_PRINTF ("\n");

            /***************************
             **   Type A
             **************************/

            if (PHAC_DISCLOOP_CHECK_ANDMASK(wTagsDetected, PHAC_DISCLOOP_TYPEA_DETECTED))
                {
                LedOn();
                if (wTagsDetected & PHAC_DISCLOOP_TYPEA_DETECTED_TAG_P2P)
                    {
                    /* If Type F tag with P2P capability was discovered, do and P2P exchange */
                    DEBUG_PRINTF("\nType A with P2P has been detected");

                    status = phOsal_Timer_Wait(&osal, 1, 1000);
                    CHECK_SUCCESS(status);

                    status = phhalHw_FieldReset(&halReader);
                    CHECK_SUCCESS(status);

                    /* Start the next polling cycle */
                    continue;
                    }

                /* Get the number of Type A tags detected */
                status = phacDiscLoop_GetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_TYPEA_NR_TAGS_FOUND, &wNumberOfTags);
                CHECK_SUCCESS(status);

                /* Loop through all the Type A tags detected and print their UIDs */
                for (loop = 0; loop < wNumberOfTags; loop++)
                    {
                    DEBUG_PRINTF ("\nCard %d:",loop + 1);
                    DEBUG_PRINTF ("\nUID: ");
                    PRINT_BUFF(pDataParams->sTypeATargetInfo.aTypeA_I3P3[loop].aUid,
                            pDataParams->sTypeATargetInfo.aTypeA_I3P3[loop].bUidSize);
                    if(loop == 0)
                        {
                        DEBUG_PRINTF("\nAtqa:");
                        PRINT_BUFF(pDataParams->sTypeATargetInfo.aTypeA_I3P3[loop].aAtqa, 2);
                        DEBUG_PRINTF("\nSak: ");
                        PRINT_BUFF(pDataParams->sTypeATargetInfo.aTypeA_I3P3[loop].aSak, 1);
                        DEBUG_PRINTF ("\n");

                        if (0x08 == (pDataParams->sTypeATargetInfo.aTypeA_I3P3[loop].aSak[0] & 0x08))
                            {
                            DEBUG_PRINTF("\nProduct: MIFARE Classic\n");

                            DEBUG_PRINTF("\nThe original key is:                ");
                            PRINT_BUFF(&Original_Key[0], 6);
                            DEBUG_PRINTF("\nThe used key for authentication is: ");
                            PRINT_BUFF(&Key[0], 6);

                            /* load a Key to the Store */
                            /* Note: If You use Key number 0x00, be aware that in SAM
                              this Key is the 'Host authentication key' !!! */
                            status = phKeyStore_FormatKeyEntry(&SwkeyStore, 1, PH_KEYSTORE_KEY_TYPE_MIFARE);

                            /* First step for us is to authenticate with the Key at the Mifare
                             * Classic in the field.
                             * You need to authenticate at any block of a sector and you
                             * may get access to all other blocks of the sector.
                             * For example authenticating at block 5 you will get access to
                             * the blocks 4, 5, 6, 7.
                             */
                            /* Mifare Classic card, set Key Store */
                            status = phKeyStore_SetKey(&SwkeyStore, 1, 0, PH_KEYSTORE_KEY_TYPE_MIFARE, &Key[0], 0);
                            CHECK_STATUS(status);

                            DEBUG_PRINTF("\nSet Key Store successful");

                            /* Initialize the Mifare (R) Classic AL component - set NULL because
                             * the keys are loaded in E2 by the function */
                            /* phKeyStore_SetKey */
                            status = phalMfc_Sw_Init(&alMfc, sizeof(phalMfc_Sw_DataParams_t), &palMifare, &SwkeyStore);
                            CHECK_STATUS(status);

                            /* Mifare Classic card, send authentication for sector 0 */
                            bUidSize = pDataParams->sTypeATargetInfo.aTypeA_I3P3[0].bUidSize;
                            memcpy( bUid, pDataParams->sTypeATargetInfo.aTypeA_I3P3[0].aUid, bUidSize );  /* PRQA S 3200 */

                            status = phalMfc_Authenticate(&alMfc, 0, PHHAL_HW_MFC_KEYA, 1, 0, bUid, bUidSize);

                            if ((status & PH_ERR_MASK) != PH_ERR_SUCCESS)
                                {
                                DEBUG_PRINTF("\n!!! Authentication was not successful.");
                                DEBUG_PRINTF("\nPlease correct the used key");
                                DEBUG_PRINTF("\nAbort of execution");
                                }
                            else
                                {
                                DEBUG_PRINTF("\nAuthentication successful");

                                /* Mifare Classic card, send authentication for sector 1 */
                                status = phalMfc_Authenticate(&alMfc, 6, PHHAL_HW_MFC_KEYA, 1, 0, bUid, bUidSize);

                                /* fill block with data */
                                Fill_Block(bDataBuffer, 15);

                                /* Write data @ block 4 */
                                PH_CHECK_SUCCESS_FCT(status, phalMfc_Write(&alMfc, 4, bDataBuffer));
                                DEBUG_PRINTF("\nWrite successful 16 bytes");

                                /* Empty the bDataBuffer */
                                memset(bDataBuffer, '\0', DATA_BUFFER_LEN);

                                /* Read the just written data.
                                 * In one reading action we always get the whole Block.
                                 */
                                DEBUG_PRINTF("\nReading the just written 16 bytes");
                                PH_CHECK_SUCCESS_FCT(status, phalMfc_Read(&alMfc, 4, bDataBuffer));

                                DEBUG_PRINTF("\nThe content of Block 4 is:\n");
                                for (loop = 0; loop < 4; loop++)
                                    {
                                    PRINT_BUFF(&bDataBuffer[loop*4], 4);
                                    DEBUG_PRINTF("\n-----Cut-----\n");
                                    }
                                }
                            }
                        else
                            {
                            if (PHAC_DISCLOOP_CHECK_ANDMASK(wTagsDetected, PHAC_DISCLOOP_TYPEA_DETECTED_TAG_TYPE1))
                                {
                                DEBUG_PRINTF ("\nType A T1-tag detected ");
                                }
                            else if (PHAC_DISCLOOP_CHECK_ANDMASK(wTagsDetected, PHAC_DISCLOOP_TYPEA_DETECTED_TAG_TYPE2))
                                {
                                DEBUG_PRINTF ("\nType A T2-tag detected ");
                                }
                            else if (PHAC_DISCLOOP_CHECK_ANDMASK(wTagsDetected, PHAC_DISCLOOP_TYPEA_DETECTED_TAG_TYPE4A))
                                {
                                DEBUG_PRINTF ("\nType 4A-tag detected ");
                                }
                            else if (PHAC_DISCLOOP_CHECK_ANDMASK(wTagsDetected, PHAC_DISCLOOP_TYPEA_DETECTED_TAG_P2P))
                                {
                                DEBUG_PRINTF ("\nType A P2P-tag detected ");
                                }
                            }
                        }

                    status = phOsal_Timer_Wait(&osal, 1, 200); // only for LED blink visibility of release mode
                    LedOff();
                    DEBUG_PRINTF("\n");
                    }
                }

            /***************************
             **   Type B
             **************************/

            if (PHAC_DISCLOOP_CHECK_ANDMASK(wTagsDetected, PHAC_DISCLOOP_TYPEB_DETECTED))
                {
                LedOn();
                DEBUG_PRINTF("\nDetected Type B tag(s)");
                /* Get number of Type B tags detected */
                status = phacDiscLoop_GetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_TYPEB_NR_TAGS_FOUND, &wNumberOfTags);

                /* Loop through all the Type B tags detected and print the Pupi */
                for (loop = 0; loop < wNumberOfTags; loop++)
                    {
                    DEBUG_PRINTF ("\nCard %d: ",loop + 1);
                    DEBUG_PRINTF ("\nUID: "); // PUPI - Pseudo Unique PICC Identifier
                    PRINT_BUFF(pDataParams->sTypeBTargetInfo.aI3P3B[loop].aPupi, 4);
                    DEBUG_PRINTF ("\n");
                    }

                status = phOsal_Timer_Wait(&osal, 1, 200); // only for LED blink visibility of release mode
                LedOff();
                }

            /***************************
             **   Type F
             **************************/

            if (PHAC_DISCLOOP_CHECK_ANDMASK(wTagsDetected, PHAC_DISCLOOP_TYPEF_DETECTED))
                {
                LedOn();

                /* TODO: Incase of P2P F, we should not print the UID*/
                if ( !(wTagsDetected & PHAC_DISCLOOP_TYPEF_DETECTED_TAG_P2P))
                    {
                    DEBUG_PRINTF("\nDetected Type F tag(s) ");
                    /* Get the number of Type F tags detected */
                    status = phacDiscLoop_GetConfig(pDataParams, PHAC_DISCLOOP_CONFIG_TYPEF_NR_TAGS_FOUND, &wNumberOfTags);

                    /* Loop through all the type F tags and print the IDm */
                    for (loop = 0; loop < wNumberOfTags; loop++)
                        {
                        DEBUG_PRINTF ("\nCard %d:",loop + 1);
                        DEBUG_PRINTF ("\nUID: "); // IDm - manufacturer ID
                        PRINT_BUFF(pDataParams->sTypeFTargetInfo.aTypeF[loop].aIDmPMm,
                                PHAC_DISCLOOP_FELICA_IDM_LENGTH);
                        DEBUG_PRINTF ("\n");
                        }
                    }

                if (wTagsDetected & PHAC_DISCLOOP_TYPEF_DETECTED_TAG_P2P)
                    {
                    /* If Type F tag with P2P capability was discovered, do and P2P exchange */
                    DEBUG_PRINTF("Type F P2P device has been detected \n");
                    }

                status = phOsal_Timer_Wait(&osal, 1, 200); // only for LED blink visibility of release mode
                LedOff();
                }
            }

        status = phpalI14443p4_ResetProtocol(&palI14443p4);
        CHECK_SUCCESS(status);
        status = phOsal_Timer_Wait(&osal, 1, 1000); // wait 1 second before next detection
        CHECK_SUCCESS(status);
        status = phhalHw_FieldReset(&halReader);
        CHECK_SUCCESS(status);
        }
    }


/***********************************************************************************************
 * Main Function
 *
 **********************************************************************************************/
int main (void)
{
    phStatus_t  status;

    DEBUG_PRINTF("\nStart Example: Classic\n");
#ifdef SPI_USED
    DEBUG_PRINTF("\nSPI link selected");
#endif /* SPI_USED */
#ifdef I2C_USED
    DEBUG_PRINTF("\nI2C link selected");
#endif /* I2C_USED */
    /* Initialize GPIO (sets up clock) */
    GPIO_Init();

    /* Set the interface link for the
     * internal chip communication */
    Set_Interface_Link();

#ifndef TUSA
    /* Set LED port pin to output */
    Set_Port();
#endif
    /* Ensure, that the LED is off */
    LedOff();

    /* Perform a hardware reset */
    Reset_reader_device();

    /* Initialize the Reader BAL (Bus Abstraction Layer) component */
    phbalReg_Stub_Init(&balReader, sizeof(phbalReg_Stub_DataParams_t));

    /* Initialize the Stub timers component */
    status = phOsal_Stub_Init(&osal);
    CHECK_SUCCESS(status);

    status = phbalReg_OpenPort(&balReader);
    CHECK_SUCCESS(status);

    /* Initialize the Reader HAL (Hardware Abstraction Layer) component */
#if defined NXPBUILD__PHHAL_HW_RC523
    status = phhalHw_Rc523_Init(
            &halReader,
            sizeof(phhalHw_Rc523_DataParams_t),
            &balReader,
            0,
            bHalBufferTx,
            sizeof(bHalBufferTx),
            bHalBufferRx,
            sizeof(bHalBufferRx));
#endif
#if defined NXPBUILD__PHHAL_HW_RC663
    status = phhalHw_Rc663_Init(
            &halReader,
            sizeof(phhalHw_Rc663_DataParams_t),
            &balReader,
            0,
            bHalBufferTx,
            sizeof(bHalBufferTx),
            bHalBufferRx,
            sizeof(bHalBufferRx));
#endif

    /* Set the parameter to use the UART interface */
    halReader.bBalConnectionType = PHHAL_HW_BUS;

    /* Set the generic pointer */
    pHal = &halReader;
    /* Read the version of the reader IC */

#if defined NXPBUILD__PHHAL_HW_RC523
    phhalHw_ReadRegister(&halReader, PHHAL_HW_RC523_REG_VERSION, &bDataBuffer[0]);
    DEBUG_PRINTF("\nReader chip PN512: 0x%02x\n", bDataBuffer[0]);
#endif
#if defined NXPBUILD__PHHAL_HW_RC663
    phhalHw_ReadRegister(&halReader, PHHAL_HW_RC663_REG_VERSION, &bDataBuffer[0]);
    DEBUG_PRINTF("\nReader chip RC663: 0x%02x\n", bDataBuffer[0]);
#endif

    /* Initializing specific objects for the communication with
     * Mifare (R) Classic cards.
     * The Mifare (R) Classic card is compliant of
     * ISO 14443-3 and ISO 14443-4
     */

    /* Initialize the I14443-A PAL layer */
    status = phpalI14443p3a_Sw_Init(&palI14443p3a, sizeof(phpalI14443p3a_Sw_DataParams_t), &halReader);
    CHECK_SUCCESS(status);

    /* Initialize the I14443-A PAL component */
    status = phpalI14443p4a_Sw_Init(&palI14443p4a, sizeof(phpalI14443p4a_Sw_DataParams_t), &halReader);
    CHECK_SUCCESS(status);

    /* Initialize the I14443-4 PAL component */
    status = phpalI14443p4_Sw_Init(&palI14443p4, sizeof(phpalI14443p4_Sw_DataParams_t), &halReader);
    CHECK_SUCCESS(status);

    /* Initialize the I14443-B PAL  component */
    status = phpalI14443p3b_Sw_Init(&palI14443p3b, sizeof(palI14443p3b), &halReader);
    CHECK_SUCCESS(status);

    /* Initialize PAL Felica PAL component */
    status = phpalFelica_Sw_Init(&palFelica, sizeof(phpalFelica_Sw_DataParams_t), &halReader);
    CHECK_SUCCESS(status);

    /* Init 18092 PAL component */
    status = phpalI18092mPI_Sw_Init(&palI18092mPI, sizeof(phpalI18092mPI_Sw_DataParams_t), pHal);
    CHECK_SUCCESS(status);

    /* Initialize the Mifare PAL component */
    status = phpalMifare_Sw_Init(&palMifare, sizeof(phpalMifare_Sw_DataParams_t), &halReader, &palI14443p4);
    CHECK_SUCCESS(status);

    /* Initialize the Felica AL component */
    status = phalFelica_Sw_Init(&alFelica, sizeof(phalFelica_Sw_DataParams_t), &palFelica);
    CHECK_SUCCESS(status);

    /* Initialize the Mful AL component */
    status = phalMful_Sw_Init(&alMful, sizeof(phalMful_Sw_DataParams_t), &palMifare, NULL, NULL, NULL);
    CHECK_SUCCESS(status);

    /* Initialize the MF DesFire EV1 component */
    status = phalMfdf_Sw_Init(&alMfdf, sizeof(phalMfdf_Sw_DataParams_t), &palMifare, NULL, NULL, NULL, &halReader);

    /* Initialize the keystore component */
    status = phKeyStore_Sw_Init(
            &SwkeyStore,
            sizeof(phKeyStore_Sw_DataParams_t),
            &pKeyEntries[0],
            NUMBER_OF_KEYENTRIES,
            &pKeyVersionPairs[0],
            NUMBER_OF_KEYVERSIONPAIRS,
            &pKUCEntries[0],
            NUMBER_OF_KUCENTRIES);
    CHECK_SUCCESS(status);

    /* Initialize the timer component */
    status = phOsal_Timer_Init(&osal);
    CHECK_SUCCESS(status);

    /* Initialize the discover component */
    status = phacDiscLoop_Sw_Init(&discLoop, sizeof(phacDiscLoop_Sw_DataParams_t), &halReader, &osal);
    CHECK_SUCCESS(status);

    discLoop.pPal1443p3aDataParams  = &palI14443p3a;
    discLoop.pPal1443p3bDataParams  = &palI14443p3b;
    discLoop.pPal1443p4aDataParams  = &palI14443p4a;
    discLoop.pPal18092mPIDataParams = &palI18092mPI;
    discLoop.pPalFelicaDataParams   = &palFelica;
    discLoop.pHalDataParams         = &halReader;
    discLoop.pOsal                  = &osal;

    /* reset the IC  */
    status = phhalHw_FieldReset(pHal);
    CHECK_SUCCESS(status);

    DiscLoopInit(&discLoop);
    DiscLoopDemo(&discLoop);

    return 0;
}

/***********************************************************************************************
 *                            End Of File
 **********************************************************************************************/
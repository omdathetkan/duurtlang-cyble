/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#include <main.h>

// Global brightness is used by the app do decrease the brightness
extern uint8_t globalBrightness;

/* This flag is used by application to know whether a Central 
* device has been connected. This is updated in BLE event callback 
* function*/
uint8 deviceConnected = 0;

/* This flag is used to let application send a L2CAP connection update request
* to Central device */
static uint8 isConnectionUpdateRequested = 1;

/* 'restartAdvertisement' flag provided the present state of power mode in firmware */
uint8 restartAdvertisement = 0;

/* Connection Parameter update values. This values are used by the BLE component
* to update the connector parameter, including connection interval, to desired 
* value */
static CYBLE_GAP_CONN_UPDATE_PARAM_T ConnectionParam =
{
    CONN_PARAM_UPDATE_MIN_CONN_INTERVAL,  		      
    CONN_PARAM_UPDATE_MAX_CONN_INTERVAL,		       
    CONN_PARAM_UPDATE_SLAVE_LATENCY,			    
    CONN_PARAM_UPDATE_SUPRV_TIMEOUT 			         	
};


/*******************************************************************************
* Function Name: StackEventHandler
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the BLE Component.
*
* Parameters:  
*  uint8 event:       Event from the CYBLE component
*  void* eventParams: A structure instance for corresponding event type. The 
*                     list of event structure is described in the component 
*                     datasheet.
*
* Return: 
*  None
*
*******************************************************************************/
void StackEventHandler(uint32 event, void *eventParam)
{
    
    CYBLE_GATTS_WRITE_REQ_PARAM_T *wrReqParam;
    
    switch(event)
    {
        case CYBLE_EVT_STACK_ON:
			/* This event is received when component is Started */
			
			/* Set restartAdvertisement flag to allow calling Advertisement 
			* API from main function */
			restartAdvertisement = 1;
			
			break;
			
		case CYBLE_EVT_TIMEOUT:
			/* Event Handling for Timeout  */
	
			break;
        
		/**********************************************************
        *                       GAP Events
        ***********************************************************/
		case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
			
			/* If the current BLE state is Disconnected, then the Advertisement
			* Start Stop event implies that advertisement has stopped */
			if(CyBle_GetState() == CYBLE_STATE_DISCONNECTED)
			{
				/* Set restartAdvertisement flag to allow calling Advertisement 
				* API from main function */
				restartAdvertisement = 1;
			}
			break;
			
		case CYBLE_EVT_GAP_DEVICE_CONNECTED: 					
			/* This event is received when device is connected over GAP layer */
			break;

			
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
			/* This event is received when device is disconnected */
			
			/* Set restartAdvertisement flag to allow calling Advertisement 
			* API from main function */
			restartAdvertisement = 1;
			
			break;
            
            
        /**********************************************************
        *                       GATT Events
        ***********************************************************/
        case CYBLE_EVT_GATT_CONNECT_IND:
			/* This event is received when device is connected over GATT level */
			
			/* Update attribute handle on GATT Connection*/
            cyBle_connHandle = *(CYBLE_CONN_HANDLE_T  *)eventParam;
			
			/* This flag is used in application to check connection status */
			deviceConnected = 1;
			break;
        
        case CYBLE_EVT_GATT_DISCONNECT_IND:
			/* This event is received when device is disconnected */
			
			/* Update deviceConnected flag*/
			deviceConnected = 0;
	


			/* Reset the isConnectionUpdateRequested flag to allow sending
			* connection parameter update request in next connection */
			isConnectionUpdateRequested = 1;
			
			
			break;
        
        case CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_RSP:
				/* If L2CAP connection parameter update response received, reset application flag */
            	isConnectionUpdateRequested = 0;
            break;
        

        case CYBLE_EVT_GATTS_WRITE_REQ:
        case CYBLE_EVT_GATTS_WRITE_CMD_REQ:
            
            wrReqParam = (CYBLE_GATTS_WRITE_REQ_PARAM_T *) eventParam; //read in event command
            
            if(wrReqParam->handleValPair.attrHandle == CYBLE_CATAN_TILE_CHAR_HANDLE) //If client writes to the number_write characteristic
                {
                    // Tile number                   
                    uint16 tile = wrReqParam->handleValPair.value.val[0];
                    
                    // Calculate tile position correction
                    tile = tilePosCor(tile);
                    
                    // Number to be written
                    uint8 number_write = wrReqParam->handleValPair.value.val[1]; //Pull out the number_write value
                    // Brightness of the number
                    uint8 bright_num = wrReqParam->handleValPair.value.val[2];
                    // Color of the number
                    uint32_t color_num;
                    color_num = wrReqParam->handleValPair.value.val[5];
                    color_num = (color_num << 8) + wrReqParam->handleValPair.value.val[3];
                    color_num = (color_num << 8) + wrReqParam->handleValPair.value.val[4];

                    // Brightness of the edge
                    uint8 bright_edge = wrReqParam->handleValPair.value.val[6];
                    // Color of the edge
                    uint32_t color_edge;
                    color_edge = wrReqParam->handleValPair.value.val[9];
                    color_edge = (color_edge << 8) + wrReqParam->handleValPair.value.val[7];
                    color_edge = (color_edge << 8) + wrReqParam->handleValPair.value.val[8];
                    
                    // Brightness of the water
                    uint8 bright_water = wrReqParam->handleValPair.value.val[10];
                    // Color of the water
                    uint32_t color_water;
                    color_water = wrReqParam->handleValPair.value.val[13];
                    color_water = (color_water << 8) + wrReqParam->handleValPair.value.val[11];
                    color_water = (color_water << 8) + wrReqParam->handleValPair.value.val[12];                

                    // Settings crap
                    uint8 settings_byte = 0;
                    settings_byte = wrReqParam->handleValPair.value.val[14];
                    uint8 update_leds_bool = settings_byte & 0x1;
                    uint8 clear_leds_bool = settings_byte & 0x2;
                    uint8 dice_bool = settings_byte & 0x4;
                    uint8 fade_bool = settings_byte & 0x8;
                    
                    CyBle_GattsWriteAttributeValue(&wrReqParam->handleValPair, 0, &cyBle_connHandle, CYBLE_GATT_DB_PEER_INITIATED); 
                    
                    // Clear specific tile
                    ClearTile(tile);
                    
                    NumberLED(number_write, offset_calc(tile), bright_num, color_num);
                    EdgeLED(bright_edge, offset_calc(tile),  color_edge);
                    WaterLED(bright_water, offset_calc(tile),  color_water, tile);
                    
                   
                    if (clear_leds_bool > 0)
                    {
                        ClearTile(tile);
                    }
                    
                    // Trigger update of all LEDs at once
                    if (update_leds_bool > 0)
                    {
                        //StripLights_Trigger(1);  
                        updateLeds = 1;
                    }
                    
                    
                }   
                
            if(wrReqParam->handleValPair.attrHandle == CYBLE_CATAN_BRIGHTNESS_CHAR_HANDLE) //If client writes to the number_write characteristic
                {
                    // Tile number                   
                    uint8 brightness = wrReqParam->handleValPair.value.val[0];
                    CyBle_GattsWriteAttributeValue(&wrReqParam->handleValPair, 0, &cyBle_connHandle, CYBLE_GATT_DB_PEER_INITIATED); 
                    globalBrightness = brightness;
                }
            
            if(wrReqParam->handleValPair.attrHandle == CYBLE_CATAN_POWER_CHAR_HANDLE) //If client writes to the number_write characteristic
                {
                    // Tile number                   
                    uint8 power = wrReqParam->handleValPair.value.val[0];
                    CyBle_GattsWriteAttributeValue(&wrReqParam->handleValPair, 0, &cyBle_connHandle, CYBLE_GATT_DB_PEER_INITIATED);
                }    
                
            if (event == CYBLE_EVT_GATTS_WRITE_REQ) //If write was a write with response request.
                {
                    CyBle_GattsWriteRsp(cyBle_connHandle); //Send response back to Client
                }
            

            
            break;
            
            
        default:
    	    break;
    }
}


/*******************************************************************************
* Function Name: UpdateConnectionParam
********************************************************************************
* Summary:
*        Send the Connection Update Request to Client device after connection 
* and modify theconnection interval for low power operation.
*
* Parameters:
*	void
*
* Return:
*  void
*
*******************************************************************************/
void UpdateConnectionParam(void)
{
	/* If device is connected and Update connection parameter not updated yet,
	* then send the Connection Parameter Update request to Client. */
    if(deviceConnected && isConnectionUpdateRequested)
	{
		/* Reset the flag to indicate that connection Update request has been sent */
		isConnectionUpdateRequested = 0;
		
		/* Send Connection Update request with set Parameter */
		CyBle_L2capLeConnectionParamUpdateRequest(cyBle_connHandle.bdHandle, &ConnectionParam);
	}
}

/* [] END OF FILE */

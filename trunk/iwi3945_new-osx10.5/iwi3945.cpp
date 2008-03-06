/*
 *  iwi3945.cpp
 *  iwi3945
 *
 *  Created by Sean Cross on 1/19/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "iwi3945.h"
#include "defines.h"
#include "compatibility.h"

// Define my superclass
#define super IOEthernetController//IO80211Controller

// REQUIRED! This macro defines the class's constructors, destructors,
// and several other methods I/O Kit requires. Do NOT use super as the
// second parameter. You must use the literal name of the superclass.
OSDefineMetaClassAndStructors(darwin_iwi3945, IOEthernetController);

// Magic to make the init/exit routines public.
extern "C" {
    extern int (*init_routine)();
    extern void (*exit_routine)();
}
extern void setCurController(IONetworkController * tmp);
extern IOWorkLoop * getWorkLoop();
extern IOInterruptEventSource * getInterruptEventSource();
extern int if_down();
extern IOPCIDevice * getPCIDevice();
extern IOMemoryMap * getMap();
extern void setUnloaded();
extern void start_undirect_scan();
extern u8 * getMyMacAddr();
extern void setMyfifnet(ifnet_t fifnet);

IOService * my_provider;
#pragma mark -
#pragma mark Overrides required for implementation

IOService *darwin_iwi3945::getProvider() {
    return my_provider;
}


UInt32 darwin_iwi3945::handleInterrupt() {
    return 0;
}

#pragma mark -
#pragma mark IONetworkController overrides

IOOutputQueue * darwin_iwi3945::createOutputQueue( void )
{
	// An IOGatedOutputQueue will serialize all calls to the driver's
    // outputPacket() function with its work loop. This essentially
    // serializes all access to the driver and the hardware through
    // the driver's work loop, which simplifies the driver but also
    // carries a small performance cost (relatively for 10/100 Mb).
    IOLog("Someone called createOutputQueue()\n");
    return IOGatedOutputQueue::withTarget( this, getWorkLoop() );
}

int darwin_iwi3945::outputRaw80211Packet( IO80211Interface * interface, mbuf_t m )
{
	return -1;
    /*IOLog("Someone called outputRaw80211Packet\n");
    int ret = super::outputRaw80211Packet(interface, m);
    IOLog("outputRaw80211Packet: Okay, returning %d\n", ret);
    return ret;*/
}

UInt32 darwin_iwi3945::getFeatures() const {
    return kIONetworkFeatureSoftwareVlan;
}

SInt32 darwin_iwi3945::apple80211Request( UInt32 req, int type, IO80211Interface * intf, void * data ) {
    SInt32 ret = 0;
    static int counter = 0;

    if( counter < 30 ) {
        counter++;
        IOLog("Someone called apple80211Request(0x%08x, %d, 0x%08x, 0x%08x)\n", req, type, intf, data);
    }
    
    // These two are defined in apple80211_ioctl.h, and ought to be the only
    // two valies /req/ can take.  They specify that /data/ should be of type apple80211req.
    // Note that SIOCGA80211 is sent to GET a value, and SIOCSA80211 is sent to SET a value.
    if( req != SIOCGA80211 && req != SIOCSA80211 ) {
        IOLog("Don't know how to deal with a request on an object of type 0x%08x\n", req);
        return 0;
    }
    
    

    // Used IOCTLs:
    // 44 43 28 51 12 19 4 5 
    switch( type ) {

            // 1:
        case APPLE80211_IOC_SSID: //req_type
            if( SIOCGA80211 == req ) {
                IOLog("Request to GET SSID\n");
                ret = getSSID(intf, (apple80211_ssid_data *)(data));
            }
            else {
                IOLog("Request to SET SSID\n");
                ret = setSSID(intf, (apple80211_ssid_data *)data);
            }
            
            break;
            
            
            // 12:
        case APPLE80211_IOC_CARD_CAPABILITIES: //req_type
            //IOLog("APPLE80211_IOC_CARD_CAPABILITIES:"
            //        " 0x%08x [%d]\n", data, data);
            if( SIOCSA80211 == req ) {
                IOLog("Don't know how to SET Capabilities!\n");
            }
            else {
                ret = getCARD_CAPABILITIES(intf,
                        (apple80211_capability_data *)data);
            }            

            break;
            
            
            
            // 23:
        case APPLE80211_IOC_STATUS_DEV_NAME: //req_type
            IOLog("APPLE80211_IOC_STATUS_DEV_NAME:"
                    " 0x%08x [%d]\n", data, data);
            if( SIOCSA80211 == req ) {
                IOLog("Don't how how to SET device name!\n");
            }
            else {
                ret = getSTATUS_DEV(intf, (apple80211_status_dev_data *)data);
            }
            
            break;
            


            // 19:
        case APPLE80211_IOC_POWER: //req_type
                
//            IOLog("APPLE80211_IOC_POWER:"
//                    " 0x%08x [%d]\n", data, data);
            if( SIOCSA80211 == req ) {
                ret = setPOWER(intf, (apple80211_power_data *)data);
            }
            else {
                ret = getPOWER(intf, (apple80211_power_data *)data);
            }
            break;
            
            
            // 4:
        case APPLE80211_IOC_CHANNEL: //req_type
        
            if( SIOCSA80211 == req ) {
                ret = setCHANNEL(intf, (apple80211_channel_data *)data);
            }
            else {
                ret = getCHANNEL(intf, (apple80211_channel_data *)data);
            }
            break;
            
            
            
        case APPLE80211_IOC_MCS_INDEX_SET:

            if( SIOCSA80211 == req ) {
                IOLog("Don't know how to SET MCS index!\n");
            }
            else {
                ret = getMCS_INDEX_SET(intf, (apple80211_mcs_index_set_data *)data);
            }
                
            break;
            
            
            
            
            // 5:
        case APPLE80211_IOC_POWERSAVE: //req_type
            if( SIOCSA80211 == req ) {
                ret = setPOWERSAVE(intf, (apple80211_powersave_data *)data);
            }
            else {
                ret = getPOWERSAVE(intf, (apple80211_powersave_data *)data);
            }
            
            
            break;
            


            
            
            // 44:
        case APPLE80211_IOC_HARDWARE_VERSION: //req_type
            if( SIOCSA80211 == req ) {
                IOLog("Don't know how to SET hardware version!\n");
            }
            else {
                ret = getHARDWARE_VERSION(intf, (apple80211_version_data *)data);
            }
             
             
            break;

            
             
             // 43:
        case APPLE80211_IOC_DRIVER_VERSION: //req_type 
            if( SIOCSA80211 == req ) {
                IOLog("Don't know how to SET driver version!\n");
            }
            else {
                ret = getDRIVER_VERSION(intf, (apple80211_version_data *)data);
            }
             
             
             break;
             

             // 28:
        case APPLE80211_IOC_LOCALE: { //req_type
            if( SIOCSA80211 == req ) {
                ret = setLOCALE(intf, (apple80211_locale_data *)data);
            }
            else {
                ret = getLOCALE(intf, (apple80211_locale_data *)data);
            }
            
            break;
        }
            
            
            // 51:
        case APPLE80211_IOC_COUNTRY_CODE: //req_type
            if( SIOCSA80211 == req ) {
                IOLog("Don't know how to SET country code!\n");
            }
            else {
                ret = getCOUNTRY_CODE(intf, (apple80211_country_code_data *)data);
            }
            
            break;
            
            
            // 14:
        case APPLE80211_IOC_PHY_MODE:
            if( SIOCSA80211 == req ) {
                ret = setPHY_MODE(intf, (apple80211_phymode_data *)data);
            }
            else {
                ret = getPHY_MODE(intf, (apple80211_phymode_data *)data);
            }
            break;
            
            //18:
        case APPLE80211_IOC_INT_MIT:
            if( SIOCSA80211 == req ) {
                ret = setINT_MIT(intf, (apple80211_intmit_data *)data);
            }
            else {
                ret = getINT_MIT(intf, (apple80211_intmit_data *)data);
            }
            break;
    
            //7:
        case APPLE80211_IOC_TXPOWER:
            if( SIOCSA80211 == req ) {
                ret = setTXPOWER(intf, (apple80211_txpower_data *)data);
            }
            else {
                ret = getTXPOWER(intf, (apple80211_txpower_data *)data);
            }
            break;
            
            //13:
        case APPLE80211_IOC_STATE:
            if( SIOCSA80211 == req ) {
                IOLog("Don't know how to SET IOC state\n");
            }
            else {
				ret = getSTATE(intf,(apple80211_state_data *)data);
            }
            break;
            
            //15:
        case APPLE80211_IOC_OP_MODE:
            if( SIOCSA80211 == req ) {
                IOLog("Don't know how to SET op mode\n");
            }
            else {
                ret = getOP_MODE(intf, (apple80211_opmode_data *)data);
            }
            break;
            
            
            //17:
        case APPLE80211_IOC_NOISE:
            if( SIOCSA80211 == req ) {
                IOLog("Don't know how to SET noise\n");
            }
            else {
                ret = getNOISE(intf, (apple80211_noise_data *)data);
            }
            break;
            
            //8:
        case APPLE80211_IOC_RATE:
            if( SIOCSA80211 == req ) {
                ret = setRATE(intf, (apple80211_rate_data *)data);
            }
            else {
                ret = getRATE(intf, (apple80211_rate_data *)data);
            }
            break;
            
            
            //16:
        case APPLE80211_IOC_RSSI:
            if( SIOCSA80211 == req ) {
                IOLog("Don't know how to SET RSSI\n");
            }
            else {
                ret = getRSSI(intf, (apple80211_rssi_data *)data);
            }
            break;
            
            
            //2:
        case APPLE80211_IOC_AUTH_TYPE:
            if( SIOCGA80211 == req ) {
                ret = getAUTH_TYPE(intf, (apple80211_authtype_data *)data);
            }
            else {
                ret = setAUTH_TYPE(intf, (apple80211_authtype_data *)data);
            }
            break;
            
            
            //6:
        case APPLE80211_IOC_PROTMODE:
            if( SIOCGA80211 == req ) {
                //IOLog("Don't know how to GET protmode\n");
				ret = getPROTMODE(intf, (apple80211_protmode_data *)data);
            }
            else {
                ret = setPROTMODE(intf, (apple80211_protmode_data *)data);
            }
            break;
            
            
            //9:
        case APPLE80211_IOC_BSSID:
            if( SIOCSA80211 == req ) {
                IOLog("Don't know how to SET bssid\n");
            }
            else {
                ret = getBSSID(intf, (apple80211_bssid_data *)data);
            }
            break;
            
            
            //39:
        case APPLE80211_IOC_ANTENNA_DIVERSITY:
            if( SIOCSA80211 == req ) {
                ret = setANTENNA_DIVERSITY(intf, (apple80211_antenna_data *)data);
            }
            else {
                ret = getANTENNA_DIVERSITY(intf, (apple80211_antenna_data *)data);
            }
            break;
            
            
            //37:
        case APPLE80211_IOC_TX_ANTENNA:
            if( SIOCSA80211 == req ) {
                ret = setTX_ANTENNA(intf, (apple80211_antenna_data *)data);
            }
            else {
                ret = getTX_ANTENNA(intf, (apple80211_antenna_data *)data);
            }
            break;
            
            
            //27:
        case APPLE80211_IOC_SUPPORTED_CHANNELS:
            if( SIOCSA80211 == req ) {
                IOLog("Don't know how to SET supported channels\n");
            }
            else {
                ret = getSUPPORTED_CHANNELS(intf, (apple80211_sup_channel_data *)data);
            }
            break;
            
            //10:
        case APPLE80211_IOC_SCAN_REQ:
            if( SIOCGA80211 == req ) {
                IOLog("Don't know how to GET scan request\n");
            }
            else {
                ret = setSCAN_REQ(intf, (apple80211_scan_data *)data);
            }
            break;
            
            
            //11:
        case APPLE80211_IOC_SCAN_RESULT:
            if( SIOCSA80211 == req ) {
                IOLog("Don't know how to SET scan result request\n");
            }
            else {
                ret = getSCAN_RESULT(intf, (apple80211_scan_result **)data);
            }
            break;
            
            
            //22:
        case APPLE80211_IOC_DISASSOCIATE:
            if( SIOCGA80211 == req ) {
                IOLog("Don't know how to GET disassociate\n");
            }
            else {
                ret = setDISASSOCIATE(intf);
            }
            break;
            
            
        default:
            IOLog("Unknown command: apple80211Request(0x%08x, %d, 0x%08x, 0x%08x)\n", req, type, intf, data);
            break;
    }


//    IOLog("Done with ioctl, returning %d.\n", ret);
    return ret;
}


void darwin_iwi3945::postMessage(UInt32 message) {
    if( fNetif )
        fNetif->postMessage(message, NULL, 0);
}



#pragma mark -
#pragma mark Setup and teardown
bool darwin_iwi3945::init(OSDictionary *dict)
{
	return super::init(dict);
}


bool darwin_iwi3945::start(IOService *provider)
{
	UInt16	reg;
	//Define the init state
	myState = APPLE80211_S_INIT;
    IOLog("iwi3945: Starting\n");
    int err = 0;
    
	do {
        
        // Note: super::start() calls createWorkLoop & getWorkLoop
		if ( super::start(provider) == 0) {
			IOLog("%s ERR: super::start failed\n", getName());
			break;
		}
		
		setCurController(this);
		my_provider=provider;
		if( init_routine() )
			return false;
		
		fTransmitQueue = createOutputQueue();
		if (fTransmitQueue == NULL)
		{
			IOLog("ERR: getOutputQueue()\n");
			break;
		}
		fTransmitQueue->setCapacity(1024);
		
		
		mac_addr = getMyMacAddr();
		
		        // Publish the MAC address
        if ( (setProperty(kIOMACAddress, mac_addr, kIOEthernetAddressSize) == false) )
        {
            IOLog("Couldn't set the kIOMACAddress property\n");
        }
        
        
        
        // Attach the IO80211Interface to this card.  This also creates a
        // new IO80211Interface, and stores the resulting object in fNetif.
		if (attachInterface((IONetworkInterface **) &fNetif, true) == false) {
			IOLog("%s attach failed\n", getName());
			break;
		}

				

		
        IOLog("registerService()\n");
		registerService();
		
		/*mediumDict = OSDictionary::withCapacity(MEDIUM_TYPE_INVALID + 1);
		addMediumType(kIOMediumIEEE80211None,  0,  MEDIUM_TYPE_NONE);
		addMediumType(kIOMediumIEEE80211Auto,  0,  MEDIUM_TYPE_AUTO);
		addMediumType(kIOMediumIEEE80211DS1,   1000000, MEDIUM_TYPE_1MBIT);
		addMediumType(kIOMediumIEEE80211DS2,   2000000, MEDIUM_TYPE_2MBIT);
		addMediumType(kIOMediumIEEE80211DS5,   5500000, MEDIUM_TYPE_5MBIT);
		addMediumType(kIOMediumIEEE80211DS11, 11000000, MEDIUM_TYPE_11MBIT);
		addMediumType(kIOMediumIEEE80211,     54000000, MEDIUM_TYPE_54MBIT, "OFDM54");
		addMediumType(kIOMediumIEEE80211OptionAdhoc, 0, MEDIUM_TYPE_ADHOC,"ADHOC");
        
		publishMediumDictionary(mediumDict);
		setCurrentMedium(mediumTable[MEDIUM_TYPE_AUTO]);
		setSelectedMedium(mediumTable[MEDIUM_TYPE_AUTO]);
		setLinkStatus(kIONetworkLinkValid, mediumTable[MEDIUM_TYPE_AUTO]);*/
		
		
		mediumDict = OSDictionary::withCapacity(MEDIUM_TYPE_INVALID + 1);
		addMediumType( kIOMediumEthernetAuto, 0, MEDIUM_TYPE_AUTO);
		//addMediumType(kIOMediumEthernetNone,  0,  MEDIUM_TYPE_NONE);
	

		publishMediumDictionary(mediumDict);
		setCurrentMedium(mediumTable[MEDIUM_TYPE_AUTO]);
		setSelectedMedium(mediumTable[MEDIUM_TYPE_AUTO]);
		setLinkStatus(kIONetworkLinkValid, mediumTable[MEDIUM_TYPE_AUTO]);
		

        return true;
    } while(false);
    
    free();
    return false;
}



void darwin_iwi3945::free(void)
{
	IOLog("iwi3945: Freeing\n");
	if( fTransmitQueue ) fTransmitQueue->release();
	IOPCIDevice *fPCIDevice = getPCIDevice();
	if( fPCIDevice) {
		printf("Stop PCI Device\n");
		fPCIDevice->setBusMasterEnable(false);
		fPCIDevice->setMemoryEnable(false);
		IOMemoryMap * map;
		map = getMap();
		if(map){
			map->unmap();
			map->release();
		}
		fPCIDevice->close(this);
		fPCIDevice->release();
	}
	super::free();
}


void darwin_iwi3945::stop(IOService *provider)
{
	IOLog("iwi3945: Stopping\n");
	setUnloaded();//Stop all the workqueue
	IOSleep(1000);//wait for unfinished thread crappy oh Yeah!
	IOWorkLoop * workqueue = getWorkLoop();
	IOInterruptEventSource * fInterruptSrc = getInterruptEventSource();
	if (fInterruptSrc && workqueue){
        workqueue->removeEventSource(fInterruptSrc);
		fInterruptSrc->disable();
		fInterruptSrc->release();
		printf("Stopping OK\n");
	}
	if_down();
	if( fNetif ) {
        detachInterface( fNetif );
        fNetif->release();
    }
	super::stop(provider);
}

/******************************************************************************* 
 * Functions which MUST be implemented by any class which inherits
 * from IO80211Controller.
 ******************************************************************************/
#pragma mark -
#pragma mark IO80211Controller entry points


SInt32
darwin_iwi3945::getSSID(IO80211Interface *interface,
						struct apple80211_ssid_data *sd)
{
    if( NULL == sd ) {
        IOLog("Quit calling getSSID() with a null ssid_data field!\n");
        return 0;
    }
    
    
    /*
    // call interface->linkState()
    if( interface->linkState() != kIO80211NetworkLinkUp ) {
        return 0x6;
    }*/
    
    memset(sd, 0, sizeof(*sd));
    sd->version = APPLE80211_VERSION;
    strncpy((char*)sd->ssid_bytes, "anetwork", sizeof(sd->ssid_bytes));
    sd->ssid_len = strlen("anetwork");
        
	return 0;
}



SInt32 
darwin_iwi3945::getMCS_INDEX_SET(IO80211Interface *interface,
                                 apple80211_mcs_index_set_data *misd)
{
    IOLog("Warning: fudged a getMCS_INDEX_SET()\n");
    
    misd->version = APPLE80211_VERSION;
    
    int offset;
    for(offset=0; offset<sizeof(misd->mcs_set_map); offset++) {
        misd->mcs_set_map[offset] = offset;
    }
    
    return kIOReturnSuccess;
}


SInt32
darwin_iwi3945::getHARDWARE_VERSION(IO80211Interface *interface,
                                    struct apple80211_version_data *hv)
{
    hv->version = APPLE80211_VERSION;
    strncpy(hv->string, "Hacked up piece of code", sizeof(hv->string));
    hv->string_len = strlen("Hacked up piece of code");
    
    return kIOReturnSuccess;
}

SInt32
darwin_iwi3945::getDRIVER_VERSION(IO80211Interface *interface,
                                    struct apple80211_version_data *hv)
{
    hv->version = APPLE80211_VERSION;
    strncpy(hv->string, "Version 0.0", sizeof(hv->string));
    hv->string_len = strlen("Version 0.0");
    
    return kIOReturnSuccess;
}    

SInt32
darwin_iwi3945::setCHANNEL(IO80211Interface *interface,
                           struct apple80211_channel_data *cd)
{
    IOLog("Warning: ignored a setCHANNEL()\n");
    return kIOReturnSuccess;
}


SInt32
darwin_iwi3945::getCHANNEL(IO80211Interface *interface,
						  struct apple80211_channel_data *cd)
{
//	IOLog("getCHANNEL c:%d f:%d\n",cd->channel.channel,cd->channel.flags);
    cd->version = APPLE80211_VERSION;
    cd->channel.version = APPLE80211_VERSION;
    cd->channel.channel = 6;
    cd->channel.flags = APPLE80211_C_FLAG_2GHZ;
	return kIOReturnSuccess;
}

SInt32
darwin_iwi3945::getBSSID(IO80211Interface *interface,
						struct apple80211_bssid_data *bd)
{
    
    bd->version = APPLE80211_VERSION;
//    memcpy(bd->bssid.octet, "FEDCBA987654", sizeof(bd->bssid.octet));
    bd->bssid.octet[0] = 0xFE;
    bd->bssid.octet[1] = 0xDC;
    bd->bssid.octet[2] = 0xBA;
    bd->bssid.octet[3] = 0x98;
    bd->bssid.octet[4] = 0x76;
    bd->bssid.octet[5] = 0x54;

//    IOLog("getBSSID %s\n",escape_essid((const char*)bd->bssid.octet,sizeof(bd->bssid.octet)));

	return 0;
}

SInt32
darwin_iwi3945::getCARD_CAPABILITIES(IO80211Interface *interface,
									  struct apple80211_capability_data *cd)
{
    if( !cd ) {
        IOLog("Quit calling getCARD_CAPABILITIES without specifying *cd!\n");
        return 0;
    }
    cd->version = APPLE80211_VERSION;
    cd->capabilities[0] = 0xab; // Values taken directly from AirPort_Brcm43xx
    cd->capabilities[1] = 0x7e; // I would guess they define settings for the various radios.
	return 0;
}

SInt32
darwin_iwi3945::getSTATE(IO80211Interface *interface,
						  struct apple80211_state_data *sd)
{
    if( !sd ) {
        IOLog("Quit calling getSTATE without specifying *sd!\n");
        return 0;
    }
	
    sd->version = APPLE80211_VERSION;
    sd->state = APPLE80211_S_RUN;//= myState;
	/*APPLE80211_S_INIT	= 0,			// default state
	APPLE80211_S_SCAN	= 1,			// scanning
	APPLE80211_S_AUTH	= 2,			// try to authenticate
	APPLE80211_S_ASSOC	= 3,			// try to assoc
	APPLE80211_S_RUN	= 4,			// associated*/
	return 0;
}

SInt32
darwin_iwi3945::getRSSI(IO80211Interface *interface,
					   struct apple80211_rssi_data *rd)
{
	IOLog("getRSSI \n");
	return 0;
}

SInt32
darwin_iwi3945::getPOWER(IO80211Interface *interface,
						struct apple80211_power_data *pd)
{
    IOLog("getPOWER \n");
    
    //interface->setPowerState(kIO80211SystemPowerStateAwake, this);
    pd->version = APPLE80211_VERSION;
    pd->num_radios = 3;
    pd->power_state[0] = APPLE80211_POWER_ON;
    pd->power_state[1] = APPLE80211_POWER_ON;
    pd->power_state[2] = APPLE80211_POWER_ON;

    return kIOReturnSuccess;
    /*
	//IOPMprot *p=pm_vars;
	//memset(&(pd->power_state),0,sizeof(pd->power_state));

	//pd->num_radios=p->myCurrentState;//theNumberOfPowerStates;
	for (int c=0;c < p->theNumberOfPowerStates;c++)
	{
		IOPMPowerState *pstate=&p->thePowerStates[c];
		IOPMPowerFlags f=pstate->capabilityFlags;
		if (c < APPLE80211_MAX_RADIO) 
		{
			pd->power_state[c]=f;
		}
	
	IOPMPowerFlags pf=p->myCurrentState;
	IOLog("powerf 0x%4x\n",pf);
	//memcpy(&pd->power_state,(void*)pf,sizeof(IOPMPowerFlags));
	//IOLog("powerf 0x%4x\n",pd->power_state);
	//interface->setPowerState(pf,this);
	IOLog("getPOWER %d, %d %d %d %d\n",pd->num_radios, pd->power_state[0],pd->power_state[1],pd->power_state[2],pd->power_state[3]);
	return 0;
     */
}


SInt32
darwin_iwi3945::getPOWERSAVE(IO80211Interface *interface,
                             apple80211_powersave_data *psd)
{
    psd->version = APPLE80211_VERSION;
    psd->powersave_level = APPLE80211_POWERSAVE_MODE_80211;
    
    return kIOReturnSuccess;
}

SInt32
darwin_iwi3945::setPOWERSAVE(IO80211Interface *interface,
                             apple80211_powersave_data *psd)
{
    IOLog("Warning: Ignored a setPOWERSAVE\n");
    return kIOReturnSuccess;
}





SInt32 darwin_iwi3945::getASSOCIATE_RESULT( IO80211Interface * interface, 
                           struct apple80211_assoc_result_data * ard )
{
	IOLog("getASSOCIATE_RESULT \n");
	return 0;
}

SInt32
darwin_iwi3945::getRATE(IO80211Interface *interface,
					   struct apple80211_rate_data *rd)
{
	IOLog("getRATE %d\n",rd->rate);
	return 0;
}

SInt32
darwin_iwi3945::getSTATUS_DEV(IO80211Interface *interface,
							 struct apple80211_status_dev_data *dd)
{
    if( !interface ) {
        IOLog("No interface object exists!\n");
        return -1;
    }
    if( dd == NULL ) {
        IOLog("Quit calling getSTATUS_DEV without *dd!\n");
        return -1;
    }



    dd->version = APPLE80211_VERSION;

    bzero(dd->dev_name, sizeof(dd->dev_name));
    strncpy((char*)dd->dev_name, "iwi3945", sizeof(dd->dev_name));


	return kIOReturnSuccess;
}

SInt32
darwin_iwi3945::getRATE_SET(IO80211Interface	*interface,
						   struct apple80211_rate_set_data *rd)
{
	IOLog("getRATE_SET %d r0:%d f0:%d\n",rd->num_rates, rd->rates[0].rate,rd->rates[0].flags);
	return 0;
}

SInt32	darwin_iwi3945::getASSOCIATION_STATUS( IO80211Interface * interface, struct apple80211_assoc_status_data * asd )
{
	IOLog("getASSOCIATION_STATUS %d\n",asd->status);
	return 0;
}


SInt32 
darwin_iwi3945::getLOCALE(IO80211Interface *interface, apple80211_locale_data *ld)
{
    
    ld->version = APPLE80211_VERSION;
    ld->locale  = APPLE80211_LOCALE_FCC;
 
    
    return kIOReturnSuccess;
}

SInt32 
darwin_iwi3945::getCOUNTRY_CODE(IO80211Interface *interface, apple80211_country_code_data *cd) {
    cd->version = APPLE80211_VERSION;
    strncpy((char*)cd->cc, "us", sizeof(cd->cc));
    return kIOReturnSuccess;
}


SInt32
darwin_iwi3945::getPHY_MODE(IO80211Interface *interface, apple80211_phymode_data *pd)
{
    pd->version = APPLE80211_VERSION;
    pd->phy_mode = APPLE80211_MODE_11A | APPLE80211_MODE_11B | APPLE80211_MODE_11G;
    pd->active_phy_mode = APPLE80211_MODE_11B;
    return kIOReturnSuccess;
}

SInt32 
darwin_iwi3945::getINT_MIT(IO80211Interface *interface, apple80211_intmit_data *mitd)
{
    mitd->version = APPLE80211_VERSION;
    mitd->int_mit = APPLE80211_INT_MIT_AUTO;
    return kIOReturnSuccess;
}

SInt32 
darwin_iwi3945::getTXPOWER(IO80211Interface *interface, apple80211_txpower_data *tx)
{
    tx->version = APPLE80211_VERSION;
    tx->txpower_unit = APPLE80211_UNIT_PERCENT;
    tx->txpower = 80;
    
    return kIOReturnSuccess;
}

SInt32 
darwin_iwi3945::getOP_MODE(IO80211Interface *interface, apple80211_opmode_data *od)
{
    od->version = APPLE80211_VERSION;
    od->op_mode = APPLE80211_M_STA;
    return kIOReturnSuccess;
}

SInt32 
darwin_iwi3945::getNOISE(IO80211Interface *interface, apple80211_noise_data *nd)
{
    nd->version = APPLE80211_VERSION;
    nd->num_radios = 3;
    nd->noise_unit = APPLE80211_UNIT_PERCENT;
    nd->noise[0] = 90;
    nd->noise[1] = 80;
    nd->noise[2] = 70;
    nd->aggregate_noise = 40;
    
    return kIOReturnSuccess;
}

SInt32
darwin_iwi3945::getSCAN_RESULT(IO80211Interface *interface, apple80211_scan_result **sr)
{
    IOLog("Someone wanted a scan result.\n");
    IOLog("Scan result *sr: 0x%08x\n", sr);
	myState = APPLE80211_S_INIT;
	
	
	/*sr->version = APPLE80211_VERSION;
	sr->asr_noise = 60; //oh good AP XD
	sr->asr_cap = 0xab;		// Same as us ;)
	sr->asr_bssid.octet[0] = 0xFE;
    sr->asr_bssid.octet[1] = 0xDC;
    sr->asr_bssid.octet[2] = 0xBA;
    sr->asr_bssid.octet[3] = 0x98;
    sr->asr_bssid.octet[4] = 0x76;
    sr->asr_bssid.octet[5] = 0x54;
	
	
	strncpy((char*)sr->asr_ssid, "anetwork", sizeof(sr->asr_ssid));
    sr->asr_ssid_len = strlen("anetwork");
	
	sr->asr_age = 1;	// (ms) non-zero for cached scan result
	sr->asr_ie_len = 0;
	sr->asr_ie_data = NULL; */
	
    return kIOReturnSuccess;
}



SInt32 
darwin_iwi3945::setRATE(IO80211Interface *interface, apple80211_rate_data *rd)
{
    IOLog("Warning: ignored setRATE\n");
    return kIOReturnSuccess;
}

SInt32 
darwin_iwi3945::getSUPPORTED_CHANNELS(IO80211Interface *interface, apple80211_sup_channel_data *ad) {
    ad->version = APPLE80211_VERSION;
    ad->num_channels = 13;
    
    int i;
    for(i=1; i<=ad->num_channels; i++) {
        ad->supported_channels[i-1].version = APPLE80211_VERSION;
        ad->supported_channels[i-1].channel = i;
        ad->supported_channels[i-1].flags   = APPLE80211_C_FLAG_2GHZ;
    }
    
    return kIOReturnSuccess;
}
        

SInt32 
darwin_iwi3945::getTX_ANTENNA(IO80211Interface *interface, apple80211_antenna_data *ad)
{
    ad->version = APPLE80211_VERSION;
    ad->num_radios = 3;
    ad->antenna_index[0] = 1;
    ad->antenna_index[1] = 1;
    ad->antenna_index[2] = 1;
    return kIOReturnSuccess;
}
    

SInt32 
darwin_iwi3945::getANTENNA_DIVERSITY(IO80211Interface *interface, apple80211_antenna_data *ad)
{
    ad->version = APPLE80211_VERSION;
    ad->num_radios = 3;
    ad->antenna_index[0] = 1;
    ad->antenna_index[1] = 1;
    ad->antenna_index[2] = 1;
    return kIOReturnSuccess;
}

SInt32
darwin_iwi3945::getSTATION_LIST(IO80211Interface *interface, apple80211_sta_data *sd)
{
    int i;
    IOLog("Feeding a list of stations to the driver...\n");
    sd->num_stations = 4;
    
    for(i=0; i<4; i++) {
        struct apple80211_station *sta = &(sd->station_list[i]);
        
        sta->version = APPLE80211_VERSION;
        
        memset(&(sta->sta_mac), i, sizeof(sta->sta_mac));
        sta->sta_rssi = 1;
    }
    
    
    return kIOReturnSuccess;
}

SInt32 
darwin_iwi3945::setANTENNA_DIVERSITY(IO80211Interface *interface, apple80211_antenna_data *ad)
{
    IOLog("Warning: ignoring setANTENNA_DIVERSITY\n");
    return kIOReturnSuccess;
}

SInt32 
darwin_iwi3945::setTX_ANTENNA(IO80211Interface *interface, apple80211_antenna_data *ad)
{
    IOLog("Warning: ignoring setTX_ANTENNA\n");
    return kIOReturnSuccess;
}


SInt32 
darwin_iwi3945::setTXPOWER(IO80211Interface *interface, apple80211_txpower_data *td)
{
    IOLog("Warning: Ignored setTXPOWER\n");
    return kIOReturnSuccess;
}

SInt32 
darwin_iwi3945::setINT_MIT(IO80211Interface *interface, apple80211_intmit_data *md)
{
    IOLog("Warning: Ignored setINT_MIT\n");
    return kIOReturnSuccess;
}

SInt32 
darwin_iwi3945::getPROTMODE(IO80211Interface *interface, apple80211_protmode_data *pd)
{
	pd->version = APPLE80211_VERSION;
	pd->protmode = APPLE80211_PROTMODE_OFF; //no prot at this moment
	pd->threshold = 8;		// number of bytes
    return kIOReturnSuccess;
}


SInt32 
darwin_iwi3945::setPROTMODE(IO80211Interface *interface, apple80211_protmode_data *pd)
{
    IOLog("Warning: Ignored setPROTMODE\n");
    return kIOReturnSuccess;
}



SInt32
darwin_iwi3945::setPHY_MODE(IO80211Interface *interface, apple80211_phymode_data *pd)
{
    IOLog("Warning: Ignoring a setPHY_MODE\n");
    return kIOReturnSuccess;
}


SInt32
darwin_iwi3945::setLOCALE(IO80211Interface *interface, apple80211_locale_data *ld) {
    IOLog("Warning: Ignored a setLOCALE\n");
    return kIOReturnSuccess;
}



SInt32
darwin_iwi3945::setSCAN_REQ(IO80211Interface *interface,
						   struct apple80211_scan_data *sd)
{
    if( !sd ) {
        IOLog("Please don't call setSCAN_REQ without an *sd\n");
        return -1 ;
    }
    
    IOLog("Scan requested.  Type: %d\n", sd->scan_type);
    myState = APPLE80211_S_SCAN;
    //if( sd->scan_type == APPLE80211_SCAN_TYPE_ACTIVE ) {
    //    memcpy(sd->bssid.octet, "DACAFEBABE", sizeof(sd->bssid.octet));
    //}
	
	//hw scan
	start_undirect_scan();
	IOSleep(1000);
    myState = APPLE80211_S_INIT;
	
    postMessage(APPLE80211_IOC_SCAN_REQ);
    
	return kIOReturnSuccess;
}

SInt32
darwin_iwi3945::setASSOCIATE(IO80211Interface *interface,struct apple80211_assoc_data *ad)
{
	IOLog("setASSOCIATE \n");
    
    postMessage(APPLE80211_IOC_SCAN_RESULT);
	return 0;
}

SInt32
darwin_iwi3945::setPOWER(IO80211Interface *interface,
						struct apple80211_power_data *pd)
{
    if( NULL == pd ) {
        IOLog("Please don't call setPOWER without a *pd struct\n");
        return -1;
    }
    
    
    /*
	IOLog("setPOWER %d, %d %d %d %d\n",pd->num_radios, pd->power_state[0],pd->power_state[1],pd->power_state[2],pd->power_state[3]);
	if (pd->power_state[pd->num_radios]==1) {
		IOLog("power on\n");
	}
	else
	{
		IOLog("power off ignored\n");
		return -1;
	}
    */
    
	return kIOReturnSuccess;
}

SInt32
darwin_iwi3945::setCIPHER_KEY(IO80211Interface *interface,
							 struct apple80211_key *key)
{
	IOLog("setCIPHER_KEY \n");
	return 0;
}

SInt32
darwin_iwi3945::getAUTH_TYPE(IO80211Interface *interface,
							struct apple80211_authtype_data *ad)
{

	ad->version = APPLE80211_VERSION;
	ad->authtype_lower = APPLE80211_AUTHTYPE_OPEN;	//	open at this moment
	ad->authtype_upper = APPLE80211_AUTHTYPE_NONE;	//	NO upper AUTHTYPE
	return 0;
}

SInt32
darwin_iwi3945::setAUTH_TYPE(IO80211Interface *interface,
							struct apple80211_authtype_data *ad)
{
	IOLog("setAUTH_TYPE \n");
	return 0;
}

SInt32
darwin_iwi3945::setDISASSOCIATE(IO80211Interface	*interface)
{
	IOLog("setDISASSOCIATE \n");
	return 0;
}

SInt32
darwin_iwi3945::setSSID(IO80211Interface *interface,
					   struct apple80211_ssid_data *sd)
{
	IOLog("setSSID \n");
	return 0;
}

SInt32
darwin_iwi3945::setAP_MODE(IO80211Interface *interface,
						  struct apple80211_apmode_data *ad)
{
	IOLog("setAP_MODE \n");
	return 0;
}

bool darwin_iwi3945::attachInterfaceWithMacAddress( void * macAddr, 
												UInt32 macLen, 
												IONetworkInterface ** interface, 
												bool doRegister ,
												UInt32 timeout  )
{
	//IOLog("attachInterfaceWithMacAddress \n");
	//return super::attachInterfaceWithMacAddress(macAddr,macLen,interface,doRegister,timeout);
	return true;
}												
												
void darwin_iwi3945::dataLinkLayerAttachComplete( IO80211Interface * interface )											
{
	//IOLog("dataLinkLayerAttachComplete \n");
	//super::dataLinkLayerAttachComplete(interface);
	return;
}


#pragma mark -
#pragma mark System entry points

IOOptionBits darwin_iwi3945::getState( void ) const
{
	IOOptionBits r=super::getState();
	//IWI_DEBUG_FN("getState = %x\n",r);
	return r;
}


IOReturn setWakeOnMagicPacket( bool active )
{
    //magicPacketEnabled = active;
    return kIOReturnSuccess;
}

int darwin_iwi3945::up()
{
	//if_up()
}



void darwin_iwi3945::down()
{
	//if_down();
}




/*-------------------------------------------------------------------------
 * Called by IOEthernetInterface client to enable the controller.
 * This method is always called while running on the default workloop thread.
 *-------------------------------------------------------------------------*/

IOReturn darwin_iwi3945::enable( IONetworkInterface* netif )
{
	IOLog("darwin_iwi3945::enable()\n");
    
	if (!fifnet)
	{
		char ii[4];
		sprintf(ii,"%s%d" ,netif->getNamePrefix(), netif->getUnitNumber());
		ifnet_find_by_name(ii,&fifnet);
		setMyfifnet(fifnet);
	}
    
    /* Start our IOOutputQueue object:	*/
    fTransmitQueue->setCapacity( 1024 );
	fTransmitQueue->service(IOBasicOutputQueue::kServiceAsync);
    fTransmitQueue->start();
    
	ifnet_set_flags(fifnet, IFF_RUNNING, IFF_RUNNING );
	
    return kIOReturnSuccess;
}/* end enable netif */


/*-------------------------------------------------------------------------
 * Called by IOEthernetInterface client to disable the controller.
 * This method is always called while running on the default workloop
 * thread.
 *-------------------------------------------------------------------------*/

IOReturn darwin_iwi3945::disable( IONetworkInterface* /*netif*/ )
{
    IOLog("darwin_iwi3945::disable()\n");
    
    /* Disable our IOOutputQueue object. This will prevent the
     * outputPacket() method from being called.
     */
    
    fTransmitQueue->stop();
    
    fTransmitQueue->setCapacity( 0 );
    fTransmitQueue->flush();	/* Flush all packets currently in the output queue.	*/
    
    /* If we have no active clients, then disable the controller.	*/
    /*
	if ( debugEnabled == false )
		putToSleep( false );
    
    netifEnabled = false;
    */
    return kIOReturnSuccess;
}/* end disable netif */



/*SInt32 darwin_iwi3945::apple80211_ioctl(
                                        IO80211Interface *interface, 
                                        ifnet_t ifn, 
                                        u_int32_t cmd, 
                                        void *data)
{
    IOLog("darwin_iwi3945::apple80211_ioctl(%d, %d, %p)\n", ifn, cmd, data);
    return super::apple80211_ioctl(interface, ifn, cmd, data);
}*/

void darwin_iwi3945::interruptOccurred(OSObject * owner, 
	void		*src,  IOService *nub, int source)
{
}
IOReturn darwin_iwi3945::setMulticastMode(bool active) {

	return kIOReturnSuccess;
}

IOReturn darwin_iwi3945::setMulticastList(IOEthernetAddress * addrs, UInt32 count) {
	 return kIOReturnSuccess;
}

IOReturn darwin_iwi3945::getPacketFilters(const OSSymbol * group, UInt32 *         filters) const
{
	 if ( ( group == gIOEthernetWakeOnLANFilterGroup ) )//&& ( magicPacketSupported ) )
	{
		*filters = kIOEthernetWakeOnMagicPacket;
		return kIOReturnSuccess;
	}

    // For any other filter groups, return the default set of filters
    // reported by IOEthernetController.

	return super::getPacketFilters( group, filters );
}

void darwin_iwi3945::getPacketBufferConstraints(IOPacketBufferConstraints * constraints) const {
	assert(constraintsP);
    constraints->alignStart  = kIOPacketBufferAlign1;	
    constraints->alignLength = kIOPacketBufferAlign1;	
}

IOReturn darwin_iwi3945::enablePacketFilter(const OSSymbol * group,
                                        UInt32           aFilter,
                                        UInt32           enabledFilters,
                                        IOOptionBits     options)
{
	return super::enablePacketFilter(group,aFilter,enabledFilters,options);
}

IOReturn darwin_iwi3945::getMaxPacketSize(UInt32 * maxSize) const
{
    *maxSize = 1600;//kIOEthernetMaxPacketSize;//;//IPW_RX_BUF_SIZE;
    return kIOReturnSuccess;
}

IOReturn darwin_iwi3945::getMinPacketSize(UInt32 * minSize) const
{
    *minSize = 32;//kIOEthernetMinPacketSize;//;
    return kIOReturnSuccess;
}

bool darwin_iwi3945::configureInterface( IONetworkInterface *netif )
{
    IOLog("darwin_iwi3945::configureInterface()\n");
    return super::configureInterface(netif);
}


//FIXME: Mac from iwl3945
IOReturn darwin_iwi3945::getHardwareAddress(IOEthernetAddress *addr)
{
	addr = (IOEthernetAddress*)getMyMacAddr();
    return kIOReturnSuccess;
}

IO80211Interface *darwin_iwi3945::getNetworkInterface()
{
    //IOLog("darwin_iwi3945::getNetworkInterface()\n");
    //return super::getNetworkInterface();
	return NULL;
}



IOOutputQueue *darwin_iwi3945::getOutputQueue() const {
    //IOLog("Getting output queue\n");
    return fTransmitQueue;
}



bool darwin_iwi3945::addMediumType(UInt32 type, UInt32 speed, UInt32 code, char* name) {    
    IONetworkMedium * medium;
    bool              ret = false;
    
    medium = IONetworkMedium::medium(type, speed, 0, code, name);
    if (medium) {
        ret = IONetworkMedium::addMedium(mediumDict, medium);
        if (ret)
            mediumTable[code] = medium;
        medium->release();
    }
    return ret;
}


/*static IOReturn darwin_iwi3945::powerChangeHandler(void *target, void *refCon, UInt32
            messageType, IOService *service, void *messageArgument,
            vm_size_t argSize ) {
    IOLog("Called powerChangeHandler.  Ignoring.\n");
    return 0;
}

static IOReturn darwin_iwi3945::powerDownHandler(void *target, void *refCon, UInt32
            messageType, IOService *service, void *messageArgument,
            vm_size_t argSize ) {
    IOLog("Called powerDownHandler.  Ignoring.\n");
    return 0;
}*/

classdef wsaConstants
     
    properties
    
        WSA4000
        WSA5000
                   
        WSA_MIN_ANTENNA
        WSA_MAX_ANTENNA
        WSA_MIN_FSHIFT
        WSA_MAX_FSHIFT
        WSA_MIN_IF_GAIN 
        WSA_MAX_IF_GAIN
        WSA_MIN_RF_GAIN 
        WSA_MAX_RF_GAIN
        WSA_MIN_SPP
        WSA_MAX_SPP
        WSA_MIN_PPB
        WSA_MAX_PPB
        WSA_TRIGGER_MODE_LEVEL
        WSA_TRIGGER_MODE_NONE
        WSA_TRIGGER_MODE_PULSE
        WSA_TRIGGER_SYNC_STATE_MASTER
        WSA_TRIGGER_SYNC_STATE_SLAVE
        WSA_BLOCK_CAPTURE_MODE
        WSA_STREAM_CAPTURE_MODE
        WSA_SWEEP_CAPTURE_MODE

        I16Q16_DATA_STREAM_ID
        I16_DATA_STREAM_ID
        I32_DATA_STREAM_ID   
        IF_PACKET_TYPE
        CONTEXT_PACKET_TYPE
        EXTENSION_PACKET_TYPE
        
        DIGITIZER_STREAM_ID
        RECEIVER_STREAM_ID
        BANDWIDTH_INDICATOR_MASK
        REFERENCE_LEVEL_INDICATOR_MASK
        RF_OFFSET_INDICATOR_MASK
    
        WSA_ERR_NOWSA
        WSA_ERR_UNKNOWNPRODSER
        WSA_ERR_UNKNOWNPRODVSN
        WSA_ERR_UNKNOWNFWRVSN
        WSA_ERR_UNKNOWNRFEVSN
        WSA_ERR_PRODOBSOLETE
        WSA_ERR_DATAACCESSDENIED
        WSA_ERR_INV4000COMMAND
        WSA_ERR_INV5000COMMAND
        WSA_ERR_WSANOTRDY
        WSA_ERR_WSAINUSE
        WSA_ERR_SETFAILED
        WSA_ERR_OPENFAILED
        WSA_ERR_INITFAILED
        WSA_ERR_INVADCCORRVALUE
        WSA_ERR_INVINTFMETHOD
        WSA_ERR_USBNOTAVBL
        WSA_ERR_USBOPENFAILED
        WSA_ERR_USBINITFAILED
        WSA_ERR_INVIPHOSTADDRESS
        WSA_ERR_ETHERNETNOTAVBL
        WSA_ERR_ETHERNETCONNECTFAILED
        WSA_ERR_ETHERNETINITFAILED
        WSA_ERR_WINSOCKSTARTUPFAILED
        WSA_ERR_SOCKETSETFUPFAILED
        WSA_ERR_SOCKETERROR
        WSA_ERR_SOCKETDROPPED
        WSA_ERR_NODATABUS
        WSA_ERR_READFRAMEFAILED
        WSA_ERR_INVSAMPLESIZE
        WSA_ERR_NOTIQFRAME
        WSA_ERR_INVDECIMATIONRATE
        WSA_ERR_VRTPACKETSIZE
        WSA_ERR_INVTIMESTAMP
        WSA_ERR_INVCAPTURESIZE
        WSA_ERR_PACKETOUTOFORDER
        WSA_ERR_CAPTUREACCESSDENIED
        WSA_ERR_FREQOUTOFBOUND
        WSA_ERR_INVFREQRES
        WSA_ERR_PLLLOCKFAILED
        WSA_ERR_INVSTOPFREQ
        WSA_ERR_STARTOOB
        WSA_ERR_STOPOOB
        WSA_ERR_INVRFGAIN
        WSA_ERR_INVIFGAIN
        WSA_ERR_INVRUNMODE
        WSA_ERR_INVTRIGGERMODE
        WSA_ERR_INVTRIGGERDELAY
        WSA_ERR_INVTRIGGERSYNC  
        WSA_ERR_INVDWELL
        WSA_ERR_NOCTRLPIPE
        WSA_ERR_CMDSENDFAILED
        WSA_ERR_CMDINVALID
        WSA_ERR_RESPUNKNOWN
        WSA_ERR_QUERYNORESP
        WSA_ERR_INVANTENNAPORT
        WSA_ERR_INVFILTERMODE
        WSA_ERR_INVCALIBRATEMODE
        WSA_ERR_INVRFESETTING
        WSA_ERR_INVPLLREFSOURCE
        WSA_ERR_FILECREATEFAILED
        WSA_ERR_FILEOPENFAILED
        WSA_ERR_FILEREADFAILED
        WSA_ERR_FILEWRITEFAILED
        WSA_ERR_INVNUMBER
        WSA_ERR_INVREGADDR
        WSA_ERR_MALLOCFAILED
        WSA_ERR_UNKNOWN_ERROR
        WSA_ERR_INVINPUT
        WSA_ERR_SWEEPSTARTFAIL
        WSA_ERR_SWEEPSTOPFAIL
        WSA_ERR_SWEEPRESUMEFAIL
        WSA_ERR_SWEEPSTATUSFAIL
        WSA_ERR_SWEEPSIZEFAIL
        WSA_ERR_SWEEPENTRYSAVEFAIL
        WSA_ERR_SWEEPENTRYCOPYFAIL
        WSA_ERR_SWEEPENTRYNEWFAIL
        WSA_ERR_SWEEPENTRYDELETEFAIL
        WSA_ERR_SWEEPALREADYRUNNING
        WSA_ERR_SWEEPLISTEMPTY
        WSA_ERR_SWEEPNOTRUNNING
        WSA_ERR_SWEEPIDOOB
        WSA_ERR_SWEEPMODEUNDEF
        WSA_ERR_INVSWEEPSTARTID
        WSA_ERR_SWEEPWHILESTREAMING
        WSA_ERR_STREAMALREADYRUNNING	
        WSA_ERR_STREAMNOTRUNNING   
        WSA_ERR_STREAMWHILESWEEPING 
        WSA_ERR_INVSTREAMSTARTID	
    end
    
    methods
        
        function this = wsaConstants()
            
            this.WSA4000 = 'WSA4000';
            this.WSA5000 = 'WSA5000';
            
            % valid antenna ports
            this.WSA_MIN_ANTENNA = 1;
            this.WSA_MAX_ANTENNA = 2;
            
            % frequency shift constants
            this.WSA_MAX_FSHIFT = 125000000;
            this.WSA_MIN_FSHIFT = -125000000;
            
            % gain constants
            this.WSA_MIN_IF_GAIN = -10;
            this.WSA_MAX_IF_GAIN = 34;
            this.WSA_MIN_RF_GAIN  = 1;
            this.WSA_MAX_RF_GAIN = 4;
                       
            % packet size constants
            this.WSA_MIN_SPP = 128;
            this.WSA_MAX_SPP = 65529;
            this.WSA_MIN_PPB = 1;
            this.WSA_MAX_PPB =  4294967295;
            
            % trigger modes
            this.WSA_TRIGGER_MODE_LEVEL = 'LEVEL';
            this.WSA_TRIGGER_MODE_NONE = 'NONE';
            this.WSA_TRIGGER_MODE_PULSE = 'PULSE';
            
            % trigger sync states
            this.WSA_TRIGGER_SYNC_STATE_MASTER = 'MASTER';
            this.WSA_TRIGGER_SYNC_STATE_SLAVE = 'SLAVE';
            
            % capture modes
            this.WSA_BLOCK_CAPTURE_MODE = 'BLOCK';
            this.WSA_STREAM_CAPTURE_MODE = 'STREAMING';
            this.WSA_SWEEP_CAPTURE_MODE = 'SWEEPING';
            
            % wsa vrt packet types
            this.IF_PACKET_TYPE = 1;
            this.CONTEXT_PACKET_TYPE = 4;
            this.EXTENSION_PACKET_TYPE = 5;
            
            % wsa vrt stream ids
            this.I16Q16_DATA_STREAM_ID = -1879048189;
            this.I16_DATA_STREAM_ID = -1879048187;
            this.I32_DATA_STREAM_ID = -1879048186;
            
            this.DIGITIZER_STREAM_ID = -1879048190;
            this.RECEIVER_STREAM_ID = -1879048191;
            
            this.BANDWIDTH_INDICATOR_MASK =  -1610612736;
            this.REFERENCE_LEVEL_INDICATOR_MASK = -2130706432;
            this.RF_OFFSET_INDICATOR_MASK = -2080374784;
    
            LNEG_NUM = (-10000);


            % // ///////////////////////////////
            % // WSA RELATED ERRORS		      //
            % // ///////////////////////////////
            this.WSA_ERR_NOWSA = (LNEG_NUM - 1);
            this.WSA_ERR_UNKNOWNPRODSER = (LNEG_NUM - 2);
            this.WSA_ERR_UNKNOWNPRODVSN = (LNEG_NUM - 3);
            this.WSA_ERR_UNKNOWNFWRVSN = (LNEG_NUM - 4);
            this.WSA_ERR_UNKNOWNRFEVSN = (LNEG_NUM - 5);
            this.WSA_ERR_PRODOBSOLETE = (LNEG_NUM - 6);
            this.WSA_ERR_DATAACCESSDENIED = (LNEG_NUM - 7);
            this.WSA_ERR_INV4000COMMAND = (LNEG_NUM - 8);
            this.WSA_ERR_INV5000COMMAND = (LNEG_NUM - 9);
            
            % // ///////////////////////////////
            % // WSA SETUP ERRORS			  //
            % // ///////////////////////////////
            this.WSA_ERR_WSANOTRDY = (LNEG_NUM - 101);
            this.WSA_ERR_WSAINUSE = (LNEG_NUM - 102);
            this.WSA_ERR_SETFAILED = (LNEG_NUM - 103);
            this.WSA_ERR_OPENFAILED = (LNEG_NUM - 104);
            this.WSA_ERR_INITFAILED = (LNEG_NUM - 105);
            this.WSA_ERR_INVADCCORRVALUE = (LNEG_NUM - 106);

             % // ///////////////////////////////
             % // INTERFACE/CONNECTION ERRORS  //
             % // ///////////////////////////////
             this.WSA_ERR_INVINTFMETHOD	= (LNEG_NUM - 201);
             this.WSA_ERR_USBNOTAVBL = (LNEG_NUM - 202);
             this.WSA_ERR_USBOPENFAILED	= (LNEG_NUM - 203);
             this.WSA_ERR_USBINITFAILED	= (LNEG_NUM - 204);
             this.WSA_ERR_INVIPHOSTADDRESS = (LNEG_NUM - 205);
             this.WSA_ERR_ETHERNETNOTAVBL = (LNEG_NUM - 206);
             this.WSA_ERR_ETHERNETCONNECTFAILED	= (LNEG_NUM - 207);
             this.WSA_ERR_ETHERNETINITFAILED = (LNEG_NUM - 209);
             this.WSA_ERR_WINSOCKSTARTUPFAILED = (LNEG_NUM - 210);
             this.WSA_ERR_SOCKETSETFUPFAILED = (LNEG_NUM - 211);
             this.WSA_ERR_SOCKETERROR = (LNEG_NUM - 212);
             this.WSA_ERR_SOCKETDROPPED = (LNEG_NUM - 213);

             % // ///////////////////////////////
             % // DATA ACQUISITION ERRORS      //
             % // ///////////////////////////////
             this.WSA_ERR_NODATABUS = (LNEG_NUM - 401);
             this.WSA_ERR_READFRAMEFAILED = (LNEG_NUM - 402);
             this.WSA_ERR_INVSAMPLESIZE	= (LNEG_NUM - 403);
             this.WSA_ERR_NOTIQFRAME = (LNEG_NUM - 405);
             this.WSA_ERR_INVDECIMATIONRATE = (LNEG_NUM - 406);
             this.WSA_ERR_VRTPACKETSIZE = (LNEG_NUM - 407);
             this.WSA_ERR_INVTIMESTAMP = (LNEG_NUM - 408);
             this.WSA_ERR_INVCAPTURESIZE = (LNEG_NUM - 409);
             this.WSA_ERR_PACKETOUTOFORDER = (LNEG_NUM - 410);
             this.WSA_ERR_CAPTUREACCESSDENIED = (LNEG_NUM - 411);

             % // ///////////////////////////////
             % // FREQUENCY ERRORS			   //
             % // ///////////////////////////////
             this.WSA_ERR_FREQOUTOFBOUND = (LNEG_NUM - 601);
             this.WSA_ERR_INVFREQRES = (LNEG_NUM - 602);
             this.WSA_ERR_PLLLOCKFAILED	= (LNEG_NUM - 603);
             this.WSA_ERR_INVSTOPFREQ = (LNEG_NUM - 604);
             this.WSA_ERR_STARTOOB = (LNEG_NUM - 605);
             this.WSA_ERR_STOPOOB = (LNEG_NUM - 606);

             % // ///////////////////////////////
             % // GAIN ERRORS                  //
             % // ///////////////////////////////
             this.WSA_ERR_INVRFGAIN	= (LNEG_NUM - 801);
             this.WSA_ERR_INVIFGAIN	= (LNEG_NUM - 802);


             % // ///////////////////////////////
             % // RUNMODE ERRORS				//
             % // ///////////////////////////////
             this.WSA_ERR_INVRUNMODE = (LNEG_NUM - 1001);


             % // ///////////////////////////////
             % // TRIGGER ERRORS		       //
             % // ///////////////////////////////
             this.WSA_ERR_INVTRIGGERMODE = (LNEG_NUM - 1201);
             this.WSA_ERR_INVTRIGGERDELAY = (LNEG_NUM - 1202);
             this.WSA_ERR_INVTRIGGERSYNC = (LNEG_NUM - 1203);  

             % // ///////////////////////////////
             % // TIME RELATED ERRORS	       //
             % // ///////////////////////////////
             this.WSA_ERR_INVDWELL = (LNEG_NUM - 1301);

             % // ///////////////////////////////
             % // CTRL/CMD ERRORS	           //
             % // ///////////////////////////////
             this.WSA_ERR_NOCTRLPIPE = (LNEG_NUM - 1500);
             this.WSA_ERR_CMDSENDFAILED	= (LNEG_NUM - 1501);
             this.WSA_ERR_CMDINVALID = (LNEG_NUM - 1502);
             this.WSA_ERR_RESPUNKNOWN = (LNEG_NUM - 1503);
             this.WSA_ERR_QUERYNORESP = (LNEG_NUM - 1504);


             % // ///////////////////////////////
             % // RFE ERRORS				    //
             % // ///////////////////////////////
             this.WSA_ERR_INVANTENNAPORT = (LNEG_NUM - 1601);
             this.WSA_ERR_INVFILTERMODE = (LNEG_NUM - 1602);
             this.WSA_ERR_INVCALIBRATEMODE = (LNEG_NUM - 1603);
             this.WSA_ERR_INVRFESETTING = (LNEG_NUM - 1604);
             this.WSA_ERR_INVPLLREFSOURCE = (LNEG_NUM - 1605);


             % // ///////////////////////////////
             % // FILE RELATED ERRORS			//
             % // ///////////////////////////////
             this.WSA_ERR_FILECREATEFAILED = (LNEG_NUM - 1900);
             this.WSA_ERR_FILEOPENFAILED = (LNEG_NUM - 1901);
             this.WSA_ERR_FILEREADFAILED = (LNEG_NUM - 1902);
             this.WSA_ERR_FILEWRITEFAILED = (LNEG_NUM - 1903);


             % // ///////////////////////////////
             % // OTHERS ERRORS				//
             % // ///////////////////////////////
             this.WSA_ERR_INVNUMBER	= (LNEG_NUM - 2000);
             this.WSA_ERR_INVREGADDR = (LNEG_NUM - 2001);
             this.WSA_ERR_MALLOCFAILED = (LNEG_NUM - 2002);
             this.WSA_ERR_UNKNOWN_ERROR	= (LNEG_NUM - 2003);
             this.WSA_ERR_INVINPUT = (LNEG_NUM - 2004);

             % // ///////////////////////////////
             % // SWEEP ERRORS				   //
             % // ///////////////////////////////
             this.WSA_ERR_SWEEPSTARTFAIL = (LNEG_NUM - 3000);
             this.WSA_ERR_SWEEPSTOPFAIL	= (LNEG_NUM - 3001);
             this.WSA_ERR_SWEEPRESUMEFAIL = (LNEG_NUM - 3002);
             this.WSA_ERR_SWEEPSTATUSFAIL = (LNEG_NUM - 3003);
             this.WSA_ERR_SWEEPSIZEFAIL	= (LNEG_NUM - 3004);
             this.WSA_ERR_SWEEPENTRYSAVEFAIL = (LNEG_NUM - 3005);
             this.WSA_ERR_SWEEPENTRYCOPYFAIL = (LNEG_NUM - 3006);
             this.WSA_ERR_SWEEPENTRYNEWFAIL	= (LNEG_NUM - 3007);
             this.WSA_ERR_SWEEPENTRYDELETEFAIL = (LNEG_NUM - 3008);
             this.WSA_ERR_SWEEPALREADYRUNNING = (LNEG_NUM - 3009);
             this.WSA_ERR_SWEEPLISTEMPTY = (LNEG_NUM - 3010);
             this.WSA_ERR_SWEEPNOTRUNNING = (LNEG_NUM - 3011);
             this.WSA_ERR_SWEEPIDOOB = (LNEG_NUM - 3012);
             this.WSA_ERR_SWEEPMODEUNDEF = (LNEG_NUM - 3013);
             this.WSA_ERR_INVSWEEPSTARTID = (LNEG_NUM - 3014);
             this.WSA_ERR_SWEEPWHILESTREAMING = (LNEG_NUM - 3015);
            
            %// ///////////////////////////////
            %// STREAM ERRORS  			     //
            %// ///////////////////////////////
            this.WSA_ERR_STREAMALREADYRUNNING =	(LNEG_NUM - 4000);
            this.WSA_ERR_STREAMNOTRUNNING = (LNEG_NUM - 4001);
            this.WSA_ERR_STREAMWHILESWEEPING = (LNEG_NUM - 4002);
            this.WSA_ERR_INVSTREAMSTARTID = (LNEG_NUM - 4003);
        end
    end
       
end
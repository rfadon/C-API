%%=======================================================================%%
%% ====================   WSA MATLAB INTERFACE   ======================= %%
%%=======================================================================%% 

% This script contains the WSA matlab interface class

classdef wsaInterface < handle
    properties
        cppObjectHandle;
        connected = false;
        minFrequency = 0
        maxFrequency = 0
        bandwidth = 0
        minIfGain = 0
        maxIfGain = 0
        minDecimationRate = 0
        maxDecimationRate = 0
        model = 'None';
        macAddress = 'None';
        firmwareVersion = 'None';
        constants;
    end
    methods
        
        % Constructor to build the handle
        function this = wsaInterface()
            this.cppObjectHandle = wsaOpenConnection();
            this.constants = wsaConstants;
        end
        
        % Destruct the WSA interface class
        function delete(this)
            wsaCloseConnection(this.cppObjectHandle);
            this.connected = false;
        end
        
        %%=======================================================================%%
        %%                           CONNECT/DISCONNECT SECTION                  %%
        %%=======================================================================%%    
        
        % Establish a connection
        % 
        % this: refers to the wsaInterface
        % ipAdress: string data type 
        % 
        % errorCode: int32 data type  
        % errorMessage: string data type 
        function [errorCode errorMessage ] = connect(this, ipAdress)
            
            [errorCode ...
                errorMessage ...
                this.model...
                this.firmwareVersion...
                this.macAddress...
                this.minFrequency ...
                this.maxFrequency ...
                this.bandwidth ...
                this.minDecimationRate ...
                this.maxDecimationRate ...
            ] = wsaConnect(this.cppObjectHandle, ipAdress);
            if errorCode >= 0
                this.connected = true;
            end
        end
        
        
         % Close an established connection to the wsa
         % this: refers to the wsaInterface
         function disconnect(this)
            wsaCloseConnection(this.cppObjectHandle);
            this.connected = false;
         end
         
        %%=======================================================================%%
        %%                   DEVICE CONTROL SECTION                            %
        %%=======================================================================%%         
        
        % Reset the WSA to default settings.
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type  
        % errorMessage: string data type
        % 
        function [errorCode errorMessage] = reset(this)
            command = ':*rst';
            [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);        
        end
        
        
        %%=======================================================================%%
        %%                   RFE MODE CONTROL SECTION                            %
        %%=======================================================================%%         
        
        
        % Retrieve the current WSA RFE mode
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type  
        % errorMessage: string data type
        % rfeMode:String data type 
        % 
        function [errorCode errorMessage RFEMode] = getRfeMode(this)
            command = 'INPUT:MODE?';
            [errorCode errorMessage RFEMode] = wsaQuerySCPICommand(this.cppObjectHandle, command);
        
        end
        
        
        % Set the WSA's RFE mode
        % 
        % this: refers to the wsaInterface
        % rfeMode: String data type, WSA RFE Mode (ZIF, HDR, or SH)
        %
        % errorCode: int32 data type  
        % errorMessage: string data type
        % 
        function [errorCode errorMessage] = setRfeMode(this, rfeMode)
            command = sprintf('INPUT:MODE %s',rfeMode);
            [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);        
        end
        
        
        %%=======================================================================%%
        %%                   FREQUENCY CONTROL SECTION                            %
        %%=======================================================================%%         
        
        % Retrieve the center frequency that the WSA tuned to.
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type  
        % errorMessage: string data type
        % centerFrequency:int64 data type (Hz)  
        % 
        function [errorCode errorMessage centerFrequency] = getFrequency(this)
            command = 'FREQ:CENT?';
            [errorCode errorMessage response] = wsaQuerySCPICommand(this.cppObjectHandle, command);
            centerFrequency = str2double(response);             
        end
        
        
        % Tune the WSA to the desired center frequency
        %
        % this: refers to the wsaInterface
        % centerFrequency:int64 data type (Hz) 
        %
        % errorCode: int32 data type  
        % errorMessage: string data type 
        function [errorCode errorMessage] = setFrequency(this, centerFrequency) 
            if ((centerFrequency > this.maxFrequency) || (centerFrequency < this.minFrequency))
                
                errorCode = this.constants.WSA_ERR_FREQOUTOFBOUND;
                errorMessage = getErrorMessage(errorCode);    
            else
              command = sprintf('FREQ:CENT %d Hz', centerFrequency);
              [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
            end 
        end
               
        
        % Get the frequency shift value in the WSA
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type 
        % errorMessage: string data type
        % frequencyShift: int64 data type (Hz) 
        function [errorCode errorMessage frequencyShift] = getFrequencyShift(this)
            command = 'FREQ:SHIFT?';
            [errorCode errorMessage response] = wsaQuerySCPICommand(this.cppObjectHandle, command);
            frequencyShift = str2double(response);
            
        end  
        
        
        % Set the frequency shift value in the WSA
        %
        % this: refers to the wsaInterface
        % frequencyShift: int64 data type (Hz) 
        %
        % errorCode: int32 data type  
        % errorMessage: string data type 
        function [errorCode errorMessage] = setFrequencyShift(this, frequencyShift) 
            if (frequencyShift > this.constants.WSA_MAX_FSHIFT ||...
                frequencyShift < this.constants.WSA_MIN_FSHIFT)
				
                errorCode = this.constants.WSA_ERR_FREQOUTOFBOUND;
                errorMessage = getErrorMessage(errorCode);
            else            
				command = sprintf('FREQ:SHIFT %d Hz', (frequencyShift));
				[errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
            
            end
            
        end
        
        
        %%=======================================================================%%
        %%                       GAIN CONTROL SECTION                            %%
        %%=======================================================================%% 
        
        % Get the attenuator state from the WSA
        % 
        % this: refers to the wsaInterface
        %
        % errorCode: int32 data type   
        % errorMessage: string data type 
        % attenuator: double data type
        function [errorCode errorMessage attenuator] = getAttenuator(this)
			
            if strcmp(this.model, this.constants.WSA4000)
                errorCode = this.constants.WSA_ERR_INV4000COMMAND;
                attenuator = errorCode;
                errorMessage = getErrorMessage(errorCode);
                return
            end
            command = 'INPUT:ATTENUATOR?';
            [errorCode errorMessage response] = wsaQuerySCPICommand(this.cppObjectHandle, command);
            attenuator = str2double(response); 
        end 
        
        
        % Set the attenuator state in the WSA
        % 
        % this: refers to the wsaInterface
        % attenuator: double data type
        % 
        % errorCode: int32 data type  
        % errorMessage: string data type 
        function [errorCode errorMessage] = setAttenuator(this, attenuator) 
            if strcmp(this.model, this.constants.WSA4000)
                errorCode = this.constants.WSA_ERR_INV4000COMMAND;
                errorMessage = getErrorMessage(errorCode);
                return
            end

                command = sprintf('INPUT:ATTENUATOR %d', (attenuator));
                [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
        end
        
        
        % Get the IF gain value of the WSA
        % 
        % this: refers to the wsaInterface
        %
        % errorCode: int32 data type   
        % errorMessage: string data type 
        % ifGainValue: double data type
        function [errorCode errorMessage ifGainValue] = getGainIF(this)
			
            if strcmp(this.model, this.constants.WSA5000)
                errorCode = this.constants.WSA_ERR_INV5000COMMAND;
                ifGainValue = errorCode;
                errorMessage = getErrorMessage(errorCode);
                return
            end
            command = 'INPUT:GAIN:IF?';
            [errorCode errorMessage response] = wsaQuerySCPICommand(this.cppObjectHandle, command);
            ifGainValue = str2double(response); 
        end
        
        
        % Set the IF gain in the WSA
        % 
        % this: refers to the wsaInterface
        % ifGainValue: double data type
        % 
        % errorCode: int32 data type  
        % errorMessage: string data type 
        function [errorCode errorMessage] = setGainIF(this, ifGainValue) 
            if strcmp(this.model, this.constants.WSA5000)
                errorCode = this.constants.WSA_ERR_INV5000COMMAND;
                errorMessage = getErrorMessage(errorCode);
                return
            end
            
            if (ifGainValue > this.constants.WSA_MAX_IF_GAIN ||...
                ifGainValue < this.constants.WSA_MIN_IF_GAIN)
                
                errorCode = this.constants.WSA_ERR_INVIFGAIN;
                errorMessage = getErrorMessage(errorCode);
            else
                command = sprintf('INPUT:GAIN:IF %d dB', (ifGainValue));
                [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
            end

        end
        
        
        % Get the RF gain value in the WSA
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type   
        % errorMessage: string data type 
        % rfGainValue: double data type
        %   1 -  HIGH
        %   2 -  MED
        %   3 -  LOW
        %   4 -  VLOW
        function [errorCode errorMessage rfGainValue] = getGainRF(this)
            if strcmp(this.model, this.constants.WSA5000)
                errorCode = this.constants.WSA_ERR_INV5000COMMAND;
                rfGainValue = errorCode;
                errorMessage = getErrorMessage(errorCode);
                return
            end
            command = 'INPUT:GAIN:RF?';
            [errorCode errorMessage response] = wsaQuerySCPICommand(this.cppObjectHandle, command);
            rfGainSet = response;
             
            if strcmp(rfGainSet,'HIGH')
                rfGainValue = 1;
            elseif strcmp(rfGainSet,'MED')
                rfGainValue = 2;
            elseif strcmp(rfGainSet,'LOW')
                rfGainValue = 3;
            elseif strcmp(rfGainSet,'VLOW')
                rfGainValue = 4;
            else
                errorCode = this.constants.WSA_ERR_INVRFGAIN;
                errorMessage = getErrorMessage(errorCode);
                rfGainValue = this.constants.WSA_ERR_INVRFGAIN;
            end
        end
          
        
        % Set the RF gain value in the WSA
        % 
        % this: refers to the wsaInterface
        % rfGainValue: double data type
        %   1 -  HIGH
        %   2 -  MED
        %   3 -  LOW
        %   4 -  VLOW
        % 
        % errorCode: int32 data type   
        % errorMessage: string data type 
        function [errorCode errorMessage] = setGainRF(this, rfGainLevel) 
            rfGainSet = '';
            if strcmp(this.model, this.constants.WSA5000)
                errorCode = this.constants.WSA_ERR_INV5000COMMAND;
                errorMessage = getErrorMessage(errorCode);
                return
            end
            if (rfGainLevel < this.constants.WSA_MIN_RF_GAIN ||...
                rfGainLevel > this.constants.WSA_MAX_RF_GAIN)
                
                errorCode = this.constants.WSA_ERR_INVRFGAIN;
                errorMessage = getErrorMessage(errorCode);
            else
                if (rfGainLevel  == 1)
                    rfGainSet = sprintf('%sHIGH', rfGainSet);
                elseif rfGainLevel  == 2
                    rfGainSet = sprintf('%sMED', rfGainSet);
                elseif rfGainLevel  == 3
                    rfGainSet = sprintf('%sLOW', rfGainSet);
                elseif rfGainLevel  == 4
                    rfGainSet = sprintf('%sVLOW', rfGainSet);
                end
				
                command = sprintf('INPUT:GAIN:RF %s', rfGainSet);
                [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
            end
        end

        
        %%=======================================================================%%
        %%                       ANTENNA CONTROL SECTION                         %%
        %%=======================================================================%% 

        % Get the antenna port which is currently used by the RFE
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type   
        % errorMessage: string data type 
        % antennaPort: double data type
        function [errorCode errorMessage antennaPort] = getAntennaPort(this)
            if strcmp(this.model, this.constants.WSA5000)
                errorCode = this.constants.WSA_ERR_INV5000COMMAND;
                antennaPort = errorCode;
                errorMessage = getErrorMessage(errorCode);
                return
            end
            command = 'INPUT:ANTENNA?';
            [errorCode errorMessage response] = wsaQuerySCPICommand(this.cppObjectHandle, command);
            antennaPort = str2double(response);
             
            if (antennaPort  > this.constants.WSA_MAX_ANTENNA ||...
                antennaPort  < this.constants.WSA_MIN_ANTENNA)
				
                errorCode = this.constants.WSA_ERR_INVANTENNAPORT;
                antennaPort  = this.constants.WSA_ERR_INVANTENNAPORT;
                errorMessage = getErrorMessage(errorCode);
            end
        end

        
        % Set the antenna port to be used by the RFE
        % 
        % this: refers to the wsaInterface
        % antennaPort: double data type
        % 
        % errorCode: int32 data type  
        % errorMessage: string data type  
        function [errorCode errorMessage] = setAntennaPort(this, antennaPort)
            if strcmp(this.model, this.constants.WSA5000)
                errorCode = this.constants.WSA_ERR_INV5000COMMAND;
                errorMessage = getErrorMessage(errorCode);
                return
            end
			if (antennaPort  > this.constants.WSA_MAX_ANTENNA ||...
                antennaPort  < this.constants.WSA_MIN_ANTENNA)
				
                errorCode = this.constants.WSA_ERR_INVANTENNAPORT;
				errorMessage = getErrorMessage(errorCode);
			else
				command = sprintf('INPUT:ANTENNA %d', antennaPort);
				[errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
			end
        end
        
        
        %%=======================================================================%%
        %%                      DECIMATION CONTROL SECTION                       %%
        %%=======================================================================%%        
        
        
        % Get the current decimation rate of the WSA
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type  
        % errorMessage: string data type 
        % decimationRate: double data type
        % 1 - off
        % valid values: 4 - 1024
        function [errorCode errorMessage decimationRate] = getDecimation(this)
			command = 'SENSE:DEC?';
            [errorCode errorMessage response] = wsaQuerySCPICommand(this.cppObjectHandle, command);
            decimationRate = str2double(response); 
            
            if (decimationRate ~= 1 &&...
                (decimationRate  >this.maxDecimationRate ||...
                decimationRate < this.minDecimationRate))
                
                errorCode = this.constants.WSA_ERR_INVDECIMATIONRATE;
                decimationRate = this.constants.WSA_ERR_INVDECIMATIONRATE;
                errorMessage = getErrorMessage(errorCode);
            end
        end
        
        
        % Set the decimation rate to be used by the WSA
        % 
        % this: refers to the wsaInterface
        % decimationRate: double data type,
        % 1 - off
        % valid values: 4 - 1024
        %
        % errorCode: int32 data type  
        % errorMessage: string data type 
        function [errorCode errorMessage] = setDecimation(this, decimationRate) 
            if (decimationRate ~= 1 &&...
                (decimationRate  > this.maxDecimationRate ||...
                decimationRate < this.minDecimationRate))
                
                errorCode = this.constants.WSA_ERR_INVDECIMATIONRATE;
                errorMessage = getErrorMessage(errorCode);
            else
                command = sprintf('SENSE:DEC %d', decimationRate);
                [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
            end
        
        end

        
        %%=======================================================================%%
        %%                         PLL  CONTROL SECTION                          %%
        %%=======================================================================%%   
          
        
        % Get the Pll Reference source of the WSA
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        % refPll: string data type
        % ''INT'' - Internal PLL Reference source 
        % ''EXT'' - External PLL Reference source
        function [errorCode errorMessage refPll] = getRefPll(this)
            command = 'SOURCE:REFERENCE:PLL?';
            [errorCode errorMessage refPll] = wsaQuerySCPICommand(this.cppObjectHandle, command);
        end
        
        
        % Set the PLL Reference source of the WSA
        % 
        % this: refers to the wsaInterface
        % refPll: string data type
        % ''INT'' - Internal PLL Reference source 
        % ''EXT'' - External PLL Reference source
        % 
        % errorCode: int32 data type
        % errorMessage: string data type 
        function [errorCode errorMessage] = setRefPll(this, refPll )
		   command = sprintf('SOURCE:REFERENCE:PLL %s', refPll);
           [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);  
        end
        
        % Reset the Pll Reference source of the WSA
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] = resetReferencePll(this)
           command = sprintf('SOURCE:REFERENCE:PLL:RESET');
           [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
        end
        
        
        % Check if the Pll source is locked or unlocked the Pll Reference source of the WSA
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        % pllLockStatus: double data type
        % 1 - locked
        % 0 - unlocked
        function [errorCode errorMessage pllLockStatus] = getLockRefPll(this)
             command = 'LOCK:REFerence?';
             [errorCode errorMessage response] = wsaQuerySCPICommand(this.cppObjectHandle, command);
             pllLockStatus = str2double(response);
        end
 
        
        %%=======================================================================%%
        %%                      TRIGGER CONTROL SECTION                          %%
        %%=======================================================================%%  
        
        
        % Get trigger mode from the WSA
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        % triggerType: string data type
        % 'NONE' - no trigger mode is currently active in the WSA
        % 'LEVEL' - a leveled trigger mode is currently active in the WSA
        % 'PULSE' - a pulse trigger mode is currently active in the WSA
        function [errorCode errorMessage triggerType] = getTriggerType(this)
            command = 'TRIGGER:TYPE?';
            [errorCode errorMessage triggerType] = wsaQuerySCPICommand(this.cppObjectHandle, command);   
            
            if (strcmp(triggerType, this.constants.WSA_TRIGGER_MODE_LEVEL) &&...
                strcmp(triggerType, this.constants.WSA_TRIGGER_MODE_NONE) &&...
                strcmp(triggerType, this.constants.WSA_TRIGGER_MODE_PULSE))
                
                errorCode = this.constants.WSA_ERR_INVTRIGGERMODE;
                triggerType = this.constants.WSA_ERR_INVTRIGGERMODE;
                errorMessage = getErrorMessage(errorCode);
            end
              
        end
                
        
        % Set the trigger mode in the WSA
        % 
        % this: refers to the wsaInterface
        % triggerType: string data type
        % 'NONE' - no trigger mode 
        % 'LEVEL' - a leveled trigger mode
        % 'PULSE' - a pulse trigger mode
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] = setTriggerType(this, triggerType)
            if (strcmp(triggerType, this.constants.WSA_TRIGGER_MODE_LEVEL) &&...
                strcmp(triggerType, this.constants.WSA_TRIGGER_MODE_NONE) &&...
                strcmp(triggerType, this.constants.WSA_TRIGGER_MODE_PULSE))
                
                errorCode = this.constants.WSA_ERR_INVTRIGGERMODE;
                errorMessage = getErrorMessage(errorCode);
            else
				command = sprintf('TRIGGER:TYPE %s', triggerType);
			   [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
            end
            
        end
       
        
        % Get the trigger level from the WSA
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        % startFrequency: int64 data type(in Hz)
        % stopFrequency: int64 data type(in Hz)
        % amplitude: double data type (in dBm)
        function [errorCode errorMessage startFrequency,stopFrequency,amplitude] = getTriggerLevel(this)
			command = 'TRIG:LEVEL?';
			[errorCode errorMessage response] = wsaQuerySCPICommand(this.cppObjectHandle, command);
			[startFrequencyStr tempStr] = strtok(response, ',');
			[stopFrequencyStr amplitudeStr] = strtok(tempStr, ',');
			startFrequency = str2double(startFrequencyStr);
			stopFrequency = str2double(stopFrequencyStr);
			amplitude = str2double(amplitudeStr);
             
            if (startFrequency > this.maxFrequency ||...
                startFrequency < this.minFrequency||...
                stopFrequency > this.maxFrequency ||...
                stopFrequency < this.minFrequency)
                
                errorCode = this.constants.WSA_ERR_FREQOUTOFBOUND;
                startFrequency = this.constants.WSA_ERR_FREQOUTOFBOUND;
                stopFrequency = this.constants.WSA_ERR_FREQOUTOFBOUND;
                errorMessage = getErrorMessage(errorCode);
            end
        end
        
        
        % Set the trigger level in the WSA
        % 
        % this: refers to the wsaInterface
        % startFrequency: int64 data type (in Hz)
        % stopFrequency: int64 data type (in Hz)
        % amplitude: double data type (in dBm)
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] = setTriggerLevel(this, startFrequency, stopFrequency ,amplitude)
            if (startFrequency > this.maxFrequency ||...
                startFrequency < this.minFrequency ||...
                stopFrequency > this.maxFrequency ||...
                stopFrequency < this.minFrequency)
                
                errorCode = this.constants.WSA_ERR_FREQOUTOFBOUND;
                errorMessage = getErrorMessage(errorCode);
            else
                command = sprintf('TRIG:LEVEL %d, %d, %d', startFrequency, stopFrequency, amplitude);
                [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
            end
        end

        % Get trigger sync delay in the WSA
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        % syncDelay: int32 data type 
        function [errorCode errorMessage syncDelay] = getTriggerSyncDelay(this)
			
			command = 'TRIGGER:DELAY?';
			[errorCode errorMessage temp] = wsaQuerySCPICommand(this.cppObjectHandle, command);
			syncDelay = str2double(temp);
            if (syncDelay < 0)
                errorCode = this.constants.WSA_ERR_INVTRIGGERDELAY;
                syncDelay = this.constants.WSA_ERR_INVTRIGGERDELAY;
                errorMessage = getErrorMessage(errorCode);
            end
        end
        
        
        % Set the trigger sync delay in the WSA
        % 
        % this: refers to the wsaInterface
        % syncDelay: int32 data type
        % 
        % errorCode: int32 data type (must be multiple of 8)
        % errorMessage: string data type
        function [errorCode errorMessage] = setTriggerSyncDelay(this, syncDelay)
		   
		   if (syncDelay < 0)
                errorCode = this.constants.WSA_ERR_INVTRIGGERDELAY;
                errorMessage = getErrorMessage(errorCode);
            else
				command = sprintf('TRIGGER:DELAY %d', syncDelay);
				[errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
		   end
        end
        
        
        % Get trigger sync state in the WSA
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        % syncState: string data type
        function [errorCode errorMessage syncState] = getTriggerSyncState(this)
			
			command = 'TRIGGER:SYNC?';
			[errorCode errorMessage syncState] = wsaQuerySCPICommand(this.cppObjectHandle, command);
			
            if ( strcmp(syncState, this.constants.WSA_TRIGGER_SYNC_STATE_MASTER) &&...
                strcmp(syncState, this.constants.WSA_TRIGGER_SYNC_STATE_SLAVE))
                errorCode = this.constants.WSA_ERR_INVTRIGGERSYNC;
                
                syncState = this.constants.WSA_ERR_INVTRIGGERSYNC;
                errorMessage = getErrorMessage(errorCode);
            end
        end
        
        
        % Set the trigger sync state in the WSA
        % 
        % this: refers to the wsaInterface
        % syncState: string data type (SLAVE or MASTER)
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] = setTriggerSyncState(this, syncState)
		   
            if (strcmp(syncState, this.constants.WSA_TRIGGER_SYNC_STATE_MASTER) &&...
                strcmp(syncState, this.constants.WSA_TRIGGER_SYNC_STATE_SLAVE))
                
                errorCode = this.constants.WSA_ERR_INVTRIGGERSYNC;
                errorMessage = getErrorMessage(errorCode);
            else
				command = sprintf('TRIGGER:SYNC %s', syncState);
				[errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
            end
        end
        
        
        %%=======================================================================%%
        %%                      SAMPLE SIZE CONTROL SECTION                      %%
        %%=======================================================================%% 
        
        
        % Get number of samples inside an IF packet
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        % samplesPerPacket: double data type 
        function [errorCode errorMessage samplesPerPacket] = getSamplesPerPacket(this) 
			command = 'TRACE:SPPACKET?';
            [errorCode errorMessage response] = wsaQuerySCPICommand(this.cppObjectHandle, command);
            samplesPerPacket = str2double(response); 
             
            if (samplesPerPacket > this.constants.WSA_MAX_SPP ||...
                samplesPerPacket < this.constants.WSA_MIN_SPP)
                
                errorCode = this.constants.WSA_ERR_INVSAMPLESIZE;
                samplesPerPacket = this.constants.WSA_ERR_INVSAMPLESIZE;
                errorMessage = getErrorMessage(errorCode);
            end
        end
        
        
        % Set number of samples in an IF packet
        % 
        % this: refers to the wsaInterface
        % samplesPerPacket: double data type (must be a multiple of 16)
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] = setSamplesPerPacket(this, samplesPerPacket)
            if (samplesPerPacket > this.constants.WSA_MAX_SPP ||...
                samplesPerPacket < this.constants.WSA_MIN_SPP)
                
                errorCode = this.constants.WSA_ERR_INVSAMPLESIZE;
                errorMessage = getErrorMessage(errorCode);
            else
                command = sprintf('TRACE:SPPACKET %d', samplesPerPacket);
                [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
            end
        end
        
        
        % Get number of  IF packets per capture
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type  
        % errorMessage: string data type
        % packetsPerBlock: double data type
        function [errorCode errorMessage packetsPerBlock] = getPacketsPerBlock(this) 
            command = 'TRACE:BLOCK:PACKETS?';
            [errorCode errorMessage response] = wsaQuerySCPICommand(this.cppObjectHandle, command);
            packetsPerBlock = str2double(response);
            
            if (packetsPerBlock > this.constants.WSA_MAX_PPB ||...
                packetsPerBlock < this.constants.WSA_MIN_PPB)
                
                errorCode = this.constants.WSA_ERR_INVCAPTURESIZE;
                packetsPerBlock = this.constants.WSA_ERR_INVCAPTURESIZE;
                errorMessage = getErrorMessage(errorCode);
            end
        end
        
        
        % Set number of IF packets per captured block
        % 
        % this: refers to the wsaInterface
        % packetsPerBlock: double data type
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] = setPacketsPerBlock(this, packetsPerBlock)
            if (packetsPerBlock > this.constants.WSA_MAX_PPB ||...
                packetsPerBlock < this.constants.WSA_MIN_PPB)
                
                errorCode = this.constants.WSA_ERR_INVCAPTURESIZE;
                errorMessage = getErrorMessage(errorCode);
            else
               command = sprintf('TRACE:BLOCK:PACKETS %d', packetsPerBlock);
               [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
            end
        end
        
        
        %%=======================================================================%%
        %%                      DATA ACQUISITION SECTION                         %%
        %%=======================================================================%%       
        
        
        % Retrieve the WSA's current capture mode
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        % captureMode: string data type
        function [errorCode errorMessage captureMode] = getCaptureMode(this) 
           command = sprintf('SYST:CAPT:MODE?');
           [errorCode errorMessage captureMode] = wsaQuerySCPICommand(this.cppObjectHandle, command);
        end
        
        
        % Instruct the WSA to capture a block of signal data
        % and store it in internal memory. 
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] = captureBlock(this) 
           command = sprintf('TRACE:BLOCK:DATA?');
           [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
        end  
             
        
        % Determines if current user has read access from the WSA.
        % 
        % this: refers to the wsaInterface
        % readStatus: double data type
        % 0 - current user does not have read access
        % 1 - current user has read access 
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage readStatus] = getReadStatus(this)
			command = 'SYSTem:LOCK:HAVE? ACQuisition';
            [errorCode errorMessage response] = wsaQuerySCPICommand(this.cppObjectHandle, command);
            readStatus = str2double(response); 
        end
        
        
        % Attempts to obtain read access from the WSA
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        % readStatus: double data type
        % 0 - current user has not obtained read access 
        % 1 - current user has obtained read access
        function [errorCode errorMessage accessStatus] = getReadDataAccess(this)
			command = 'SYSTem:LOCK:REQuest? ACQuisition';
            [errorCode errorMessage response] = wsaQuerySCPICommand(this.cppObjectHandle, command);
            accessStatus = str2double(response); 
        end  
     
        
        % Clear the WSA's internal data buffer 
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] =  flushData(this)
            command = sprintf('SYSTEM:FLUSH');
           [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
        end
        
        
        % Remove all remaining data in the data socket
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] =  cleanDataSocket(this)
           [errorCode errorMessage ] = wsaCleanDataSocket(this.cppObjectHandle);
        end

        
        % Read a VRT packet from WSA containing I&Q data
        % 
        % this: refers to the wsaInterface
        % samplesPerPacket: double data type
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        % packetType: uint8 data type
        % 1 - IF packet
        % 4 - context packet (receiver or digitizer packet)
        % 5 - extension packet
        % receiverIndicatorField: double data type
        % receiverReferencePoint: double data type
        % receiverFrequency: int64 data type
        % recieverGainIf: int16 data type
        % recieverGainRf: int16 data type
        % recieverTemperature, double data type (not available yet)
        % digitizerIndicatorField: double data type
        % digitizerBandwidth: int64 data type
        % digitizerReferenceLevel: int16 data type
        % digitizerRfFrequencyOffset: int64 data type
        % timeStampSeconds: udouble data type UTC format and will represent the
        % number of seconds occurred since Midnight, January 1, 1970, GMT
        % timeStampPicoSeconds: uin64 data type
        % data: int16 data type, matrix size:[sampleSize 2]
        function [errorCode...
				 errorMessage... 
			     headerPacketCount...
                 headerSamplesPerPacket...
			     headerPacketType...
			     headerStreamId...  
			     headerTimeStampSeconds...
			     headerTimeStampPicoSeconds...
			     receiverIndicatorField...
			     receiverReferencePoint...
			     receiverFrequency...
			     receiverGainIf...
			     receiverGainRf...
			     receiverTemperature...
			     digitizerIndicatorField...
			     digitizerBandwidth...
                 digitizerReferenceLevel...
			     digitizerRfFrequencyOffset...
			     extensionIndicatorField...
			     extensionSweepId...
			     extensionStreamId...
			     trailerValidDataIndicator...
			     trailerRefLockIndicator...
			     trailerSpectralInversionIndicator...
			     trailerOverRangeIndicator...
                 trailerSampleLossIndicator...                 
			     data] = readVRTPacket(this, samplesPerPacket)
     
			[errorCode...
				 errorMessage... 
			     headerPacketCount...
                 headerSamplesPerPacket...
			     headerPacketType...
			     headerStreamId...  
			     headerTimeStampSeconds...
			     headerTimeStampPicoSeconds...
			     receiverIndicatorField...
			     receiverReferencePoint...
			     receiverFrequency...
			     receiverGainIf...
			     receiverGainRf...
			     receiverTemperature...
			     digitizerIndicatorField...
			     digitizerBandwidth...
                 digitizerReferenceLevel...
			     digitizerRfFrequencyOffset...
			     extensionIndicatorField...
			     extensionSweepId...
			     extensionStreamId...
			     trailerValidDataIndicator...
			     trailerRefLockIndicator...
			     trailerSpectralInversionIndicator...
			     trailerOverRangeIndicator...
                 trailerSampleLossIndicator...     
                  IQ16Data...
                  I32Data] = wsaReadVRTPacket(this.cppObjectHandle, samplesPerPacket);
            if (headerStreamId == this.constants.I16Q16_DATA_STREAM_ID)
                data = IQ16Data;
            elseif (headerStreamId == this.constants.I32_DATA_STREAM_ID)
                data = I32Data;
            elseif (headerStreamId == this.constants.I16_DATA_STREAM_ID)
                data = IQ16Data(:,1);
            else
                data = 0;
            end
        end
        
        
        %%=======================================================================%%
        %%                       STREAM  MODE CONTROL SECTION                    %%
        %%=======================================================================%%  
        
        
        % Start stream mode in the WSA
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] = streamStart(this)
            [errorCode errorMessage captureMode] = this.getCaptureMode();
            
            if (errorCode < 0)
               return
            
            elseif (strcmp(captureMode, this.constants.WSA_STREAM_CAPTURE_MODE) == 1)
                errorCode = this.constants.WSA_ERR_STREAMALREADYRUNNING;
                errorMessage = getErrorMessage(errorCode);
            
            elseif (strcmp(captureMode, this.constants.WSA_SWEEP_CAPTURE_MODE) == 1)
                errorCode = this.constants.WSA_ERR_STREAMWHILESWEEPING;
                errorMessage = getErrorMessage(errorCode);
            else
                command = sprintf('TRACE:STREAM:START');
                [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
            end
        end
        
        
        % Start stream mode in the WSA with an id
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] = streamStartID(this, streamStartID)
            [errorCode errorMessage captureMode] = this.getCaptureMode();

            if (errorCode < 0)
               return
            elseif (strcmp(captureMode, this.constants.WSA_STREAM_CAPTURE_MODE) == 1)
                
                errorCode = this.constants.WSA_ERR_STREAMALREADYRUNNING;
                errorMessage = getErrorMessage(errorCode);
            
            elseif (strcmp(captureMode, this.constants.WSA_SWEEP_CAPTURE_MODE) == 1)
                errorCode = this.constants.WSA_ERR_STREAMWHILESWEEPING;
                errorMessage = getErrorMessage(errorCode);
            else
            command = sprintf('TRACE:STREAM:START %d', streamStartID);
            [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
            end
        end
        
        
        % Stop stream mode in the WSA and clean data socket
        %
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] = streamStop(this)
            [errorCode errorMessage captureMode] = this.getCaptureMode();
            if (errorCode < 0)
               return
            
            elseif (strcmp(captureMode, this.constants.WSA_STREAM_CAPTURE_MODE) == 0)
                errorCode = this.constants.WSA_ERR_STREAMNOTRUNNING;
                errorMessage = getErrorMessage(errorCode);
            
            else
                command = sprintf('TRACE:STREAM:STOP');
                [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
            end
        end
        
        %%=======================================================================%%
        %%                       SWEEP  MODE CONTROL SECTION                     %%
        %%=======================================================================%%       

        
        % Returns the sweep status of the WSA
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        % status: string data type
        % RUNNING - sweep mode is active
        % STOPPED - sweep mode is inactive 
        function [errorCode errorMessage status] = getSweepStatus(this)
            command = 'SWEEP:LIST:STATUS?';
            [errorCode errorMessage status] = wsaQuerySCPICommand(this.cppObjectHandle, command);
                          
        end
          
        
        % Start sweep mode in the WSA
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] = sweepStart(this)
            [errorCode errorMessage captureMode] = this.getCaptureMode();
            if (errorCode < 0)
               return
            
            elseif (strcmp(captureMode, this.constants.WSA_STREAM_CAPTURE_MODE))
                errorCode = this.constants.WSA_ERR_SWEEPWHILESTREAMING;
                errorMessage = getErrorMessage(errorCode);
            
            elseif (strcmp(captureMode, this.constants.WSA_SWEEP_CAPTURE_MODE))
                errorCode = this.constants.WSA_ERR_SWEEPALREADYRUNNING;
                errorMessage = getErrorMessage(errorCode);
            
            else
                command = sprintf('SWEEP:LIST:START');
                [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
            end
           
           
        end
     
        
        % Start sweep mode in the WSA with a sweep ID
        % 
        % this: refers to the wsaInterface
        % sweepId: double data type
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] = sweepStartID(this, sweepId)
            [errorCode errorMessage captureMode] = this.getCaptureMode();
            if (errorCode < 0)
               return
            
            elseif (strcmp(captureMode, this.constants.WSA_STREAM_CAPTURE_MODE))
                errorCode = this.constants.WSA_ERR_SWEEPWHILESTREAMING;
                errorMessage = getErrorMessage(errorCode);
            
            elseif (strcmp(captureMode, this.constants.WSA_SWEEP_CAPTURE_MODE))
                errorCode = this.constants.WSA_ERR_SWEEPALREADYRUNNING;
                errorMessage = getErrorMessage(errorCode);
            
            else
               command = sprintf('SWEEP:LIST:START %d', sweepId);
               [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
            end
           
        end
        
        
        % Stop sweep mode in the WSA and clean data socket
        %
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] = sweepStop(this)
            
            [errorCode errorMessage captureMode] = this.getCaptureMode();
            if (errorCode < 0)
               return
            
            elseif (strcmp(captureMode, this.constants.WSA_SWEEP_CAPTURE_MODE) == 0)
                errorCode = this.constants.WSA_ERR_SWEEPNOTRUNNING;
                errorMessage = getErrorMessage(errorCode);
            
            else 
                command = sprintf('SWEEP:LIST:STOP');
                [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
            end
        end
     
        
        % Resume sweep mode in the WSA
        %
        % this: refers to the wsaInterface
        %
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] = sweepResume(this)
            
            [errorCode , ~, status] = this.getSweepStatus();
            
            if (errorCode < 0 || strcmp(status, this.constants.WSA_SWEEP_STATUS_RUNNING))
                errorCode = this.constants.WSA_ERR_SWEEPALREADYRUNNING;
                errorMessage = getErrorMessage(errorCode);
            else
               command = sprintf('SWEEP:LIST:RESUME');
               [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
            end
  
        end
     
        
        % Reset the current user's sweep entry in the WSA
        %
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] = sweepResetEntry(this)
           
		   command = sprintf('SWEEP:ENTRY:NEW');
           [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);  
        end

        
    %%=======================================================================%%
    %%                       SWEEP  ENTRY CONTROL SECTION                    %%
    %%=======================================================================%% 

    
        % Get the number of sweep entries in the WSA
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        % entrySize: int16 data type
        function [errorCode errorMessage entrySize] = getSweepEntrySize(this)
             
			command = 'SWEEP:ENTRY:COUNT?';
            [errorCode errorMessage response] = wsaQuerySCPICommand(this.cppObjectHandle, command);
            entrySize = str2double(response);
        end     
        
        
        % Copy a sweep entry from the WSA's sweep list to the user's sweep entry in the WSA 
        % 
        % this: refers to the wsaInterface
        % position: double data type
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] = sweepCopytEntry(this, position)
			[errorCode , ~, entrySize] = this.getSweepEntrySize();
            
            if (errorCode < 0 || position > entrySize || position < 0)
                errorCode = this.constants.WSA_ERR_SWEEPIDOOB;
                errorMessage = getErrorMessage(errorCode);
            else                
               command = sprintf('SWEEP:ENTRY:COPY %d', position);
               [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
            end
        end
        
        
        % copy a sweep entry from the WSA's sweep list to the user's sweep entry in the WSA
        % 
        % this: refers to the wsaInterface
        % position: double data type
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] = sweepSaveEntry(this, position)
			[errorCode , ~, entrySize] = this.getSweepEntrySize();
            
            if (errorCode < 0 || position > entrySize || position < 0)
                errorCode = this.constants.WSA_ERR_SWEEPIDOOB;
                errorMessage = getErrorMessage(errorCode);
            else  
               command = sprintf('SWEEP:ENTRY:SAVE %d', position);
               [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
            end
        end
        
        
        % delete a sweep entry in the WSA's sweep list
        % 
        % this: refers to the wsaInterface
        % position: double data type
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] = sweepDeletetEntry(this, position)
			[errorCode , ~, entrySize] = this.getSweepEntrySize();
            
            if (errorCode < 0 || position > entrySize || position < 0)
                errorCode = this.constants.WSA_ERR_SWEEPIDOOB;
                errorMessage = getErrorMessage(errorCode);
            else  
               command = sprintf('SWEEP:ENTRY:DELETE %d', position);
               [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
            end
        end
        
        
        % delete all the  sweep entries in the WSA's sweep list
        % 
        % this: refers to the wsaInterface
        % position: double data type
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] = sweepDeletetALL(this)
               command = sprintf('SWEEP:ENTRY:DELETE ALL');
               [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
            end
 
        %%=======================================================================%%
        %%                       SWEEP ENTRY FREQUENCY SECTION                   %%
        %%=======================================================================%% 
        
        
        % Get the start frequency and the stop frequency from the user's current sweep entry in the WSA
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        % sweepStartFrequency: int64 data type
        % sweepStopFrequency: int64 data type
        function [errorCode errorMessage startFrequency stopFrequency] = getSweepFrequency(this)
			command = 'SWEEP:ENTRY:FREQ:CENTER?';
            [errorCode errorMessage response] = wsaQuerySCPICommand(this.cppObjectHandle, command);
            [sweepStartFrequencyStr sweepStopFrequencyStr] = strtok(response, ',');  
            startFrequency = str2double(sweepStartFrequencyStr);
            stopFrequency = str2double(sweepStopFrequencyStr);
             
            if (startFrequency > this.maxFrequency ||...
                startFrequency < this.minFrequency || ...
                stopFrequency > this.maxFrequency ||...
                stopFrequency < this.minFrequency || ...
                stopFrequency < startFrequency)

				errorCode = this.constants.WSA_ERR_FREQOUTOFBOUND;
				errorMessage = getErrorMessage(errorCode);
            end
        end
        
        
        % Set the start frequency and the stop frequency in the user's current sweep entry in the WSA
        % 
        % this: refers to the wsaInterface
        % sweepStartFrequency: int64 data type
        % sweepStopFrequency: int64 data type
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] = setSweepFrequency(this, startFrequency, stopFrequency)

            if (startFrequency > this.maxFrequency ||...
                startFrequency < this.minFrequency || ...
                stopFrequency > this.maxFrequency ||...
                stopFrequency < this.minFrequency || ...
                stopFrequency < startFrequency)
				
				errorCode = this.constants.WSA_ERR_FREQOUTOFBOUND;
				errorMessage = getErrorMessage(errorCode);
            else
                command = sprintf('SWEEP:ENTRY:FREQ:CENTER %d, %d Hz', startFrequency,stopFrequency);
                [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
        
            end
        end
       
        
        % Get the frequency step in the user's current sweep entry in the WSA
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        % sweepFrequencyStep: int64 data type
        function [errorCode errorMessage FrequencyStep] = getSweepFrequencyStep(this) 
			command = 'SWEEP:ENTRY:FREQ:STEP?';
            [errorCode errorMessage response] = wsaQuerySCPICommand(this.cppObjectHandle, command);
            FrequencyStep = str2double(response); 
				
			if (FrequencyStep > this.maxFrequency ||...
                FrequencyStep < this.minFrequency)
				
            errorCode = this.constants.WSA_ERR_FREQOUTOFBOUND;
				errorMessage = getErrorMessage(errorCode);
			end
        end
        
        
        % Set the frequency in the user's current sweep entry in the WSA
        % 
        % this: refers to the wsaInterface
        % sweepFrequencyStep: int64 data type
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] = setSweepFrequencyStep(this, FrequencyStep)
            
            if (FrequencyStep > this.maxFrequency ||...
                FrequencyStep < this.minFrequency)
                
                errorCode = this.constants.WSA_ERR_FREQOUTOFBOUND;
                errorMessage = getErrorMessage(errorCode);
            else
                command = sprintf('SWEEP:ENTRY:FREQ:STEP %d Hz', (FrequencyStep));
                [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
            end
        end
             
        
        % Get the frequency shift value in the user's current sweep entry in the WSA
        % 
        % this: refers to the wsaInterface
        % sweepFrequencyShift: int64 data type
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage frequencyShift] = getSweepFrequencyShift(this)
            
			command = 'SWEEP:ENTRY:FREQ:SHIFT?';
            [errorCode errorMessage response] = wsaQuerySCPICommand(this.cppObjectHandle, command);
            frequencyShift = str2double(response);
			
			if (frequencyShift > this.constants.WSA_MAX_FSHIFT ||...
                frequencyShift < this.constants.WSA_MIN_FSHIFT)
				
                errorCode = this.constants.WSA_ERR_FREQOUTOFBOUND;
				errorMessage = getErrorMessage(errorCode);
			end
        end  
       
        
        % Set the frequency shift in the user's current sweep entry in the WSA
        % 
        % this: refers to the wsaInterface
        % sweepFrequencyShift: int64 data type
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] = setSweepFrequencyShift(this, frequencyShift) 
            
			if ((frequencyShift > this.constants.WSA_MAX_FSHIFT) ||...
                (frequencyShift < this.constants.WSA_MIN_FSHIFT))
                
                errorCode = this.constants.WSA_ERR_FREQOUTOFBOUND;
                errorMessage = getErrorMessage(errorCode);
            else
				command = sprintf('SWEEP:ENTRY:FREQ:SHIFT %d Hz', (frequencyShift));
                [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
			end
                      
        end
        
        
        %%=======================================================================%%
        %%                       SWEEP ENTRY GAIN SECTION                        %%
        %%=======================================================================%%       
          
        
        % Get the IF gain value from the user's current sweep entry in the WSA
        % 
        % this: refers to the wsaInterface
        %
        % errorCode: int32 data type   
        % errorMessage: string data type 
        % ifGainValue: double data type
        function [errorCode errorMessage ifGainValue] = getSweepGainIF(this)
            if strcmp(this.model, this.constants.WSA5000)
                errorCode = this.constants.WSA_ERR_INV5000COMMAND;
                ifGainValue = errorCode;
                errorMessage = getErrorMessage(errorCode);
                return
            end
			command = 'SWEEP:ENTRY:GAIN:IF?';
            [errorCode errorMessage response] = wsaQuerySCPICommand(this.cppObjectHandle, command);
            ifGainValue = str2double(response);
		   
			if (ifGainValue > this.constants.WSA_MAX_IF_GAIN ||...
                ifGainValue < this.constants.WSA_MIN_IF_GAIN)
                
                errorCode = this.constants.WSA_ERR_INVIFGAIN;
                ifGainValue = this.constants.WSA_ERR_INVIFGAIN;
                errorMessage = getErrorMessage(errorCode);
			end
        end
        
        
        % Set the IF gain in the user's current sweep entry in the WSA
        % 
        % this: refers to the wsaInterface
        % ifGainValue: double data type
        % 
        % errorCode: int32 data type  
        % errorMessage: string data type 
        function [errorCode errorMessage] = setSweepGainIF(this, ifGainValue) 
            if strcmp(this.model, this.constants.WSA5000)
                errorCode = this.constants.WSA_ERR_INV5000COMMAND;
                errorMessage = getErrorMessage(errorCode);
                return
            end
            if (ifGainValue > this.constants.WSA_MAX_IF_GAIN ||...
                ifGainValue < this.constants.WSA_MIN_IF_GAIN)
                
                errorCode = this.constants.WSA_ERR_INVIFGAIN;
                errorMessage = getErrorMessage(errorCode);
            else
				command = sprintf('SWEEP:ENTRY:GAIN:IF %d', (ifGainValue));
				[errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command); 
            end
        end
        
        
        % Get the RF gain value from the user's current sweep entry in the WSA
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type   
        % errorMessage: string data type 
        % rfGainValue: double data type
        %   1 -  HIGH
        %   2 -  MED
        %   3 -  LOW
        %   4 -  VLOW
        function [errorCode errorMessage rfGainValue] = getSweepGainRF(this)
            if strcmp(this.model, this.constants.WSA5000)
                errorCode = this.constants.WSA_ERR_INV5000COMMAND;
                rfGainValue = errorCode;
                errorMessage = getErrorMessage(errorCode);
                return
            end
			command = 'SWEEP:ENTRY:GAIN:RF?';
            [errorCode errorMessage response] = wsaQuerySCPICommand(this.cppObjectHandle, command);
            rfGainSet = response;
            
            if strcmp(rfGainSet,'HIGH')
                rfGainValue = 1;
            elseif strcmp(rfGainSet,'MED')
                rfGainValue = 2;
            elseif strcmp(rfGainSet,'LOW')
                rfGainValue = 3;
            elseif strcmp(rfGainSet,'VLOW')
                rfGainValue = 4;
			else
                errorCode = this.constants.WSA_ERR_INVRFGAIN;
                errorMessage = getErrorMessage(errorCode);
                rfGainValue = this.constants.WSA_ERR_INVRFGAIN;
            end
        
        end
          
        
        % Set the RF gain value in the user's current sweep entry in the WSA
        % 
        % this: refers to the wsaInterface
        % rfGainValue: double data type
        %   1 -  HIGH
        %   2 -  MED
        %   3 -  LOW
        %   4 -  VLOW
        % 
        % errorCode: int32 data type   
        % errorMessage: string data type
        function [errorCode errorMessage] = setSweepGainRF(this, rfGainLevel) 
            if strcmp(this.model, this.constants.WSA5000)
                errorCode = this.constants.WSA_ERR_INV5000COMMAND;
                errorMessage = getErrorMessage(errorCode);
                return
            end
             rfGainSet = '';
            if (rfGainLevel < this.constants.WSA_MIN_RF_GAIN || rfGainLevel > this.constants.WSA_MAX_RF_GAIN)
                errorCode = this.constants.WSA_ERR_INVRFGAIN;
                errorMessage = getErrorMessage(errorCode);
            else
                if rfGainLevel  == 1
                    rfGainSet = sprintf('%sHIGH', rfGainSet);
                
                elseif rfGainLevel  == 2
                    rfGainSet = sprintf('%sMED', rfGainSet);
                
                elseif rfGainLevel  == 3
                    rfGainSet = sprintf('%sLOW', rfGainSet);
                
                elseif rfGainLevel  == 4
                    rfGainSet = sprintf('%sVLOW', rfGainSet);
                end
               
                command = sprintf('SWEEP:ENTRY:GAIN:RF %s', rfGainSet);
                [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
            end
        end
        
        
        %%=======================================================================%%
        %%                       SWEEP ENTRY ANTENNA SECTION                     %%
        %%=======================================================================%%  
        
        
        % Get the antenna port from user's current sweep entry in the WSA
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type   
        % errorMessage: string data type 
        % antennaPort: double data type
        function [errorCode errorMessage antennaPort] = getSweepAntennaPort(this)
            if strcmp(this.model, this.constants.WSA5000)
                errorCode = this.constants.WSA_ERR_INV5000COMMAND;
                antennaPort = this.constants.WSA_ERR_INV5000COMMAND;
                errorMessage = getErrorMessage(errorCode);
                return
            end
			command = 'SWEEP:ENTRY:ANTENNA?';
            [errorCode errorMessage response] = wsaQuerySCPICommand(this.cppObjectHandle, command);
            antennaPort = str2double(response); 
            
            if (antennaPort  > this.constants.WSA_MAX_ANTENNA ||...
                antennaPort  < this.constants.WSA_MIN_ANTENNA)
                
                errorCode = this.constants.WSA_ERR_INVANTENNAPORT;
                antennaPort  = this.constants.WSA_ERR_INVANTENNAPORT;
                errorMessage = getErrorMessage(errorCode);
            end
        end
        
        
        % Set the antenna port in the user's current sweep entry in the WSA
        % 
        % this: refers to the wsaInterface
        % antennaPort: double data type
        % 
        % errorCode: int32 data type  
        % errorMessage: string data type 
        function [errorCode errorMessage] = setSweepAntennaPort(this, antennaPort) 
            if strcmp(this.model, this.constants.WSA5000)
                errorCode = this.constants.WSA_ERR_INV5000COMMAND;
                errorMessage = getErrorMessage(errorCode);
                return
            end
			if (antennaPort  > this.constants.WSA_MAX_ANTENNA ||...
                antennaPort  < this.constants.WSA_MIN_ANTENNA)
				
                errorCode = this.constants.WSA_ERR_INVANTENNAPORT;
				errorMessage = getErrorMessage(errorCode);
			else
				command = sprintf('SWEEP:ENTRY:ANTENNA %d', antennaPort);
				[errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
			end
        end

        
        %%=======================================================================%%
        %%                       SWEEP ENTRY DECIMATION SECTION                  %%
        %%=======================================================================%%  

        
        % Get the current decimation from the user's current sweep entry in the WSA
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type  
        % errorMessage: string data type 
        % decimationRate: double data type
        function [errorCode errorMessage decimationRate] = getSweepDecimation(this)
            
			command = 'SWEEP:ENTRY:DECIMATION?';
            [errorCode errorMessage response] = wsaQuerySCPICommand(this.cppObjectHandle, command);
            decimationRate = str2double(response); 
			
            if (decimationRate ~= 1 &&...
                (decimationRate  > this.maxDecimationRate||...
                decimationRate < this.minDecimationRate))
                
                errorCode = this.constants.WSA_ERR_INVDECIMATIONRATE;
                decimationRate = this.constants.WSA_ERR_INVDECIMATIONRATE;
                errorMessage = getErrorMessage(errorCode);
            end
        end
        
        
        % Set the decimation rate in user's current sweep entry in the WSA
        % 
        % this: refers to the wsaInterface
        % decimationRate: double data type
        % 
        % errorCode: int32 data type  
        % errorMessage: string data type 
        function [errorCode errorMessage] = setSweepDecimation(this, decimationRate) 
                        
			if (decimationRate ~= 1 &&...
                (decimationRate  > this.maxDecimationRate||...
                decimationRate < this.minDecimationRate))
                
                errorCode = this.constants.WSA_ERR_INVDECIMATIONRATE;
                errorMessage = getErrorMessage(errorCode);
            else
				command = sprintf('SWEEP:ENTRY:DECIMATION %d', decimationRate);
				[errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
			end
        end
        
        %%=======================================================================%%
        %%                      SWEEP ENTRY SAMPLE SIZE SECTION                  %%
        %%=======================================================================%%

        
        % Get number of samples per VRT packet from the user's current sweep entry in the WSA
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        % sweepSamplesPerPacket: double data type 
        function [errorCode errorMessage samplesPerPacket] = getSweepSamplesPerPacket(this) 
            
			command = 'SWEEP:ENTRY:SPPACKET?'; 
            [errorCode errorMessage response] = wsaQuerySCPICommand(this.cppObjectHandle, command);
            samplesPerPacket = str2double(response);  
			
            if (samplesPerPacket > this.constants.WSA_MAX_SPP ||...
                samplesPerPacket < this.constants.WSA_MIN_SPP)
                
                errorCode = this.constants.WSA_ERR_INVSAMPLESIZE;
                samplesPerPacket = this.constants.WSA_ERR_INVSAMPLESIZE;
                errorMessage = getErrorMessage(errorCode);
            end
        end
        
        
        % Set number of samples per VRT packet in the user's current sweep entry in the WSA
        % 
        % this: refers to the wsaInterface
        % sweepSamplesPerPacket: double data type
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] = setSweepSamplesPerPacket(this, samplesPerPacket) 
			
			if (samplesPerPacket > this.constants.WSA_MAX_SPP ||...
                samplesPerPacket < this.constants.WSA_MIN_SPP)
                
                errorCode = this.constants.WSA_ERR_INVSAMPLESIZE;
                errorMessage = getErrorMessage(errorCode);
			else
				command = sprintf('SWEEP:ENTRY:SPPACKET %d', samplesPerPacket);
				[errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
			end
        end
        
        
        % Get number of packets per capture from the user's current sweep entry in the WSA
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type  
        % errorMessage: string data type
        % sweepSacketsPerBlock: double data type
        function [errorCode errorMessage packetsPerBlock] = getSweepPacketsPerBlock(this) 
			command = 'SWEEP:ENTRY:PPBLOCK?';
            [errorCode errorMessage response] = wsaQuerySCPICommand(this.cppObjectHandle, command);
            packetsPerBlock = str2double(response);
            
            if (packetsPerBlock > this.constants.WSA_MAX_PPB || packetsPerBlock < this.constants.WSA_MIN_PPB)
                errorCode = this.constants.WSA_ERR_INVCAPTURESIZE;
                packetsPerBlock = this.constants.WSA_ERR_INVCAPTURESIZE;
                errorMessage = getErrorMessage(errorCode);
            end
        end
        
        
        % Set number of VRT packets per captured block in the user's current sweep entry in the WSA
        % 
        % this: refers to the wsaInterface
        % sweepPacketsPerBlock: double data type
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] = setSweepPacketsPerBlock(this, packetsPerBlock)
            
            if (packetsPerBlock > this.constants.WSA_MAX_PPB ||...
                packetsPerBlock < this.constants.WSA_MIN_PPB)
                
                errorCode = this.constants.WSA_ERR_INVCAPTURESIZE;
                errorMessage = getErrorMessage(errorCode);
			else
				command = sprintf('SWEEP:ENTRY:PPBLOCK %d', packetsPerBlock);
				[errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
            end
        end
        
        %%=======================================================================%%
        %%                      SWEEP ENTRY TRIGGER SECTION                      %%
        %%=======================================================================%%        
        
        
        % Get trigger type from the user's current sweep entry in the WSA
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        % triggerType: string data type
        % 'NONE' - no trigger mode is currently active in the WSA
        % 'LEVEL' - a leveled trigger mode is currently active in the WSA
        % 'PULSE' - the WSA will trigger whenever it receives a pulse
        function [errorCode errorMessage triggerType] = getSweepTriggerType(this)
			
			command = 'SWEEP:ENTRY:TRIGGER:TYPE?';
			[errorCode errorMessage triggerType] = wsaQuerySCPICommand(this.cppObjectHandle, command);
			
            if (strcmp(triggerType, this.constants.WSA_TRIGGER_MODE_PULSE) &&...
                strcmp(triggerType, this.constants.WSA_TRIGGER_MODE_LEVEL) &&...
                strcmp(triggerType, this.constants.WSA_TRIGGER_MODE_NONE))
                
                errorCode = this.constants.WSA_ERR_INVTRIGGERMODE;
                triggerType = this.constants.WSA_ERR_INVTRIGGERMODE;
                errorMessage = getErrorMessage(errorCode);
            end
        end
        
        
        % Set the trigger type in the user's current sweep entry in the WSA
        % 
        % this: refers to the wsaInterface
        % triggerType: string data type
        % 'NONE' - no trigger mode 
        % 'LEVEL' - a leveled trigger mode
        % 'PULSE' - the WSA will trigger whenever it receives a pulse
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] = setSweepTriggerType(this, triggerType)
		   
		   if (strcmp(triggerType, this.constants.WSA_TRIGGER_MODE_PULSE) && strcmp(triggerType, this.constants.WSA_TRIGGER_MODE_LEVEL) && strcmp(triggerType, this.constants.WSA_TRIGGER_MODE_NONE))
                errorCode = this.constants.WSA_ERR_INVTRIGGERMODE;
                errorMessage = getErrorMessage(errorCode);
            else
				command = sprintf('SWEEP:ENTRY:TRIGGER:TYPE %s', triggerType);
				[errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
		   end
        end
        
        
        % Get the trigger level from the user's current sweep entry in the WSA
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        % startFrequency: int64 data type(in Hz)
        % stopFrequency: int64 data type(in Hz)
        % amplitude: double data type (in dBm)
        function [errorCode errorMessage startFrequency stopFrequency amplitude] = getSweepTriggerLevel(this)
			command = 'SWEEP:ENTRY:TRIG:LEVEL?';
            [errorCode errorMessage response] = wsaQuerySCPICommand(this.cppObjectHandle, command);
            [startFrequencyStr tempStr] = strtok(response, ',');
            [stopFrequencyStr amplitudeStr] = strtok(tempStr, ',');
            startFrequency = str2double(startFrequencyStr);
            stopFrequency = str2double(stopFrequencyStr);
            amplitude = str2double(amplitudeStr);
            
            if (startFrequency > this.maxFrequency ||...
                startFrequency < this.minFrequency||...
                stopFrequency > this.maxFrequency ||...
                stopFrequency < this.minFrequency)
                
                errorCode = this.constants.WSA_ERR_FREQOUTOFBOUND;
                startFrequency = this.constants.WSA_ERR_FREQOUTOFBOUND;
                stopFrequency = this.constants.WSA_ERR_FREQOUTOFBOUND;
                errorMessage = getErrorMessage(errorCode);
            end
        end
       
        
        % Set the trigger levels in the user's current sweep entry in the WSA
        % 
        % this: refers to the wsaInterface
        % startFrequency: int64 data type (in Hz)
        % stopFrequency: int64 data type (in Hz)
        % amplitude: double data type (in dBm)
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] = setSweepTriggerLevel(this, startFrequency, stopFrequency ,amplitude )
            
            if (startFrequency > this.maxFrequency ||... 
                startFrequency < this.minFrequency ||... 
                stopFrequency > this.maxFrequency ||...
                stopFrequency < this.minFrequency)
                
                errorCode = this.constants.WSA_ERR_FREQOUTOFBOUND;
                errorMessage = getErrorMessage(errorCode);
            else
				command = sprintf('SWEEP:ENTRY:TRIG:LEVEL %d, %d, %d', startFrequency, stopFrequency, amplitude);
				[errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
            end
        end
        
        
        % Get trigger sync delay from the user's current sweep entry in the WSA
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        % syncDelay: int32 data type (must be multiple of 8)
        function [errorCode errorMessage syncDelay] = getSweepTriggerSyncDelay(this)
			
			command = 'SWEEP:LIST:TRIGGER:DELAY?';
			[errorCode errorMessage temp] = wsaQuerySCPICommand(this.cppObjectHandle, command);
			syncDelay = str2double(temp);
            if (syncDelay < 0)
                errorCode = this.constants.WSA_ERR_INVTRIGGERDELAY;
                syncDelay = this.constants.WSA_ERR_INVTRIGGERDELAY;
                errorMessage = getErrorMessage(errorCode);
            end
        end
        
        % Set the trigger sync delay in the user's current sweep entry in the WSA
        % 
        % this: refers to the wsaInterface
        % syncDelay: int32 data type
        % 
        % errorCode: int32 data type (must be multiple of 8)
        % errorMessage: string data type
        function [errorCode errorMessage] = setSweepTriggerSyncDelay(this, syncDelay)
		   
		   if (syncDelay < 0)
                errorCode = this.constants.WSA_ERR_INVTRIGGERDELAY;
                errorMessage = getErrorMessage(errorCode);
            else
				command = sprintf('SWEEP:LIST:TRIGGER:DELAY %d', syncDelay);
				[errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
		   end
        end
        
        
        % Get trigger sync state from the user's current sweep entry in the WSA
        % 
        % this: refers to the wsaInterface
        % 
        % errorCode: int32 data type 
        % errorMessage: string data type
        % syncState: string data type
        function [errorCode errorMessage syncState] = getSweepSyncState(this)
			
			command = 'SWEEP:LIST:TRIGGER:SYNC?';
			[errorCode errorMessage syncState] = wsaQuerySCPICommand(this.cppObjectHandle, command);
			
            if ( strcmp(syncState, this.constants.WSA_TRIGGER_SYNC_STATE_MASTER) &&...
                strcmp(syncState, this.constants.WSA_TRIGGER_SYNC_STATE_SLAVE))
                errorCode = this.constants.WSA_ERR_INVTRIGGERSYNC;
                
                syncState = this.constants.WSA_ERR_INVTRIGGERSYNC;
                errorMessage = getErrorMessage(errorCode);
            end
        end
        
        
        % Set the trigger sync state in the user's current sweep entry in the WSA
        % 
        % this: refers to the wsaInterface
        % syncState: string data type (SLAVE or MASTER)
        % 
        % errorCode: int32 data type
        % errorMessage: string data type
        function [errorCode errorMessage] = setSweepTriggerSyncState(this, syncState)
		   
            if (strcmp(syncState, this.constants.WSA_TRIGGER_SYNC_STATE_MASTER) &&...
                strcmp(syncState, this.constants.WSA_TRIGGER_SYNC_STATE_SLAVE))
                
            errorCode = this.constants.WSA_ERR_INVTRIGGERSYNC;
            errorMessage = getErrorMessage(errorCode);
            else
				command = sprintf('SWEEP:LIST:TRIGGER:SYNC %s', syncState);
				[errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
            end
        end
        
        
        %%=======================================================================%%
        %%                      SWEEP ENTRY DWELL SECTION                        %%
        %%=======================================================================%%

        % Get the sweep Dwell value in in the user's current sweep entry in the WSA
        function [errorCode errorMessage dwellSecs dwellMicroSecs] = getSweepDwell(this)
            
            command = 'SWEEP:ENTRY:DWELL?';
            [errorCode errorMessage response] = wsaQuerySCPICommand(this.cppObjectHandle, command);
            [dwellSecsStr dwellMicroSecsStr] = strtok(response);
            dwellSecs = val(dwellSecsStr);
            dwellMicroSecs = val(dwellMicroSecsStr);
        end
        
        % Set the sweep Dwell value in in the user's current sweep entry in the WSA
        function [errorCode errorMessage] = setSweepDwell(this, dwellSecs,  dwellMicroSecs)
            command =  sprintf('SWEEP:ENTRY:DWELL %d %d',dwellSecs,  dwellMicroSecs);
            [errorCode errorMessage ] = wsaSendSCPICommand(this.cppObjectHandle, command);
        end
        
        %%=======================================================================%%
        %%                      SCPI COMMAND SECTION                             %%
        %%=======================================================================%%   

        % Send a SCPI command to the wsa and query for a response
        function [errorCode errorMessage response] = querySCPICommand(this, command)
            
            [errorCode errorMessage response] = wsaQuerySCPICommand(this.cppObjectHandle, command);
        end
        
        % Send a SCPI command to the wsa
        function [errorCode errorMessage] = sendSCPICommand(this, command)
            
            [errorCode errorMessage] = wsaSendSCPICommand(this.cppObjectHandle, command);
        end

    end
end

% Retrieve an error message based on an error code
function [errorMessage] = getErrorMessage(errorCode)

    [errorMessage] = wsaGetErrorMessage(int16(errorCode));
end




















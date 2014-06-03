
% This function will read and parse VRT data
function [header digitizer receiver extension data trailer] = readWSAVRT(wsaHandle, sampleSize)
    
    % add VRT packet classes to path
    [mFilePath, ~, ~] = fileparts(mfilename('fullpath'));
    addpath(fullfile(mFilePath, '/', 'vrt'));

    % initialize VRT packet classes
    header = vrtHeader;
    receiver = vrtReceiver;
    digitizer = vrtDigitizer;
    extension = vrtExtension;
    trailer = vrtTrailer;
    headerPacketType = 0;
    
    
    % read VRT packets until IF data packet is received
    while headerPacketType ~= wsaHandle.constants.IF_PACKET_TYPE
        %% read VRT packet from the wsa
        [readResult...
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
        data] = wsaHandle.readVRTPacket(int32(sampleSize));
    
        header.packetCount = headerPacketCount;
        header.samplesPerPacket = headerSamplesPerPacket;
        header.packetType = headerPacketType;
        header.streamId = headerStreamId;
        header.timeStampSeconds = headerTimeStampSeconds;
        header.timeStampPicoSeconds = headerTimeStampPicoSeconds;
        
        % parse context packets
        if (header.packetType == wsaHandle.constants.CONTEXT_PACKET_TYPE)
            
            % parse digitizer packet
            if (header.streamId == wsaHandle.constants.DIGITIZER_STREAM_ID)

                if (digitizerIndicatorField == wsaHandle.constants.BANDWIDTH_INDICATOR_MASK)
                    digitizer.bandwidth = digitizerBandwidth;

                elseif (digitizerIndicatorField == wsaHandle.constants.REFERENCE_LEVEL_INDICATOR_MASK)
                    digitizer.referenceLevel = digitizerReferenceLevel;

                elseif (digitizerIndicatorField == wsaHandle.constants.RF_OFFSET_INDICATOR_MASK)
                    digitizer.rfFreqOffset = digitizerRfFrequencyOffset;

                end
        % parse receiver packet
        elseif (header.streamId == wsaHandle.constants.RECEIVER_STREAM_ID)
             receiver.referencePoint = receiverReferencePoint;
             receiver.frequency = receiverFrequency;
             receiver.gainIF = receiverGainIf;
             receiver.gainRF = receiverGainRf;
             receiver.temperature = receiverTemperature;

        end
        
        % parse extension packet
        elseif (header.packetType == wsaHandle.constants.EXTENSION_PACKET_TYPE)
             extension.sweepStartId = extensionSweepId;
             extension.streamStartId = extensionStreamId;
    
        % parse trailer of if data packet
        elseif (header.packetType == wsaHandle.constants.IF_PACKET_TYPE)
            trailer.validDataIndicator =  trailerValidDataIndicator;
            trailer.refLockIndicator = trailerRefLockIndicator;
            trailer.spectralInversionIndicator = trailerSpectralInversionIndicator;
            trailer.overRangeIndicator = trailerOverRangeIndicator;
            trailer.sampleLossIndicator = trailerSampleLossIndicator;
        end
        
    end
end


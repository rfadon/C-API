
classdef vrtHeader
    properties
        pkt_count;
        samples_per_packet;
        packet_type;
        stream_id;
        timeStampSeconds;
        timeStampPicoSeconds
    end
end
  
classdef vrtReceiver
    properties
        indicator_field;
        pkt_count;
        reference_point;
        freq;
        gain_if;
        gain_rf;
        temperature;
    end
end


//structure to hold digitizer packet data
struct wsa_digitizer_packet {
	int32_t indicator_field;
	uint8_t pkt_count;
	long double bandwidth;
	int16_t reference_level;
	long double rf_freq_offset;
};

//structure to hold extension packet data
struct wsa_extension_packet {
	int32_t indicator_field;
	uint8_t pkt_count;
	uint32_t sweep_start_id;
	uint32_t stream_start_id;
};

// These values will be defined in a future release
struct wsa_vrt_packet_trailer {
	uint8_t valid_data_indicator;
	uint8_t ref_lock_indicator;
	uint8_t spectral_inversion_indicator;
	uint8_t over_range_indicator;
	uint8_t sample_loss_indicator;
};
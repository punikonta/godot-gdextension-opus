#include "opus_node.h"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/variant/utility_functions.hpp"

using namespace godot;

void OpusNode::_bind_methods() {
    ClassDB::bind_method(D_METHOD("encode", "pcm_data"), &OpusNode::encode);
    ClassDB::bind_method(D_METHOD("decode", "opus_packet"), &OpusNode::decode);

    ClassDB::bind_method(D_METHOD("set_bitrate", "bitrate"), &OpusNode::set_bitrate);
    ClassDB::bind_method(D_METHOD("get_bitrate"), &OpusNode::get_bitrate);
    
    ClassDB::bind_method(D_METHOD("set_complexity", "complexity"), &OpusNode::set_complexity);
    ClassDB::bind_method(D_METHOD("get_complexity"), &OpusNode::get_complexity);

    ClassDB::bind_method(D_METHOD("set_sample_rate", "sample_rate"), &OpusNode::set_sample_rate);
    ClassDB::bind_method(D_METHOD("get_sample_rate"), &OpusNode::get_sample_rate);
    
    ClassDB::bind_method(D_METHOD("get_frame_size"), &OpusNode::get_frame_size);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "bitrate", PROPERTY_HINT_RANGE, "6000,128000,100"), "set_bitrate", "get_bitrate");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "complexity", PROPERTY_HINT_RANGE, "0,10,1"), "set_complexity", "get_complexity");
    
    ADD_PROPERTY(PropertyInfo(Variant::INT, "sample_rate", PROPERTY_HINT_ENUM, "8000,12000,16000,24000,48000"), "set_sample_rate", "get_sample_rate");
}

OpusNode::OpusNode() {
    encoder = nullptr;
    decoder = nullptr;
    max_payload_bytes = 4000;
    
    bitrate = 24000;
    complexity = 10;
    sample_rate = 48000;

    _update_opus_state();
}

OpusNode::~OpusNode() {
    if (encoder) opus_encoder_destroy(encoder);
    if (decoder) opus_decoder_destroy(decoder);
}

void OpusNode::_update_opus_state() {
    if (encoder) opus_encoder_destroy(encoder);
    if (decoder) opus_decoder_destroy(decoder);

    int err = OPUS_OK;
    encoder = opus_encoder_create(sample_rate, 1, OPUS_APPLICATION_VOIP, &err);
    if (err != OPUS_OK) {
        UtilityFunctions::printerr("Opus encoder initialization failed: ", err);
        return;
    }

    decoder = opus_decoder_create(sample_rate, 1, &err);
    if (err != OPUS_OK) {
        UtilityFunctions::printerr("Opus decoder initialization failed: ", err);
        return;
    }

    opus_encoder_ctl(encoder, OPUS_SET_BITRATE(bitrate));
    opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(complexity));
    opus_encoder_ctl(encoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE)); 
}

void OpusNode::set_bitrate(int bitrate) {
    bitrate = bitrate;
    if (encoder) {
        opus_encoder_ctl(encoder, OPUS_SET_BITRATE(bitrate));
    }
}

int OpusNode::get_bitrate() const { 
    return bitrate; 
}

void OpusNode::set_complexity(int complexity) {
    complexity = complexity;
    if (encoder) {
        opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(complexity));
    }
}

int OpusNode::get_complexity() const { 
    return complexity; 
}

void OpusNode::set_sample_rate(int sample_rate) {
    int actual_rate = sample_rate;
    if (sample_rate == 0) actual_rate = 8000;
    else if (sample_rate == 1) actual_rate = 12000;
    else if (sample_rate == 2) actual_rate = 16000;
    else if (sample_rate == 3) actual_rate = 24000;
    else if (sample_rate == 4) actual_rate = 48000;

    if (sample_rate != actual_rate) {
        sample_rate = actual_rate;
        _update_opus_state();
    }
}

int OpusNode::get_sample_rate() const { 
    switch(sample_rate) {
        case 8000: return 0;
        case 12000: return 1;
        case 16000: return 2;
        case 24000: return 3;
        case 48000: return 4;
        default: return 4;
    }
}

int OpusNode::get_frame_size() const {
    return (sample_rate / 1000) * 20;
}

PackedByteArray OpusNode::encode(const PackedVector2Array &pcm_data) {
    PackedByteArray output;
    if (!encoder || pcm_data.is_empty()) return output;

    int frame_size = pcm_data.size(); 
    PackedFloat32Array mono_pcm;
    mono_pcm.resize(frame_size);
    for (int i = 0; i < frame_size; i++) {
        mono_pcm.set(i, (pcm_data[i].x + pcm_data[i].y) * 0.5f);
    }

    output.resize(max_payload_bytes);
    int bytes = opus_encode_float(encoder, mono_pcm.ptr(), frame_size, output.ptrw(), max_payload_bytes);
    
    if (bytes < 0) {
        UtilityFunctions::printerr("Opus encoding failed: ", bytes);
        return PackedByteArray();
    }
    
    output.resize(bytes);
    return output;
}

PackedVector2Array OpusNode::decode(const PackedByteArray &opus_packet) {
    PackedVector2Array output;
    
    if (!decoder) return output; 

    int max_frames = (sample_rate / 1000) * 120; 
    int plc_frames = (sample_rate / 1000) * 20; 

    PackedFloat32Array mono_pcm;
    mono_pcm.resize(max_frames);
    
    int decoded_frames = 0;

    if (opus_packet.is_empty()) {
        // filler audio behavior
        decoded_frames = opus_decode_float(decoder, NULL, 0, mono_pcm.ptrw(), plc_frames, 0);
    } else {
        // normal decoding
        decoded_frames = opus_decode_float(decoder, opus_packet.ptr(), opus_packet.size(), mono_pcm.ptrw(), max_frames, 0);
    }
    
    if (decoded_frames < 0) {
        UtilityFunctions::printerr("Opus decoding failed: ", decoded_frames);
        return PackedVector2Array();
    }
    
    output.resize(decoded_frames);
    for (int i = 0; i < decoded_frames; i++) {
        float sample = mono_pcm[i];
        output.set(i, Vector2(sample, sample));
    }
    
    return output;
}
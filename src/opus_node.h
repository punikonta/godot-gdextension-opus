#ifndef OPUS_NODE_H
#define OPUS_NODE_H

#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/variant/packed_byte_array.hpp"
#include "godot_cpp/variant/packed_vector2_array.hpp"
#include "godot_cpp/variant/packed_float32_array.hpp"
#include "opus.h"

namespace godot {

class OpusNode : public Node {
    GDCLASS(OpusNode, Node)

private:
    OpusEncoder *encoder;
    OpusDecoder *decoder;
    int max_payload_bytes;
    
    int bitrate;
    int complexity;
    int sample_rate;

    void _update_opus_state();

protected:
    static void _bind_methods();

public:
    OpusNode();
    ~OpusNode();

    void set_bitrate(int p_bitrate);
    int get_bitrate() const;

    void set_complexity(int p_complexity);
    int get_complexity() const;

    void set_sample_rate(int p_sample_rate);
    int get_sample_rate() const;

    int get_frame_size() const; 

    PackedByteArray encode(const PackedVector2Array &pcm_data);
    PackedVector2Array decode(const PackedByteArray &opus_packet);
};

}
#endif
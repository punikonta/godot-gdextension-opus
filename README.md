# Opus (libopus) for Godot

This GDextension adds a node `OpusNode` that provides `enncode` and `decode` funcions for the [Opus Audio Codec](https://opus-codec.org/) for Godot.

Primary use-case is real-time VoIP communication.

## Work in progress

This repo is currently work in progress, but it mostly already works as is.

## Building

The `SConstruct` has been extended to build `opus-1.6.1` (sources included within this repo) along with the extension itself, so no extra steps are needed to build this extension, other than calling `scons` itself.

I haven't confirmed successful builds on any platforms other than Windows and Linux on x86_64 yet.

TODOs:

* test builds on other platforms/architectures
* provide example project and documentation
* readme extension

## License

The template uses the original public domain license provided by the `godot-cpp-template`. The extention itself (`opus_node.h` and `opus_node.cpp`) is licensed under the MIT License and located within the src directory of this repository.

The [Opus Audio Codec](https://opus-codec.org/) is licensed under the [three clause BSD license](https://opus-codec.org/license/).

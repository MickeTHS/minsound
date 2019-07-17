/**

Michael Nilsson 2019

Usage: 
// initiate the sounds device (will select the system default)
SiSound::Sound_device dev;
dev.init();

// Standard WAV PCM 16 MONO 441000 must be preloaded and stored as .sisound file
//SiSound::Sound_device::read_from_wav_to_buffer("D:\\projects\\si\\openal-soft\\data\\boeing_747_take_off_mono_16pcm.wav", data);
//SiSound::Sound_device::write_buffer_to_file(data, "D:\\projects\\si\\openal-soft\\data\\boeing_747_raw.sisound");
// or store it in an archive

// When the sisound format file exists either in a buffer or file, it can be loaded using any of these methods:
// auto audio = dev.create_audio_from_file("D:\\projects\\si\\openal-soft\\data\\boeing_747_raw.sisound", 0);
// auto audio = dev.create_audio_from_buffer(&data[0], data.size(), 0);
// auto audio = dev.create_audio_from_buffer(data, 0);

// When the sampled audio has been loaded, a Sound_audio object has been allocted and the render buffer has been set up in OpenAL, next you need to create an audio instance:
auto instance = audio->create_instance();

// You should assign a position of the sound:
instance->set_pos(0, 0, 0); // 100% left and 100% right audio
instance->set_pos(-10, 0, 0); // sound is on the left
instance->set_pos(10, 0, 0); // sound is on the right

// Play the audio
instance->play();

// Check if the sample is still playing:
if (instance->is_playing());

// Stop playback
instance->stop();

// Deleting the objects must be done in the order of: instances, audios, device


*/

#pragma once

#include <memory>
#include <string>
#include <vector>

namespace Minsound {

    struct ALCdevice;
    struct ALCcontext;

    struct Sound_instance {
        Sound_instance(uint32_t al_source);
        virtual ~Sound_instance();

        void play(bool looping);
        void stop();
        bool is_playing();
        void set_offset_seconds(float seconds);
        void set_pos(float x, float y, float z);
        void set_volume(float vol);

        uint32_t _al_source;
        float _pos[3];
        float _volume;

    };

    struct Sound_audio {
        Sound_audio(uint32_t al_sampleset, uint32_t id);
        virtual ~Sound_audio();

        std::shared_ptr<Sound_instance> create_instance();

        uint32_t _id;

        uint32_t _al_sampleset;
    };


    class Sound_audio_invalid_format: public std::exception
    {
    virtual const char* what() const throw()
    {
        return "Sound must be one channel (mono), 44100H, PCM16";
    }
    };

    
    class Sound_audio_file_not_found: public std::exception
    {
    virtual const char* what() const throw()
    {
        return "File not found";
    }
    };


    struct Sound_device {
        bool init();

        Sound_device();
        virtual ~Sound_device();

        static bool read_from_wav_to_buffer(const std::string& filepath, std::vector<uint8_t>& data);
        static bool write_buffer_to_file(const std::vector<uint8_t>& buffer, const std::string& filepath);

        std::shared_ptr<Sound_audio> create_audio_from_file(const std::string& filepath, const int id);
        std::shared_ptr<Sound_audio> create_audio_from_buffer(const std::vector<uint8_t>& buffer, const int id);
        std::shared_ptr<Sound_audio> create_audio_from_buffer(const uint8_t* buffer, size_t len, const int id);

    private:
        bool bind_buffer(const char** buffer, size_t size, uint32_t& al_source);

        ALCdevice *_device;
        ALCcontext *_context;

        std::vector<Sound_audio> _sounds;
    };


};
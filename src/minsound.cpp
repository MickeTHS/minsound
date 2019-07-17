#include "minsound/minsound.h"

namespace Minsound {

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "AL/alc.h"
#include "AL/al.h"
#include "AL/alext.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#ifndef ALC_ENUMERATE_ALL_EXT
#define ALC_DEFAULT_ALL_DEVICES_SPECIFIER        0x1012
#define ALC_ALL_DEVICES_SPECIFIER                0x1013
#endif

#ifndef ALC_EXT_EFX
#define ALC_EFX_MAJOR_VERSION                    0x20001
#define ALC_EFX_MINOR_VERSION                    0x20002
#define ALC_MAX_AUXILIARY_SENDS                  0x20003
#endif


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

static void printDeviceList(const char *list)
{
    if(!list || *list == '\0')
        printf("    !!! none !!!\n");
    else do {
        printf("    %s\n", list);
        list += strlen(list) + 1;
    } while(*list != '\0');
}


Sound_device::Sound_device() {
    _device = 0;
    _context = 0;
}

Sound_device::~Sound_device() {
    alcMakeContextCurrent(NULL);

    if (_context != 0) {
        alcDestroyContext(_context);
    }

    if (_device != 0) {
        alcCloseDevice(_device);
    }
}

bool Sound_device::init() {
    
    printf("Available playback devices:\n");
    if(alcIsExtensionPresent(NULL, "ALC_ENUMERATE_ALL_EXT") != AL_FALSE)
        printDeviceList(alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER));
    else
        printDeviceList(alcGetString(NULL, ALC_DEVICE_SPECIFIER));
    printf("Available capture devices:\n");
    printDeviceList(alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER));

    if(alcIsExtensionPresent(NULL, "ALC_ENUMERATE_ALL_EXT") != AL_FALSE)
        printf("Default playback device: %s\n",
               alcGetString(NULL, ALC_DEFAULT_ALL_DEVICES_SPECIFIER));
    else
        printf("Default playback device: %s\n",
               alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER));
    printf("Default capture device: %s\n",
           alcGetString(NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER));

    //printALCInfo(NULL);

    _device = alcOpenDevice(NULL);
    if(!_device)
    {
        printf("\n!!! Failed to open default device!!!\n\n");
        return false;
    }
    //printALCInfo(device);
    //printHRTFInfo(device);

    _context = alcCreateContext(_device, NULL);
    if(!_context || alcMakeContextCurrent(_context) == ALC_FALSE)
    {
        if (_context) {
            alcDestroyContext(_context);
        }

        alcCloseDevice(_device);
        printf("\n!!! Failed to set a context !!!\n\n");
        return false;
    }

    return true;
}

bool Sound_device::bind_buffer(const char** buffer, size_t size, uint32_t& al_source) {
    uint32_t alSource;
    uint32_t alSampleSet;

    //create a source
    alGenSources(1, &alSource);

    //create buffer
    alGenBuffers(1, &alSampleSet);

    //put the data into our sampleset buffer
    alBufferData(alSampleSet, AL_FORMAT_MONO16, buffer, size, 44100);

    //assign the buffer to this source
    alSourcei(alSource, AL_BUFFER, alSampleSet);

    al_source = alSource;

    return true;
}

bool Sound_device::write_buffer_to_file(const std::vector<uint8_t>& buffer, const std::string& filepath) {
    FILE* fp = fopen(filepath.c_str(), "wb");

    uint32_t sample_size = buffer.size();

    fwrite(&sample_size, sizeof(uint32_t), 1, fp);
    fwrite(&buffer[0], buffer.size(), 1, fp);

    fclose(fp);

    return true;
}

bool Sound_device::read_from_wav_to_buffer(const std::string& filepath, std::vector<uint8_t>& data) {
    
    drwav* pWav = drwav_open_file(filepath.c_str());

    if (pWav == NULL) {
        throw Sound_audio_file_not_found();
    }

    if (pWav->channels != 1) {
        throw Sound_audio_invalid_format();
    }

    int32_t sample_size = pWav->totalPCMFrameCount * pWav->channels * sizeof(int16_t);

    drwav_int16* pSampleData = (drwav_int16*)malloc((size_t)pWav->totalPCMFrameCount * pWav->channels * sizeof(int16_t));
    drwav_read_s16(pWav, pWav->totalPCMFrameCount, pSampleData);

    data.resize(sample_size);
    memcpy(&data[0], &pSampleData[0], sample_size);

    free(pSampleData);

    return true;
}

std::shared_ptr<Sound_audio> Sound_device::create_audio_from_file(const std::string& filepath, const int id) {
    FILE* fp = fopen(filepath.c_str(), "rb");

    if (!fp) {
        throw Sound_audio_file_not_found();
    }

    uint32_t sample_size;

    std::vector<uint8_t> data;
    fread(&sample_size, sizeof(uint32_t), 1, fp);
    data.resize(sample_size);
    fread(&data[0], sample_size, 1, fp);

    auto audio = create_audio_from_buffer(&data[0], data.size(), id);

    fclose(fp);

    return audio;
}

std::shared_ptr<Sound_audio> Sound_device::create_audio_from_buffer(const std::vector<uint8_t>& buffer, const int id) {
    return create_audio_from_buffer(&buffer[0], buffer.size(), id);
}

std::shared_ptr<Sound_audio> Sound_device::create_audio_from_buffer(const uint8_t* buffer, size_t len, const int id) {

    uint32_t al_sampleset;
    
    alGenBuffers(1, &al_sampleset);
    alBufferData(al_sampleset, AL_FORMAT_MONO16, &buffer[0], len, 44100);
    
    return std::make_shared<Sound_audio>(al_sampleset, id);
}

Sound_audio::Sound_audio(uint32_t al_sampleset, uint32_t id) {
    _al_sampleset = al_sampleset;
    _id = id;
}

Sound_audio::~Sound_audio() {
    alDeleteBuffers(1, &_al_sampleset);
}

std::shared_ptr<Sound_instance> Sound_audio::create_instance() {
    uint32_t al_source;
    alGenSources(1, &al_source);
    alSourcei(al_source, AL_BUFFER, _al_sampleset);
    
    return std::make_shared<Sound_instance>(al_source);
}

Sound_instance::Sound_instance(uint32_t al_source) {
    _al_source = al_source;
    _pos[0] = 0;
    _pos[1] = 0;
    _pos[2] = 0;
}

Sound_instance::~Sound_instance() {
    alDeleteSources(1, &_al_source);
}

void Sound_instance::play(bool looping) {
    alSourcei(_al_source, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);

    alSourcePlay(_al_source);
}

void Sound_instance::stop() {
    alSourceStop(_al_source);
}

bool Sound_instance::is_playing() {
    int source_state = 0;

    alGetSourcei(_al_source, AL_SOURCE_STATE, &source_state);

    return source_state == AL_PLAYING;
}

void Sound_instance::set_pos(float x, float y, float z) {
    _pos[0] = x;
    _pos[1] = x;
    _pos[2] = x;

    alSource3f(_al_source, AL_POSITION, _pos[0], _pos[1], _pos[2]);
}

void Sound_instance::set_volume(float vol) {
    _volume = vol;
    alSourcef(_al_source, AL_GAIN, vol);
}

void Sound_instance::set_offset_seconds(float seconds) {
    alSourcef(_al_source, AL_SEC_OFFSET, seconds);
}

}
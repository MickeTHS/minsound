#include <minsound/minsound.h>

#include <vector>
#include <chrono>

int main(char** argc, int argv) {
    Minsound::Sound_device dev;
    dev.init();

    std::vector<uint8_t> data;

    //Minsound::Sound_device::read_from_wav_to_buffer("D:\\projects\\si\\openal-soft\\data\\boeing_747_take_off_mono_16pcm.wav", data);
    //Minsound::Sound_device::write_buffer_to_file(data, "D:\\projects\\si\\openal-soft\\data\\boeing_747_raw.sisound");
    //Minsound::Sound_device::read_from_wav_to_buffer("D:\\projects\\si\\openal-soft\\data\\boeing_747_take_off_mono_16pcm_lowpass.wav", data);
    //Minsound::Sound_device::write_buffer_to_file(data, "D:\\projects\\si\\openal-soft\\data\\boeing_747_lowpass_raw.sisound");


    auto audio_outside = dev.create_audio_from_file("D:\\projects\\si\\openal-soft\\data\\boeing_747_raw.sisound", 0);
    auto audio_inside = dev.create_audio_from_file("D:\\projects\\si\\openal-soft\\data\\boeing_747_lowpass_raw.sisound", 1);

    auto instance_outside = audio_outside->create_instance();
    auto instance_inside = audio_inside->create_instance();

    //instance_outside->play(false);
    instance_inside->play(false);

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    while(instance_outside->is_playing() || instance_inside->is_playing()) {
        end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;

        if (elapsed_seconds.count() > 4.0 && instance_inside->is_playing()) {
            instance_inside->stop();
            instance_outside->set_offset_seconds(elapsed_seconds.count());
            instance_outside->play(false);
        }
    }
    
    return 0;
}
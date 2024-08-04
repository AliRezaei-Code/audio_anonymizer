#include <iostream>
#include <cstdlib>
#include <vector>
#include <portaudio.h>
#include <sndfile.h>
#include <soundtouch/SoundTouch.h>
#include <cstring>  // Include this header for memcpy

using namespace std;
using namespace soundtouch;

// Constants for audio processing
const int SAMPLE_RATE = 44100;
const int FRAMES_PER_BUFFER = 512;

// Function to read audio file
float* readAudioFile(const string &fileName, SF_INFO &sfinfo, size_t &bufferSize) {
    SNDFILE *file = sf_open(fileName.c_str(), SFM_READ, &sfinfo);
    if (!file) {
        cerr << "Error opening file: " << fileName << endl;
        exit(1);
    }

    bufferSize = sfinfo.frames * sfinfo.channels;
    float *buffer = (float*)malloc(bufferSize * sizeof(float));
    if (!buffer) {
        cerr << "Memory allocation failed for audio buffer" << endl;
        sf_close(file);
        exit(1);
    }

    sf_readf_float(file, buffer, sfinfo.frames);
    sf_close(file);

    return buffer;
}

// Function to write audio file
void writeAudioFile(const string &fileName, float *buffer, size_t bufferSize, SF_INFO &sfinfo) {
    SNDFILE *file = sf_open(fileName.c_str(), SFM_WRITE, &sfinfo);
    if (!file) {
        cerr << "Error opening file for writing: " << fileName << endl;
        free(buffer);
        exit(1);
    }

    sf_writef_float(file, buffer, bufferSize / sfinfo.channels);
    sf_close(file);
}

// Function to process audio (pitch shifting and time stretching)
void processAudio(float *buffer, size_t &bufferSize, int channels, float pitchShift, float tempoChange) {
    SoundTouch soundTouch;
    soundTouch.setSampleRate(SAMPLE_RATE);
    soundTouch.setChannels(channels);
    soundTouch.setPitchSemiTones(pitchShift);
    soundTouch.setTempoChange(tempoChange);

    soundTouch.putSamples(buffer, bufferSize / channels);
    
    // Allocate buffer for processed samples (use vector for dynamic resizing)
    vector<float> processedBuffer(bufferSize * 2);
    int numSamples = soundTouch.receiveSamples(processedBuffer.data(), processedBuffer.size() / channels);

    // Resize original buffer to accommodate processed samples
    bufferSize = numSamples * channels;
    buffer = (float*)realloc(buffer, bufferSize * sizeof(float));
    if (!buffer) {
        cerr << "Memory reallocation failed for audio buffer" << endl;
        exit(1);
    }

    // Copy processed samples back to original buffer
    memcpy(buffer, processedBuffer.data(), bufferSize * sizeof(float));
}

int main() {
    // Input and output file names
    string inputFileName = "input.wav";
    string outputFileName = "output_anonymized.wav";

    // Audio file info
    SF_INFO sfinfo;
    size_t bufferSize;
    float *audioBuffer = readAudioFile(inputFileName, sfinfo, bufferSize);

    // Parameters for audio processing
    float pitchShift = 5.0f;  // Shift pitch by 5 semitones
    float tempoChange = 10.0f;  // Increase tempo by 10%

    // Process audio (pitch shifting and time stretching)
    processAudio(audioBuffer, bufferSize, sfinfo.channels, pitchShift, tempoChange);

    // Add noise to audio (simple example)
    float noiseLevel = 0.02f;
    for (size_t i = 0; i < bufferSize; ++i) {
        audioBuffer[i] += noiseLevel * ((float)rand() / RAND_MAX - 0.5f);
    }

    // Write processed audio to output file
    writeAudioFile(outputFileName, audioBuffer, bufferSize, sfinfo);

    // Free allocated memory
    free(audioBuffer);

    cout << "Audio anonymization complete. Output saved to " << outputFileName << endl;
    return 0;
}

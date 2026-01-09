#include "AudioManager.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <vector>

AudioManager::AudioManager()
    : m_device(nullptr)
    , m_context(nullptr)
    , m_bufferFootstep(0)
    , m_bufferDoor(0)
    , m_bufferFire(0)
    , m_bufferBackground(0)
    , m_bufferBaby(0)
    , m_sourceFootstep(0)
    , m_sourceDoor(0)
    , m_sourceFire(0)
    , m_sourceBackground(0)
    , m_sourceBaby(0)
    , m_initialized(false)
{
}

AudioManager::~AudioManager() {
    cleanup();
}

bool AudioManager::init() {
    // Open the default audio device
    m_device = alcOpenDevice(nullptr);
    if (!m_device) {
        std::cerr << "AudioManager: Failed to open audio device" << std::endl;
        return false;
    }

    // Create audio context
    m_context = alcCreateContext(m_device, nullptr);
    if (!m_context) {
        std::cerr << "AudioManager: Failed to create audio context" << std::endl;
        alcCloseDevice(m_device);
        m_device = nullptr;
        return false;
    }

    // Make context current
    if (!alcMakeContextCurrent(m_context)) {
        std::cerr << "AudioManager: Failed to make context current" << std::endl;
        alcDestroyContext(m_context);
        alcCloseDevice(m_device);
        m_context = nullptr;
        m_device = nullptr;
        return false;
    }

    // Generate sources
    alGenSources(1, &m_sourceFootstep);
    alGenSources(1, &m_sourceDoor);
    alGenSources(1, &m_sourceFire);
    alGenSources(1, &m_sourceBackground);
    alGenSources(1, &m_sourceBaby);

    // Check for errors
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        std::cerr << "AudioManager: Failed to generate sources, error: " << error << std::endl;
        cleanup();
        return false;
    }

    m_initialized = true;
    std::cout << "AudioManager: Initialized successfully" << std::endl;
    return true;
}

bool AudioManager::loadSounds(const std::string& audioPath) {
    if (!m_initialized) {
        std::cerr << "AudioManager: Cannot load sounds - not initialized" << std::endl;
        return false;
    }

    // Load footstep sound
    m_bufferFootstep = loadWavFile(audioPath + "/footsteps.wav");
    if (m_bufferFootstep == 0) {
        std::cerr << "AudioManager: Failed to load footsteps.wav" << std::endl;
        return false;
    }

    // Load door sound
    m_bufferDoor = loadWavFile(audioPath + "/opening-door.wav");
    if (m_bufferDoor == 0) {
        std::cerr << "AudioManager: Failed to load opening-door.wav" << std::endl;
        return false;
    }

    // Load fire sound
    m_bufferFire = loadWavFile(audioPath + "/fire-sounds.wav");
    if (m_bufferFire == 0) {
        std::cerr << "AudioManager: Failed to load fire-sounds.wav" << std::endl;
        return false;
    }

    // Load background music
    m_bufferBackground = loadWavFile(audioPath + "/horror-background.wav");
    if (m_bufferBackground == 0) {
        std::cerr << "AudioManager: Failed to load horror-background.wav" << std::endl;
        return false;
    }

    // Load baby crying
    m_bufferBaby = loadWavFile(audioPath + "/crying-baby.wav");
    if (m_bufferBaby == 0) {
        std::cerr << "AudioManager: Failed to load crying-baby.wav" << std::endl;
        return false;
    }

    // Attach buffers to sources
    alSourcei(m_sourceFootstep, AL_BUFFER, m_bufferFootstep);
    alSourcei(m_sourceDoor, AL_BUFFER, m_bufferDoor);
    alSourcei(m_sourceFire, AL_BUFFER, m_bufferFire);
    alSourcei(m_sourceBackground, AL_BUFFER, m_bufferBackground);
    alSourcei(m_sourceBaby, AL_BUFFER, m_bufferBaby);

    // Configure footstep source (one-shot, at listener)
    alSourcef(m_sourceFootstep, AL_GAIN, 0.7f);
    alSourcei(m_sourceFootstep, AL_SOURCE_RELATIVE, AL_TRUE);  // Relative to listener
    alSource3f(m_sourceFootstep, AL_POSITION, 0.0f, 0.0f, 0.0f);

    // Configure door source (one-shot, at listener)
    alSourcef(m_sourceDoor, AL_GAIN, 0.8f);
    alSourcei(m_sourceDoor, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(m_sourceDoor, AL_POSITION, 0.0f, 0.0f, 0.0f);

    // Configure fire source (looping ambient)
    alSourcef(m_sourceFire, AL_GAIN, 0.0f);  // Start silent
    alSourcei(m_sourceFire, AL_LOOPING, AL_TRUE);
    alSourcei(m_sourceFire, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(m_sourceFire, AL_POSITION, 0.0f, 0.0f, 0.0f);

    // Configure background music (low volume, looping, everywhere)
    alSourcef(m_sourceBackground, AL_GAIN, 0.12f);  // Low background volume
    alSourcei(m_sourceBackground, AL_LOOPING, AL_TRUE);
    alSourcei(m_sourceBackground, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(m_sourceBackground, AL_POSITION, 0.0f, 0.0f, 0.0f);

    // Configure baby crying (looping, volume controlled by distance)
    alSourcef(m_sourceBaby, AL_GAIN, BABY_MIN_VOLUME);
    alSourcei(m_sourceBaby, AL_LOOPING, AL_TRUE);
    alSourcei(m_sourceBaby, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(m_sourceBaby, AL_POSITION, 0.0f, 0.0f, 0.0f);

    // Start playing looping sounds
    alSourcePlay(m_sourceFire);
    alSourcePlay(m_sourceBackground);
    alSourcePlay(m_sourceBaby);

    std::cout << "AudioManager: Loaded all sounds successfully" << std::endl;
    return true;
}

void AudioManager::cleanup() {
    if (m_initialized) {
        // Stop all sources
        alSourceStop(m_sourceFootstep);
        alSourceStop(m_sourceDoor);
        alSourceStop(m_sourceFire);
        alSourceStop(m_sourceBackground);
        alSourceStop(m_sourceBaby);

        // Delete sources
        alDeleteSources(1, &m_sourceFootstep);
        alDeleteSources(1, &m_sourceDoor);
        alDeleteSources(1, &m_sourceFire);
        alDeleteSources(1, &m_sourceBackground);
        alDeleteSources(1, &m_sourceBaby);

        // Delete buffers
        if (m_bufferFootstep) alDeleteBuffers(1, &m_bufferFootstep);
        if (m_bufferDoor) alDeleteBuffers(1, &m_bufferDoor);
        if (m_bufferFire) alDeleteBuffers(1, &m_bufferFire);
        if (m_bufferBackground) alDeleteBuffers(1, &m_bufferBackground);
        if (m_bufferBaby) alDeleteBuffers(1, &m_bufferBaby);

        m_initialized = false;
    }

    // Destroy context and close device
    if (m_context) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(m_context);
        m_context = nullptr;
    }

    if (m_device) {
        alcCloseDevice(m_device);
        m_device = nullptr;
    }
}

void AudioManager::playFootstep() {
    if (!m_initialized) return;

    // Check if already playing - if so, don't interrupt
    ALint state;
    alGetSourcei(m_sourceFootstep, AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING) {
        alSourcePlay(m_sourceFootstep);
    }
}

void AudioManager::playDoorSound() {
    if (!m_initialized) return;

    // Always restart for door sounds
    alSourceStop(m_sourceDoor);
    alSourcePlay(m_sourceDoor);
}

void AudioManager::updateFireAmbient(float distanceToNearest) {
    if (!m_initialized) return;

    // Calculate volume based on distance
    // Full volume at FIRE_MIN_DISTANCE or closer
    // Silent at FIRE_MAX_DISTANCE or further
    float volume = 0.0f;

    if (distanceToNearest <= FIRE_MIN_DISTANCE) {
        volume = 1.0f;
    } else if (distanceToNearest >= FIRE_MAX_DISTANCE) {
        volume = 0.0f;
    } else {
        // Linear falloff between min and max distance
        float range = FIRE_MAX_DISTANCE - FIRE_MIN_DISTANCE;
        volume = 1.0f - ((distanceToNearest - FIRE_MIN_DISTANCE) / range);
    }

    // Apply volume (with a maximum cap for comfort)
    alSourcef(m_sourceFire, AL_GAIN, volume * 0.85f);
}
void AudioManager::updateBabyCrying(float distanceToSource) {
    if (!m_initialized) return;

    float volume;
    if (distanceToSource <= BABY_MIN_DISTANCE) {
        volume = BABY_MAX_VOLUME;
    } else if (distanceToSource >= BABY_MAX_DISTANCE) {
        volume = BABY_MIN_VOLUME;
    } else {
        // Linear interpolation between max and min volume
        float t = (distanceToSource - BABY_MIN_DISTANCE) / (BABY_MAX_DISTANCE - BABY_MIN_DISTANCE);
        volume = BABY_MAX_VOLUME - t * (BABY_MAX_VOLUME - BABY_MIN_VOLUME);
    }

    alSourcef(m_sourceBaby, AL_GAIN, volume);
}
void AudioManager::updateListener(const glm::vec3& position, const glm::vec3& front, const glm::vec3& up) {
    if (!m_initialized) return;

    // Set listener position
    alListener3f(AL_POSITION, position.x, position.y, position.z);

    // Set listener orientation (forward and up vectors)
    ALfloat orientation[6] = {
        front.x, front.y, front.z,
        up.x, up.y, up.z
    };
    alListenerfv(AL_ORIENTATION, orientation);
}

ALuint AudioManager::loadWavFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "AudioManager: Cannot open file: " << filename << std::endl;
        return 0;
    }

    // Read RIFF header
    char riff[4];
    file.read(riff, 4);
    if (strncmp(riff, "RIFF", 4) != 0) {
        std::cerr << "AudioManager: Invalid WAV file (no RIFF): " << filename << std::endl;
        return 0;
    }

    // Skip file size
    file.seekg(4, std::ios::cur);

    // Read WAVE format
    char wave[4];
    file.read(wave, 4);
    if (strncmp(wave, "WAVE", 4) != 0) {
        std::cerr << "AudioManager: Invalid WAV file (no WAVE): " << filename << std::endl;
        return 0;
    }

    // Find fmt chunk
    char chunkId[4];
    uint32_t chunkSize;

    while (file.read(chunkId, 4)) {
        file.read(reinterpret_cast<char*>(&chunkSize), 4);

        if (strncmp(chunkId, "fmt ", 4) == 0) {
            break;
        }
        file.seekg(chunkSize, std::ios::cur);
    }

    if (strncmp(chunkId, "fmt ", 4) != 0) {
        std::cerr << "AudioManager: No fmt chunk found: " << filename << std::endl;
        return 0;
    }

    // Read format data
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;

    file.read(reinterpret_cast<char*>(&audioFormat), 2);
    file.read(reinterpret_cast<char*>(&numChannels), 2);
    file.read(reinterpret_cast<char*>(&sampleRate), 4);
    file.read(reinterpret_cast<char*>(&byteRate), 4);
    file.read(reinterpret_cast<char*>(&blockAlign), 2);
    file.read(reinterpret_cast<char*>(&bitsPerSample), 2);

    // Skip any extra format bytes
    if (chunkSize > 16) {
        file.seekg(chunkSize - 16, std::ios::cur);
    }

    // Find data chunk
    while (file.read(chunkId, 4)) {
        file.read(reinterpret_cast<char*>(&chunkSize), 4);

        if (strncmp(chunkId, "data", 4) == 0) {
            break;
        }
        file.seekg(chunkSize, std::ios::cur);
    }

    if (strncmp(chunkId, "data", 4) != 0) {
        std::cerr << "AudioManager: No data chunk found: " << filename << std::endl;
        return 0;
    }

    // Read audio data
    std::vector<char> audioData(chunkSize);
    file.read(audioData.data(), chunkSize);

    file.close();

    // Determine OpenAL format
    ALenum format;
    if (numChannels == 1) {
        format = (bitsPerSample == 16) ? AL_FORMAT_MONO16 : AL_FORMAT_MONO8;
    } else {
        format = (bitsPerSample == 16) ? AL_FORMAT_STEREO16 : AL_FORMAT_STEREO8;
    }

    // Create OpenAL buffer
    ALuint buffer;
    alGenBuffers(1, &buffer);
    alBufferData(buffer, format, audioData.data(), chunkSize, sampleRate);

    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        std::cerr << "AudioManager: Failed to buffer audio data: " << filename << " (error: " << error << ")" << std::endl;
        alDeleteBuffers(1, &buffer);
        return 0;
    }

    std::cout << "AudioManager: Loaded " << filename << " (" << numChannels << "ch, " 
              << sampleRate << "Hz, " << bitsPerSample << "bit)" << std::endl;

    return buffer;
}

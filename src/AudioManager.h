#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <AL/al.h>
#include <AL/alc.h>
#include <glm/glm.hpp>
#include <string>

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    // Initialize OpenAL - call once at startup
    bool init();

    // Load audio files - call after init
    bool loadSounds(const std::string& audioPath);

    // Cleanup - called automatically in destructor
    void cleanup();

    // Play footstep sound (one-shot, at listener position)
    void playFootstep();

    // Play door sound (one-shot, at listener position)
    void playDoorSound();

    // Update fire ambient volume based on distance to nearest torch
    // distanceToNearest: distance in world units to the closest torch
    void updateFireAmbient(float distanceToNearest);

    // Update baby crying sound based on distance to source
    void updateBabyCrying(float distanceToSource);

    // Update listener position and orientation (call each frame)
    void updateListener(const glm::vec3& position, const glm::vec3& front, const glm::vec3& up);

    // Check if audio system is initialized
    bool isInitialized() const { return m_initialized; }

private:
    // Load a single WAV file into an OpenAL buffer
    ALuint loadWavFile(const std::string& filename);

    // OpenAL device and context
    ALCdevice* m_device;
    ALCcontext* m_context;

    // Sound buffers
    ALuint m_bufferFootstep;
    ALuint m_bufferDoor;
    ALuint m_bufferFire;
    ALuint m_bufferBackground;
    ALuint m_bufferBaby;

    // Sound sources
    ALuint m_sourceFootstep;
    ALuint m_sourceDoor;
    ALuint m_sourceFire;       // Looping ambient fire
    ALuint m_sourceBackground; // Looping background music
    ALuint m_sourceBaby;       // Looping baby crying

    // State
    bool m_initialized;

    // Fire ambient settings
    static constexpr float FIRE_MAX_DISTANCE = 5.0f;   // Distance at which fire is silent
    static constexpr float FIRE_MIN_DISTANCE = 1.0f;   // Distance at which fire is full volume

    // Baby crying settings
    static constexpr float BABY_MAX_DISTANCE = 40.0f;  // Distance at which baby is at min volume
    static constexpr float BABY_MIN_DISTANCE = 2.0f;   // Distance at which baby is at max volume
    static constexpr float BABY_MIN_VOLUME = 0.15f;    // Minimum volume (heard everywhere)
    static constexpr float BABY_MAX_VOLUME = 0.7f;     // Maximum volume when close
};

#endif // AUDIO_MANAGER_H

#ifndef _SOUND_
#define _SOUND_

#include "common.h"
#include "vector3.h"
#include <external/OpenALsoft/include/al/alc.h>
#include <external/OpenALsoft/include/al/al.h>
#include <vorbis/vorbisfile.h>

typedef struct {
    ALuint soundID;
} TSound;

typedef struct {
    OggVorbis_File vorbisFile;
    ALuint bufferID;
    ALenum format;
    ALuint freq;
    // totalBytes is total lenght of pcm data decoded from file 
    ALuint totalBytes;
    // blockSize is length of 'data' 
    ALuint blockSize;
    // pcmSize is length of pcm data in 'data', readen from file 
    // which is used to set pcm data to openal buffer
    ALuint pcmSize;
    ALbyte * data;
    ALbyte streamed;
} TSoundBuffer;

typedef struct {
    ALCdevice * device;
    ALCcontext * context;
} TSoundSystem;

#define STREAMED_BUFFER_SIZE (65536)

void SoundSystem_Initialize( TSoundSystem * sndSys );
void SoundSystem_Free( TSoundSystem * sndSys );
void SoundSource_Create( TSound * sound, TSoundBuffer * buffer );
void SoundSource_Free( TSound * sound );
void SoundBuffer_LoadFile( const char * filename, TSoundBuffer * buffer, char streamed );
void SoundBuffer_Free( TSoundBuffer * buffer );
void SoundSource_Play( TSound * sound );
bool SoundSource_IsPlaying( TSound * sound );
void Util_CheckALErrorFunc( const char * file, int line );
void SoundListener_SetVolume( float vol );
void SoundListener_SetOrientation( const TVec3 * look, const TVec3 * up );
void SoundListener_SetPosition( const TVec3 * position );
void SoundSource_SetPosition( TSound * sound, const TVec3 * position );
void SoundSource_SetVolume( TSound * sound, float vol );
// some useful macro 
#ifdef _DEBUG_GL_
#   define Util_CheckALError( alFunc ) ( alFunc ); Util_CheckALErrorFunc( __FILE__, __LINE__ )
#else
#   define Util_CheckALError( alFunc ) ( alFunc )
#endif

#endif
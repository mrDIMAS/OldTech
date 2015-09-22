#include "sound.h"
#include <ogg/ogg.h>


void ReadSoundBufferBlock( TSoundBuffer * buffer );

void SoundSystem_Initialize( TSoundSystem * sndSys ) {
    sndSys->device = alcOpenDevice( NULL );
    sndSys->context = alcCreateContext( sndSys->device, 0 );
    alcMakeContextCurrent( sndSys->context );
}

void SoundSystem_Free( TSoundSystem * sndSys ) {
    alcMakeContextCurrent( 0 );
    alcDestroyContext( sndSys->context );
    alcCloseDevice( sndSys->device );
}

void SoundSource_Create( TSound * sound, TSoundBuffer * buffer ) {
    Util_CheckALError( alGenSources( 1, &sound->soundID ));
    Util_CheckALError( alSourcei( sound->soundID, AL_BUFFER, buffer->bufferID ));
}

void SoundSource_Free( TSound * sound ) {
    Util_CheckALError( alDeleteSources( 1, &sound->soundID ));
}

void SoundSource_Play( TSound * sound ) {
    assert( sound );
    if( alIsSource( sound->soundID )) {
        Util_CheckALError( alSourcePlay( sound->soundID ));
    }
}

bool SoundSource_IsPlaying( TSound * sound ) {
    int sourceState;
    Util_CheckALError( alGetSourcei( sound->soundID, AL_SOURCE_STATE, &sourceState ));
    return sourceState == AL_PLAYING;
}

void SoundSource_SetVolume( TSound * sound, float vol ) {
    Util_CheckALError( alSourcef( sound->soundID, AL_GAIN, vol ));
}

void SoundListener_SetVolume( float vol ) {
    Util_CheckALError( alListenerf( AL_GAIN, vol ));
}

void SoundListener_SetOrientation( const TVec3 * look, const TVec3 * up ) {
    float orientation[6];
    orientation[0] = look->x;
    orientation[1] = look->y;
    orientation[2] = look->z;
    orientation[3] = up->x;
    orientation[4] = up->y;
    orientation[5] = up->z;
    Util_CheckALError( alListenerfv( AL_ORIENTATION, orientation ));
}

void SoundListener_SetPosition( const TVec3 * position ) {
    Util_CheckALError( alListener3f( AL_POSITION, position->x, position->y, position->z ));
}

void SoundSource_SetPosition( TSound * sound, const TVec3 * position ) {
    Util_CheckALError( alSource3f( sound->soundID, AL_POSITION, position->x, position->y, position->z ));
}

void Util_CheckALErrorFunc( const char * file, int line ) {
    ALenum g = alGetError();
    switch (g) {
    case AL_INVALID_ENUM:
        Util_RaiseError( "OpenAL Error: AL_INVALID_ENUM in '%s' in line %d", file, line );
    case AL_INVALID_VALUE:
        Util_RaiseError( "OpenAL Error: AL_INVALID_VALUE in '%s' in line %d", file, line );
    case AL_INVALID_OPERATION:
        Util_RaiseError( "OpenAL Error: AL_INVALID_OPERATION in '%s' in line %d", file, line );
    case AL_OUT_OF_MEMORY:
        Util_RaiseError( "OpenAL Error: AL_OUT_OF_MEMORY in '%s' in line %d", file, line );
    };
}

void ReadSoundBufferBlock( TSoundBuffer * buffer ) {
    assert( buffer );

    unsigned int totalcharsReaden = 0;
    int channel = 0;

    OggVorbis_File * vorbisFile = &buffer->vorbisFile;

    while( totalcharsReaden < buffer->blockSize ) {
        int charsReaden = ov_read( vorbisFile, (char*)(buffer->data + totalcharsReaden), buffer->blockSize - totalcharsReaden, 0, 2, 1, &channel );
        if( charsReaden <= 0 ) {
            if( charsReaden == 0 ) {
                /* rewind */
                ov_time_seek( vorbisFile, 0 );
            }
            break;
        }
        totalcharsReaden += charsReaden;
    };
    buffer->pcmSize = totalcharsReaden;

    Util_CheckALError( alBufferData( buffer->bufferID, buffer->format, buffer->data, buffer->pcmSize, buffer->freq ));
}

void SoundBuffer_LoadFile( const char * filename, TSoundBuffer * buffer, char streamed ) {
    assert( filename );
    assert( buffer );

    Util_CheckALError( alGenBuffers( 1, &buffer->bufferID ));

    FILE * file = fopen( filename, "rb" );

    if( ov_open_callbacks( file, &buffer->vorbisFile, 0, -1, OV_CALLBACKS_DEFAULT ) ) {
        Util_RaiseError( "File '%s' isn't ogg compressed file!", filename );
    };

    vorbis_info * vorbisInfo = ov_info( &buffer->vorbisFile, -1 );

    /* check channels */
    if( vorbisInfo->channels > 2 ) {
        Util_RaiseError( "File '%s' not stereo or mono!", filename );
    }
    if( vorbisInfo->channels == 1 ) {
        buffer->format = AL_FORMAT_MONO16;
    } else if( vorbisInfo->channels == 2 ) {
        buffer->format = AL_FORMAT_STEREO16;
    }

    buffer->streamed = streamed;
    buffer->freq = vorbisInfo->rate;
    int samplePairLen = (buffer->format == AL_FORMAT_STEREO16) ? 4 : 2;
    buffer->totalBytes = ov_pcm_total( &buffer->vorbisFile, -1 ) * samplePairLen;

    /* allocate memory */
    if( buffer->streamed ) {
        buffer->blockSize = STREAMED_BUFFER_SIZE;
    } else {
        buffer->blockSize = buffer->totalBytes;
    };

    buffer->data = Memory_AllocateClean( buffer->blockSize );

    if( !streamed ) {
        /* read entire file from file */
        ReadSoundBufferBlock( buffer );

        ov_clear( &buffer->vorbisFile );
        fclose( file );
    }
}

void SoundBuffer_Free( TSoundBuffer * buffer ) {
    assert( buffer );

    alDeleteBuffers( 1, &buffer->bufferID );
}
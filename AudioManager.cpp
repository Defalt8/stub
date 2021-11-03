#include <AudioManager.hpp>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>

constexpr ALuint INVALID_AL_ID = 0;

ALCdevice *_g_al_device = 0;
int _g_handles = 0;

struct AudioManagerData
{
	ALCdevice *device;
	ALCcontext *context;
	AudioManagerData(ALCdevice* _device = 0, ALCcontext* _context = 0) :
		device(_device),
		context(_context)
	{}
};

//
// AudioManager::AudioBuffer
//

AudioManager::AudioBuffer::AudioBuffer(const AudioManager& _audio_manager) :
	audio_manager(_audio_manager),
	buffer_id(m_buffer_id),
	format(buffer_format),
	size(buffer_size),
	frequency(buffer_frequency),
	buffer_format(FORMAT_NONE),
	buffer_size(0),
	buffer_frequency(0),
	m_buffer_id(INVALID_AL_ID)
{}

AudioManager::AudioBuffer::~AudioBuffer()
{
	this->destroy();
}

AudioManager::AudioBuffer::AudioBuffer(AudioBuffer&& audio_buffer) :
	audio_manager(audio_buffer.audio_manager),
	buffer_id(m_buffer_id),
	format(buffer_format),
	size(buffer_size),
	frequency(buffer_frequency),
	buffer_format(audio_buffer.buffer_format),
	buffer_size(audio_buffer.buffer_size),
	buffer_frequency(audio_buffer.buffer_frequency),
	m_buffer_id(audio_buffer.m_buffer_id)
{
	audio_buffer.m_buffer_id = INVALID_AL_ID;
}

bool AudioManager::AudioBuffer::create()
{
	if(!audio_manager.makeCurrent())
		return false;
	this->destroy();
	while(alGetError() != AL_NO_ERROR);
	alGenBuffers(1, &m_buffer_id);
	if(alGetError() != AL_NO_ERROR || m_buffer_id == INVALID_AL_ID)
	{
		this->destroy();
		return false;
	}
	return true;
}

bool AudioManager::AudioBuffer::destroy()
{
	if(!audio_manager.makeCurrent() || m_buffer_id == INVALID_AL_ID)
		return false;
	while(alGetError() != AL_NO_ERROR);
	alDeleteBuffers(1, &m_buffer_id);
	buffer_format = FORMAT_NONE;
	buffer_size = 0;
	buffer_frequency = 0;
	m_buffer_id = INVALID_AL_ID;
	return alGetError() == AL_NO_ERROR;
}

bool AudioManager::AudioBuffer::isValid() const
{
	return audio_manager.makeCurrent() && m_buffer_id != INVALID_AL_ID && alIsBuffer(m_buffer_id);
}

bool AudioManager::AudioBuffer::loadFromFile(const char* wav_file_path)
{
	if(!axl::util::File::exists(wav_file_path) || !AudioBuffer::isValid() || !audio_manager.makeCurrent())
		return false;
	axl::media::audio::WAV wav;
	if(!wav.loadFromFile(wav_file_path))
		return false;
	Format _format = FORMAT_NONE;
	void* _data = 0;
	size_t _size = 0, _frequency = 0;
	if(wav.format_chunk.format_tag != (uint16_t)axl::media::audio::WAV::WaveFormat::PCM)
		return false;
	switch(wav.format_chunk.bits_per_sample)
	{
		default: return false;
		case 8:
			switch(wav.format_chunk.channels)
			{
				default: return false;
				case 1: _format = FORMAT_MONO8; break;
				case 2: _format = FORMAT_STEREO8; break;
			}
			break;
		case 16:
			switch(wav.format_chunk.channels)
			{
				default: return false;
				case 1: _format = FORMAT_MONO16; break;
				case 2: _format = FORMAT_STEREO16; break;
			}
			break;
	}
	_data = wav.wave_data;
	_size = wav.data_header.chunk_size;
	_frequency = wav.format_chunk.samples_per_sec;
	return AudioBuffer::setData(_format, _data, _size, _frequency);
}

bool AudioManager::AudioBuffer::setData(Format _format, void* _data, size_t _size, size_t _frequency)
{
	if(!_data || _size==0 || _frequency==0 || !AudioBuffer::isValid() || !audio_manager.makeCurrent())
		return false;
	ALenum al_format;
	switch(_format)
	{
		case FORMAT_NONE:
		default: return false;
		case FORMAT_MONO8: al_format = AL_FORMAT_MONO8; break;
		case FORMAT_MONO16: al_format = AL_FORMAT_MONO16; break;
		case FORMAT_STEREO8: al_format = AL_FORMAT_STEREO8; break;
		case FORMAT_STEREO16: al_format = AL_FORMAT_STEREO16; break;
	}
	while(alGetError() != AL_NO_ERROR);
	alBufferData(m_buffer_id, al_format, _data, _size, _frequency);
	if(alGetError() == AL_NO_ERROR)
	{
		buffer_format = _format;
		buffer_size = _size;
		buffer_frequency = _frequency;
		return true;
	}
	return false;
}

//
// AudioManager::AudioSource
//

AudioManager::AudioSource::AudioSource(const AudioManager& _audio_manager) :
	audio_manager(_audio_manager),
	audio_buffer(source_audio_buffer),
	source_id(m_source_id),
	loop(source_loop),
	pitch(source_pitch),
	gain(source_gain),
	min_gain(source_min_gain),
	max_gain(source_max_gain),
	max_distance(source_max_distance),
	position(source_position),
	velocity(source_velocity),
	direction(source_direction),
	source_audio_buffer(0),
	source_loop(false),
	source_pitch(1.f),
	source_gain(1.f),
	source_min_gain(0.f),
	source_max_gain(1.f),
	source_max_distance(1000.f),
	source_position(0.f, 0.f, 0.f),
	source_velocity(0.f, 0.f, 0.f),
	source_direction(0.f),
	m_source_id(INVALID_AL_ID)
{}

AudioManager::AudioSource::~AudioSource()
{
	this->destroy();
}

AudioManager::AudioSource::AudioSource(AudioSource&& audio_source) :
	audio_manager(audio_source.audio_manager),
	audio_buffer(source_audio_buffer),
	source_id(m_source_id),
	loop(source_loop),
	pitch(source_pitch),
	gain(source_gain),
	min_gain(source_min_gain),
	max_gain(source_max_gain),
	max_distance(source_max_distance),
	position(source_position),
	velocity(source_velocity),
	direction(source_direction),
	source_audio_buffer(audio_source.source_audio_buffer),
	source_loop(audio_source.source_loop),
	source_pitch(audio_source.source_pitch),
	source_gain(audio_source.source_gain),
	source_min_gain(audio_source.source_min_gain),
	source_max_gain(audio_source.source_max_gain),
	source_max_distance(audio_source.source_max_distance),
	source_position(audio_source.source_position),
	source_velocity(audio_source.source_velocity),
	source_direction(audio_source.source_direction),
	m_source_id(audio_source.m_source_id)
{
	audio_source.m_source_id = INVALID_AL_ID;
}

bool AudioManager::AudioSource::create()
{
	if(!audio_manager.makeCurrent())
		return false;
	this->destroy();
	while(alGetError() != AL_NO_ERROR);
	alGenSources(1, &m_source_id);
	if(alGetError() != AL_NO_ERROR || m_source_id == INVALID_AL_ID)
	{
		this->destroy();
		return false;
	}
	alSourcei(m_source_id, AL_LOOPING, (ALint)source_loop);
	alSourcef(m_source_id, AL_PITCH, source_pitch);
	alSourcef(m_source_id, AL_GAIN, source_gain);
	alSourcef(m_source_id, AL_MIN_GAIN, source_min_gain);
	alSourcef(m_source_id, AL_MAX_GAIN, source_max_gain);
	alSourcef(m_source_id, AL_MAX_DISTANCE, source_max_distance);
	alSource3f(m_source_id, AL_POSITION, source_position.x, source_position.y, source_position.z);
	alSource3f(m_source_id, AL_VELOCITY, source_velocity.x, source_velocity.y, source_velocity.z);
	alSource3f(m_source_id, AL_DIRECTION, source_direction.x, source_direction.y, source_direction.z);
	if(source_audio_buffer)
		this->setBuffer(source_audio_buffer);
	return true;
}

bool AudioManager::AudioSource::destroy()
{
	if(!audio_manager.makeCurrent() || m_source_id == INVALID_AL_ID)
		return false;
	while(alGetError() != AL_NO_ERROR);
	alSourceStop(m_source_id);
	alSourcei(m_source_id, AL_BUFFER, 0);
	alDeleteSources(1, &m_source_id);
	m_source_id = INVALID_AL_ID;
	return alGetError() == AL_NO_ERROR;
}

bool AudioManager::AudioSource::isValid() const
{
	return audio_manager.makeCurrent() && m_source_id != INVALID_AL_ID && alIsSource(m_source_id);
}

bool AudioManager::AudioSource::isPlaying() const
{
	if(!AudioSource::isValid() || !audio_manager.makeCurrent())
		return false;
	while(alGetError() != AL_NO_ERROR);
	ALint state;
	alGetSourcei(m_source_id, AL_SOURCE_STATE, &state);
	if(alGetError() == AL_NO_ERROR)
		return state == AL_PLAYING;
	return false;
}

bool AudioManager::AudioSource::isPaused() const
{
	if(!AudioSource::isValid() || !audio_manager.makeCurrent())
		return false;
	while(alGetError() != AL_NO_ERROR);
	ALint state;
	alGetSourcei(m_source_id, AL_SOURCE_STATE, &state);
	if(alGetError() == AL_NO_ERROR)
		return state == AL_PAUSED;
	return false;
}

bool AudioManager::AudioSource::isStopped() const
{
	if(!AudioSource::isValid() || !audio_manager.makeCurrent())
		return false;
	while(alGetError() != AL_NO_ERROR);
	ALint state;
	alGetSourcei(m_source_id, AL_SOURCE_STATE, &state);
	if(alGetError() == AL_NO_ERROR)
		return state == AL_STOPPED;
	return false;
}

bool AudioManager::AudioSource::play() const
{
	if(!AudioSource::isValid() || !audio_manager.makeCurrent())
		return false;
	while(alGetError() != AL_NO_ERROR);
	alSourcePlay(m_source_id);
	return alGetError() == AL_NO_ERROR;
}

bool AudioManager::AudioSource::pause() const
{
	if(!AudioSource::isValid() || !audio_manager.makeCurrent())
		return false;
	while(alGetError() != AL_NO_ERROR);
	alSourcePause(m_source_id);
	return alGetError() == AL_NO_ERROR;
}

bool AudioManager::AudioSource::stop() const
{
	if(!AudioSource::isValid() || !audio_manager.makeCurrent())
		return false;
	while(alGetError() != AL_NO_ERROR);
	alSourceStop(m_source_id);
	return alGetError() == AL_NO_ERROR;
}

bool AudioManager::AudioSource::setLoop(bool _loop)
{
	if(!AudioSource::isValid() || !audio_manager.makeCurrent())
		return false;
	while(alGetError() != AL_NO_ERROR);
	alSourcei(m_source_id, AL_LOOPING, (ALint)_loop);
	if(alGetError() == AL_NO_ERROR)
	{
		source_loop = _loop;
		return true;
	}
	return false;
}

bool AudioManager::AudioSource::setPitch(float _pitch)
{
	if(!AudioSource::isValid() || !audio_manager.makeCurrent())
		return false;
	while(alGetError() != AL_NO_ERROR);
	alSourcef(m_source_id, AL_PITCH, _pitch);
	if(alGetError() == AL_NO_ERROR)
	{
		source_pitch = _pitch;
		return true;
	}
	return false;
}

bool AudioManager::AudioSource::setGain(float _gain)
{
	if(!AudioSource::isValid() || !audio_manager.makeCurrent())
		return false;
	while(alGetError() != AL_NO_ERROR);
	alSourcef(m_source_id, AL_GAIN, _gain);
	if(alGetError() == AL_NO_ERROR)
	{
		source_gain = _gain;
		return true;
	}
	return false;
}

bool AudioManager::AudioSource::setMinGain(float _min_gain)
{
	if(!AudioSource::isValid() || !audio_manager.makeCurrent())
		return false;
	while(alGetError() != AL_NO_ERROR);
	alSourcef(m_source_id, AL_MIN_GAIN, _min_gain);
	if(alGetError() == AL_NO_ERROR)
	{
		source_min_gain = _min_gain;
		return true;
	}
	return false;
}

bool AudioManager::AudioSource::setMaxGain(float _max_gain)
{
	if(!AudioSource::isValid() || !audio_manager.makeCurrent())
		return false;
	while(alGetError() != AL_NO_ERROR);
	alSourcef(m_source_id, AL_MAX_GAIN, _max_gain);
	if(alGetError() == AL_NO_ERROR)
	{
		source_max_gain = _max_gain;
		return true;
	}
	return false;
}

bool AudioManager::AudioSource::setMaxDistance(float _max_distance)
{
	if(!AudioSource::isValid() || !audio_manager.makeCurrent())
		return false;
	while(alGetError() != AL_NO_ERROR);
	alSourcef(m_source_id, AL_MAX_DISTANCE, _max_distance);
	if(alGetError() == AL_NO_ERROR)
	{
		source_max_distance = _max_distance;
		return true;
	}
	return false;
}

bool AudioManager::AudioSource::setPosition(const axl::math::Vec3f& _position)
{
	if(!AudioSource::isValid() || !audio_manager.makeCurrent())
		return false;
	while(alGetError() != AL_NO_ERROR);
	alSource3f(m_source_id, AL_POSITION, _position.x, _position.y, _position.z);
	if(alGetError() == AL_NO_ERROR)
	{
		source_position = _position;
		return true;
	}
	return false;
}

bool AudioManager::AudioSource::setVelocity(const axl::math::Vec3f& _velocity)
{
	if(!AudioSource::isValid() || !audio_manager.makeCurrent())
		return false;
	while(alGetError() != AL_NO_ERROR);
	alSource3f(m_source_id, AL_VELOCITY, _velocity.x, _velocity.y, _velocity.z);
	if(alGetError() == AL_NO_ERROR)
	{
		source_velocity = _velocity;
		return true;
	}
	return false;
}

bool AudioManager::AudioSource::setDirection(const axl::math::Vec3f& _direction)
{
	if(!AudioSource::isValid() || !audio_manager.makeCurrent())
		return false;
	while(alGetError() != AL_NO_ERROR);
	alSource3f(m_source_id, AL_DIRECTION, _direction.x, _direction.y, _direction.z);
	if(alGetError() == AL_NO_ERROR)
	{
		source_direction = _direction;
		return true;
	}
	return false;
}

bool AudioManager::AudioSource::setBuffer(const AudioBuffer* audio_buffer)
{
	if(!AudioSource::isValid() || (audio_buffer && !audio_buffer->isValid()) || !audio_manager.makeCurrent())
		return false;
	while(alGetError() != AL_NO_ERROR);
	if(!audio_buffer)
		alSourcei(m_source_id, AL_BUFFER, 0);
	else
		alSourcei(m_source_id, AL_BUFFER, audio_buffer->buffer_id);
	if(alGetError() == AL_NO_ERROR)
	{
		source_audio_buffer = audio_buffer;
		return true;
	}
	return false;
}

//
// AudioManager
//

AudioManager::AudioManager() :
	m_reserved(new AudioManagerData()),
	m_sources(),
	m_buffers(),
	position(audioman_position),
	velocity(audioman_velocity),
	orientation_at(audioman_orientation_at),
	orientation_up(audioman_orientation_up),
	audioman_position(0.f, 0.f, 0.f),
	audioman_velocity(0.f, 0.f, 0.f),
	audioman_orientation_at(0.f, 0.f, -1.f),
	audioman_orientation_up(0.f, 1.f, 0.f)
{}

AudioManager::~AudioManager()
{
	this->destroy();
	if(m_reserved)
		delete (AudioManagerData*)m_reserved;
}

bool AudioManager::create()
{
	AudioManagerData *data = ((AudioManagerData*)m_reserved);
	if(!data)
		m_reserved = data = new AudioManagerData();
	if(!data)
		return false;
	data->device = _g_al_device ? _g_al_device : alcOpenDevice(NULL);
	++_g_handles;
	if(!data->device)
		return false;
	data->context = alcCreateContext(data->device, NULL);
	if(!data->context)
	{
		if(_g_handles <= 0)
			alcCloseDevice(data->device);
		else --_g_handles;
		data->device = 0;
		return false;
	}
	if(!AudioManager::makeCurrent())
		return false;
	while(alGetError() != AL_NO_ERROR);
	alListener3f(AL_POSITION, audioman_position.x, audioman_position.y, audioman_position.z);
	alListener3f(AL_VELOCITY, audioman_velocity.x, audioman_velocity.y, audioman_velocity.z);
	float fp_orientation[] = { audioman_orientation_at.x, audioman_orientation_at.y, audioman_orientation_at.z, audioman_orientation_up.x, audioman_orientation_up.y, audioman_orientation_up.z };
	alListenerfv(AL_ORIENTATION, fp_orientation);
	return alGetError() == AL_NO_ERROR;
}

bool AudioManager::destroy()
{
	AudioManagerData *data = ((AudioManagerData*)m_reserved);
	if(!data || !data->device || !data->context)
		return false;
	for(axl::ds::ListIterator<AudioBuffer*> it = m_buffers.first(); it.isNotNull(); ++it)
	{
		AudioBuffer* buffer = it.ref();
		if(!buffer)
			continue;
		buffer->destroy();
	}
	for(axl::ds::ListIterator<AudioSource*> it = m_sources.first(); it.isNotNull(); ++it)
	{
		AudioSource* source = it.ref();
		if(!source)
			continue;
		source->destroy();
	}
	alcGetCurrentContext() != data->context || alcMakeContextCurrent(0);
	alcDestroyContext(data->context);
	if(_g_handles <= 0)
		alcCloseDevice(data->device);
	else
		--_g_handles;
	data->context = 0;
	data->device = 0;
	return true;
}

bool AudioManager::isValid() const
{
	AudioManagerData *data = ((AudioManagerData*)m_reserved);
	return data && data->device && data->context;
}

bool AudioManager::setPosition(const axl::math::Vec3f& _position)
{
	if(AudioManager::makeCurrent())
	{
		while(alGetError() != AL_NO_ERROR);
		alListener3f(AL_POSITION, _position.x, _position.y, _position.z);
		if(alGetError() != AL_NO_ERROR)
			return false;
	}
	audioman_position = _position;
	return true;
}

bool AudioManager::setVelocity(const axl::math::Vec3f& _velocity)
{
	if(AudioManager::makeCurrent())
	{
		while(alGetError() != AL_NO_ERROR);
		alListener3f(AL_VELOCITY, _velocity.x, _velocity.y, _velocity.z);
		if(alGetError() != AL_NO_ERROR)
			return false;
	}
	audioman_velocity = _velocity;
	return true;
}

bool AudioManager::setOrientationAt(const axl::math::Vec3f& _orientation_at)
{
	if(AudioManager::makeCurrent())
	{
		while(alGetError() != AL_NO_ERROR);
		float fp_orientation[] = { _orientation_at.x, _orientation_at.y, _orientation_at.z, audioman_orientation_up.x, audioman_orientation_up.y, audioman_orientation_up.z };
		alListenerfv(AL_ORIENTATION, fp_orientation);
		if(alGetError() != AL_NO_ERROR)
			return false;
	}
	audioman_orientation_at = _orientation_at;
	return true;
}

bool AudioManager::setOrientationUp(const axl::math::Vec3f& _orientation_up)
{
	if(AudioManager::makeCurrent())
	{
		while(alGetError() != AL_NO_ERROR);
		float fp_orientation[] = { audioman_orientation_at.x, audioman_orientation_at.y, audioman_orientation_at.z, _orientation_up.x, _orientation_up.y, _orientation_up.z };
		alListenerfv(AL_ORIENTATION, fp_orientation);
		if(alGetError() != AL_NO_ERROR)
			return false;
	}
	audioman_orientation_up = _orientation_up;
	return true;
}

bool AudioManager::setOrientation(const axl::math::Vec3f& _orientation_at, const axl::math::Vec3f& _orientation_up)
{
	if(AudioManager::makeCurrent())
	{
		while(alGetError() != AL_NO_ERROR);
		float fp_orientation[] = { _orientation_at.x, _orientation_at.y, _orientation_at.z, _orientation_up.x, _orientation_up.y, _orientation_up.z };
		alListenerfv(AL_ORIENTATION, fp_orientation);
		if(alGetError() != AL_NO_ERROR)
			return false;
	}
	audioman_orientation_at = _orientation_at;
	audioman_orientation_up = _orientation_up;
	return true;
}

AudioManager::AudioBuffer* AudioManager::newBuffer()
{
	if(!AudioManager::isValid())
		return 0;
	AudioBuffer *buffer = new AudioBuffer(*this);
	if(!buffer)
		return 0;
	if(!buffer->create())
	{
		delete buffer;
		return 0;
	}
	return buffer;
}

bool AudioManager::deleteBuffer(AudioManager::AudioBuffer* audio_buffer)
{
	if(!audio_buffer || !AudioManager::isValid())
		return false;
	for(axl::ds::ListIterator<AudioBuffer*> it = m_buffers.first(); it.isNotNull(); ++it)
	{
		AudioBuffer* buffer = it.ref();
		if(!buffer || buffer != audio_buffer)
			continue;
		m_buffers.remove(it);
		buffer->destroy();
		delete buffer;
		return true;
	}
	return false;
}

AudioManager::AudioSource* AudioManager::newSource()
{
	if(!AudioManager::isValid())
		return 0;
	AudioSource *source = new AudioSource(*this);
	if(!source)
		return 0;
	if(!source->create())
	{
		delete source;
		return 0;
	}
	return source;
}

bool AudioManager::deleteSource(AudioManager::AudioSource* audio_source)
{
	if(!audio_source || !AudioManager::isValid())
		return false;
	for(axl::ds::ListIterator<AudioSource*> it = m_sources.first(); it.isNotNull(); ++it)
	{
		AudioSource* source = it.ref();
		if(!source || source != audio_source)
			continue;
		m_sources.remove(it);
		source->destroy();
		delete source;
		return true;
	}
	return false;
}

bool AudioManager::makeCurrent() const
{
	AudioManagerData *data = ((AudioManagerData*)m_reserved);
	return data && data->device && data->context && (alcGetCurrentContext() == data->context || alcMakeContextCurrent(data->context));
}

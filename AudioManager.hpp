#pragma once
#include <stdafx.hpp>

class AudioManager
{
	public:
		class AudioBuffer
		{
			public:
				enum Format
				{
					FORMAT_NONE,
					FORMAT_MONO8,
					FORMAT_MONO16,
					FORMAT_STEREO8,
					FORMAT_STEREO16 
				};
			private:
				friend class AudioManager;
				AudioBuffer(const AudioManager& audio_manager);
			public:
				~AudioBuffer();
				AudioBuffer(AudioBuffer&& audio_buffer);
				AudioBuffer(const AudioBuffer&) = delete;
			public:
				bool create();
				bool destroy();
				bool isValid() const;
				bool loadFromFile(const char* wav_file_path);
				bool setData(Format format, void* data, size_t size, size_t frequency);
			public:
				const AudioManager& audio_manager;
				const unsigned int& buffer_id;
				const Format& format;
				const size_t& size;
				const size_t& frequency;
			protected:
				Format buffer_format;
				size_t buffer_size;
				size_t buffer_frequency;
			private:
				unsigned int m_buffer_id;
		};
		class AudioSource
		{
			private:
				friend class AudioManager;
				AudioSource(const AudioManager& audio_manager);
			public:
				~AudioSource();
				AudioSource(AudioSource&& audio_source);
				AudioSource(const AudioSource&) = delete;
			public:
				bool create();
				bool destroy();
				bool isValid() const;
				bool isPlaying() const;
				bool isPaused() const;
				bool isStopped() const;
				bool play() const;
				bool pause() const;
				bool stop() const;
				bool setLoop(bool loop);
				bool setPitch(float pitch);
				bool setGain(float gain);
				bool setMinGain(float min_gain);
				bool setMaxGain(float max_gain);
				bool setMaxDistance(float max_distance);
				bool setPosition(const axl::math::Vec3f& position);
				bool setVelocity(const axl::math::Vec3f& velocity);
				bool setDirection(const axl::math::Vec3f& direction);
				bool setBuffer(const AudioBuffer* audio_buffer);
			public:
				const AudioManager& audio_manager;
				const AudioBuffer*const& audio_buffer;
				const unsigned int& source_id;
				const bool& loop;
				const float& pitch;
				const float& gain;
				const float& min_gain;
				const float& max_gain;
				const float& max_distance;
				const axl::math::Vec3f& position;
				const axl::math::Vec3f& velocity;
				const axl::math::Vec3f& direction;
			protected:
				const AudioBuffer* source_audio_buffer;
				bool source_loop;
				float source_pitch;
				float source_gain;
				float source_min_gain;
				float source_max_gain;
				float source_max_distance;
				axl::math::Vec3f source_position;
				axl::math::Vec3f source_velocity;
				axl::math::Vec3f source_direction;
			private:
				unsigned int m_source_id;
		};
	public:
		AudioManager();
		~AudioManager();
		AudioManager(const AudioManager&) = delete;
	public:
		bool create();
		bool destroy();
		bool isValid() const;
		bool setPosition(const axl::math::Vec3f& position);
		bool setVelocity(const axl::math::Vec3f& velocity);
		bool setOrientationAt(const axl::math::Vec3f& orientation_at);
		bool setOrientationUp(const axl::math::Vec3f& orientation_up);
		bool setOrientation(const axl::math::Vec3f& orientation_at, const axl::math::Vec3f& orientation_up);
		AudioBuffer* newBuffer();
		bool deleteBuffer(AudioBuffer* audio_buffer);
		AudioSource* newSource();
		bool deleteSource(AudioSource* audio_source);
	protected:
		bool makeCurrent() const;
	public:
		const axl::math::Vec3f& position;
		const axl::math::Vec3f& velocity;
		const axl::math::Vec3f& orientation_at;
		const axl::math::Vec3f& orientation_up;
	protected:
		axl::math::Vec3f audioman_position;
		axl::math::Vec3f audioman_velocity;
		axl::math::Vec3f audioman_orientation_at;
		axl::math::Vec3f audioman_orientation_up;
	private:
		void* m_reserved;
		axl::ds::List<AudioBuffer*> m_buffers;
		axl::ds::List<AudioSource*> m_sources;
};

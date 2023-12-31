// Simplest Audio Player SDL2.cpp : �������̨Ӧ�ó������ڵ㡣

/**
* ��򵥵�SDL2������Ƶ�����ӣ�SDL2 ���� PCM��
* Simplest Audio Play SDL2 (SDL2 play PCM)
*
* ԭ����
* ������ Lei Xiaohua
* leixiaohua1020@126.com
* �й���ý��ѧ/���ֵ��Ӽ���
* Communication University of China / Digital TV Technology
* http://blog.csdn.net/leixiaohua1020
*
* �޸ģ�
* ���ĳ� Liu Wenchen
* 812288728@qq.com
* ���ӿƼ���ѧ/������Ϣ
* University of Electronic Science and Technology of China / Electronic and Information Science
* https://blog.csdn.net/ProgramNovice
*
* ������ʹ�� SDL2 ���� PCM ��Ƶ�������ݡ�
* SDL ʵ�����ǶԵײ��ͼ API��Direct3D��OpenGL���ķ�װ��ʹ���������Լ���ֱ�ӵ��õײ� API��
*
*
* �������ò�������:
*
* [��ʼ��]
* SDL_Init(): ��ʼ�� SDL��
* SDL_OpenAudio(): ���ݲ������洢�� SDL_AudioSpec������Ƶ�豸��
* SDL_PauseAudio(): ������Ƶ���ݡ�
*
* [ѭ����������]
* SDL_Delay(): ��ʱ�ȴ�������ɡ�
*
* This software plays PCM raw audio data using SDL2.
* SDL is a wrapper of low-level API (DirectSound).
* Use SDL is much easier than directly call these low-level API.
*
* The process is shown as follows:
*
* [Init]
* SDL_Init(): Init SDL.
* SDL_OpenAudio(): Opens the audio device with the desired
*					parameters (In SDL_AudioSpec).
* SDL_PauseAudio(): Play Audio.
*
* [Loop to play data]
* SDL_Delay(): Wait for completetion of playback.
*/

#include "stdafx.h"

#include <stdio.h>
#include <tchar.h>

// ��������޷��������ⲿ���� __imp__fprintf���÷����ں��� _ShowError �б�����
#pragma comment(lib, "legacy_stdio_definitions.lib")
extern "C"
{
	// ��������޷��������ⲿ���� __imp____iob_func���÷����ں��� _ShowError �б�����
	FILE __iob_func[3] = { *stdin, *stdout, *stderr };
}

extern "C"
{
#include "SDL2/SDL.h"
}

// Buffer:
// |-----------|-------------|
// chunk-------pos---len-----|
static  Uint8  *audio_chunk;
static  Uint32  audio_len;
static  Uint8  *audio_pos;

/* ��Ƶ�ص�����
* ��ʼ���ź󣬻�����Ƶ�������߳������ûص�������������Ƶ���ݵĲ��䣬��������ÿ�β��� 4096 ���ֽ�
* The audio function callback takes the following parameters:
* stream: A pointer to the audio buffer to be filled
* len: The length (in bytes) of the audio buffer
*
*/
void  fill_audio(void *udata, Uint8 *stream, int len)
{
	// SDL 2.0
	SDL_memset(stream, 0, len);
	if (audio_len == 0)		/*  Only  play  if  we  have  data  left  */
		return;
	len = (len > audio_len ? audio_len : len); /*  Mix  as  much  data  as  possible  */
	/* �������ź���
	* dst: Ŀ�����ݣ�����ǻص���������� stream ָ��ָ��ģ�ֱ��ʹ�ûص��� stream ָ�뼴��
	* src: ��Ƶ���ݣ�����ǽ���Ҫ���ŵ���Ƶ���ݻ쵽 stream ����ȥ����ô�������������Ҫ���Ĳ��ŵ�����
	* len: ��Ƶ���ݵĳ���
	* volume: ��������Χ 0~128 ��SAL_MIX_MAXVOLUME Ϊ 128�����õ���������������Ӳ��������
	*/
	SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
	audio_pos += len;
	audio_len -= len;
}


int main(int argc, char* argv[])
{
	// Step 1: ��ʼ����Ƶ��ϵͳ�ͼ�ʱ����ϵͳ
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER))
	{
		printf("Could not initialize SDL - %s.\n", SDL_GetError());
		return -1;
	}
	// Step 2: ������Ƶ��Ϣ����Ƶ�豸
	// SDL_AudioSpec �ǰ�����Ƶ�����ʽ�Ľṹ�壬ͬʱ��Ҳ��������Ƶ�豸��Ҫ��������ʱ���õĻص�����
	SDL_AudioSpec wanted_spec;
	wanted_spec.freq = 44100; // ������
	wanted_spec.format = AUDIO_S16SYS; // ��Ƶ���ݸ�ʽ
	wanted_spec.channels = 2; // ͨ����
	wanted_spec.silence = 0; // ��Ƶ���徲��ֵ
	wanted_spec.samples = 1024; // ������ 512��1024�����ò����ʿ��ܻᵼ�¿���
	wanted_spec.callback = fill_audio;// Ϊ��Ƶ�豸�ṩ���ݻص�����ֵʹ�� SDL ����Ԥ�ȶ����SDL_QueueAudio() �ص�������
	// ʹ�������������Ƶ�豸
	if (SDL_OpenAudio(&wanted_spec, NULL) < 0)
	{
		printf("Can't open audio.\n");
		return -1;
	}

	FILE *fp = fopen("NocturneNo2inEflat_44.1k_s16le.pcm", "rb+");
	if (fp == nullptr)
	{
		printf("Could not open this file.\n");
		return -1;
	}

	const int pcm_buffer_size = 4096;
	char *pcm_buffer = (char *)malloc(pcm_buffer_size);
	int data_count = 0;

	// Step 3: ��ʼ����
	SDL_PauseAudio(0);

	// Step 4: ѭ����������
	while (1)
	{
		if (fread(pcm_buffer, 1, pcm_buffer_size, fp) != pcm_buffer_size)
		{
			// Loop
			fseek(fp, 0, SEEK_SET);
			fread(pcm_buffer, 1, pcm_buffer_size, fp);
			data_count = 0;
		}
		printf("Now playing %10d bytes data.\n", data_count);
		data_count += pcm_buffer_size;
		// Set audio buffer (PCM data)
		audio_chunk = (Uint8 *)pcm_buffer;
		// Audio buffer length
		audio_len = pcm_buffer_size;
		audio_pos = audio_chunk;
		// Wait until finish
		while (audio_len > 0)
		{
			// ʹ�� SDL_Delay ���� 1ms ���ӳ٣��õ�ǰ������ʣ��δ���ŵĳ��ȴ��� 0 ���ǰ����ӳٽ��еȴ�
			SDL_Delay(1);
		}
	}

	// Step 5: �ر���Ƶ�豸
	SDL_CloseAudio();
	fclose(fp);
	free(pcm_buffer);
	// Step 6: �˳� SDL ϵͳ
	SDL_Quit();

	system("pause");
	return 0;
}


// Simplest Audio Player SDL2.cpp : 定义控制台应用程序的入口点。

/**
* 最简单的SDL2播放音频的例子（SDL2 播放 PCM）
* Simplest Audio Play SDL2 (SDL2 play PCM)
*
* 原程序：
* 雷霄骅 Lei Xiaohua
* leixiaohua1020@126.com
* 中国传媒大学/数字电视技术
* Communication University of China / Digital TV Technology
* http://blog.csdn.net/leixiaohua1020
*
* 修改：
* 刘文晨 Liu Wenchen
* 812288728@qq.com
* 电子科技大学/电子信息
* University of Electronic Science and Technology of China / Electronic and Information Science
* https://blog.csdn.net/ProgramNovice
*
* 本程序使用 SDL2 播放 PCM 音频采样数据。
* SDL 实际上是对底层绘图 API（Direct3D，OpenGL）的封装，使用起来明显简单于直接调用底层 API。
*
*
* 函数调用步骤如下:
*
* [初始化]
* SDL_Init(): 初始化 SDL。
* SDL_OpenAudio(): 根据参数（存储于 SDL_AudioSpec）打开音频设备。
* SDL_PauseAudio(): 播放音频数据。
*
* [循环播放数据]
* SDL_Delay(): 延时等待播放完成。
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

// 解决报错：无法解析的外部符号 __imp__fprintf，该符号在函数 _ShowError 中被引用
#pragma comment(lib, "legacy_stdio_definitions.lib")
extern "C"
{
	// 解决报错：无法解析的外部符号 __imp____iob_func，该符号在函数 _ShowError 中被引用
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

/* 音频回调函数
* 开始播放后，会有音频其他子线程来调用回调函数，进行音频数据的补充，经过测试每次补充 4096 个字节
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
	/* 混音播放函数
	* dst: 目标数据，这个是回调函数里面的 stream 指针指向的，直接使用回调的 stream 指针即可
	* src: 音频数据，这个是将需要播放的音频数据混到 stream 里面去，那么这里就是我们需要填充的播放的数据
	* len: 音频数据的长度
	* volume: 音量，范围 0~128 ，SAL_MIX_MAXVOLUME 为 128，设置的是软音量，不是硬件的音响
	*/
	SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
	audio_pos += len;
	audio_len -= len;
}


int main(int argc, char* argv[])
{
	// Step 1: 初始化音频子系统和计时器子系统
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER))
	{
		printf("Could not initialize SDL - %s.\n", SDL_GetError());
		return -1;
	}
	// Step 2: 根据音频信息打开音频设备
	// SDL_AudioSpec 是包含音频输出格式的结构体，同时它也包含当音频设备需要更多数据时调用的回调函数
	SDL_AudioSpec wanted_spec;
	wanted_spec.freq = 44100; // 采样率
	wanted_spec.format = AUDIO_S16SYS; // 音频数据格式
	wanted_spec.channels = 2; // 通道数
	wanted_spec.silence = 0; // 音频缓冲静音值
	wanted_spec.samples = 1024; // 基本是 512、1024，设置不合适可能会导致卡顿
	wanted_spec.callback = fill_audio;// 为音频设备提供数据回调（空值使用 SDL 自身预先定义的SDL_QueueAudio() 回调函数）
	// 使用所需参数打开音频设备
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

	// Step 3: 开始播放
	SDL_PauseAudio(0);

	// Step 4: 循环补充数据
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
			// 使用 SDL_Delay 进行 1ms 的延迟，用当前缓存区剩余未播放的长度大于 0 结合前面的延迟进行等待
			SDL_Delay(1);
		}
	}

	// Step 5: 关闭音频设备
	SDL_CloseAudio();
	fclose(fp);
	free(pcm_buffer);
	// Step 6: 退出 SDL 系统
	SDL_Quit();

	system("pause");
	return 0;
}


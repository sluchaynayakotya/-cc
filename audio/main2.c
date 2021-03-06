#include <windows.h>

int
main () {
  // source: https://gist.github.com/kbjorklu/6317308

  HWAVEOUT hWaveOut = NULL;
	WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 8, 0 };
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
	char buffer[8000 * 60] = {};

	// See http://goo.gl/hQdTi
	for (DWORD t = 0; t < sizeof(buffer); ++t)
		buffer[t] = ((((t * (t >> 8 | t >> 9) & 46 & t >> 8)) ^ (t & t >> 13 | t >> 6)) & 0xFF);

	WAVEHDR header = { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 };
	waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));

  // cant mix multiple waves  with waveout, need directsound

  Sleep(2 * 1000);

  waveOutReset ( hWaveOut );
  waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));

  for (DWORD t = 0; t < sizeof(buffer); ++t)
		buffer[t] = t << 2;

  header = ( WAVEHDR ) { buffer, sizeof(buffer), 0, 0, 0, 0, 0, 0 };
	waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));

  Sleep(2 * 1000);

  waveOutReset ( hWaveOut );
  waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));

	waveOutClose(hWaveOut);



}
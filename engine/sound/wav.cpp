//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name wav.cpp - wav support */
//
//      (c) Copyright 2003-2007 by Lutz Sammer, Fabrice Rossi and Nehal Mistry
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//
//      $Id$

//@{

/*----------------------------------------------------------------------------
-- Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "myendian.h"

#include "iolib.h"
#include "sound_server.h"
#include "wav.h"

/*----------------------------------------------------------------------------
--  Declaration
----------------------------------------------------------------------------*/

/**
**  Private wav data structure to handle wav streaming.
*/
struct WavData {
	CFile *WavFile;       /// Wav file handle
	int ChunkRem;         /// Bytes remaining in chunk
};

class CSampleWav : public CSample
{
public:
	~CSampleWav();
	int Read(void *buf, int len);

	WavData Data;
};

class CSampleWavStream : public CSample
{
public:
	~CSampleWavStream();
	int Read(void *buf, int len);

	WavData Data;
};

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

int CSampleWavStream::Read(void *buf, int len)
{
	WavChunk chunk;
	unsigned char *sndbuf;
	int comp; // number of compressed bytes actually read
	int i;
	int read;
	int bufrem;

	if (this->Pos > SOUND_BUFFER_SIZE / 2) {
		memcpy(this->Buffer, this->Buffer + this->Pos, this->Len);
		this->Pos = 0;
	}

	while (this->Len < SOUND_BUFFER_SIZE / 4) {
		if (!this->Data.ChunkRem) {
			// read next chunk
			comp = this->Data.WavFile->read(&chunk, sizeof(chunk));

			if (!comp) {
				// EOF
				this->Data.ChunkRem = 0;
				break;
			}

			chunk.Magic = ConvertLE32(chunk.Magic);
			chunk.Length = ConvertLE32(chunk.Length);
			if (chunk.Magic != DATA) {
				this->Data.WavFile->seek(chunk.Length, SEEK_CUR);
				continue;
			}
			this->Data.ChunkRem = chunk.Length;
		}

		bufrem = SOUND_BUFFER_SIZE - (this->Pos + this->Len);
		if (this->Data.ChunkRem > bufrem) {
			read = bufrem;
		} else {
			read = this->Data.ChunkRem;
		}
		this->Data.ChunkRem -= read;

		sndbuf = this->Buffer + this->Pos + this->Len;

		comp = this->Data.WavFile->read(sndbuf, read);
		if (!comp) {
			break;
		}

		read >>= 1;
		for (i = 0; i < read; ++i) {
			((unsigned short *)sndbuf)[i] = ConvertLE16(((unsigned short *)sndbuf)[i]);
		}

		this->Len += comp;
	}

	if (this->Len < len) {
		len = this->Len;
	}

	memcpy(buf, this->Buffer + this->Pos, len);
	this->Pos += len;
	this->Len -= len;

	return len;
}

CSampleWavStream::~CSampleWavStream()
{
	this->Data.WavFile->close();
	delete this->Data.WavFile;
	delete[] this->Buffer;
}

int CSampleWav::Read(void *buf, int len)
{
	if (len > this->Len) {
		len = this->Len;
	}

	memcpy(buf, this->Buffer + this->Pos, len);
	this->Pos += len;
	this->Len -= len;

	return len;
}

CSampleWav::~CSampleWav()
{
	delete[] this->Buffer;
}


/**
**  Load wav.
**
**  @param name   File name.
**  @param flags  Load flags.
**
**  @return       Returns the loaded sample.
**
**  @todo         Add ADPCM loading support!
*/
CSample *LoadWav(const std::string &name, int flags)
{
	CSample *sample;
	WavData *data;
	CFile *f;
	WavChunk chunk;
	WavFMT wavfmt;
	unsigned int t;

	f = new CFile;
	if (f->open(name.c_str(), CL_OPEN_READ) == -1) {
		printf("Can't open file `%s'\n", name.c_str());
		delete f;
		return NULL;
	}
	f->read(&chunk, sizeof(chunk));

	// Convert to native format

	chunk.Magic = ConvertLE32(chunk.Magic);
	chunk.Length = ConvertLE32(chunk.Length);

	if (chunk.Magic != RIFF) {
		f->close();
		delete f;
		return NULL;
	}

	f->read(&t, sizeof(t));
	t = ConvertLE32(t);
	if (t != WAVE) {
		printf("Wrong magic %x (not %x)\n", t, WAVE);
		f->close();
		delete f;
		return NULL;
	}

	f->read(&wavfmt, sizeof(wavfmt));

	// Convert to native format

	wavfmt.FMTchunk = ConvertLE32(wavfmt.FMTchunk);
	wavfmt.FMTlength = ConvertLE32(wavfmt.FMTlength);
	wavfmt.Encoding = ConvertLE16(wavfmt.Encoding);
	wavfmt.Channels = ConvertLE16(wavfmt.Channels);
	wavfmt.Frequency = ConvertLE32(wavfmt.Frequency);
	wavfmt.ByteRate = ConvertLE32(wavfmt.ByteRate);
	wavfmt.SampleSize = ConvertLE16(wavfmt.SampleSize);
	wavfmt.BitsPerSample = ConvertLE16(wavfmt.BitsPerSample);

	if (wavfmt.FMTchunk != FMT) {
		printf("Wrong magic %x (not %x)\n", wavfmt.FMTchunk, FMT);
		f->close();
		delete f;
		return NULL;
	}
	if (wavfmt.FMTlength != 16 && wavfmt.FMTlength != 18) {
		printf("Wrong length %d (not %d)\n", wavfmt.FMTlength, 16);
		f->close();
		delete f;
		return NULL;
	}

	if (wavfmt.FMTlength == 18) {
		if (f->read(&chunk, 2) != 2) {
			f->close();
			delete f;
			return NULL;
		}
	}

	//
	//  Check if supported
	//
	if (wavfmt.Encoding != WAV_PCM_CODE) {
		printf("Unsupported encoding %d\n", wavfmt.Encoding);
		f->close();
		delete f;
		return NULL;
	}
	if (wavfmt.Channels != WAV_MONO && wavfmt.Channels != WAV_STEREO) {
		printf("Unsupported channels %d\n", wavfmt.Channels);
		f->close();
		delete f;
		return NULL;
	}
	if (wavfmt.SampleSize != 1 && wavfmt.SampleSize != 2 && wavfmt.SampleSize != 4) {
		printf("Unsupported sample size %d\n", wavfmt.SampleSize);
		f->close();
		delete f;
		return NULL;
	}
	if (wavfmt.BitsPerSample != 8 && wavfmt.BitsPerSample != 16) {
		printf("Unsupported bits per sample %d\n", wavfmt.BitsPerSample);
		f->close();
		delete f;
		return NULL;
	}
	Assert(wavfmt.Frequency == 44100 || wavfmt.Frequency == 22050 ||
		wavfmt.Frequency == 11025);

	//
	//  Read sample
	//
	if (flags & PlayAudioStream) {
		sample = new CSampleWavStream;
		((CSampleWavStream *)sample)->Data.WavFile = f;
		data = &((CSampleWavStream *)sample)->Data;
	} else {
		sample = new CSampleWav;
		((CSampleWav *)sample)->Data.WavFile = f;
		data = &((CSampleWav *)sample)->Data;
	}
	sample->Channels = wavfmt.Channels;
	sample->SampleSize = wavfmt.SampleSize * 8 / sample->Channels;
	sample->Frequency = wavfmt.Frequency;
	sample->BitsPerSample = wavfmt.BitsPerSample;
	sample->Len = 0;
	sample->Pos = 0;

	if (flags & PlayAudioStream) {
		data->ChunkRem = 0;
		sample->Buffer = new unsigned char[SOUND_BUFFER_SIZE];
	} else {
		int comp; // number of compressed bytes actually read
		int i;
		int rem;
		int read;
		int bufrem;
		char sndbuf[SOUND_BUFFER_SIZE];

		sample->Buffer = NULL;
		read = 0;
		rem = 0;
		while (1) {
			if (!rem) {
				// read next chunk
				comp = f->read(&chunk, sizeof(chunk));

				if (!comp) {
					// EOF
					break;
				}

				chunk.Magic = ConvertLE32(chunk.Magic);
				chunk.Length = ConvertLE32(chunk.Length);
				if (chunk.Magic != DATA) {
					f->seek(chunk.Length, SEEK_CUR);
					continue;
				}
				rem = chunk.Length;
			}

			bufrem = SOUND_BUFFER_SIZE;
			if (rem > bufrem) {
				read = bufrem;
			} else {
				read = rem;
			}
			rem -= read;

			unsigned char *b = new unsigned char[sample->Len + read];
			Assert(b);
			memcpy(b, sample->Buffer, sample->Len);
			delete[] sample->Buffer;
			sample->Buffer = b;

			comp = data->WavFile->read(sndbuf, read);
			Assert(comp == read);

			if (sample->SampleSize == 16) {
				read >>= 1;
				for (i = 0; i < read; ++i) {
					((unsigned short *)(sample->Buffer + sample->Pos + sample->Len))[i] =
						ConvertLE16(((unsigned short *)sndbuf)[i]);
				}
			} else {
				memcpy((sample->Buffer + sample->Pos + sample->Len), sndbuf, comp);
			}

			sample->Len += comp;
		}

		data->WavFile->close();
		delete data->WavFile;
	}

	return sample;
}

//@}

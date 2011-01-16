/*
	Audio File Library
	Copyright (C) 1998-2000, Michael Pruett <michael@68k.org>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the
	Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	Boston, MA  02111-1307  USA.
*/

/*
	NEXT.cpp

	This file contains routines for parsing NeXT/Sun .snd format sound
	files.
*/

#include "config.h"
#include "NeXT.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Setup.h"
#include "Track.h"
#include "af_vfs.h"
#include "afinternal.h"
#include "audiofile.h"
#include "byteorder.h"
#include "util.h"

const int _af_next_compression_types[_AF_NEXT_NUM_COMPTYPES] =
{
	AF_COMPRESSION_G711_ULAW,
	AF_COMPRESSION_G711_ALAW
};

_AFfilesetup _af_next_default_filesetup =
{
	_AF_VALID_FILESETUP,	/* valid */
	AF_FILE_NEXTSND,	/* fileFormat */
	true,			/* trackSet */
	true,			/* instrumentSet */
	true,			/* miscellaneousSet */
	1,			/* trackCount */
	NULL,			/* tracks */
	0,			/* instrumentCount */
	NULL,			/* instruments */
	0,			/* miscellaneousCount */
	NULL			/* miscellaneous */
};

NeXTFile::NeXTFile()
{
	setFormatByteOrder(AF_BYTEORDER_BIGENDIAN);
}

status NeXTFile::readInit(AFfilesetup setup)
{
	uint32_t id, offset, length, encoding, sampleRate, channelCount;

	instruments = NULL;
	instrumentCount = 0;

	miscellaneous = NULL;
	miscellaneousCount = 0;

	tracks = NULL;	/* Allocate this later. */
	trackCount = 1;

	af_fseek(fh, 0, SEEK_SET);

	af_read(&id, 4, fh);
	assert(!memcmp(&id, ".snd", 4));

	readU32(&offset);
	readU32(&length);
	readU32(&encoding);
	readU32(&sampleRate);
	readU32(&channelCount);

#ifdef DEBUG
	printf("id, offset, length, encoding, sampleRate, channelCount:\n"
		" %d %d %d %d %d %d\n",
		id, offset, length, encoding, sampleRate, channelCount);
#endif

	Track *track = _af_track_new();
	if (!track)
		return AF_FAIL;

	tracks = track;

	track->f.byteOrder = AF_BYTEORDER_BIGENDIAN;

	/* Override the compression type later if necessary. */
	track->f.compressionType = AF_COMPRESSION_NONE;

	track->fpos_first_frame = offset;
	track->data_size = af_flength(fh) - offset;

	switch (encoding)
	{
		case _AU_FORMAT_MULAW_8:
			track->f.sampleWidth = 16;
			track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
			track->f.compressionType = AF_COMPRESSION_G711_ULAW;
			break;
		case _AU_FORMAT_ALAW_8:
			track->f.sampleWidth = 16;
			track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
			track->f.compressionType = AF_COMPRESSION_G711_ALAW;
			break;
		case _AU_FORMAT_LINEAR_8:
			track->f.sampleWidth = 8;
			track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
			break;
		case _AU_FORMAT_LINEAR_16:
			track->f.sampleWidth = 16;
			track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
			break;
		case _AU_FORMAT_LINEAR_24:
			track->f.sampleWidth = 24;
			track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
			break;
		case _AU_FORMAT_LINEAR_32:
			track->f.sampleWidth = 32;
			track->f.sampleFormat = AF_SAMPFMT_TWOSCOMP;
			break;
		case _AU_FORMAT_FLOAT:
			track->f.sampleWidth = 32;
			track->f.sampleFormat = AF_SAMPFMT_FLOAT;
			break;
		case _AU_FORMAT_DOUBLE:
			track->f.sampleWidth = 64;
			track->f.sampleFormat = AF_SAMPFMT_DOUBLE;
			break;

		default:
			/*
				This encoding method is not recognized.
			*/
			_af_error(AF_BAD_SAMPFMT, "bad sample format");
			return AF_FAIL;
	}

	_af_set_sample_format(&track->f, track->f.sampleFormat, track->f.sampleWidth);

	track->f.sampleRate = sampleRate;
	track->f.channelCount = channelCount;
	int frameSize = _af_format_frame_size(&track->f, false);
	track->totalfframes = length / frameSize;

#ifdef DEBUG
	printf("_af_next_read_init\n");
	_af_print_filehandle(file);
#endif

	/* The file has been parsed successfully. */
	return AF_SUCCEED;
}

bool NeXTFile::recognize(File *fh)
{
	uint8_t buffer[4];

	af_fseek(fh, 0, SEEK_SET);

	if (af_read(buffer, 4, fh) != 4 || memcmp(buffer, ".snd", 4) != 0)
		return false;

	return true;
}

AFfilesetup NeXTFile::completeSetup(AFfilesetup setup)
{
	if (setup->trackSet && setup->trackCount != 1)
	{
		_af_error(AF_BAD_NUMTRACKS, "NeXT files must have exactly 1 track");
		return AF_NULL_FILESETUP;
	}

	TrackSetup *track = setup->getTrack();
	if (!track)
		return AF_NULL_FILESETUP;

	if (track->f.sampleFormat == AF_SAMPFMT_UNSIGNED)
	{
		_af_error(AF_BAD_FILEFMT, "NeXT format does not support unsigned data");
		_af_set_sample_format(&track->f, AF_SAMPFMT_TWOSCOMP, track->f.sampleWidth);
	}

	if (track->f.sampleFormat == AF_SAMPFMT_TWOSCOMP)
	{
		if (track->f.sampleWidth != 8 &&
			track->f.sampleWidth != 16 &&
			track->f.sampleWidth != 24 &&
			track->f.sampleWidth != 32)
		{
			_af_error(AF_BAD_WIDTH, "invalid sample width %d for NeXT file (only 8-, 16-, 24-, and 32-bit data are allowed)");
			return AF_NULL_FILESETUP;
		}
	}

	if (track->f.compressionType != AF_COMPRESSION_NONE &&
		track->f.compressionType != AF_COMPRESSION_G711_ULAW &&
		track->f.compressionType != AF_COMPRESSION_G711_ALAW)
	{
		_af_error(AF_BAD_NOT_IMPLEMENTED, "compression format not implemented for NeXT files");
		return AF_NULL_FILESETUP;
	}

	if (track->f.byteOrder != AF_BYTEORDER_BIGENDIAN && track->byteOrderSet)
	{
		_af_error(AF_BAD_BYTEORDER, "NeXT format supports only big-endian data");
		track->f.byteOrder = AF_BYTEORDER_BIGENDIAN;
	}

	if (track->aesDataSet)
	{
		_af_error(AF_BAD_FILESETUP, "NeXT files cannot have AES data");
		return AF_NULL_FILESETUP;
	}

	if (track->markersSet && track->markerCount != 0)
	{
		_af_error(AF_BAD_FILESETUP, "NeXT format does not support markers");
		return AF_NULL_FILESETUP;
	}

	if (setup->instrumentSet && setup->instrumentCount != 0)
	{
		_af_error(AF_BAD_FILESETUP, "NeXT format does not support instruments");
		return AF_NULL_FILESETUP;
	}

	if (setup->miscellaneousSet && setup->miscellaneousCount != 0)
	{
		_af_error(AF_BAD_FILESETUP, "NeXT format does not support miscellaneous data");
		return AF_NULL_FILESETUP;
	}

	return _af_filesetup_copy(setup, &_af_next_default_filesetup, false);
}

// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#include "config.h"
#include "LocateUri.hxx"
#include "Print.hxx"
#include "PlaylistAny.hxx"
#include "PlaylistSong.hxx"
#include "SongEnumerator.hxx"
#include "SongPrint.hxx"
#include "song/DetachedSong.hxx"
#include "fs/Traits.hxx"
#include "thread/Mutex.hxx"
#include "Partition.hxx"
#include "Instance.hxx"

static void
playlist_provider_print(Response &r,
			const SongLoader &loader,
			const char *uri,
			SongEnumerator &e, bool detail) noexcept
{
	const auto base_uri = uri != nullptr
		? PathTraitsUTF8::GetParent(uri)
		: ".";

	std::unique_ptr<DetachedSong> song;
	while ((song = e.NextSong()) != nullptr) {
		if (playlist_check_translate_song(*song, base_uri,
						  loader) &&
		    detail)
			song_print_info(r, *song);
		else
			/* fallback if no detail was requested or no
			   detail was available */
			song_print_uri(r, *song);
	}
}

bool
playlist_file_print(Response &r, Partition &partition,
		    const SongLoader &loader,
		    const LocatedUri &uri, bool detail)
{
	Mutex mutex;

#ifndef ENABLE_DATABASE
	(void)partition;
#endif

	auto playlist = playlist_open_any(uri,
#ifdef ENABLE_DATABASE
					  partition.instance.storage,
#endif
					  mutex);
	if (playlist == nullptr)
		return false;

	playlist_provider_print(r, loader, uri.canonical_uri, *playlist, detail);
	return true;
}

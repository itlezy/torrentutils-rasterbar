#include <cstdlib>

#include <windows.h>
#include <fileapi.h>

#include <spdlog/spdlog.h>

#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/torrent_info.hpp>

#include <iostream>

int main(int argc, char* argv[]) try
{
	spdlog::info("Laudem!!");

	lt::session s;
	lt::add_torrent_params p;
	lt::session_params sp;

	sp.settings.set_bool(lt::settings_pack::enable_dht, true);
	sp.settings.set_int(lt::settings_pack::alert_mask, 0x7fffffff);
	sp.settings.set_str(lt::settings_pack::dht_bootstrap_nodes, "dht.libtorrent.org:25401,router.bittorrent.com:6881,router.utorrent.com:6881,dht.transmissionbt.com:6881,dht.aelitis.com:6881");

	p.flags |= lt::torrent_flags::upload_mode;
	p.flags |= lt::torrent_flags::stop_when_ready;

	p.save_path = ".";
	p.ti = std::make_shared<lt::torrent_info>(argv[1]);
	s.add_torrent(p);

	spdlog::info("Torrent added {}", argv[1]);

	// wait for the user to end
	char a;
	int ret = std::scanf("%c\n", &a);
	(void)ret; // ignore

	return 0;
}
catch (std::exception const& e) {
	std::cerr << "ERROR: " << e.what() << "\n";
}

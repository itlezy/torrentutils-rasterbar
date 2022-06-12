#include <cstdlib>

#include <iostream>
#include <thread>
#include <chrono>

#include <libtorrent/session.hpp>
#include <libtorrent/session_params.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/write_resume_data.hpp>

int main(int argc, char const* argv[]) try
{
	if (argc != 2) {
		std::cerr << "usage: " << argv[0] << " <magnet-url>" << std::endl;
		return 1;
	}

	lt::settings_pack p;

	p.set_bool(lt::settings_pack::enable_dht, true);

	p.set_bool(lt::settings_pack::enable_upnp, true);
	p.set_bool(lt::settings_pack::enable_natpmp, true);

	p.set_int(lt::settings_pack::alert_mask, lt::alert_category::status
		| lt::alert_category::error);

	p.set_str(lt::settings_pack::dht_bootstrap_nodes, "dht.libtorrent.org:25401,router.bittorrent.com:6881,router.utorrent.com:6881,dht.transmissionbt.com:6881,dht.aelitis.com:6881");

	lt::session ses(p);

	lt::add_torrent_params atp = lt::parse_magnet_uri(argv[1]);
	atp.save_path = "."; // save in current dir
	//atp.flags |= lt::torrent_flags::upload_mode;
	//atp.flags |= lt::torrent_flags::stop_when_ready;

	lt::torrent_handle h = ses.add_torrent(std::move(atp));

	h.set_flags(lt::torrent_flags::upload_mode);
	h.unset_flags(lt::torrent_flags::auto_managed);

	for (;;) {
		std::vector<lt::alert*> alerts;
		ses.pop_alerts(&alerts);

		for (lt::alert const* a : alerts) {
			std::cout << a->message() << std::endl;

			// if we receive the finished alert or an error, we're done
			if (lt::alert_cast<lt::metadata_received_alert>(a)) {
				const lt::metadata_received_alert* p = lt::alert_cast<lt::metadata_received_alert>(a);

				//const lt::metadata_received_alert* p = static_cast<const lt::metadata_received_alert*>(a)

				lt::torrent_handle h = p->handle;
				h.save_resume_data(lt::torrent_handle::save_info_dict);

				//const auto torrentCreator = lt::create_torrent(*m_nativeInfo);
				//const lt::entry torrentEntry = torrentCreator.generate();
				//const nonstd::expected<void, QString> result = Utils::IO::saveToFile(path, torrentEntry);

				std::cout << "metadata received!, saving resume data..";
				goto done;
			}

			if (lt::alert_cast<lt::torrent_finished_alert>(a)) {
				std::cout << "torrent done!";
				goto done;
			}

			if (lt::alert_cast<lt::torrent_error_alert>(a)) {
				goto done;
			}

		}
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
done:
	std::cout << "done, shutting down" << std::endl;
	Sleep(12 * 1000);
}
catch (std::exception& e)
{
	std::cerr << "Error: " << e.what() << std::endl;
}


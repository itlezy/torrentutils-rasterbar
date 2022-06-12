#include <cstdlib>

#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>

#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/session_params.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/create_torrent.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/torrent_status.hpp>
#include <libtorrent/read_resume_data.hpp>
#include <libtorrent/write_resume_data.hpp>
#include <libtorrent/error_code.hpp>
#include <libtorrent/magnet_uri.hpp>

#include <spdlog/spdlog.h>

namespace {

	using clk = std::chrono::steady_clock;

	// return the name of a torrent status enum
	char const* state(lt::torrent_status::state_t s)
	{
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#endif
		switch (s) {
		case lt::torrent_status::checking_files: return "checking";
		case lt::torrent_status::downloading_metadata: return "dl metadata";
		case lt::torrent_status::downloading: return "downloading";
		case lt::torrent_status::finished: return "finished";
		case lt::torrent_status::seeding: return "seeding";
		case lt::torrent_status::checking_resume_data: return "checking resume";
		default: return "<>";
		}
#ifdef __clang__
#pragma clang diagnostic pop
#endif
	}

} // anonymous namespace

int main(int argc, char const* argv[]) try
{
	spdlog::info("Laudem!!");

	if (argc != 2) {
		std::cerr << "usage: " << argv[0] << " <magnet-url>" << std::endl;
		return 1;
	}

	lt::settings_pack pack;
	pack.set_int(lt::settings_pack::alert_mask
		, lt::alert_category::error
		| lt::alert_category::storage
		| lt::alert_category::status);

	pack.set_bool(lt::settings_pack::enable_dht, true);
	pack.set_bool(lt::settings_pack::enable_upnp, true);
	pack.set_bool(lt::settings_pack::enable_natpmp, true);
	pack.set_str(lt::settings_pack::dht_bootstrap_nodes, "dht.libtorrent.org:25401,router.bittorrent.com:6881,router.utorrent.com:6881,dht.transmissionbt.com:6881,dht.aelitis.com:6881");

	lt::session ses(pack);
	clk::time_point last_save_resume = clk::now();

	std::string hashes[] = {
		
	};


	for (std::string value : hashes) {
		spdlog::info("Adding torrent {}", value);

		lt::add_torrent_params add_tor_params = lt::parse_magnet_uri(value);
		//magnet.flags |= lt::torrent_flags::upload_mode;
		add_tor_params.save_path = "."; // save in current dir

		ses.async_add_torrent(std::move(add_tor_params));

		// this is the handle we'll set once we get the notification of it being
		// added
		lt::torrent_handle h;
	}


	// set when we're exiting
	bool done = false;

	for (;;) {
		std::vector<lt::alert*> alerts;
		ses.pop_alerts(&alerts);

		for (lt::alert const* a : alerts) {
			//std::cout << a->message() << std::endl;
			spdlog::info(a->message());

			if (auto at = lt::alert_cast<lt::add_torrent_alert>(a)) {
				//h = at->handle;

				//h.set_flags(lt::torrent_flags::upload_mode);
				//h.unset_flags(lt::torrent_flags::auto_managed);
			}

			// if we receive the finished alert or an error, we're done
			if (lt::alert_cast<lt::metadata_received_alert>(a)) {
				const lt::metadata_received_alert* p = lt::alert_cast<lt::metadata_received_alert>(a);

				//const lt::torrent_info torrentInfo = *p->handle.torrent_file();
				const std::shared_ptr<lt::torrent_info> torrentInfoSh = std::make_shared<lt::torrent_info>(*p->handle.torrent_file());

				if (torrentInfoSh != nullptr) {
					spdlog::info("torrentInfo gut");
				}

				const std::shared_ptr<lt::torrent_info> completeTorrentInfo = p->handle.torrent_file_with_hashes();

				if (completeTorrentInfo != nullptr) {
					spdlog::info("completeTorrentInfo gut");
				}

				const auto torrentCreator = lt::create_torrent((completeTorrentInfo != nullptr) ? *completeTorrentInfo : *torrentInfoSh);

				const lt::entry torrentEntry = torrentCreator.generate();

				std::vector<char> buffer;
				lt::bencode(std::back_inserter(buffer), torrentEntry);

				std::ofstream outfile("M_" + (p->handle.status().name) + ".torrent", std::ios::out | std::ios::binary);

				outfile.write(&buffer[0], buffer.size());

				outfile.flush();
				outfile.close();

				spdlog::info("metadata received, torrent file saved!");

				//h.save_resume_data(lt::torrent_handle::save_info_dict);
				ses.remove_torrent(p->handle, lt::session::delete_files);

				//goto done;
			}

			// when resume data is ready, save it
			if (auto rd = lt::alert_cast<lt::save_resume_data_alert>(a)) {
				std::ofstream of(".resume_file", std::ios_base::binary);
				of.unsetf(std::ios_base::skipws);
				auto const b = write_resume_data_buf(rd->params);
				of.write(b.data(), int(b.size()));

				if (done) goto done;
			}

			if (lt::alert_cast<lt::save_resume_data_failed_alert>(a)) {
				std::cout << "Save resume data failed!!";
				if (done) goto done;
			}

			if (auto st = lt::alert_cast<lt::state_update_alert>(a)) {

			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1500));

		// ask the session to post a state_update_alert, to update our
		// state output for the torrent
		ses.post_torrent_updates();
	}

done:
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	spdlog::info("done, exiting");
	std::this_thread::sleep_for(std::chrono::milliseconds(3000));
}
catch (std::exception& e)
{
	spdlog::error("Error {}", e.what());
}


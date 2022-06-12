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
		"magnet:?xt=urn:btih:8670eca1418ffc10d9f5623347f0967983386218",
"magnet:?xt=urn:btih:8670f0be93477579c5d4f15adddb582785815416",
"magnet:?xt=urn:btih:8670f0d289d89cf212f1e05d3d43d51fb07d6798",
"magnet:?xt=urn:btih:8670f3cb102c027bea7d31f5b991f3cfb18c8664",
"magnet:?xt=urn:btih:8670f48f9101d10a17b4dcc1be65481a5e6dbb92",
"magnet:?xt=urn:btih:8670f5c4894ad3969ffa857735e3caeb222e5938",
"magnet:?xt=urn:btih:8670f66790b42f4c98b4c108c834fffb6a089edf",
"magnet:?xt=urn:btih:8670f739459134d4bfe09d68dbfcc07dc60888be",
"magnet:?xt=urn:btih:8670f78a2c481a7ca30aa1f95f18487d63887763",
"magnet:?xt=urn:btih:8670f93038005530df1cb2d7ed46a51420619112",
"magnet:?xt=urn:btih:8670f9d2cee4011b0b7a07f1cc330db40c1ea621",
"magnet:?xt=urn:btih:8670fa2e61e23005c12a6050f15004787941e49e",
"magnet:?xt=urn:btih:8670fc7e05fded240af90472be2ca308c7078a9a",
"magnet:?xt=urn:btih:8670fcc197db391536a2fe5cc0f8af12f8e0f38f",
"magnet:?xt=urn:btih:8670fd18af9487fedbf877c4b93d52fd4b028f0b",
"magnet:?xt=urn:btih:8670fda8e7035f7095cf87c42a97971c8bff1aad",
"magnet:?xt=urn:btih:b16a74239c2cf59cf0f1cd3ce73eb4dcb3ac232e",
"magnet:?xt=urn:btih:b9433b26a89578270e2926fe3005d4c40e45e31b",
"magnet:?xt=urn:btih:b944f5f2db78f821bf0df2e5fdca0409a772bdbb",
"magnet:?xt=urn:btih:b9484d4fad0b507441f0ada8942a87984632ea80",
"magnet:?xt=urn:btih:b9498a7fbcbc1e1f8e454b04ff83c49493dd5b89",
"magnet:?xt=urn:btih:b94ff651a73aacd501e2f07c5f870265fc09c29f",
"magnet:?xt=urn:btih:b95413c2ef3161497135fb9e85e51ef04af0f716",
"magnet:?xt=urn:btih:b9544d540916873a0ac3040dbf277b1ad7c60589",
"magnet:?xt=urn:btih:b9546231bf1300697e149c79b47ffa5067378502",
"magnet:?xt=urn:btih:b95468067810ad441c9f8e4869c38749e4cafaa8",
"magnet:?xt=urn:btih:b9546b2167d8d0a4432a86e218960dcd1c9eb76e",
"magnet:?xt=urn:btih:b95477d900c1b56095008961b42bd79bc9892216",
"magnet:?xt=urn:btih:b954dcca260ce199264b55887f5ada54f43797d6",
"magnet:?xt=urn:btih:b95702251e8ca5c96685e9a987bd340a16c25d18",
"magnet:?xt=urn:btih:b957df602242c8eace6f00807440cb3969773763",
"magnet:?xt=urn:btih:b95b4051a53fe76dbebf0f23cb55ae392e349328",
"magnet:?xt=urn:btih:b95cfd0f4b5aea820312c0ba9853ec3fed44765a",
"magnet:?xt=urn:btih:b95d75e27db589c18c10fcfce8003b6d5825357c",
"magnet:?xt=urn:btih:b9a50e5ff23ca02c4bf4bae4ed13f9d2db87faf6",
"magnet:?xt=urn:btih:b9b757ee76b21e5fd960526e4adea81933370301",
"magnet:?xt=urn:btih:989942b385c881148f9f2151f99e5fa6c19c3a33",
"magnet:?xt=urn:btih:9899434772c23763c4458bb890b73fb5e1006078",
"magnet:?xt=urn:btih:989944fa0502c32206d904e2a5a44dd79cbfafcd",
"magnet:?xt=urn:btih:9899464856f527d3bbb476ff69072e39d65c8452",
"magnet:?xt=urn:btih:98994671946e549e4476d3fd18cdf2584f5223c0",
"magnet:?xt=urn:btih:9899496da05666048839486a023d436a3024c829",
"magnet:?xt=urn:btih:98994b7d12966d6515617f4c4196b473f190f0b2",
"magnet:?xt=urn:btih:98994b9a1807e15861ee8194964b09e616317782",
"magnet:?xt=urn:btih:98994c64917c99c573a98bec535e2a187c82ef3c",
"magnet:?xt=urn:btih:98994ceadac09fdf341f8f3955b9a80b5d7c17e3",
"magnet:?xt=urn:btih:98994e5c7c98972ce87c79c1819dd48f3e3020cb",
"magnet:?xt=urn:btih:98994f163aa6b6473d3792f207ae3042fd684b76",
"magnet:?xt=urn:btih:9899525f28cb5eec529186f3142607d17353e794",
"magnet:?xt=urn:btih:989952e2c12185c2beabacf7ac20531a4113c5fd",
"magnet:?xt=urn:btih:9899537343d1b007a4d91e94d61b9df2b2d76063",
"magnet:?xt=urn:btih:989953e62a56f0126e294e7c0a5584d3d6eb5efd",
"magnet:?xt=urn:btih:9899635e2ae027175ac33ba7eb121fa4a238ed99",
"magnet:?xt=urn:btih:98996fc2c744a019a56ea576fc5235327c9e21d2",
"magnet:?xt=urn:btih:98997569ad20b5ef6e8eca51ef9220f6bf89264b",
"magnet:?xt=urn:btih:98997e38430b5b0a83676bb7a5735925b376fd83",
"magnet:?xt=urn:btih:8606189b94035292d97397476d883c0915b7601c",
"magnet:?xt=urn:btih:862f028e7f4a00dd1a81b5a27fd84897b9599caf",
"magnet:?xt=urn:btih:866883547d571a44804f1b9fd2112ac91fd7e7d8",
"magnet:?xt=urn:btih:86706259c82ce17aeda273b68f9073ee06297154",
"magnet:?xt=urn:btih:8670a88ea06e3465e15366bfe57269230e618f9f",
"magnet:?xt=urn:btih:8670a946a72b068aa9c252885310b75abcca09b1",
"magnet:?xt=urn:btih:8670c2de1bc5ff38d70dc8215ed4e24a63a4fee8",
"magnet:?xt=urn:btih:8670c5120a7da0784a3dfe5332426bb75e6eb7fe",
"magnet:?xt=urn:btih:8670cdb2e325c7aaf622682eaf6e498c1325021d",
"magnet:?xt=urn:btih:8670e46163f3bdb3afcdf2cee740abb8331dfb29",
"magnet:?xt=urn:btih:8670e923f1277b8b9f84735f6a279800ae3e1e70",
"magnet:?xt=urn:btih:8670f69dcb4cc866c76077a8af67076f45f0724a",
"magnet:?xt=urn:btih:8670f9f10e11b27b2f258de7a3eb96db9e7f50c7",
"magnet:?xt=urn:btih:8671fe978d6e68dbc5e1f2c0663dddb033cd954b",
"magnet:?xt=urn:btih:867283dd98e71c62b3fe960b47c913abaa82f5ad",
"magnet:?xt=urn:btih:867dfc627e32f2ccfc6b7c2a210fd6d41b030257",
"magnet:?xt=urn:btih:867f5f9e2ef464483d73ed059b1c0fe68fa5c633",
"magnet:?xt=urn:btih:b8e92fbf6095f9c2976734dc63ee13552a96aa11",
"magnet:?xt=urn:btih:b949c00c056fbdf0870535ae0270aadd595be782",
"magnet:?xt=urn:btih:b953370d3b0ce9957c5eff6f64c68581666a37eb",
"magnet:?xt=urn:btih:b959021b98e7017f0e3e15ed059366c2cf33e13c",
"magnet:?xt=urn:btih:b95e40335329082a4d42015fab5e78af7ef1ecf1",
"magnet:?xt=urn:btih:b95e43611d75be8408ec17f06b3d67107ddf26f8",
"magnet:?xt=urn:btih:b95e4d797f3b1dec657c5933dda3ffd779de6de1",
"magnet:?xt=urn:btih:b95e564274d1f97289ff87add15a7434c8d8eb93",
"magnet:?xt=urn:btih:b95e5b6f56f11916b8f4f4d341ce7c193b86f483",
"magnet:?xt=urn:btih:b95e5e6849cfca286c38c9cada69b641b470801d",
"magnet:?xt=urn:btih:b95e6b672fbbf083707443259d152ff542a2e4fe",
"magnet:?xt=urn:btih:b95e76fb88a1a20138fef7c4fc66b6b27c328e36",
"magnet:?xt=urn:btih:b95e9daacc7b3641b93394d9f427dd88c267e3d1",
"magnet:?xt=urn:btih:b95ed03b620d621fd09426ad5a608ac8d831df51",
"magnet:?xt=urn:btih:b95ed32de0ae0eb94ec49459d18941b68fefb367",
"magnet:?xt=urn:btih:b95f0f2e366c8d133c9320042979e432a191e675",
"magnet:?xt=urn:btih:b95f77a02a7405c3dac78cfc1ff7d34d287ec2a8",
"magnet:?xt=urn:btih:b95f8529041d39ee0e0de87b2589a803265f0b3c",
"magnet:?xt=urn:btih:b95fbc8c5f4bbf2f471919ad02745f2c6f36aa29",
"magnet:?xt=urn:btih:804ce77dd1afc76eda347dfa62f8a8f9e79a702e",
"magnet:?xt=urn:btih:8610f5938237a7a56cb0eb08b059f2c2c0a2e6cc",
"magnet:?xt=urn:btih:866349493fd65155abc37a2bf3ffd6b9d2ebd7c3",
"magnet:?xt=urn:btih:8663cceef5057e92c2e109799dbb3298e69ba6cd",
"magnet:?xt=urn:btih:866a910f1f3201bfb52e368eec2514ba608f3412",
"magnet:?xt=urn:btih:867008bc242556ba54d5d8668c70708714a7a985",
"magnet:?xt=urn:btih:86709869b7d7f34f7b7f8e2dfb00e0700b4bc9c9",
"magnet:?xt=urn:btih:8670b5d45dc5c5d6c593a7e8d9ab1f8eefc1d53e",
"magnet:?xt=urn:btih:8670e10756c01bb70209c1a40fc543419bfae514",
"magnet:?xt=urn:btih:8670e2dc4c52cfb6b9e395d532f337261de8a382",
"magnet:?xt=urn:btih:8670e3d129a995104939a9e37783ce45e725d5af",
"magnet:?xt=urn:btih:8670f40c3a2e73633b1fa3beffa08cbad151b2de",
"magnet:?xt=urn:btih:8670f4134087004045599584125f972381e566ec",
"magnet:?xt=urn:btih:867393a0f4fc1cf8e55ca0638bc77a3000c1731e",
"magnet:?xt=urn:btih:867e0afd270fc29c80e9176428783fd059d79c76",
"magnet:?xt=urn:btih:856228d0d635ada6295035f9405c1d2c3e238743",
"magnet:?xt=urn:btih:908b53cc37bbc36b317c61f974d19e4d6a09bc26",
"magnet:?xt=urn:btih:916929d7e66c03f9696dd4b9129218ebb0cd2369",
"magnet:?xt=urn:btih:929cf3c6965c8d07673b903599e671527aa0458d",
"magnet:?xt=urn:btih:9535088a8e72baee8b2aef33916be44087270c02",
"magnet:?xt=urn:btih:95444b616b6778b4f5aa154e1077fefd4760407d",
"magnet:?xt=urn:btih:9559269be51c75111e28a253af99a1a8be464a92"
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


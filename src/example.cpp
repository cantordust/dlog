#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <fstream>
#include <iomanip>
#include <ctime>
#include "dlog.hpp"

static std::mt19937_64 rng;
static std::uniform_int_distribution<uint> d_sleep(100, 5000);
static std::uniform_int_distribution<uint> d_actions(0, 20);

using namespace Async;

static inline std::string get_prefix(const uint _log_level)
{
	std::string prefix;
	switch (_log_level)
	{
		case 1:
			prefix = "(Debug)";
			break;

		case 2:
			prefix = "(Info)";
			break;

		case 3:
			prefix = "(Warn)";
			break;

		case 4:
			prefix = "(Error)";
			break;

		case 5:
			prefix = "(Critical)";
			break;

		default:
			prefix = "";
			break;
	}
	return std::string(prefix + std::string((12 - prefix.length()), ' '));
}

static inline std::string get_infix(const uint _log_level)
{
	return "";
}

static inline std::string get_suffix(const uint _log_level)
{
	return "";
}

std::string time()
{
	std::stringstream tm;
	std::time_t cur_time(std::time(nullptr));
	tm << std::put_time(std::localtime(&cur_time), "%c %Z");
	return tm.str();
}

uint uint_void()
{
	uint sleep(d_sleep(rng));
	dlog() << "\tuint_void sleeping for " << sleep << " ms";
	std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
	dlog() << "\tuint_void slept for " << sleep << " ms";
	return sleep;
}

void void_uint(const uint _val)
{
	dlog() << "\tvoid_uint sleeping for " << _val << " ms";
	std::this_thread::sleep_for(std::chrono::milliseconds(_val));
	dlog() << "\tvoid_uint slept for " << _val << " ms";
}

int main()
{
	rng.seed(static_cast<uint>(std::chrono::high_resolution_clock().now().time_since_epoch().count()));

	// Set the minimal log level
	dlog::set_default_log_level(3);

	uint threads(5);
	bool log_file_exists(false);
	std::string log_file_name("test.log");

	// Declare streams as static to prevent
	// them from disappearing when main() exits
	static std::ofstream log_file(log_file_name, std::ios::out | (log_file_exists ? std::ios::app : std::ios::trunc));
	log_file_exists = true;

	uint worker(0);

	// Output a header to the log file, all parameters set
	dlog(0, log_file, "### ", " ", " ###\n") << "Log started on " << time()
											 << " in thread " << std::this_thread::get_id();

	std::vector<std::thread> workers;
	for (uint i = 0; i < threads; ++i)
	{
		workers.emplace_back([&]
		{
			// Worker ID and log level
			uint w(++worker);

			// Output to std::cout only above a certain log level
			dlog(w) << "--> Worker " << w << " created in thread "
					<< std::this_thread::get_id();

			while(true)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(d_sleep(rng)));

				// Output to a file
				dlog(w, log_file) << "(" << time() << ")\t"
								  << " Message from worker " << w << " in thread "
								  << std::this_thread::get_id();
				uint action(d_actions(rng));
				if (action == 0)
				{
					// Always output if log level is not specified
					dlog() << "--> Worker " << w << " in thread "
						   << std::this_thread::get_id() << " exiting";
					break;
				}
				else if (action % 2 == 0)
				{
					// It is possible to print futures without blocking
					// by passing the future rather than future.get()
					auto future = std::async(std::launch::async, uint_void);
					dlog() << "Future value from uint_void: " << future;
				}
				else if (action % 3 == 0)
				{
					// Void futures can also be handled
					auto future = std::async(std::launch::async, void_uint, d_sleep(rng));
					dlog() << "Future returned from void_uint" << future;
				}
				else
				{
					// Multiple futures can be printed together
					auto future1 = std::async(std::launch::async, uint_void);
					auto future2 = std::async(std::launch::async, uint_void);
					auto future3 = std::async(std::launch::async, uint_void);

					dlog() << "Multiple futures:"
						   << "\n\tfuture 1: " << future1
						   << "\n\tfuture 2: " << future2
						   << "\n\tfuture 3: " << future3;
				}
			}
		});
	}

	for (auto& worker : workers)
	{
		worker.join();
	}

	// Output a footer to the log file, all parameters set
	dlog(0, log_file, "\n### ", " ", " ###") << "Log ended on " << time();

	dlog() << "*** Calling dlog() from main() ***";

	// We can also call dlog from the main thread
	uint rock(0);
	for (uint i = 1; i <= 30; ++i)
	{
		uint action(d_actions(rng));
		if (action % 2 == 0)
		{
			dlog() << "iteration " << i << ": action 1";
			// It is possible to print futures without blocking
			// by passing the future rather than future.get()
			auto future = std::async(std::launch::async, uint_void);
			dlog() << "(iteration " << i << ") Future value from uint_void: " << future;
		}
		else if (action % 3 == 0)
		{
			dlog() << "iteration " << i << ": action 2";
			// Void futures can also be handled
			auto future = std::async(std::launch::async, void_uint, d_sleep(rng));
			dlog() << "(iteration " << i << ") Future returned from void_uint" << future;
		}
		else
		{
			dlog() << "iteration " << i << ": action 3";

			// Multiple futures can be printed together
			auto future1 = std::async(std::launch::async, uint_void);

			auto future2 = std::async(std::launch::async, void_uint, d_sleep(rng));

			auto future3 = std::async(std::launch::async, [&]{ dlog() << "#### Lambdas rock " << ++rock << " times!"; });

			sp<std::packaged_task<uint()>> pt(std::make_shared<std::packaged_task<uint()>>(uint_void));
			auto future4 = pt->get_future();
			std::thread([spt = pt]
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(5000));
				(*spt)();
			}).detach();

			sp<std::promise<uint>> pm(std::make_shared<std::promise<uint>>());
			auto future5 = pm->get_future();
			std::thread([spm = pm]
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10000));
				spm->set_value(uint_void());
			}).detach();

			dlog() << "(iteration " << i << ") Multiple futures:"
				   << "\n\tfuture 1: " << future1
				   << "\n\tfuture 2: " << future2
				   << "\n\tfuture 3: " << future3
				   << "\n\tfuture 4: " << future4
				   << "\n\tfuture 5: " << future5;
		}
	}

	dlog() << "*** Exiting main...";

	return 0;
}

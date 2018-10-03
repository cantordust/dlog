#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <fstream>
#include <ctime>
#include "dlog.hpp"

///=============================================================================
///	Current time.
///=============================================================================

std::string time()
{
	std::stringstream tm;
	std::time_t cur_time(std::time(nullptr));
	tm << std::put_time(std::localtime(&cur_time), "%c %Z");
	return tm.str();
}

///=============================================================================
///	Random number generator and number distributions.
///=============================================================================

static std::mt19937_64 rng;
static std::uniform_int_distribution<uint> sleep_dist(100, 1500);
static std::uniform_int_distribution<uint> level_dist(1, 4);
static std::uniform_int_distribution<uint> action_dist(0, 3);

using namespace Async;

///=============================================================================
///	Log levels, affix set generator and log level die.
///=============================================================================

enum class LogLevel : uint
{
	Log = 0,
	Info,
	Warn,
	Error,
	Critical
};

AffixSet afx(const LogLevel _level)
{
	switch (_level)
	{
	case LogLevel::Log:
		return {0, "(0) [Log     ][" + time() + "] ", " - "};

	case LogLevel::Info:
		return {1, "(1) [Info    ][" + time() + "] ", " / "};

	case LogLevel::Warn:
		return {2, "(2) [Warn    ][" + time() + "] ", " | "};

	case LogLevel::Error:
		return {3, "(3) [Error   ][" + time() + "] ", " \\ "};

	case LogLevel::Critical:
		return {4, "(4) [Critical][" + time() + "] ", " - "};

	default:
		return AffixSet();
	}
}

LogLevel rnd_level()
{
	return static_cast<LogLevel>(level_dist(rng));
}

///=============================================================================
///	Global variables.
///=============================================================================

/// Default log level.
/// No output will be produced if the log level supplied to
/// the log() functions above is lower than this.
LogLevel log_level(LogLevel::Error);

/// Number of threads in the threadpool.
uint threads(3);

/// Number of records to generate.
uint records(100);

///=============================================================================
///	Test functions.
/// @todo: More functions.
///=============================================================================

template<LogLevel level = LogLevel::Log>
uint uint_void()
{
	uint sleep(sleep_dist(rng));
	dlog("\tuint_void sleeping for", sleep, "ms");
	std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
	dlog("\tuint_void slept for", sleep, "ms");
	return sleep;
}

template<LogLevel level = LogLevel::Log>
void void_uint(const uint _val)
{
	dlog("\tvoid_uint sleeping for", _val, "ms");
	std::this_thread::sleep_for(std::chrono::milliseconds(_val));
	dlog("\tvoid_uint slept for", _val, "ms");
}

void act()
{
	static std::atomic<uint> rock(0);
	switch (action_dist(rng))
	{
	case 0:
		uint_void();
		break;

	case 1:
		void_uint(sleep_dist(rng));
		break;

	default:
		std::async(std::launch::async, [&]{ dlog("#### Lambdas rock", ++rock, "times!"); });
	}
}

struct Test
{
	uint id = level_dist(rng);

	friend std::ostream& operator << (std::ostream& _os, const Test& _test)
	{
		return _os << "Test id: " + std::to_string(_test.id);
	}
};

///=============================================================================
///	Main event
///=============================================================================

int main()
{
	rng.seed(static_cast<uint>(std::chrono::high_resolution_clock().now().time_since_epoch().count()));

	/// Set the log level.
	dlog::set_log_level(static_cast<uint>(log_level));

	/// Log file
	bool log_file_exists(false);
	std::string log_file_name("test.log");

	///=====================================
	/// Formatting options
	///=====================================

	{
		Test t;
		dlog d("Formatting test:\n");
		d.left().setfill(' ');
		d.format(t, 20) + "|\n";
		d.format("Another string", 20) + "|" << std::endl;
	}

	///=====================================
	/// Multi-threaded output
	///=====================================

	// Declare streams as static to prevent
	// them from disappearing when main() exits
	std::shared_ptr<std::ofstream> log_file(std::make_shared<std::ofstream>(log_file_name, std::ios::out | (log_file_exists ? std::ios::app : std::ios::trunc)));
	log_file_exists = true;

	uint worker(0);

	// Output a header to the log file
	dlog(log_file, "###", time(), ": start of log in thread", std::this_thread::get_id(), "###");

	std::vector<std::thread> workers;
	for (uint i = 0; i < threads; ++i)
	{
		workers.emplace_back([&]
		{
			// Worker ID and log level
			uint w(++worker);

			// Output to std::cout only above a certain log level
			dlog(">>> Worker", w, "created in thread", std::this_thread::get_id());

			for (uint r = 0; r < records; ++r)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(sleep_dist(rng)));

				/// Output to std::cout.
				dlog(afx(rnd_level()), "\tMessage from worker", w, "in thread", std::this_thread::get_id());

				// Output to a file.
				dlog(log_file, afx(rnd_level()), "\tMessage from worker", w, "in thread", std::this_thread::get_id());
			}
		});
	}

	for (auto& worker : workers)
	{
		worker.join();
	}

	// Output a footer to the log file, all parameters set
	dlog(log_file, "\n###",  time(), ": end of log ###");

	dlog("*** Calling dlog from main() ***");

	// We can also call dlog from the main thread
	for (uint r = 1; r <= records; ++r)
	{
		dlog("Record", r);
		act();
	}

	dlog("*** Exiting main...");

	return 0;
}

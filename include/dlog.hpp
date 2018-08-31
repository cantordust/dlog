#ifndef DLOG_HPP
#define DLOG_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <future>
#include <queue>
#include <mutex>

#include "threadpool.hpp"

namespace Async
{
	template<typename T1, typename T2>
	using hmap = std::unordered_map<T1, T2>;

	/// Set of strings affixed to the input
	/// at various positions.
	struct AffixSet
	{
		/// Default values
		struct Default
		{
			inline static uint log_level{0};
			inline static std::string prefix{""};
			inline static std::string infix{" "};
			inline static std::string suffix{"\n"};
		};

		uint log_level{Default::log_level};
		std::string prefix{Default::prefix};
		std::string infix{Default::infix};
		std::string suffix{Default::suffix};
	};

	/// @class The dlog class.
	/// @details
	/// dlog ("debug log") is a tiny header-only library
	/// for printing output from multiple threads to
	/// arbitrary streams without garbling the output.
	class dlog
	{

	public:

		/// Version string.
		inline static const std::string version{"0.2.1"};

		/// Default log level.
		inline static uint log_level{0};

	private:

		/// Threadpool for multi-threaded output.
		inline static ThreadPool tp{std::thread::hardware_concurrency()};

		/// Mutex for accessing the print queue.
		inline static std::mutex semaphore_mutex;

		/// Pointers to output streams and
		/// corresponding mutexes.
		inline static hmap<std::ostream*, std::mutex> semaphores;

		bool out{true};

		/// Strings appended to the input.
		/// If not set in the constructor, these
		/// are given the default values.
		AffixSet afs;

		/// Stream associated with this log.
		std::ostream& stream;

		/// Buffer for storing the output.
		std::stringstream buffer;

	public:

		template<typename ... Args>
		dlog(std::ostream& _stream,
			 AffixSet _afs,
			 Args&& ... _args)
			:
			  out(_afs.log_level == 0 || _afs.log_level >= log_level),
			  afs(_afs),
			  stream(_stream)
		{
			if (out)
			{
				buffer << afs.prefix;
				gobble(std::forward<Args>(_args)...);
			}
		}

		~dlog()
		{
			if (out)
			{
				buffer << afs.suffix;
				flush(stream, buffer.str());
			}
		}

//		template<typename T>
//		dlog& operator << (const T& _t)
//		{
//			if (out)
//			{
//				std::stringstream ss;
//				ss << _t;
//				auto task(std::make_shared<ptask>([buf = buffer, inf = infix, tp = std::make_shared<std::string>(ss.str())]
//				{
//					*buf << *inf << *tp;
//				}
//				));
//				print_queue.emplace([=]{ (*task)(); });
//			}
//			return *this;
//		}

//		dlog& operator << (std::ostream& (*_fp)(std::ostream&))
//		{
//			if (out)
//			{
//				auto task(std::make_shared<ptask>([buf = buffer, inf = infix, fp = _fp]
//				{
//					*buf << *inf << fp;
//				}
//				));
//				print_queue.emplace([=]{ (*task)(); });
//			}
//			return *this;
//		}

//		template<typename T, typename std::enable_if<!std::is_void<T>::value, int>::type = 0>
//		dlog& operator << (std::future<T>& _future)
//		{
//			if (out)
//			{
//				sp<std::future<T>> future(std::make_shared<std::future<T>>(std::move(_future)));
//				auto task(std::make_shared<ptask>([buf = buffer, inf = infix, f = future]
//				{
//					*buf << *inf << f->get();
//				}
//				));
//				print_queue.emplace([=]{ (*task)(); });
//			}
//			return *this;
//		}

//		template<typename T, typename std::enable_if<std::is_void<T>::value, int>::type = 0>
//		dlog& operator << (std::future<T>& _future)
//		{
//			return *this;
//		}

		static void set_threads(const uint _count)
		{
			tp.resize(_count);
		}

	private:

		template<typename Param, typename ... Params>
		void gobble(Param&& _param, Params&& ... _params)
		{
			buffer << std::forward<Param>(_param);
			std::array<int, sizeof...(_params)> status{(buffer << afs.infix << std::forward<Params>(_params), 0) ...};
		}

		static void flush(std::ostream& _stream, std::string&& _content)
		{
			if (_content.size() > 0)
			{
				tp.enqueue([&, content = std::move(_content)]
				{
					glock lock(semaphore_mutex);
					std::ostream* os(std::addressof(_stream));
					if (os)
					{
						glock lk(semaphores[os]);
						*os << content;
					}
					else
					{
						semaphores.erase(os);
					}
				});
			}
		}
	};

	template<typename Stream, typename ... Args>
	inline void log(Stream& _stream, AffixSet _afs, Args&& ... _args)
	{
		dlog(_stream, _afs, std::forward<Args>(_args)...);
	}

	template<typename Stream, typename ... Args>
	inline void log(Stream& _stream, Args&& ... _args)
	{
		dlog(_stream, AffixSet(), std::forward<Args>(_args)...);
	}

	template<typename ... Args>
	inline void log(AffixSet _afs, Args&& ... _args)
	{
		dlog(std::cout, _afs, std::forward<Args>(_args)...);
	}
}

#endif // DLOG_HPP

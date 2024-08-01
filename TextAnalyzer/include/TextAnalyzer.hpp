#ifndef TEXTANALYZER_HPP
#define TEXTANALYZER_HPP

#include <iostream>
#include <fstream>
#include <filesystem>

#include <vector> 
#include <string>
#include <string_view>

#include <format>
#include <chrono>
#include <future>
#include <queue>
#include <algorithm>

constexpr auto THREADS_DEFAULT = 4;

namespace tean
{
	using position = std::pair<int, int>;

	struct compare_position
	{
		bool operator()(const position& a, const position& b)
		{
			return a.first >= b.first && a.second >= b.second;
		}
	};

	using positions_array = std::vector<position>;

	class text_analyzer
	{
	public:

		text_analyzer();

        text_analyzer(const std::string_view _file_path);

		positions_array get_indexes() const;

		size_t find_substr(const std::string substr, const size_t threads_count);

	private:

		void processing_threads_data();

		bool file_analyze();

		void configure();

		positions_array _find_substr(const std::string_view substr, const size_t offset, const size_t bytes_to_read);

        std::vector < std::future<positions_array>> threads{};
		positions_array substr_positions{};
		inline static std::atomic<int> thread_count = 1;

		std::filesystem::path file_path;
		uintmax_t text_size = 0;

		std::atomic_flag flag_write = ATOMIC_FLAG_INIT;
    };

    static std::ostream& operator<<(std::ostream& ost, const position& pos)
    {
	    return ost << std::format("Row - {}; Col - {};\n", pos.first, pos.second);
    }
   
}

#endif// TEXTANALYZER_HPP
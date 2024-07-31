#include "../include/TextAnalyzer.hpp"

namespace fs = std::filesystem;
using namespace tean;

/*inline static atomic<int> new_bytes = 0;

void* operator new(std::size_t count) {
	new_bytes.fetch_add(count);
	return malloc(count);
}*/


text_analyzer::text_analyzer() {};

text_analyzer::text_analyzer(const std::string_view _file_path) : text_analyzer()
{
	file_path = _file_path;
}

positions_array tean::text_analyzer::get_indexes() const
{
    return substr_positions;
}

size_t text_analyzer::find_substr(const std::string substr, const size_t threads_count = 1)
{

	if (!file_analyze())
		return -1;

	if (substr.size() == 0)
		return 0;

	threads.clear();
	threads.resize(threads_count);

	size_t rem = text_size % threads_count;
	size_t string_size = text_size / threads_count;
	size_t pos = 0;

	//auto wait = async(launch::async, &text_analyzer::wait_threads, this);
	for (auto& my_thread : threads)
	{
		my_thread = async(std::launch::async, &text_analyzer::_find_substr, this, substr, pos, string_size + (rem > 0 ? 1 : 0));
		pos += string_size + (rem > 0 ? 1 : 0);
		if (rem > 0)
			rem--;
	}
	flag_write.test_and_set();
	flag_write.notify_one();

	processing_threads_data();

	return substr_positions.size();
}

void text_analyzer::processing_threads_data()
{
	int row_id = 1;
	int last_col_id = 0;

	int this_last_row_id = 0;
	int this_last_col_id = 0;

	positions_array::iterator info_pair;
	for (auto& my_thread : threads)
	{
		my_thread.wait();
		positions_array temp(move(my_thread.get()));

		info_pair = temp.end() - 1;

		this_last_row_id = info_pair->first;
		this_last_col_id = info_pair->second;

		temp.erase(info_pair);

		for (auto& pos : temp)
		{
			if (pos.first == 0)
				pos.second += last_col_id;
			pos.second++;
			pos.first += row_id;
		}
		if(temp.size() != 0)
			substr_positions.insert(substr_positions.end(), std::make_move_iterator(temp.begin()), std::make_move_iterator(temp.end()));
		//substr_positions.append_range(move(temp));
		
		if (this_last_row_id == 0)
			last_col_id += this_last_col_id;
		else
			last_col_id = this_last_col_id;
		row_id += this_last_row_id;
	}
}

bool text_analyzer::file_analyze()
{
	text_size = fs::file_size(file_path);
	if (!fs::exists(file_path) || text_size == 0)
		return false;

	std::cout << std::format("Text size - {} bytes", text_size) << std::endl;

	return true;
};

positions_array text_analyzer::_find_substr(const std::string_view substr, const size_t offset = 0, const size_t bytes_to_read = 0)
{
	//auto begin = chrono::system_clock::now();

	int thread_id = thread_count.load();
	thread_count.fetch_add(1);

	//cout << format("Thread {} from pos {} - {} bytes\n", thread_id, offset, bytes_to_read);

	std::ifstream file(file_path.native(), std::ios::binary);
	file.seekg(offset, std::ios_base::beg);

	size_t substr_size = substr.size();
	positions_array temp_positions{};

	if (file.eof() || substr_size == 0)
		return temp_positions;

	int symbols_found = 0;
	char ch = 0;

	int col_id = 0;
	int row_id = 0;

	int overflow = 0;

	for (size_t i = 0; ((bytes_to_read > 0 ? i < bytes_to_read : true) || symbols_found != 0) && !file.eof(); i++)
	{
		if (symbols_found == substr_size)
		{
			temp_positions.push_back(std::make_pair(row_id, int(col_id + overflow - substr_size)));

			symbols_found = 0;
		}
		ch = file.get();
		if (ch == '\n')
		{
			row_id++;
			col_id = 0;
			symbols_found = 0;
		}
		else
		{
			if (ch == substr[symbols_found])
				symbols_found++;
			else
				symbols_found = 0;
			if (i < bytes_to_read)
				col_id++;
			else
				overflow++;
		}
	}

	file.close();

	//auto end = chrono::system_clock::now();

	//cout << format("Thread {} time - {} mcs\n", thread_id, chrono::duration_cast<chrono::microseconds>(end - begin).count());

	temp_positions.push_back(std::make_pair(row_id, col_id));

	return temp_positions;
}

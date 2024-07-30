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
#include <tuple>

constexpr auto THREADS_DEFAULT = 4;
constexpr auto FILENAME_DEFAULT = "file.txt";

namespace fs = std::filesystem;
using namespace std;

/*inline static atomic<int> new_bytes = 0;

void* operator new(std::size_t count) {
	new_bytes.fetch_add(count);
	return malloc(count);
}*/

namespace tean
{
	using position = pair<int, int>;

	struct compare_position
	{
		bool operator()(const position& a, const position& b)
		{
			return a.first >= b.first && a.second >= b.second;
		}
	};

	using positions_array = vector<position>;

	class text_analyzer
	{
	public:

		text_analyzer() {};

		text_analyzer(const string_view _file_path) : text_analyzer()
		{
			file_path = _file_path;
		}

		positions_array get_indexes() const
		{
			return substr_positions;
		}

		static positions_array get_positions(positions_array indexes)
		{
			positions_array substr_positions{};

			return substr_positions;
		}

		size_t find_substr(const string substr, const size_t threads_count = 1)
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
				my_thread = async(launch::async, &text_analyzer::_find_substr, this, substr, pos, string_size + (rem > 0 ? 1 : 0));
				pos += string_size + (rem > 0 ? 1 : 0);
				if (rem > 0)
					rem--;
			}
			flag_write.test_and_set();
			flag_write.notify_one();

			processing_threads_data();

			return substr_positions.size();
		}

	private:

		void processing_threads_data()
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

				for (size_t i = 0; i < temp.size(); i++)
				{
					if (temp[i].first == 0)
						temp[i].second += last_col_id;
					temp[i].second++;
					temp[i].first += row_id;
				}
				substr_positions.append_range(move(temp));
				
				if (this_last_row_id == 0)
					last_col_id += this_last_col_id;
				else
					last_col_id = this_last_col_id;
				row_id += this_last_row_id;
			}
		}

		bool file_analyze()
		{
			text_size = fs::file_size(file_path);
			if (!fs::exists(file_path) || text_size == 0)
				return false;

			cout << format("Text size - {} bytes", text_size) << endl;

			return true;
		};

		positions_array _find_substr(const string_view substr, const size_t offset = 0, const size_t bytes_to_read = 0)
		{
			//auto begin = chrono::system_clock::now();

			int thread_id = thread_count.load();
			thread_count.fetch_add(1);

			//cout << format("Thread {} from pos {} - {} bytes\n", thread_id, offset, bytes_to_read);

			ifstream file(file_path.native(), ios::binary);
			file.seekg(offset, ios_base::beg);

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
					temp_positions.push_back(make_pair(row_id, col_id + overflow - substr_size));

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

			temp_positions.push_back(make_pair(row_id, col_id));

			return temp_positions;
		}

		vector < future<positions_array>> threads{};
		positions_array substr_positions{};
		inline static atomic<int> thread_count = 1;

		fs::path file_path;
		uintmax_t text_size = 0;

		atomic_flag flag_write = ATOMIC_FLAG_INIT;
	};

	static ostream& operator<<(ostream& ost, const position& pos)
	{
		return ost << format("Row - {}; Col - {};\n", pos.first, pos.second);
	}
}

using namespace tean;

int main(int argc, char** argv)
{
	fs::path file_path{};
	if (argc >= 2)
		file_path = argv[1];
	else
	{
		string_view path_to_exe = argv[0];
		auto path_to_folder = filesystem::path(path_to_exe).parent_path();
		for (const auto& p : fs::recursive_directory_iterator(path_to_folder))
			if (p.path().filename() == FILENAME_DEFAULT)
			{
				file_path = p.path();
				break;
			}
	}
	if (file_path == " " || !filesystem::exists(file_path))
	{
		cout << "Invalid file path" << endl;
		return -1;
	}
	cout << format("Path to .txt file - {}", file_path.string()) << endl;
	string substr{};
	if (argc >= 3)
	{
		substr = string(argv[2]);
		erase(substr, ' ');
	}
	else
		substr = "as";

	int threads_count{};

	if (argc >= 4)
		threads_count = stoi(argv[3]);

	if(threads_count == 0)
		threads_count = THREADS_DEFAULT;


	tean::text_analyzer text1(file_path.string());

	cout << format("Threads count - {}", threads_count) << endl;
	cout << format("String to find - {}", substr) << endl;

	auto begin = chrono::system_clock::now();

	size_t positions_count = text1.find_substr(substr, threads_count);

	auto end = chrono::system_clock::now();

	if (argc >= 5 && stoi(argv[4]) == 1)
	{
		auto text_positions = text1.get_indexes();

		for (const auto& pos : text_positions)
		{
			cout << pos;
		}
	}
	cout << format("Indexes found - {}", positions_count) << endl;
	cout << format("Program time - {} mcs\n", chrono::duration_cast<chrono::microseconds>(end - begin).count());
	//cout << format("New operator memory usage - {} bytes", new_bytes.load()) << endl;

	return 0;
}